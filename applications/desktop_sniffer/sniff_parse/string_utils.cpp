#include "one_net_port_specific.h"
#include "string_utils.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>



// return -1 if all whitespace
int find_first_non_whitespace_index(std::string& str)
{
    int stringlen = str.length();
    int i = 0;

    while(i < stringlen)
    {
        if(!isspace(str[i]))
        {
            return i;
        }
        i++;
    }

    return -1;
}


// return -1 if all whitespace
int find_last_non_whitespace_index(std::string& str)
{
    int stringlen = str.length();
    int i = stringlen - 1;

    while(i >= 0)
    {
        if(!isspace(str[i]))
        {
            return i;
        }
        i--;
    }

    return -1;
}


// return -1 if all whitespace
int find_first_whitespace_index(std::string& str)
{
    int stringlen = str.length();
    int i = 0;

    while(i < stringlen)
    {
        if(isspace(str[i]))
        {
            return i;
        }
        i++;
    }

    return -1;
}


// return -1 if all whitespace
int find_last_whitespace_index(std::string& str)
{
    int stringlen = str.length();
    int i = stringlen - 1;

    while(i >= 0)
    {
        if(isspace(str[i]))
        {
            return i;
        }
        i--;
    }

    return -1;
}


void strip_leading_whitespace(std::string& str)
{
    int index = find_first_non_whitespace_index(str);

    if(index == -1)
    {
        str = "";
        return;
    }

    str = str.substr(index);
}


void strip_trailing_whitespace(std::string& str)
{
    int index = find_last_non_whitespace_index(str);

    if(index == -1)
    {
        str = "";
        return;
    }

    str = str.substr(0, index + 1);
}


void strip_leading_and_trailing_whitespace(std::string& str)
{
    strip_leading_whitespace(str);
    strip_trailing_whitespace(str);
}


// for some reason, can't seem to use isspace from cctype so use this wrapper
static bool iswhitespace(char c)
{
    return isspace(c);
}


bool ishyphen(char c)
{
    return (c == '-');
}


void strip_all_whitespace(std::string& str)
{
    str.erase(std::remove_if(str.begin(), str.end(), iswhitespace), str.end());
}


void strip_all_hyphens(std::string& str)
{
    str.erase(std::remove_if(str.begin(), str.end(), ishyphen), str.end());
}


void str_toupper(std::string& str)
{
    int stringlen = str.length();
    for(int i = 0; i < stringlen; i++)
    {
        str[i] = toupper(str[i]);
    }
}


void str_tolower(std::string& str)
{
    int stringlen = str.length();
    for(int i = 0; i < stringlen; i++)
    {
        str[i] = tolower(str[i]);
    }
}


std::string capitalize(std::string str)
{
    if(str.length() == 0)
    {
        return "";
    }

    int stringlen = str.length();
    std::string new_string = str;
    new_string[0] = toupper(new_string[0]);
    for(int i = 1; i < stringlen; i++)
    {
        if(isalpha(new_string[i-1]) || new_string[i-1] == '-')
        {
            new_string[i] = tolower(new_string[i]);
        }
        else
        {
            new_string[i] = toupper(new_string[i]);
        }
    }
    return new_string;
}


bool legal_digits(const std::string& value_str, bool hex, bool whitespace_allowed)
{
    int len = value_str.length();
    for(int i = 0; i < len; i++)
    {
        if(isspace(value_str[i]))
        {
            if(whitespace_allowed)
            {
                continue;
            }
            return false;
        }
        else if(isdigit(value_str[i]))
        {
            continue;
        }
        else if(isxdigit(value_str[i]))
        {
            if(hex)
            {
                continue;
            }
        }

        return false;
    }

    return true;
}


bool string_to_uint8(std::string str, UInt8& value, bool hex)
{
    uint64_t u64_val;
    if(!string_to_uint64_t(str, u64_val, hex))
    {
        return false;
    }

    if(u64_val > 0xFF)
    {
        return false;
    }

    value = (UInt8) u64_val;
    return true;
}


