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
    cout << "Usage: ./decode_array -i encoded_array ---> decodes encoded_array and outputs decoded array to screen\n";
    cout << "Usage: ./decode_array -if encoded_array.txt ---> Reads encoded array from file encoded_array.txt and\n"
            "       outputs enoded array to screen\n";
    cout << "Usage: ./decode_array -i encoded_array -of decoded_array.txt ---> decodes encoded_array and outputs\n"
            "       decoded array to file decoded_array.txt\n";
    cout << "Usage: ./decode_array -if encoded_array.txt ---> Reads encoded array from file encoded_array.txt and\n"
            "       outputs enoded array to file decoded_array.txt\n";
    cout << "encoded array should be a array of hexadcimal digit values.  The array is assumed to be hexadecimal\n"
            "       regardless of whether it is preceded with '0x'.\n";
    cout << "Usage: f argument means 'file'.  -i --> input from command line.  -if --> input from file.\n"
            "       -of --> output to file.  If -o argument is missing, stdout will be used for output.\n";
    cout << "Example: ./decode_array -i 0xB4B4B4BC ---> outputs 0x000001 to screen\n";
    cout << "Example: ./decode_array -i 0xB4B4B4BC -of decoded_array.txt--->\n"
            "       outputs 0x000001 to file decoded_array.txt\n";
    cout << "Example: ./decode_array -if encoded_array.txt ---> Reads 0xB4B4B4BC from encoded_array.txt, outputs\n"
            "       0x000001 to screen\n";
    cout << "Example: ./decode_array -if encoded_array.txt -of decoded_array.txt---> Reads 0xB4B4B4BC from\n"
            "       encoded_array.txt, outputs 0x000001 to file decoded_array.txt\n";
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error;
    UInt8 encoded_bytes[255];
    UInt8 decoded_bytes[255];

    UInt8 num_encoded_bytes = sizeof(decoded_bytes);

    if(!ParseArgsForInputArray(argc, argv, encoded_bytes, num_encoded_bytes,
      error))
    {
        cout << "Could not parse input array.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(num_encoded_bytes == 0 || num_encoded_bytes % 4 != 0)
    {
        cout << "Number of bytes in input array must be a positive multiple of 4" << endl;
        usage();
        exit(0);
    }

    if(on_decode(decoded_bytes, encoded_bytes, num_encoded_bytes) != ONS_SUCCESS)
    {
        cout << "Could not decode input array.\n" << endl;
        usage();
        exit(0);
    }

    UInt8 num_decoded_bytes = num_encoded_bytes / 4 * 3;
    string decoded_str = bytes_to_hex_string(decoded_bytes, num_decoded_bytes);

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
