#include "parse_utility_args.h"
#include "string_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <iomanip>
using namespace std;


void usage()
{
    cout << "Usage: ./hex_to_dec value_in_hex.\n";
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        usage();
        exit(0);
    }

    string value_str(argv[1]);
    value_str = strip_leading0x(value_str);
    uint64_t value;
    if(!string_to_uint64_t(value_str, value, true))
    {
        cout << "Could not convert input " << argv[1] << " to a 64-bit unsigned hex value.\n";
        exit(0);
    }

    cout << dec << value << endl;
    return 0;
}
