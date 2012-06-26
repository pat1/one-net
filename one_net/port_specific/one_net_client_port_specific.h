#ifndef _ONE_NET_CLIENT_PORT_SPECIFIC_H
#define _ONE_NET_CLIENT_PORT_SPECIFIC_H

#include "config_options.h"

#ifdef ONE_NET_CLIENT


#include "one_net_status_codes.h"
#include "one_net_constants.h"
#include "one_net_message.h"
#include "one_net_acknowledge.h"


//! \defgroup ON_CLIENT_port_specific CLIENT Specific ONE-NET functionality
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
    \file one_net_client_port_specific.h
    \brief CLIENT specific ONE-NET declarations.

    These are interfaces for application and hardware specific functions that
    must be implemented for the ONE-NET project to compile (and work).

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_acknowledge.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ON_CLIENT_port_specific_const
//! \ingroup ON_CLIENT_port_specific
//! @{

//! @} ON_CLIENT_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ON_CLIENT_port_specific_typedefs
//! \ingroup ON_CLIENT_port_specific
//! @{
    
//! @} ON_CLIENT_port_specific_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ON_CLIENT_port_specific_pub_var
//! \ingroup ON_CLIENT_port_specific
//! @{

//! @} ON_CLIENT_port_specific_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ON_CLIENT_port_specific_pub_func
//! \ingroup ON_CLIENT_port_specific
//! @{
    
    
/*!
    \brief Callback when the CLIENT has successfully joined a network.
    
    This function is called by ONE-NET after a CLIENT has successfully joined
    a network in which it had been invited (received the invite packet).

    \param[in] RAW_DID The did assigned to the CLIENT.
    \param[in] status Result of the invitation (success, timeout, cancelled)

    \return void
*/
void one_net_client_invite_result(const on_raw_did_t * const RAW_DID,
  one_net_status_t status);
  
  
/*!
    \brief Callback when a CLIENT has been removed.
    
    This function is called by ONE-NET after a CLIENT has been
    removed from the network.

    \param[in] raw_did The did of the device that was removed.
    \param[in] this_device_removed TRUE if this device was removed, false
               otherwise.

    \return void
*/
void one_net_client_client_removed(const on_raw_did_t * const raw_did,
    BOOL this_device_removed);
  
  
/*!
    \brief Callback when a CLIENT has been added.
    
    This function is called by ONE-NET after a CLIENT has been
    added to the network.

    \param[in] raw_did The did of the device that was added.

    \return void
*/
void one_net_client_client_added(const on_raw_did_t * const raw_did);


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
#ifndef ONE_NET_MULTI_HOP
on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack);
#else
on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops);
#endif


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Handles the received stream packet.
	
    \param[in] txn The block / stream transaction
    \param[in] bs_msg The block / stream message
    \param[in] stream_pkt The packet received
    \param[out] The ACK or NACK that should be returned in the response, if any
                 
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/
on_message_status_t one_net_client_handle_stream_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, stream_pkt_t* stream_pkt, on_ack_nack_t* ack_nack);
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Handles the received block packet.
	
    \param[in] txn The block / stream transaction
    \param[in] bs_msg The block / stream message
    \param[in] block_pkt The packet received
    \param[out] The ACK or NACK that should be returned in the response, if any
                 
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/
on_message_status_t one_net_client_handle_block_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, block_pkt_t* block_pkt, on_ack_nack_t* ack_nack);


/*!
    \brief Called when a chunk of a block has been received.
	
    \param[in] bs_msg The block / stream message
    \param[in] byte_idx The byte index of the start of the chunk
    \param[in] chunk_size The size of the chunk
    \param[out] The ACK or NACK that should be returned in the response, if any
                 
    \return ON_MSG_ACCEPT_CHUNK to mark the chunk as valid and move on.
            ON_MSG_REJECT_CHUNK to force the other side to send the entire chunk again.
            See on_message_status_t for other options.
*/
on_message_status_t one_net_client_block_chunk_received(
  block_stream_msg_t* bs_msg, UInt32 byte_idx, UInt8 chunk_size,
  on_ack_nack_t* ack_nack);
#endif


/*!
    \brief Callback for the application to handle an ack or a nack.

    This function is application dependent.  It is called by ONE-NET when a
    CLIENT receives an ACK or a NACK.  For many applications, no ACK/ NACK
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
    the one_net_client_handle_single_pkt and pass it the ACK payload as
    the payload.  This is generally how this is handled.
    one_net_client_handle_single_pkt should look at the message class and,
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
      one_net_client_handle_single_pkt for more details of how this should
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
#ifndef ONE_NET_MULTI_HOP
on_message_status_t one_net_client_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries);
#else
on_message_status_t one_net_client_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries,
  UInt8 hops, UInt8* const max_hops);
#endif


/*!
    \brief Returns a pointer to the invite key to use in for joining a network.
    
    \return A pointer to the invite key to use.
*/
one_net_xtea_key_t* one_net_client_get_invite_key(void);