bool string_to_uint16(std::string str, UInt16& value, bool hex)
{
    uint64_t u64_val;
    if(!string_to_uint64_t(str, u64_val, hex))
    {
        return false;
    }

    if(u64_val > 0xFFFFFF)
    {
        return false;
    }

    value = (UInt16) u64_val;
    return true;
}


bool string_to_uint32(std::string str, UInt32& value, bool hex)
{
    uint64_t u64_val;
    if(!string_to_uint64_t(str, u64_val, hex))
    {
        return false;
    }

    if(u64_val > 0xFFFFFFFF)
    {
        return false;
    }

    value = (UInt32) u64_val;
    return true;
}


bool string_to_uint64_t(std::string str, uint64_t& value, bool hex)
{
    if(!legal_digits(str, hex, false))
    {
        return false;
    }

    const char* format = hex ? "%llx" : "%lld";
    if(sscanf(str.c_str(), format, &value) != 1)
    {
        return false;
    }

    return true;
}


bool string_to_int8(std::string str, SInt8& value, bool hex)
{
    int64_t s64_val;
    if(!string_to_int64_t(str, s64_val, hex))
    {
        return false;
    }

    if(s64_val > 0x7F || s64_val < -0x80)
    {
        return false;
    }

    value = (SInt8) s64_val;
    return true;
}


bool string_to_int16(std::string str, SInt16& value, bool hex)
{
    int64_t s64_val;
    if(!string_to_int64_t(str, s64_val, hex))
    {
        return false;
    }

    if(s64_val > 0x7FFF || s64_val < -0x8000)
    {
        return false;
    }

    value = (SInt16) s64_val;
    return true;
}


bool string_to_int32(std::string str, SInt32& value, bool hex)
{
    int64_t s64_val;
    if(!string_to_int64_t(str, s64_val, hex))
    {
        return false;
    }

    if(s64_val > 0x7FFFFFFF || s64_val < -0x80000000)
    {
        return false;
    }

    value = (SInt32) s64_val;
    return true;
}


bool string_to_int64_t(std::string str, int64_t& value, bool hex)
{
    if(str.length() > 0 && str[0] == '-')
    {
        bool ret = string_to_int64_t(str.substr(1), value, hex);
        value = -value;
        return ret;
    }
    if(!legal_digits(str, hex, false))
    {
        return false;
    }

    const char* format = hex ? "%llx" : "%lld";
    if(sscanf(str.c_str(), format, &value) != 1)
    {
        return false;
    }

    return true;
}


bool string_to_bool(const std::string& str, bool& value)
{
    if(str == "true")
    {
        value = true;
    }
    else if(str == "false")
    {
        value = false;
    }
    else
    {
        return false;
    }
    return true;
}
















bool nibble_to_hex_char(UInt8 nibble, char& hex_char)
{
    if(nibble > 15)
    {
        return false;
    }
    else if(nibble < 10)
    {
        hex_char = (nibble + '0');
    }
    else
    {
        hex_char = (nibble + 'A' -  10);
    }

    return true;
}


bool hex_char_to_nibble(char hex_char, UInt8& nibble)
{
    hex_char = toupper(hex_char);
    if(!isxdigit(hex_char))
    {
        return false;
    }
    if(isdigit(hex_char))
    {
        nibble = hex_char - '0';
    }
    else
    {
        nibble = hex_char - 'A' + 10;
    }

    return true;
}

std::string byte_to_hex_string(UInt8 byte)
{
    std::string hex_string = "00";
    nibble_to_hex_char((byte >> 4), hex_string[0]);
    nibble_to_hex_char((byte &0x0F), hex_string[1]);
    return hex_string;
}


