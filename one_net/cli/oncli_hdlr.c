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




//==============================================================================
//								CONSTANTS
//! \defgroup oncli_hdlr_const
//! \ingroup oncli_hdlr
//! @{


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


// Command handlers.
#ifdef _ENABLE_ECHO_COMMAND
	static oncli_status_t echo_cmd_hdlr(const char * const ASCII_PARAM_LIST);
#endif
#ifdef _ENABLE_LIST_COMMAND 
	static oncli_status_t list_cmd_hdlr(void);
#endif

// Parsing functions.
static UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const UInt16 NUM_ASCII_CHAR);


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
    
    else
    {
        *CMD_STR = CMD;
        return ONCLI_INVALID_CMD;
    } // else the command was invalid //
    

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
    
    \return ONCLI_SUCCESS if saving the current settings was successful
*/
static oncli_status_t list_cmd_hdlr(void)
{
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);
    return ONCLI_SUCCESS;
} // list_cmd_hdlr //
#endif


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


//! @} oncli_hdlr_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} oncli_hdlr
