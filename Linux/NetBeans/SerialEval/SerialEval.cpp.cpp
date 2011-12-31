#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <termios.h>
#include <iostream>
#include "khbit.h"
using namespace std;


int open_port(char *devname);



int main(int argc, char** argv)
{
    int fd;
    const int BAUDRATE = B38400;
    char *device = "/dev/ttyS0";
    int i, n;
    struct termios old_sio;
    struct termios new_sio;
    struct termios old_console;
    struct termios new_console;

    unsigned char sio_buffer[BUFSIZ];

    fd = open_port(device);
    fprintf(stderr, "open_port(%s) returned %d\n", device, fd);
    if (fd == -1) {
        exit(EXIT_FAILURE);
    }
    n = tcgetattr(fd, &old_sio);
    if (n < 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    new_sio = old_sio;

    n = cfsetispeed(&new_sio, BAUDRATE);
    if (n < 0) {
        perror("cfsetispeed");
        exit(EXIT_FAILURE);
    }

    n = cfsetospeed(&new_sio, BAUDRATE);
    if (n < 0) {
        perror("cfsetospeed");
        exit(EXIT_FAILURE);
    }

    /* Make sure the serial port doesn't diddle with raw data */
    cfmakeraw(&new_sio);
    tcsetattr(fd, TCSANOW, &new_sio);
    tcflush(fd, TCIOFLUSH);

#if 0
    /* Test: First send all 8-bit chars over serial channel */
    for (i = 0; i <= 0xff; i++) {
        unsigned char ch;
        ch = i;
        n = write(fd, &ch, 1);
    }
#endif

    fprintf(stderr, "Entering loop.  Exit with ctrl-c or ctrl-d\n");

    tcgetattr(STDOUT_FILENO, &old_console);
    new_console = old_console; /* Will work with copy of attributes */

    /* set uncooked mode and make sure nothing gets changed */
    cfmakeraw(&new_console);
    tcsetattr(STDOUT_FILENO, TCSANOW, &new_console);

    while (1)
    {
        unsigned char inchar;
        /*
         * Get chars from console and sent to serial port.
         */
        if (kbhit())
        {
            inchar = getch();
            /*putchar(inchar);fflush(stdout); */ /* debug: make it echo */
            if (inchar == 0x03)
            {
                fprintf(stderr, "\r\nExiting because of ctrl-c\r\n");
                break;
            }
            if (inchar == 0x04)
            {
                fprintf(stderr, "\r\nExiting because of ctrl-d\r\n");
                break;
            }
            n = write(fd, &inchar, 1);
            if (n < 1)
            {
                perror("write");
                break;
            }
        }

        /*
         *  See if any chars on the serial port.
         *  Handle cr/lf normally, otherwise show
         *  hex values of non-printing chars.
         */
        n = read(fd, sio_buffer, sizeof(sio_buffer)-1);
        if (n > 0)
        {
            sio_buffer[n] = 0;
            for (i = 0; i < n; i++)
            {
                if (isprint(sio_buffer[i]) ||
                           (sio_buffer[i] == '\r') ||
                           (sio_buffer[i] == '\n'))
                {
                    printf("%c", sio_buffer[i]);
                    fflush(stdout);
                }
                else
                {
                    printf("<%02x> ", sio_buffer[i]);
                    fflush(stdout);
                }
            }
        }
    }
    tcsetattr(fd, TCSANOW, &old_sio);
    tcsetattr(STDOUT_FILENO, TCSANOW, &old_console);
    close(fd);
    return 0;
}

/*
 * open_port(char *devname) - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int open_port(char *devname)
{
    int fd; /* File descriptor for the port */
    fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port");
    }
    return (fd);
}
