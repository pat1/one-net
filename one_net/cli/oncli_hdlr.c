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

#ifdef _ENABLE_SET_DATA_RATE_COMMAND
oncli_status_t set_data_rate_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_USER_PIN_COMMAND
	static oncli_status_t user_pin_cmd_hdlr(const char * const ASCII_PARAM_LIST);
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
	static oncli_status_t change_key_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_CHANGE_STREAM_KEY_COMMAND
	static oncli_status_t change_stream_key_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _ENABLE_CHANNEL_COMMAND
static oncli_status_t channel_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif

#ifdef _RANGE_TESTING
static oncli_status_t range_test_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif



// temporary debugging
static oncli_status_t add_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST);




// Parsing functions.
static UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const UInt16 NUM_ASCII_CHAR);

static oncli_status_t oncli_parse_channel(const char * ASCII,
  UInt8 * const channel);
  
#ifdef _ENABLE_INVITE_COMMAND
static oncli_status_t parse_invite_key(const char * ASCII,
  char** end_ptr, one_net_xtea_key_t * const key);
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
    else if(!strnicmp(ONCLI_LIST_CMD_STR, CMD, strlen(ONCLI_LIST_CMD_STR)))
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
        *cmd_hdlr = &change_key_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change key command was received //
	#endif
    
	#ifdef _ENABLE_CHANGE_STREAM_KEY_COMMAND
    else if(!strnicmp(ONCLI_CHANGE_STREAM_KEY_CMD_STR, CMD,
      strlen(ONCLI_CHANGE_STREAM_KEY_CMD_STR)))
    {
        *CMD_STR = ONCLI_CHANGE_STREAM_KEY_CMD_STR;

        if(CMD[strlen(ONCLI_CHANGE_STREAM_KEY_CMD_STR)] !=
          ONCLI_PARAM_DELIMITER)
        {
            return ONCLI_PARSE_ERR;
        } // if the end of the command is not valid //

        *next_state = ONCLI_RX_PARAM_NEW_LINE_STATE;
        *cmd_hdlr = &change_stream_key_cmd_hdlr;

        return ONCLI_SUCCESS;
    } // else if the change stream key command was received //
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
    oncli_send_msg("MH Repeater Available : %s\n",
      mh_repeater_available ? TRUE_STR : FALSE_STR);
    #endif
    
    #if defined(_ONE_NET_MASTER) && defined(_ONE_NET_CLIENT)
	if(device_is_master || client_joined_network)
    #elif defined(_ONE_NET_CLIENT)
    if(client_joined_network)
    #endif
	{
        // print encryption keys
		oncli_send_msg    ("Non-stream message key : ");
	    oncli_print_xtea_key(&(on_base_param->current_key));
        oncli_send_msg("\n");
    	#ifdef _STREAM_MESSAGES_ENABLED
		oncli_send_msg("Stream message key     : ");
	    oncli_print_xtea_key(&(on_base_param->stream_key));			
        oncli_send_msg("\n");
		#endif
        oncli_send_msg("\n");
        // print the NID and the DID
        if(oncli_print_sid((on_encoded_sid_t*)(on_base_param->sid)) !=
          ONCLI_SUCCESS)
        {
            return ONCLI_CMD_FAIL;
        }
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
            oncli_print_did(&(client->device_send_info.did));
            oncli_send_msg("\n");
            oncli_send_msg("\n\nSend To Master: %s\n\nFeatures...\n\n",
              client->flags & ON_SEND_TO_MASTER ? TRUE_STR : FALSE_STR);
            oncli_print_features(client->device_send_info.features);
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
    if(device_is_master)
    {
        return ((one_net_master_erase_settings() == ONS_SUCCESS) ?
          ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    }
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
    if(device_is_master)
    {
        return ((one_net_master_save_settings() == ONS_SUCCESS) ?
          ONCLI_SUCCESS : ONCLI_CMD_FAIL);
    }
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






#ifdef _ENABLE_SET_DATA_RATE_COMMAND
#include "tick.h"
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

    if(device_is_master)
    {
        if(one_net_master_add_client(features, &out_base_param,
          &out_master_param) != ONS_SUCCESS)
        {
            return ONCLI_CMD_FAIL;
        }
    }
    else
    {
        on_encode(&(on_base_param->sid[ON_ENCODED_NID_LEN]), did,
          ON_ENCODED_DID_LEN);
        on_encode(on_base_param->sid, add_nid, ON_ENCODED_NID_LEN);
        master->device.features = add_master_features;
        master->device.expected_nonce = ON_INVALID_NONCE;
        master->device.last_nonce = ON_INVALID_NONCE;
        master->device.send_nonce = 0;
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
        one_net_memmove(on_base_param->stream_key, add_key,
          sizeof(one_net_xtea_key_t));
        on_base_param->stream_encrypt = add_stream_encrypt;
    #endif
    #ifdef _BLOCK_MESSAGES_ENABLED
        on_base_param->fragment_delay_low = add_fragment_delay_low;
        on_base_param->fragment_delay_high = add_fragment_delay_high;
    #endif
    
        client_joined_network = TRUE;
        client_looking_for_invite = FALSE;
    }

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
static oncli_status_t rm_dev_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_ASSIGN_PEER_COMMAND
static oncli_status_t assign_peer_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    const char * PARAM_PTR = ASCII_PARAM_LIST;

    char * end_ptr = 0;
    
    on_raw_did_t peer_did, src_did;
    on_encoded_did_t enc_peer_did, enc_src_did;
    
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
    
    if(on_encode(enc_src_did, src_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS ||
      on_encode(enc_peer_did, peer_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        return ONS_INTERNAL_ERR;
    }
    
    if(!is_my_did(&enc_src_did))
    {
        return ONCLI_INTERNAL_ERR; // currently unavailable.
    }
    
    switch(one_net_add_peer_to_list(src_unit, NULL, &enc_peer_did, peer_unit))
    {
        case ONS_SUCCESS: return ONCLI_SUCCESS;
        case ONS_RSRC_FULL: return ONCLI_RSRC_UNAVAILABLE;
        default: return ONCLI_CMD_FAIL;
    }
}
#endif


#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
static oncli_status_t unassign_peer_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_UPDATE_MASTER_COMMAND
static oncli_status_t update_master_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
static oncli_status_t change_keep_alive_cmd_hdlr(
  const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_CHANGE_KEY_COMMAND
static oncli_status_t change_key_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
}
#endif


#ifdef _ENABLE_CHANGE_STREAM_KEY_COMMAND
static oncli_status_t change_stream_key_cmd_hdlr(const char * const ASCII_PARAM_LIST)
{
    return ONCLI_SUCCESS;
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



//! @} oncli_hdlr_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} oncli_hdlr
