#include "attribute.h"
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;



const int attribute::NUM_ATTRIBUTES = 19;


const string attribute::ATTRIBUTE_STR[] =
{
    "timestamp",
    "encoded bytes",
    "decoded bytes",
    "encoded payload",
    "encrypted payload",
    "decrypted payload",
    "payload detail",
    "header",
    "rptr did",
    "src did",
    "dst did",
    "nid",
    "pid",
    "msg id",
    "msg crc",
    "payload crc",
    "hops",
    "key",
    "valid"
};


attribute::attribute()
{
    attribute_present.resize(NUM_ATTRIBUTES);
    this->set_attributes(true);
}


attribute::attribute(const attribute& orig)
{
    attribute_present.resize(NUM_ATTRIBUTES);
    this->attribute_present.assign(orig.attribute_present.begin(),
        orig.attribute_present.end());
}


attribute& attribute::operator = (const attribute& that)
{
    if(this == &that)
    {
        return *this; // self-assignment
    }

    this->attribute_present.assign(that.attribute_present.begin(),
        that.attribute_present.end());

    return *this;
}


attribute::~attribute()
{

}


void attribute::set_attributes(bool present)
{
    attribute_present.assign(NUM_ATTRIBUTES, present);
}


bool attribute::set_attribute(ATTRIBUTE att, bool present)
{
    if(att == ATTRIBUTE_INVALID)
    {
        return false;
    }
    attribute_present[att] = present;
    return true;
}


bool attribute::set_attribute(const string& att_string, bool present)
{
    ATTRIBUTE att = string_to_attribute(att_string);
    return set_attribute(att, present);
}


attribute::ATTRIBUTE attribute::string_to_attribute(const string& att_string)
{
    for(int i = 0; i < NUM_ATTRIBUTES; i++)
    {
        if(ATTRIBUTE_STR[i].compare(att_string) == 0)
        {
            return (ATTRIBUTE) i;
        }
    }
    return ATTRIBUTE_INVALID;
}


string attribute::attribute_to_string(ATTRIBUTE att)
{
    if(att == ATTRIBUTE_INVALID)
    {
        return "";
    }

    return ATTRIBUTE_STR[(int) att];
}


void attribute::display_attributes(ostream& outs) const
{
    for(int i = 0; i < NUM_ATTRIBUTES; i++)
    {
        outs << left << setw(25) << ATTRIBUTE_STR[i]  << (attribute_present[i] ?
            "present\n" : "not present\n");
    }
}


bool attribute::display(ATTRIBUTE att) const
{
    if(att == ATTRIBUTE_INVALID)
    {
        return false;
    }

    return this->attribute_present[att];
}


bool attribute::get_attribute(ATTRIBUTE att) const
{
    if(att == attribute::ATTRIBUTE_INVALID)
    {
        return false;
    }

    return this->attribute_present.at((int)att);
}