bool hex_string_to_byte(std::string hex_string, UInt8& byte)
{
    if(hex_string.length() != 2)
    {
        return false;
    }

    UInt8 high_nib, low_nib;
    if(!hex_char_to_nibble(hex_string[0], high_nib) || !hex_char_to_nibble(
        hex_string[1], low_nib))
    {
        return false;
    }

    byte = (high_nib << 4) + low_nib;
    return true;
}


std::string bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes)
{
    return bytes_to_hex_string(bytes, num_bytes, ' ', 0, 0);
}


std::string bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes,
    char separator, UInt8 num_spaces_between, UInt8 bytes_per_line)
{
    std::string hex_string = "";
    if(bytes == NULL)
    {
        return hex_string;
    }
    std::string separator_string = "";
    for(int i = 0; i < num_spaces_between; i++)
    {
        separator_string += separator;
    }
    for(int i = 0; i < num_bytes; i++)
    {
        if(i > 0)
        {
            if(bytes_per_line > 0 && i % bytes_per_line == 0)
            {
                hex_string += "\n";
            }
            else
            {
                hex_string += separator_string;
            }
        }

        hex_string += byte_to_hex_string(bytes[i]);
    }

    return hex_string;
}


std::string uint16_to_hex_string(UInt16 value)
{
    UInt8 bytes[2];
    std::string str;
    one_net_int16_to_byte_stream(value, bytes);
    str = bytes_to_hex_string(bytes, 2);
    return str;
}


std::string uint32_to_hex_string(UInt32 value)
{
    std::string str;
    UInt8 bytes[4];
    one_net_int32_to_byte_stream(value, bytes);
    str = bytes_to_hex_string(bytes, 4);
    return str;
}


std::string uint64_to_hex_string(uint64_t value)
{
    std::string msb_string, lsb_string, str;
    UInt32 msb = (value >> 32);
    UInt32 lsb = (value & 0xFFFFFFFF);
    msb_string = uint32_to_hex_string(msb);
    lsb_string = uint32_to_hex_string(lsb);
    str = msb_string + lsb_string;
    return str;
}


bool hex_string_to_bytes(std::string hex_string, UInt8* bytes, UInt8& num_bytes)
{
    int string_len = hex_string.length();
    if(string_len % 2 != 0 || string_len > 2 * num_bytes)
    {
        return false;
    }

    num_bytes = (UInt8) (string_len / 2);
    for(int i = 0; i < num_bytes; i ++)
    {
        if(!hex_string_to_byte(hex_string.substr(i * 2, 2), bytes[i]))
        {
            return false;
        }
    }

    return true;
}


