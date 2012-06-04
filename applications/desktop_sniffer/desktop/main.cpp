#include "cli.h"
#include <cstdlib>
#include <iostream>
#include <termios.h>
using namespace std;


extern speed_t serial_device_baud;
extern string serial_device;


void usage()
{
    cout << "usage : ./desktop_sniffer device_name baud_rate\n";
    cout << "usage : ./desktop_sniffer device_name --> default baud rate is 115,200\n";
    cout << "usage : ./desktop_sniffer --> default device is /dev/ttyS0 --> default baud rate is 115,200\n";
    cout << "Valid data rates are the following : 38400, 115200, 230400\n";    
}


int main(int argc, char** argv)
{
    if(argc > 3)
    {
        usage();
        exit(1);
    }
    if(argc > 1)
    {
        serial_device = argv[1];
    }
    if(argc > 2)
    {
        int new_baud = atoi(argv[2]);
        switch(new_baud)
        {
            case 38400:  serial_device_baud = B38400; break;
            case 115200: serial_device_baud = B115200; break;
            case 230400: serial_device_baud = B230400; break;
            default: usage(); exit(0);
        }
    }
    while(cli())
    {
    }
    return 0;
}
