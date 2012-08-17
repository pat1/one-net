#ifndef STRING_UTILS_H
#define	STRING_UTILS_H


#include <string>
#include <vector>
#include "xtea_key.h"
#include <map>
extern "C"
{
    #include "one_net_types.h"
    #include "one_net_xtea.h"
}
using namespace std;


enum ADD_REMOVE_DISPLAY
{
    ADD,
    REMOVE,
    DISPLAY,
    ADD_REMOVE_ERROR
};


struct string_int_struct
{
    string str;
    int val;
};


int find_first_non_whitespace_index(string& str);
int find_last_non_whitespace_index(string& str);
int find_first_whitespace_index(string& str);
int find_last_whitespace_index(string& str);
void strip_leading_whitespace(string& str);
void strip_trailing_whitespace(string& str);
void strip_leading_and_trailing_whitespace(string& str);
void strip_all_whitespace(string& str);
bool ishyphen(char c);
void strip_all_hyphens(string& str);
void str_toupper(string& str);
void str_tolower(string& str);
void split_string(string str, string& first_str, string& remaining);
ADD_REMOVE_DISPLAY parse_add_remove_display(string& str);
bool parse_key_string(string& str, xtea_key& key);
bool parse_invite_key_string(string& str, xtea_key& invite_key);
bool display_invite_key(ostream& outs, const xtea_key& invite_key);
void display_key(ostream& outs, const xtea_key& key);
void display_keys(ostream& outs, bool invite_key, const vector<xtea_key>& keys);
void print_ascii(ostream& outs, const string& str);
bool legal_digits(const string& value_str, bool hex, bool whitespace_allowed);
bool string_to_uint8(string str, UInt8& value, bool hex);
bool string_to_uint64_t(string str, uint64_t& value, bool hex);
bool string_to_bool(const string& str, bool& value);
vector <uint64_t> string_to_uint64_t_vector(string str);
int kbhit(void);
bool nibble_to_hex_char(UInt8 nibble, char& hex_char);
bool hex_char_to_nibble(char hex_char, UInt8& nibble);
bool byte_to_hex_string(UInt8 byte, string& hex_string);
bool hex_string_to_byte(string hex_string, UInt8& byte);
bool bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes,
    string& hex_string, char separator, UInt8 num_spaces_between,
    UInt8 bytes_per_line);
bool hex_string_to_bytes(string hex_string, UInt8* bytes, UInt8& num_bytes);
bool uint16_to_hex_string(UInt16 value, string& str);
bool uint32_to_hex_string(UInt32 value, string& str);
bool uint64_to_hex_string(uint64_t value, string& str);
bool encoded_did_to_string(const on_encoded_did_t* enc_did, string& str);
bool raw_did_to_string(uint16_t raw_did, string& str);
bool encoded_nid_to_string(const on_encoded_nid_t* enc_nid, string& str);
bool raw_nid_to_string(uint64_t raw_nid, string& str);
void struct_timeval_to_string(struct timeval timestamp, string& str);

string value_to_bit_string(UInt32 value, unsigned int num_bits);


map<int, string> create_int_string_map(const string_int_struct[],
  unsigned int size);
map<string, int> create_string_int_map(const string_int_struct[],
  unsigned int size);



#endif	/* STRING_UTILS_H */
