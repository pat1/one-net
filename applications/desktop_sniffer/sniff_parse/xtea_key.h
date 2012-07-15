#ifndef XTEA_KEY_T_H
#define	XTEA_KEY_T_H

#include "one_net_xtea.h"
#include "one_net_types.h"
#include "one_net_constants.h"
#include <vector>
using namespace std;


struct xtea_key
{
    UInt8 bytes[ONE_NET_XTEA_KEY_LEN];

    const UInt8& operator[](uint8_t indx) const
    {
        return bytes[indx];
    }


    UInt8& operator[](uint8_t indx)
    {
        return bytes[indx];
    }



    xtea_key();
    xtea_key(const UInt8* key_bytes);
    xtea_key(const xtea_key& orig);
    ~xtea_key();
    xtea_key& operator = (const xtea_key& that);
    bool equal(const xtea_key& key) const;
    static bool insert_key(vector<xtea_key>& keys, const xtea_key& key,
        bool deep_copy);
    static bool delete_key(vector<xtea_key>& keys, const xtea_key& key);
    static int find_key(const vector<xtea_key>& keys, const xtea_key& key);
    static vector<xtea_key> copy_keys(const vector<xtea_key> keys,
        bool deep_copy);
};



#endif	/* XTEA_KEY_T_H */
