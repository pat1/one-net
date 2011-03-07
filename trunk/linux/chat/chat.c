//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file chat.c
   \brief chat function for running ONE_NET chat program in FC5 environment
    
   \author  Adam Meadows
   \date $Date: 2006/10/02 08:37:39 $
   \version $Rev: 82 $
   \note Threshold Corporation
   
*/ 

#include "chat.h"
#include "debug.h"

// TBD REMOVE INCLUDING THIS.  THIS IS NEEDED FOR THE ADDRESSES
#include "one_net.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

//==============================================================================
//                                  CONSTANTS

static const char * const MASTER_STR = "M";
static const char * const CLIENT1_STR = "C1";
static const char * const CLIENT2_STR = "C2";
static const char * const CLIENT3_STR = "C3";

static const char * const AUTO_STR = "auto";
static const char * const SERIAL_STR = "serial";

const on_encoded_sid_t ENCODED_MASTER_SID = {0xD2, 0x66, 0x99, 0x55, 0xAA, 0x33,
  0xB4, 0xBC};

//const one_net_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] = {{0x00, 0x20},
//  {0x00, 0x30}, {0x00, 0x40}};
const one_net_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] = {{0x00, 0x20},
    {0x00, 0x30}};

const one_net_xtea_key_t EVAL_KEY = {0x02, 0x99, 0x33, 0x13, 0x97, 0x22, 0xAA,
  0xA4, 0xDD, 0xBE, 0xEF, 0x79, 0x00, 0x11, 0x3A, 0x20};

const UInt8 DEFAULT_SINGLE_BLOCK_ENCRYPT = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;

//! Data to be sent to test the block transaction
const char BLOCK_TEST_DATA [] =
  "This is the MASTER send block test data.  It consists of the lower case \
  alphabet followed by 0-9, followed by the upper case alphabet\n \
  abcdefghijklmnopqrstuvwxyz\n0123456789\nABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

//                              CONSTANTS END
//                                  CONSTANTS END
//==============================================================================

UInt8 mode_value;
UInt8 node_select_val;

void usage(void);


/*!
    \brief Reads a string in from STDIN

    Reads until white space occurs or LEN - 1 bytes are read.  NULL terminates
    the string.  Backspaces will erase the last character.

    \param[out] str The string read in
    \param[in] SIZE The size in bytes of str.  This needs to be > 1 since this
      function NULL terminates the string.
*/
BOOL getlstr(char * str, const unsigned int SIZE)
{
    unsigned int len = 0;

    if(!str || SIZE <= 1)
    {
        return FALSE;
    } // if parameters are invalid //

    while(len < SIZE - 1)
    {
        str[len] = getchar();
        if(str[len] == '\b')
        {
            if(len)
            {
                len--;
            } // if len is positive //
        } // if a backspace //
        else if(str[len] == '\t' || str[len] == '\r' || str[len] == '\n'
         || str[len] == ' ')
        {
            len++;
            break;
        } // else if white space //
        else
        {
            len++;
        } // else character //
    } // loop to read in the characters //

    str[SIZE - 1] = '\0';
    return TRUE;
} // getnstr //


int main(int argc, char **argv) 
{
    //by default no debug
    set_debug_level(0);

    //check correct # of args
    if(argc == 4)
    {
        //set up debugging if filename given
        set_debug_level(1);
        set_debug_file(argv[3]);
    }
    else if( argc != 3)
    {
        usage();
        return 0;
    }
   
    //check mode_value
    if(strcmp(argv[2], AUTO_STR) == 0)
    {
        mode_value = AUTO;
    }
    else if(strcmp(argv[2], SERIAL_STR) == 0)
    {
        mode_value = SERIAL;
    }
    else
    {
        usage();
        return 0;
    }
 
    // initialize shared memory
    if(init_shm() == -1)
    {
        printf("init_shm failed!\n");
        fflush(NULL);
        return 1;
    } // if init_shm failed //
  
    // check node_select
    if(strcmp(argv[1], MASTER_STR) == 0)
    {
        node_select_val = MASTER_NODE;
        master_chat(); 
    }
    else if(strcmp(argv[1], CLIENT1_STR) == 0)
    {
        node_select_val = CLIENT1;
        client_chat(); 
    }
    else if(strcmp(argv[1], CLIENT2_STR) == 0)
    {
        node_select_val = CLIENT2;
        client_chat(); 
    }
    else if(strcmp(argv[1], CLIENT3_STR) == 0)
    {
        node_select_val = CLIENT3;
        client_chat(); 
    }
    else
    {
        usage();
        return 0;
    }

    return 0;
} // end main //


void usage(void)
{
    printf("\nusage:\n"
      "\tchat TYPE MODE [DEBUG FILE]\n\n"
      "\t\tTYPE \t\t- ( M | C1 | C2 | C3 )\n"
      "\t\tMODE \t\t- ( auto | serial )\n"
      "\t\tDEBUG FILE \t- optional debug filename\n\n");
} //usage
