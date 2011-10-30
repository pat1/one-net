//! \addtogroup oncli_str
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
    \file oncli_str.c

    \brief Contains string constants associated with the ONE-NET Command Line
      interface.

    These strings are to be used in a case insensitive manner.
*/

#include "oncli_str.h"


//==============================================================================
//								CONSTANTS
//! \defgroup oncli_str_const
//! \ingroup oncli_str
//! @{
    
    

// Mode strings
#ifdef _AUTO_MODE
//! Auto mode string
const char * const ONCLI_AUTO_MODE_STR = "AUTO MODE";
#endif

#ifdef _SNIFFER_MODE
//! sniffer string
const char * const ONCLI_SNIFFER_STR = "SNIFFER";
#endif

//! Serial mode string
const char * const ONCLI_SERIAL_MODE_STR = "SERIAL MODE";



// Device strings
//! MASTER string
const char * const ONCLI_MASTER_STR = "MASTER";

//! CLIENT string
const char * const ONCLI_CLIENT_STR = "CLIENT";



// Argument strings
//! on argument string
const char * const ONCLI_ON_STR = "on";

//! off argument string
const char * const ONCLI_OFF_STR = "off";



// Command Strings
#ifdef _ENABLE_ECHO_COMMAND
	//! echo command string
	const char * const ONCLI_ECHO_CMD_STR = "echo";
#endif



// Reponse Strings
//! Response sent when the command was successfully parsed.
const char * const ONCLI_CMD_SUCCESS_STR = "OK\n";

//! String to output when an action succeeded
const char * const ONCLI_SUCCEEDED_STR = "succeeded";

//! String to output when an action failed
const char * const ONCLI_FAILED_STR = "failed";

//! String sent to notify that the input is being cleared.
const char * const ONCLI_CLR_INPUT_STR = "Clearing input\n";



// Error formats
//! indicates that the command was not valid
const char * const ONCLI_INVALID_CMD_FMT = "Invalid command \"%s\"\n";

//! Indicates why the command was invalid.
const char * const ONCLI_CMD_FAIL_FMT = "The \"%s\" command failed - %s\n";

//! String that is output when a command
const char * const ONCLI_INVALID_CMD_FOR_DEVICE_FMT
  = "Invalid command for %s\n";

//! Format for outputting an internal error at a location, and informing the
//! user that the device is halting.
const char * const ONCLI_INTERNAL_ERR_FMT
  = "Internal error occurred at %08X.\n";

//! Format to indicate an error occurred, the number indicates what type of
//! error and the address indicates the approximate location.
const char * const ONCLI_GENERAL_ERROR_FMT
  = "Error %d occurred at %08X.\n";
 
//! Format to indicate that a character not in the character set was received.
const char * const ONCLI_RX_INVALID_CH_FMT
  = "An invalid character (0x%02X) was received\n";

//! Format to indicate the output string is too short
const char * const ONCLI_OUTPUT_STR_TOO_SHORT_FMT
  = "Buffer is not big enough; have %u, needed %u\n";



// Error strings
//! String to indicate the operation is already in progress
const char * const ONCLI_IN_PROGRESS_STR = "already in progress";

//! String to indicate that the required resources are unavailable
const char * const ONCLI_RSRC_UNAVAILABLE_STR
  = "Required resources are not available";

//! String to indicate that the request is not supported  
const char * const ONCLI_UNSUPPORTED_STR = "Unsupported request";

//! Indicates that the format was invalid.
const char * const ONCLI_INVALID_FORMAT_STR = "invalid format";

//! Indicates that a "Should Not Get Here" error occurred.
const char * const ONCLI_SNGH_STR = "should not get here error";

//! Indicates a ONE-NET operation was attempted, but ONE-NET was not initialized.
const char * const ONCLI_ONS_NOT_INIT_ERR_STR = "initialization not complete, please try again";

//! String to indicate that an internal error has occured.
const char * const ONCLI_INTERNAL_ERR_STR = "Internal error";

//! String to indicate the destination is not valid
const char * const ONCLI_INVALID_DST_STR
  = "Invalid DID and/or unit";

//! String to indicate the device needs to join a network
const char * const ONCLI_NEED_TO_JOIN_STR
  = "Device needs to join a network first";

//! The length of the parameters exceeds the limit.
const char * const ONCLI_INVALID_CMD_LEN_STR
  = "The command (and parameters) exceeds the max possible length\n";

//! String to indicate loading from the flash failed.
const char * const ONCLI_LOAD_FAIL_STR
  = "Loading from flash failed!  Starting as a new device.  The flash should"
  "be erased\n";
 
//! Fatal error 1
const char * const ONCLI_FATAL_ERR_1_STR = "\nFATAL ERROR: 1\n";  
  
  
  


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
//                              PRIVATE VARIABLES
//! \defgroup oncli_str_pri_var
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup oncli_str_pri_func
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup oncli_str_pub_func
//! \ingroup oncli_str
//! @{


//! @} oncli_str_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup oncli_str_pri_func
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} oncli_str