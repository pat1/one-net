#include "filter.h"
#include "xtea_key.h"
#include "packet.h"
#include "string_utils.h"
#include <string>
#include <list>
#include <ostream>
#include <cstdio>
#include <cstring>
#include <climits>
using namespace std;


extern "C"
{
    #include "one_net_constants.h"
    #include "one_net_packet.h"
};



filter_range::filter_range()
{
    this->low = 0;
    this->high = 0;
}


filter_range::filter_range(uint64_t value)
{
    this->low = value;
    this->high = value;
}


filter_range::filter_range(uint64_t low, uint64_t high)
{
    if(low > high)
    {
        uint64_t tmp = low;
        low = high;
        high = tmp;
    }

    this->low = low;
    this->high = high;
}


filter_range::filter_range(const filter_range& orig)
{
    this->low = orig.low;
    this->high = orig.high;
}


filter_range& filter_range::operator = (const filter_range& that)
{
    this->low = that.low;
    this->high = that.high;
    return *this;
}


filter_range::~filter_range()
{

}


void filter_range::display_value(ostream& outs, uint64_t value, bool hex,
    bool key, unsigned int width)
{
    if(hex)
    {
        char value_str[100]; // make it bigger than we'll ever need
        char byte_str[3];
        unsigned int length;
        if(key)
        {
            length = width * 3;
        }
        else
        {
            length = width * 2 + 3;
            strcpy(value_str, "0x");
        }

        if(width == 0)
        {
            return;
        }

        value_str[length - 1] = 0;

        char* ptr = &value_str[length - 3];
        UInt8 byte;

        for(unsigned int i = 0; i < width; i++)
        {
            byte = (UInt8) (value & 0xFF);
            sprintf(byte_str, "%02X", byte);
            value >>= 8;
            memcpy(ptr, byte_str, 2);

            if(key && i < width - 1)
            {
                ptr--;
                *ptr = '-';
            }

            ptr -= 2;
        }

        outs << value_str;
    }
    else
    {
        outs << value;
    }
}


void filter_range::display_range(ostream& outs, bool hex, bool key,
    unsigned int width) const
{
    outs << "(";
    display_value(outs, low, hex, key, width);
    outs << " - ";
    display_value(outs, high, hex, key, width);
    outs << ")";
}


void filter_range::display(ostream& outs, bool hex, bool key,
    unsigned int width) const
{
    if(low == high)
    {
        display_value(outs, low, hex, key, width);
    }
    else
    {
        display_range(outs, hex, key, width);
    }
}


uint64_t filter_range::getlow() const
{
    return low;
}


uint64_t filter_range::gethigh() const
{
    return high;
}


void filter_range::setlow(uint64_t value)
{
    low = value;
}


void filter_range::sethigh(uint64_t value)
{
    high = value;
}


bool filter_range::inside(const filter_range& fr, uint64_t value)
{
    return (value >= fr.low && value <= fr.high);
}


filter_list::filter_list()
{
    this->accepted_values.clear();
}


filter_list::filter_list(const filter_list& orig)
{
    this->accepted_values.clear();
    list<filter_range>::const_iterator it;
    for(it = orig.accepted_values.begin(); it != orig.accepted_values.end();
        it++)
    {
        this->accepted_values.push_back(filter_range(it->getlow(),
            it->gethigh()));
    }
}


filter_list& filter_list::operator = (const filter_list& that)
{
    this->accepted_values.clear();
    list<filter_range>::const_iterator it;
    for(it = that.accepted_values.begin(); it != that.accepted_values.end();
        it++)
    {
        this->accepted_values.push_back(filter_range(it->getlow(),
            it->gethigh()));
    }

    return *this;
}


filter_list::~filter_list()
{

}


bool filter_list::value_accepted(uint64_t value) const
{
    if(accepted_values.empty())
    {
        return true;
    }
    int index;
    return find_value(value, index);
}


void filter_list::accept_value(uint64_t value)
{
    if(accepted_values.empty())
    {
        filter_range fr(value);
        accepted_values.push_back(fr);
        return;
    }


    int index;
    if(find_value(value, index))
    {
        return; // already in there.
    }


    bool index_1_less_found, index_1_more_found;
    list<filter_range>::iterator it = accepted_values.begin();
    advance(it, index);
    // uint64_t low;   // TODO -- low apears not to be used anywhere, so
                       // commenting out.  Is it supposed to be used?
    uint64_t high;

    if(value == 0)
    {
        index_1_less_found = false;
    }
    else
    {
        index_1_less_found = find_value(value - 1, index);
    }

    if(value == numeric_limits<uint64_t>::max())
    {
        index_1_more_found = false;
    }
    else
    {
        index_1_more_found = find_value(value + 1, index);
    }


    if(!index_1_less_found && !index_1_more_found)
    {
        filter_range fr(value);
        accepted_values.insert(it, fr);
    }
    else if(index_1_less_found && index_1_more_found)
    {
        // we pulled an "inside straight", so combine these two ranges into one
        high = it->gethigh();
        accepted_values.erase(it);
        it--;
        it->sethigh(high);
    }
    else if(index_1_less_found && !index_1_more_found)
    {
        it--;
        it->sethigh(value);
        return;
    }
    else
    {
        it->setlow(value);
    }
}


