#ifndef _ONCLI_STR_H 
#define _ONCLI_STR_H 


//! \defgroup oncli_str ONE-NET Command Line Interface Strings
//! \ingroup oncli
//! @{

/*
    Copyright (c) 2010, Threshold Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice,
          this list of conditions, and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of Threshold Corporation (trustee of ONE-NET) nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHEWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!
    \file oncli_str.h

    \brief Contains string constants associated with the ONE-NET Command Line
      interface.

    These strings are to be used in a case insensitive manner.
*/



#include "config_options.h"

#ifdef _ENABLE_CLI

//==============================================================================
//								CONSTANTS
//! \defgroup oncli_str_const 
//! \ingroup oncli_str
//! @{

enum
{
    //! Number of strings in the admin string array
    ONCLI_ADMIN_STR_COUNT = 35,
    
    //! Number of strings in the extended admin string array
    ONCLI_EXTENDED_ADMIN_STR_COUNT = 1,
    
    //! Number of strings in the MASTER update string array
    ONCLI_M_UPDATE_STR_COUNT = 10
};

//! Eval board general error codes
typedef enum _oncli_gen_err
{
    ONCLI_GEN_ERR_BAD_PARAMETER = 1,        //!< Invalid paramter passed to a function
    ONCLI_GEN_ERR_PAYLOAD = 2,              //!< Error encountered parsing a message payload
    ONCLI_GEN_ERR_WRONG_BLOCK_DATA_TYPE = 3 //!< The block data payload message type did 
                                            //!< not match the type from the single data message

} oncli_gen_err_t;

// Application Formats
extern const char * const ONCLI_STARTUP_FMT;
extern const char * const ONCLI_STARTUP_REV_FMT;

// Response formats
extern const char * const ONCLI_DEVICE_ADD_FMT;
extern const char * const ONCLI_ADD_DEVICE_FAILED_FMT;
extern const char * const ONCLI_JOINED_FMT;
extern const char * const ONCLI_RX_DATA_FMT;
extern const char * const ONCLI_RX_TXT_FMT;
extern const char * const ONCLI_SINGLE_RESULT_FMT;
extern const char * const ONCLI_BLOCK_RESULT_FMT;
extern const char * const ONCLI_UPDATE_RESULT_FMT;
extern const char * const ONCLI_UPDATE_RESULT_WITH_OUT_DID_FMT;
extern const char * const ONCLI_UNKNOWN_UPDATE_RESULT_FMT;
extern const char * const ONCLI_DATA_RATE_TEST_RESULT_FMT;
extern const char * const ONCLI_GET_CHANNEL_RESPONSE_FMT;
#ifdef _ONE_NET_DEBUG
    //! debug format strings
    extern const char * const ONCLI_DEBUG_PREFIX_STR;
    extern const char * const ONCLI_DEBUG_LENGTH_STR;
    extern const char * const ONCLI_DEBUG_BYTE_STR;
    extern const char * const ONCLI_DEBUG_SUFFIX_STR;
#endif

// Response string
extern const char * const ONCLI_CMD_SUCCESS_STR;
extern const char * const ONCLI_SUCCEEDED_STR;
extern const char * const ONCLI_FAILED_STR;
extern const char * const ONCLI_CLR_INPUT_STR;
extern const char * const ONCLI_M_RM_DEV_ANYWAY_STR;
extern const char * const ONCLI_CHANNEL_NOT_SELECTED_STR;

// Error Formats
extern const char * const ONCLI_INVALID_CMD_FMT;
extern const char * const ONCLI_CMD_FAIL_FMT;
extern const char * const ONCLI_INVALID_CMD_FOR_DEVICE_FMT;
extern const char * const ONCLI_INTERNAL_ERR_FMT;
extern const char * const ONCLI_GENERAL_ERROR_FMT;
extern const char * const ONCLI_RX_INVALID_CH_FMT;
extern const char * const ONCLI_OUTPUT_STR_TOO_SHORT_FMT;

// Error Strings
extern const char * const ONCLI_IN_PROGRESS_STR;
extern const char * const ONCLI_RSRC_UNAVAILABLE_STR;
extern const char * const ONCLI_UNSUPPORTED_STR;
extern const char * const ONCLI_INVALID_FORMAT_STR;
extern const char * const ONCLI_SNGH_STR;
extern const char * const ONCLI_ONS_NOT_INIT_ERR_STR;
extern const char * const ONCLI_INTERNAL_ERR_STR;
extern const char * const ONCLI_INVALID_DST_STR;
extern const char * const ONCLI_NEED_TO_JOIN_STR;
extern const char * const ONCLI_INVALID_CMD_LEN_STR;
extern const char * const ONCLI_LOAD_FAIL_STR;
extern const char * const ONCLI_DISPLAY_INVITE_STR;
extern const char * const ONCLI_FATAL_ERR_1_STR;
enum {ONCLI_FATAL_ERR_1_LEN = 15};



