#ifndef _ONE_NET_MASTER_PORT_SPECIFIC_H
#define _ONE_NET_MASTER_PORT_SPECIFIC_H

#include "config_options.h"

#ifdef _ONE_NET_MASTER


#include "one_net_status_codes.h"
#include "one_net_constants.h"
#include "one_net_acknowledge.h"
#include "one_net_master.h"


//! \defgroup ON_MASTER_port_specific MASTER Specific ONE-NET functionality
//! \ingroup ONE-NET_port_specific 
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
    \file one_net_master_port_specific.h
    \brief MASTER specific ONE-NET declarations.

    These are interfaces for application and hardware specific functions that
    must be implemented for the ONE-NET project to compile (and work).

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_acknowledge.h"


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
    \brief Handles the received single packet.
	
    \param[in] raw_pld the raw single payload.  Note that ack_nack.ayload below
      also points to this parameter, so the memory can be changed.  Therefore,
      if the application code needs this payload to NOT change after sending
      a response, it must copy it.
    \param[in/out] msg_hdr of packet / response -- only the pid portion of
      this parameter should be changed.  The message type is irrelevant for
      a response message (the equivalent of a "message type" can be specified
      in the "handle" portion of the "ack_nack" parameter below.  The message
      id should not be changed.  The pid should be changed to the type of ACK
      or NACK that should be sent.
	\param[in] src_did the raw address of the source
	\param[in] repeater_did the raw address of the repeater, if any.  This will
               always equal the source id if 1) this is not a multi-hop message
               and 2) this is not a "forwarded" message by either the peer
               manager or the application code.  Most applications will ignore
               this parameter.
	\param[out] ack_nack: Contains three parts...
                  nack_reason... if nacking, the reason for the NACK.  Irrelevant for ACKs
	            handle... if including a payload, how the payload should be parsed.
                  For example, if this is a ffast query or command response that contains
                  a status message, the handle should be set to ON_ACK_STATUS.
                  If the message contains a time, this should be set to
                  ON_ACK_TIME_MS or ON_NACK_MS.  If the message contains an
                  application-specific value, this should be set to ON_ACK_VALUE
                  or ON_NACK_VALUE.  If simply sending an ACK or a NACK without
                  any payload, this parameter can be ignored.
                payload...
                  The "payload" of the NACK or ACK.  Irrelevant if the handle
                  is not set.  Note that this also points to raw_pld to save
                  space.
    \param[in] hops the number of hops it took to get here.  Can be ignored by
                    most applications.
    \param[in/out] max_hops in --> the maximum number of hops that was set for the message
	                        out --> the maximum number of hops to use for a response.  Can
                                    generally be ignored if you want the maximum number of
                                    hops to remain unchanged.
                 
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
*/
#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_master_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack);
#else
on_message_status_t one_net_master_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops);
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Handles the received block packet.
	
    \param[in] raw_pld the raw block payload.  Note that ack_nack.ayload below
      also points to this parameter, so the memory can be changed.  Therefore,
      if the application code needs this payload to NOT change after sending
      a response, it must copy it.
    \param[in/out] msg_hdr of packet / response -- only the pid portion of
      this parameter should be changed.  The message type is irrelevant for
      a response message (the equivalent of a "message type" can be specified
      in the "handle" portion of the "ack_nack" parameter below.  The message
      id should not be changed.  The pid should be changed to the type of ACK
      or NACK that should be sent.
	\param[in] src_did the raw address of the source
	\param[in] repeater_did the raw address of the repeater, if any.  This will
               always equal the source id if 1) this is not a multi-hop message
               and 2) this is not a "forwarded" message by either the peer
               manager or the application code.  Most applications will ignore
               this parameter.
	\param[out] ack_nack: Contains three parts...
                  nack_reason... if nacking, the reason for the NACK.  Irrelevant for ACKs
	            handle... if including a payload, how the payload should be parsed.
                  For example, if this is a ffast query or command response that contains
                  a status message, the handle should be set to ON_ACK_STATUS.
                  If the message contains a time, this should be set to
                  ON_ACK_TIME_MS or ON_NACK_MS.  If the message contains an
                  application-specific value, this should be set to ON_ACK_VALUE
                  or ON_NACK_VALUE.  If simply sending an ACK or a NACK without
                  any payload, this parameter can be ignored.
                payload...
                  The "payload" of the NACK or ACK.  Irrelevant if the handle
                  is not set.  Note that this also points to raw_pld to save
                  space.
    \param[in] hops the number of hops it took to get here.  Can be ignored by
                    most applications.
    \param[in/out] max_hops in --> the maximum number of hops that was set for the message
	                        out --> the maximum number of hops to use for a response.  Can
                                    generally be ignored if you want the maximum number of
                                    hops to remain unchanged.
                 
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
*/
#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_master_handle_block_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack);
#else
on_message_status_t one_net_master_handle_block_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops);
#endif
#endif