bool filter_list::accept_range(uint64_t low, uint64_t high)
{
    if(low > high)
    {
        return false;
    }

    if(accepted_values.empty())
    {
        filter_range fr(low, high);
        accepted_values.push_front(fr);
        return true;
    }

    int index_low_neighbor, index_high_neighbor, index_low, index_high;
    bool found_low_neighbor, found_high_neighbor, found_low, found_high;
    found_low = find_value(low, index_low);
    found_high = find_value(high, index_high);
    list<filter_range>::iterator it1,it2;
    it1 = it2 = accepted_values.begin();
    advance(it1, index_low);
    advance(it2, index_high);

    if(found_low && found_high)
    {
        if(index_low == index_high)
        {
            return true; // nothing to do.
        }

        // merging.  erase anything in between
        it1->sethigh(it2->gethigh());
        accepted_values.erase(it1, it2);
        return true;
    }

    if(!found_low)
    {
        if(low > 0)
        {
            found_low_neighbor = find_value(low - 1, index_low_neighbor);
        }
        if(low < numeric_limits<uint64_t>::max())
        {
            found_high_neighbor = find_value(low + 1, index_high_neighbor);
        }

        if(!found_low_neighbor && !found_high_neighbor)
        {
            filter_range fr(low, low);
            accepted_values.insert(it1, 1, fr);
        }
        else if(found_low_neighbor && found_high_neighbor)
        {
            // merge
            uint64_t new_high = it1->gethigh();
            it1--;
            it1->sethigh(new_high);
            it1++;
            accepted_values.erase(it1);

        }
        else if(found_low_neighbor)
        {
            it1--;
            it1->sethigh(low);
        }
        else
        {
            it1->setlow(low);
        }
    }
    else
    {
        if(high > 0)
        {
            found_low_neighbor = find_value(high - 1, index_low_neighbor);
        }
        if(high < numeric_limits<uint64_t>::max())
        {
            found_high_neighbor = find_value(high + 1, index_high_neighbor);
        }

        if(!found_low_neighbor && !found_high_neighbor)
        {
            filter_range fr(high, high);
            accepted_values.insert(it2, 1, fr);
        }
        else if(found_low_neighbor && found_high_neighbor)
        {
            // merge
            uint64_t new_high = it2->gethigh();
            it2--;
            it2->sethigh(new_high);
            it2++;
            accepted_values.erase(it2);

        }
        else if(found_low_neighbor)
        {
            it2--;
            it2->sethigh(high);
        }
        else
        {
            it2->setlow(high);
        }
    }

    return accept_range(low, high);
}


void filter_list::reject_value(uint64_t value)
{
    reject_range(value, value);
}


bool filter_list::reject_range(uint64_t low, uint64_t high)
{
    if(low > high)
    {
        return false;
    }

    // Scenario one -- no filters so far.  Make one excluding these values
    if(accepted_values.empty())
    {
        if(low > 0)
        {
            filter_range fr_low(0, low - 1);
            accepted_values.push_front(fr_low);
        }

        if(high < numeric_limits<uint64_t>::max())
        {
            filter_range fr_high(high + 1, numeric_limits<uint64_t>::max());
            accepted_values.push_back(fr_high);
        }
        return true;
    }

    int index_low, index_high;
    bool found_low, found_high;
    found_low = find_value(low, index_low);
    found_high = find_value(high, index_high);
    list<filter_range>::iterator it1,it2;
    it1 = it2 = accepted_values.begin();
    advance(it1, index_low);
    advance(it2, index_high);

    if(!found_low && !found_high)
    {
        if(index_low == index_high)
        {
            return true; // nothing to do.
        }

        // delete anything in between
        accepted_values.erase(it1, it2);
    }
    else if(found_low)
    {
        if(it1->getlow() == low && it1->gethigh() == low)
        {
            accepted_values.erase(it1);
        }
        else if(it1->getlow() == low)
        {
            it1->setlow(low + 1);
        }
        else if(it1->gethigh() == low)
        {
            it1->sethigh(low - 1);
        }
        else
        {
            it1->sethigh(low - 1);
            filter_range new_fr(low + 1, it1->gethigh());
            accepted_values.insert(it1, 1, new_fr);
        }
    }
    else
    {
        if(it2->getlow() == high && it2->gethigh() == high)
        {
            accepted_values.erase(it2);
        }
        else if(it2->getlow() == high)
        {
            it2->setlow(high + 1);
        }
        else if(it1->gethigh() == high)
        {
            it2->sethigh(high - 1);
        }
        else
        {
            it2->sethigh(high - 1);
            filter_range new_fr(high + 1, it2->gethigh());
            accepted_values.insert(it2, 1, new_fr);
        }
    }

    return reject_range(low, high);
}


