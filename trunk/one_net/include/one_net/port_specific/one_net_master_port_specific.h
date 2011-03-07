#ifndef _ONE_NET_MASTER_PORT_SPECIFIC_H
#define _ONE_NET_MASTER_PORT_SPECIFIC_H

#include <one_net/port_specific/config_options.h>


//! \defgroup ON_MASTER_port_specific MASTER Specific ONE-NET functionality
//! \ingroup ONE-NET_port_specific 
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
    \file one_net_master_port_specific.h
    \brief MASTER specific ONE-NET declarations.

    These are interfaces for application and hardware specific functions that
    must be implemented for the ONE-NET project to compile (and work).

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include <one_net/port_specific/one_net_master_port_const.h>
#include <one_net/port_specific/one_net_port_specific.h>

#include <one_net/one_net_master.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ON_MASTER_port_specific_const
//! \ingroup ON_MASTER_port_specific
//! @{

//! @} ON_MASTER_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ON_MASTER_port_specific_typedefs
//! \ingroup ON_MASTER_port_specific
//! @{
    
//! @} ON_MASTER_port_specific_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ON_MASTER_port_specific_pub_var
//! \ingroup ON_MASTER_port_specific
//! @{

//! @} ON_MASTER_port_specific_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ON_MASTER_port_specific_pub_func
//! \ingroup ON_MASTER_port_specific
//! @{

/*!
    \brief Alerts the application that a device is awake.
    
    This allows the MASTER to send a message to the device since it knows the
    device is awake since it just received a message from the device.  Otherwise
    the MASTER would have to guess if the device is awake, attempt the
    transaction, and if it failed, it would have to send try sending it again.
    
    \param[in] DID The device that just sent a message.
    
    \return void
*/
void one_net_master_device_is_awake(const one_net_raw_did_t *DID);


/*!
    \brief Callback for the application to handle the received packet.

    Called by ONE-NET when a MASTER receives a packet.  If the packet causes
    the MASTER to send a transaction back to the sender, the MASTER must
    remember the address of the sender and pass those back in when it sends the
    response.

    \param[in] RX_PLD The decoded, decrypted payload that was received.
    \param[in] RX_PLD_LEN The number of bytes received
    \param[in] SRC_ADDR The raw address of the sender.

    \return TRUE If it was a valid packet (and it will be handled)
            FALSE If it was not a valid packet and is being ignored.
*/
BOOL one_net_master_handle_single_pkt(const UInt8 *RX_PLD, UInt16 RX_PLD_LEN, 
                                      const one_net_raw_did_t *SRC_ADDR);


/*!
    \brief The status of a single transaction.

    Callback to report the status of sending an application single data packet.

    \param[in] STATUS The status of the transaction.
    \param[in] RETRY_COUNT The number of times that the packet had to be resent.
    \param[in] DATA The data that was sent.
    \param[in] DST The raw did of where the packet was sent.

    \return void
*/
void one_net_master_single_txn_status(one_net_status_t STATUS,
                  UInt8 RETRY_COUNT, const UInt8 *DATA,
                  const one_net_raw_did_t *DST);


/*!
    \brief Called when a block or stream request is received.

    This allows the app to decide if it is willing and able to send or receive
    the requested block or stream transaction.

    \param[in] TYPE The type of transaction to be performed (ON_BLOCK or STREAM
      only).
    \param[in] SEND TRUE if this device is to send the transaction.
                    FALSE if this device is to receive the transaction.
    \param[in] DATA_TYPE The type of data to be sent or received.
    \param[in] DATA_LEN 0 if a STREAM transaction, else the total number of
       bytes to be sent/received for a block transaction.
    \param[in] DID The device ID of the device making the request.

    \return TRUE if the request is to be granted.
            FALSE if the request is to be ignored.
*/
BOOL one_net_master_txn_requested(UInt8 TYPE, BOOL SEND,
                                  UInt16 DATA_TYPE, UInt16 DATA_LEN,
                                  const one_net_raw_did_t *DID);


/*!
    \brief Callback for the application to handle the received block packet.

    \param[in] PLD The decoded, decrypted payload that was received.
    \param[in] LEN The length of PLD in bytes.
    \param[in] SRC_ADDR The raw address of the sender

    \return TRUE if it was a valid packet (and it will be handled).
            FALSE If it was not a valid packet and is being ignored
*/
BOOL one_net_master_handle_block_pkt(const UInt8 *PLD, UInt16 LEN,
                                     const one_net_raw_did_t *SRC_ADDR);


/*!
    \brief The status of a block transaction.

    \param[in] STATUS The status of the transaction.
    \param[in] DID The device ID of the other device involved in the txn.
    
    \return void
*/
void one_net_master_block_txn_status(one_net_status_t STATUS,
                                     const  one_net_raw_did_t *DID);


