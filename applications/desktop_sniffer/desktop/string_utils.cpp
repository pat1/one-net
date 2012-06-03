#include "string_utils.h"
#include <string>
#include <sstream>
#include <cctype>
#include <iostream>
#include <vector>
#include <termios.h>
#include <stdint.h>
#include <iomanip>
using namespace std;


extern "C"
{
    #include "one_net_port_specific.h"
    #include "one_net_types.h"
}




// return -1 if all whitespace
int find_first_non_whitespace_index(string& str)
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
int find_last_non_whitespace_index(string& str)
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
int find_first_whitespace_index(string& str)
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
int find_last_whitespace_index(string& str)
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


void strip_leading_whitespace(string& str)
{
    int index = find_first_non_whitespace_index(str);
    
    if(index == -1)
    {
        str = "";
        return;
    }
    
    str = str.substr(index);
}


void strip_trailing_whitespace(string& str)
{
    int index = find_last_non_whitespace_index(str);
    
    if(index == -1)
    {
        str = "";
        return;
    }
    
    str = str.substr(0, index + 1);
}


void strip_leading_and_trailing_whitespace(string& str)
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


void strip_all_whitespace(string& str)
{
    str.erase(remove_if(str.begin(), str.end(), iswhitespace), str.end());
}


void strip_all_hyphens(string& str)
{
    str.erase(remove_if(str.begin(), str.end(), ishyphen), str.end());
}


void str_toupper(string& str)
{
    int stringlen = str.length();
    for(int i = 0; i < stringlen; i++)
    {
        str[i] = toupper(str[i]);
    }
}


void str_tolower(string& str)
{
    int stringlen = str.length();
    for(int i = 0; i < stringlen; i++)
    {
        str[i] = tolower(str[i]);
    }
}


void split_string(string str, string& first_str, string& remaining)
{
    first_str = str;
    remaining = "";

    strip_leading_and_trailing_whitespace(str);

    int index = find_first_whitespace_index(str);
    if(index <= 0)
    {
        return;
    }

    first_str = str.substr(0, index);
    remaining = str.substr(index);
    strip_leading_whitespace(remaining);
}


ADD_REMOVE_DISPLAY parse_add_remove_display(string& str)
{
    strip_leading_and_trailing_whitespace(str);
    str_tolower(str);
    if(str.compare("add") == 0)
    {
        return ADD;
    }
    else if(str.compare("remove") == 0)
    {
        return REMOVE;
    }
    else if(str.compare("display") == 0)
    {
        return DISPLAY;
    }

    return ADD_REMOVE_ERROR;
}


bool ascii_char_to_nibble(char c, UInt8& nibble)
{
    c = toupper(c);
    if(!isxdigit(c))
    {
        return false;
    }
    
    if(isdigit(c))
    {
        nibble = c - '0';
    }
    else
    {
        nibble = c - 'A' + 10;
    }
    return true;
}


bool ascii_hex_to_UInt8(const string& str, UInt8& byte)
{
    UInt8 high_nibble, low_nibble;

    if(str.length() != 2)
    {
        return false;
    }
    if(!ascii_char_to_nibble(str[0], high_nibble))
    {
        return false;
    }
    if(!ascii_char_to_nibble(str[1], low_nibble))
    {
        return false;
    }

    byte = (high_nibble << 4) + low_nibble;
    return true;
}


bool nibble_to_ascii(UInt8 nibble, char& c)
{
    if(nibble < 10)
    {
        c = '0' + nibble;
    }
    else if(nibble < 16)
    {
        c = 'A' + nibble - 10;
    }
    else
    {
        return false;
    }

    return false;
}


string UInt8_to_ascii(UInt8 byte)
{
    UInt8 high_nibble = (byte >> 4);
    UInt8 low_nibble = byte & 0x0F;
    string str_rep = "00";
    nibble_to_ascii(high_nibble, str_rep[0]);
    nibble_to_ascii(low_nibble, str_rep[1]);
    return str_rep;
}


string UInt8_array_to_ascii(const UInt8* array, unsigned int len)
{
    string str = "";
    if(!array)
    {
        return str;
    }

    for(unsigned int i = 0; i < len; i++)
    {
        str += UInt8_to_ascii(array[i]);
    }
    return str;
}


string xtea_key_to_ascii_hex(const xtea_key& key)
{
    string key_str = "";
    for(int i = 0; i < ONE_NET_XTEA_KEY_LEN; i++)
    {
        if(i != 0)
        {
            key_str += "-";
        }

        key_str += UInt8_to_ascii(key[i]);
    }

    return key_str;
}


static bool ascii_hex_to_xtea_key(string& str, xtea_key& key)
{
    if(str.length() != 2 * ONE_NET_XTEA_KEY_LEN)
    {
        return false;
    }

    for(int i =0; i < ONE_NET_XTEA_KEY_LEN; i++)
    {
        string byte_str = str.substr(i * 2, 2);

        if(!ascii_hex_to_UInt8(byte_str, key[i]))
        {
            return false;
        }
    }

    return true;
}


