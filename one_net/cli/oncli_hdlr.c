//! \addtogroup oncli_hdlr
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
    \file oncli_hdlr.c
    \brief Contains the implementation for handling of ONE-NET Command Line
      Interface commands.
*/

#include "config_options.h"
#include "oncli_hdlr.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "oncli.h"
#include "oncli_str.h"
#include "str.h"
#include "one_net.h"
#include "oncli_port.h"
#include "one_net_data_rate.h"
#include "tal.h"
#include "one_net_encode.h"
#include "one_net_application.h"
#ifdef _PEER
#include "one_net_peer.h"
#endif
#include "one_net_port_specific.h"
#ifdef _ONE_NET_CLIENT
#include "one_net_client.h"
#include "one_net_client_port_specific.h"
#endif
#ifdef _ONE_NET_MASTER
#include "one_net_master_port_specific.h"
#include "one_net_master_port_const.h"
#include "one_net_master.h"
#endif
#ifdef _NON_VOLATILE_MEMORY
#include "dfi.h"
#endif

#ifdef _DEBUGGING_TOOLS
#include "one_net_timer.h"
#endif

#include "tick.h"

#ifdef _ENABLE_SINGLE_COMMAND
#include "io_port_mapping.h"
#endif




//==============================================================================
//								CONSTANTS
//! \defgroup oncli_hdlr_const
//! \ingroup oncli_hdlr
//! @{



enum
{
    //! The number of ASCII hex characters per raw did
    ONCLI_ASCII_RAW_DID_SIZE = 3,
};


//! Parameter delimiter
static const char ONCLI_PARAM_DELIMITER = ':';

// debugging -- temporary
static const char* ADD_DEV_CMD_STR = "add dev";




//! @} oncli_hdlr_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup oncli_hdlr_typedefs
//! \ingroup oncli_hdlr
//! @{

//! @} oncli_hdlr_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup oncli_hdlr_pri_var
//! \ingroup oncli_hdlr
//! @{

// temporary --  debugging
static const on_features_t add_master_features = {0x2E, 0x03, 0x3F, 0x87};
static const on_raw_nid_t  add_nid = {0x00, 0x00, 0x00, 0x00, 0x10};
static const one_net_xtea_key_t add_key = {0x00, 0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
static const UInt8 add_flags = ON_JOINED | ON_SEND_TO_MASTER;
static const tick_t add_keep_alive = 1800000;
static const UInt8 add_channel = 1;
static const UInt8 add_single_block_encrypt =
  ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;
#ifdef _BLOCK_MESSAGES_ENABLED
static const UInt8 add_fragment_delay_low = 125;
static const UInt8 add_fragment_delay_high = 25;
#endif
#ifdef _STREAM_MESSAGES_ENABLED
static const UInt8 add_stream_encrypt = ONE_NET_STREAM_ENCRYPT_XTEA8;
#endif




#ifdef _DEBUGGING_TOOLS
// Debug memory
typedef enum
{
    DEBUG_MEMORY_ON_STATE,
    DEBUG_MEMORY_TIMER,
    #ifdef _PEER
    DEBUG_MEMORY_PEER,
    #endif
    DEBUG_MEMORY_BASE_PARAM,
    DEBUG_MEMORY_INVITE_TXN,
    DEBUG_MEMORY_RESPONSE_TXN,
    DEBUG_MEMORY_SINGLE_TXN,
    #ifdef _BLOCK_MESSAGES_ENABLED
    DEBUG_MEMORY_BLOCK_TXN,
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    DEBUG_MEMORY_STREAM_TXN,
    #endif
    DEBUG_MEMORY_COUNT
} debug_memory_t;


static const char* debug_memory_str[DEBUG_MEMORY_COUNT] =
{
    "on_state",
    "timer",
    #ifdef _PEER
    "peer",
    #endif    
    "base_param",
    "invite_txn",
    "response_txn",
    "single_txn",
    #ifdef _BLOCK_MESSAGES_ENABLED
    "block_txn",
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    "stream_txn",
    #endif
};


//! The memory currently being pointed to.
static UInt8* memory_ptr = NULL;

//! The relevant length of the memory currently being pointed to.
int memory_len = 0;
#endif






//! @} oncli_hdlr_pri_var
//							PRIVATE VARIABLES END
//==============================================================================



//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup oncli_hdlr_pub_var
//! \ingroup oncli_hdlr
//! @{



//! @} oncli_hdlr_pub_var
//							PRIVATE VARIABLES END
//==============================================================================




//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup oncli_hdlr_pri_func
//! \ingroup oncli_hdlr
//! @{


// Command handlers.
#ifdef _ENABLE_ECHO_COMMAND
	static oncli_status_t echo_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_LIST_COMMAND 
	static oncli_status_t list_cmd_hdlr(void);
#endif

#ifdef _ENABLE_ERASE_COMMAND 
	static oncli_status_t erase_cmd_hdlr(void);
#endif

#ifdef _ENABLE_SAVE_COMMAND 
	static oncli_status_t save_cmd_hdlr(void);
#endif

#if defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)
oncli_status_t sniff_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_SINGLE_COMMAND
static oncli_status_t oncli_send_single(const on_raw_did_t* const dst,
    const UInt8* const payload, const BOOL send_to_peer_list,
    const on_priority_t priority);
static oncli_status_t single_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t single_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t status_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t query_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t fast_query_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t set_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
static oncli_status_t parse_and_send_pin_msg(
  const char * const ASCII_PARAM_LIST, UInt16 msg_class);
#endif

#ifdef _ENABLE_SET_DATA_RATE_COMMAND
oncli_status_t set_data_rate_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_USER_PIN_COMMAND
	static oncli_status_t user_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
	static oncli_status_t change_frag_dly_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif



// MASTER only command handlers
#ifdef _ENABLE_INVITE_COMMAND
	static oncli_status_t invite_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_CANCEL_INVITE_COMMAND
	static oncli_status_t cancel_invite_cmd_hdlr(void);
#endif
#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
	static oncli_status_t rm_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#if defined(_ENABLE_ASSIGN_PEER_COMMAND) || defined(_ENABLE_UNASSIGN_PEER_COMMAND)
static oncli_change_peer_list(BOOL ASSIGN,
  const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_ASSIGN_PEER_COMMAND
	static oncli_status_t assign_peer_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
	static oncli_status_t unassign_peer_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_UPDATE_MASTER_COMMAND
	static oncli_status_t update_master_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif  
#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
	static oncli_status_t change_keep_alive_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_CHANGE_KEY_COMMAND
static oncli_status_t change_single_block_key_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_CHANNEL_COMMAND
static oncli_status_t channel_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_JOIN_COMMAND
oncli_status_t join_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_SETNI_COMMAND
static oncli_status_t setni_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _RANGE_TESTING
static oncli_status_t range_test_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ONE_NET_MULTI_HOP
// temporary debugging
static oncli_status_t mh_repeat_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

// temporary debugging
static oncli_status_t add_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST);




// Parsing functions.
static UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const UInt16 NUM_ASCII_CHAR);
#ifdef _ENABLE_SINGLE_COMMAND

#ifdef _PEER
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit,
  on_encoded_did_t * const enc_dst, BOOL* send_to_peer_list);
#else
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit,
  on_encoded_did_t * const enc_dst);
#endif

static const char * parse_ascii_tx_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len);
static const char * parse_ascii_tx_text_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len);
#endif
static oncli_status_t oncli_parse_channel(const char * ASCII,
  UInt8 * const channel);
  
#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t parse_invite_key(const char * ASCII,
  char** end_ptr, one_net_xtea_key_t * const key);
#endif


// debugging tools
#ifdef _DEBUGGING_TOOLS
static int get_memory_loc(UInt8** mem_ptr, debug_memory_t memory_type,
  int index, int offset);
static int parse_memory_str(UInt8** mem_ptr,
  const char * const ASCII_PARAM_LIST);
static oncli_status_t memory_cmd_hdlr(
  const char * const ASCII_PARAM_LIST);
static oncli_status_t memdump_cmd_hdlr(void);
static oncli_status_t memset_cmd_hdlr(
  const char * const ASCII_PARAM_LIST);
static oncli_status_t memload_cmd_hdlr(
  const char * const ASCII_PARAM_LIST);
static oncli_status_t pause_cmd_hdlr(void);
static oncli_status_t proceed_cmd_hdlr(void);
static oncli_status_t ratchet_cmd_hdlr(void);
static oncli_status_t interval_cmd_hdlr(
  const char * const ASCII_PARAM_LIST);
#endif





//! @} oncli_hdlr_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup oncli_hdlr_pub_func
//! \ingroup oncli_hdlr
//! @{

