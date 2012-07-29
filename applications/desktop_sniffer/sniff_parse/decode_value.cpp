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
    cout << "Usage: ./decode_value -i encoded_value ---> decodes encoded_value and outputs decoded value to screen\n";
    cout << "Usage: ./decode_value -if encoded_value.txt ---> Reads encoded value from file encoded_value.txt and\n"
            "       outputs enoded value to screen\n";
    cout << "Usage: ./decode_value -i encoded_value -of decoded_value.txt ---> decodes encoded_value and outputs\n"
            "       decoded value to file decoded_value.txt\n";
    cout << "Usage: ./decode_value -if encoded_value.txt ---> Reads encoded value from file encoded_value.txt and\n"
            "       outputs enoded value to file decoded_value.txt\n";
    cout << "encoded value should be a value less than or equal to 0xFFFFFFFFFFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./decode_value -i 0x1 ---> outputs 0xBC to screen\n";
    cout << "Example: ./decode_value -i 0x1 -of decoded_value.txt--->\n"
            "       outputs 0xBC to file decoded_value.txt\n";
    cout << "Example: ./decode_value -if encoded_value.txt ---> Reads 0x1 from encoded_value.txt, outputs\n"
            "       0xBC to screen\n";
    cout << "Example: ./decode_value -if encoded_value.txt -of decoded_value.txt---> Reads 0x1 from\n"
            "       encoded_value.txt, outputs 0xBC to file decoded_value.txt\n";
}


one_net_status_t on_decode_value(uint64_t* decoded, uint64_t encoded)
{
    if(decoded == NULL)
    {
        return ONS_BAD_PARAM;
    }

    if(encoded == 0)
    {
        *decoded = 0xB4;
        return ONS_SUCCESS;
    }

    *decoded = 0;
    UInt8 shift_factor = 0;
    while(encoded > 0)
    {
        UInt8 encoded_8 = encoded & 0xFF;
        UInt8 decoded_byte = encoded_to_decoded_byte(encoded_8, false);
        (*decoded) += (((int64_t) decoded_byte) << shift_factor);

        if(decoded_byte == 0xFF)
        {
            return ONS_BAD_PARAM;
        }

        shift_factor += 6;
        encoded >>= 8;
    }

    return ONS_SUCCESS;
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t decoded, encoded;


    if(!ParseArgsForInputValue(argc, argv, encoded, error))
    {
        cout << "Could not parse input value.  Error = " << error << endl;
        usage();
        exit(0);
    }

    uint64_t original_encoded = encoded;
    if(on_decode_value(&decoded, encoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << original_encoded << " is not a valid encoded value.\n";
        usage();
        exit(0);
    }

    str = uint64_to_hex_string(decoded);
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
