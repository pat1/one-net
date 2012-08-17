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
    cout << "Usage: ./decode_did -i encoded_did ---> decodes encoded_did and outputs decoded did to screen\n";
    cout << "Usage: ./decode_did -if encoded_did.txt ---> Reads encoded did from file encoded_did.txt and\n"
            "       outputs enoded did to screen\n";
    cout << "Usage: ./decode_did -i encoded_did -of decoded_did.txt ---> decodes encoded_did and outputs\n"
            "       decoded did to file decoded_did.txt\n";
    cout << "Usage: ./decode_did -if encoded_did.txt ---> Reads encoded did from file encoded_did.txt and\n"
            "       outputs enoded did to file decoded_did.txt\n";
    cout << "encoded did should be a value less than or equal to 0xFFFFFFFFFFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./decode_did -i 0xB4B4B4B4B4BC ---> outputs 0x000000001 to screen\n";
    cout << "Example: ./decode_did -i 0xB4B4B4B4B4BC -of decoded_did.txt --->\n"
            "       outputs 0x000000001 to file decoded_did.txt\n";
    cout << "Example: ./decode_did -if encoded_did.txt ---> Reads 0xB4B4B4B4B4BC from encoded_did.txt, outputs\n"
            "       0x000000001 to screen\n";
    cout << "Example: ./decode_did -if encoded_did.txt -of decoded_did.txt---> Reads 0xB4B4B4B4B4BC from\n"
            "       encoded_did.txt, outputs 0x000000001 to file decoded_did.txt\n";
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t encoded_64;
    UInt16 decoded, encoded;


    if(!ParseArgsForInputValue(argc, argv, encoded_64, error))
    {
        cout << "Could not parse input did.  Error = " << error << endl;
        usage();
        exit(0);
    }
    encoded = (UInt16) encoded_64;

    if(encoded_64 > 0xFFFF || on_decode_uint16(&decoded, encoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << encoded << " is not a valid encoded did.\n";
        usage();
        exit(0);
    }

    raw_did_to_string(decoded, str);

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