// Command Strings
#ifdef _ENABLE_IDLE_COMMAND
extern const char * const ONCLI_IDLE_CMD_STR;
#endif
extern const char * const ONCLI_SINGLE_CMD_STR;
extern const char * const ONCLI_SET_PIN_CMD_STR;
extern const char * const ONCLI_SET_VALUE_CMD_STR;
extern const char * const ONCLI_SINGLE_TXT_CMD_STR;
extern const char * const ONCLI_BLOCK_CMD_STR;
extern const char * const ONCLI_BLOCK_TXT_CMD_STR;

extern const char * const ONCLI_ERASE_CMD_STR;
extern const char * const ONCLI_SAVE_CMD_STR;

#ifdef _ENABLE_DUMP_COMMAND
extern const char * const ONCLI_DUMP_CMD_STR;
#endif

#ifdef _ENABLE_RSINGLE_COMMAND
extern const char * const ONCLI_RSEND_CMD_STR;
#endif

#ifdef _ENABLE_RSSI_COMMAND
extern const char * const ONCLI_RSSI_CMD_STR;
#endif

#ifdef _ENABLE_LIST_COMMAND
extern const char * const ONCLI_LIST_CMD_STR;
#ifdef _PEER
extern const char * const ONCLI_LIST_PEER_TABLE_HEADING;
extern const char * const ONCLI_LIST_PEER_FMT;
extern const char * const ONCLI_LIST_NO_PEERS;
#endif
#endif

// Mode strings
#ifdef _AUTO_MODE
	extern const char * const ONCLI_AUTO_MODE_STR;
#endif
extern const char * const ONCLI_SERIAL_MODE_STR;

// Device strings
extern const char * const ONCLI_MASTER_STR;
extern const char * const ONCLI_CLIENT_STR;
#ifdef _SNIFFER_MODE
	extern const char * const ONCLI_SNIFFER_STR;
#endif

// MASTER only command strings
extern const char * const ONCLI_INVITE_CMD_STR;
extern const char * const ONCLI_CANCEL_INVITE_CMD_STR;
#ifdef _ENABLE_ASSIGN_PEER_COMMAND
extern const char * const ONCLI_ASSIGN_PEER_CMD_STR;
#endif
#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
extern const char * const ONCLI_UNASSIGN_PEER_CMD_STR;
#endif
extern const char * const ONCLI_UPDATE_MASTER_CMD_STR;
extern const char * const ONCLI_CHANGE_KEEP_ALIVE_CMD_STR;
extern const char * const ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR;
extern const char * const ONCLI_CHANGE_KEY_CMD_STR;
extern const char * const ONCLI_RM_DEV_CMD_STR;
extern const char * const ONCLI_DATA_RATE_TEST_CMD_STR;
extern const char * const ONCLI_GET_CHANNEL_CMD_STR;

// CLIENT only command strings
extern const char * const ONCLI_USER_PIN_CMD_STR;

// Mode strings strings
extern const char * const ONCLI_JOIN_CMD_STR;
extern const char * const ONCLI_CHANNEL_CMD_STR;
extern const char * const ONCLI_SETNI_CMD_STR;
#ifdef _SNIFFER_MODE
	extern const char * const ONCLI_SNIFF_CMD_STR;
#endif
#ifdef _AUTO_MODE
	extern const char * const ONCLI_MODE_CMD_STR;
#endif
extern const char * const ONCLI_ECHO_CMD_STR;

// Transaction strings
extern const char * const ONCLI_SINGLE_TXN_STR;
extern const char * const ONCLI_BLOCK_TXN_STR;

// Argument strings
extern const char * const ONCLI_SET_STR;
extern const char * const ONCLI_CLR_STR;
extern const char * const ONCLI_LOW_STR;
extern const char * const ONCLI_HIGH_STR;
extern const char * const ONCLI_TOGGLE_STR;
extern const char * const ONCLI_INPUT_STR;
extern const char * const ONCLI_OUTPUT_STR;
extern const char * const ONCLI_DISABLE_STR;
extern const char * const ONCLI_QUIET_STR;
extern const char * const ONCLI_VERBOSE_STR;
extern const char * const ONCLI_ON_STR;
extern const char * const ONCLI_OFF_STR;
#ifdef _US_CHANNELS
extern const char * const ONCLI_US_STR;
#endif
#ifdef _EUROPE_CHANNELS
extern const char * const ONCLI_EUR_STR;
#endif

// Transaction type strings
extern const char * const ONCLI_SINGLE_STR;
extern const char * const ONCLI_BLOCK_STR;

// Admin message format
extern const char * const ONCLI_RX_ADMIN_FMT;

// Admin strings
extern const char * const ONCLI_ADMIN_MSG_STR;
extern const char * const ONCLI_EXTENDED_ADMIN_MSG_STR;

// Admin message string table
extern const char * const ONCLI_ADMIN_STR[];

// Extended admin message string table
extern const char * const ONCLI_EXTENDED_ADMIN_STR[];

// MASTER update string table
extern const char * const ONCLI_M_UPDATE_STR[];

// status strings
extern const char * const ONCLI_ONS_STR[];

//! @} oncli_str_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup oncli_str_typedefs
//! \ingroup oncli_str
//! @{

//! @} oncli_str_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup oncli_str_pub_var
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup oncli_str_pub_func
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli_str

#endif // #ifdef _ENABLE_CLI

#endif // #ifdef _ONCLI_STR_H //