bool parse_key_string(string& str, xtea_key& key)
{
    strip_all_whitespace(str);
    int stringlen = str.length();
    if(stringlen != 3 * ONE_NET_XTEA_KEY_LEN - 1)
    {
        return false;
    }

    strip_all_hyphens(str);
    return ascii_hex_to_xtea_key(str, key);
}


bool parse_invite_key_string(string& str, xtea_key& invite_key)
{
    strip_all_whitespace(str);
    int stringlen = str.length();
    if(stringlen != 2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE + (2 - 1))
    {
        return false;
    }

    strip_all_hyphens(str);
    stringlen = str.length();
    if(stringlen != 2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE)
    {
        return false;
    }

    memmove(invite_key.bytes, &str[0], 2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
    memmove(&invite_key[2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE], &str[0],
        2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE);

    return true;
}


bool display_invite_key(ostream& outs, const xtea_key& invite_key)
{
    for(int i = 0; i < 2 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE; i++)
    {
        if(!isprint(invite_key[i]))
        {
            return false; // not printable
        }
    }

    for(int i = 0; i < ONE_NET_XTEA_KEY_FRAGMENT_SIZE; i++)
    {
        outs << (char) invite_key[i];
    }
    outs << "-";
    for(int i = 0; i < ONE_NET_XTEA_KEY_FRAGMENT_SIZE; i++)
    {
        outs << (char) invite_key[i+ONE_NET_XTEA_KEY_FRAGMENT_SIZE];
    }
    return true;
}


void display_key(ostream& outs, const xtea_key& key)
{
    outs << xtea_key_to_ascii_hex(key);
}


void display_keys(ostream& outs, bool invite_key, const vector<xtea_key>& keys)
{
    int num_keys = keys.size();

    if(invite_key)
    {
        outs << "\n# of invite keys : " << num_keys << "\n";
    }
    else
    {
        outs << "\n# of keys : " << num_keys << "\n";
    }

    for(int i = 0; i < num_keys; i++)
    {
        outs << "Key " << i + 1 << " : ";
        xtea_key key = keys[i];
        if(invite_key)
        {
            display_invite_key(outs, key);
        }
        else
        {
            display_key(outs, key);
        }
        outs << "\n";
    }
}


void print_ascii(ostream& outs, const string& str)
{
    int num_bytes = str.length();
    outs << "num bytes : " << num_bytes << " : ";
    for(int i = 0; i < num_bytes; i++)
    {
        outs << (int) str[i] << " ";
    }
    outs << endl;
}


int kbhit(void)
{
    struct termios oldstuff;
    struct termios newstuff;
    struct timeval tv = {0,1}; /* set one microsecond timeout */
    fd_set rdfds;
    int fd = STDIN_FILENO;
    int retval;

    FD_ZERO(&rdfds);
    FD_SET(fd, &rdfds);

    tcgetattr(fd, &oldstuff);
    newstuff = oldstuff; /* work on a copy of the attributes */

    /*
     * Resetting these flags will set the terminal to raw mode.
     * Note that ctrl-c won't cause program exit, so there
     * is no emergency panic escape. (If your calling program
     * doesn't handle things properly, you will have to kill
     * the process externally.)
     */
    /*newstuff.c_lflag &= ~(ICANON | ECHO | IGNBRK); */

    cfmakeraw(&newstuff);
    tcsetattr(fd, TCSANOW, &newstuff);   /* set new attributes           */

    if (select(fd + 1, &rdfds, NULL, NULL, &tv) == -1) {
        perror("select");
        exit(1);
    }
    if (FD_ISSET(fd, &rdfds)) {
        retval = 1;
    }
    else {
        retval = 0;
    }

    tcsetattr(fd, TCSANOW, &oldstuff);   /* restore old attributes       */

    return retval;
}


bool legal_digits(const string& value_str, bool hex, bool whitespace_allowed)
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


bool string_to_uint8(string str, UInt8& value, bool hex)
{
    if(!legal_digits(str, hex, false))
    {
        return false;
    }

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


bool string_to_uint64_t(string str, uint64_t& value, bool hex)
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


bool string_to_bool(const string& str, bool& value)
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


vector <uint64_t> string_to_uint64_t_vector(string str)
{
    vector<uint64_t> values;
    string value_str;

    uint64_t value;
    stringstream ss(str);

    while(ss >> value_str)
    {
        bool hex = false;
        if(value_str.length() > 2 && value_str.find_first_of("0x", 0) == 0)
        {
            hex = true;
            value_str = value_str.substr(2);
        }

        if(!string_to_uint64_t(value_str, value, hex))
        {
            vector<uint64_t> empty_values;
            return empty_values;
        }

        values.push_back(value);
    }

    return values;
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

bool byte_to_hex_string(UInt8 byte, string& hex_string)
{
    hex_string = "00";
    nibble_to_hex_char((byte >> 4), hex_string[0]);
    nibble_to_hex_char((byte &0x0F), hex_string[1]);
    return true;
}


bool hex_string_to_byte(string hex_string, UInt8& byte)
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


bool bytes_to_hex_string(const UInt8* bytes, UInt8 num_bytes,
    string& hex_string, char separator, UInt8 num_spaces_between,
    UInt8 bytes_per_line)
{
    hex_string = "";
    string separator_string = "";
    for(int i = 0; i < num_spaces_between; i++)
    {
        separator_string += separator;
    }
    string hex_byte_string;
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

        if(!byte_to_hex_string(bytes[i], hex_byte_string))
        {
            return false;
        }
        hex_string += hex_byte_string;
    }

    return true;
}


bool uint16_to_hex_string(UInt16 value, string& str)
{
    UInt8 bytes[2];
    one_net_int16_to_byte_stream(value, bytes);
    return bytes_to_hex_string(bytes, 2, str, ' ', 0, 0);
}


bool uint32_to_hex_string(UInt32 value, string& str)
{
    UInt8 bytes[4];
    one_net_int32_to_byte_stream(value, bytes);
    return bytes_to_hex_string(bytes, 4, str, ' ', 0, 0);
}


bool uint64_to_hex_string(uint64_t value, string& str)
{
    string msb_string, lsb_string;
    UInt32 msb = (value >> 32);
    UInt32 lsb = (value & 0xFFFFFFFF);
    uint32_to_hex_string(msb, msb_string);
    uint32_to_hex_string(lsb, lsb_string);
    str = msb_string + lsb_string;
    return true;
}


bool hex_string_to_bytes(string hex_string, UInt8* bytes, UInt8& num_bytes)
{
    int string_len = hex_string.length();
    if(string_len % 2 != 0 || string_len > 2 * num_bytes)
    {
        return false;
    }

    int byte_stream_len = string_len / 2;
    for(int i = 0; i < byte_stream_len; i += 2)
    {
        if(!hex_string_to_byte(hex_string.substr(i * 2, 2), bytes[i]))
        {
            return false;
        }
    }

    return true;
}


bool encoded_did_to_string(const on_encoded_did_t* enc_did, string& str)
{
    bytes_to_hex_string((const UInt8*) enc_did, 2, str, ' ', 0, 0);
    str = "0x" + str;
    return true;
}


bool raw_did_to_string(uint16_t raw_did, string& str)
{
    if(raw_did > 0xFFF)
    {
        return false;
    }
    char msb_nibble;
    nibble_to_hex_char((raw_did >> 8) & 0x0F, msb_nibble);
    UInt8 least_sig = (UInt8) (raw_did & 0xFF);
    string least_sig_string;
    byte_to_hex_string(least_sig, least_sig_string);
    str = "0x";
    str += msb_nibble;
    str += least_sig_string;
    return true;
}


bool encoded_nid_to_string(const on_encoded_nid_t* enc_nid, string& str)
{
    bytes_to_hex_string((const UInt8*) enc_nid, 6, str, ' ', 0, 0);
    str = "0x" + str;
    return true;
}


bool raw_nid_to_string(uint64_t raw_nid, string& str)
{
    UInt32 most_sig = (raw_nid >> 32);
    if(most_sig > 0xF)
    {
        return false;
    }
    char msb_nibble;
    nibble_to_hex_char(((UInt8) most_sig) & 0x0F, msb_nibble);
    UInt32 least_sig = (UInt32) (raw_nid & 0xFFFFFFFF);
    string least_sig_string;
    uint32_to_hex_string(least_sig, least_sig_string);
    str = "0x";
    str += msb_nibble;
    str += least_sig_string;
    return true;
}


void struct_timeval_to_string(struct timeval timestamp, string& str)
{
    stringstream ss;
    ss << timestamp.tv_sec;
    ss << " ";
    ss << setfill('0') << setw(6) << timestamp.tv_usec;
    string sec_str, usec_str;
    ss >> sec_str >> usec_str;
    str = sec_str + "." + usec_str;
}




map<int, string> create_int_string_map(const string_int_struct pairs[],
  unsigned int size)
{
    map<int, string> int_string_map;
    for(unsigned int i = 0; i < size; i++)
    {
        int_string_map[pairs[i].val] = pairs[i].str;
    }

    return int_string_map;
}


map<string, int> create_string_int_map(const string_int_struct pairs[],
  unsigned int size)
{
    map<string, int> string_int_map;
    for(unsigned int i = 0; i < size; i++)
    {
        string_int_map[pairs[i].str] = pairs[i].val;
    }

    return string_int_map;
}
