#ifndef _ONCLI_PORT_H 
#define _ONCLI_PORT_H 


//! \defgroup oncli_port ONE-NET Command Line Interface Port specific
//!   functionality
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
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
    \file oncli_port.h

    \brief Contains declarations for handling ONE-NET Command Line Interface
      commands that affect the application.

    These declarations are needed by the oncli, but must be implemented by the
    application was the commands that require this functionality change the
    state/status of the application.

    The ONE-NET Command Line Interface is an ASCII protocol designed for the
    ONE-NET evaluation boards so a user can easily test and evaluate the
    ONE-NET protocol (MAC layer).
*/


#include "config_options.h"
#include "one_net_types.h"
#include "one_net_application.h"


//==============================================================================
//								CONSTANTS
//! \defgroup oncli_port_const 
//! \ingroup oncli_port
//! @{

//! @} oncli_port_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup oncli_port_typedefs
//! \ingroup oncli_port
//! @{


//! Return codes for oncli handler functions
typedef enum
{
    ONCLI_SUCCESS,                  //!< The command was handled successfully
    ONCLI_BAD_PARAM,                //!< A param passed to the hdlr was invalid
    ONCLI_INTERNAL_ERR,             //!< An internal error occured
    ONCLI_ALREADY_IN_PROGRESS,      //!< The action is already in progress
    ONCLI_RSRC_UNAVAILABLE,         //!< The resource is unavailable
    ONCLI_UNSUPPORTED,              //!< If a request is not supported
    ONCLI_INVALID_CMD,              //!< The command is invalid
    ONCLI_CMD_FAIL,                 //!< If the command was not successful
    ONCLI_INVALID_DST,              //!< The destination did.unit is invalid
    ONCLI_NOT_JOINED,               //!< The device needs to join a network.
    ONCLI_PARSE_ERR,                //!< The cli data is not formatted properly
    ONCLI_SNGH_INTERNAL_ERR,        //!< Encountered a "Should Not Get Here" error.
    ONCLI_ONS_NOT_INIT_ERR          //!< Encountered a ONS_NOT_INIT return code from ONE-NET.
} oncli_status_t;


//! The different verbosity levels the device can have
typedef enum
{
    ONCLI_QUIET,                    //!< quiet mode
    ONCLI_VERBOSE                   //!< verbose mode
} oncli_verbose_t;


//! @} oncli_port_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup oncli_port_pub_var
//! \ingroup oncli_port
//! @{

//! @} oncli_port_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup oncli_port_pub_func
//! \ingroup oncli_port
//! @{


/*!
    \brief Sets the verbosity level.
    
    \param[in] VERBOSITY The verbosity level (see oncli_verbose_t).
    
    \return ONCLI_SUCCESS if the action was successful
            ONCLI_BAD_PARAM if the parameter was invalid
            ONCLI_INVALID_CMD_FOR_NODE if the command is not valid for the node
              type
*/
oncli_status_t oncli_set_verbosity(const UInt8 VERBOSITY);


/*!
    \brief Outputs the prompt
    
    \param void
    
    \return void
*/
void oncli_print_prompt(void);


/*!
    \brief Resets the device to sniff mode.
    
    \param CHANNEL The channel to sniff.
    
    \return ONCLI_SUCCESS if setting to sniff mode was successful
            ONCLI_INVALID_CMD_FOR_MODE if the command is not valid for the mode
              the device is in.
            ONCLI_INTERNAL_ERR if something unexpected happened.
*/
#ifdef _SNIFFER_MODE
	oncli_status_t oncli_reset_sniff(const UInt8 CHANNEL);
#endif


/*!
    \brief Changes a user pin function.
    
    \param[in] pin The user pin number (between 1 & 255) to change.
    \param[in] pin_type The functionality for the pin.  Input, output, or
      disable -- see on_pin_state_t for options
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed in are invalid
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_CMD_FAIL If the command failed.
*/
oncli_status_t oncli_set_user_pin_type(UInt8 pin, on_pin_state_t pin_type);


/*!
      \brief Print the current configuration of the user pins.
 
      For each user pin, print the pin number and whether it is 
      configured as an input, an output, or disabled.

      \return void
*/
void oncli_print_user_pin_cfg(void);


/*!
    \brief Parses and displays a packet

    \param[in] packet_bytes The bytes that make up the packet.
    \param[in] num_bytes The number of bytes in the packet.
    \param[in] enc_keys the block / single keys to check.  If not relevant, set to
                 NULL and set num_keys to 0.
    \param[in] num_enc_keys the number of block / single keys to check.
    \param[in] invite_keys the invite keys to check.  If not relevant, set to
                 NULL and set num_invite_keys to 0.
    \param[in] num_invite_keys the number of invite keys to check.
    \param[in] stream_keys the stream keys to check.  If not relevant, set to
                 NULL and set num_stream_keys to 0.
    \param[in] num_stream_keys the number of stream keys to check.
    
    \return void
*/
#if _DEBUG_VERBOSE_LEVEL < 2
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes);
#else
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes,
  const one_net_xtea_key_t* const enc_keys, UInt8 num_enc_keys,
  const one_net_xtea_key_t* const invite_keys, UInt8 num_invite_keys,
  const one_net_xtea_key_t* const stream_keys, UInt8 num_stream_keys);
#endif



//! @} oncli_port_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli_port


#endif // #ifdef _ONCLI_PORT_H //