/*!
    \brief Callback for the application to handle an ack or a nack.

    This function is application dependent.  It is called by ONE-NET when a
    MASTER receives an ACK or a NACK.  For many applications, no ACK/ NACK
    handling is needed and the application waits for the entire transaction
    to be completed.  In this case (which is very common), the function should
    simply be an empty function that returns ON_MSG_CONTINUE, which tells
    ONE-NET to use its default handling.
    
    For NON-defgault handling, this function allows the application code to
    change a nack reason, change the number of retries, change the message it
    is sending, change the number of hops, or abort the transaction.  Even if
    nothing is changed, it informs the application code of what is going on
    with each individual response.  The application may be keeping tracking
    of how many ACKs versus NACKs it gets for whatever reason.
    
    
    "Fast query" messages and "command" messages or any other message which
    involves response messages containing status messages, may wish to call
    the one_net_master_handle_single_pkt and pass it the ACK payload as
    the payload.  This is generally how this is handled.
    one_net_master_handle_single_pkt should look at the message class and,
    if it is ONA_STATUS_FAST_QUERY_RESP or ONA_STATUS_COMMAND_RESP, this
    tells the function that the message was a response to a command or a fast
    query.
	

    \param[in/out] raw_pld Raw payload of the ORIGINAL message so that the
       application code can see what is being ACK'd or NACK'd.  Changable, but
       generally is NOT changed.
    \param[in/out] msg_hdr PID, Message ID, and Message Type(i.e. admin,
      app, etc.) of the ORIGINAL message.  This is changable, but like the
      raw payload, is generally NOT changed.  In particular, the message id
      is assigned by ONE-NET, so it should only be changed by ONE-NET.  If the
      application code needs to change the message id to a new one, it should
      call the function get_new_message_id() and use its return value.  Again,
      use caution when changing the message id.  There is usually no need to
      change it.
    \param[in] resp_msg_hdr The PID and Message ID of the response. Generally
      the message TYPE is irrelevant in a response.  The "handle" of the ACK
      or the NACK should be looked at instead for how to parse.
    \param[in] resp_msg_hdr The PID and Message ID of the response. Generally
      the message TYPE is irrelevant in a response.  The "handle" of the ACK
      or the NACK should be looked at instead for how to parse.  The message
      ID in the response should match the message id in the original message.
      If it does not, the original raw payload should be considered invalid.
      ONE-NET will consider this an invalid response if the message
      IDs do not match.  However, this function is still called to notify
      the application code.  If the message IDs DO NOT match, however, any
      changes made in this function will be ignored by ONE-NET.
    \param[in/out] resp_ack_nack The response (ACK or NACK), possibly with a
      payload and a NACK reason.  The "handle" will describe how the payload
      should be interpreted.  The nack reason can be changed and possibly
      a time in the payload if the nack reason involves a time that ONE-NET
      understands (i.e. length of time to pause).
    \param[in] resp_ack_nack The response (ACK or NACK), possibly with a
      payload and a NACK reason.  The "handle" will describe how the payload
      should be interpreted.  See the description of ack_nack in the
      one_net_master_handle_single_pkt for more details of how this should
      be interpreted.  This function can change an ACK to a NACK or vice versa
      and can also change the NACK reason.
	\param[in] src_did the raw address of the source
	\param[in] repeater_did the raw address of the repeater, if any.  This will
               always equal the source id if 1) this is not a multi-hop message
               and 2) this is not a "forwarded" message by either the peer
               manager or the application code.  Most applications will ignore
               this parameter.
    \param[in/out] retries The number of times this message has been sent.
      Generally this is not changed, but it can be.
    \param[in] hops the number of hops it took to get here.  Can be ignored by
                    most applications.
    \param[in/out] max_hops in --> the maximum number of hops that was set for
                                   the message
	                       out --> the maximum number of hops to use.  Can
                                   generally be ignored if you want the maximum
                                   number of hops to remain unchanged.
	
    \return ON_MSG_CONTINUED If ONE-NET should proceed with any further handling
              of the transaction
            ON_MSG_IGNORE to ignore this response and treat it at as if it had
              never been received
            ON_MSG_SUCCESS, ON_MSG_FAIL, ON_MSG_ABORT, or an
              application-specific code if this transaction should be
              terminated prematurely.  The nack reason (if it is to be
              changed), should be set here because this return code will
              result in a call to a callback function with both the NACK
              reason and this return code passed as parameters.
*/
#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_master_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries);
#else
on_message_status_t one_net_master_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries,
  UInt8 hops, UInt8* const max_hops);
