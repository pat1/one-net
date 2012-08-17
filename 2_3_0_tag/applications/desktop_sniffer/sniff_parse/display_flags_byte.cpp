#include <iostream>
#include <cstdlib>
#include "string_utils.h"
#include "on_display.h"
using namespace std;



void usage()
{
    cout << "./display_flags_byte 0xXX (where XX is two hexadecimal digits)\n";
    exit(1);
}


int main(int argc, char* argv[])
{
    if(argc != 2 || (strlen(argv[1]) != 4))
    {
        usage();
    }

    string input_string = strip_leading0x(string(argv[1]));
    UInt8 flags;
    if(!string_to_uint8(input_string, flags, true))
    {
        usage();
    }

    display_flags_byte(flags, cout);
    cout << endl;
    return 0;
}
