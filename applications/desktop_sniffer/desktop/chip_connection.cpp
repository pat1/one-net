#include "chip_connection.h"
#include "time_utils.h"
#include "string_utils.h"
#include <iostream>
#include <fcntl.h>
#include <string>
#include <fstream>
using namespace std;



chip_connection::chip_connection()
{
}


chip_connection::chip_connection(string device_name, speed_t baud_rate)
{
    int temp;
    this->device = device_name;
    this->baud_rate = baud_rate;
    chip_fd = open(device_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (chip_fd < 0)
    {
        return;
    }

    temp = tcgetattr(chip_fd, &old_sio);
    if (temp < 0)
    {
        chip_fd = -1;
        return;
    }

    new_sio = old_sio;

    temp = cfsetispeed(&new_sio, baud_rate);
    if (temp < 0)
    {
        chip_fd = -1;
    }

    temp = cfsetospeed(&new_sio, baud_rate);
    if (temp < 0)
    {
        chip_fd = -1;
        return;
    }

    cfmakeraw(&new_sio);
    new_sio.c_lflag |= ICANON;
    new_sio.c_cflag &= ~CRTSCTS;

    tcsetattr(chip_fd, TCSANOW, &new_sio);
    tcflush(chip_fd, TCIOFLUSH);
    tcgetattr(STDOUT_FILENO, &old_console);
    new_console = old_console; /* Will work with copy of attributes */

    cfmakeraw(&new_console);
    new_console.c_lflag |= ICANON;
    old_console.c_lflag |= ICANON;
    tcflush(chip_fd, TCIFLUSH);
}


chip_connection::chip_connection(const chip_connection& orig)
{
}


chip_connection::~chip_connection()
{
    if(chip_fd < 0)
    {
        return;
    }

    tcsetattr(STDOUT_FILENO, TCSANOW, &old_console);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_console);
    tcsetattr(chip_fd, TCSANOW, &old_sio);
    close(chip_fd);
    chip_fd = -1;
}


void chip_connection::set_console(bool chip)
{
    tcsetattr(STDOUT_FILENO, TCSANOW, chip ? &new_console: &old_console);
    tcsetattr(STDIN_FILENO, TCSANOW, chip ? &new_console: &old_console);
}


int chip_connection::get_chip_fd()
{
    return chip_fd;
}


bool chip_connection::send_bytes_to_chip(string bytes, bool add_newline)
{
    strip_leading_and_trailing_whitespace(bytes);
    tcflush(chip_fd, TCOFLUSH);

    if(add_newline)
    {
        bytes += "\n";
    }

    return ((write(chip_fd, &bytes[0], bytes.length())) == (int) bytes.length());
}


string chip_connection::read_bytes(struct timeval time_period)
{
    string chip_bytes;
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    struct timeval expire_time = add_timeval(time_now, time_period);
    char buffer[1000];
    int bytes_read = 0;

    do
    {
        bytes_read = read(chip_fd, buffer, sizeof(buffer) - 1);
        if(bytes_read > 0)
        {
            buffer[bytes_read] = 0;
            chip_bytes += string(buffer);
        }

        gettimeofday(&time_now, NULL);
    }
    while(bytes_read > 0 || timeval_compare(time_now, expire_time) < 0);
    return chip_bytes;
}
