#ifndef CHIP_CONNECTION_H
#define	CHIP_CONNECTION_H


#include <string>
#include <fcntl.h>
#include <termios.h>
using namespace std;


class chip_connection
{
public:
    chip_connection();
    chip_connection(string device_name, speed_t baud_rate);
    chip_connection(const chip_connection& orig);
    virtual ~chip_connection();
    void set_console(bool chip);
    int get_chip_fd();
    bool send_bytes_to_chip(string bytes, bool add_newline);
    string read_bytes(struct timeval time_period);
private:
    string device;
    int chip_fd;
    speed_t baud_rate;
    struct termios old_sio;
    struct termios new_sio;
    struct termios old_console;
    struct termios new_console;
};


#endif	/* CHIP_CONNECTION_H */

