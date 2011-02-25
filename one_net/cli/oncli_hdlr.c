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


#ifdef _R8C_TINY
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //


#include "oncli_hdlr.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "oncli.h"
#include "oncli_port.h"
#include "oncli_str.h"
#include "one_net.h"
#include "one_net_eval.h"
#include "one_net_eval_hal.h"
#include "one_net_master.h"
#include "one_net_client.h"
#include "one_net_client_net.h"
#include "one_net_port_specific.h"
#include "one_net_encode.h"
#ifdef _ENHANCED_INVITE
    #include "one_net_eval.h"
    #include "one_net_xtea.h"
#endif

#ifdef  _ENABLE_DUMP_COMMAND
#include "flash.h"
#endif

#ifdef _ENABLE_RSSI_COMMAND 
#include "uart.h"
#endif

#include "str.h"
#include "dfi.h"

extern BOOL client_joined_network;
extern one_net_raw_did_t client_did;


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

//! @} oncli_hdlr_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup oncli_hdlr_pri_func
//! \ingroup oncli_hdlr
//! @{

// Transaction command handlers.
#ifdef _ENABLE_SINGLE_COMMAND
	static oncli_status_t single_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_SET_VALUE_COMMAND
	static oncli_status_t set_value_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_SET_PIN_COMMAND
	static oncli_status_t set_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#if defined(_ENABLE_SET_VALUE_COMMAND) || defined(_ENABLE_SET_PIN_COMMAND) || defined(_ENABLE_SINGLE_COMMAND)
	static oncli_status_t oncli_send_single(const one_net_raw_did_t* const dst,
        const UInt8* const payload, const on_priority_t priority);
#endif
#if defined(_ENABLE_SET_VALUE_COMMAND) || defined(_ENABLE_SET_PIN_COMMAND)
    static oncli_status_t oncli_send_set_value(const one_net_raw_did_t* const dst_did,
        const ona_msg_type_t msg_type, const UInt16 new_value, const UInt8 dst_unit);
#endif
#ifdef _ENABLE_SINGLE_TEXT_COMMAND
	static oncli_status_t single_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_BLOCK_COMMAND
	static oncli_status_t block_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_BLOCK_TEXT_COMMAND
	static oncli_status_t block_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_ERASE_COMMAND
	static oncli_status_t erase_cmd_hdlr(void);
#endif
#ifdef _ENABLE_SAVE_COMMAND
	static oncli_status_t save_cmd_hdlr(void);
#endif
#ifdef _ENABLE_DUMP_COMMAND 
	static oncli_status_t dump_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_MEMDUMP_COMMAND 
	static oncli_status_t memdump_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_MEMLOAD_COMMAND 
	static oncli_status_t memload_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_RSINGLE_COMMAND 
	static oncli_status_t rsend_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_RSSI_COMMAND 
	static oncli_status_t rssi_cmd_hdlr(void);
#endif
#ifdef _ENABLE_LIST_COMMAND 
	static oncli_status_t list_cmd_hdlr(void);
#endif
#ifdef _ENABLE_IDLE_COMMAND
    static oncli_status_t idle_cmd_hdlr(const char* const ASCII_PARAM_LIST);
#endif

// MASTER only command handlers
#ifdef _ENABLE_INVITE_COMMAND
	static oncli_status_t invite_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_CANCEL_INVITE_COMMAND
	static oncli_status_t cancel_invite_cmd_hdlr(void);
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
#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
	static oncli_status_t change_frag_dly_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_CHANGE_KEY_COMMAND
	static oncli_status_t change_key_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
	static oncli_status_t rm_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_DATA_RATE_TEST_COMMAND
	static oncli_status_t data_rate_test_cmd_hdlr(
	  const char * const ASCII_PARAM_LIST);
#endif

// CLIENT only command handlers
#ifdef _ENABLE_USER_PIN_COMMAND
	static oncli_status_t user_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

// Join command - if specifying a channel and a timeout
#if defined(_ENABLE_JOIN_COMMAND) && defined(_ENHANCED_INVITE)
    static oncli_status_t join_cmd_hdlr(const char* const ASCII_PARAM_LIST);
#endif

// Mode command handlers
#ifdef _ENABLE_CHANNEL_COMMAND
	static oncli_status_t channel_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_SETNI_COMMAND
	static oncli_status_t setni_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _SNIFFER_MODE
	#ifdef _ENABLE_SNIFF_COMMAND
		static oncli_status_t sniff_cmd_hdlr(const char * const ASCII_PARAM_LIST);
	#endif
#endif
#ifdef _ENABLE_MODE_COMMAND
	static oncli_status_t mode_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_ECHO_COMMAND
	static oncli_status_t echo_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

// parsing functions
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit, one_net_raw_did_t * const dst,
  UInt8 * const priority, BOOL * const send_to_peer);
static const char * parse_ascii_tx_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len);
static const char * parse_ascii_tx_text_data(const char * ASCII, UInt8 * data,
  UInt16 * data_len);
#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t parse_invite_key(const char * ASCII,
  one_net_xtea_key_t * const key);
#endif

//#if (defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)) ||\
//  defined(_ENABLE_INVITE_COMMAND)
static oncli_status_t parse_channel(const char * ASCII, UInt8 * const channel);
//#endif

static UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const unsigned int NUM_ASCII_CHAR);