void filter_list::display(ostream& outs, bool hex, bool key,
    unsigned int width) const
{
    if(accepted_values.size() == 0)
    {
        outs << "No filter";
        return;
    }


    list<filter_range>::const_iterator it;
    for(it = accepted_values.begin(); it != accepted_values.end(); it++)
    {
        it->display(outs, hex, key, width);
        outs << " ";
    }
}


void filter_list::remove_all()
{
    accepted_values.clear();
}


bool filter_list::find_value(uint64_t value, int& index) const
{
    list<filter_range>::const_iterator it;
    const int num_ranges = accepted_values.size();

    if(num_ranges == 0)
    {
        index = 0;
        return false;
    }

    it = accepted_values.begin();
    for(index = 0; index < num_ranges; index++)
    {
        if(value < it->getlow())
        {
            return false;
        }
        if(value <= it->gethigh())
        {
            return true;
        }
        it++;
    }

    return false;
}




const int filter::NUM_FILTER_TYPES = 29;
const int filter::NUM_MATCH_FILTERS = 4;
const int filter::NUM_KEY_FILTERS = 2;
const int filter::NUM_RANGE_FILTERS = filter::NUM_FILTER_TYPES -
    filter::NUM_MATCH_FILTERS - filter::NUM_KEY_FILTERS;



const string filter::FILTER_TYPE_STR[] =
{
    "timestamp",
    "rptr_did",
    "dst_did",
    "src_did",
    "nid",
    "pid",
    "msg_id",
    "msg_crc",
    "pld_crc",
    "txn_nonce",
    "resp_nonce",
    "pld_msg_type",
    "msg_class",
    "msg_type",
    "msg_data",
    "nack_reason",
    "ack_nack_handle",
    "admin_type",
    "ack_nack_time",
    "ack_nack_value",
    "key_fragment",
    "hops",
    "max_hops",
    "msg_crc_match",
    "payload_crc_match",
    "valid",
    "valid decode",
    "invite_keys",
    "keys"
};


const filter::filter_display_attribute filter::FILTER_DISPLAY_ATTRIBUTE[] =
{
    {false, false, 0}, // timestamp
    {true, false, 2}, // repeater did
    {true, false, 2}, // dest. did
    {true, false, 2}, // src did
    {true, false, 5}, // nid
    {true, false, 1}, // pid
    {true, false, 1}, // msg id
    {true, false, 1}, // msg crc
    {true, false, 1}, // pld crc
    {true, false, 1}, // txn nonce
    {true, false, 1}, // resp nonce
    {true, false, 1}, // payload message type
    {true, false, 2}, // message class
    {true, false, 2}, // message type
    {false, false, 0}, // message data
    {true, false, 1},  // nack reason
    {true, false, 1},  // ack / nack handle
    {true, false, 1},  // admin type
    {false, false, 0}, // ack / nack time
    {false, false, 0}, // ack / nack value
    {true,  true,  4}, // key fragment
    {false, false, 0}, // hops
    {false, false, 0}  // max hops
};


const string filter::FILTER_MATCH_STR[] =
{
    "No filter",
    "Must Match",
    "Must Not Match"
};


filter::filter()
{
    this->filters.resize(NUM_RANGE_FILTERS);
    this->msg_crc_match = MUST_MATCH;
    this->pld_crc_match = MUST_MATCH;
    this->valid_decode_match = MUST_MATCH;
    this->valid_match = MUST_MATCH;
    this->keys = xtea_key::copy_keys(packet::keys, true);
    this->invite_keys = xtea_key::copy_keys(packet::invite_keys, true);
}


filter::filter(const filter& orig)
{
    for(int i = 0; i < NUM_RANGE_FILTERS; i++)
    {
        filter_list fl(orig.filters.at(i));
        filters.push_back(fl);
    }

    this->msg_crc_match = orig.msg_crc_match;
    this->pld_crc_match = orig.pld_crc_match;
    this->valid_decode_match = orig.valid_decode_match;
    this->valid_match = orig.valid_match;
    this->keys = xtea_key::copy_keys(orig.keys, true);
    this->invite_keys = xtea_key::copy_keys(orig.invite_keys, true);
}