#endif



#ifdef _NON_VOLATILE_MEMORY
/*!
    \brief Saves ONE-NET master settings to non-volatile memory
    
    \return ONS_SUCCESS If parameters were saved successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_master_save_settings(void);


/*!
    \brief Loads ONE-NET master settings from non-volatile memory(i.e. Flash)
    
    \return ONS_SUCCESS If parameters were loaded successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_master_load_settings(void);


/*!
    \brief Erases ONE-NET master settings(if any) from non-volatile memory(i.e. Flash)
    
    \return ONS_SUCCESS If non-volatile memory was erased successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_master_erase_settings(void);
#endif


/*!
    \brief Returns the raw SID for the master to use.
    
    \return Pointer to the raw SID to use.
*/
on_raw_sid_t* one_net_master_get_raw_sid(void);


/*!
    \brief Resets the device in MASTER mode.
    
    When the device is reset to MASTER mode, the network is empty and CLIENT
    will need to be added to the network using their unique key.
    
    \param[in] raw_sid The raw SID tio use for the network.
    \param[in] channel The channel to use.  Only relevant if non-negative.
               Also only relevant if non-volatile memory loading does not
               override the channel
    
    \return ONS_SUCCESS If reseting to MASTER mode was successful
            ONS_FAIL If the command failed
*/
one_net_status_t one_net_master_reset_master(on_raw_sid_t* raw_sid,
  SInt8 channel);


/*!
    \brief The status of a single transaction.

    Callback to report the status of sending an application single data packet.

    \param[in] status The status of the transaction.
    \param[in] retry_count The number of times that the packet had to be sent.
    \param[in] msg_hdr message id, message type, and pid of the message.
    \param[in] data The data that was sent.
    \param[in] dst The raw did of where the packet was sent.
    \param[in] ack_nack The reason for failure, if relevant.  A response
               payload, if relevant.
    \param[in] hops Number of hops it took to deliver the message, if
               known and relevant.  Negative number implies unknown or
               not relevant / reliable.
    \return void
*/
#ifndef _ONE_NET_MULTI_HOP
void one_net_master_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack);
#else
void one_net_master_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack, SInt8 hops);
#endif


/*!
    \brief Returns results of the invite new CLIENT operation.

    \param[in] STATUS ONS_SUCCESS if the device was successfully added.
                      TIME_OUT if the operation timed out.
                      ONS_CANCELED is the operation was cancelled
    \param[in] KEY The unique key used to add the device.
    \param[in] CLIENT_DID The did the CLIENT was assigned if STATUS == SUCCESS,
      otherwise 0.

    \return void
*/
void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t* KEY, const on_raw_did_t *CLIENT_DID);
  
  
/*!
    \brief Reports the results of a device settings update that the application
      layer initiated.

    \param[in] update What was being updated.
    \param[in] did The device that was being updated.
    \param[in] ack_nack The result of the update attempt.  If successful,
               the nack reason will be "no error".  If unsuccessful, the
               nack reason will contain the reason for failure.

    \return void
*/
void one_net_master_update_result(one_net_mac_update_t update,
  const on_raw_did_t* did, const on_ack_nack_t* ack_nack);