/*!
    \brief Parses a command that has been completely read in.
    
    Parses the command and will call the appropriate handler to parse the
    parameters and execute the command.  This function will also return why
    a command may have failed. It is within this function that we decide
    whether or not to continue reading data looking for paramters to 
    functions. 

    12/12/08: With the revised implementation of the SID command, we want
    the behavior of the command to vary depending on whether or not 
    parameters are supplied. So, we must set the commnand to expect 
    parameters. This means that at least a semicolon must be typed
    after the command to ensure that the command handler is executed.
    It is then up to the command handler to figure out if there are any
    parameters after the semicolon and take the appropriate action.

    \param[in] CMD The command to be parsed.
    \param[out] CMD_STR The command string of the command that was passed in.
    \param[out] read_param_state The next state for reading in the command
      parameters.  This is only set if a state to read in parameters is
      required.
    \param[out] cmd_hdlr The handler that should be called when the parameters
      are read in.  If this is NULL, and ONCLI_SUCCESS is returned, that means
      the command had no parameters are is complete.
    
    \return ONCLI_SUCCESS The the command was successfully parsed
            ONCLI_BAD_PARAM If any of the parameters passed in were invalid.
            ONCLI_PARSE_ERR If the command was not parsed correctly
*/
oncli_status_t oncli_parse_cmd(const char * const CMD, const char ** CMD_STR,
  UInt8 * const next_state, oncli_cmd_hdlr_t * const cmd_hdlr)
{
    oncli_status_t oncli_status = ONCLI_INTERNAL_ERR;

    if(!CMD || !CMD_STR || !next_state || !cmd_hdlr)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //

    *cmd_hdlr = 0;

	#ifdef _ENABLE_ECHO_COMMAND
    if(!strnicmp(ONCLI_ECHO_CMD_STR, CMD, strlen(ONCLI_ECHO_CMD_STR)))
    {
        *CMD_STR = ONCLI_ECHO_CMD_STR;
        
        if(CMD[strlen(ONCLI_ECHO_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //
        
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &echo_cmd_hdlr;
        
        return ONCLI_SUCCESS;
    } // else if the echo command was received //
	#endif
    
	#ifdef _ENABLE_LIST_COMMAND 
    if(!strnicmp(ONCLI_LIST_CMD_STR, CMD, strlen(ONCLI_LIST_CMD_STR)))
    {
        *CMD_STR = ONCLI_LIST_CMD_STR;

        if(CMD[strlen(ONCLI_LIST_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return list_cmd_hdlr();
    } // else if the list command was received //
	#endif
    
	#ifdef _ENABLE_ERASE_COMMAND 
    else if(!strnicmp(ONCLI_ERASE_CMD_STR, CMD, strlen(ONCLI_ERASE_CMD_STR)))
    {
        *CMD_STR = ONCLI_ERASE_CMD_STR;

        if(CMD[strlen(ONCLI_ERASE_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return erase_cmd_hdlr();
    } // else if the erase command was received //
	#endif
    
	#ifdef _ENABLE_SAVE_COMMAND 
    else if(!strnicmp(ONCLI_SAVE_CMD_STR, CMD, strlen(ONCLI_SAVE_CMD_STR)))
    {
        *CMD_STR = ONCLI_SAVE_CMD_STR;

        if(CMD[strlen(ONCLI_SAVE_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return save_cmd_hdlr();
    } // else if the save command was received //
	#endif

	#if defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)
    else if(!strnicmp(ONCLI_SNIFF_CMD_STR, CMD, strlen(ONCLI_SNIFF_CMD_STR)))
    {
        oncli_status = sniff_cmd_hdlr(CMD + strlen(ONCLI_SNIFF_CMD_STR) + 1);
        
        *CMD_STR = ONCLI_SNIFF_CMD_STR;

        if(CMD[strlen(ONCLI_SNIFF_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &sniff_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the sniff command was received //
	#endif
    
	#ifdef _ENABLE_SINGLE_COMMAND    
    else if(!strnicmp(ONCLI_SINGLE_TXT_CMD_STR, CMD, strlen(ONCLI_SINGLE_TXT_CMD_STR)))
    {
        *CMD_STR = ONCLI_SINGLE_TXT_CMD_STR;

        if(CMD[strlen(ONCLI_SINGLE_TXT_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &single_txt_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the send single text command was received //    
    
    else if(!strnicmp(ONCLI_SINGLE_CMD_STR, CMD, strlen(ONCLI_SINGLE_CMD_STR)))
    {
        *CMD_STR = ONCLI_SINGLE_CMD_STR;

        if(CMD[strlen(ONCLI_SINGLE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &single_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the send single command was received //
    
    else if(!strnicmp(ONCLI_STATUS_PIN_CMD_STR, CMD,
      strlen(ONCLI_STATUS_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_STATUS_PIN_CMD_STR;

        if(CMD[strlen(ONCLI_STATUS_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &status_pin_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the status pin command was received //
    
    else if(!strnicmp(ONCLI_QUERY_PIN_CMD_STR, CMD,
      strlen(ONCLI_QUERY_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_QUERY_PIN_CMD_STR;

        if(CMD[strlen(ONCLI_QUERY_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &query_pin_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the query command was received //

    else if(!strnicmp(ONCLI_FAST_QUERY_PIN_CMD_STR, CMD,
      strlen(ONCLI_FAST_QUERY_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_FAST_QUERY_PIN_CMD_STR;

        if(CMD[strlen(ONCLI_FAST_QUERY_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &fast_query_pin_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the fast query command was received //
    
    else if(!strnicmp(ONCLI_SET_PIN_CMD_STR, CMD, strlen(ONCLI_SET_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_SET_PIN_CMD_STR;

        if(CMD[strlen(ONCLI_SET_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &set_pin_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the set pin command was received //
	#endif // _ENABLE_SINGLE_COMMAND //

	#ifdef _ENABLE_SET_DATA_RATE_COMMAND
    else if(!strnicmp(ONCLI_SET_DATA_RATE_CMD_STR, CMD,
      strlen(ONCLI_SET_DATA_RATE_CMD_STR)))
    {
        *CMD_STR = ONCLI_SET_DATA_RATE_CMD_STR;
        
        if(CMD[strlen(ONCLI_SET_DATA_RATE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //
        
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &set_data_rate_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the set data rate command was received //
	#endif
    
	#ifdef _ENABLE_USER_PIN_COMMAND
    else if(!strnicmp(ONCLI_USER_PIN_CMD_STR, CMD,
      strlen(ONCLI_USER_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_USER_PIN_CMD_STR;
        
        if(CMD[strlen(ONCLI_USER_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //
        
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &user_pin_cmd_hdlr;
        
        return ONCLI_SUCCESS;
    } // else if the user pin command was received //
	#endif
    
	#ifdef _ENABLE_INVITE_COMMAND
    else if(!strnicmp(ONCLI_INVITE_CMD_STR, CMD, strlen(ONCLI_INVITE_CMD_STR)))
    {
        *CMD_STR = ONCLI_INVITE_CMD_STR;

        if(CMD[strlen(ONCLI_INVITE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &invite_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the invite command was received //
	#endif
	
	#ifdef _ENABLE_CANCEL_INVITE_COMMAND
    else if(!strnicmp(ONCLI_CANCEL_INVITE_CMD_STR, CMD,
      strlen(ONCLI_CANCEL_INVITE_CMD_STR)))
    {
        *CMD_STR = ONCLI_CANCEL_INVITE_CMD_STR;

        if(CMD[strlen(ONCLI_CANCEL_INVITE_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return cancel_invite_cmd_hdlr();
    } // else if the cancel invite command was received //
	#endif

	#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    else if(!strnicmp(ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR, CMD,
      strlen(ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR;

        if(CMD[strlen(ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR)]
          != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &change_frag_dly_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change fragment delay command was received //
	#endif

	#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
    else if(!strnicmp(ONCLI_RM_DEV_CMD_STR, CMD, strlen(ONCLI_RM_DEV_CMD_STR)))
    {
        *CMD_STR = ONCLI_RM_DEV_CMD_STR;

        if(CMD[strlen(ONCLI_RM_DEV_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &rm_dev_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the remove device command was received //
	#endif
    
	#ifdef _ENABLE_ASSIGN_PEER_COMMAND
    else if(!strnicmp(ONCLI_ASSIGN_PEER_CMD_STR, CMD,
      strlen(ONCLI_ASSIGN_PEER_CMD_STR)))
    {
        *CMD_STR = ONCLI_ASSIGN_PEER_CMD_STR;

        if(CMD[strlen(ONCLI_ASSIGN_PEER_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &assign_peer_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the assign peer command was received //
	#endif
		
	#ifdef _ENABLE_UNASSIGN_PEER_COMMAND	
    else if(!strnicmp(ONCLI_UNASSIGN_PEER_CMD_STR, CMD,
      strlen(ONCLI_UNASSIGN_PEER_CMD_STR)))
    {
        *CMD_STR = ONCLI_UNASSIGN_PEER_CMD_STR;

        if(CMD[strlen(ONCLI_UNASSIGN_PEER_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &unassign_peer_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the unassign peer command was received //
	#endif

	#ifdef _ENABLE_UPDATE_MASTER_COMMAND
    else if(!strnicmp(ONCLI_UPDATE_MASTER_CMD_STR, CMD,
      strlen(ONCLI_UPDATE_MASTER_CMD_STR)))
    {
        *CMD_STR = ONCLI_UPDATE_MASTER_CMD_STR;

        if(CMD[strlen(ONCLI_UPDATE_MASTER_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &update_master_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the update MASTER command was received //
	#endif
    
	#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
    else if(!strnicmp(ONCLI_CHANGE_KEEP_ALIVE_CMD_STR, CMD,
      strlen(ONCLI_CHANGE_KEEP_ALIVE_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANGE_KEEP_ALIVE_CMD_STR;

        if(CMD[strlen(ONCLI_CHANGE_KEEP_ALIVE_CMD_STR)]
          != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &change_keep_alive_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change keep alive command was received //
	#endif

    // debugging temporary
    else if(!strnicmp("add dev", CMD, 7))
    {
        *CMD_STR = ADD_DEV_CMD_STR;

        if(CMD[7] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &add_dev_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the "add dev" command was received //
    
	#ifdef _ENABLE_CHANGE_KEY_COMMAND
    else if(!strnicmp(ONCLI_CHANGE_KEY_CMD_STR, CMD,
      strlen(ONCLI_CHANGE_KEY_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANGE_KEY_CMD_STR;

        if(CMD[strlen(ONCLI_CHANGE_KEY_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &change_single_block_key_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change key command was received //
	#endif
    
	#ifdef _ENABLE_CHANNEL_COMMAND
    if(!strnicmp(ONCLI_CHANNEL_CMD_STR, CMD,
      strlen(ONCLI_CHANNEL_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANNEL_CMD_STR;

        if(CMD[strlen(ONCLI_CHANNEL_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &channel_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the channel command was received //
	#endif
    
	#ifdef _ENABLE_JOIN_COMMAND
    if(!strnicmp(ONCLI_JOIN_CMD_STR, CMD, strlen(ONCLI_JOIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_JOIN_CMD_STR;

        #ifndef _ENHANCED_INVITE
        if(CMD[strlen(ONCLI_JOIN_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //
        
		return (one_net_client_reset_client(one_net_client_get_invite_key()) ==
          ONS_SUCCESS) ? ONCLI_SUCCESS : ONCLI_CMD_FAIL;
        #else
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &join_cmd_hdlr;
        
        return ONCLI_SUCCESS;
        #endif        
    } // else if the join command was received //
    #endif
    
    #ifdef _ENABLE_SETNI_COMMAND
    if(!strnicmp(ONCLI_SETNI_CMD_STR, CMD, strlen(ONCLI_SETNI_CMD_STR)))
    {
        *CMD_STR = ONCLI_SETNI_CMD_STR;

        if(CMD[strlen(ONCLI_SETNI_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &setni_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the sid command was received //
    #endif
    
    #ifdef _RANGE_TESTING
    if(!strnicmp(ONCLI_RANGE_TEST_CMD_STR, CMD,
      strlen(ONCLI_RANGE_TEST_CMD_STR)))
    {
        *CMD_STR = ONCLI_RANGE_TEST_CMD_STR;

        if(CMD[strlen(ONCLI_RANGE_TEST_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &range_test_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the range test command was received //
    #endif
    
    #ifdef _ONE_NET_MULTI_HOP
    if(!strnicmp(ONCLI_MH_REPEAT_CMD_STR, CMD,
      strlen(ONCLI_MH_REPEAT_CMD_STR)))
    {
        *CMD_STR = ONCLI_MH_REPEAT_CMD_STR;

        if(CMD[strlen(ONCLI_MH_REPEAT_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &mh_repeat_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the range test command was received //
    #endif
    
    #ifdef _DEBUGGING_TOOLS
    if(!strnicmp(ONCLI_MEMORY_CMD_STR, CMD, strlen(ONCLI_MEMORY_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMORY_CMD_STR;

        if(CMD[strlen(ONCLI_MEMORY_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &memory_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the memory command was received //

    if(!strnicmp(ONCLI_MEMLOAD_CMD_STR, CMD, strlen(ONCLI_MEMLOAD_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMLOAD_CMD_STR;

        if(CMD[strlen(ONCLI_MEMLOAD_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &memload_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the memload command was received //

    if(!strnicmp(ONCLI_MEMSET_CMD_STR, CMD, strlen(ONCLI_MEMSET_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMSET_CMD_STR;

        if(CMD[strlen(ONCLI_MEMSET_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &memset_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the memset command was received //

    if(!strnicmp(ONCLI_MEMDUMP_CMD_STR, CMD, strlen(ONCLI_MEMDUMP_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMDUMP_CMD_STR;

        if(CMD[strlen(ONCLI_MEMDUMP_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return memdump_cmd_hdlr();
    } // else if the memdump command was received //

    if(!strnicmp(ONCLI_INTERVAL_CMD_STR, CMD, strlen(ONCLI_INTERVAL_CMD_STR)))
    {
        *CMD_STR = ONCLI_INTERVAL_CMD_STR;

        if(CMD[strlen(ONCLI_INTERVAL_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &interval_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the interval command was received //

    if(!strnicmp(ONCLI_PAUSE_CMD_STR, CMD, strlen(ONCLI_PAUSE_CMD_STR)))
    {
        *CMD_STR = ONCLI_PAUSE_CMD_STR;

        if(CMD[strlen(ONCLI_PAUSE_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return pause_cmd_hdlr();
    } // else if the pause command was received //
    if(!strnicmp(ONCLI_RATCHET_CMD_STR, CMD, strlen(ONCLI_RATCHET_CMD_STR)))
    {
        *CMD_STR = ONCLI_RATCHET_CMD_STR;

        if(CMD[strlen(ONCLI_RATCHET_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return ratchet_cmd_hdlr();
    } // else if the ratchet command was received //
    if(!strnicmp(ONCLI_PROCEED_CMD_STR, CMD, strlen(ONCLI_PROCEED_CMD_STR)))
    {
        *CMD_STR = ONCLI_PROCEED_CMD_STR;

        if(CMD[strlen(ONCLI_PROCEED_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return proceed_cmd_hdlr();
    } // else if the proceed command was received //
    #endif
    
    else
    {
        *CMD_STR = CMD;
        return ONCLI_INVALID_CMD;
    } // else the command was invalid //
}

   
   
//! @} oncli_hdlr_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup oncli_hdlr_pri_func
//! \ingroup oncli_hdlr
//! @{


#ifdef _ENABLE_ECHO_COMMAND
/*!
    \brief Handles receiving the echo command and it's parameters
    
    The echo command has the form
    
    echo:[ON | OFF]

    \param ASCII_PARAM_LIST ASCII parameter list.

    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
oncli_status_t echo_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    BOOL echo = FALSE;
    UInt8 len;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if(!strnicmp(ONCLI_ON_STR, ASCII_PARAM_LIST, strlen(ONCLI_ON_STR)))
    {
        echo = TRUE;
        len = strlen(ONCLI_ON_STR);
    } // if going to quiet mode //
    else if(!strnicmp(ONCLI_OFF_STR, ASCII_PARAM_LIST, strlen(ONCLI_OFF_STR)))
    {
        echo = FALSE;
        len = strlen(ONCLI_OFF_STR);
    } // else if verbose mode //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else unknown value //

    if(ASCII_PARAM_LIST[len] != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the termination is not correct //

    echo_on = echo;
    return ONCLI_SUCCESS;
} // echo_cmd_hdlr //
#endif


#ifdef _ENABLE_LIST_COMMAND
/*!
    \brief Prints information about the current configuration
    of the device.
    
    \param void
    
    \return ONCLI_SUCCESS if listing the current settings was successful
*/
static oncli_status_t list_cmd_hdlr(void)
{
    oncli_status_t status;
    UInt8 i;
    
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);
      
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        oncli_print_invite();
    }
    #endif
    
    #ifdef _ONE_NET_MULTI_HOP
    oncli_send_msg("# of Network MH Devices : %d\n", num_mh_devices);
    oncli_send_msg("# of Network MH Repeaters : %d\n", num_mh_repeaters);
    #endif
    
    #if defined(_ONE_NET_MASTER) && defined(_ONE_NET_CLIENT)
	if(device_is_master || client_joined_network)
    #elif defined(_ONE_NET_CLIENT)
    if(client_joined_network)
    #endif
	{
        // print encryption key
		oncli_send_msg    ("Message key : ");
	    oncli_print_xtea_key(&(on_base_param->current_key));
        oncli_send_msg("\n\n");
        // print the NID and the DID
        if(oncli_print_sid((on_encoded_sid_t*)(on_base_param->sid)) !=
          ONCLI_SUCCESS)
        {
            return ONCLI_CMD_FAIL;
        }
        
        #ifdef _BLOCK_MESSAGES_ENABLED
        oncli_send_msg("\n\n");
        oncli_print_fragment_delays();
        #endif
	}
    
    oncli_send_msg("\n\nDevice Features...\n");
    oncli_print_features(on_base_param->features);
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master && !client_joined_network)
    {
        oncli_send_msg("\n\nCLIENT : Not Joined\n\n");
        return ONCLI_SUCCESS;
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
    if(device_is_master && (on_state == ON_JOIN_NETWORK ||
      on_state == ON_INIT_STATE))
    {
        oncli_send_msg("\n\nMASTER : Initializing\n\n");
        return ONCLI_SUCCESS;        
    }
    #endif
    
    // print channel
    oncli_send_msg("\n\nChannel: ");
    if((status = oncli_print_channel()) != ONCLI_SUCCESS)
    {
        return status;
    }
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        oncli_send_msg("\n\nSend To Master: %s\n\n", master->flags &
          ON_SEND_TO_MASTER ? TRUE_STR : FALSE_STR);
        oncli_send_msg("\n\nMaster Features...\n\n");
        oncli_print_features(master->device.features);
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        // print the client list
        oncli_send_msg("Client count: %d\n", master_param->client_count);
        for (i = 0; i < master_param->client_count; i++)
        {
            on_client_t* client = &client_list[i];
            oncli_send_msg("\n\n\n  Client %d : ", i + 1);
            oncli_print_did(&(client->device.did));
            oncli_send_msg("\n");
            oncli_send_msg("\n\nSend To Master: %s\n\nFeatures...\n\n",
              client->flags & ON_SEND_TO_MASTER ? TRUE_STR : FALSE_STR);
            oncli_print_features(client->device.features);
        }
    }
    #endif
    
    #ifdef _PEER
    #ifdef _ONE_NET_CLIENT
    if(device_is_master || client_joined_network)
    #endif
    {
        oncli_print_peer_list();
    }
    #endif
    oncli_send_msg("\n\n");
    oncli_print_user_pin_cfg();
    return ONCLI_SUCCESS;
} // list_cmd_hdlr //
#endif


#ifdef _ENABLE_ERASE_COMMAND
/*!
    \brief Erases the settings from non-volatile memory
    
    \param void
    
    \return ONCLI_SUCCESS if erasing the current settings was successful
            ONCLI_CMD_FAIL otherwise
*/
static oncli_status_t erase_cmd_hdlr(void)
{
    #ifdef _ONE_NET_MASTER
    #ifdef _ONE_NET_CLIENT
    if(device_is_master)
    {
        return ((one_net_master_erase_settings() == ONS_SUCCESS) ?
          ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    }
    #else
    return ((one_net_master_erase_settings() == ONS_SUCCESS) ?
      ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    #endif
    #endif
    
    #ifdef _ONE_NET_CLIENT
    return ((one_net_client_erase_settings() == ONS_SUCCESS) ?
      ONCLI_SUCCESS : ONCLI_CMD_FAIL);    
    #endif
}
#endif


#ifdef _ENABLE_SAVE_COMMAND
/*!
    \brief Saves the settings to non-volatile memory
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
            ONCLI_CMD_FAIL otherwise
*/
static oncli_status_t save_cmd_hdlr(void)
{
    #ifdef _ONE_NET_MASTER
    #ifdef _ONE_NET_CLIENT
    if(device_is_master)
    {
        return ((one_net_master_save_settings() == ONS_SUCCESS) ?
          ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    }
    #else
    return ((one_net_master_save_settings() == ONS_SUCCESS) ?
      ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    #endif
    #endif
    
    #ifdef _ONE_NET_CLIENT
    return ((one_net_client_save_settings() == ONS_SUCCESS) ?
      ONCLI_SUCCESS : ONCLI_CMD_FAIL);    
    #endif
}
#endif


/*!
    \brief Puts the device into sniffer mode
    
    The sniff command has the form
    
    sniff:NN
    
    where NN is the channel number to sniff.  The channel is 0 based.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
#if defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)
oncli_status_t sniff_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    oncli_status_t status;

    UInt8 channel;

    if((status = oncli_parse_channel(ASCII_PARAM_LIST, &channel)) !=
      ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the channel was not successful //

    return oncli_reset_sniff(channel);
} // sniff_cmd_hdlr //
#endif


#ifdef _ENABLE_SINGLE_COMMAND
/*!
    \brief Handles receiving the single command and all its parameters.
    
    The single command has the form
    
    single:SRC UNIT:DST UNIT:RAW DST DID:PRIORITY:AABBCCDDEE
    
    where AABBCCDDEE is the packet to send in ASCII hex characters ('0' - '9',
    'A' - 'F' (lower case is valid also)).  Only the parameters (starting with
    SRC UNIT) are passed in.
    
    \param ASCII_PARAM_LIST ASCII parameter list.  The parameters in this list
      should be seperated by ':'.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out
              because the resource is full.
            ONCLI_INVALID_DST If the destination is invalid
            ONCLI_NOT_JOINED If the device needs to join a network before the
              command can be carried out.
            ONCLI_INTERNAL_ERR If something unexpected occured
*/
static oncli_status_t single_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_encoded_did_t enc_dst;
    UInt8 src_unit, dst_unit;
    #ifdef _PEER
    BOOL send_to_peer_list;
    #endif
    UInt16 data_len;
    UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // get the send parameters
    #ifdef _PEER
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &enc_dst, &send_to_peer_list)))
    #else
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &enc_dst)))
    #endif
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // get the data
    data_len = sizeof(raw_pld);
    if(!(PARAM_PTR = parse_ascii_tx_data(PARAM_PTR, raw_pld,
      &data_len)) || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //

    
    switch((*one_net_send_single)(ONE_NET_RAW_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, NULL,
      #ifdef _PEER
          send_to_peer_list ? NULL : enc_dst, send_to_peer_list,  src_unit
      #else
          &enc_dst
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , 0
      #endif    
      ))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        default: return ONCLI_RSRC_UNAVAILABLE;
    }
} // single_cmd_hdlr //


/*!
    \brief Handles receiving the single text command and all it's parameters.
    
    The single text command has the form
    
    single txt:SRC UNIT:DST UNIT:RAW DST DID:PRIORITY:"abc"
    
    where "abc" is the text to send.  Only the parameters (starting with
    SRC UNIT) are passed in.
    
    \param ASCII_PARAM_LIST ASCII parameter list.  The parameters in this list
      should be seperated by ':'.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out
              because the resource is full.
            ONCLI_CMD_FAIL If the command failed.
*/
static oncli_status_t single_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_encoded_did_t enc_dst;
    UInt16 data_len;
    UInt8 src_unit, dst_unit;
    UInt8 raw_pld[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN] = {0};
    #ifdef _PEER
    BOOL send_to_peer_list;
    #endif
    
    UInt8 raw_pid, pld_len, msg_type;
    
    #ifdef _EXTENDED_SINGLE
    UInt8* text_start_ptr = &raw_pld[ONA_MSG_SECOND_IDX];
    #endif

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // get the send parameters
    #ifdef _PEER
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &enc_dst, &send_to_peer_list)))
    #else
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &enc_dst)))
    #endif
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter // 

    // get the data
    data_len = sizeof(raw_pld) - ONA_MSG_DATA_IDX;
    #ifdef _EXTENDED_SINGLE
    data_len = sizeof(raw_pld) - ONA_MSG_DATA_IDX - 1;
    #endif
    
    if(!(PARAM_PTR = parse_ascii_tx_text_data(PARAM_PTR, &raw_pld[ONA_MSG_DATA_IDX],
      &data_len)) || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //
    
    raw_pid = ONE_NET_RAW_SINGLE_DATA;
    pld_len = ONA_SINGLE_PACKET_PAYLOAD_LEN;
    msg_type = ONA_SIMPLE_TEXT;

    #ifdef _EXTENDED_SINGLE
    if(data_len <= ONA_SINGLE_PACKET_PAYLOAD_LEN - ONA_MSG_DATA_IDX)
    {
        // pid, pld_len, msg_type already set.  Do nothing.
    }
    else if(data_len < ONA_LARGE_SINGLE_PACKET_PAYLOAD_LEN -
      ONA_MSG_DATA_IDX -1)
    {
        raw_pid = ONE_NET_RAW_LARGE_SINGLE_DATA;
        pld_len = ONA_LARGE_SINGLE_PACKET_PAYLOAD_LEN;
        msg_type = ONA_TEXT;
        // add a NULL terminator
        text_start_ptr[data_len] = 0;
    }
    else
    {
        raw_pid = ONE_NET_RAW_EXTENDED_SINGLE_DATA;
        pld_len = ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN;
        msg_type = ONA_TEXT;
        // add a NULL terminator
        text_start_ptr[data_len] = 0;
    }
    #endif
    
    // store the message class/message type in the payload
    put_msg_hdr(ONA_COMMAND | msg_type, raw_pld);

    // store the source and destination unit numbers in the payload
    put_dst_unit(dst_unit, raw_pld);
    put_src_unit(src_unit, raw_pld);
    

    switch((*one_net_send_single)(raw_pid, ON_APP_MSG, raw_pld, pld_len,
      ONE_NET_HIGH_PRIORITY, NULL,
      #ifdef _PEER
          send_to_peer_list ? NULL : enc_dst, send_to_peer_list,  src_unit
      #else
          &enc_dst
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , 0
      #endif    
      ))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        default: return ONCLI_RSRC_UNAVAILABLE;
    }
} // single_txt_cmd_hdlr //


static oncli_status_t status_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return parse_and_send_pin_msg(ASCII_PARAM_LIST, ONA_STATUS);
}


static oncli_status_t query_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return parse_and_send_pin_msg(ASCII_PARAM_LIST, ONA_QUERY);
}


static oncli_status_t fast_query_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return parse_and_send_pin_msg(ASCII_PARAM_LIST, ONA_FAST_QUERY);
}


static oncli_status_t set_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return parse_and_send_pin_msg(ASCII_PARAM_LIST, ONA_COMMAND);
}


/*!
    \brief Parses and sends commands, status messages, queries, and fast queries
    
    Valid commands are of the following type...
    
    status pin:DDD:P
    query pin:DDD:P
    fast query pin:DDD:P
    set pin:DDD:P:V
    
    where DDD is the raw DID of the device receiving the message, P is the pin
              number, V is the value to set the pin.
              
    status pin:003:2 sends the status of THIS DEVICE's pin number 2 to device 003
    query pin:003:2 tells device 003 to report back the current state of it pin number 2
    fast query pin:003:2 Same as "query pin:003:2" except this is a "fast query" (i.e.
        device 003 should NOT send back a separate message, but instead only send
        back the status in the response message.
    set pin:003:2:1 commands device device 003 to set pin 2 to state 1(high).  Valid states
        are 0(low), 1(high), or 2(toggle).  This can only be used as if pin 2 of device 003
        is an output pin.
    
    

    \param[in] ASCII_PARAM_LIST ASCII parameter list.
    \param[in] msg_class Themessage class of the outgoing single app message

    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See user_pin for more return values.
*/
static oncli_status_t parse_and_send_pin_msg(
  const char * const ASCII_PARAM_LIST, UInt16 msg_class)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    const char * endptr = NULL;
    on_encoded_did_t enc_dst;
    UInt16 raw_did_int;
    on_raw_did_t raw_dst;
    UInt8 src_unit, dst_unit, unit, pin_value;
    UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];
    
    one_net_memset(raw_pld, 0, sizeof(raw_pld));

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // store the message class/message type in the payload
    put_msg_hdr(msg_class | ONA_SWITCH, raw_pld);
    
    raw_did_int = one_net_strtol(PARAM_PTR, &endptr, 16);
    if(raw_did_int < 0x001 || raw_did_int > 0xFFF || *endptr != ':')
    {
        return ONCLI_PARSE_ERR;
    }
    
    raw_did_int <<= 4;
    raw_dst[0] = (raw_did_int & 0xFF00) >> 8;
    raw_dst[1] = raw_did_int & 0xFF;
    on_encode(enc_dst, raw_dst, ON_ENCODED_DID_LEN);
    
    if(*endptr == '\n')
    {
        return ONCLI_PARSE_ERR;
    }
    
    endptr++;
    PARAM_PTR = endptr;
    
    unit = one_net_strtol(PARAM_PTR, &endptr, 16);
    if(unit >= ONE_NET_NUM_UNITS)
    {
        return ONCLI_PARSE_ERR;
    }
    
    if(msg_class == ONA_COMMAND)
    {
        if(*endptr != ':')
        {
            return ONCLI_PARSE_ERR;
        }
        endptr++;
        PARAM_PTR = endptr;
        
        pin_value = one_net_strtol(PARAM_PTR, &endptr, 10);
        if(pin_value >= ONA_TOGGLE)
        {
            return ONCLI_PARSE_ERR;
        }
    }
    
    if(*endptr != '\n')
    {
        return ONCLI_PARSE_ERR;
    }
    
    src_unit = ONE_NET_DEV_UNIT;
    dst_unit = unit;

    if(msg_class == ONA_STATUS)
    {
        src_unit = unit;
        dst_unit = ONE_NET_DEV_UNIT;
        switch(src_unit)
        {
            case 0: pin_value = USER_PIN0; break;
            case 1: pin_value = USER_PIN1; break;
            case 2: pin_value = USER_PIN2; break;
            case 3: pin_value = USER_PIN3; break;
        }
    }
    
    if(msg_class == ONA_STATUS || msg_class == ONA_COMMAND)
    {
        put_msg_data(pin_value, raw_pld);
    }


    // store the source and destination unit numbers in the payload
    put_dst_unit(dst_unit, raw_pld);
    put_src_unit(src_unit, raw_pld);
      
    switch((*one_net_send_single)(ONE_NET_RAW_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, NULL, &enc_dst
      #ifdef _PEER
          , FALSE,  src_unit
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , 0
      #endif    
      ))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        default: return ONCLI_RSRC_UNAVAILABLE;
    }
}
#endif // _ENABLE_SINGLE_COMMAND //


#ifdef _ENABLE_SET_DATA_RATE_COMMAND
oncli_status_t set_data_rate_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    long int new_data_rate;
    oncli_status_t status;
    const char* endptr = 0;
    
    new_data_rate = one_net_strtol(ASCII_PARAM_LIST, &endptr, 10);
    if(*endptr == 0 || endptr == ASCII_PARAM_LIST || *endptr != '\n')
    {
        return ONCLI_PARSE_ERR;
    }
    
    if(new_data_rate < 0 || new_data_rate >=
      (long int) ONE_NET_DATA_RATE_LIMIT)
    {
        return ONCLI_UNSUPPORTED;
    }
    
    switch(one_net_set_data_rate((UInt8) new_data_rate))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_DEVICE_NOT_CAPABLE: return ONCLI_UNSUPPORTED;
        default: return ONCLI_CMD_FAIL;
    }
}
#endif


/*!
    \brief Handles receiving the user pin command and all it's parameters.

    \param[in] ASCII_PARAM_LIST ASCII parameter list.

    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See user_pin for more return values.
*/
#ifdef _ENABLE_USER_PIN_COMMAND
static oncli_status_t user_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr;

    UInt8 pin;
    UInt8 pin_type;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(!isdigit(*PARAM_PTR))
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //
    
    // read in the pin
    pin = (UInt8) one_net_strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR)
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //
    PARAM_PTR = end_ptr;

    // check the delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // check the action
    if(!strnicmp(PARAM_PTR, ONCLI_INPUT_STR, strlen(ONCLI_INPUT_STR)))
    {
        pin_type = ON_INPUT_PIN;
        PARAM_PTR += strlen(ONCLI_INPUT_STR);
    } // if it should be an input //
    else if(!strnicmp(PARAM_PTR, ONCLI_OUTPUT_STR, strlen(ONCLI_OUTPUT_STR)))
    {
        pin_type = ON_OUTPUT_PIN;
        PARAM_PTR += strlen(ONCLI_OUTPUT_STR);
    } // else if it should be an output //
    else if(!strnicmp(PARAM_PTR, ONCLI_DISABLE_STR, strlen(ONCLI_DISABLE_STR)))
    {
        pin_type = ON_DISABLE_PIN;
        PARAM_PTR += strlen(ONCLI_DISABLE_STR);
    } // else if it should be an output //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else if the priority is invalid //

    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //
    
    return oncli_set_user_pin_type(pin, pin_type);
} // user_pin_cmd_hdlr //
#endif


// temporary -- debugging
#include "one_net_master.h"
#include "one_net_prand.h"
static oncli_status_t add_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_raw_did_t did;
    on_features_t features;
    on_base_param_t out_base_param;
    on_master_t out_master_param;


    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(!device_is_master)
    {
        // read in the did
        if(ascii_hex_to_byte_stream(PARAM_PTR, did, ONCLI_ASCII_RAW_DID_SIZE)
          != ONCLI_ASCII_RAW_DID_SIZE)
        {
            return ONCLI_PARSE_ERR;
        } // if converting the source peer did failed //
        PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    }
    else
    {
        // read in features
        if(ascii_hex_to_byte_stream(PARAM_PTR, (UInt8*) &features,
          2 * sizeof(on_features_t)) != 2 * sizeof(on_features_t))
        {
            return ONCLI_PARSE_ERR;
        } // if converting the features failed //    
    
        PARAM_PTR += (2 * sizeof(on_features_t));
    }
    
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        if(one_net_master_add_client(features, &out_base_param,
          &out_master_param) != ONS_SUCCESS)
        {
            return ONCLI_CMD_FAIL;
        }        
    }
    #endif
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        on_encode(&(on_base_param->sid[ON_ENCODED_NID_LEN]), did,
          ON_ENCODED_DID_LEN);
        on_encode(on_base_param->sid, add_nid, ON_ENCODED_NID_LEN);
        master->device.features = add_master_features;
        master->device.expected_nonce = one_net_prand(get_tick_count(), ON_MAX_NONCE);
        master->device.last_nonce = one_net_prand(get_tick_count(), ON_MAX_NONCE);
        master->device.send_nonce = one_net_prand(get_tick_count(), ON_MAX_NONCE);
        master->device.msg_id = one_net_prand(get_tick_count(), ON_MAX_MSG_ID);
        one_net_memmove(master->device.did, MASTER_ENCODED_DID,
          ON_ENCODED_DID_LEN);
    #ifdef _ONE_NET_MULTI_HOP
        master->device.max_hops = features_max_hops(add_master_features);       
        master->device.hops = 0;
    #endif
        one_net_memmove(on_base_param->current_key, add_key,
          sizeof(one_net_xtea_key_t));
        master->keep_alive_interval = add_keep_alive;
        master->flags = add_flags;
        on_base_param->single_block_encrypt = add_single_block_encrypt;
        on_base_param->channel = add_channel;
    #ifdef _STREAM_MESSAGES_ENABLED
        on_base_param->stream_encrypt = add_stream_encrypt;
    #endif
    #ifdef _BLOCK_MESSAGES_ENABLED
        on_base_param->fragment_delay_low = add_fragment_delay_low;
        on_base_param->fragment_delay_high = add_fragment_delay_high;
    #endif
    
        on_state = ON_LISTEN_FOR_DATA;
        client_joined_network = TRUE;
        client_looking_for_invite = FALSE;
    }
    #endif

    return ONCLI_SUCCESS;
}



#ifdef _ENABLE_INVITE_COMMAND
/*!
    \brief Handles receiving the invite command and all its parameters.
    
    The invite command has the form
    
    invite:AAAA-BBBB:TTTT
    
    where AAAA-BBBB is the unique invite key for the CLIENT to invite
    and TTTT (note : TTTT is not necessarily exactly 4 characters) is an
    optional timeout time
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
*/
static oncli_status_t invite_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    oncli_status_t status;
    char* ptr;
    char* end_ptr;
    long int timeout = ONE_NET_MASTER_INVITE_DURATION;
    one_net_xtea_key_t invite_key;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = parse_invite_key(ASCII_PARAM_LIST, &end_ptr,
      &invite_key)) != ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the key was not successful //
    
    if(*end_ptr == ONCLI_PARAM_DELIMITER)
    {
        ptr = end_ptr + 1;
        timeout = one_net_strtol(ptr, &end_ptr, 10);
        if(timeout <= 0)
        {
            return ONCLI_BAD_PARAM;
        }
    }
    
    if(*end_ptr != '\n')
    {
        return ONCLI_PARSE_ERR;
    }

    switch(one_net_master_invite(&invite_key, (UInt32) timeout))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_DEVICE_LIMIT: return ONCLI_RSRC_UNAVAILABLE;
        case ONS_ALREADY_IN_PROGRESS: return ONCLI_ALREADY_IN_PROGRESS;
        case ONS_NOT_INIT: return ONCLI_ONS_NOT_INIT_ERR;
        case ONS_BAD_PARAM: return ONCLI_BAD_PARAM;
        case ONS_INTERNAL_ERR: return ONCLI_INTERNAL_ERR;
        default: return ONCLI_CMD_FAIL;
    }
}
#endif


#ifdef _ENABLE_CANCEL_INVITE_COMMAND
extern one_net_xtea_key_t invite_key;
static oncli_status_t cancel_invite_cmd_hdlr(void)
{
    one_net_master_invite_result(ONS_CANCELED, &invite_key, 0);
    one_net_master_cancel_invite(&invite_key);
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
/*!
    \brief Handles receiving the remove device command and all its parameters.
    
    \param[in] ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_ALREADY_IN_PROGRESS if the system is already in the middle
                of a device deletion.
            ONCLI_RSRC_UNAVAILABLE is the maste is currently processing a
                transaction.
            ONCLI_BAD_PARAM If any of the parameters passed into this functtion
              are invalid.
            ONCLI_INVALID_DST If the device that we are attempting to remove is
              not part of the network
*/
static oncli_status_t rm_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_raw_did_t dst;
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    }
    #endif

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // read in the did
    if(ascii_hex_to_byte_stream(PARAM_PTR, dst, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw peer did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;

    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //

    switch(one_net_master_remove_device(&dst))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_ALREADY_IN_PROGRESS: return ONCLI_ALREADY_IN_PROGRESS;
        case ONS_BUSY: return ONCLI_RSRC_UNAVAILABLE;
        case ONS_INCORRECT_ADDR: return ONCLI_INVALID_DST;
        default: return ONCLI_CMD_FAIL;
    }
} // rm_dev_cmd_hdlr //
#endif


#if defined(_ENABLE_ASSIGN_PEER_COMMAND) || defined(_ENABLE_UNASSIGN_PEER_COMMAND)
/*!
    \brief Changes a peer list.
    
    The command has the form "aaaaaaa:bbb:c:ddd:e"
    
    where aaaaaaa is either "assign peer" or "unassign peer"
    
    Note that "unassign peer" has "wildcards".  "assign peer" does not.
    
    where bbb is the three digit raw did of the source.
    where c is the source unit ("f" is a wildcard)
    where ddd is the three digit raw did of the peer ("000" is a wildcard).
    where e is the peer unit ("f" is a wildcard).

    
    \param [in] ASSIGN.  If true, we are adding a peer.  If false, we are
                         deleting a peer.
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONS_RSRC_FULL If the peer assignment is for the master and there
              is no room on the list.
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONS_INCORRECT_ADDR If the either address is undecipherable or not
              part of the network or both addresses are the same.
            ONCLI_INVALID_CMD_FOR_NODE If a client attempts to execute this
              master-only command.
            ONCLI_CMD_FAIL For any other failure
*/
static oncli_change_peer_list(BOOL ASSIGN,
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    on_raw_did_t peer_did, src_did;
    on_encoded_did_t enc_peer_did, enc_src_did;
    
    UInt8 peer_unit, src_unit;
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    }
    #endif
    

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    // read in the source did
    if(ascii_hex_to_byte_stream(PARAM_PTR, src_did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the source peer did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the source unit
    src_unit = one_net_strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    PARAM_PTR = end_ptr;
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // read in the peer did
    if(ascii_hex_to_byte_stream(PARAM_PTR, peer_did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the peer destination did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;

    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the peer unit
    peer_unit = one_net_strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    

    // we're the master and we want to assign a peer to a client
    switch(one_net_master_peer_assignment(ASSIGN, &src_did, src_unit,
        &peer_did, peer_unit))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_RSRC_FULL: return ONCLI_RSRC_UNAVAILABLE;
        case ONS_INCORRECT_ADDR: return ONCLI_INVALID_DST;
        default: return ONCLI_CMD_FAIL;
    }
}
#endif


#ifdef _ENABLE_ASSIGN_PEER_COMMAND
/*!
    \brief Adds a peer to a peer list of a device
    
    The command has the form "assign peer:bbb:c:ddd:e"

    
    where bbb is the three digit raw did of the source.
    where c is the source unit
    where ddd is the three digit raw did of the peer
    where e is the peer unit


    \param [in] ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONS_RSRC_FULL If the peer assignment is for the master and there
              is no room on the list.
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONS_INCORRECT_ADDR If the either address is undecipherable or not
              part of the network or both addresses are the same.
            ONCLI_INVALID_CMD_FOR_NODE If a client attempts to execute this
              master-only command.
            ONCLI_CMD_FAIL For any other failure
*/
static oncli_status_t assign_peer_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return oncli_change_peer_list(TRUE, ASCII_PARAM_LIST);
}
#endif


#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
/*!
    \brief Removes peer(s) from a peer list
    
    The command has the form "unassign peer:bbb:c:ddd:e"
    
    where bbb is the three digit raw did of the source.
    where c is the source unit ("f" is a wildcard)
    where ddd is the three digit raw did of the peer ("000" is a wildcard).
    where e is the peer unit ("f" is a wildcard).
    
    
    Wildcards mean that all existing peers that match will be deleted.
    
    For example, "unassign peer:003:f:000:5" will delete all peers in device
    003 with any source unit, any peer device, and a peer unit of 5.
    

    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONS_RSRC_FULL If the peer assignment is for the master and there
              is no room on the list.
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONS_INCORRECT_ADDR If the either address is undecipherable or not
              part of the network or both addresses are the same.
            ONCLI_INVALID_CMD_FOR_NODE If a client attempts to execute this
              master-only command.
            ONCLI_CMD_FAIL For any other failure
*/
static oncli_status_t unassign_peer_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    return oncli_change_peer_list(FALSE, ASCII_PARAM_LIST);
}
#endif


#ifdef _ENABLE_JOIN_COMMAND
/*!
    \brief Handles receiving the join command and all its parameters
    
    The join command has the form
    
    join:TTT:LL:NN
	
	TTT = timeout (in milliseconds)
	LL  = Locale (US or Europe)
	NN  = Channel number within locale
    
    If no parameters, all channels on all locales are scanned, no timeout.
	If no timeout is specified, there is no timeout.
	If a locale is specified, but not channel number, all channels for that
	locale are searched.
	If a locale and a channel are specified, only that channel is listened to.
	
	Example: join:30000:US:5
	will look for an invite on US Channel 5 for 30 seconds
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
oncli_status_t join_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
	oncli_status_t status;
	one_net_xtea_key_t* invite_key;
	UInt16 timeout = 0;
    UInt8 low_channel = 0;
	UInt8 high_channel = ONE_NET_MAX_CHANNEL;
    const char * PARAM_PTR = ASCII_PARAM_LIST;
	const char* END_PTR = 0;
	
    // get the unique invite code for this device
    invite_key = (one_net_xtea_key_t *) one_net_client_get_invite_key();

	if(*PARAM_PTR == '\n')
    {
		return (one_net_client_reset_client(one_net_client_get_invite_key()) ==
          ONS_SUCCESS) ? ONCLI_SUCCESS : ONCLI_CMD_FAIL;
	}
	 
    // Get the timeout if it's there
    if(isdigit(*PARAM_PTR))
    {
        timeout = one_net_strtol(PARAM_PTR, &END_PTR, 0);
	    PARAM_PTR = END_PTR;
        if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the command isn't formatted properly //
        PARAM_PTR++;
    } // get the timeout time
	
	if(*PARAM_PTR != '\n')
    {
		if((status = oncli_parse_channel(PARAM_PTR, &low_channel)) != ONCLI_SUCCESS)
        {
            return status;
        } // if parsing the channel was not successful //
		
		high_channel = low_channel;
	}

#ifdef _STREAM_MESSAGES_ENABLED
    if(one_net_client_look_for_invite(invite_key,
      ONE_NET_STREAM_ENCRYPT_XTEA8, ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
      low_channel, high_channel, timeout) !=
	    ONS_SUCCESS)
#else
    if(one_net_client_look_for_invite(invite_key,
      ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32, low_channel, high_channel, timeout)
      != ONS_SUCCESS)
#endif
	{
		return ONCLI_INTERNAL_ERR;
	}
	
	return ONCLI_SUCCESS;
} // join_cmd_hdlr //
#endif


#ifdef _ENABLE_UPDATE_MASTER_COMMAND
/*!
    \brief Sets the update MASTER flag in a CLIENT.

    The update_master_cmd_hdlr command has the form

    set update master flag:did:command

    where command is either "set" or "clear".  This is a MASTER only function.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See set_update_master for more return values.
*/
static oncli_status_t update_master_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_raw_did_t dst;
    BOOL update_master = FALSE;
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    }
    #endif

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // read in the peer did
    if(ascii_hex_to_byte_stream(PARAM_PTR, dst, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw peer did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // get the flag
    if(!strnicmp(ONCLI_SET_STR, PARAM_PTR, strlen(ONCLI_SET_STR)))
    {
        PARAM_PTR += strlen(ONCLI_SET_STR);
        update_master = TRUE;
    } // if updating the MASTER //
    else if(!strnicmp(ONCLI_CLR_STR, PARAM_PTR, strlen(ONCLI_CLR_STR)))
    {
        PARAM_PTR += strlen(ONCLI_CLR_STR);
        update_master = FALSE;
    } // else if not updating the MASTER //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else invalid parameter //

    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //
    
    
    switch(one_net_master_set_update_master_flag(update_master, &dst))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
        } // success case //

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
        } // incorrect address case //

        default:
        {
            return ONCLI_CMD_FAIL;
        } // default case //
    } // switch(one_net_master_set_update_master_flag) //
}
#endif


#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
static oncli_status_t change_keep_alive_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr = 0;
    on_raw_did_t dst;
    UInt32 keep_alive;
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    }
    #endif

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // read in the peer did
    if(ascii_hex_to_byte_stream(PARAM_PTR, dst, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw peer did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER || !isdigit(*PARAM_PTR))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // get the value
    keep_alive = one_net_strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //


    switch(one_net_master_change_client_keep_alive(&dst, keep_alive))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
        } // success case //

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
        } // incorrect address case //

        default:
        {
            return ONCLI_CMD_FAIL;
        } // default case //
    } // switch(one_net_master_change_client_keep_alive) //
}
#endif


#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
/*!
    \brief Handles receiving the change fragment delay command and all its
      parameters.
    
    The change fragment delay command has the form
    
    change fragment delay:did:priority:time_ms
    
    
    For example, "Change fragment delay:003:low:100" will change the low
    priority fragment delay of device 003 to 100 ms.
    
    This is a MASTER only command.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
              
            See below for other errors
            
*/
static oncli_status_t change_frag_dly_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr;
    on_raw_did_t did;
    UInt16 delay;
    UInt8 priority;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    }
    #endif
    
    // read in the peer did
    if(ascii_hex_to_byte_stream(PARAM_PTR, did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // read in the priority
    if(!strnicmp(PARAM_PTR, ONCLI_LOW_STR, strlen(ONCLI_LOW_STR)))
    {
        priority = ONE_NET_LOW_PRIORITY;
        PARAM_PTR += strlen(ONCLI_LOW_STR);
    } // if it's low priority //
    else if(!strnicmp(PARAM_PTR, ONCLI_HIGH_STR, strlen(ONCLI_HIGH_STR)))
    {
        priority = ONE_NET_HIGH_PRIORITY;
        PARAM_PTR += strlen(ONCLI_HIGH_STR);
    } // else if it's high priority
    else
    {
        return ONCLI_PARSE_ERR;
    } // else the priority is invalid //
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER || !isdigit(*PARAM_PTR))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // read in delay
    delay = one_net_strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //
    
    switch(one_net_master_change_frag_dly(&did,
      priority == ONE_NET_LOW_PRIORITY ? delay : 0,
      priority == ONE_NET_HIGH_PRIORITY ? delay : 0))
    {
        case ONS_DEVICE_NOT_CAPABLE: return ONCLI_UNSUPPORTED;
        case ONS_RSRC_FULL: return ONCLI_RSRC_UNAVAILABLE;
        case ONS_INCORRECT_ADDR: return ONCLI_INVALID_DST;
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        default: return ONCLI_CMD_FAIL;
    }
} // change_frag_dly_cmd_hdlr //
#endif


#ifdef _ENABLE_CHANGE_KEY_COMMAND
static oncli_status_t change_single_block_key_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    enum
    {
        // number of characters in the key that are grouped together.
        KEY_CH_GROUP_SIZE = 2,

        // The position where the group delimiter character occurs (every nth).
        // This is 1 more than the size of the grouping.
        KEY_DELIMITER_POS,

        // number of ascii characters to convert to a binary stream
        NUM_CH_TO_CONVERT = 2
    };
    
    one_net_status_t status;

    // delimits the unique key into 4 sets of 4 values.
    const char GROUP_DELIMITER = '-';

    one_net_xtea_key_fragment_t key_fragment;

    UInt8 param_idx, key_idx;
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ONCLI_INVALID_CMD_FOR_NODE; // master-only command
    }
    #endif

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // parse the key parameter
    param_idx = 0;
    key_idx = 0;
    while(param_idx < 11)
    {
        if((param_idx % KEY_DELIMITER_POS) == KEY_CH_GROUP_SIZE)
        {
            if(ASCII_PARAM_LIST[param_idx] != GROUP_DELIMITER)
            {
                return ONCLI_PARSE_ERR;
            } // if the character is not the delimiter //
            
            param_idx++;
        } // if the character should be the delimiter //
        else
        {
            if(ascii_hex_to_byte_stream(&(ASCII_PARAM_LIST[param_idx]),
              &(key_fragment[key_idx++]), NUM_CH_TO_CONVERT)
              != NUM_CH_TO_CONVERT)
            {
                return ONCLI_PARSE_ERR;
            } // if the character is invalid //
            
             param_idx += NUM_CH_TO_CONVERT;
        } // else it should be a key character //
    } // loop to read in the unique key //

    status = one_net_master_change_key_fragment(key_fragment);
    switch(status)
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_ALREADY_IN_PROGRESS: return ONCLI_ALREADY_IN_PROGRESS;
        case ONS_BAD_KEY_FRAGMENT: return ONCLI_BAD_KEY_FRAGMENT;
        default: return ONCLI_CMD_FAIL;
    }
}
#endif


#ifdef _ENABLE_CHANNEL_COMMAND
/*!
    \brief Handles receiving the channel command and all it's parameters
    
    The channel command has the form
    
    channel:LL:NN
    
    where LL is the locale("US" or "EUR" and NN is the channel number to change to.
    The channel is 1 based(1-25 for US and 1-3 for Europe).  This command is only
    valid in MASTER mode.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
static oncli_status_t channel_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    oncli_status_t status;
    UInt8 channel;

    if((status = oncli_parse_channel(ASCII_PARAM_LIST, &channel)) !=
      ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the channel was not successful //
    
    one_net_reset_master_with_channel(channel);
    return ONCLI_SUCCESS;
} // channel_cmd_hdlr //
#endif


#ifdef _ENABLE_SETNI_COMMAND
/*!
    \brief Handles receiving the setni command and all its parameters.
    
    The setni command has the form
    
    setni:123456789:GGGG-HHHH
    
    where 123456789 is a valid NID (one of the NID's allocated to ONE-NET evaluation boards).  

    where GGGG-HHHH is an invite code. It will be repeated to produce the 
    full invite code (GGGG-HHHH-GGGG-HHHH).

    The manufacturing data segment in data flash will contain a full SID (where the
    master DID is appended to the NID).
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
oncli_status_t setni_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{

    enum
    {
        SETNI_CMD_NID_OFFSET = 0,
        SETNI_CMD_NID_NIBBLES = 9,
        SETNI_CMD_SID_NIBBLES = 12,
        SETNI_CMD_SEPARATOR_OFFSET = 9,
        SETNI_CMD_INVITE_OFFSET = 10,
        SETNI_CMD_INVITE_SEP_OFFSET = 14,
        SETNI_CMD_INVITE_INPUT_LENGTH = 9,
        SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH = 4,
        // total parameter length is SID + delimiter + invite + newline
        SETNI_CMD_TOTAL_LENGTH = SETNI_CMD_NID_NIBBLES+1+SETNI_CMD_INVITE_INPUT_LENGTH+1
    };

    char * end_ptr = 0;

    UInt8 sid;
    BOOL set_sid_and_invite;
    UInt8 * ptr_segment;
    UInt8 * ptr_invite;
    UInt8 i;
    UInt8 mfg_data_segment[(SETNI_CMD_SID_NIBBLES/2)+SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH*4];

    // delimits the starting address and length
    const char SETNI_CMD_SEPARATOR = ':';
    const char SID_INVITE_SEPARATOR = '-';

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    else if (strlen(ASCII_PARAM_LIST) != SETNI_CMD_TOTAL_LENGTH)
    {
        return ONCLI_PARSE_ERR;
    }
    else if (*(ASCII_PARAM_LIST+SETNI_CMD_SEPARATOR_OFFSET) != SETNI_CMD_SEPARATOR)
    {
        return ONCLI_PARSE_ERR;
    }
    else if (*(ASCII_PARAM_LIST+SETNI_CMD_INVITE_SEP_OFFSET) != SID_INVITE_SEPARATOR)
    {
        return ONCLI_PARSE_ERR;
    }

    //
    // make sure there is no manufacturing data segment in data flash
    // this command can only be executed once. if you want to change the
    // SID and invite code, you need to erase data flash using some means
    // other than the CLI erase command. for example, using a Renesas HEW.
    //
    ptr_segment = dfi_find_last_segment_of_type(DFI_ST_DEVICE_MFG_DATA);
    if ((UInt8 *) ptr_segment != (UInt8 *) 0)
    {
        //
        // we found manufacturing data, so issue error and
        // do not complete the command.
        //
        return ONCLI_UNSUPPORTED;
    }

    //
    // extract the NID from the parameters, convert the NID to binary,
    // extend it to become an SID, and copy it to the manufacturing data segment buffer.
    //
    if(ascii_hex_to_byte_stream(ASCII_PARAM_LIST+SETNI_CMD_NID_OFFSET, &mfg_data_segment[0],
      SETNI_CMD_NID_NIBBLES) !=SETNI_CMD_NID_NIBBLES)
    {
        return ONCLI_PARSE_ERR;
    }

    // a raw SID is 6 bytes (nn nn nn nn nd dd), the setni provides the 9 nibble raw NID
    // (nn nn nn nn n). we need to set the d dd portion of the rae SID to 0 01 so the NID
    // supplied becomes a raw SID of nn nn nn nn n0 01 as we save it to flash,
    // since we always want the master DID to be 001.
    mfg_data_segment[ON_RAW_NID_LEN] &= 0xf0;
    mfg_data_segment[ON_RAW_NID_LEN] = 0x01;

    //
    // copy the invite code from the parameter list to the manufacturing data buffer.
    //
    one_net_memmove(&mfg_data_segment[SETNI_CMD_SID_NIBBLES/2],
      ASCII_PARAM_LIST+SETNI_CMD_INVITE_OFFSET, SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH);
    one_net_memmove(&mfg_data_segment[(SETNI_CMD_SID_NIBBLES/2)+SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH],
      ASCII_PARAM_LIST+SETNI_CMD_INVITE_SEP_OFFSET+1, SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH);
    
    // 
    // duplicate the 8 byte invite code supplied to give a full 16 byte invite code
    //
    one_net_memmove(&mfg_data_segment[(SETNI_CMD_SID_NIBBLES/2)+(SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH*2)],
      &mfg_data_segment[SETNI_CMD_SID_NIBBLES/2], SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH*2);

    //
    // verify that the invite key characters supplied are valid for an invite key
    //
    ptr_invite=&mfg_data_segment[SETNI_CMD_SID_NIBBLES/2];
    for (i=0; i<SETNI_CMD_INVITE_KEY_SEGMENT_LENGTH*2; i++)
    {
        if(!oncli_is_valid_unique_key_ch(ptr_invite[i]))
        {
            return ONCLI_PARSE_ERR;
        }
    }
    
    //
    // write the manufacturing data segment buffer to data flash
    //
    if (dfi_write_segment_of_type(DFI_ST_DEVICE_MFG_DATA, &mfg_data_segment[0],
      sizeof(mfg_data_segment)) == (UInt8 *) 0)
    {
        return ONCLI_CMD_FAIL;
    }
    
    oncli_send_msg("Erasing current settings and resetting the system with "
      "a new NID and invite key.\n");
    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        one_net_master_erase_settings();
        one_net_master_reset_master(one_net_master_get_raw_sid());
    }
    #endif
    
    // TODO -- what about resetting the client?
    return ONCLI_SUCCESS;
} // setni_cmd_hdlr //
#endif


#ifdef _RANGE_TESTING
/*!
    \brief Turns on or off range testing for debugging purposes.  Sets a
           device as either within range or out of range of this device
    
    The "range test" command has the form
    
    range test:on  --> turns range testing on
    range test:off  --> turns range testing off
    range test:clear --> makes all devices out of range
    range test:display --> Displays in-range DIDs.
    range test:add:003 --> places device with raw DID 003 within range
    range test:remove:003 --> places device with raw DID 003 out of range
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
static oncli_status_t range_test_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    on_raw_did_t did;
    on_encoded_did_t enc_did;
    BOOL turn_on = FALSE;
    BOOL turn_off = FALSE;
    BOOL clear = FALSE;
    BOOL display = FALSE;
    BOOL add = FALSE;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(!strnicmp(PARAM_PTR, ONCLI_ON_STR, strlen(ONCLI_ON_STR)))
    {
        PARAM_PTR += strlen(ONCLI_ON_STR);
        turn_on = TRUE;
    }
    else if(!strnicmp(PARAM_PTR, ONCLI_OFF_STR, strlen(ONCLI_OFF_STR)))
    {
        PARAM_PTR += strlen(ONCLI_OFF_STR);
        turn_off = TRUE;
    }
    else if(!strnicmp(PARAM_PTR, CLEAR_STR, strlen(CLEAR_STR)))
    {
        PARAM_PTR += strlen(CLEAR_STR);
        clear = TRUE;
    }
    else if(!strnicmp(PARAM_PTR, DISPLAY_STR, strlen(DISPLAY_STR)))
    {
        PARAM_PTR += strlen(DISPLAY_STR);
        display = TRUE;
    }
    else
    {
        if(!strnicmp(PARAM_PTR, ADD_STR, strlen(ADD_STR)))
        {
            add = TRUE;
            PARAM_PTR += strlen(ADD_STR);
        }
        else if(!strnicmp(PARAM_PTR, REMOVE_STR, strlen(REMOVE_STR)))
        {
            PARAM_PTR += strlen(REMOVE_STR);
        }
        else
        {
            return ONCLI_PARSE_ERR;
        }
        
        if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        }
        PARAM_PTR++;
        
        // read in the did
        if(ascii_hex_to_byte_stream(PARAM_PTR, did, ONCLI_ASCII_RAW_DID_SIZE)
          != ONCLI_ASCII_RAW_DID_SIZE)
        {
            return ONCLI_PARSE_ERR;
        } // if converting the source peer did failed //
        PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
        
        if(on_encode(enc_did, did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
        {
            return ONCLI_PARSE_ERR;
        }
    }
    
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    }
    
    if(turn_on || turn_off)
    {
        enable_range_testing(turn_on);
        return ONCLI_SUCCESS;
    }
    else if(clear)
    {
        reset_range_test_did_array();
        return ONCLI_SUCCESS;
    }
    else if(display)
    {
        UInt8 i;
        on_encoded_did_t enc_did_array[RANGE_TESTING_ARRAY_SIZE];
        UInt8 num_in_range = RANGE_TESTING_ARRAY_SIZE;
        if(!devices_within_range(enc_did_array, &num_in_range))
        {
            oncli_send_msg("In-Range Device List Unretrievable.\n");
            return ONCLI_CMD_FAIL;
        }
        
        oncli_send_msg("# of In-Range Devices : %d\n", num_in_range);
        for(i = 0; i < num_in_range; i++)
        {
            oncli_send_msg("Encoded DID : %02X%02X -- Raw ",
              enc_did_array[i][0], enc_did_array[i][1]);
            oncli_print_did((on_encoded_did_t*) enc_did_array[i]);
            oncli_send_msg("\n");
        }
        return ONCLI_SUCCESS;
    }
    
    
    if(adjust_range_test_did_array(&enc_did, add))
    {
        return ONCLI_SUCCESS;
    }
    
    return ONCLI_CMD_FAIL;
} // range_test_cmd_hdlr //
#endif


#ifdef _ONE_NET_MULTI_HOP
/*!
    \brief Sets the num_mh_devices and num_mh_repeaters counts manually
    
    This function is useful mostly for debugging multi-hop applications.
    Multi-hop works only when num_mh_repeat is a positive number.  Setting
    these variables, particularly the num_mh_repeat variable, can serve to
    
    1) Quickly turn multi-hop on and off.
    2) Correct these variable values if they become incorrect due to either
       a non-ONE-NET wireless invite or deletion process (i.e. manual adding
       or deleting a device through the command line or otherwise).
    3) Or due to losing these values due to power cycling.
    4) Or due to any other experiment or bug that may cause these values to
       become incorrect.

    \param[in] ASCII_PARAM_LIST String containing the memory to load

    \return ONCLI_SUCCESS if the parsing of the string worked and the parsed
                values were valid.
            ONCLI_PARSE_ERR if the string could not be parsed.
            ONCLI_BAD_PARAM if the string parsed correctly, but the parsed
                values were invalid.
*/
static oncli_status_t mh_repeat_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    const char* endptr = 0;
    int num_mh = 0;
    int num_mh_repeat = 0;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    num_mh = one_net_strtol(PARAM_PTR, &endptr, 10);
    if(*endptr == 0 || endptr == PARAM_PTR || *endptr != ':')
    {
        return ONCLI_PARSE_ERR;
    }    
    endptr++;
    PARAM_PTR = endptr;
    num_mh_repeat = one_net_strtol(PARAM_PTR, &endptr, 10);
    if(*endptr == 0 || endptr == PARAM_PTR || *endptr != '\n')
    {
        return ONCLI_PARSE_ERR;
    }
    
    if(num_mh_repeat > num_mh)
    {
        // all multi-hop repeaters are multi-hop, so this is an impossible
        // scenario.
        return ONCLI_BAD_PARAM;
    }

    num_mh_devices = num_mh;
    num_mh_repeaters = num_mh_repeat;
    return ONCLI_SUCCESS;
}
#endif





#ifdef _DEBUGGING_TOOLS
/*!
    \brief Loads memory into the stored memory location.
    
    This function takes a hex string representing bytes, converts it to a
    byte stream, then copies it into the stored memory location.

    \param[in] ASCII_PARAM_LIST String containing the memory to load

    \return ONCLI_SUCCESS if the parsing and copying of the string succeeded.
            ONCLI_CDM_FAIL otherwise
*/
static oncli_status_t memload_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    enum
    {
        MAX_LOAD_LEN = 50,
        MAX_ASCII_CHAR = MAX_LOAD_LEN * 2
    };
    
    UInt8 byte_stream[MAX_LOAD_LEN];
    UInt16 num_ascii_char = 0;
    UInt16 actual_len = 0;
    UInt16 num_convert;
    UInt16 max_len = MAX_LOAD_LEN > memory_len ? memory_len : MAX_LOAD_LEN;

    const char* ptr = ASCII_PARAM_LIST;
    while(num_ascii_char < MAX_ASCII_CHAR && *ptr != '\n')
    {
        num_ascii_char++;
        ptr++;
    }

    num_convert = ascii_hex_to_byte_stream(ASCII_PARAM_LIST, byte_stream,
        num_ascii_char);
        
    actual_len = max_len >= (num_convert / 2) ? (num_convert / 2) : max_len;
        
    if(actual_len == 0)
    {
        return ONCLI_PARSE_ERR;
    }
        
    one_net_memmove(memory_ptr, byte_stream, actual_len);
    oncli_send_msg("Copied %d bytes.\n", actual_len);
    return ONCLI_SUCCESS;
} // memload_cmd_hdlr //


/*!
    \brief Similar to memset in the C specification
    
    memset:18:A6 will store 0xA6 into the stored memory location.  For
         example, if the stored memory location was 0x5670, after this
         command, addresses 0x5670 through 0x5681 would contain the value
         0xA6.

    \param[in] ASCII_PARAM_LIST String containing the number of bytes to
         set and the value to set them to.

    \return ONCLI_SUCCESS if the parsing and setting of the value succeeded.
            ONCLI_CDM_FAIL otherwise
*/
static oncli_status_t memset_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    char* ptr = (char*) ASCII_PARAM_LIST;
    char* end_ptr;
    int num_bytes = 0;
    UInt8 fill_byte_digit, fill_byte;
    UInt16 i;
    
    num_bytes = one_net_strtol(ptr, &end_ptr, 0);
    if(memory_ptr == NULL || !end_ptr || end_ptr == ptr || num_bytes == 0)
    {
        return ONCLI_PARSE_ERR; // error
    } // if parsing the data failed //
    ptr = end_ptr;
    
    if(*ptr != ':')
    {
        return ONCLI_PARSE_ERR; // error
    }
    ptr++;

    fill_byte_digit = ascii_hex_to_nibble(*ptr);
    if(fill_byte_digit > 0x0F)
    {
       return ONCLI_PARSE_ERR;
    } // if the conversion failed //
    ptr++;
    fill_byte = fill_byte_digit << 4;
    
    fill_byte_digit = ascii_hex_to_nibble(*ptr);
    if(fill_byte_digit > 0x0F)
    {
       return ONCLI_PARSE_ERR;
    } // if the conversion failed //
    
    fill_byte |= fill_byte_digit;
    
    ptr = memory_ptr;
    for(i = 0; i < num_bytes; i++)
    {
        *ptr = fill_byte;
        ptr++;
    }
    
    return ONCLI_SUCCESS;
} // memset_cmd_hdlr //


/*!
    \brief Displays the contents of the stored memory location in hex.

    \return ONCLI_SUCCESS if there is a valid stored memory location
            ONCLI_CMD_FAIL otherwise.
*/
static oncli_status_t memdump_cmd_hdlr(void)
{
    if(memory_ptr == NULL || memory_len <= 0)
    {
        return ONCLI_CMD_FAIL;
    }
    
    oncli_send_msg("Ratchet = %s\n", ratchet ? ONCLI_ON_STR : ONCLI_OFF_STR);
    oncli_send_msg("Proceed = %s\n", proceed ? ONCLI_ON_STR : ONCLI_OFF_STR);
    oncli_send_msg("Pause = %s\n", pause ? ONCLI_ON_STR : ONCLI_OFF_STR);
    oncli_send_msg("Pausing = %s\n", pausing ? ONCLI_ON_STR : ONCLI_OFF_STR);
    oncli_send_msg("\n\n");
    delay_ms(25);   
    print_timers();
    delay_ms(25);   
    print_intervals();
    oncli_send_msg("\n\n");
    delay_ms(25);   

    xdump(memory_ptr, memory_len);
    return ONCLI_SUCCESS;
}


/*!
    \brief Returns a certain segment of memory given parameeters.


    \param[out] mem_ptr The address of the memory requested.
    \param[in] memory_type The type of mempory requested.
    \param[in] index If the memory type is an array, the element requested
    \param[in] offset The offset(in bytes) requested from the start of the
                 memory of element in question.  Must be non-negative.

    \return the number of bytes of relevant memory.  -1 upon error
*/
static int get_memory_loc(UInt8** mem_ptr, debug_memory_t memory_type,
  int index, int offset)
{
    int len = 0;
    
    if(!mem_ptr)
    {
        return 0;
    }
    
    *mem_ptr = NULL;
    
    switch(memory_type)
    {
        case DEBUG_MEMORY_ON_STATE:
            *mem_ptr = (UInt8*) &on_state;
            len = sizeof(on_state_t);
            break;
        case DEBUG_MEMORY_TIMER:
            *mem_ptr = (UInt8*) &timer[0];
            if(index < 0)
            {
                len = ONT_NUM_TIMERS * sizeof(ont_timer_t);
            }
            else if(index < ONT_NUM_TIMERS)
            {
                len = sizeof(ont_timer_t);
                *mem_ptr = (UInt8*) &timer[index];
            }
            else
            {
                return -1; // error
            }
            
            break;
        #ifdef _PEER
        case DEBUG_MEMORY_PEER:
            *mem_ptr = (UInt8*) &peer[0];
            if(index < 0)
            {
                len = ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_unit_t);
            }
            else if(index < ONE_NET_MAX_PEER_UNIT)
            {
                len = sizeof(on_peer_unit_t);
                *mem_ptr = (UInt8*) &peer[index];
            }
            else
            {
                return -1; // error
            }
            
            break;
        #endif
        case DEBUG_MEMORY_BASE_PARAM:
            *mem_ptr = (UInt8*) &on_base_param;
            len = sizeof(on_base_param_t);
            break;
        case DEBUG_MEMORY_INVITE_TXN:
            *mem_ptr = (UInt8*) &invite_txn;
            len = sizeof(on_txn_t);
            break;        
        case DEBUG_MEMORY_RESPONSE_TXN:
            *mem_ptr = (UInt8*) &response_txn;
            len = sizeof(on_txn_t);
            break; 
        case DEBUG_MEMORY_SINGLE_TXN:
            *mem_ptr = (UInt8*) &single_txn;
            len = sizeof(on_txn_t);
            break; 
        #ifdef _BLOCK_MESSAGES_ENABLED
        case DEBUG_MEMORY_BLOCK_TXN:
            *mem_ptr = (UInt8*) &block_txn;
            len = sizeof(on_txn_t);
            break; 
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case DEBUG_MEMORY_STREAM_TXN:
            *mem_ptr = (UInt8*) &stream_txn;
            len = sizeof(on_txn_t);
            break; 
        #endif
        default:
            return -1; // error
    }


    if(offset < 0 || offset >= len)
    {
        return -1; // error
    }
    
    (*mem_ptr) += offset;
    return len - offset;
}


/*!
    \brief Parses a string and returns a certain segment of memory
      given the string parameeters.


    \param[out] mem_ptr The address of the memory requested.
    \param[in] ASCII_PARAM_LIST The string to parse for parameters.


    \return the number of bytes of relevant memory.  -1 upon error
*/
static int parse_memory_str(UInt8** mem_ptr,
  const char * const ASCII_PARAM_LIST)
{
    int i, offset;
    debug_memory_t memory_type;
    const char* ptr = ASCII_PARAM_LIST;
    char* end_ptr;
    BOOL found = FALSE;
    BOOL has_index; // true for memory that is an array, false otherwise
    int index = -1; // relevant only if has_index is true
    
    if(!mem_ptr || !ASCII_PARAM_LIST)
    {
        return -1; // error
    }
    
    offset = 0;
    
    for(i = 0; !found && i < DEBUG_MEMORY_COUNT; i++)
    {
        if(!strnicmp(debug_memory_str[i], ptr, strlen(debug_memory_str[i])))
        {
            memory_type = i;
            found = TRUE;
            ptr += strlen(debug_memory_str[i]);
        }
    }
    
    if(!found)
    {
        return -1;  // invalid string.
    }
    
    switch(memory_type)
    {
        case DEBUG_MEMORY_TIMER:
        #ifdef _PEER
        case DEBUG_MEMORY_PEER:
        #endif
            has_index = TRUE; // these are arrays.
            break;
        default:
            has_index = FALSE; // not an array.
    }
    
    // look for an index, if relevant
    if(has_index && ((*ptr) == ':'))
    {
        ptr++;
        index = one_net_strtol(ptr, &end_ptr, 0);
        if(!end_ptr || end_ptr == ptr)
        {
            return -1; // error
        } // if parsing the data failed //
        ptr = end_ptr;
    }
    
    // now look for an offset, if any.
    if((*ptr) == ':')
    {
        ptr++;
        offset = one_net_strtol(ptr, &end_ptr, 0);
        if(!end_ptr || end_ptr == ptr)
        {
            return -1; // error
        } // if parsing the data failed //
        ptr = end_ptr;
    }
    
    if((*ptr) != '\n')
    {
        return -1; // error
    }

    return get_memory_loc(mem_ptr, memory_type, index, offset);
}


/*!
    \brief Parses a string and sets the location and length of the memory
      pointed to it in global variables


    \param[in] ASCII_PARAM_LIST The string to parse for parameters.


    \return ONCLI_SUCCESS if the memory string is valid, ONCLI_PARSE_ERR
      otherwise.
*/
static oncli_status_t memory_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    UInt8* mem_ptr;
    int len = parse_memory_str(&mem_ptr, ASCII_PARAM_LIST);
    if(len < 1)
    {
        return ONCLI_PARSE_ERR;
    }
    
    oncli_send_msg("Address = (%p) : Length = %d\n", mem_ptr, len); 
    memory_ptr = mem_ptr;
    memory_len = len;
    return ONCLI_SUCCESS;
}




static oncli_status_t pause_cmd_hdlr(void)
{
    pause = !pause;
    return ONCLI_SUCCESS;
}


static oncli_status_t proceed_cmd_hdlr(void)
{
    proceed = TRUE;
    return ONCLI_SUCCESS;
}


static oncli_status_t ratchet_cmd_hdlr(void)
{
    ratchet = !ratchet;
    return ONCLI_SUCCESS;
}


/*!
    \brief Sets an interval.  See the debug_intervals[] array
           for a list of settable intervals.
           
    interval:1:25000 would set interval 1 to 25,000 ms(again, see the
             debug_intevals[] array for a list of intervals and their indexes.

    \param[in] ASCII_PARAM_LIST The string to parse for parameters.

    \return ONCLI_SUCCESS if the interval was set.
            ONCLI_PARSE_ERR otherwise.
*/
static oncli_status_t interval_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    UInt8 interval_index;
    tick_t new_interval;
    const char* ptr = ASCII_PARAM_LIST;
    const char* end_ptr = ptr;
    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_PARSE_ERR;
    }

    interval_index = one_net_strtol(ptr, (char **)&end_ptr, 0);
    
    if(!end_ptr || end_ptr == ptr || *end_ptr != ':' ||
        interval_index >= NUM_DEBUG_INTERVALS)
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //
    
    ptr = end_ptr;
    ptr++;
    
    new_interval = one_net_strtol(ptr, (char **)&end_ptr, 0);

    if(!end_ptr || end_ptr == ptr || *end_ptr != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //
    
    *(debug_intervals[interval_index]) = new_interval;
    return ONCLI_SUCCESS;
}
#endif



/*!
    \brief Converts a string of ASCII hex digits to a byte stream.
    
    \param[in] STR The ASCII string of hex digits.
    \param[out] byte_stream The byte stream that results from STR
    \param[in] NUM_ASCII_CHAR The number of ascii characters to convert.  This
      is really twice the number of bytes that were converted.
    
    \return The number of ASCII characters that were converted.
*/
static UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const UInt16 NUM_ASCII_CHAR)
{
    UInt16 num_converted;
    
    UInt8 hex;

    if(!STR || !byte_stream || !NUM_ASCII_CHAR)
    {
        return 0;
    } // if any of the parameters are invalid //

    for(num_converted = 0; num_converted < NUM_ASCII_CHAR; num_converted++)
    {
        hex = ascii_hex_to_nibble(STR[num_converted]);
        if(hex > 0x0F)
        {
            break;
        } // if the conversion failed //

        if(num_converted & 0x01)
        {
            byte_stream[num_converted >> 1] |= hex;
        } // if the second nibble in the byte //
        else
        {
            byte_stream[num_converted >> 1] = hex << 4;
        } // else the first nibble in the byte //
    } // loop to convert payload from ascii //
    
    return num_converted;
} // ascii_hex_to_byte_stream //


/*!
    \brief Parses the channel data from the oncli and returns the adjust channel
      number based on the data read in.

    \param[in] ASCII The ascii parameters to be parsed.
    \param[out] channel The adjusted channel.
    
    \return ONCLI_SUCCESS If the parameters were successfully parsed.
            ONCLI_BAD_PARAM If any of the parameter are invalid
            ONCLI_PARSE_ERR If ASCII could not be parsed
            ONCLI_INTERNAL_ERR If something unexpected occured.
*/
static oncli_status_t oncli_parse_channel(const char * ASCII, UInt8 * const channel)
{
    enum
    {
        ONCLI_US,                   //! US region
        ONCLI_EUR                   //! European region
    };
    
    const char * END_PTR = 0;
    
    UInt8 region;

    if(!ASCII || !channel)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // get the region
#ifdef _US_CHANNELS
    if(!strnicmp(ASCII, ONCLI_US_STR, strlen(ONCLI_US_STR)))
    {
        region = ONCLI_US;
        ASCII += strlen(ONCLI_US_STR);
    } // if it's a US frequency //
#endif
#ifdef _EUROPE_CHANNELS
#ifdef _US_CHANNELS
    else if(!strnicmp(ASCII, ONCLI_EUR_STR, strlen(ONCLI_EUR_STR)))
#else
    if(!strnicmp(ASCII, ONCLI_EUR_STR, strlen(ONCLI_EUR_STR)))
#endif
    {
        region = ONCLI_EUR;
        ASCII += strlen(ONCLI_EUR_STR);
    } // else if it's a European frequency //
#endif
    else
    {
        return ONCLI_PARSE_ERR;
    } // else the priority is invalid //
    
    if(*ASCII != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if the command isn't formatted properly //
    ASCII++;

    // Get the channel number
    if(!isdigit(*ASCII))
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //

    *channel = one_net_strtol(ASCII, &END_PTR, 0) - 1;
    if(!END_PTR || END_PTR == ASCII || (*END_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if the parameter was not valid //
    
    switch(region)
    {
#ifdef _US_CHANNELS
        case ONCLI_US:
        {
            if((SInt8)(*channel) < (SInt8)ONE_NET_MIN_US_CHANNEL
              || *channel > ONE_NET_MAX_US_CHANNEL)
            {
                return ONCLI_PARSE_ERR;
            } // if the parameter is invalid //
            break;
        } // US case //
#endif
#ifdef _EUROPE_CHANNELS
        case ONCLI_EUR:
        {
            #ifdef _US_CHANNELS
            *channel += ONE_NET_EUR_CHANNEL_1;
            #endif
			// typecast to override "comparison is always false" warning
            if((SInt8) *channel < (SInt8)ONE_NET_MIN_EUR_CHANNEL
              || *channel > ONE_NET_MAX_EUR_CHANNEL)
            {
                return ONCLI_PARSE_ERR;
            } // if the parameter is invalid //
            break;
        } // European case //
#endif       
        default:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // default case //
    } // switch(region) //
    
    return ONCLI_SUCCESS;
} // oncli_parse_channel //


/*!
    \brief Parses the key (or portion of the key) from the ASCII parameter.
    
    ASCII contains two groups of four chars: xxxx-xxxx.
    These are duplicated in the upper two groups. So, for example if this
    function is passed "eval-0006" as the ASCII invite key

    the key is created as a 16 byte ASCII conversion of "eval0006eval0006"
    
    
    \param[in] ASCII The ASCII representation of the key
    \param[out] end_ptr Pointer to the character AFTER the key
    \param[out] key They key that ASCII represents.
    
    \return ONCLI_SUCCESS If the key was successfully parsed
            ONCLI_BAD_PARAM if any of the parameters are invalid
            ONCLI_PARSE_ERR If the ASCII parameter was not formatted correctly.
*/
#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t parse_invite_key(const char * ASCII,
  char** end_ptr, one_net_xtea_key_t * const key)
{
    enum
    {
        END_OF_PARAM_IDX = 9, // now it requires xxxx-xxxx

        // number of characters in the key that are grouped together.
        KEY_CH_GROUP_SIZE = 4,

        // The position where the group delimiter character occurs (every nth).
        // This is 1 more than the size of the grouping.
        KEY_DELIMITER_POS
    };

    // delimits the unique key into 2 sets of 4 values.
    const char GROUP_DELIMITER = '-';

    UInt8 param_idx, key_idx;

    if(!ASCII || !key || !end_ptr)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    *end_ptr = (char*) (ASCII + END_OF_PARAM_IDX);

    // parse the key parameter
    for(param_idx = 0, key_idx = 0; param_idx < END_OF_PARAM_IDX; param_idx++)
    {
        if((param_idx % KEY_DELIMITER_POS) == KEY_CH_GROUP_SIZE)
        {
            if(ASCII[param_idx] != GROUP_DELIMITER)
            {
                return ONCLI_PARSE_ERR;
            } // if the character is not the delimiter //
        } // if the character should be the delimiter //
        else
        {
            if(!oncli_is_valid_unique_key_ch(ASCII[param_idx]))
            {
                return ONCLI_PARSE_ERR;
            } // if the character is invalid //

            (*key)[key_idx+8] = ASCII[param_idx]; // group is duplicated
            (*key)[key_idx++] = ASCII[param_idx];
        } // else it should be a key character //
    } // loop to read in the unique key //
    
    return ONCLI_SUCCESS;
} // parse_invite_key //
#endif


#ifdef _ENABLE_SINGLE_COMMAND
/*!
    \brief Parses the ASCII parameter string for the parameters needed to send
      a transaction. 

    Some parameters to be parsed are optional. That is, if the pointer to an
    ouput paramter is NULL (zero), then that paramter will not be parsed.
    This variation was added when we found that we needed to modify the
    syntax of the "single" command after changing from 8 bit to 4 bit
    unit numbers. The new syntax of the "single" command no longer includes
    the source and destination unit numbers. Rather than writing a new 
    parse function, we decided to make the src_unit and dst_unit parameters
    optional.

    \param[in] PARAM_PTR The ASCII parameter list.
    \param[out] src_unit The unit in the source device sending the message or NULL 
    if there is no src_unit to be parsed.
    \param[out] dst_unit The unit in the destination device the message i or NULL 
    if there is no dst_unit to be parsed.
      intended for.
    \param[out] enc_dst The encoded destination device.
    \param[out] send_to_peer_list True if sending to peer list, false otherwise
    \return If successful, pointer to first characer after the last parameter
      that was parsed, or 0 if parsing was not successful.
*/
#ifdef _PEER
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit,
  on_encoded_did_t * const enc_dst, BOOL* send_to_peer_list)
#else
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit,
  on_encoded_did_t * const enc_dst)
#endif
{
    char * end_ptr;
    on_raw_did_t raw_did;

    if(!PARAM_PTR || !enc_dst)
    {
        return 0;
    } // if any of the parameters are invalid //

    if (src_unit)
    {
        // read the src unit
        *src_unit = one_net_strtol(PARAM_PTR, &end_ptr, 16);
        if(!end_ptr || end_ptr == PARAM_PTR)
        {
            return 0;
        } // if the parameter was not valid //
        PARAM_PTR = end_ptr;

        // check the parameter delimiter
        if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
        {
            return 0;
        } // if malformed parameter //
        PARAM_PTR++;
    }

    if (dst_unit)
    {
        // read the dst unit
        *dst_unit = one_net_strtol(PARAM_PTR, &end_ptr, 16);
        if(!end_ptr || end_ptr == PARAM_PTR)
        {
            return 0;
        } // if the parameter was not valid //
        PARAM_PTR = end_ptr;

        // check the parameter delimiter
        if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
        {
            return 0;
        } // if malformed parameter //
        PARAM_PTR++;
    }

    // read in the raw did
    if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
    {
        #ifdef _PEER
        *send_to_peer_list = FALSE;
        #endif
        if(ascii_hex_to_byte_stream(PARAM_PTR, raw_did, ONCLI_ASCII_RAW_DID_SIZE)
          != ONCLI_ASCII_RAW_DID_SIZE)
        {
            return 0;
        } // if converting the raw destination did failed //
        
        // make sure the raw did is skipped
        PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
        
        if(on_encode(*enc_dst, raw_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
        {
            return 0;
        }
    } // if the did is present //
    else
    {
        #ifdef _PEER
        *send_to_peer_list = TRUE;
        #endif
        return 0;
    } // else the did is not present //

    return PARAM_PTR;
} // parse_ascii_tx_param //


/*!
    \brief Parses the ASCII form of data to send in a transaction.
    
    Converts the ASCII data to a byte stream.  ASCII must be twice as big
    as data since it takes 2 ASCII bytes for every byte that will be sent.
    
    \param[in] ASCII The ASCII data to convert.
    \param[out] data The binary version of the data in ASCII.
    \param[in/out] On input, the size of data.  On output, the number of bytes
      contained in data.

    \return If successful, a pointer to the first byte beyond the data that was
      converted, or 0 if failed to convert the data.
*/
static const char * parse_ascii_tx_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len)
{
    if(!ASCII || !data || !data_len || !*data_len)
    {
        return 0;
    } // if any of the parameters are invalid //

    if((*data_len  = ascii_hex_to_byte_stream(ASCII, data, *data_len << 1))
      & 0x01)
    {
        // did not parse an even number of ASCII bytes, meaning the last byte
        // was not complete.
        *data_len = 0;
        return 0;
    } // if converted an odd number of characters //

    ASCII += *data_len;
    
    // ascii_hex_to_byte_stream returns the number of ASCII characters that
    // were converted, so divide the result by 2 to get the actual number
    // of bytes in the stream.
    *data_len >>= 1;

    return ASCII;
} // parse_ascii_tx_data //


/*!
    \brief Parses the ASCII text to send in the transaction.
    
    The ASCII string passed in must contain the '"''s delimiting the start and
    end of the text.
    
    \param[in] ASCII The text to parse.
    \param[out] data The resulting data.
    \param[in/out] data_len On input, the size of data.  On output, the number
      of bytes that were parsed into data.

    \return If successful, a pointer to the first byte beyond the data that was
      converted, or 0 if failed to convert the data.
*/
static const char * parse_ascii_tx_text_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len)
{
    const char TEXT_DELIMITER = '"';

    UInt16 num_ch = 0;

    if(!ASCII || !data || !data_len || !*data_len)
    {
        return 0;
    } // if any of the parameters are invalid //
    
    if(*ASCII++ != TEXT_DELIMITER)
    {
        return FALSE;
    } // if the string is invalid //
    
    while(*ASCII != TEXT_DELIMITER && num_ch < *data_len)
    {
        data[num_ch++] = *ASCII++;
    } // while more text to read, and more space to store it in //
    
    if(*ASCII++ != TEXT_DELIMITER)
    {
        *data_len = 0;
        return 0;
    } // if the text data was invalid //
    
    *data_len = num_ch;
    return ASCII;
} // parse_ascii_tx_text_data //
#endif



//! @} oncli_hdlr_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} oncli_hdlr