filter& filter::operator = (const filter& that)
{
    if(this == &that)
    {
        return *this; // self-assignment
    }

    int size = filters.size();
    for(int i = 0; i < size; i++)
    {
        this->filters[i] = that.filters[i];
    }
    this->msg_crc_match = that.msg_crc_match;
    this->pld_crc_match = that.pld_crc_match;
    this->valid_match = that.valid_match;
    this->valid_decode_match = that.valid_match;
    this->keys = xtea_key::copy_keys(that.keys, true);
    this->invite_keys = xtea_key::copy_keys(that.invite_keys, true);
    return *this;
}


filter::~filter()
{
}


filter::FILTER_TYPE filter::string_to_filter_type(const string& ft_string)
{
    for(int i = 0; i < NUM_FILTER_TYPES; i++)
    {
        if(FILTER_TYPE_STR[i].compare(ft_string) == 0)
        {
            return (FILTER_TYPE) i;
        }
    }
    return FILTER_INVALID;
}


string filter::filter_type_to_string(FILTER_TYPE ft)
{
    if(ft == FILTER_INVALID)
    {
        return "";
    }

    return FILTER_TYPE_STR[(int) ft];
}


bool filter::value_accepted(FILTER_TYPE ft, uint64_t value) const
{
    if((int)ft >= (int)FILTER_MSG_CRC_MATCH)
    {
        return false;
    }

    return this->filters[(int)ft].value_accepted(value);
}


bool filter::match_value_accepted(FILTER_TYPE ft, bool value) const
{
    FILTER_MATCH fm;
    switch(ft)
    {
        case FILTER_MSG_CRC_MATCH:
            fm = msg_crc_match; break;
        case FILTER_PAYLOAD_CRC_MATCH:
            fm = pld_crc_match; break;
        case FILTER_VALID_DECODE_MATCH:
            fm = valid_decode_match; break;
        case FILTER_VALID_MATCH:
            fm = valid_match; break;
        default:
            return false;
    }

    if(fm == WILDCARD)
    {
        return true;
    }
    else if(value && fm == MUST_MATCH)
    {
        return true;
    }
    else if(!value && fm == MUST_NOT_MATCH)
    {
        return true;
    }

    return false;
}


bool filter::value_accepted(FILTER_TYPE ft, const xtea_key& key) const
{
    switch(ft)
    {
        case FILTER_INVITE_KEYS:
            return (xtea_key::find_key(invite_keys, key) >= 0);
        case FILTER_KEYS:
            return (xtea_key::find_key(keys, key) >= 0);
        default:
            return false;
    }
}


bool filter::set_match_value(FILTER_TYPE ft, FILTER_MATCH fm)
{
    if(ft == FILTER_VALID_MATCH)
    {
        valid_match = fm;
    }
    else if(ft == FILTER_VALID_DECODE_MATCH)
    {
        valid_decode_match = fm;
    }
    else if(ft == FILTER_MSG_CRC_MATCH)
    {
        msg_crc_match = fm;
    }
    else if(ft == FILTER_PAYLOAD_CRC_MATCH)
    {
        pld_crc_match = fm;
    }
    else
    {
        return false;
    }

    return true;
}


bool filter::accept_value(FILTER_TYPE ft, uint64_t value)
{
    switch(ft)
    {
        case FILTER_INVITE_KEYS:
            if(value == 0 || value > packet::invite_keys.size())
            {
                return false;
            }
            xtea_key::insert_key(invite_keys, packet::invite_keys.at(value - 1),
                true);
            return true;
        case FILTER_KEYS:
            if(value == 0 || value > packet::keys.size())
            {
                return false;
            }
            xtea_key::insert_key(keys, packet::keys.at(value - 1), true);
            return true;
        case FILTER_INVALID:
            return false;
        case FILTER_MSG_CRC_MATCH:
            if(value > (int) MUST_NOT_MATCH)
            {
                return false;
            }
            msg_crc_match = (FILTER_MATCH) value;
            return true;
        case FILTER_PAYLOAD_CRC_MATCH:
            if(value > (int) MUST_NOT_MATCH)
            {
                return false;
            }
            pld_crc_match = (FILTER_MATCH) value;
            return true;
        case FILTER_VALID_DECODE_MATCH:
            if(value > (int) MUST_NOT_MATCH)
            {
                return false;
            }
            valid_decode_match = (FILTER_MATCH) value;
            return true;
        case FILTER_VALID_MATCH:
            if(value > (int) MUST_NOT_MATCH)
            {
                return false;
            }
            valid_match = (FILTER_MATCH) value;
            return true;
        default:
            return accept_range(ft, value, value);
    }
}


