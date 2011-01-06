#ifndef _ONCLI_PORT_H 
#define _ONCLI_PORT_H 


//! \defgroup oncli_port ONE-NET Command Line Interface Port specific
//!   functionality
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

#ifdef _ENABLE_CLI

#include "oncli_port_const.h"
#include "one_net_application.h"

#include "pal.h"


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

typedef enum
{
    ONCLI_INPUT_PIN = INPUT,        //!< Value when setting a pin as an input
    ONCLI_OUTPUT_PIN = OUTPUT,      //!< Value when setting a pin as an output
    ONCLI_DISABLE_PIN = 2           //!< Indicates the pin is not being used
} oncli_pin_t;


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
	#ifdef _AUTO_MODE
    	ONCLI_INVALID_CMD_FOR_MODE,     //!< cmd is unavailable in the current mode
	#endif
    ONCLI_INVALID_CMD_FOR_NODE,     //!< cmd is unavailable for current node
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
    \brief Returns the function to use to send a single data packet.
    
    This function is application specific and will need to be implemented in
    the application layer.
    
    \param void
    
    \return The function to use to send a single data packet, or 0 if a single
      data packet should not be sent.
*/
one_net_send_single_func_t oncli_get_send_single_txn_func(void);


/*!
    \brief Queues a block data request and saves the data to be sent.
    
    Sets the block data passed in from the command line so the application can
    return it during calls to get_next_payload.

    \param[in] SEND TRUE if this device is going to send the data.
                    FALSE if this device is going to receive the data
    \param[in] DATA_TYPE The type of block data to be transferred.
    \param[in] PRIORITY The priority of the block transaction.
    \param[in] DID The other device involved in the transaction.
    \param[in] SRC_UNIT The unit originating the request.
    \param[in] DATA The data to set.
    \param[in] LEN The size of DATA in bytes.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out
              because the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_q_block_request(BOOL SEND, UInt16 DATA_TYPE,
  UInt8 PRIORITY, const one_net_raw_did_t *DID,
  UInt8 SRC_UNIT, UInt8 DST_UNIT, const UInt8 * DATA, UInt16 LEN);


/*!
    \brief Invites a CLIENT to join the network.
    
    This function is only valid if the device is in MASTER mode.
    
    \param[in] KEY The unique key of the device being added to the network.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_invite(const one_net_xtea_key_t *KEY);


/*!
    \brief Cancels an invite request.
    
    This command is only valid if the device is in MASTER mode.
    
    \return ONCLI_SUCCESS If the invite was canceled
            ONCLI_INTERNAL_ERR If something unexpected occured
            ONCLI_INVALID_CMD_FOR_NODE if the command is not valid for the node type
*/
#ifdef _ENABLE_CANCEL_INVITE_COMMAND
oncli_status_t oncli_cancel_invite(void);
#endif