#ifdef _NON_VOLATILE_MEMORY
/*!
    \brief Saves ONE-NET client settings to non-volatile memory
    
    \return ONS_SUCCESS If parameters were saved successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_client_save_settings(void);


/*!
    \brief Loads ONE-NET client settings from non-volatile memory(i.e. Flash)
    
    \return ONS_SUCCESS If parameters were loaded successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_client_load_settings(void);


/*!
    \brief Erases ONE-NET client settings(if any) from non-volatile memory(i.e. Flash)
    
    \return ONS_SUCCESS If non-volatile memory was erased successfully
            ONS_FAIL or any other failure message upon failure.
*/
one_net_status_t one_net_client_erase_settings(void);
#endif


/*!
    \brief Resets the device in client mode.
    
    When the device is reset to client mode, the network is empty and the
    client will need to be added to the network using its invite key.
    
    \param[in] invite_key The invite key for use when listening for
                 invitations.
    
    \return ONS_SUCCESS If reseting to client mode was successful
            ONS_FAIL If the command failed
*/
#ifdef ENHANCED_INVITE
one_net_status_t one_net_client_reset_client(const one_net_xtea_key_t* invite_key,
  UInt8 low_channel, UInt8 high_channel, tick_t timeout_time);
#else
one_net_status_t one_net_client_reset_client(const one_net_xtea_key_t* invite_key);
#endif

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
#ifndef ONE_NET_MULTI_HOP
void one_net_client_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack);
#else
void one_net_client_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack, SInt8 hops);
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Application-level code called by ONE-NET when initiating a block
           transfer containing default the block / stream parameters and
           allowing the application level code to change it.
    
    The function comes pre-loaded with the default parameters that the core-level
    ONE-NET code has determined should generally be good default paramters.  The
    application-code should change these values here if it wants to.

    
    \param[in] dst The destination of the transfer.
    \param[in] transfer_size The number of bytes to be transferred.
    \param[in/out] priority The priority of the transfer.
    \param[in/out] chunk_size The "chunk size" involved in the transfer.
    \param[in/out] frag_delay The time to wait between packet sends.
    \param[in/out] chunk_delay The time to pause between "chunks" of the message.
    \param[in/out] data_rate The data rate to use for the transfer.
    \param[in/out] channel The channel to use for the transfer.
    \param[in/out] timeout The time to wait for a response before assuming that
                   communication has been lost.
    \param[out]    If rejecting the transfer and there is an ack or nack associated
                   with it, this value should be filled in.
    
    \return The nack reason if rejecting the transfer.
            
*/
on_nack_rsn_t one_net_client_get_default_block_transfer_values(
  const on_encoded_did_t* dst, UInt32 transfer_size, UInt8* priority,
  UInt8* chunk_size, UInt16* frag_delay, UInt16* chunk_delay, UInt8* data_rate,
  UInt8* channel, UInt16* timeout, on_ack_nack_t* ack_nack);
  
  
/*!
    \brief Callback function for a response received for a block or stream
           message
    
    \param[in/out] txn The transaction.
    \param[in/out] bs_msg The block or stream message being responded to.
    \param[in] pkt The packet structure.
    \param[in] raw_payload_bytes The raw payload bytes that are being responded to.
    \param[in/out] ack_nack The ack or nack atttached to the response.
           
    \return ON_MSG_IGNORE to ignore the response.
            ON_MSG_TERMINATE to terminate the transaction
            ON_MSG_ACCEPT_PACKET If packet is good.
            If packet is rejected, the ack_nack reason and / or payload should be filled in.
*/
on_message_status_t one_net_client_handle_bs_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack);


/*!
    \brief Callback function called when a block transaction is complete.
    
    Several things can cause this function to be called.
    
    1. We are the source and everything transferred successfully and we are
       informing the application code that this is the case, and we will also
       immediately inform the destination device, any repeaters, and possibly
       the master.
    2. We are the source and we need to terminate prematurely on our end.  We
       need to inform our application code as well as the other device(s).
    3. We are the destination and everything transferred successfully and we are
       informing the application code that this is the case.  No other messages
       are needed.
    4. We are the destination and we need to terminate prematurely on our end.
       Any ACKs or NACKs to the sending device have been handled elsewhere.
       
    If we are the source, then the ack_nack message will be non-NULL and
    will be pre-loaded with what ONE-NET intends to send the other device(s) in
    the termination message.  This function can either leave the ack_nack
    alone, it can change it, or it tells ONE-NET NOT TO send this termination
    message.  It does that by returning anything but ON_MSG_RESPOND.
    
    If we are not the source, we might either abort immediately or "hang around"
    as a service to any other device(s) and give them an ACK or a NACK or
    stay as a repeater or whatever. In that case, this function should return
    ON_MSG_RESPOND and we will, if we are the destination and we are the ones
    terminating the device, at least tell the sending device that we are
    terminating and why.  On the other hand, we can also simply "drop out" of
    the message and the other device(s) will eventually time out.
    
    The termination message should be changed if it is something that would
    not make sense to the other devices.
    
    Note that the status can also be changed if desired by this application code.
       

    \param[in] msg The block / stream message that is being terminated.
    \param[in] terminating_device The device that terminated this transaction.
    \param[in/out] status The status of the message that was just completed.
    \param[in/out] ack_nack Any ACK or NACK associated with this termination.
    
    \return ON_MSG_RESPOND if this device should inform the other devices
              of the termination.
            All other return types abort immediately with no further messages.
*/
on_message_status_t one_net_client_block_txn_status(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);