bool filter::accept_values(FILTER_TYPE ft, vector<uint64_t> values)
{
    int len = values.size();
    for(int i = 0; i < len; i++)
    {
        if(!accept_value(ft, values.at(i)))
        {
            return false;
        }
    }

    return true;
}


bool filter::accept_range(FILTER_TYPE ft, uint64_t low, uint64_t high)
{
    if((int)ft >= (int)FILTER_MSG_CRC_MATCH)
    {
        return false;
    }

    return filters[(int) ft].accept_range(low, high);
}


bool filter::reject_value(FILTER_TYPE ft, uint64_t value)
{
    if(ft == FILTER_INVITE_KEYS)
    {
        if(value == 0 || value > invite_keys.size())
        {
            return false;
        }

        invite_keys.erase(invite_keys.begin() + (value - 1));
        return true;
    }
    else if(ft == FILTER_KEYS)
    {
        if(value == 0 || value > keys.size())
        {
            return false;
        }

        keys.erase(keys.begin() + (value - 1));
        return true;
    }
    else if((int)ft >= (int)FILTER_MSG_CRC_MATCH)
    {
        return false;
    }

    return reject_range(ft, value, value);
}


bool filter::reject_values(FILTER_TYPE ft, vector<uint64_t> values)
{
    if(ft != FILTER_KEYS && ft != FILTER_INVITE_KEYS && (int)ft >=
        (int)FILTER_MSG_CRC_MATCH)
    {
        return false;
    }

    sort(values.begin(), values.end());
    int len = values.size();
    for(int i = len - 1; i >= 0; i--)
    {
        if(!reject_value(ft, values.at(i)))
        {
            return false;
        }
    }
    return true;
}


bool filter::reject_range(FILTER_TYPE ft, uint64_t low, uint64_t high)
{
    if((int)ft >= (int)FILTER_MSG_CRC_MATCH)
    {
        return false;
    }

    return filters[(int)ft].reject_range(low, high);
}


void filter::display(ostream& outs) const
{
    filter_display_attribute fda;
    string title;

    for(int i = 0; i < NUM_RANGE_FILTERS; i++)
    {
        title = FILTER_TYPE_STR[i];
        fda = FILTER_DISPLAY_ATTRIBUTE[i];
        outs << title << " : ";
        filters.at(i).display(outs, fda.hex, fda.key, fda.width);
        outs << "\n";
    }

    outs << "Message CRC Match : " <<
        FILTER_MATCH_STR[(int) msg_crc_match] << "\n";
    outs << "Payload CRC Match : " <<
        FILTER_MATCH_STR[(int) pld_crc_match] << "\n";
    outs << "Valid Decode Match : " <<
        FILTER_MATCH_STR[(int) valid_decode_match] << "\n";
    outs << "Valid Match : " <<
        FILTER_MATCH_STR[(int) valid_match] << "\n";
    outs << "Invite Keys...\n";
    display_keys(outs, true, invite_keys);
    outs << "\nKeys...\n";
    display_keys(outs, false, keys);
}


void filter::remove_filter()
{
    for(vector<filter_list>::iterator it = filters.begin(); it != filters.end();
        it++)
    {
        it->remove_all();
    }

    msg_crc_match = WILDCARD;
    pld_crc_match = WILDCARD;
    valid_decode_match = WILDCARD;
    valid_match = WILDCARD;
    invite_keys = xtea_key::copy_keys(packet::invite_keys, true);
    keys = xtea_key::copy_keys(packet::keys, true);
}


bool filter::remove_filter(FILTER_TYPE ft)
{
    switch(ft)
    {
        case FILTER_INVALID:
            return false;
        case FILTER_MSG_CRC_MATCH:
        case FILTER_PAYLOAD_CRC_MATCH:
        case FILTER_VALID_DECODE_MATCH:
        case FILTER_VALID_MATCH:
            return this->set_match_value(ft, WILDCARD);
        case FILTER_INVITE_KEYS:
            invite_keys = xtea_key::copy_keys(packet::invite_keys, true); break;
        case FILTER_KEYS:
            keys = xtea_key::copy_keys(packet::keys, true); break;
        default:
            filters.at((int) ft).remove_all();
    }

    return true;
}


const vector<xtea_key>* filter::get_keys(bool invite) const
{
    return (invite ? &invite_keys : &keys);
}