/*!
    \brief Assigns a peer device.
    
    This command is only valid if the device is in MASTER mode.

    \param[in] PEER_DID The peer device being assigned.
    \param[in] PEER_UNIT The unit on the peer device being assigned.
    \param[in] SRC_DID The device being assigned the peer.
    \param[in] SRC_UNIT The unit on the device being assigned the peer.

    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_assign_peer(const one_net_raw_did_t *PEER_DID,
  UInt8 PEER_UNIT, const one_net_raw_did_t *SRC_DID,
  UInt8 SRC_UNIT);

/*!
    \brief Unassigns a peer device.
    
    This command is only valid if the device is in MASTER mode.

    \param[in] PEER_DID The peer device being unassigned.
    \param[in] PEER_UNIT The unit on the peer device being unassigned.
    \param[in] SRC_DID The device being unassigned the peer.
    \param[in] SRC_UNIT The unit on the device being unassigned the peer.

    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_unassign_peer(const one_net_raw_did_t *PEER_DID,
  UInt8 PEER_UNIT, const one_net_raw_did_t *SRC_DID,
  UInt8 SRC_UNIT);


/*!
    \brief Sets (or clears) the update MASTER flag for a given device.
    
    This command is valid only when the device is in MASTER mode.
    
    \param[in] SET TRUE to set the flag.
                   FALSE to clear the flag
    \param[in] DST The device to update.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_set_update_master_flag(BOOL SET,
  const one_net_raw_did_t *DST);


/*!
    \brief Changes the keep alive interval for a given CLIENT
    
    This command is valid only when the device is in MASTER mode.
    
    \param[in] KEEP_ALIVE The interval to set (in ms)
    \param[in] DST The device to update.
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_change_keep_alive(UInt32 KEEP_ALIVE,
  const one_net_raw_did_t *DST);


/*!
    \brief Sets the fragment delay at a given priority for a device.
    
    This command is only valid in MASTER mode.
    
    \param[in] DID The device whose fragment delay is being updated.
    \param[in] PRIORITY The priority of the fragment delay that is being updated
    \param[in] DLY The new fragment delay (in ms).
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_BAD_PARAM If any of the parameters passed into this function
              are invalid.
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_change_frag_dly(const one_net_raw_did_t *DID,
  UInt8 PRIORITY, UInt32 DLY);


/*!
    \brief Changes the key.
    
    This command is only valid if the device is in MASTER mode
    
    \param[in] FRAGMENT The new key fragment to append to the old key.
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed in are invalid
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_PARSE_ERR If the cli command/parameters are not formatted
              properly.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_change_key(
  const one_net_xtea_key_fragment_t *FRAGMENT);


/*!
    \brief Sent to a device to remove it from the network.
    
    This command is valid only when the device is in MASTER mode.
    
    \param[in] DST The device being removed
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed in are invalid
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_remove_device(const one_net_raw_did_t *DST);


/*!
    \brief Starts a data rate test between two devices.
    
    This command is valid only when the device is in MASTER mode.
    
    \param[in] SENDER The sending device of the data rate test.
    \param[in] RECEIVER The receiving device of the data rate test.
    \param[in] DATA_RATE The data rate being tested (see data_rate_t for values)
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed in are invalid
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
*/
oncli_status_t oncli_start_data_rate_test(
  const one_net_raw_did_t *SENDER,
  const one_net_raw_did_t *RECEIVER, UInt8 DATA_RATE);


/*!
    \brief Prints the channel the MASTER is on.
    
    If the MASTER has not yet chosen a channel, prints "Channel not selected."
    
    \param[in] prompt_flag If TRUE print a prompt, otherwise don't.
    
    \return ONCLI_SUCCESS If the message was successfully output.
            ONCLI_INVALID_CMD_FOR_DEVICE if the command is not valid for the
              current mode of the device.
*/
oncli_status_t oncli_print_channel(BOOL prompt_flag);


/*!
    \brief Prints the invite code.
    
    
    \param[in] prompt_flag If TRUE print a prompt, otherwise don't.
    
    \return ONCLI_SUCCESS If the message was successfully output.
            ONCLI_INVALID_CMD_FOR_DEVICE if the command is not valid for the
              current mode of the device.
*/
oncli_status_t oncli_print_invite(BOOL prompt_flag);

/*!
    \brief Prints the NID.
    
    
    \param[in] prompt_flag If TRUE print a prompt, otherwise don't.
    
    \return ONCLI_SUCCESS If the message was successfully output.
            ONCLI_INVALID_CMD_FOR_DEVICE if the command is not valid for the
              current mode of the device.
*/
oncli_status_t oncli_print_nid(BOOL prompt_flag);


/*!
    \brief Changes a user pin function.
    
    This command is only valid if the device is in CLIENT mode.
    
    \param[in] PIN The user pin number (between 1 & 255) to change.
    \param[in] PIN_TYPE The functionality for the pin.  See direction_t for
      values.
    
    \return ONCLI_SUCCESS if the command was successful
            ONCLI_BAD_PARAM If any of the parameters passed in are invalid
            ONCLI_INVALID_CMD_FOR_DEVICE If the command is not valid for the
              current mode of the device.
            ONCLI_RSRC_UNAVAILABLE If the command can not be carried out because
              the resource is full or doesn't exist.
            ONCLI_CMD_FAILED If the command failed.
*/
oncli_status_t oncli_set_user_pin_type(const UInt8 PIN, const UInt8 PIN_TYPE);


