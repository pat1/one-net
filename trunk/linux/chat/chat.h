//
// Copyright 2006, Threshold Corporation, All rights reserved.
//

/*!
    \file chat.h
    \brief Header file for ONE-NET chat program (FC5 Version)

*/

#ifndef __CHAT_H__
#define __CHAT_H__

#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include "one_net_types.h"
#include "rf_shm.h"

//==============================================================================
//                              CONSTANTS 

enum
{
    SERIAL,
    AUTO,
    MASTER_NODE,
    CLIENT1,
    CLIENT2,
    CLIENT3,
    STDIN = 1,
    STDERR = 2,
    CHAT = 0x01,
    JOIN_RESPONSE = 0x02,
    NUM_AUTO_CLIENTS = 2,
    PAYLOAD_SIZE = ONE_NET_RAW_SINGLE_DATA_LEN,

    // 1 extra for ID, 1 for null termination
    MAX_PAYLOAD_CHARS = PAYLOAD_SIZE - 2
};

enum
{
    // The default channel
    DEFAULT_CHANNEL = 11,

    // The evaluation data rate
    DATA_RATE = 0,

    // Default encryption method
    EVAL_SINGLE_BLOCK_ENCRYPTION = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,

    // Default stream encryption method
    EVAL_STREAM_ENCRYPTION = ONE_NET_STREAM_ENCRYPT_XTEA8
};

static const char EXIT_CHAR = '`';
static const char CMD_CHAR = '~';

extern const on_encoded_sid_t ENCODED_MASTER_SID;
extern const one_net_raw_did_t RAW_AUTO_CLIENT_DID[];
extern const one_net_xtea_key_t EVAL_KEY;

extern const UInt8 DEFAULT_SINGLE_BLOCK_ENCRYPT;

extern const char BLOCK_TEST_DATA[];

//                              CONSTANTS END
//==============================================================================


//==============================================================================
//                              TYPEDEFS 

//                              TYPEDEFS END
//==============================================================================


//==============================================================================
//                          PUBLIC VARIABLES 
extern UInt8 mode_value;
extern UInt8 node_select_val;
extern char * debug_file;
//                          PUBLIC VARIABLES END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS 

void master_chat(void);
void client_chat(void);

BOOL getlstr(char * str, const unsigned int SIZE);

//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

#endif
