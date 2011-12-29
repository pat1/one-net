#ifndef _ONE_NET_MASTER_PORT_SPECIFIC_H
#define _ONE_NET_MASTER_PORT_SPECIFIC_H

#include "config_options.h"


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

#include "one_net_master_port_const.h"
#include "one_net_port_specific.h"
#include "one_net_application.h"
#include "one_net_master.h"



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
    \brief Callback for the application to handle status messages.

    This function is application dependent.  It is called by ONE-NET when a
    MASTER receives a status message, either alone or as a response to a query
	for the current status of a unit on another device.
	

    \param[in] raw_pld the raw un-parsed 5 byte single payload 
    \param[in] msg_class ONA_STATUS or ONA_STATUS_QUERY_RESP or ONA_STATUS_FAST_QUERY_RESP or ONA_STATUS_ACK_RESP
    \param[in] msg_type the unit type of the destination unit
    \param[in] src_unit the source unit number(this is the unit that the msg_data value correspons to
	\param[in] dst_unit the destination unit
	\param[in] msg_data the data in the incoming message and possibly in the outgoing message
	\param[in] src_addr the raw address of the device sending the status message
	\param[in/out] nack_reason Any nack reason than has occurred already or that will be set by this function
	\param[in/out] ack_nack_handle If a payload is to be set, this denotes how it should be interpreted.
	\param[in/out] ack_nack_payload Any information beyond a nack reason that is either passed to this function
	               or is returned from it.

	\return TRUE if the transaction should be "handled" further.
	        FALSE if any ahndling should be cancelled and the applciation code plans to take
			      over all handling of the packet.
*/
BOOL one_net_master_handle_status_msg(const UInt8* const raw_pld, const ona_msg_class_t msg_class,
       const ona_msg_type_t msg_type, const UInt8 src_unit,
       const UInt8 dst_unit, const UInt16 msg_data,
	   const one_net_raw_did_t* const src_addr,
	   on_nack_rsn_t* const nack_reason, on_ack_nack_handle_t* const ack_nack_handle,
	   ack_nack_payload_t* const ack_nack_payload);


/*!
    \brief Callback for the application to handle an ack or a nack.

    This function is application dependent.  It is called by ONE-NET when a
    MASTER receives an ACK or a NACK.  ONE-NET provides default handling for
	query responses.  In addition, the application code can change a NACK reason or
	change an ACK to a NACK or vice versa.  It can also change the number of retries
	to either prolong the number of retries or decrease them.  The application can
	provide its own handling by returning false.  If it wants to abort the transaction
	for whatever reason, either because it has its own handling or because it does not
	want to handle the message at all, it should return false.
	
    \param[in] msg_type The type of message(application, admin, or extended admin)
    \param[in/out] The payload that was originally sent and which will possibly be sent again.
    \param[in] payload_len length of the payload
	\param[out] need_txn_payload TRUE if the function needs the raw original transaction payload
	            FALSE if it does not.  This function will originally be called WITHOUT being
				provided the raw original transaction payload.  Due to the overhead of encrypting
				and decrypting, this will not be done if it's not necessary.  If the function is
				passed a NULL payload and it needs that payload, it should set the need_txn_payload
				to true and return true.  ONE-NET will re-call the function, providing the raw
				transaction payload.
    \param[in/out] retries the number of times this message has been sent
	\param[in] src_did the did of the device we received this ACK or NACK from.
	\param[in/out] The reason for the NACK.  If this is null or equals ON_NACK_RSN_NO_ERROR
	               then we received an ACK, not a NACK.
	\param[in/out] ack_nack_handle The way the data(if any) provided by the ACk or NACK should be
	               interpreted.
	\param[in/out] ack_nack_payload The data(if any) sent with the ACK/NACK message.
	
    \return TRUE If ONE-NET should proceed with any further handling of the transaction
            FALSE If ONE-NET should consider the transaction complete for whatever reason.
*/
BOOL one_net_master_handle_ack_nack_response(const ona_msg_type_t msg_type,
    UInt8* const payload, const UInt8 payload_len, BOOL* const need_txn_payload,
    UInt8* const retries,
	const one_net_raw_did_t* const src_did, on_nack_rsn_t* const nack_reason,
	on_ack_nack_handle_t* const ack_nack_handle,
	ack_nack_payload_t* const ack_nack_payload);



/*!
    \brief Callback for the application to handle the received packet.

    This function is application dependent.  It is called by ONE-NET when a
    MASTER receives a packet.  ONE-NET provides default handling for
	query messages.  In addition, the application code can fill in a NACK reason
	if needed.  The application can provide its own handling by setting the
	useDefaultHandling parameter to false.  If it cannot send an immediate
	response, it should set useDefaultHandling to false.
	
    \param[in] raw_pld the raw un-parsed 5 byte single payload 
    \param[in] msg_class the type of the payload (ONA_STATUS, ONA_QUERY, ONA_POLL or ONA_COMMAND)
    \param[in] msg_type the unit type of the destination unit
    \param[in] src_unit the source unit number
	\param[in] dst_unit the destination unit
	\param[in/out] msg_data the data in the incoming message and possibly in the outgoing message
	\param[in] SRC_ADDR the raw address of the source
	\param[out] useDefaultHandling flag set that is read by ONE-NET.  If true, then ONE-NET will use
	            its default handling for queries
	            If false, the application provides its own handling
	\param[in/out] nack_reason The "reason" that should be given if a NACK needs to be sent
	\param[out] ack_nack_handle Filled in by application code if sending data with the ACK/NACK message
	\param[out] ack_nack_payload Filled in by application code if sending data with the ACK/NACK message
	
    \return TRUE If it was a valid packet (and it will be handled)
            FALSE If it was an invalid packet and a NACK should be sent
*/
BOOL one_net_master_handle_single_pkt(const UInt8* const raw_pld,
       const ona_msg_class_t msg_class, ona_msg_type_t* const msg_type,
	   const UInt8 src_unit, const UInt8 dst_unit, UInt16* const msg_data,
       const one_net_raw_did_t* const SRC_ADDR,
	   BOOL* const useDefaultHandling, on_nack_rsn_t* const nack_reason,
	   on_ack_nack_handle_t* const ack_nack_handle,
	   ack_nack_payload_t* const ack_nack_payload);
	   

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
\brief Callback to check if device should grant the requested transaction

\param[in] TYPE The type of transaction being requested.  Currently only
  handles ON_BLOCK
\param[in] SEND TRUE if this device is the sender of the data.  Currently
  only handles FALSE.
\param[in] DATA_TYPE The type of data being requested
\param[in] DATA_LEN The number of bytes to be transferred.  If the
               application rejects this value, it can set this value
               to one tht it WILL accept if re-requested.  However,
               it should not change THIS variable, but instead
               place the new value in the ack_nack_payload.
\param[in] DID The other end of the transaction.
\param[in] src_unit The source unit for this transaction.
\param[in] dst_unit The destination unit for this transaction.
\param[in/out] nack_reason Reason for rejection, if any.
\param[in/out] ack_nack_handle "Handle" of the ack_nack_payload, if any.
\param[in/out] ack_nack_payload Payload contained in the ACK or NACK,
               if any.

\return TRUE if the transaction should be ACK'd or NACK'd.
        FALSE if the transaction should not be ACK'd or NACK'd.
*/
BOOL one_net_master_txn_requested(const UInt8 TYPE, const BOOL SEND,
  const UInt8 DATA_TYPE, const UInt16 DATA_LEN,
  const one_net_raw_did_t * const DID, const UInt8 src_unit,
  const UInt8 dst_unit, on_nack_rsn_t* const nack_reason,
  on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload);


/*!
    \brief Callback for the application to handle the received block packet.

    \param[in] LEN The length of the payload in bytes.
    \param[in] SRC_ADDR The raw address of the sender
    \param[out] nack_reason The reason (if any) the message should
               be NACK'd.
    \param[out] ack_nack_handle The handle (if any) of the NACK
                payload.
    \param[out] ack_nack_payload The payload (if any) of the NACK

    \return TRUE if an ACK or a NACK should be sent.
            FALSE if an ACK or a NACK should not be sent.
*/
BOOL one_net_master_handle_block_pkt(const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR,
  on_nack_rsn_t* const nack_reason,
  on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload);


/*!
    \brief The status of a block transaction.

    \param[in] STATUS The status of the transaction.
    \param[in] DID The device ID of the other device involved in the txn.
    
    \return void
*/
void one_net_master_block_txn_status(one_net_status_t STATUS,
                                     const  one_net_raw_did_t *DID);


#ifdef _STREAM_MESSAGES_ENABLED
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
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Returns the next payload to send for the given transaction by
           adjusting a pointer visible to the ONE-NET code to the start
           of the block.  Also sets the length to be sent.

    \param[in] TYPE The type of transaction.  Must be ON_BLOCK.
    \param[out] len The number of bytes to be sent
      (must be <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN minus the header size)
    \param[in] DST The device receiving the next payload.

    \return A non-Error nack reason if the block transaction should proceed
            An error nack reason if the block transaction should not proceed.
*/
on_nack_rsn_t one_net_master_next_payload(UInt8 TYPE, UInt16 *len,
                                          const one_net_raw_did_t *DST);
#endif


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