/*!
    \brief Alerts the application code has missed a required check-in

    \param[in] client The client that missed the required check-in

    \return TRUE if the master should ping the client
            FALSE otherwise
*/
BOOL one_net_master_client_missed_check_in(on_client_t* client);


/*!
    \brief Alerts the application that a device is awake.
    
    This allows the MASTER to send a message to the device since it knows the
    device is awake since it just received a message from the device.  Otherwise
    the MASTER would have to guess if the device is awake, attempt the
    transaction, and if it failed, it would have to send try sending it again.
    
    \param[in] DID The device that just sent a message.
    \param[in] responding FALSE if the device initiated the message.
                          TRUE if the device is responding to a message
    
    \return TRUE if there is activity for the device and a "Stay-Awake" should
                 be sent.  Irrelevant unless responding is FALSE.
            FALSE otherwise
*/
BOOL one_net_master_device_is_awake(BOOL responding,
  const on_raw_did_t *DID);
  
#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Alerts the master when a device has requested that another device
           should be reserved for service as a multi-hop repeater.
    
    Alerts the master when a device has requested that another device should be
    reserved for service as a multi-hop repeater in a block or stream transfer.
    The master is informed of the proposed parameters of this transfer.  The
    master may accept these paraemters or reject the parameters and offer
    acceptable parameters, or may decide to reject the transfer completely.  The
    master should fill in the NACk reason and payload with the rejection reason
    and the (if relevant) acceptable parameters.  To accept the parameters, the
    master can do nothing in this function since the NACK reason is already set
    as "no error".
    
    Possible reasons that a master might reject this request include...
    
    1. The master might be reserving the repeater or the requesting device for
       something else.
    2. The master anticipates that the proposed transfer might clog the airwaves
    3. The repeater is busy.
    4. The master doesn't like the data rate, channel, or duration of the
       proposed transfer.
    5. The master does not want any block or stream or multi-hop transfers to
       proceed at the moment for whatever reason.
    6. There is already another block / stream transfer going on somewhere and
       the master only wants one at a time.
    7. One or both clients are no longer in the network. or there is about to
       be a key change.
    8. Some application-specific reason.
    
    \param[in] requesting_client The device that is being proposed as a repeater.
    \param[in] repeater_client The device that is being proposed as a repeater.
    \param[in] channel The channel that the transfer shall proceed on.
    \param[in] data_rate The data rate that the transfer shall proceed on.
    \param[in] data_rate The priority that the transfer shall proceed on.
    \param[in] estimated_time The best-guess by the sending device of how long
               the transfer will take, in milliseconds.
    \param[out] The NACK reason and payload if the master decides to reject this
               request.
*/
void one_net_master_repeater_requested(on_client_t* requesting_client,
  on_client_t* repeater_client, UInt8 channel, UInt8 data_rate, UInt8 priority,
  UInt32 estimated_time, on_ack_nack_t* ack_nack);
  
on_nack_rsn_t one_net_master_get_default_block_transfer_values(
  const on_client_t* src, const on_client_t* dst,
  UInt32 transfer_size, UInt8* priority, UInt8* chunk_size, UInt16* frag_delay,
  UInt16* chunk_delay, UInt8* data_rate, UInt8* channel,
  on_ack_nack_t* ack_nack);
  
void one_net_master_block_txn_status(block_stream_msg_t* msg,
  on_message_status_t status, on_ack_nack_t* ack_nack);
#endif

#ifdef _STREAM_MESSAGES_ENABLED
on_nack_rsn_t one_net_master_get_default_stream_transfer_values(
  const on_client_t* src, const on_client_t* dst, UInt32 time_ms,
  UInt8* data_rate, UInt8* channel, on_ack_nack_t* ack_nack);
  
void one_net_master_stream_txn_status(block_stream_msg_t* msg,
  on_message_status_t status, on_ack_nack_t* ack_nack);
#endif
                 


//! @} ON_MASTER_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ON_MASTER_port_specific


#endif // ifdef _ONE_NET_MASTER //

#endif // _ONE_NET_MASTER_PORT_SPECIFIC_H //
