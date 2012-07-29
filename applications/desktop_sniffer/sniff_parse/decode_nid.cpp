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
    cout << "Usage: ./decode_nid -i encoded_nid ---> decodes encoded_nid and outputs decoded nid to screen\n";
    cout << "Usage: ./decode_nid -if encoded_nid.txt ---> Reads encoded nid from file encoded_nid.txt and\n"
            "       outputs enoded nid to screen\n";
    cout << "Usage: ./decode_nid -i encoded_nid -of decoded_nid.txt ---> decodes encoded_nid and outputs\n"
            "       decoded nid to file decoded_nid.txt\n";
    cout << "Usage: ./decode_nid -if encoded_nid.txt ---> Reads encoded nid from file encoded_nid.txt and\n"
            "       outputs enoded nid to file decoded_nid.txt\n";
    cout << "encoded NID should be a value less than or equal to 0xFFFFFFFFFFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./decode_nid -i 0xB4B4B4B4B4BC ---> outputs 0x000000001 to screen\n";
    cout << "Example: ./decode_nid -i 0xB4B4B4B4B4BC -of decoded_nid.txt --->\n"
            "       outputs 0x000000001 to file decoded_nid.txt\n";
    cout << "Example: ./decode_nid -if encoded_nid.txt ---> Reads 0xB4B4B4B4B4BC from encoded_nid.txt, outputs\n"
            "       0x000000001 to screen\n";
    cout << "Example: ./decode_nid -if encoded_nid.txt -of decoded_nid.txt---> Reads 0xB4B4B4B4B4BC from\n"
            "       encoded_nid.txt, outputs 0x000000001 to file decoded_nid.txt\n";
}


one_net_status_t on_decode_48_bit(uint64_t* decoded, uint64_t encoded)
{
    if(decoded == NULL || encoded > 0xFFFFFFFFFFFFLL)
    {
        return ONS_BAD_PARAM;
    } 

    UInt16 decoded_12_bit, encoded_16_bit;
    *decoded = 0;
    one_net_status_t status;

    encoded_16_bit = encoded & 0xFFFF;
    if((status = on_decode_uint16(&decoded_12_bit, encoded_16_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *decoded = decoded_12_bit;

    encoded >>= 16;
    encoded_16_bit = encoded & 0xFFFF;
    if((status = on_decode_uint16(&decoded_12_bit, encoded_16_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *decoded += (((uint64_t) decoded_12_bit) << 12);

    encoded >>= 16;
    encoded_16_bit = encoded & 0xFFFF;
    if((status = on_decode_uint16(&decoded_12_bit, encoded_16_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *decoded += (((uint64_t) decoded_12_bit) << 24);

    return ONS_SUCCESS;
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t decoded, encoded;


    if(!ParseArgsForInputValue(argc, argv, encoded, error))
    {
        cout << "Could not parse input nid.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(encoded > 0xFFFFFFFFFFFFLL || on_decode_48_bit(&decoded, encoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << encoded << " is not a valid encoded nid.\n";
        usage();
        exit(0);
    }

    raw_nid_to_string(decoded, str);

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
