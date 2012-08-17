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
    cout << "Usage: ./encode_array -i decoded_array ---> encodes decoded_array and outputs encoded array to screen\n";
    cout << "Usage: ./encode_array -if decoded_array.txt ---> Reads decoded array from file decoded_array.txt and\n"
            "       outputs decodedd array to screen\n";
    cout << "Usage: ./encode_array -i decoded_array -of encoded_array.txt ---> encodes decoded_array and outputs\n"
            "       encoded array to file encoded_array.txt\n";
    cout << "Usage: ./encode_array -if decoded_array.txt ---> Reads decoded array from file decoded_array.txt and\n"
            "       outputs decoded array to file encoded_array.txt\n";
    cout << "decoded array should be a array of hexadcimal digit values.  The array is assumed to be hexadecimal\n"
            "       regardless of whether it is preceded with '0x'.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./encode_array -i 0xB4B4B4BC ---> outputs 0x000001 to screen\n";
    cout << "Example: ./encode_array -i 0xB4B4B4BC -of encoded_array.txt--->\n"
            "       outputs 0x000001 to file encoded_array.txt\n";
    cout << "Example: ./encode_array -if decoded_array.txt ---> Reads 0xB4B4B4BC from decoded_array.txt, outputs\n"
            "       0x000001 to screen\n";
    cout << "Example: ./encode_array -if decoded_array.txt -of encoded_array.txt---> Reads 0xB4B4B4BC from\n"
            "       decoded_array.txt, outputs 0x000001 to file encoded_array.txt\n";
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error;
    UInt8 decoded_bytes[255];
    UInt8 encoded_bytes[255];

    UInt8 num_decoded_bytes = 0xBF;

    if(!ParseArgsForInputArray(argc, argv, decoded_bytes, num_decoded_bytes,
      error))
    {
        cout << "Could not parse input array.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(num_decoded_bytes == 0 || num_decoded_bytes % 3 != 0)
    {
        cout << "Number of bytes in input array must be a positive multiple of 3" << endl;
        usage();
        exit(0);
    }

    UInt8 num_encoded_bytes = num_decoded_bytes / 3 * 4;
    if(on_encode(encoded_bytes, decoded_bytes, num_encoded_bytes) != ONS_SUCCESS)
    {
        cout << "Could not encode input array.\n" << endl;
        usage();
        exit(0);
    }

    string decoded_str = bytes_to_hex_string(encoded_bytes, num_encoded_bytes);

    if(!ParseArgsForOutputStream(argc, argv, &outs, error))
    {
        cout << "Output Error.  Error = " << error << endl;
        usage();
        exit(0);
    }

    *outs << decoded_str;
    close_stream(outs);
    return 0;
}
