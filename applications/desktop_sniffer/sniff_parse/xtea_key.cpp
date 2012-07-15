#include "xtea_key.h"
#include "one_net_xtea.h"
#include "one_net_types.h"
#include "one_net_constants.h"
#include <vector>
#include <cstring>
using namespace std;


xtea_key::xtea_key()
{

}


xtea_key::xtea_key(const UInt8* key_bytes)
{
    memcpy(bytes, key_bytes, sizeof(bytes));
}


xtea_key::xtea_key(const xtea_key& orig)
{
    memcpy(bytes, orig.bytes, sizeof(bytes));
}


xtea_key::~xtea_key()
{

}


xtea_key& xtea_key::operator = (const xtea_key& that)
{
    if(this == &that)
    {
        return *this;
    }
    memcpy(bytes, that.bytes, sizeof(bytes));
    return *this;
}



bool xtea_key::equal(const xtea_key& key) const
{
    return (memcmp(bytes, key.bytes, ONE_NET_XTEA_KEY_LEN) == 0);
}


int xtea_key::find_key(const vector<xtea_key>& keys, const xtea_key& key)
{
    int size = keys.size();
    for(int i = 0; i < size; i++)
    {
        if(key.equal(keys.at(i)))
        {
            return i;
        }
    }

    return -1;
}


// true if a key was added, false otherwise
bool xtea_key::insert_key(vector<xtea_key>& keys, const xtea_key& key,
    bool deep_copy)
{
    if(find_key(keys, key) == -1)
    {
        if(!deep_copy)
        {
            keys.push_back(key);
        }
        else
        {
            keys.push_back(xtea_key(key));
        }
        return true;
    }
    return false;
}


// true if a key was deleted, false otherwise
bool xtea_key::delete_key(vector<xtea_key>& keys, const xtea_key& key)
{
    int index = find_key(keys, key);
    if(index == -1)
    {
        return false;
    }

    keys.erase (keys.begin() + index);
    return true;
}


vector<xtea_key> xtea_key::copy_keys(const vector<xtea_key> keys, bool
    deep_copy)
{
    vector<xtea_key> new_key_vector;
    int num_keys = keys.size();
    for(int i = 0; i < num_keys; i++)
    {
        insert_key(new_key_vector, keys[i], deep_copy);
    }
    return new_key_vector;
}