/*!
    \brief Resets the device to CLIENT mode.
    
    When the device is reset to CLIENT mode, it is not part of the network, and
    will need to be added to a network using it's unique key.
    
    \param void
    
    \return ONCLI_SUCCESS if the command was succesful
            ONCLI_INTERNAL_ERR If something unexpected occured
            ONCLI_INVALID_CMD_FOR_MODE If the command is not valid in the
              given mode
*/
oncli_status_t oncli_reset_client(void);



/*!
    \brief Resets the device to MASTER mode.
    
    When the device is reset to MASTER mode, the network is empty and CLIENT
    will need to be added to the network using their unique key.
    
    \return ONCLI_SUCCESS If reseting to MASTER mode was successful
            ONCLI_BAD_PARAM If any of the parameters were invalid
            ONCLI_CMD_FAIL If the command failed
            See set_device_t for more return values.
*/
oncli_status_t oncli_reset_master(void);


/*!
    \brief Resets the device to MASTER mode on the given channel.
    
    When the device is reset to MASTER mode, the network is empty and CLIENT
    will need to be added to the network using their unique key.
    
    \param[in] SID The system ID to use.  If this value is NULL, a SID is
      chosen for the caller.
    \param[in] CHANNEL The channel for the MASTER to run on.
    
    \return ONCLI_SUCCESS If reseting to MASTER mode was successful
            ONCLI_BAD_PARAM If any of the parameters were invalid
            See set_device_t for more return values.
*/
oncli_status_t oncli_reset_master_with_channel(
  const one_net_raw_sid_t *SID, UInt8 CHANNEL);


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
    \brief Sets the verbosity level.
    
    \param[in] VERBOSITY The verbosity level (see oncli_verbose_t).
    
    \return ONCLI_SUCCESS if the action was successful
            ONCLI_BAD_PARAM if the parameter was invalid
            ONCLI_INVALID_CMD_FOR_NODE if the command is not valid for the node
              type
*/
oncli_status_t oncli_set_verbosity(const UInt8 VERBOSITY);


/*!
    \brief Returns the devices current raw SID.
    
    \param void
    
    \return The devices current SID
*/
const one_net_raw_sid_t * oncli_get_sid(void);


/*!
    \brief Returns the string representation of the node type the device is
      currently set to.

    \param void
    
    \return The string representing the node type.  0 if an error occured.
*/
const char * oncli_node_type_str(void);


/*!
    \brief Returns the string representation of the mode type the device
      is currently operating at.

    \param void

    \return The string representing the mode type.  0 if an error occured.
*/
#ifdef _AUTO_MODE
	const char * oncli_mode_type_str(void);
#endif


/*!
    \brief Returns TRUE if the device is operating as a MASTER
    
    \param void
    
    \return TRUE if the device is operating as the MASTER.
            FALSE if the device is not operating as the MASTER
*/
BOOL oncli_is_master(void);


/*!
    \brief Returns TRUE if the device is operating as a CLIENT.
    
    \param void
    
    \return TRUE if the device is operating as a CLIENT.
            FALSE if the device is not operating as a CLIENT.
*/
BOOL oncli_is_client(void);


/*!
    \brief Outputs the prompt
    
    \param void
    
    \return void
*/
void oncli_print_prompt(void);


/*!
 *     \brief Print the current configuration of the user pins.
 *
 *     For easc user pin, print the pin number and whether it is 
 *     configured as an input, an output, or disabled.
 *     \param 
 *     \return 
 */
oncli_print_user_pin_cfg(void);

// TODO: RWM: add function comment
oncli_status_t oncli_print_master_peer(BOOL prompt_flag);


//! @} oncli_port_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli_port

#endif // #ifdef _ENABLE_CLI

#endif // #ifdef _ONCLI_PORT_H //

