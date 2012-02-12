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
    ONCLI_ONS_NOT_INIT_ERR,         //!< Encountered a ONS_NOT_INIT return code from ONE-NET.
    ONCLI_INVALID_CMD_FOR_MODE,     //!< Command is not allowed in the current mode of operation
    ONCLI_INVALID_CMD_FOR_NODE,     //!< Command is invalid for this device type.  Generally
                                    //!< this is a master versus client type of thing.
    ONCLI_BAD_KEY_FRAGMENT          //!< Key fragment is invalid or is already part of the key
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
    \param sniff_time_ms The duration to sniff after the first packet is
                         received.  0 means sniff indefinitely.
    
    \return ONCLI_SUCCESS if setting to sniff mode was successful
            ONCLI_INVALID_CMD_FOR_MODE if the command is not valid for the mode
              the device is in.
            ONCLI_INTERNAL_ERR if something unexpected happened.
*/
#ifdef _SNIFFER_MODE
	oncli_status_t oncli_reset_sniff(const UInt8 CHANNEL, tick_t sniff_time_ms);
#endif



#if _DEBUG_VERBOSE_LEVEL > 0
#if _DEBUG_VERBOSE_LEVEL > 1
/*!
    \brief Displays a DID in verbose fashion.

    \param[in] description The description to prepend in front of the DID
    \param[in] enc_did The encioded DID to display.
    
    \return void
*/
void debug_display_did(const char* const description,
  const on_encoded_did_t* const enc_did);


/*!
    \brief Displays an NID in verbose fashion.

    \param[in] description The description to prepend in front of the NID
    \param[in] enc_nid The encioded NID to display.
    
    \return void
*/
void debug_display_nid(const char* const description,
  const on_encoded_nid_t* const enc_nid);
#endif
#endif



//! @} oncli_port_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli_port


#endif // #ifdef _ONCLI_PORT_H //
