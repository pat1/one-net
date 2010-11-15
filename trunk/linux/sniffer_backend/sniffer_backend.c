/*!
 *     \file sniffer_backend.c
 *     \brief Reads binary data from a serial port where the data is supplied by a sniffer front end.
 *
 *     The eval_adi code is being modified and called sniffer_adi to make a ONE-NET sniffer
 *     front end. The ONE-NET sniffer front end will listen for ONE-NET messages at a low
 *     level (as the bits arrive from the transceiver) and send the messages byte by byte
 *     out the serial port (UART). The data will be raw binary bytes as they are read
 *     from the transceiver. All decoding, decrypting, and packet analysis will be performed
 *     by this ONE-NET sniffer backend.
 *
 *     This program is desiged to run on a Linux system.
 *
 *     \author Ruairi Long
 *     \note Threshold Corporation
 *
 */

/***************************************************************************
 *   Copyright (C) 2008 by Integration Lab   *
 *   ilab@thguest.threshold   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>		/* Standard input/output definitions */
#include <string.h>		/* String function definitions */
#include <unistd.h>		/* UNIX standard function definitions */
#include <fcntl.h>		/* File control definitions */
#include <errno.h>		/* Error number definitions */
#include <termios.h>
#include <string.h>
#include <stdlib.h>
//#include <iostream.h>
//#include <cstdlib>
#include <string.h>
#include <stdlib.h>
//using namespace std;
#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "one_net_types.h"
// DEFINE MAIN STRUCTURE FOR PASSING DATA BETWEEN MODULES



// MAIN PROGRAM
int
main (int argc, char *argv[])
{
  int fd, itmp, action;
  unsigned char chandigit1;
  unsigned char chandigit2;
  FILE *fp;
  fp = fopen ("log.txt", "a+");

  printf ("sniffer_temp starting\n");

  fd = open ("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
    {
      printf ("Failed to open /dev/ttys0, errno=%d \n", errno);
      exit (1);
    }



  struct termios attribs;
  itmp = tcgetattr (fd, &attribs);	// initialize termios struct
  itmp = isatty (fd);		//  make sure it is a terminal
  if (itmp != 1)
    {
      printf ("Device name is not a terminal\n");
      exit (1);
    }

  attribs.c_cc[VMIN] = 0;	// THIS PARAMETER IS VERY IMPORTANT
  attribs.c_cc[VTIME] = 1;	// ALSO VERY IMPORTANT
  attribs.c_lflag = 0;		// clear ICANON flag among others
  attribs.c_oflag = 0;
  itmp = cfsetispeed (&attribs, B115200);	// set receive baud rate
  itmp = cfsetospeed (&attribs, B115200);	// set receive baud rate
  itmp = tcsetattr (fd, TCSANOW, &attribs);
  itmp = tcsetattr (fd, TCSAFLUSH, &attribs);


  // ASSIGN SOME LOCALS
  int cur_char;
  unsigned short int i = 0, value1;
  unsigned short int value;
  unsigned short int value10;
  unsigned short int count, code, channel;
  int ierr, tempValue, ControlCharactersCount;
  unsigned short int length[16], length2[16];
  char hexvalues[1000];
  char *output, buffer[10];
  unsigned char sniffmodecommand[] =
    { 's', 'n', 'i', 'f', 'f', ':', 'u', 's', ':', '1', '0', '\n' };
  sniffmodecommand[9] = chandigit1;
  sniffmodecommand[10] = chandigit2;

  // THE FOLLOWING ARE THE LENGTHS IN BYTES OF EACH FRAME, NOT INCLUDING 
  // THE START BYTES OR CHECKSUM BYTES
  count = 0;

  printf ("\nReading bytes ...........\n");

  itmp = tcflush (fd, TCIOFLUSH);
  itmp = tcflush (fd, ONLCR);

  while (1)			// read and process data
    {
      int byte_in_msg;

      itmp = read (fd, &value10, 1);	// read single byte from serial port

      if (-1 != itmp)
	{			// if read was successful, continue
	  value = value10 & 255;	// extract lower 8 bits that were actually read
	  if (value < 255)
	    {			// frame start?
	      // initialize byte count



	      tempValue = value;
	      buffer[count] = value;	// store byte to build up data frame before call to analyze* modules


	      if ((value == 0xf0) || (value == 0xf1))
		{
		  printf ("\n%02x: ", value);
		  byte_in_msg = 0;
		}
	      else
		{
		  byte_in_msg++;
		  if (byte_in_msg == 11)
		    {
		      printf (" [%02x]", value);
		    }
		  else
		    {
		      printf (" %02x", value);
		    }
		}

	      count++;
              fflush(NULL);

	    }			// end if value==255
	}



      if (count == 10)
	{
	  fwrite (buffer, 1, sizeof (buffer), fp);
	  count = 0;
	}
    }				// end main while loop


  close (fd);
  fclose (fp);
  return 0;
}
