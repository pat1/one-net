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

#include "config_options.h"
#include "oncli_str.h"
#include "one_net_data_rate.h"


//==============================================================================
//								CONSTANTS
//! \defgroup oncli_str_const
//! \ingroup oncli_str
//! @{
   
   

// Application Format
const char * const ONCLI_STARTUP_FMT = "ONE-NET Evaluation Version %d.%d";

//! Tail end of the startup banner, because the va_args does not seem to be able to 
//! handle more than two arguments.
//! Oct. 31 -- This appears to no longer be true?  Keeping it as it is though.
const char * const ONCLI_STARTUP_REV_FMT = ".%d (Build %03d)\n";   

    

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



// Region Strings -- add as more regions are added (so far only US and Europe)
#ifdef _US_CHANNELS
//! US argument string
const char * const ONCLI_US_STR = "US";
#endif

#ifdef _EUROPE_CHANNELS
//! European argument string
const char * const ONCLI_EUR_STR = "EUR";
#endif



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

#ifdef _ENABLE_LIST_COMMAND
	//! Info command string
	const char * const ONCLI_LIST_CMD_STR = "list";
#ifdef _PEER
	//! Format for printing peers in the CLI list command
	const char * const ONCLI_LIST_PEER_FMT = "  %03d:%d:%03d:%d\n";

	//! Peer table heading string.
	const char * const ONCLI_LIST_PEER_TABLE_HEADING = "Peer table:\n";

	//! No peers in table string.
	const char * const ONCLI_LIST_NO_PEERS = "  No peers.\n";
#endif
#endif

#if defined(_SNIFFER_MODE) && defined(_ENABLE_SNIFF_COMMAND)
//! sniff command string
const char * const ONCLI_SNIFF_CMD_STR = "sniff";
#endif

#ifdef _ENABLE_ERASE_COMMAND
	//! Erase data flash command string
	const char * const ONCLI_ERASE_CMD_STR = "erase";
#endif

#ifdef _ENABLE_SAVE_COMMAND
	//! Save data flash command string
	const char * const ONCLI_SAVE_CMD_STR = "save";
#endif

#ifdef _DATA_RATE
    //! Sets the data rate of a device
    const char * const ONCLI_SET_DATA_RATE_CMD_STR = "set data rate";
#endif

#ifdef _ENABLE_USER_PIN_COMMAND
//! Command to enable or disable user pins
const char * const ONCLI_USER_PIN_CMD_STR = "user pin";

//! input argument string
const char * const ONCLI_INPUT_STR = "input";

//! output argument string
const char * const ONCLI_OUTPUT_STR = "output";

//! disable argument string
const char * const ONCLI_DISABLE_STR = "disable";
#endif


// Master only commands
#ifdef _ENABLE_INVITE_COMMAND
	//! Command to invite a specific CLIENT to join the network.
	const char * const ONCLI_INVITE_CMD_STR = "invite";
#endif

#ifdef _ENABLE_CANCEL_INVITE_COMMAND
	//! Cancel Invite command string
	const char * const ONCLI_CANCEL_INVITE_CMD_STR = "cancel invite";
#endif

#ifdef _ENABLE_ASSIGN_PEER_COMMAND
	//! Assign peer command string
	const char * const ONCLI_ASSIGN_PEER_CMD_STR = "assign peer";
#endif

#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
	//! Unassign peer command string
	const char * const ONCLI_UNASSIGN_PEER_CMD_STR = "unassign peer";
#endif

#ifdef _ENABLE_UPDATE_MASTER_COMMAND
	//! Update MASTER command string
	const char * const ONCLI_UPDATE_MASTER_CMD_STR = "set update master flag";
#endif

#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
	//! Change keep alive command string
	const char * const ONCLI_CHANGE_KEEP_ALIVE_CMD_STR = "change keep-alive";
#endif

#ifdef _ENABLE_CHANGE_KEY_COMMAND
	//! Change key command string
	const char * const ONCLI_CHANGE_KEY_CMD_STR = "change key";
#endif

#ifdef _ENABLE_CHANGE_STREAM_KEY_COMMAND
	//! Change stream key command string
	const char * const ONCLI_CHANGE_STREAM_KEY_CMD_STR = "change stream key";
#endif

#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
	//! Remove device command string
	const char * const ONCLI_RM_DEV_CMD_STR = "remove device";
#endif

#ifdef _ENABLE_CHANNEL_COMMAND
	//! Command to restart in MASTER mode on the given channel
    //! (with an empty network)
	const char * const ONCLI_CHANNEL_CMD_STR = "channel";
#endif

#ifdef _RANGE_TESTING
    //! Command to adjust range testing
    const char* const ONCLI_RANGE_TEST_CMD_STR = "range test";
    
    //! "add" argument
    const char* const ADD_STR = "add";
    
    //! "remove" argument
    const char* const REMOVE_STR = "remove";
    
    //! "clear" argument
    const char* const CLEAR_STR = "clear";
    
    //! "display" argument
    const char* const DISPLAY_STR = "display";
#endif

#ifdef _RANGE_TESTING
	//! Command set the mh_repeater_avaialable variable true or false.
	const char * const ONCLI_MH_REPEAT_CMD_STR = "mh_repeat";
#endif



// Response Formats

//! Format output to report the channel the device is on
const char * const ONCLI_GET_CHANNEL_RESPONSE_FMT = "%s %u\n";

#ifdef _ONE_NET_MASTER
//! Format output when a device successfully joined the network.
const char * const ONCLI_DEVICE_ADD_FMT
  = "Device %.4s-%.4s added as %03X\n";

//! Format output when a device is not added to the network.
const char * const ONCLI_DEVICE_NOT_ADDED_FMT
  = "Device %.4s-%.4s not added.  Invite %s\n";
#endif

#ifdef _ONE_NET_CLIENT
//! Format output when a CLIENT successfully joins a network
const char * const ONCLI_JOINED_FMT = "Successfully joined network as %03X\n";
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

//! Message to output when the channel has not been set yet.
const char * const ONCLI_CHANNEL_NOT_SELECTED_STR = "channel not selected";

//! Message to output when the channel has not been set yet.
const char * const ONCLI_CHANNEL_INVALID_STR = "invalid channel";




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


#ifdef _ONE_NET_CLIENT
//! String to display invite code
const char * const ONCLI_DISPLAY_INVITE_STR = "Invite code: %.4s-%.4s\n";
#endif


// Data Rate Strings
const char* const DATA_RATE_STR[ONE_NET_DATA_RATE_LIMIT] =
{
    "38,400",
    "76,800",
    "115,200",
    "153,600",
    "192,000",
    "230,400"
};

const char* const CAPABLE_STR = "Capable";
const char* const NOT_CAPABLE_STR = "Not Capable";
const char* const TRUE_STR = "True";
const char* const FALSE_STR = "False";




  
  
  


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
//                              PUBLIC VARIABLES
//! \defgroup oncli_str_pub_var
//! \ingroup oncli_str
//! @{





//! @} oncli_str_pub_var
//                              PUBLIC VARIABLES END
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
