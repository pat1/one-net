#include "parse_utility_args.h"
#include "string_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <stdint.h>
#include "string_utils.h"
#include "one_net_encode.h"
#include "one_net_status_codes.h"
using namespace std;


void usage()
{
    cout << "Usage: ./encode_value -i decoded_value ---> encodes decoded_value and outputs encoded value to screen\n";
    cout << "Usage: ./encode_value -if decoded_value.txt ---> Reads decoded value from file decoded_value.txt and\n"
            "       outputs enoded value to screen\n";
    cout << "Usage: ./encode_value -i decoded_value -of encoded_value.txt ---> encodes decoded_value and outputs\n"
            "       encoded value to file encoded_value.txt\n";
    cout << "Usage: ./encode_value -if decoded_value.txt ---> Reads decoded value from file decoded_value.txt and\n"
            "       outputs enoded value to file encoded_value.txt\n";
    cout << "Decoded value should be a value less than or equal to 0xFFFFFFFFFFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./encode_value -i 0x1 ---> outputs 0xBC to screen\n";
    cout << "Example: ./encode_value -i 0x1 -of encoded_value.txt--->\n"
            "       outputs 0xBC to file encoded_value.txt\n";
    cout << "Example: ./encode_value -if decoded_value.txt ---> Reads 0x1 from decoded_value.txt, outputs\n"
            "       0xBC to screen\n";
    cout << "Example: ./encode_value -if decoded_value.txt -of encoded_value.txt---> Reads 0x1 from\n"
            "       decoded_value.txt, outputs 0xBC to file encoded_value.txt\n";
}


one_net_status_t on_encode_value(uint64_t* encoded, uint64_t decoded)
{
    if(encoded == NULL || decoded > 0xFFFFFFFFFFFFLL)
    {
        return ONS_BAD_PARAM;
    }

    if(decoded == 0)
    {
        *encoded = 0xB4;
        return ONS_SUCCESS;
    }

    *encoded = 0;
    UInt8 shift_factor = 0;
    while(decoded > 0)
    {
        UInt8 decoded_8 = decoded & 0x3F;
        UInt8 encoded_byte = decoded_to_encoded_byte(decoded_8, false);
        (*encoded) += (((int64_t) encoded_byte) << shift_factor);
        shift_factor += 8;
        decoded >>= 6;
    }

    return ONS_SUCCESS;
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t encoded, decoded;


    if(!ParseArgsForInputValue(argc, argv, decoded, error))
    {
        cout << "Could not parse input value.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(decoded > 0xFFFFFFFFFFFFLL || on_encode_value(&encoded, decoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << decoded << " is not a valid decoded value.\n";
        usage();
        exit(0);
    }

    str = uint64_to_hex_string(encoded);
    str = strip_leading0x(str);
    strip_leading_zeroes(str);
    str = "0x" + str;
    
    if(!ParseArgsForOutputStream(argc, argv, &outs, error))
    {
        cout << "Output Error.  Error = " << error << endl;
        usage();
        exit(0);
    }

    *outs << str;
    close_stream(outs);
    return 0;
}
