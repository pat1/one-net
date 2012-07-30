#include "parse_utility_args.h"
#include "string_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <stdint.h>
#include "string_utils.h"
#include "one_net_crc.h"
#include "one_net_status_codes.h"
using namespace std;


void usage()
{
    cout << "Usage: ./calc_crc -i 0x33445566778899 ---> Computes 8-bit ONE-NET CRC over 0x33445566778899\n";
    cout << "Usage: ./calc_crc -if array.txt ---> Computes 8-bit ONE-NET CRC over an array stored in array.txt\n";
}


int main(int argc, char* argv[])
{
    string error;
    UInt8 array_bytes[255];
    UInt8 num_array_bytes = sizeof(array_bytes);

    if(!ParseArgsForInputArray(argc, argv, array_bytes, num_array_bytes,
      error))
    {
        cout << "Could not parse input array.  Error = " << error << endl;
        usage();
        exit(0);
    }

    if(num_array_bytes == 0)
    {
        cout << "Number of bytes in input array must be positive." << endl;
        usage();
        exit(0);
    }

    UInt8 crc = (UInt8) one_net_compute_crc(array_bytes, num_array_bytes,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);

    cout << "0x" << hex << (int) crc;
    return 0;
}
