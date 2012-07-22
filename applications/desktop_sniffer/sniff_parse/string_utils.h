#ifndef STRING_UTILS_H
#define	STRING_UTILS_H


#include <string>
#include <map>
#include "one_net_types.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "xtea_key.h"

#ifdef WIN32
#include "Winsock2.h" // for struct timeval
#else
#include "sys/time.h" // for struct timeval
#endif


struct string_int_struct
{
    std::string str;
    int val;
};


int find_first_non_whitespace_index(std::string& str);
int find_last_non_whitespace_index(std::string& str);
int find_first_whitespace_index(std::string& str);
int find_last_whitespace_index(std::string& str);
void strip_leading_whitespace(std::string& str);
void strip_trailing_whitespace(std::string& str);
void strip_leading_and_trailing_whitespace(std::string& str);
void strip_all_whitespace(std::string& str);
bool ishyphen(char c);
void strip_all_hyphens(std::string& str);
void str_toupper(std::string& str);
void str_tolower(std::string& str);
std::string capitalize(std::string str);
bool legal_digits(const std::string& value_str, bool hex, bool whitespace_allowed);
bool string_to_uint8(std::string str, UInt8& value, bool hex);
bool string_to_uint16(std::string str, UInt16& value, bool hex);
bool string_to_uint32(std::string str, UInt32& value, bool hex);
bool string_to_uint64_t(std::string str, uint64_t& value, bool hex);
bool string_to_int8(std::string str, SInt8& value, bool hex);
bool string_to_int16(std::string str, SInt16& value, bool hex);
bool string_to_int32(std::string str, SInt32& value, bool hex);
bool string_to_int64_t(std::string str, int64_t& value, bool hex);
bool string_to_bool(const std::string& str, bool& value);
bool nibble_to_hex_char(UInt8 nibble, char& hex_char);
bool hex_char_to_nibble(char hex_char, UInt8& nibble);
std::string byte_to_hex_string(UInt8 byte);
bool hex_string_to_byte(std::string hex_string, UInt8& byte);
std::string bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes);
std::string bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes,
    char separator, UInt8 num_spaces_between, UInt8 bytes_per_line);
bool hex_string_to_bytes(std::string hex_string, UInt8* bytes, UInt8& num_bytes);
std::string uint16_to_hex_string(UInt16 value);
std::string uint32_to_hex_string(UInt32 value);
std::string uint64_to_hex_string(uint64_t value);
bool encoded_did_to_string(const on_encoded_did_t* enc_did, std::string& str);
bool encoded_did_to_string(UInt16 enc_did, std::string& str);
bool raw_did_to_string(uint16_t raw_did, std::string& str);
bool encoded_nid_to_string(const on_encoded_nid_t* enc_nid, std::string& str);
bool encoded_nid_to_string(uint64_t enc_nid, std::string& str);
bool raw_nid_to_string(uint64_t raw_nid, std::string& str);
void struct_timeval_to_string(struct timeval timestamp, std::string& str);
std::string value_to_bit_string(UInt32 value, unsigned int num_bits);
std::string bytes_to_bit_string(const UInt8* bytes, unsigned int num_bytes);
bool string_to_xtea_key(std::string key_string, one_net_xtea_key_t& key);
void xtea_key_to_string(std::string& key_string, one_net_xtea_key_t& key);
bool format_invite_key_fragment(std::string& string_rep,
  const one_net_xtea_key_fragment_t& key_frag);
bool format_invite_key_string(std::string& string_rep,
  const one_net_xtea_key_t& key);
std::string format_key_string(const one_net_xtea_key_t& key);
void display_keys(ostream& outs, bool invite_key, const vector<xtea_key>& keys);
// Note: This function assumes that the encoded has been checked for validity
std::string detailed_did_display(UInt16 encoded_did, UInt16 raw_did);
// Note: This function assumes that the encoded has been checked for validity
std::string detailed_did_display(UInt16 did, bool is_encoded);


std::map<int, std::string> create_int_string_map(const string_int_struct pairs[],
  unsigned int size);
std::map<std::string, int> create_string_int_map(const string_int_struct pairs[],
  unsigned int size);




#endif