#ifdef ONE_NET_MH_CLIENT_REPEATER
/*!
    \brief Application-level code called byu ONE-NET when this device is
           requested to function as a repeater for a block / stream message
    
    This function is called when another device is attempting to set up a
    block / stream message and has requested this device to reserve itself
    as a repeater for that purpose. This function will be passed the parameters
    of the proposed block / stream transfer.  Possible parameters of interest
    will be the estimated time of the transfer and the devices involved.
    
    Generally this function should reject the request if it feels it cannot
    comply for any reason.  Reasons could include not being reasonably certain
    that it will be able to function as a repeater for at least the time
    requested for whatever reason (low power, busy with its own messages,
    not expected to be powered up for the entire message, a high percentage
    of dropped message, it is reserved as a repeater for someone else, etc.)
    
    The ack_nack parameter is pre-loaded to assume acceptance.  If the repeater
    rejects, it should change the ack_nack variable to indicate rejection along
    with a reason, if any, is to be sent.
    
    \param[in] bs_msg The paramters of the block/ stream message this device is
               supposed to serve as a repeater for.
    \param[out] ack_nack This is pre-loaded for acceptance.  If accepting, no
                changes are needed.  If rejecting, the ack_nack variable should
                be changed in this function.
    
    \return void
*/
void one_net_client_repeater_requested(block_stream_msg_t* bs_msg,
  on_ack_nack_t* ack_nack);
#endif
#endif

#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Application-level code called by ONE-NET when initiating a stream
           transfer containing default the block / stream parameters and
           allowing the application level code to change it.
    
    The function comes pre-loaded with the default parameters that the core-level
    ONE-NET code has determined should generally be good default paramters.  The
    application-code should change these values here if it wants to.

    
    \param[in] dst The destination of the transfer.
    \param[in] time_ms Proposed duration of the stream transfer.  If time is 0, then the time is unknown.
    \param[in/out] priority The priority of the transfer.
    \param[in/out] frag_delay The time to wait between packet sends.
    \param[in/out] data_rate The data rate to use for the transfer.
    \param[in/out] channel The channel to use for the transfer.
    \param[in/out] timeout The time to wait for a response before assuming that
                   communication has been lost.
    \param[out]    If rejecting the transfer and there is an ack or nack associated
                   with it, this value should be filled in.
    
    \return The nack reason if rejecting the transfer.
            
*/
on_nack_rsn_t one_net_client_get_default_stream_transfer_values(
  const on_encoded_did_t* dst, UInt32 time_ms, UInt8* priority,
  UInt16* frag_delay, UInt8* data_rate, UInt8* channel, UInt16* timeout,
  on_ack_nack_t* ack_nack);


/*!
    \brief Callback function called when a stream transaction is complete.
    
    Several things can cause this function to be called.
    
    1. We are the source and everything transferred successfully and we are
       informing the application code that this is the case, and we will also
       immediately inform the destination device, any repeaters, and possibly
       the master.
    2. We are the source and we need to terminate prematurely on our end.  We
       need to inform our application code as well as the other device(s).
    3. We are the destination and everything transferred successfully and we are
       informing the application code that this is the case.  No other messages
       are needed.
    4. We are the destination and we need to terminate prematurely on our end.
       Any ACKs or NACKs to the sending device have been handled elsewhere.
       
    If we are the source, then the ack_nack message will be non-NULL and
    will be pre-loaded with what ONE-NET intends to send the other device(s) in
    the termination message.  This function can either leave the ack_nack
    alone, it can change it, or it tells ONE-NET NOT TO send this termination
    message.  It does that by returning anything but ON_MSG_RESPOND.
    
    If we are not the source, we might either abort immediately or "hang around"
    as a service to any other device(s) and give them an ACK or a NACK or
    stay as a repeater or whatever. In that case, this function should return
    ON_MSG_RESPOND and we will, if we are the destination and we are the ones
    terminating the device, at least tell the sending device that we are
    terminating and why.  On the other hand, we can also simply "drop out" of
    the message and the other device(s) will eventually time out.
    
    The termination message should be changed if it is something that would
    not make sense to the other devices.
    
    Note that the status can also be changed if desired by this application code.
       

    \param[in] msg The block / stream message that is being terminated.
    \param[in] terminating_device The device that terminated this transaction.
    \param[in/out] status The status of the message that was just completed.
    \param[in/out] ack_nack Any ACK or NACK associated with this termination.
    
    \return ON_MSG_RESPOND if this device should inform the other devices
              of the termination.
            All other return types abort immediately with no further messages.
*/
on_message_status_t one_net_client_stream_txn_status(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);
#endif



//! @} ON_CLIENT_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ON_CLIENT_port_specific


#endif // ifdef ONE_NET_CLIENT //


#endif // _ONE_NET_CLIENT_PORT_SPECIFIC_H //