/*!
    \brief Callback for the application to handle the received stream packet.

    \param[in] PLD The decoded, decrypted payload that was received.
    \param[in] LEN The length of the PLD in bytes.
    \param[in] SRC_ADDR The raw address of the sender.

    \return TRUE if it was a valid packet (and it will be handled).
            FALSE If it was not a valid packet and is being ignored
*/
BOOL one_net_master_handle_stream_pkt(const UInt8 *PLD, UInt16 LEN,
  const one_net_raw_did_t *SRC_ADDR);


/*!
    \brief Callback to report the status of a stream transaction.

    \param[in] STATUS The status of the transaction.
    \param[in] DID The device ID of the other device involved in the txn.

    \return void
*/
void one_net_master_stream_txn_status(one_net_status_t STATUS,
                                      const one_net_raw_did_t *DID);


/*!
    \brief Callback to retrieve the next payload for a block or stream 
      transaction
    
    The application needs to keep track of where in the block or stream
    transaction it is.  The ONE-NET code will keep track of the last block for
    retransmissions.  This function will be called when the ONE-NET code wants
    to retrieve the next block or stream payload for transmission.  The
    implementer must set len to the number of bytes being sent, and return a
    pointer to the data to be sent.
    
    \param[in] TYPE The type of transaction.  Must be either ON_BLOCK or STREAM
    \param[out] len The number of bytes to be sent
      (must be <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
    \param[in] DST The device receiving the next payload.

    \return Pointer to the data to be sent.
*/
const UInt8 * one_net_master_next_payload(UInt8 TYPE, UInt16 *len,
                                          const one_net_raw_did_t *DST);


/*!
    \brief Returns results of the invite new CLIENT operation.

    \param[in] STATUS SUCCESS if the device was successfully added.
                      TIME_OUT if the operation timed out
    \param[in] KEY The unique key used to add the device.
    \param[in] CLIENT_DID The did the CLIENT was assigned if STATUS == SUCCESS,
      otherwise 0.

    \return void
*/
void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t KEY, const one_net_raw_did_t *CLIENT_DID);


/*!
    \brief Reports the results of the data rate test to the MASTER application.

    If ATTEMPTS is 0, then there was an error in performing the data rate test
    (such was the MASTER did not hear the results reported back in time from the
    sender, or the test was never carried out).  If either the SENDER or
    RECEIVER's address is the broadcast address, then the MASTER was unable to
    encode/decode the DID's and the transaction was aborted.

    \param[in] SENDER The sender of the data rate test transaction.
    \param[in] RECEIVER The receiver of the data rate test transaction.
    \param[in] DATA_RATE The data rate that was tested.
    \param[in] SUCCESS_COUNT The number of successful data rate transactions.
    \param[in] ATTEMPTS The number of data rate transactions that were
      attempted.  If this is 0, then the transaction was not carried out or
      completed.

    \return void
*/
void one_net_master_data_rate_result(const one_net_raw_did_t *SENDER,
  const one_net_raw_did_t *RECEIVER, UInt8 DATA_RATE,
  UInt8 SUCCESS_COUNT, UInt8 ATTEMPTS);


/*!
    \brief Reports the results of a device settings update that the application
      layer initiated.

    \param[in] UPDATE What was being updated.
    \param[in] DID The device that was being updated.
    \param[in] SUCCEEDED TRUE if the operation succeeded.
                         FALSE if the operation failed.

    \return void
*/
void one_net_master_update_result(one_net_mac_update_t UPDATE,
  const one_net_raw_did_t *DID, BOOL SUCCEEDED);


/*!
    \brief Reports the result of sending the remove device message.
    
    If the application wants the device removed from the network, it should
    return TRUE.  If the application does not want the device removed (because
    it is going to try and send the remove device again), it should return
    FALSE
    
    \param[in] DID The device ID of the CLIENT being removed from the network.
    \param[in] SUCCEEDED TRUE if the REMOVE_DEVICE message succeeded.
                         FALSE if the REMOVE_DEVICE admin message failed.
*/
BOOL one_net_master_remove_device_result(const one_net_raw_did_t *DID,
  BOOL SUCCEEDED);


/*!
    \brief Called when settings have been changed that need to be saved.

    This data should be saved into non-volatile memory and passed in to
    initialize ONE-NET when the device starts up.

    \param[in] PARAM The parameters to save in non-volatile memory.
    \param[in] PARAM_LEN The number of bytes to save.

    \return void
*/
void one_net_master_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN);

//! @} ON_MASTER_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ON_MASTER_port_specific

#endif // _ONE_NET_MASTER_PORT_SPECIFIC_H //

