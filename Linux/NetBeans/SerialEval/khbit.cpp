/* demo of a couple of console functions for Linux
 * Note that these work for me with gcc 4.x on my
 * Centos 5.x installation.
 *
 * Your mileage may vary.
 *
 * davekw7x
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

int kbhit(void)
{
    struct termios oldstuff;
    struct termios newstuff;
    struct timeval tv = {0,1}; /* set one microsecond timeout */
    fd_set rdfds;
    int fd = STDIN_FILENO;
    int retval;

    FD_ZERO(&rdfds);
    FD_SET(fd, &rdfds);

    tcgetattr(fd, &oldstuff);
    newstuff = oldstuff; /* work on a copy of the attributes */

    /*
     * Resetting these flags will set the terminal to raw mode.
     * Note that ctrl-c won't cause program exit, so there
     * is no emergency panic escape. (If your calling program
     * doesn't handle things properly, you will have to kill
     * the process externally.)
     */
    /*newstuff.c_lflag &= ~(ICANON | ECHO | IGNBRK); */

    cfmakeraw(&newstuff);
    tcsetattr(fd, TCSANOW, &newstuff);   /* set new attributes           */

    if (select(fd + 1, &rdfds, NULL, NULL, &tv) == -1)
    {
        perror("select");
        exit(1);
    }
    if (FD_ISSET(fd, &rdfds))
    {
        retval = 1;
    }
    else
    {
        retval = 0;
    }

    tcsetattr(fd, TCSANOW, &oldstuff);   /* restore old attributes       */

    return retval;
}

int getch(void)
{
    struct termios oldstuff;
    struct termios newstuff;
    int fd = STDIN_FILENO;
    int inch;

    tcgetattr(fd, &oldstuff);
    newstuff = oldstuff; /* will work on copy of attributes */

    /*
     * Resetting these flags will set the terminal to raw mode.
     * Note that ctrl-c won't cause program exit, so there
     * is no emergency panic escape. (If your calling program
     * doesn't handle things properly, you will have to kill
     * the process externally.)
     */
    /*newstuff.c_lflag &= ~(ICANON | ECHO | IGNBRK);*/
    cfmakeraw(&newstuff);
    tcsetattr(fd, TCSANOW, &newstuff);   /* set new attributes           */

    inch = EOF;
    read(fd, &inch, 1);                  /* read a character in raw mode */

    tcsetattr(fd, TCSANOW, &oldstuff);   /* restore old attributes       */
    return inch;
}

