//! \addtogroup oncli_str
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

// Application Format
#ifdef _CLOCK_OUT_DIVIDE_BY_TWO
    const char * const ONCLI_STARTUP_FMT = "ONE-NET Clock Test Version %d.%d";
#else
    const char * const ONCLI_STARTUP_FMT = "ONE-NET Evaluation Version %d.%d";
#endif

//! Tail end of the startup banner, because the va_args does not seem to be able to 
//! handle more than two arguments.
const char * const ONCLI_STARTUP_REV_FMT = ".%d (Build %03d)\n";


// Response formats
//! Format output when a device successfully joined the network.
const char * const ONCLI_DEVICE_ADD_FMT
  = "Device %.4s-%.4s added as %03X\n";

//! Format output when a device is not added to the network.
const char * const ONCLI_ADD_DEVICE_FAILED_FMT
  = "Device %.4s-%.4s not added\n";

//! Format output when a CLIENT successfully joins a network
const char * const ONCLI_JOINED_FMT = "Successfully joined network as %03X\n";

//! Format output when a data transaction has been received
const char * const ONCLI_RX_DATA_FMT
  = "Received %s transaction data from %03X, len %u\n";

//! Format output when a text transaction has been received
const char * const ONCLI_RX_TXT_FMT
  = "Received text from %03X:\n%.*s\n";

//! Format output to report the results of a single transaction
const char * const ONCLI_SINGLE_RESULT_FMT
  = "Single transaction with %03X; return status: %s\n";

const char * const ONCLI_BLOCK_RESULT_FMT
  = "Block transaction with %03X; return status: %s\n";

//! Format output to report the results of updating a device's parameters.
const char * const ONCLI_UPDATE_RESULT_FMT = "Updating %s on %03X %s.\n";

//! Format output to report the results of updating a device's parameters
//! without displaying a device id.
const char * const ONCLI_UPDATE_RESULT_WITH_OUT_DID_FMT = "Updating %s %s.\n";

//! Format output to report results of updating a device's parameters where
//! there is no string for the parameter
const char * const ONCLI_UNKNOWN_UPDATE_RESULT_FMT
  = "Updating %02X on %032X %s.\n";
  
//! Format output to report the results of a data rate test
const char * const ONCLI_DATA_RATE_TEST_RESULT_FMT
  = "Device %03X received %u out of %u test transmissions from %03X at %u\n";

//! Format output to report the channel the device is on
const char * const ONCLI_GET_CHANNEL_RESPONSE_FMT = "%s %u\n";

#ifdef _ONE_NET_DEBUG
    //! debug format strings
    const char * const ONCLI_DEBUG_PREFIX_STR = "d[%04x:%02x:";
    const char * const ONCLI_DEBUG_LENGTH_STR = "%02x:";
    const char * const ONCLI_DEBUG_BYTE_STR = "%02x ";
    const char * const ONCLI_DEBUG_SUFFIX_STR = "]\n";
#endif



// Response strings
//! Response sent when the command was successfully parsed.
const char * const ONCLI_CMD_SUCCESS_STR = "OK\n";

//! String to output when an action succeeded
const char * const ONCLI_SUCCEEDED_STR = "succeeded";

//! String to output when an action failed
const char * const ONCLI_FAILED_STR = "failed";

//! String sent to notify that the input is being cleared.
const char * const ONCLI_CLR_INPUT_STR = "Clearing input\n";

//! Message output when a MASTER is removing a device from it's table after
//! the rermove device transaction with the device failed. 
const char * const ONCLI_M_RM_DEV_ANYWAY_STR
  = "Removing device from MASTER table anyhow.\n";

//! Message to output when the channel has not been set yet.
const char * const ONCLI_CHANNEL_NOT_SELECTED_STR = "channel not selected.\n";


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


//! String to display invite code
const char * const ONCLI_DISPLAY_INVITE_STR
  = "Invite code: %.4s-%.4s\n";

// Mode strings
//! Auto mode string
const char * const ONCLI_AUTO_MODE_STR = "AUTO MODE";

//! Serial mode string
const char * const ONCLI_SERIAL_MODE_STR = "SERIAL MODE";


// Device strings
//! MASTER string
const char * const ONCLI_MASTER_STR = "MASTER";

//! CLIENT string
const char * const ONCLI_CLIENT_STR = "CLIENT";

//! sniffer string
const char * const ONCLI_SNIFFER_STR = "SNIFFER";


// Command strings
//! Single command string
const char * const ONCLI_SINGLE_CMD_STR = "single";

//! Single text command string
const char * const ONCLI_SINGLE_TXT_CMD_STR = "single text";

//! Block command string
const char * const ONCLI_BLOCK_CMD_STR = "block";

//! Block text command string
const char * const ONCLI_BLOCK_TXT_CMD_STR = "block text";

