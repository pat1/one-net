#include "parse_utility_args.h"
#include "string_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
using namespace std;


void usage()
{
    cout << "Usage: ./dec_to_hex value_in_dec.\n";
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        usage();
        exit(0);
    }

    string value_str(argv[1]);
    uint64_t value;
    if(!string_to_uint64_t(value_str, value, false))
    {
        cout << "Could not convert input " << argv[1] << " to a 64-bit unsigned decimal value.\n";
        exit(0);
    }

    cout << "0x" << hex << value << endl;
    return 0;
}
