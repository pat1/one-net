#ifndef FILTER_H
#define	FILTER_H


#include <stdint.h>
#include <list>
#include <ostream>
#include <vector>
#include "xtea_key.h"
using namespace std;


class filter_range
{
public:
    filter_range();
    filter_range(uint64_t value);
    filter_range(uint64_t low, uint64_t high);
    filter_range(const filter_range& orig);
    filter_range& operator = (const filter_range& that);
    ~filter_range();
    void display(ostream& outs, bool hex, bool key, unsigned int width) const;
    uint64_t getlow() const;
    uint64_t gethigh() const;
    void setlow(uint64_t value);
    void sethigh(uint64_t value);
    static bool inside(const filter_range& fr, uint64_t value);
    
private:
    static void display_value(ostream& outs, uint64_t value, bool hex, bool key,
        unsigned int width);
    void display_range(ostream& outs, bool hex, bool key,
        unsigned int width) const;


    uint64_t low;
    uint64_t high;
};


class filter_list
{
public:
    filter_list();
    filter_list(const filter_list& orig);
    filter_list& operator = (const filter_list& that);
    ~filter_list();
    bool value_accepted(uint64_t value) const;
    void accept_value(uint64_t value);
    bool accept_range(uint64_t low, uint64_t high);
    void reject_value(uint64_t value);
    bool reject_range(uint64_t low, uint64_t high);
    void display(ostream& outs, bool hex, bool key, unsigned int width) const;
    void remove_all();
private:
    bool find_value(uint64_t value, int& index) const;
    list<filter_range> accepted_values;
};



class filter
{
public:
    enum FILTER_MATCH
    {
        WILDCARD,
        MUST_MATCH,
        MUST_NOT_MATCH
    };

    enum FILTER_TYPE
    {
        FILTER_TIMESTAMP,
        FILTER_RPTR_DID,
        FILTER_DST_DID,
        FILTER_SRC_DID,
        FILTER_NID,
        FILTER_PID,
        FILTER_MSG_ID,
        FILTER_MSG_CRC,
        FILTER_PLD_CRC,
        FILTER_PLD_MSG_TYPE,
        FILTER_MSG_CLASS,
        FILTER_MSG_TYPE,
        FILTER_MSG_DATA,
        FILTER_NACK_RSN,
        FILTER_ACK_NACK_HANDLE,
        FILTER_ADMIN_TYPE,
        FILTER_ACK_NACK_TIME,
        FILTER_ACK_NACK_VALUE,
        FILTER_KEY_FRAGMENT,
        FILTER_HOPS,
        FILTER_MAX_HOPS,
        FILTER_MSG_CRC_MATCH,
        FILTER_PAYLOAD_CRC_MATCH,
        FILTER_VALID_MATCH,
        FILTER_VALID_DECODE_MATCH,
        FILTER_INVITE_KEYS,
        FILTER_KEYS,
        FILTER_INVALID
    };



    struct filter_display_attribute
    {
        bool hex;
        bool key;
        unsigned int width; // relevant only if hex is true.
    };

    static const int NUM_FILTER_TYPES;
    static const int NUM_MATCH_FILTERS;
    static const int NUM_KEY_FILTERS;
    static const int NUM_RANGE_FILTERS;
    static const string FILTER_TYPE_STR[];
    static const filter_display_attribute FILTER_DISPLAY_ATTRIBUTE[];
    static const string FILTER_MATCH_STR[];

    filter();
    filter(const filter& orig);
    filter& operator = (const filter& that);
    ~filter();
    static FILTER_TYPE string_to_filter_type(const string& ft_string);
    static string filter_type_to_string(FILTER_TYPE ft);
    bool value_accepted(FILTER_TYPE ft, uint64_t value) const;
    bool value_accepted(FILTER_TYPE ft, const xtea_key& key) const;
    bool match_value_accepted(FILTER_TYPE ft, bool value) const;
    bool accept_value(FILTER_TYPE ft, uint64_t value);
    bool accept_values(FILTER_TYPE ft, vector<uint64_t> values);
    bool accept_range(FILTER_TYPE ft, uint64_t low, uint64_t high);
    bool reject_value(FILTER_TYPE ft, uint64_t value);
    bool reject_range(FILTER_TYPE ft, uint64_t low, uint64_t high);
    bool reject_values(FILTER_TYPE ft, vector<uint64_t> values);
    void display(ostream& outs) const;
    void remove_filter();
    bool set_match_value(FILTER_TYPE ft, FILTER_MATCH fm);
    bool remove_filter(FILTER_TYPE ft);
    const vector<xtea_key>* get_keys(bool invite) const;


private:
    vector<filter_list> filters;
    vector<xtea_key> keys;
    vector<xtea_key> invite_keys;
    FILTER_MATCH msg_crc_match;
    FILTER_MATCH pld_crc_match;
    FILTER_MATCH valid_decode_match;
    FILTER_MATCH valid_match;
};

#endif	/* FILTER_H */

