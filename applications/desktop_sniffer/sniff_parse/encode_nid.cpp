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
    cout << "Usage: ./encode_nid -i decoded_nid ---> encodes decoded_nid and outputs encoded nid to screen\n";
    cout << "Usage: ./encode_nid -if decoded_nid.txt ---> Reads decoded nid from file decoded_nid.txt and\n"
            "       outputs enoded nid to screen\n";
    cout << "Usage: ./encode_nid -i decoded_nid -of encoded_nid.txt ---> encodes decoded_nid and outputs\n"
            "       encoded nid to file encoded_nid.txt\n";
    cout << "Usage: ./encode_nid -if decoded_nid.txt ---> Reads decoded nid from file decoded_nid.txt and\n"
            "       outputs enoded nid to file encoded_nid.txt\n";
    cout << "Decoded NID should be a value less than or equal to 0xFFFFFFFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./encode_nid -i 0x000000001 ---> outputs 0xB4B4B4B4B4BC to screen\n";
    cout << "Example: ./encode_nid -i 0x000000001 -of encoded_nid.txt--->\n"
            "       outputs 0xB4B4B4B4B4BC to file encoded_nid.txt\n";
    cout << "Example: ./encode_nid -if decoded_nid.txt ---> Reads 0x000000001 from decoded_nid.txt, outputs\n"
            "       0xB4B4B4B4B4BC to screen\n";
    cout << "Example: ./encode_nid -if decoded_nid.txt -of encoded_nid.txt---> Reads 0x000000001 from\n"
            "       decoded_nid.txt, outputs 0xB4B4B4B4B4BC to file encoded_nid.txt\n";
}


one_net_status_t on_encode_36_bit(uint64_t* encoded, uint64_t decoded)
{
    if(encoded == NULL || decoded > 0xFFFFFFFFFLL)
    {
        return ONS_BAD_PARAM;
    } 

    UInt16 encoded_16_bit, decoded_12_bit;
    *encoded = 0;
    one_net_status_t status;

    decoded_12_bit = decoded & 0xFFF;
    if((status = on_encode_uint16(&encoded_16_bit, decoded_12_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *encoded = encoded_16_bit;

    decoded >>= 12;
    decoded_12_bit = decoded & 0xFFF;
    if((status = on_encode_uint16(&encoded_16_bit, decoded_12_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *encoded += (((uint64_t) encoded_16_bit) << 16);

    decoded >>= 12;
    decoded_12_bit = decoded & 0xFFF;
    if((status = on_encode_uint16(&encoded_16_bit, decoded_12_bit)) !=
        ONS_SUCCESS)
    {
        return status;
    }
    *encoded += (((uint64_t) encoded_16_bit) << 32);

    return ONS_SUCCESS;
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t encoded, decoded;


    if(!ParseArgsForInputValue(argc, argv, decoded, error))
    {
        cout << "Could not parse input nid.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(decoded > 0xFFFFFFFFFLL || on_encode_36_bit(&encoded, decoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << decoded << " is not a valid decoded nid.\n";
        usage();
        exit(0);
    }

    encoded_nid_to_string(encoded, str);

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
