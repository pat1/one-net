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
    cout << "Usage: ./encode_did -i decoded_did ---> encodes decoded_did and outputs encoded did to screen\n";
    cout << "Usage: ./encode_did -if decoded_did.txt ---> Reads decoded did from file decoded_did.txt and\n"
            "       outputs enoded did to screen\n";
    cout << "Usage: ./encode_did -i decoded_did -of encoded_did.txt ---> encodes decoded_did and outputs\n"
            "       encoded did to file encoded_did.txt\n";
    cout << "Usage: ./encode_did -if decoded_did.txt ---> Reads decoded did from file decoded_did.txt and\n"
            "       outputs enoded did to file encoded_did.txt\n";
    cout << "Decoded DID should be a value less than or equal to 0xFFF.  If the value is represented\n"
            "       in hexadecimal, it should be proceeded with '0x'.  Otherwise it will be interpreted as decimal.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./encode_did -i 0x001 ---> outputs 0xB4BC to screen\n";
    cout << "Example: ./encode_did -i 0x001 -of encoded_did.txt--->\n"
            "       outputs 0xB4BC to file encoded_did.txt\n";
    cout << "Example: ./encode_did -if decoded_did.txt ---> Reads 0x001 from decoded_did.txt, outputs\n"
            "       0xB4BC to screen\n";
    cout << "Example: ./encode_did -if decoded_did.txt -of encoded_did.txt---> Reads 0x001 from\n"
            "       decoded_did.txt, outputs 0xB4BC to file encoded_did.txt\n";
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error, str;

    uint64_t decoded_64;
    UInt16 encoded, decoded;


    if(!ParseArgsForInputValue(argc, argv, decoded_64, error))
    {
        cout << "Could not parse input did.  Error = " << error << endl;
        usage();
        exit(0);
    }
    decoded = (UInt16)decoded_64;

    if(decoded_64 > 0xFFF || on_encode_uint16(&encoded, decoded) != ONS_SUCCESS)
    {
        cout << "0x" << hex << decoded << " is not a valid decoded did.\n";
        usage();
        exit(0);
    }

    encoded_did_to_string(encoded, str);

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