// Return true if valid encoded value
bool encoded_did_to_string(const on_encoded_did_t* enc_did, std::string& str)
{
    if(!enc_did)
    {
        str = "Internal Error";
        return false;
    }
    on_raw_did_t raw_did;
    bool ret = true;
    if(on_decode(raw_did, *enc_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        ret = false;
    }
    str = bytes_to_hex_string((const UInt8*) enc_did, 2);
    str = "0x" + str;
    return ret;
}


// Return true if valid encoded value
bool encoded_did_to_string(UInt16 enc_did, std::string& str)
{
    UInt16 decoded;
    bool ret = (on_decode_uint16(&decoded, enc_did) == ONS_SUCCESS);
    str = "0x" + byte_to_hex_string(enc_did >> 8) + byte_to_hex_string(enc_did & 0xFF);
    return ret;
}


bool raw_did_to_string(uint16_t raw_did, std::string& str)
{
    if(raw_did > 0xFFF)
    {
        return false;
    }
    char msb_nibble;
    nibble_to_hex_char((raw_did >> 8) & 0x0F, msb_nibble);
    UInt8 least_sig = (UInt8) (raw_did & 0xFF);
    std::string least_sig_string = byte_to_hex_string(least_sig);
    str = "0x";
    str += msb_nibble;
    str += least_sig_string;
    return true;
}


// true if NID has a valid encoding, false otherwise.
bool encoded_nid_to_string(const on_encoded_nid_t* enc_nid, std::string& str)
{
    if(!enc_nid)
    {
        str = "Internal Error";
        return false;
    }
    on_raw_nid_t raw_nid;
    bool ret = true;
    if(on_decode(raw_nid, *enc_nid, ON_ENCODED_NID_LEN) != ONS_SUCCESS)
    {
        ret = false;
    }
    str = bytes_to_hex_string((const UInt8*) enc_nid, 6);
    str = "0x" + str;
    return true;
}


bool encoded_nid_to_string(uint64_t enc_nid, std::string& str)
{
    std::string enc_nid_str;
    UInt8 enc_nid_bytes[8];
    enc_nid_str = uint64_to_hex_string(enc_nid);
    UInt8 num_bytes;
    hex_string_to_bytes(enc_nid_str, enc_nid_bytes, num_bytes);
    return encoded_nid_to_string((const on_encoded_nid_t*) &enc_nid_bytes[2], str);
}


bool raw_nid_to_string(uint64_t raw_nid, std::string& str)
{
    UInt32 most_sig = (raw_nid >> 32);
    if(most_sig > 0xF)
    {
        return false;
    }
    char msb_nibble;
    nibble_to_hex_char(((UInt8) most_sig) & 0x0F, msb_nibble);
    UInt32 least_sig = (UInt32) (raw_nid & 0xFFFFFFFF);
    std::string least_sig_string;
    least_sig_string = uint32_to_hex_string(least_sig);
    str = "0x";
    str += msb_nibble;
    str += least_sig_string;
    return true;
}


void struct_timeval_to_string(struct timeval timestamp, std::string& str)
{
    std::stringstream ss;
    ss << timestamp.tv_sec;
    ss << " ";
    ss << std::setfill('0') << std::setw(6) << timestamp.tv_usec;
    std::string sec_str, usec_str;
    ss >> sec_str >> usec_str;
    str = sec_str + "." + usec_str;
}


//helper function
static unsigned int msb(UInt32 value)
{
    unsigned int msb = 31;
    unsigned int mask = 0x80000000;

    if(value < 2)
    {
        return 0;
    }

    while(value < mask)
    {
        msb--;
        mask >>= 1;
    }

    return msb;
}


std::string value_to_bit_string(UInt32 value, unsigned int num_bits)
{
    if(num_bits == 0)
    {
        num_bits = msb(value) + 1;
    }

    std::string str;
    str.assign(num_bits, '0');

    unsigned int mask = 1;
    for(unsigned int i = num_bits; i > 0; i--)
    {
        if(mask & value)
        {
            str[i-1] = '1';
        }
        mask <<= 1;
    }
    return str;
}


std::string bytes_to_bit_string(const UInt8* bytes, unsigned int num_bytes)
{
    std::string str = "";
    if(bytes == NULL)
    {
        return str;
    }

    for(unsigned int i = 0; i < num_bytes; i++)
    {
        str += value_to_bit_string(bytes[i], 8);
    }
    return str;
}


bool string_to_xtea_key(std::string key_string, one_net_xtea_key_t& key)
{
    strip_all_whitespace(key_string);
    strip_all_hyphens(key_string);
    if(key_string.length() != 2 * ONE_NET_XTEA_KEY_LEN)
    {
        return false;
    }
    if(!legal_digits(key_string, true, false))
    {
        return false;
    }

    UInt8 tmp = ONE_NET_XTEA_KEY_LEN;
    return hex_string_to_bytes(key_string, key, tmp);
}


void xtea_key_to_string(std::string& key_string, one_net_xtea_key_t& key)
{
    key_string = bytes_to_hex_string((const UInt8*) key, (UInt8) ONE_NET_XTEA_KEY_LEN);
}


bool format_invite_key_fragment(std::string& string_rep,
  const one_net_xtea_key_fragment_t& key_frag)
{
    string_rep = "";
    for(int i = 0; i < ONE_NET_XTEA_KEY_FRAGMENT_SIZE; i++)
    {
        char c = key_frag[i];
        if(!isalnum(c))
        {
            string_rep = "Unprintable";
            return false;
        }
        
        string_rep += c;
    }
    
    return true;
}


bool format_invite_key_string(std::string& string_rep,
  const one_net_xtea_key_t& key)
{
    string_rep = "";
    bool first_and_second_halves_match = (memcmp(&key[0], &key[ONE_NET_XTEA_KEY_LEN / 2],
      ONE_NET_XTEA_KEY_LEN / 2) == 0);

    for(int i = 0; i < 4; i++)
    {
        if(first_and_second_halves_match && i >= 2)
        {
            return true;
        }
        if(i > 0)
        {
            string_rep += "-";
        }

        std::string key_frag_string_rep;
        one_net_xtea_key_fragment_t key_frag;
        memcpy(key_frag, &key[i * ONE_NET_XTEA_KEY_FRAGMENT_SIZE],
          ONE_NET_XTEA_KEY_FRAGMENT_SIZE);

        if(!format_invite_key_fragment(key_frag_string_rep, key_frag))
        {
            string_rep = "Unprintable";
            return false;
        }
        string_rep += key_frag_string_rep;
    }
    return true;
}


std::string format_key_string(const one_net_xtea_key_t& key)
{
    std::string string_rep = "";
    for(int i = 0; i < ONE_NET_XTEA_KEY_LEN; i++)
    {
        if(i > 0 & i % 4 == 0)
        {
            string_rep += "-";
        }
        string_rep += byte_to_hex_string(key[i]);
    }
    return string_rep;
}


void display_keys(ostream& outs, bool invite_key, const vector<xtea_key>& keys)
{
    unsigned int num_keys = keys.size();

    if(invite_key)
    {
        outs << "\n# of invite keys : " << num_keys << "\n";
    }
    else
    {
        outs << "\n# of keys : " << num_keys << "\n";
    }

    std::string str;
    for(unsigned int i = 0; i < num_keys; i++)
    {
        outs << "Key " << i + 1 << " : ";
        xtea_key key = keys[i];
        if(invite_key)
        {
            format_invite_key_string(str, key.bytes);
            outs << str;
        }
        else
        {
            outs << format_key_string(key.bytes);
        }
        outs << "\n";
    }
}







// Note: This function assumes that the encoded has been checked for validity
std::string detailed_did_display(UInt16 encoded_did, UInt16 raw_did)
{
    std::string tmp;
    std::string str = "Encoded DID: ";
    encoded_did_to_string(encoded_did, tmp);
    str += tmp;
    str += ", Decoded DID: ";
    raw_did_to_string(raw_did, tmp);
    str += tmp;
    return str;
}


// Note: This function assumes that the encoded has been checked for validity
std::string detailed_did_display(UInt16 did, bool is_encoded)
{
    UInt16 encoded_did = did;
    UInt16 raw_did;
    on_decode_uint16(&raw_did, encoded_did);
    if(!is_encoded)
    {
        raw_did = did;
        on_encode_uint16(&encoded_did, raw_did);
    }
    return detailed_did_display(encoded_did, raw_did);
}




std::map<int, std::string> create_int_string_map(const string_int_struct pairs[],
  unsigned int size)
{
    std::map<int, std::string> int_string_map;
    for(unsigned int i = 0; i < size; i++)
    {
        int_string_map[pairs[i].val] = pairs[i].str;
    }

    return int_string_map;
}


std::map<std::string, int> create_string_int_map(const string_int_struct pairs[],
  unsigned int size)
{
    std::map<std::string, int> string_int_map;
    for(unsigned int i = 0; i < size; i++)
    {
        string_int_map[pairs[i].str] = pairs[i].val;
    }

    return string_int_map;
}

