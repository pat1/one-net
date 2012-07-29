#include "parse_utility_args.h"
#include "string_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <stdint.h>
#include "string_utils.h"
#include "one_net_xtea.h"
#include "one_net_status_codes.h"
using namespace std;


void usage()
{
    cout << "Usage: ./decrypt_array -k 0x33333333333333333333333333333333 -rounds 32 -i encrypted_array ---> decrypts\n"
            "       encrypted_array using key 0x33333333333333333333333333333333 using 32 rounds and outputs\n"
            "       plaintext array to screen\n";
    cout << "Usage: ./decrypt_array -if encrypted.txt -kf key.txt -rounds 32 -of plaintext.txt ---> decrypts\n"
            "       an array stored in plaintext.txt using the key stored in key.txt using 32 rounds and outputs\n"
            "       the plaintext array to file plaintext.txt\n";
}


int main(int argc, char* argv[])
{
    ostream* outs;
    string error;
    UInt8 plaintext_bytes[255];
    UInt8 encrypted_bytes[255];
    UInt8 rounds;
    one_net_xtea_key_t key;
    UInt8 num_encrypted_bytes = 0xFF;

    if(!ParseArgsForInputArray(argc, argv, encrypted_bytes, num_encrypted_bytes,
      error))
    {
        cout << "Could not parse input array.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(num_encrypted_bytes == 0 || num_encrypted_bytes % ONE_NET_XTEA_BLOCK_SIZE != 0)
    {
        cout << "Number of bytes in input array must be a positive multiple of " <<
          ONE_NET_XTEA_BLOCK_SIZE << endl;
        usage();
        exit(0);
    }

    UInt8 num_plaintext_bytes = num_encrypted_bytes;
    memcpy(plaintext_bytes, plaintext_bytes, num_plaintext_bytes);
    UInt8 num_blocks = num_plaintext_bytes / ONE_NET_XTEA_BLOCK_SIZE;

    if(!ParseArgsForRounds(argc, argv, rounds, error))
    {
        cout << "Could not input for number of rounds: Error = " << error << endl;
        usage();
        exit(0);
    }

    if(!ParseArgsForKey(argc, argv, key, error))
    {
        cout << "Could not parse input for XTEA Key: Error = " << error << endl;
        usage();
        exit(0);
    }

    for(UInt8 i = 0; i < num_blocks; i++)
    {
        one_net_xtea_decipher(rounds, &plaintext_bytes[i*ONE_NET_XTEA_BLOCK_SIZE],
          (const one_net_xtea_key_t* const) key);
    }

    string plaintext_str = bytes_to_hex_string(plaintext_bytes, num_plaintext_bytes);

    if(!ParseArgsForOutputStream(argc, argv, &outs, error))
    {
        cout << "Output Error.  Error = " << error << endl;
        usage();
        exit(0);
    }

    *outs << plaintext_str;
    close_stream(outs);
    return 0;
}