//! Erase data flash command string
const char * const ONCLI_ERASE_CMD_STR = "erase";

//! Save data flash command string
const char * const ONCLI_SAVE_CMD_STR = "save";

#ifdef _ENABLE_DUMP_COMMAND
//! Dump data flash command string
const char * const ONCLI_DUMP_CMD_STR = "dump";
#endif

#ifdef _ENABLE_RSINGLE_COMMAND
//! Repeat sending messages
const char * const ONCLI_RSEND_CMD_STR = "rsingle";
#endif

#ifdef _ENABLE_RSSI_COMMAND
//! Start monitoring RSSI
const char * const ONCLI_RSSI_CMD_STR = "rssi";
#endif

#ifdef _ENABLE_LIST_COMMAND

//! Info command string
const char * const ONCLI_LIST_CMD_STR = "list";

//! Format for printing peers in the CLI list command
const char * const ONCLI_LIST_PEER_FMT = "  %03d:%d:%03d:%d\n";

//! Peer table heading string.
const char * const ONCLI_LIST_PEER_TABLE_HEADING = "Peer table:\n";

//! No peers in table string.
const char * const ONCLI_LIST_NO_PEERS = "  No peers.\n";

#endif


// MASTER only command strings
//! Command to invite a specific CLIENT to join the network.
const char * const ONCLI_INVITE_CMD_STR = "invite";

//! Cancel Invite command string
const char * const ONCLI_CANCEL_INVITE_CMD_STR = "cancel invite";

//! Assign peer command string
const char * const ONCLI_ASSIGN_PEER_CMD_STR = "assign peer";

//! Unassign peer command string
const char * const ONCLI_UNASSIGN_PEER_CMD_STR = "unassign peer";

//! Update MASTER command string
const char * const ONCLI_UPDATE_MASTER_CMD_STR = "set update master flag";

//! Change keep alive command string
const char * const ONCLI_CHANGE_KEEP_ALIVE_CMD_STR = "change keep-alive";

//! Change fragment delay string
const char * const ONCLI_CHANGE_FRAGMENT_DELAY_CMD_STR
  = "change fragment delay";

//! Change key command string
const char * const ONCLI_CHANGE_KEY_CMD_STR = "change key";

//! Remove device command string
const char * const ONCLI_RM_DEV_CMD_STR = "remove device";

//! Data Rate Test command string
const char * const ONCLI_DATA_RATE_TEST_CMD_STR = "data rate test";

//! Get Channel command string
const char * const ONCLI_GET_CHANNEL_CMD_STR = "get channel";


// CLIENT only command strings
//! Command to enable or disable user pins
const char * const ONCLI_USER_PIN_CMD_STR = "user pin";


// Mode strings strings
//! Command to restart in CLIENT mode and look for an invite from a MASTER
const char * const ONCLI_JOIN_CMD_STR = "join";

//! Command to restart in MASTER mode on the given channel (with an empty
//! netowrk)
const char * const ONCLI_CHANNEL_CMD_STR = "channel";

//! SETNI command string
const char * const ONCLI_SETNI_CMD_STR = "setni";

//! sniff command string
const char * const ONCLI_SNIFF_CMD_STR = "sniff";

//! mode command string
const char * const ONCLI_MODE_CMD_STR = "mode";

//! echo command string
const char * const ONCLI_ECHO_CMD_STR = "echo";


// Transaction strings
//! string for a single transaction
const char * const ONCLI_SINGLE_TXN_STR = "Single";

//! string for a block transaction
const char * const ONCLI_BLOCK_TXN_STR = "Block";


// Argument strings
//! set argument string
const char * const ONCLI_SET_STR = "set";

//! clear argument string
const char * const ONCLI_CLR_STR = "clr";

//! low argument string
const char * const ONCLI_LOW_STR = "low";

//! high argument string
const char * const ONCLI_HIGH_STR = "high";

//! input argument string
const char * const ONCLI_INPUT_STR = "input";

//! output argument string
const char * const ONCLI_OUTPUT_STR = "output";

//! disable argument string
const char * const ONCLI_DISABLE_STR = "disable";

//! quiet argument string
const char * const ONCLI_QUIET_STR = "quiet";

//! verbose argument string
const char * const ONCLI_VERBOSE_STR = "verbose";

//! on argument string
const char * const ONCLI_ON_STR = "on";

//! off argument string
const char * const ONCLI_OFF_STR = "off";

//! US argument string
const char * const ONCLI_US_STR = "US";

//! European argument string
const char * const ONCLI_EUR_STR = "EUR";


// Transaction type strings
//! Single transaction string
const char * const ONCLI_SINGLE_STR = "Single";

//! Block transaction string
const char * const ONCLI_BLOCK_STR = "Block";


//! Admin message format
const char * const ONCLI_RX_ADMIN_FMT
  = "Received %s %s message %s\n";
  