BOOL oncli_is_valid_unique_key_ch(const char CH);

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

    // Need to compare the text commands first since the regular send commands
    // are a substring of the send text commands
	
	#ifdef _ENABLE_SINGLE_TEXT_COMMAND
    if(!strnicmp(ONCLI_SINGLE_TXT_CMD_STR, CMD, strlen(ONCLI_SINGLE_TXT_CMD_STR)))
    {
        *CMD_STR = ONCLI_SINGLE_TXT_CMD_STR;

        if(CMD[strlen(ONCLI_SINGLE_TXT_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE;
        *cmd_hdlr = &single_txt_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // if the send single text command was received //
	#endif
	
	#ifdef _ENABLE_SET_PIN_COMMAND
    if(!strnicmp(ONCLI_SET_PIN_CMD_STR, CMD, strlen(ONCLI_SET_PIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_SET_PIN_CMD_STR;

        if(CMD[strlen(ONCLI_SET_PIN_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &set_pin_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // if the set pin command was received //
	#endif
	
	#ifdef _ENABLE_IDLE_COMMAND
    if(!strnicmp(ONCLI_IDLE_CMD_STR, CMD, strlen(ONCLI_IDLE_CMD_STR)))
    {
        *CMD_STR = ONCLI_IDLE_CMD_STR;

        if(CMD[strlen(ONCLI_IDLE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &idle_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // if the set pin command was received //
	#endif
		
	#ifdef _ENABLE_SET_VALUE_COMMAND
    if(!strnicmp(ONCLI_SET_VALUE_CMD_STR, CMD, strlen(ONCLI_SET_VALUE_CMD_STR)))
    {
        *CMD_STR = ONCLI_SET_VALUE_CMD_STR;

        if(CMD[strlen(ONCLI_SET_VALUE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &set_value_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // if the set value command was received //
	#endif
			
	#ifdef _ENABLE_SINGLE_COMMAND
    if(!strnicmp(ONCLI_SINGLE_CMD_STR, CMD, strlen(ONCLI_SINGLE_CMD_STR)))
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
	#endif
	
	#ifdef _ENABLE_BLOCK_TEXT_COMMAND
    if(!strnicmp(ONCLI_BLOCK_TXT_CMD_STR, CMD,
      strlen(ONCLI_BLOCK_TXT_CMD_STR)))
    {
        *CMD_STR = ONCLI_BLOCK_TXT_CMD_STR;

        if(CMD[strlen(ONCLI_BLOCK_TXT_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE;
        *cmd_hdlr = &block_txt_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the send block text command was received //
	#endif
	
	#ifdef _ENABLE_BLOCK_COMMAND
    if(!strnicmp(ONCLI_BLOCK_CMD_STR, CMD, strlen(ONCLI_BLOCK_CMD_STR)))
    {
        *CMD_STR = ONCLI_BLOCK_CMD_STR;

        if(CMD[strlen(ONCLI_BLOCK_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &block_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the send block command was received //
	#endif
	
	#ifdef _ENABLE_ERASE_COMMAND
    if(!strnicmp(ONCLI_ERASE_CMD_STR, CMD, strlen(ONCLI_ERASE_CMD_STR)))
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
    if(!strnicmp(ONCLI_SAVE_CMD_STR, CMD, strlen(ONCLI_SAVE_CMD_STR)))
    {
        *CMD_STR = ONCLI_SAVE_CMD_STR;

        if(CMD[strlen(ONCLI_SAVE_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return save_cmd_hdlr();
    } // else if the save command was received //
	#endif

	#ifdef _ENABLE_DUMP_COMMAND 
    if(!strnicmp(ONCLI_DUMP_CMD_STR, CMD, strlen(ONCLI_DUMP_CMD_STR)))
    {
        *CMD_STR = ONCLI_DUMP_CMD_STR;

        // parameters are optional
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &dump_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the dump command was received //
	#endif
	
	#ifdef _ENABLE_MEMDUMP_COMMAND 
    if(!strnicmp(ONCLI_MEMDUMP_CMD_STR, CMD, strlen(ONCLI_MEMDUMP_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMDUMP_CMD_STR;

        if(CMD[strlen(ONCLI_MEMDUMP_CMD_STR)]
          != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &memdump_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the memdump command was received //
	#endif
	
	#ifdef _ENABLE_MEMLOAD_COMMAND 
    if(!strnicmp(ONCLI_MEMLOAD_CMD_STR, CMD, strlen(ONCLI_MEMLOAD_CMD_STR)))
    {
        *CMD_STR = ONCLI_MEMLOAD_CMD_STR;

        if(CMD[strlen(ONCLI_MEMLOAD_CMD_STR)]
          != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &memload_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the memload command was received //
	#endif	

	#ifdef _ENABLE_RSINGLE_COMMAND 
    if(!strnicmp(ONCLI_RSEND_CMD_STR, CMD, strlen(ONCLI_RSEND_CMD_STR)))
    {
        *CMD_STR = ONCLI_RSEND_CMD_STR;

        if(CMD[strlen(ONCLI_RSEND_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        // parameters are optional
        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &rsend_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the rsend command was received //
	#endif

	#ifdef _ENABLE_RSSI_COMMAND 
    if(!strnicmp(ONCLI_RSSI_CMD_STR, CMD, strlen(ONCLI_RSSI_CMD_STR)))
    {
        *CMD_STR = ONCLI_RSSI_CMD_STR;

        if(CMD[strlen(ONCLI_RSSI_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return rssi_cmd_hdlr();
    } // else if the rssi command was received //
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

	#ifdef _ENABLE_INVITE_COMMAND
    if(!strnicmp(ONCLI_INVITE_CMD_STR, CMD, strlen(ONCLI_INVITE_CMD_STR)))
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
    if(!strnicmp(ONCLI_CANCEL_INVITE_CMD_STR, CMD,
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

	#ifdef _ENABLE_ASSIGN_PEER_COMMAND
    if(!strnicmp(ONCLI_ASSIGN_PEER_CMD_STR, CMD,
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
    if(!strnicmp(ONCLI_UNASSIGN_PEER_CMD_STR, CMD,
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
    if(!strnicmp(ONCLI_UPDATE_MASTER_CMD_STR, CMD,
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
    if(!strnicmp(ONCLI_CHANGE_KEEP_ALIVE_CMD_STR, CMD,
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
	
	#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    if(!strnicmp(ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR, CMD,
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
	
	#ifdef _ENABLE_CHANGE_KEY_COMMAND
    if(!strnicmp(ONCLI_CHANGE_KEY_CMD_STR, CMD,
      strlen(ONCLI_CHANGE_KEY_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANGE_KEY_CMD_STR;

        if(CMD[strlen(ONCLI_CHANGE_KEY_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &change_key_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change key command was received //
	#endif
	
	#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
    if(!strnicmp(ONCLI_RM_DEV_CMD_STR, CMD, strlen(ONCLI_RM_DEV_CMD_STR)))
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
	
	#ifdef _ENABLE_DATA_RATE_TEST_COMMAND
    if(!strnicmp(ONCLI_DATA_RATE_TEST_CMD_STR, CMD,
      strlen(ONCLI_DATA_RATE_TEST_CMD_STR)))
    {
        *CMD_STR = ONCLI_DATA_RATE_TEST_CMD_STR;

        if(CMD[strlen(ONCLI_DATA_RATE_TEST_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &data_rate_test_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the data rate test command was received //
	#endif
	
	#ifdef _ENABLE_GET_CHANNEL_COMMAND
    if(!strnicmp(ONCLI_GET_CHANNEL_CMD_STR, CMD,
      strlen(ONCLI_GET_CHANNEL_CMD_STR)))
    {
        *CMD_STR = ONCLI_GET_CHANNEL_CMD_STR;
        
        if(CMD[strlen(ONCLI_GET_CHANNEL_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        return oncli_print_channel(TRUE);
    } // else if the channel command was received //
	#endif
	
	#ifdef _ENABLE_USER_PIN_COMMAND
    if(!strnicmp(ONCLI_USER_PIN_CMD_STR, CMD,
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
	
	#ifdef _ENABLE_JOIN_COMMAND
	#ifndef _ENHANCED_INVITE
	
    if(!strnicmp(ONCLI_JOIN_CMD_STR, CMD, strlen(ONCLI_JOIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_JOIN_CMD_STR;

        if(CMD[strlen(ONCLI_JOIN_CMD_STR)] != '\n')
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //
        
        return oncli_reset_client();
    } // else if the join command was received //
	#else
    if(!strnicmp(ONCLI_JOIN_CMD_STR, CMD, strlen(ONCLI_JOIN_CMD_STR)))
    {
        *CMD_STR = ONCLI_JOIN_CMD_STR;

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &join_cmd_hdlr;

        return ONCLI_SUCCESS;
	}
	#endif
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
	
	#if defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)
    if(!strnicmp(ONCLI_SNIFF_CMD_STR, CMD, strlen(ONCLI_SNIFF_CMD_STR)))
    {
        oncli_status = sniff_cmd_hdlr(CMD + strlen(ONCLI_SETNI_CMD_STR) + 1);
        
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

	#if defined(_AUTO_MODE) && defined(_ENABLE_MODE_COMMAND)
    if(!strnicmp(ONCLI_MODE_CMD_STR, CMD, strlen(ONCLI_MODE_CMD_STR)))
    {
        *CMD_STR = ONCLI_MODE_CMD_STR;

        if(CMD[strlen(ONCLI_MODE_CMD_STR)] != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &mode_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the mode command was received //
	#endif

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
    else
    {
        *CMD_STR = CMD;
        return ONCLI_INVALID_CMD;
    } // else the command was invalid //
	#endif

    return FALSE;
} // oncli_parse_cmd //

//! @} oncli_hdlr_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup oncli_hdlr_pri_func
//! \ingroup oncli_hdlr
//! @{

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
#ifdef _ENABLE_SINGLE_COMMAND
static oncli_status_t single_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    one_net_raw_did_t dst;
	
    BOOL send_to_peer; // we're not using this, but function wants it?
    
    UInt8 priority;
    UInt16 data_len;
    UInt8 pld[ONE_NET_RAW_SINGLE_DATA_LEN];

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // get the send parameters
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, NULL,
      NULL, &dst, &priority, &send_to_peer)))
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // get the data
    data_len = sizeof(pld);
    if(!(PARAM_PTR = parse_ascii_tx_data(PARAM_PTR, pld,
      &data_len)) || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //
	
    return oncli_send_single(&dst, pld, priority);
} // single_cmd_hdlr //
#endif


#if defined(_ENABLE_SET_VALUE_COMMAND) || defined(_ENABLE_SET_PIN_COMMAND) || defined(_ENABLE_SINGLE_COMMAND)
static oncli_status_t oncli_send_single(const one_net_raw_did_t* const dst,
    const UInt8* const payload, const on_priority_t priority)
{
    UInt8 src_unit, dst_unit;
    one_net_send_single_func_t send_single_func;
    UInt8 data_len = ONE_NET_RAW_SINGLE_DATA_LEN;
    
    if(!(send_single_func = oncli_get_send_single_txn_func()))
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if the command is not valid for this device //
		
    // extract the src and dst units in case they are needed by the *send_single_func
    src_unit = (payload[ONA_MSG_SRC_UNIT_IDX] >> ONA_MSG_SRC_UNIT_SHIFT) & ONA_MSG_DST_UNIT_MASK;
    dst_unit = payload[ONA_MSG_SRC_UNIT_IDX] & ONA_MSG_DST_UNIT_MASK;

    switch(((*send_single_func)(payload, data_len, data_len, (UInt8) priority,
      dst, src_unit)))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        case ONS_BAD_PARAM:
        {
            // A bad param could be because the src unit is not valid, so call
            // it a parse error so the user checks the ascii parameters.
            return ONCLI_PARSE_ERR;
            break;
        } // bad parameter case //
        
        case ONS_RSRC_FULL:
        {
            return ONCLI_RSRC_UNAVAILABLE;
            break;
        } // resource full case //
        
        case ONS_BAD_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // invalid destination case //
        
        case ONS_NOT_JOINED:
        {
            return ONCLI_NOT_JOINED;
            break;
        } // device needs to join a network first //

        default:
        {
            break;
        } // default case //
    } // switch(send_single_func) //
    
    return ONCLI_INTERNAL_ERR;	
}
#endif


#ifdef _ENABLE_SET_VALUE_COMMAND
/*!
    \brief Handles receiving the set value command and all its parameters.
    
    The set value command has the form
    
    set pin:RAW DST DID:DST UNIT:MSG TYPE:NEW VALUE 
    
    where MSG TYPE is the type of the destination of the logical unit (i.e. ONA_SWITCH)
    and NEW VALUE is the new value of the unit
	
    
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
static oncli_status_t set_value_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr = 0;
    
    one_net_raw_did_t dst_did;
    UInt8 dst_unit;
    UInt16 new_value;
    ona_msg_type_t dst_log_unit_type;
	
    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    // read in the raw destination did
    if(ascii_hex_to_byte_stream(PARAM_PTR, dst_did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw destination did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the destination unit
    dst_unit = strtol(PARAM_PTR, &end_ptr, 16);
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
    
    // read in the logical type of the destination unit
    dst_log_unit_type = strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    PARAM_PTR = end_ptr;
	
    if(dst_log_unit_type >= ONA_MSG_TYPE_MASK)
    {
        // invalid type.
		return ONCLI_PARSE_ERR;
    }
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the new value for the destination unit
    new_value = strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    PARAM_PTR = end_ptr;
	   
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //

    return oncli_send_set_value(&dst_did, dst_log_unit_type, new_value, dst_unit);
}
#endif


#if defined(_ENABLE_SET_VALUE_COMMAND) || defined(_ENABLE_SET_PIN_COMMAND)
static oncli_status_t oncli_send_set_value(const one_net_raw_did_t* const dst_did,
    const ona_msg_type_t msg_type, const UInt16 new_value, const UInt8 dst_unit)
{
    const ona_msg_class_t msg_class = ONA_COMMAND;
	const UInt8 src_unit = 0; // just need something here.  Doesn't really matter what.

    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN];
	
    put_msg_hdr(msg_class | msg_type, payload);
    put_src_unit(src_unit, payload);
    put_dst_unit(dst_unit, payload);
    put_msg_data(new_value, payload);

    return oncli_send_single(dst_did, payload, ONE_NET_SEND_SINGLE_PRIORITY);
}
#endif


#ifdef _ENABLE_SET_PIN_COMMAND
/*!
    \brief Handles receiving the set pin command and all its parameters.
    
    The set pin command has the form
    
    set pin:RAW DST DID:DST UNIT:NEW PIN VALUE
    
    where NEW PIN VALUE is low, high, or toggle
    
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
static oncli_status_t set_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr = 0;
    
    one_net_raw_did_t dst_did;
    UInt8 dst_unit, new_pin_value;
	
    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    // read in the raw destination did
    if(ascii_hex_to_byte_stream(PARAM_PTR, dst_did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw destination did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the destination unit
    dst_unit = strtol(PARAM_PTR, &end_ptr, 16);
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
    
    // read in the value to set destination pin to (low, high, toggle)
    if(!strnicmp(PARAM_PTR, ONCLI_LOW_STR, strlen(ONCLI_LOW_STR)))
    {
        new_pin_value = ONA_OFF;
        PARAM_PTR += strlen(ONCLI_LOW_STR);
    } // if it should be an input //
    else if(!strnicmp(PARAM_PTR, ONCLI_HIGH_STR, strlen(ONCLI_HIGH_STR)))
    {
        new_pin_value = ONA_ON;
        PARAM_PTR += strlen(ONCLI_HIGH_STR);
    } // else if it should be an output //
    else if(!strnicmp(PARAM_PTR, ONCLI_TOGGLE_STR, strlen(ONCLI_TOGGLE_STR)))
    {
        new_pin_value = ONA_TOGGLE;
        PARAM_PTR += strlen(ONCLI_TOGGLE_STR);
    } // else if it should be an output //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else if the priority is invalid //
    
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //

    return oncli_send_set_value(&dst_did, ONA_SWITCH, new_pin_value, dst_unit);
} // set_pin_cmd_hdlr //
#endif


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
#ifdef _ENABLE_SINGLE_TEXT_COMMAND
static oncli_status_t single_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    one_net_send_single_func_t send_single_func;

    one_net_raw_did_t dst;

    UInt16 data_len;

    UInt8 src_unit, dst_unit, priority;
    UInt8 pld[ONE_NET_RAW_SINGLE_DATA_LEN] = {0};
    
    BOOL send_to_peer = FALSE;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if(!(send_single_func = oncli_get_send_single_txn_func()))
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if the command is not valid for this device //

    // get the send parameters
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &dst, &priority, &send_to_peer)))
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // get the data
    data_len = sizeof(pld) - ONA_MSG_HDR_LEN;
    if(!(PARAM_PTR = parse_ascii_tx_text_data(PARAM_PTR, &pld[ONA_MSG_DATA_IDX],
      &data_len)) || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //

    // store the message class/message type in the payload
    put_msg_hdr(ONA_COMMAND | ONA_SIMPLE_TEXT, &pld[0]);

    // store the source and destination unit numbers in the payload
    put_dst_unit(dst_unit, pld);
    put_src_unit(src_unit, pld);

    switch(((*send_single_func)(pld, sizeof(pld), ONA_MSG_DST_UNIT_IDX, priority,
      send_to_peer ? 0 : &dst, src_unit)))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        case ONS_BAD_PARAM:
        {
            return ONCLI_PARSE_ERR;
            break;
        } // bad parameter case //
        
        case ONS_RSRC_FULL:
        {
            return ONCLI_RSRC_UNAVAILABLE;
            break;
        } // resource full case //
        
        case ONS_BAD_ADDR:
        {
            return ONCLI_INVALID_DST;
        } // invalid address case //

        case ONS_NOT_JOINED:
        {
            return ONCLI_NOT_JOINED;
            break;
        } // device needs to join a network first //

        default:
        {
            break;
        } // default case //
    } // switch(send_single) //
    
    return ONCLI_CMD_FAIL;
} // single_txt_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the block command and all it's parameters.
    
    The block command has the form
    
    block:SRC UNIT:DST UNIT:RAW DST DID:PRIORITY:AABBCCDDEE...
    
    where AABBCCDDEE... is the packet to send in ASCII hex characters
      ('0' - '9', 'A' - 'F' (lower case is valid also)).  Only the parameters
      (starting with SRC UNIT) are passed in.
    
    \param ASCII_PARAM_LIST ASCII parameter list.  The parameters in this list
      should be seperated by ':'.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_q_block_request for more return values
*/
#ifdef _ENABLE_BLOCK_COMMAND
static oncli_status_t block_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
#ifndef _SNIFFER_FRONT_END
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    one_net_raw_did_t dst;

    UInt16 data_len;
    UInt16 data_type;                // type of block transaction being sent
    
    UInt8 src_unit, dst_unit, priority;
    
    // pld needs to consist of APP HDR + the data
    UInt8 pld[ONCLI_MAX_BLOCK_TXN_LEN + ONA_MSG_HDR_LEN];
    
    BOOL send_to_peer = FALSE;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // get the send parameters
    src_unit = 0;
    dst_unit = 0;
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, NULL,
      NULL, &dst, &priority, &send_to_peer)) || send_to_peer)
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // get the data
    data_len = sizeof(pld) - ONA_MSG_DATA_IDX;
    if(!(PARAM_PTR = parse_ascii_tx_data(PARAM_PTR, pld, &data_len))
      || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //
    
    // extract the message type from the block data payload
    get_block_data_payload_hdr(&data_type, NULL, NULL, NULL, pld);

    return oncli_q_block_request(TRUE, data_type, priority, &dst, src_unit, dst_unit, pld,
      data_len);
#else
    return ONCLI_INTERNAL_ERR;      // this function should not be used in th sniffer front end
#endif
} // block_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the block command and all it's parameters.
    
    The block text command has the form
    
    block text:SRC UNIT:DST UNIT:RAW DST DID:PRIORITY:"abcdef..."
    
    where "abcdef..." is the text to send.  Only the parameters (starting with
    SRC UNIT) are passed in.
    
    \param ASCII_PARAM_LIST ASCII parameter list.  The parameters in this list
      should be seperated by ':'.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_q_block_request for more return values
*/
#ifdef _ENABLE_BLOCK_TEXT_COMMAND
static oncli_status_t block_txt_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
#ifndef _SNIFFER_FRONT_END
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    one_net_raw_did_t dst;

    UInt16 data_len;

    UInt8 src_unit, dst_unit, priority;

    // pld needs to consist of APP HDR + the data
    UInt8 pld[ONCLI_MAX_BLOCK_TXN_LEN + ONA_MSG_HDR_LEN] = {0};
    
    BOOL send_to_peer = FALSE;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    // get the send parameters
    if(!(PARAM_PTR = parse_ascii_tx_param(ASCII_PARAM_LIST, &src_unit,
      &dst_unit, &dst, &priority, &send_to_peer)) || send_to_peer)
    {
        return ONCLI_PARSE_ERR;
    } // if failed parsing parameters //

    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // get the data, but leave room for the block data paylaod header
    data_len = sizeof(pld);
    if(!(PARAM_PTR = parse_ascii_tx_text_data(PARAM_PTR, pld+ONA_BLK_DATA_HDR_DATA_IDX, &data_len))
      || (*PARAM_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data portion failed //

    //
    // fill in the block data payload header since this is the first block
    //
    put_block_data_payload_hdr(ONA_BLOCK_TEXT, data_len+ONA_BLK_DATA_HDR_DATA_IDX,
      src_unit, dst_unit, pld);

    return oncli_q_block_request(TRUE, ONA_BLOCK_TEXT, priority, &dst, src_unit, dst_unit,
      pld, data_len+ONA_BLK_DATA_HDR_DATA_IDX);
#else
    return ONCLI_INTERNAL_ERR;      // this function should not be used in th sniffer front end
#endif
} // block_txt_cmd_hdlr //
#endif


/*!
    \brief Erases the data flash.
    
    \param void
    
    \return ONCLI_SUCCESS if erasing the flash was successful
            ONCLI_CMD_FAIL if erasing the flash failed
            ONCLI_INVALID_CMD_FOR_MODE If the device is in auto mode
*/
#ifdef _ENABLE_ERASE_COMMAND
static oncli_status_t erase_cmd_hdlr(void)
{
#ifdef _AUTO_MODE
    if(mode_type() == AUTO_MODE)
    {
        return ONCLI_INVALID_CMD_FOR_MODE;
    } // if auto mode //
#endif
    if(device_type() != MASTER_NODE && device_type() != CLIENT_NODE)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER and not a CLIENT //

    if(eval_erase_data_flash())
    {
        return ONCLI_SUCCESS;
    } // if erasing the data flash was successful //
    
    return ONCLI_CMD_FAIL;
} // erase_cmd_hdlr //
#endif


/*!
    \brief Saves the current settings to the data flash.
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
            ONCLI_CMD_FAIL if saving the current settings was not successful
            ONCLI_INVALID_CMD_FOR_MODE If the device is in auto mode
*/
#if defined(_ONE_NET_EVAL) && defined(_ENABLE_SAVE_COMMAND)
static oncli_status_t save_cmd_hdlr(void)
{
#ifdef _AUTO_MODE
    if(mode_type() == AUTO_MODE)
    {
        return ONCLI_INVALID_CMD_FOR_MODE;
    } // if auto mode //
#endif
    
    if(device_type() != MASTER_NODE && device_type() != CLIENT_NODE)
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER and not a CLIENT //

    if(eval_save())
    {
        return ONCLI_SUCCESS;
    } // if erasing the data flash was successful //
    
    return ONCLI_CMD_FAIL;
} // save_cmd_hdlr //
#endif


#ifdef _ENABLE_DUMP_COMMAND
/*!
    \brief Dumps data flash. Format: dump:ssss:llll. Where ssss is
    the starting address in hex and llll is the length in hex.
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
*/
static oncli_status_t dump_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    UInt16 start_address;
    UInt16 length;
    
    enum
    {
        DUMP_CMD_ADDRESS_OFFSET = 0,
        DUMP_CMD_ADDRESS_LENGTH = 4,
        DUMP_CMD_SEPARATOR_OFFSET = 4,
        DUMP_CMD_LENGTH_OFFSET = 5,
        DUMP_CMD_LENGTH_LENGTH = 4
    };

    // delimits the starting address and length
    const char DUMP_CMD_SEPARATOR = ':';

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if (*ASCII_PARAM_LIST == '\n')
    {
        // supply default values for start_address and length 
        start_address = DF_BLOCK_A_START;
        length = DF_BLOCK_SIZE/16;
    }
    else if (*(ASCII_PARAM_LIST+DUMP_CMD_SEPARATOR_OFFSET) != DUMP_CMD_SEPARATOR)
    {
        return ONCLI_BAD_PARAM;
    }
    else
    {
        // use the parameters supplied for start address and length
        if (ascii_hex_to_byte_stream(ASCII_PARAM_LIST+DUMP_CMD_ADDRESS_OFFSET,
          (UInt8 *) &start_address, DUMP_CMD_ADDRESS_LENGTH) != DUMP_CMD_ADDRESS_LENGTH)
        {
            start_address = 0;
        }
        else
        {
            // fix the value due to endianess
            start_address = (start_address << 8) | ((start_address >> 8) & 0x00ff);
        }
        
        if (ascii_hex_to_byte_stream(ASCII_PARAM_LIST+DUMP_CMD_LENGTH_OFFSET,
          (UInt8 *) &length, DUMP_CMD_LENGTH_LENGTH) != DUMP_CMD_LENGTH_LENGTH)
        {
            length = 16;
        }
        else
        {
            // fix the value due to endianess
            length = (length << 8) | ((length >> 8) & 0x00ff);
        }
    }

    xdump((UInt8 *) start_address, length);

    return ONCLI_SUCCESS;
} // dump_cmd_hdlr //
#endif


#ifdef _ENABLE_MEMDUMP_COMMAND
/*!
    \brief Dumps volatile memory to UART
    
    The memdump command has the form
    
    memdump:master_param or memdump:base_param or memdump:peer
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
*/
static oncli_status_t memdump_cmd_hdlr(const char* const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr = 0;
    UInt8* startAddress = 0;
	UInt16 length = 0;
	UInt8 * nv_ptr;
	

	const char* const BASE_PARAM_STR = "base_param";
	const char* const PEER_STR = "peer";
    #ifdef _ONE_NET_MASTER
	    BOOL isMaster;
	    const char* const MASTER_PARAM_STR = "master_param";
	    const char* const CLIENT_LIST_STR = "client_list";
		isMaster = oncli_is_master();
	#endif
	
    nv_ptr = oncli_get_param();
	if (nv_ptr == 0)
	{
        return ONCLI_RSRC_UNAVAILABLE_STR;
	}

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if(!strnicmp(PARAM_PTR, BASE_PARAM_STR, strlen(BASE_PARAM_STR)))
    {
        startAddress = (UInt8*) nv_ptr;
        PARAM_PTR += strlen(BASE_PARAM_STR);
    } // dump base_param memory //
#ifdef _PEER
    else if(!strnicmp(PARAM_PTR, PEER_STR, strlen(PEER_STR)))
    {
	#ifdef _ONE_NET_MASTER
	    if(isMaster)
		{
			startAddress =(UInt8*) (nv_ptr + sizeof(on_base_param_t) + sizeof(on_master_t));
			length = ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_t);    
			if(!master_get_peer_assignment_to_save(&startAddress, &length))
			{
				return ONCLI_RSRC_UNAVAILABLE_STR;
			}      
		}
		else
		{
			startAddress =(UInt8*) (nv_ptr + sizeof(on_base_param_t) + sizeof(on_master_t));
			length = ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_t);
		}
	#else
	    startAddress =(UInt8*) (nv_ptr + sizeof(on_base_param_t) + sizeof(on_master_t));
        length = ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_t);	
	#endif
        PARAM_PTR += strlen(PEER_STR);
    } // dump peer memory //
#endif
#ifdef _ONE_NET_MASTER
    else if(isMaster)
	{
        if(!strnicmp(PARAM_PTR, MASTER_PARAM_STR, strlen(MASTER_PARAM_STR)))
		{
			startAddress = (UInt8*)(nv_ptr + sizeof(on_base_param_t));
			length = sizeof(on_master_param_t);
			PARAM_PTR += strlen(MASTER_PARAM_STR);
		}
        else if(!strnicmp(PARAM_PTR, CLIENT_LIST_STR, strlen(CLIENT_LIST_STR)))
		{
			startAddress = (UInt8*)(nv_ptr + sizeof(on_base_param_t));
			length = ONE_NET_MASTER_MAX_CLIENTS * sizeof(on_client_t);
			PARAM_PTR += strlen(MASTER_PARAM_STR);
		}
		else
		{
			return ONCLI_PARSE_ERR;
		}
	}
#endif
    else
    {
        return ONCLI_PARSE_ERR;
    } // else if the priority is invalid //
    
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //


    // right now don't do anything.  Just check the paramters to make sure they work
    oncli_send_msg("Testing for bugs! :  %p %p %d\n", nv_ptr, startAddress, length);
    return ONCLI_SUCCESS;

    // replace the above with the below as soon as the above is verified to work.
    /*if(load_volatile_memory(startAddress, length))
	{
		return ONCLI_SUCCESS;
	}

    return ONCLI_RSRC_UNAVAILABLE_STR;*/
}
#endif


#ifdef _ENABLE_RSINGLE_COMMAND
/*!
    \brief Repeat sending messages
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
*/
static oncli_status_t rsend_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    tick_t ticks;
    oncli_status_t status;
    UInt16 i;

    status = ONCLI_SUCCESS;

    for (i=0; i<20; i++)
    {
        ticks = get_tick_count() + 1000;
        status = single_cmd_hdlr(ASCII_PARAM_LIST);
        if (oncli_is_master() == TRUE)
        {
            while (get_tick_count() < ticks)
            {
                one_net_master();
            }
        }
        else
        {
            while (get_tick_count() < ticks)
            {
                one_net_client();
            }
        }
    }

    return status;
} // rsend_cmd_hdlr //
#endif


#ifdef _ENABLE_RSSI_COMMAND
/*!
    \brief Monitor RSSI
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
*/
static oncli_status_t rssi_cmd_hdlr(void)
{
    tick_t ticks;
    UInt16 read_rssi(void);
    UInt16 rssi;
    UInt8 i;

    i = 1;
    while (1)
    {
        ticks = get_tick_count();
        if ((ticks & 0x0f) == 0)
        {
            while(uart_tx_bytes_free() < uart_tx_buffer_size());
            rssi = read_rssi();
            if (i == 1)
            {
                uart_write("rssi=", 5);
                uart_write_int8_hex((UInt8)((rssi>>8) & 0xff));
                uart_write_int8_hex((UInt8)(rssi & 0xff));
                uart_write(" ", 1);
                uart_write_int8_hex((UInt8)((ticks>>4) & 0xff));
                uart_write(" ", 1);
            }
            if (rssi < -75)
            {
                uart_write("C", 1);
            }
            else
            {
                uart_write("B", 1);
            }
            if ((i++%50) == 0)
            {
                uart_write("\n", 1);
                i = 1;
            }
        }
    }

    return ONCLI_SUCCESS;
} // rsend_cmd_hdlr //
#endif


#ifdef _ENABLE_LIST_COMMAND
/*!
    \brief Prints information about the current configuration
    of the eval board.
    
    \param void
    
    \return ONCLI_SUCCESS if saving the current settings was successful
*/
static oncli_status_t list_cmd_hdlr(void)
{
    one_net_raw_did_t raw_did;
    UInt8 * nv_ptr;
    on_base_param_t * on_base_param;
    on_master_param_t * master_param;
    on_client_t * client_list;
    one_net_raw_did_t tmp_client_did;

    nv_ptr = oncli_get_param();
	if (nv_ptr != (UInt8 *) 0)
	{
	    on_base_param = (on_base_param_t *) nv_ptr;
	}

    oncli_send_msg(ONCLI_STARTUP_FMT, MAJOR_VER, MINOR_VER, 0);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, REVISION_NUM, BUILD_NUM);
    oncli_print_invite(FALSE);
	delay_ms(50);

    // I don't THINK on_base_param would ever be NULL, but testing here anyway.  Getting a warning still.
	if(on_base_param != 0 && (oncli_is_master() == TRUE || client_joined_network))
	{
        // print encryption keys
		oncli_send_msg    ("Non-stream message key : ");
	    oncli_print_xtea_key(&(on_base_param->current_key));
        oncli_send_msg("\n");
		delay_ms(50);
    	#ifdef _STREAM_MESSAGES_ENABLED
		    oncli_send_msg("Stream message key     : ");
	        oncli_print_xtea_key(&(on_base_param->stream_key));			
            oncli_send_msg("\n");
			delay_ms(50);
		#endif
	}

    oncli_print_nid(FALSE);
	
	// 1/25/2010 - now displaying channel for both master and client
    oncli_send_msg("Channel: ");
    oncli_print_channel(FALSE);
		
    if (oncli_is_master() == TRUE)
    {
        //
        // handle master specific output
        //

        //
        // get the device id for a master
        //
        if (get_raw_master_did(&raw_did) != TRUE)
        {
            raw_did[0] = 0x00;
            raw_did[1] = 0x00;
        }
        oncli_send_msg("DID: 0x%03x\n", did_to_u16(&raw_did));

        //
        // print the client list
        //
        if (nv_ptr != (UInt8 *) 0)
        {
            UInt8 i;
    
            master_param = (on_master_param_t *)(nv_ptr + sizeof(on_base_param_t));
            client_list = (on_client_t *)
              (nv_ptr + sizeof(on_base_param_t) + sizeof(on_master_param_t));
            oncli_send_msg("Client count: %d\n", master_param->client_count);
            for (i=0; i<master_param->client_count; i++)
            {
				// Derek_S 11/3/2010 - slight pause so display doesn't get garbled
				delay_ms(40);
                on_decode(&tmp_client_did[0], client_list[i].did, ONE_NET_RAW_DID_LEN);
                oncli_send_msg("  client %d DID: %03x\n", i, did_to_u16(&tmp_client_did));
            }
            delay_ms(100);
        }

#ifdef _PEER
        //
        // print master peer table
        //
        oncli_print_master_peer(FALSE);
#endif
    } // list master specific data //
    else
    {
        //
        // handle client specific output
        //
        
        oncli_send_msg("Client join status: ");
        if (client_joined_network == FALSE)
        {
            //
            // we have not joined the network yet
            //
            oncli_send_msg("Not joined.\n");
        }
        else
        {
            //
            // we have joined the network
            //
            oncli_send_msg("Joined.\n");
            
            //
            // print the device id for a client
            //
            oncli_send_msg("DID: 0x%03x\n", did_to_u16(&client_did));
        }

#ifdef _PEER    
        //
        // print this client's peer list
        //
        if (nv_ptr != (UInt8 *) 0)
        {
            UInt8 index,j,count;
            on_peer_t * peer;
            one_net_raw_did_t raw_did;

            count = 0;
            peer = (on_peer_t *) (nv_ptr + sizeof(on_base_param_t) + sizeof(on_master_t));
            oncli_send_msg(ONCLI_LIST_PEER_TABLE_HEADING);
			
            for (index=0; index < ONE_NET_MAX_PEER_UNIT; index++)
            {
                if (peer->unit[index].peer_unit != ONE_NET_DEV_UNIT)
                {
                    if (on_decode(raw_did, peer->unit[index].peer_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
                    {
                        return ONCLI_INTERNAL_ERR;
                    }
                    oncli_send_msg(ONCLI_LIST_PEER_FMT, 
                      did_to_u16(&client_did), peer->unit[index].src_unit, did_to_u16(&raw_did), 
                      peer->unit[index].peer_unit);
                    delay_ms(100);
                    count++;
                }
            }
            if (count == 0)
            {
                oncli_send_msg(ONCLI_LIST_NO_PEERS);
            }
            delay_ms(100);
        }
#endif
    } // list client specific data //
    oncli_print_user_pin_cfg();
    delay_ms(100);
    oncli_print_prompt();

    return ONCLI_SUCCESS;
} // list_cmd_hdlr //
#endif


#ifdef _ENABLE_IDLE_COMMAND
/*!
    \brief Sets the microcontroller into or idle mode or removes it from idle mode.
    
    The idle command has the form
    
    idle:on or idle:off
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_invite for more possible return values.
*/
static oncli_status_t idle_cmd_hdlr(const char* const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;
    char * end_ptr = 0;
    on_state_t newState;
	
    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
	
    // read in the value to set idle to (on, off)
    if(!strnicmp(PARAM_PTR, ONCLI_ON_STR, strlen(ONCLI_ON_STR)))
    {
        newState = ON_IDLE;
        PARAM_PTR += strlen(ONCLI_ON_STR);
    } // if it should be an input //
    else if(!strnicmp(PARAM_PTR, ONCLI_OFF_STR, strlen(ONCLI_OFF_STR)))
    {
        newState = ON_LISTEN_FOR_DATA;
        PARAM_PTR += strlen(ONCLI_OFF_STR);
    } // else if it should be an output //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else if the priority is invalid //
    
    if(*PARAM_PTR != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the data is not formatted correctly //

    if(set_on_state(newState))
	{
        return ONCLI_SUCCESS;
	}
	
	// if we're "busy" and cannot change state
	return ONCLI_RSRC_UNAVAILABLE_STR;
}
#endif


/*!
    \brief Handles receiving the invite command and all it's parameters.
    
    The invite command has the form
    
    invite:AAAA-BBBB-CCCC-DDDD
    
    where AAAA-BBBB-CCCC-DDDD is the unique key for the CLIENT to invite.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_invite for more possible return values.
*/
#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t invite_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    oncli_status_t status;

    one_net_xtea_key_t key;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = parse_invite_key(ASCII_PARAM_LIST, &key)) != ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the key was not successful //

    return oncli_invite(&key);
} // invite_cmd_hdlr //
#endif


/*!
    \brief Cancels the current invite.
    
    This command is only valid for MASTERs.
    
    \param void

    \return ONCLI_SUCCESS if the command was succesful
            See oncli_cancel_invite for more possible return values.
*/
#ifdef _ENABLE_CANCEL_INVITE_COMMAND
static oncli_status_t cancel_invite_cmd_hdlr(void)
{
    return oncli_cancel_invite();
} // cancel_invite_cmd_hdlr //
#endif


/*!
    \brief Assigns a peer to a CLIENT.

    The assign_peer_cmd_hdlr command has the form

    assign_peer_cmd_hdlr:X:Y:A:B

    where A is the peer DID being assigned, B is the peer unit that is being
    assigned, X is the destination DID who is receiving the assignment, and Y is
    the destination unit receiving the assignment.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_assign_peer for more return values.
*/
#ifdef _ENABLE_ASSIGN_PEER_COMMAND
static oncli_status_t assign_peer_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    one_net_raw_did_t peer_did, src_did;
    
    UInt8 peer_unit, src_unit;

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
    src_unit = strtol(PARAM_PTR, &end_ptr, 16);
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
    peer_unit = strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    return oncli_assign_peer(&src_did, src_unit, &peer_did, peer_unit);
} // assign_peer_cmd_hdlr //
#endif


/*!
    \brief Unassigns a peer to a CLIENT.

    The unassign_peer_cmd_hdlr command has the form

    unassign_peer_cmd_hdlr:X:Y:A:B

    where A is the peer DID being unassigned, B is the peer unit that is being
    unassigned, X is the destination DID who is receiving the assignment, and Y
    is the destination unit receiving the assignment.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_unassign_peer for more return values.
*/
#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
static oncli_status_t unassign_peer_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    one_net_raw_did_t peer_did, src_did;
    
    UInt8 peer_unit, src_unit;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    // read in the source did
    if(ascii_hex_to_byte_stream(PARAM_PTR, src_did, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the raw peer did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the source unit
    src_unit = strtol(PARAM_PTR, &end_ptr, 16);
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
    } // if converting the raw destination did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;

    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    // read in the peer unit
    peer_unit = strtol(PARAM_PTR, &end_ptr, 16);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //

    return oncli_unassign_peer(&peer_did, peer_unit, &src_did, src_unit);
} // unassign_peer_cmd_hdlr //
#endif


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
#ifdef _ENABLE_UPDATE_MASTER_COMMAND
static oncli_status_t update_master_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    one_net_raw_did_t dst;

    BOOL update_master = FALSE;

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

    return oncli_set_update_master_flag(update_master, &dst);
} // update_master_cmd_hdlr //
#endif


/*!
    \brief Changes the given destination's keep aliv time value

    The update keep alive command has the form

    update keep-alive:did:interval

    This is a MASTER only function.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See update_keep_alive for more return values.
*/
#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
static oncli_status_t change_keep_alive_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    one_net_raw_did_t dst;

    UInt32 keep_alive;

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
    keep_alive = strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //

    return oncli_change_keep_alive(keep_alive, &dst);
} // change_keep_alive_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the change fragment delay command and all it's
      parameters.
    
    The change fragment delay command has the form
    
    change fragment delay:did:priority
    
    This is a MASTER only command.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_change_frag_dly for more return values.
*/
#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
static oncli_status_t change_frag_dly_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr;

    one_net_raw_did_t did;

    UInt32 delay;
    
    UInt8 priority;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
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
    delay = strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //
    
    return oncli_change_frag_dly(&did, priority, delay);
} // change_frag_dly_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the change key command and all it's parameters.
    
    The change key command has the form
    
    change key:AA-BB-CC-DD
    
    where AA-BB-CC-DD is the new 4 byte key fragment to add to the end of the
    old key (the top 4 bytes of the key are dropped).  This is a MASTER
    only command.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See change_key for more return values.
*/
#ifdef _ENABLE_CHANGE_KEY_COMMAND
oncli_status_t change_key_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
#ifndef _SNIFFER_FRONT_END
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

    // delimits the unique key into 4 sets of 4 values.
    const char GROUP_DELIMITER = '-';

    one_net_xtea_key_fragment_t key_fragment;

    UInt8 param_idx, key_idx;

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

    return oncli_change_key(&key_fragment);
#else
    return ONCLI_INTERNAL_ERR;      // this function should not be used in th sniffer front end
#endif
} // change_key_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the remove device command and all it's parameters.
    
    \param[in] ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed into this functtion
              are invalid.
*/
#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
static oncli_status_t rm_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    one_net_raw_did_t dst;

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

    return oncli_remove_device(&dst);
} // rm_dev_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the data rate test command and all it's parameters
    
    \param[in] ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            See oncli_start_data_rate_test for more return values.
*/
#ifdef _ENABLE_DATA_RATE_TEST_COMMAND
static oncli_status_t data_rate_test_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    one_net_raw_did_t * src = 0;
    one_net_raw_did_t sender, receiver;

    UInt8 data_rate;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    // read in the sender's did
    if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
    {
        if(ascii_hex_to_byte_stream(PARAM_PTR, sender, ONCLI_ASCII_RAW_DID_SIZE)
          != ONCLI_ASCII_RAW_DID_SIZE)
        {
            return ONCLI_PARSE_ERR;
        } // if converting the sender's did failed //
        PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
        
        src = &sender;
    } // if the source did is given //
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER)
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // read in the receiver's did
    if(ascii_hex_to_byte_stream(PARAM_PTR, receiver, ONCLI_ASCII_RAW_DID_SIZE)
      != ONCLI_ASCII_RAW_DID_SIZE)
    {
        return ONCLI_PARSE_ERR;
    } // if converting the receiver's did failed //
    PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    
    // check the parameter delimiter
    if(*PARAM_PTR++ != ONCLI_PARAM_DELIMITER || !isdigit(*PARAM_PTR))
    {
        return ONCLI_PARSE_ERR;
    } // if malformed parameter //
    
    // get the value
    data_rate = strtol(PARAM_PTR, &end_ptr, 0);
    if(!end_ptr || end_ptr == PARAM_PTR || (*end_ptr != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if parsing the data failed //

    return oncli_start_data_rate_test(src, &receiver, data_rate);
} // data_rate_test_cmd_hdlr //
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
    pin = strtol(PARAM_PTR, &end_ptr, 0);
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
        pin_type = ONCLI_INPUT_PIN;
        PARAM_PTR += strlen(ONCLI_INPUT_STR);
    } // if it should be an input //
    else if(!strnicmp(PARAM_PTR, ONCLI_OUTPUT_STR, strlen(ONCLI_OUTPUT_STR)))
    {
        pin_type = ONCLI_OUTPUT_PIN;
        PARAM_PTR += strlen(ONCLI_OUTPUT_STR);
    } // else if it should be an output //
    else if(!strnicmp(PARAM_PTR, ONCLI_DISABLE_STR, strlen(ONCLI_DISABLE_STR)))
    {
        pin_type = ONCLI_DISABLE_PIN;
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


/*!
    \brief Handles receiving the channel command and all it's parameters
    
    The channel command has the form
    
    channel:NN
    
    where NN is the channel number to sniff.  The channel is 0 based.  This
    command is only valid in MASTER mode.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
#ifdef _ENABLE_CHANNEL_COMMAND
oncli_status_t channel_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    oncli_status_t status;

    UInt8 channel;

    if((status = parse_channel(ASCII_PARAM_LIST, &channel)) != ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the channel was not successful //
    
    return oncli_reset_master_with_channel(oncli_get_sid(), channel);
} // channel_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the join command and all its parameters
    
    The join command has the form
    
    join:TTT:LL:NN
	
	TTT = timeout (in milliseconds)
	LL  = Locale (US or Europe)
	NN  = Channel number
    
    If no parameters, all channels on all locales are scanned, no timeout.
	If no timeout is specified, there is no timeout.
	If a locale is specified, but not channel number, all channels for that
	locale are searched.
	If a locale and a channel are specified, only that channel is listened to.
	
	Example: join:30:US:5
	will look for an invite on US Channel 5 for 30 seconds
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
#if defined(_ENABLE_JOIN_COMMAND) && defined(_ENHANCED_INVITE)
oncli_status_t join_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    // TO DO : right now, doesn't handle locales without channel
	// and doesn't handle it when the locale is not specified
	oncli_status_t status;
	one_net_xtea_key_t* invite_key;
	UInt16 timeout = 0;
    UInt8 low_channel = 0;
	UInt8 high_channel = ONE_NET_MAX_CHANNEL;
    const char * PARAM_PTR = ASCII_PARAM_LIST;
	const char* END_PTR = 0;
	
    // get the unique invite code for this device
    invite_key = (one_net_xtea_key_t *) get_invite_key();

	
	if(*PARAM_PTR == '\n')
    {
		return oncli_reset_client();
	}
	 
    // Get the timeout if it's there
    if(isdigit(*PARAM_PTR))
    {
        timeout = strtol(PARAM_PTR, &END_PTR, 0);
	    PARAM_PTR = END_PTR;
        if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the command isn't formatted properly //
        PARAM_PTR++;
    } // get the timeout time
	
	if(*PARAM_PTR != '\n')
    {
		if((status = parse_channel(PARAM_PTR, &low_channel)) != ONCLI_SUCCESS)
        {
            return status;
        } // if parsing the channel was not successful //
		
		high_channel = low_channel;
	}

#ifdef _STREAM_MESSAGES_ENABLED
    if(one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE),
      eval_encryption(ON_STREAM), low_channel, high_channel, timeout) !=
	    ONS_SUCCESS)
#else
    if(one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE),
      low_channel, high_channel, timeout) != ONS_SUCCESS)
#endif
	{
		return ONCLI_INTERNAL_ERR;
	}
	
	return ONCLI_SUCCESS;
} // join_cmd_hdlr //
#endif




/*!
    \brief Handles receiving the setni command and all it's parameters.
    
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
#ifdef _ENABLE_SETNI_COMMAND
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

    if (*ASCII_PARAM_LIST == '\n')
    {
        // 
        // skip setting the SID and invite code and just
        // print the current SID and invite code calues
        //
        oncli_print_invite(FALSE);
        return oncli_print_nid(TRUE);
    }
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
    mfg_data_segment[ONE_NET_RAW_NID_LEN] &= 0xf0;
    mfg_data_segment[ONE_NET_RAW_NID_LEN] = 0x01;

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

    if (device_type() == MASTER_NODE)
    {
        return oncli_reset_master();
    }
    else
    {
        return oncli_reset_client();
    }
} // setni_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the sniff command and all it's parameters
    
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

    if((status = parse_channel(ASCII_PARAM_LIST, &channel)) != ONCLI_SUCCESS)
    {
        return status;
    } // if parsing the channel was not successful //

    return oncli_reset_sniff(channel);
} // sniff_cmd_hdlr //
#endif


/*!
    \brief Handles receiving the mode command and all it's parameters
    
    The mode command has the form
    
    mode:MODE...
    
    where MODE... is the mode to go to.
    
    \param ASCII_PARAM_LIST ASCII parameter list.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_CMD_FAIL If the command failed.
*/
#if defined(_AUTO_MODE) && defined(_ENABLE_MODE_COMMAND)
oncli_status_t mode_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    UInt16 len = 0;
    UInt8 verbosity;

    if(!ASCII_PARAM_LIST)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if(!strnicmp(ONCLI_QUIET_STR, ASCII_PARAM_LIST, strlen(ONCLI_QUIET_STR)))
    {
        verbosity = ONCLI_QUIET;
        len = strlen(ONCLI_QUIET_STR);
    } // if going to quiet mode //
    else if(!strnicmp(ONCLI_VERBOSE_STR, ASCII_PARAM_LIST,
      strlen(ONCLI_VERBOSE_STR)))
    {
        verbosity = ONCLI_VERBOSE;
        len = strlen(ONCLI_VERBOSE_STR);
    } // else if verbose mode //
    else
    {
        return ONCLI_PARSE_ERR;
    } // else unknown value //

    if(ASCII_PARAM_LIST[len] != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the termination is not correct //

    return oncli_set_verbosity(verbosity);
} // mode_cmd_hdlr //
#endif


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
#ifdef _ENABLE_ECHO_COMMAND
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

    oncli_set_echo(echo);
    return ONCLI_SUCCESS;
} // echo_cmd_hdlr //
#endif


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
    \param[out] dst The destination device.
    \param[out] priority The priority of the transaction.
    \param[out] send_to_peer
      TRUE if the message should be sent to all the peers in the peer list.
      FALSE if the message is sent to a specific device.

    \return If successful, pointer to first characer after the last parameter
      that was parsed, or 0 if parsing was not successful.
*/
static const char * parse_ascii_tx_param(const char * PARAM_PTR,
  UInt8 * const src_unit, UInt8 * const dst_unit, one_net_raw_did_t * const dst,
  UInt8 * const priority, BOOL * const send_to_peer)
{
    char * end_ptr;

    if(!PARAM_PTR || !dst || !priority || !send_to_peer)
    {
        return 0;
    } // if any of the parameters are invalid //

    if (src_unit)
    {
        // read the src unit
        *src_unit = strtol(PARAM_PTR, &end_ptr, 16);
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
        *dst_unit = strtol(PARAM_PTR, &end_ptr, 16);
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

    // read in the raw did if it is there
    if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
    {
        if(ascii_hex_to_byte_stream(PARAM_PTR, *dst, ONCLI_ASCII_RAW_DID_SIZE)
          != ONCLI_ASCII_RAW_DID_SIZE)
        {
            return 0;
        } // if converting the raw destination did failed //
        
        *send_to_peer = FALSE;
        
        // make sure the raw did is skipped
        PARAM_PTR += ONCLI_ASCII_RAW_DID_SIZE;
    } // if the did is present //
    else
    {
        *send_to_peer = TRUE;
    } // else the did is not present //

    // check the parameter delimiter
    if(*PARAM_PTR != ONCLI_PARAM_DELIMITER)
    {
        return 0;
    } // if malformed parameter //
    PARAM_PTR++;

    // read in the priority
    if(!strnicmp(PARAM_PTR, ONCLI_LOW_STR, strlen(ONCLI_LOW_STR)))
    {
        *priority = ONE_NET_LOW_PRIORITY;
        PARAM_PTR += strlen(ONCLI_LOW_STR);
    } // if it's low priority //
    else if(!strnicmp(PARAM_PTR, ONCLI_HIGH_STR, strlen(ONCLI_HIGH_STR)))
    {
        *priority = ONE_NET_HIGH_PRIORITY;
        PARAM_PTR += strlen(ONCLI_HIGH_STR);
    } // else if it's high priority //
    else
    {
        return 0;
    } // else the priority is invalid //

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


/*!
    \brief Parses the key (or portion of the key) from the ASCII parameter.
    
    This function assumes that there are 4 groups
    
    \param[in] ASCII The ASCII representation of the key
    \param[out] key They key that ASCII represents.
    
    \return ONCLI_SUCCESS If the key was successfully parsed
            ONCLI_BAD_PARAM if any of the parameters are invalid
            ONCLI_PARSE_ERR If the ASCII parameter was not formatted correctly.
    Changes August 18, 2008: Use enters two groups of four chars: xxxx-xxxx.
    These are duplicated in the upper two groups. So, for example
    the user enters

    invite:eval-0006

    the key is created as eval-0006-eval-0006

*/

#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t parse_invite_key(const char * ASCII,
  one_net_xtea_key_t * const key)
{
    enum
    {
        // The index after the last character in the last parameter.
        // Changed to make it work with keys of the form xxxx-xxxx
        //END_OF_PARAM_IDX = 19, // previously required xxxx-xxxx-xxxx-xxxx
        END_OF_PARAM_IDX = 9,    // now it requires xxxx-xxxx

        // number of characters in the key that are grouped together.
        KEY_CH_GROUP_SIZE = 4,

        // The position where the group delimiter character occurs (every nth).
        // This is 1 more than the size of the grouping.
        KEY_DELIMITER_POS
    };

    // delimits the unique key into 2 sets of 4 values.
    const char GROUP_DELIMITER = '-';

    UInt8 param_idx, key_idx;

    if(!ASCII || !key)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    if(ASCII[END_OF_PARAM_IDX] != '\n')
    {
        return ONCLI_PARSE_ERR;
    } // if the wrong number of parameters was passed //

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
//#if (defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)) ||\
//  defined(_ENABLE_INVITE_COMMAND)
static oncli_status_t parse_channel(const char * ASCII, UInt8 * const channel)
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

    *channel = strtol(ASCII, &END_PTR, 0) - 1;
    if(!END_PTR || END_PTR == ASCII || (*END_PTR != '\n'))
    {
        return ONCLI_PARSE_ERR;
    } // if the parameter was not valid //
    
    switch(region)
    {
#ifdef _US_CHANNELS
        case ONCLI_US:
        {
            // dje: Cast to eliminate compiler warning for UInt8 < 0
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
            *channel += ONE_NET_EUR_CHANNEL_1;
			// typecast to override "comparison is always false" warning
            if((SInt8) *channel < ONE_NET_MIN_EUR_CHANNEL
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
} // parse_channel //
//#endif


/*!
    \brief Converts a string of ASCCI hex digits to a byte stream.
    
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
    \brief Checks if a given character is a valid ONE-NET unique key character.
    
    Valid unique key for adding devices characters are '2' - '9', and 'A' - 'Z'
    except for 'O' & 'L'.  The key is case sensitive.
*/
BOOL oncli_is_valid_unique_key_ch(const char CH)
{
    return (BOOL)(isalnum(CH) && CH >= '2'
      && ((CH | 0x20) != 'o' && (CH | 0x20) != 'l'));
} // oncli_is_valid_unique_key_ch //



//! @} oncli_hdlr_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} oncli_hdlr