//! Admin string
const char * const ONCLI_ADMIN_MSG_STR = "Admin";

//! Extended Admin string
const char * const ONCLI_EXTENDED_ADMIN_MSG_STR = "Extended Admin";

//! Admin message string table.
// If this is updated, need to adjust ONCLI_ADMIN_STR_COUNT
const char * const ONCLI_ADMIN_STR[ONCLI_ADMIN_STR_COUNT] =
{
    "STATUS QUERY",
    "STATUS RESPONSE",
    "SETTINGS QUERY",
    "SETTINGS RESPONSE",
    "CHANGE SETTINGS",
    "FRAGMENT DELAY QUERY",
    "FRAGMENT DELAY RESPONSE",
    "CHANGE LOW PRIORITY FRAGMENT DELAY",
    "KEEP ALIVE QUERY",
    "KEEP ALIVE RESPONSE",
    "CHANGE KEEP-ALIVE",
    "NEW KEY FRAGMENT",
    "ASSIGN PEER",
    "UNASSIGN PEER",
    "SEND LOW PRIORITY BLOCK TRANSACTION",
    "RECEIVE LOW PRIORITY BLOCK TRANSACTION",
    "SEND LOW PRIORITY STREAM TRANSACTION",
    "RECEIVE LOW PRIORITY STREAM TRANSACTION",
    "END STREAM",
    "INIT DATA RATE TEST",
    "DATA RATE RESULT",
    "INVALID PACKET 0",
    "INVALID PACKET 1",
    "INVALID PACKET 2",
    "INVALID PACKET 3",
    "INVALID PACKET 4",
    "CHANGE_PEER_DATA_RATE",
    "CHANGE HIGH PRIORITY FRAGMENT DELAY",
    "SEND HIGH PRIORITY BLOCK TRANSACTION",
    "RECEIVE HIGH PRIORITY BLOCK TRANSACTION",
    "SEND HIGH PRIORITY STREAM TRANSACTION",
    "RECEIVE HIGH PRIORITY STREAM TRANSACTION",
    "ASSIGN MULTI-HOP CAPABLE PEER",
    "STREAM KEY QUERY",
    "REMOVE DEVICE"
};

//! Extended Admin message string table.
// If this is updated, need to adjust ONCLI_EXTENDED_ADMIN_STR_COUNT
const char * const ONCLI_EXTENDED_ADMIN_STR[ONCLI_EXTENDED_ADMIN_STR_COUNT] =
{
    "CHANGE STREAM KEY"
};


//! Admin message string table.
// If this is updated, need to adjust ONCLI_M_UPDATE_STR_COUNT
const char * const ONCLI_M_UPDATE_STR[ONCLI_M_UPDATE_STR_COUNT] =
{
    "CLIENT DATA RATE",
    "PEER DATA RATE",
    "NETWORK KEY",
    "ASSIGN PEER",
    "UNASSIGN PEER",
    "REPORT TO MASTER",
    "LOW PRIORITY FRAGMENT DELAY",
    "HIGH PRIORITY FRAGMENT DELAY",
    "KEEP-ALIVE INTERVAL",
    "REMOVE DEVICE"
};


//! ONE-NET Status string tables
const char * const ONCLI_ONS_STR[] =
{
    "SUCCESS",
    "BAD PARAMETER",
    "NOT INITIALIZED",
    "ALREADY IN PROGRESS",
    "INVALID DATA",
    "MORE EXPECTED",
    "COMPLETED",
    "RESOURCE UNAVAILABLE",
    "CANCELED",
    "TIME OUT",
    "INTERNAL ERROR",
    "UNHANDLED VERSION",
    "CRC FAILED",
    "RECEIVED STAY AWAKE ACK",
    "RECEIVED NACK",
    "INCORRECT NONCE",
    "SINGLE TRANSACTION COMPLETE",
    "SINGLE TRANSACTION FAILED",
    "BLOCK TRANSACTION COMPLETE",
    "BLOCK TRANSACTION FAILED",
    "STREAM TRANSACTION COMPLETE",
    "STREAM TRANSACTION FAILED",
    "TRANSACTION QUEUED",
    "TRANSACTION DOES NOT EXIST",
    "BAD RAW PACKET LEN",
    "BAD RAW DATA",
    "BAD ENCODING",
    "INVALID PACKET TYPE",
    "INVALID PACKET",
    "UNHANDLED PACKET",
    "UNICAST ADDRESS",
    "MULTICAST ADDRESS",
    "BROADCAST ADDRESS",
    "BAD ADRESS FORMAT",
    "RECEIVED INCORRECT ADDRESS",
    "NID FAILED",
    "DID FAILED",
    "DEVICE LIMIT",
    "DEVICE HAS NOT JOINED A NETWORK",
    "READ ERROR",
    "WRITE ERROR"
};

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

