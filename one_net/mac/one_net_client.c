//! \addtogroup ONE-NET_CLIENT ONE-NET CLIENT device functionality
//! \ingroup ONE-NET
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
    \file one_net_client.c
    \brief ONE-NET CLIENT functionality implementation

    Derives from ONE-NET.  CLIENT dependent functionality.  This module is
    dependent on one_net_client_net, and must initialize one_net_client_net.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"

#ifdef _ONE_NET_CLIENT

#include "one_net_client.h"
#include "one_net_client_port_const.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "one_net_client_port_specific.h"
#include "tick.h"
#include "one_net_timer.h"
#include "tal.h"
#include "one_net_encode.h"
#include "one_net_prand.h"
#include "one_net_acknowledge.h"
#include "one_net_timer.h"
#include "one_net_crc.h"
#ifdef _PEER
#include "one_net_peer.h"
#endif



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_const
//! \ingroup ONE-NET_CLIENT
//! @{



//! @} ONE-NET_CLIENT_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_CLIENT_typedefs
//! \ingroup ONE-NET_CLIENT
//! @{

/*!
    \brief Keeps track of CLIENT transactions
*/



//! @} ONE-NET_CLIENT_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_CLIENT_pub_var
//! \ingroup ONE-NET_CLIENT
//! @{

//! Flag to signify that this client is part of a network.
BOOL client_joined_network = FALSE;

//! Flag to signify that the client is not part of a network and is looking for
//! an invitation.
BOOL client_looking_for_invite = FALSE;

#ifdef _ENHANCED_INVITE
    //! Flag to signify that an invitation attempt has expired without successfully
    //! joining a network.
    BOOL client_invite_timed_out = FALSE;
	
    //! Lowest channel to consider when looking for an invite
	one_net_channel_t low_invite_channel;
	
	one_net_channel_t high_invite_channel;	
#endif



//! @} ONE-NET_CLIENT_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================




//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_CLIENT_pri_var
//! \ingroup ONE-NET_CLIENT
//! @{


//! The MASTER device
on_master_t * const master
  = (on_master_t * const)(nv_param + sizeof(on_base_param_t));

//! The ONE_NET_RX_FROM_DEVICE_COUNT devices that have most recently sent data
//! to this device.
on_sending_dev_list_item_t sending_dev_list[ONE_NET_RX_FROM_DEVICE_COUNT];

//! Set to true if a confirmation of a key change is needed.
static BOOL confirm_key_change = FALSE;

//! Set to true upon being deleted from the network.  There will be a slight
//! two second pause before this device actually removes itself to give any
//! pending transactions to complete.
static BOOL removed = FALSE;



//! flag denoting whether the client accepting the invite has sent
//! the features
static BOOL sent_features = FALSE;

//! flag denoting whether the client accepting the invite has received
//! the keep-alive interval
static BOOL rcvd_keep_alive = FALSE;

#ifdef _BLOCK_MESSAGES_ENABLED
//! flag denoting whether the client accepting the invite has received
//! the fragment delays
static BOOL rcvd_fragment_delays = FALSE;
#endif

//! flag denoting whether the client accepting the invite has received
//! the settings / flags.
static BOOL rcvd_settings = FALSE;


//! @} ONE-NET_CLIENT_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{



// packet handlers
static on_message_status_t on_client_single_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
#ifdef _BLOCK_MESSAGES_ENABLED
static on_message_status_t on_client_block_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_client_handle_block_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
#endif
#ifdef _STREAM_MESSAGES_ENABLED
static on_message_status_t on_client_stream_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_client_handle_stream_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
#endif
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);

static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);
static one_net_status_t init_internal(void);

static one_net_status_t one_net_client_send_single(UInt8 raw_pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t expire_time_from_now
  #endif
  );

#if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static one_net_status_t send_keep_alive(tick_t send_time_from_now,
  tick_t expire_time_from_now);
#elif _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
static one_net_status_t send_keep_alive(tick_t send_time_from_now);
#else
static one_net_status_t send_keep_alive(void);
#endif
  
static BOOL look_for_invite(void);

static on_message_status_t handle_admin_pkt(const on_encoded_did_t * const
  SRC_DID, const UInt8 * const DATA, on_txn_t* txn, on_ack_nack_t* ack_nack);
  
static BOOL check_in_with_master(void);


#ifndef _ONE_NET_SIMPLE_DEVICE
static void on_client_adjust_recipient_list(const on_single_data_queue_t*
  const msg, on_recipient_list_t** recipient_send_list);
#endif


//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_pub_func
//! \ingroup ONE-NET_CLIENT
//! @{

/*!
    \brief Initializes a CLIENT to start looking for an invite message.

    This function should be called the first time a device starts up, or when
    the device should attempt to join another network.  Once a device has
    joined a network, one_net_client_init should be called to reinitialize the
    CLIENT.

    \param INVITE_KEY The unique key of this CLIENT to decrypt invite packets
      with.
    \param[in] SINGLE_BLOCK_ENCRYPT_METHOD The method to use to encrypt single
      and block packets when they are sent.
    \param[in] STREAM_ENCRYPT_METHOD The method to use to encrypt stream packets
      when they are sent.
    \param[in] min_channel lowest channel to scan on.
    \param[in] max_channel highest channel to scan on.
    \param[in] timeout_time Length of time in milliseconds to listen for the invite.
	  0 means indefinite.

    \return ONS_SUCCESS Successfully initialized the CLIENT.
            ONS_BAD_PARAM if the parameter is invalid.
*/
#if !defined(_ENHANCED_INVITE)
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD)
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD)
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#else

#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time)
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time)	  
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#endif
{
    tick_t time_now = get_tick_count();

    
    // copy some parameters over to the base parameters and the master
    // parameters.
    
    // initialize SID to invalid (all zeroes).
    one_net_memset(on_base_param->sid, 0xB4, ON_ENCODED_SID_LEN);
    on_base_param->single_block_encrypt = SINGLE_BLOCK_ENCRYPT_METHOD;
    on_base_param->version = ON_PARAM_VERSION;
    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    one_net_memmove(&(on_base_param->current_key), *INVITE_KEY,
      sizeof(on_base_param->current_key));
    #ifdef _STREAM_MESSAGES_ENABLED
    on_base_param->stream_encrypt = STREAM_ENCRYPT_METHOD;      
    #endif
    #ifdef _BLOCK_MESSAGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
    #endif 
    master->flags = 0x00;
    master->device.expected_nonce = one_net_prand(time_now, ON_MAX_NONCE);
    master->device.last_nonce = one_net_prand(time_now, ON_MAX_NONCE);
    master->device.send_nonce = one_net_prand(time_now, ON_MAX_NONCE);
    master->device.msg_id = 0;
    master->device.data_rate = ONE_NET_DATA_RATE_38_4;
    master->device.features = FEATURES_UNKNOWN;
    #ifdef _ONE_NET_MULTI_HOP
    master->device.hops = 0;
    master->device.max_hops = ON_MAX_HOPS_LIMIT;
    #endif
    one_net_memmove(master->device.did, MASTER_ENCODED_DID,
      ON_ENCODED_DID_LEN);

    // now fill in the channels and set some timers and set the state for
    // the state machine and some flags to get ready to receive invitations.
    client_joined_network = FALSE;
    client_looking_for_invite = TRUE;
    sent_features = FALSE;
    rcvd_keep_alive = FALSE;
    #ifdef _BLOCK_MESSAGES_ENABLED
    rcvd_fragment_delays = FALSE;
    #endif
    rcvd_settings = FALSE;

    #ifdef _ENHANCED_INVITE
    client_invite_timed_out = FALSE;
	low_invite_channel = min_channel;
	high_invite_channel = max_channel;
    on_base_param->channel = low_invite_channel;
	
	if(timeout_time > 0)
	{
		ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(timeout_time));
	}
    else
    {
        ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(ONE_NET_MASTER_INVITE_DURATION));
    }
    #else
    on_base_param->channel = one_net_prand(time_now, ONE_NET_MAX_CHANNEL);
    ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(ONE_NET_MASTER_INVITE_DURATION));
    #endif
    
    // set up packet handlers, etc.
    init_internal();

    ont_set_timer(ONT_GENERAL_TIMER, MS_TO_TICK(ONE_NET_SCAN_CHANNEL_TIME));
    on_state = ON_JOIN_NETWORK;
    return ONS_SUCCESS;
} // one_net_client_look_for_invite //


/*!
    \brief Initializes the CLIENT to run in a network that it has previously
      joined.

    If the CLIENT has not yet joined a network, one_net_client_look_for_invite
    needs to be called instead of this function.

    \param[in] param The parameters (not including peer) that were saved.
                     If NULL, then the caller has already initialized the
                     memory.
    \param[in] param_len The sizeof param in bytes.
    \param[in] peer_param The peer memory that was saved.  If NULL, then the
                          caller has already initialized the peer memory.
    \param[in] peer_param_len The length of the peer memory.

    \return ONS_SUCCESS If initializing the CLIENT was successful
            ONS_BAD_PARAM If any of the parameters are invalid
*/
#ifndef _PEER
one_net_status_t one_net_client_init(const UInt8 * const param,
  const UInt16 param_len)
#else
one_net_status_t one_net_client_init(const UInt8 * const param,
  const UInt16 param_len, const UInt8* const peer_param,
  const UInt16 peer_param_len)
#endif
{
    one_net_status_t status;
    
    if(param)
    {
        if(param_len != CLIENT_NV_PARAM_SIZE_BYTES)
        {
            return ONS_BAD_PARAM;
        }
        one_net_memmove(nv_param, param, CLIENT_NV_PARAM_SIZE_BYTES);
    }
    
    #ifdef _PEER
    if(peer_param != NULL)
    {
        one_net_reset_peers();
        if(peer_param_len > PEER_STORAGE_SIZE_BYTES || peer_param_len %
          sizeof(on_peer_unit_t) != 0)
        {
            return ONS_BAD_PARAM;
        }
        one_net_memmove(peer_storage, peer_param, peer_param_len);
    }
    #endif

    if(!(master->flags & ON_JOINED))
    {
        return ONS_NOT_JOINED;
    } // if not connected //

    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status;
    } // if initializing the internals failed //

    on_state = ON_LISTEN_FOR_DATA;
    client_joined_network = TRUE;
	client_looking_for_invite = FALSE;
   
    return ONS_SUCCESS;
} // one_net_client_init //


/*!
    \brief The main function for the ONE-NET CLIENT.

    \param void

    \return The number of ticks the device can sleep for.
*/
tick_t one_net_client(void)
{
    // The current transaction
    static on_txn_t * txn = 0;
    
    // The time the application can sleep for in ticks (as opposed to ms).
    // Probably relevant only for devices which sleep, but we'll let the
    // application code decide that.  We'll return the correct value
    // regardless of whether the device sleeps.
    tick_t sleep_time = 0;
    
    
    if(!pkt_hdlr.single_data_hdlr)
    {
        // shouldn't get here.  If we did, something very bad happened.
        // Reset everything and try again.
        one_net_client_reset_client(one_net_client_get_invite_key());
        return 0;
    }
    

    switch(on_state)
    {
        #ifdef _IDLE
        case ON_IDLE:
            // not sure what happens here.  This is "do-nothing" mode or
            // "do-nothing with ONE-NET messages" mode or a chance to put
            // the device in low-power or only handle application code or
            // perhaps whatever the developer wishes.
            break;
        #endif
            
        case ON_LISTEN_FOR_DATA:
        {
            //
            // Listen for a new transaction.
            // Also check to see if there are any events
            // associated with timers that need attention
            //
            if(removed && ont_inactive_or_expired(ONT_INVITE_TIMER))
            {
                // We've been removed from the network.  We paused for 3
                // seconds to allow any other transaction to complete, and that
                // pasue is now over.  Set the client_joined_network flag to
                // false.
                removed = FALSE;
                one_net_client_client_removed(NULL, TRUE);
                one_net_client_reset_client(one_net_client_get_invite_key());
                return 0;
            }
            
            if(!client_joined_network &&
              ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                // we started to accept an invite, but for whatever reason
                // we did not get 
                one_net_client_reset_client(one_net_client_get_invite_key());
                return 0;
            }
            
            if(!client_joined_network)
            {
                // this will cause us to check for the conditions under
                // which we can actual complete the invite process.
                ont_set_timer(ONT_KEEP_ALIVE_TIMER, 0);
            }
            break;
        }
        
        case ON_JOIN_NETWORK:
        {
            if(!look_for_invite())
            {
                break;
            }
            
            client_looking_for_invite = FALSE;
            
            // give it ten seconds to finish joining the network and if we
            // have not completed the process by then, start looking for
            // invites again.
            ont_set_timer(ONT_GENERAL_TIMER,
              MS_TO_TICK(INVITE_TRANSACTION_TIMEOUT));
            on_state = ON_LISTEN_FOR_DATA;
        }
        
        default:
        {
            break;
        }
    }

    one_net(&txn);

    // calculate the allowable sleep time for devices that sleep
    
    // first some cases where we cannot sleep at all.
    #ifdef _DEVICE_SLEEPS
    if(txn || ont_active(ONT_STAY_AWAKE_TIMER) || on_state !=
      ON_LISTEN_FOR_DATA)
    #else
    if(txn || on_state != ON_LISTEN_FOR_DATA)
    #endif
    {
        sleep_time = 0;
    }
    else
    {
        #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
        tick_t queue_sleep_time;
        #endif
        
        // this will be the absolute maximum -- this may be overridden.
        sleep_time = ont_get_timer(ONT_KEEP_ALIVE_TIMER);
    
        #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
        if(single_data_queue_ready_to_send(&queue_sleep_time) == -1)
        #else
        if(single_data_queue_ready_to_send() == -1)
        #endif
        {
            #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
            if(queue_sleep_time > 0 && queue_sleep_time < sleep_time)
            {
                sleep_time = queue_sleep_time;
            }
            #endif
            
            if(check_in_with_master())
            {
                sleep_time = 0;
            }
        }
        else
        {
            sleep_time = 0;
        }
    }
    
    // even if the device doesn't sleep, we'll return sleep_time.  Even the
    // application code of non-sleeping devices might find it useful.

    return sleep_time;
} // one_net_client //


/*!
    \brief Calculate CRC over the client parameters.
    
    \param[in] param pointer to non-volatile parameters.  If NULL,
               on_base_param is used.
    \param[in] param_len Length of non-volatile parameters.  If negative, this
               is disregarded.
    \param[in] peer_param pointer to peer parameters.  If NULL,
               peer is used.
    \param[in] peer_param_len Length of peer parameters.  If negative, this
               is disregarded.
    \return 8-bit CRC of the client parameters if valid
            -1 if invalid
*/
#ifndef _PEER
int client_nv_crc(const UInt8* param, int param_len)
#else
int client_nv_crc(const UInt8* param, int param_len, const UInt8* peer_param,
    int peer_param_len)
#endif
{
    UInt16 starting_crc = ON_PLD_INIT_CRC;
    const UInt8 CRC_LEN = sizeof(UInt8);
    UInt16 expected_param_len;
    
    #ifdef _PEER
    if(!peer_param)
    {
        peer_param = peer_storage;
    }
    if(peer_param_len >= 0 && peer_param_len != PEER_STORAGE_SIZE_BYTES)
    {
        return -1;
    }
    #endif
    
    if(!param)
    {
        param = nv_param;
    }
    
    if(param_len >= 0 && expected_param_len != CLIENT_NV_PARAM_SIZE_BYTES)
    {
        return -1;
    }
    

    #ifdef _PEER
    // crc over peer parameters
    starting_crc = one_net_compute_crc(peer_param, PEER_STORAGE_SIZE_BYTES,
      starting_crc, ON_PLD_CRC_ORDER);
    #endif
    
    return one_net_compute_crc(&param[CRC_LEN], CLIENT_NV_PARAM_SIZE_BYTES
      - CRC_LEN, starting_crc, ON_PLD_CRC_ORDER);
} // client_nv_crc //



//! @} ONE-NET_CLIENT_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{



// TODO -- document
static on_message_status_t on_client_single_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    BOOL stay_awake;
    on_message_status_t msg_status;
    on_msg_hdr_t msg_hdr;
    on_raw_did_t raw_src_did, raw_repeater_did;
    UInt8 response_pid;
    on_sending_device_t* device;

    on_decode(raw_src_did, *(pkt->enc_src_did), ON_ENCODED_DID_LEN);
    on_decode(raw_repeater_did, *(pkt->enc_repeater_did), ON_ENCODED_DID_LEN);
    
    msg_hdr.msg_type = *msg_type;
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    
    // we'll be sending it back to the souerce.
    if(!(device = sender_info(pkt->enc_src_did)))
    {
        // I think we should have solved this problem before now, but abort if
        // we have not.
        
        // TODO -- solve this better or confirm it is in fact solved earlier.
        *txn = 0;
        return ON_MSG_ABORT;
    }
    
    if(ack_nack->nack_reason)
    {
        // an error has already been set.  That means we don't need to do
        // anything but build the proper response packet.
        goto ocsdh_build_resp;
    }
    
    switch(*msg_type)
    {
        case ON_ADMIN_MSG:
            msg_status = handle_admin_pkt(pkt->enc_src_did,
            &raw_pld[ON_PLD_DATA_IDX], *txn, ack_nack);
            break;
        default:   
            #ifndef _ONE_NET_MULTI_HOP
            msg_status = one_net_client_handle_single_pkt(&raw_pld[ON_PLD_DATA_IDX],
              &msg_hdr, &raw_src_did, &raw_repeater_did, ack_nack);
            #else
            msg_status = one_net_client_handle_single_pkt(&raw_pld[ON_PLD_DATA_IDX],
              &msg_hdr, &raw_src_did, &raw_repeater_did, ack_nack, (*txn)->hops,
              &((*txn)->max_hops));
            #endif
            break;
    }


    if(msg_status != ON_MSG_CONTINUE)
    {
        *txn = 0;
        return msg_status;
    }
    
    // if this was a normal query response, we'll send a message in addition
    // to the ACK.
    if(ack_nack->handle == ON_ACK_STATUS && get_msg_class(
      ack_nack->payload->status_resp) == ONA_STATUS_QUERY_RESP)
    {
        one_net_client_send_single(ONE_NET_RAW_SINGLE_DATA, ON_APP_MSG,
            ack_nack->payload->status_resp, ONA_SINGLE_PACKET_PAYLOAD_LEN,
            ONE_NET_HIGH_PRIORITY, NULL, pkt->enc_src_did
        #ifdef _PEER
            , FALSE,
            get_src_unit(ack_nack->payload->status_resp)
        #endif
        #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL   
	        , 0
        #endif
        #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	        , 0
        #endif
        );
        
        // now change the handle to ON_ACK
        ack_nack->handle = ON_ACK;
    }
    
    
    // change the nonce we want.
    device->last_nonce = device->expected_nonce;
    device->expected_nonce = one_net_prand(get_tick_count(), ON_MAX_NONCE);
    

// normally we try not to use goto statements but this is embedded programming
// and it may save us a few bytes?
ocsdh_build_resp:
    stay_awake = device_should_stay_awake((const on_encoded_did_t* const)
      &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]));

    response_pid = get_single_response_pid(pkt->raw_pid,
      ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR, stay_awake);

    if(!setup_pkt_ptr(response_pid, response_txn.pkt, &response_pkt_ptrs))
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    // the response destination will be the transaction's source
    if(on_build_my_pkt_addresses(&response_pkt_ptrs,
      (const on_encoded_did_t* const)
      &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]), NULL) != ONS_SUCCESS)
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }

    #ifdef _ONE_NET_MULTI_HOP
    response_txn.hops = (*txn)->hops;
    response_txn.max_hops = (*txn)->max_hops;
    #endif
    response_txn.key = (*txn)->key;
    *txn = &response_txn;

    // TODO -- what about the hops?  We allowed the application code to
    // change them.  We need to pass that along.  Should we change "device"?

    if(on_build_response_pkt(ack_nack, &response_pkt_ptrs, *txn, device,
      FALSE) != ONS_SUCCESS)
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    if(on_complete_pkt_build(&response_pkt_ptrs, msg_hdr.msg_id, response_pid)
      != ONS_SUCCESS)
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    return ON_MSG_RESPOND;
}


// TODO -- document  
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    on_raw_did_t src_did;
    UInt8 raw_hops_field;
    on_message_status_t status = ON_MSG_DEFAULT_BHVR;
    on_msg_hdr_t msg_hdr;
    BOOL no_response;
    
    if(!ack_nack || !txn)
    {
        // not sure how we got here, but we can't do anything
        return status;
    }    

    // if no_response, then we did not get any response from the receiving
    // device, so don't bother to try to parse nonces, etc. for a non-existent
    // response message.  We'll inform the application code so that it knows,
    // so that it can change the response timeout time time if desired, and
    // so it can pause or abort the transaction as well.
    no_response = (ack_nack->nack_reason == ON_NACK_RSN_NO_RESPONSE);
    
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;

    on_decode(src_did, *(pkt->enc_dst_did), ON_ENCODED_DID_LEN);
   
    #ifndef _ONE_NET_MULTI_HOP
    status = one_net_client_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &(txn->retry));
    #else
    status = one_net_client_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &(txn->retry), pkt->hops, &(pkt->max_hops));
    #endif
    

    if(no_response)
    {
        // we'll change the response timeout here if needed,  If a pause
        // is desired, we'll ignore it.  That will be handled by the function
        // that called this function. 
    
        switch(ack_nack->handle)
        {
            case ON_ACK_SLOW_DOWN_TIME_MS:
              txn->response_timeout += ack_nack->payload->nack_time_ms;
              break;
            case ON_ACK_SPEED_UP_TIME_MS:
              txn->response_timeout -= ack_nack->payload->nack_time_ms;
              break;
        }
    }

    if(status == ON_MSG_DEFAULT_BHVR || status == ON_MSG_CONTINUE)
    {
        if(txn->retry >= ON_MAX_RETRY)
        {
            #ifdef _ONE_NET_MULTI_HOP
            // we may be able to re-send with a higher max hops if there are
            // any repeaters available
            
            if((on_base_param->num_mh_repeaters || !client_joined_network) &&
              txn->max_hops < txn->device->max_hops)
            {
                on_raw_did_t raw_did;
                on_decode(raw_did, *(pkt->enc_dst_did), ON_ENCODED_DID_LEN);
                
                if(txn->max_hops == 0)
                {
                    txn->max_hops = 1;
                }
                else
                {
                    (txn->max_hops) *= 2;
                }
                
                if(txn->max_hops > txn->device->max_hops)
                {
                    txn->max_hops = txn->device->max_hops;
                }
                
                // give the application code a chance to override if it
                // wants to.
                switch(one_net_adjust_hops(&raw_did, &txn->max_hops))
                {
                    case ON_MSG_ABORT: return ON_MSG_ABORT;
                }             
                
                txn->hops = 0;
                txn->retry = 0;
                pkt->hops = txn->hops;
                pkt->max_hops = txn->max_hops;

                return ON_MSG_CONTINUE;
            }
            #endif
            return ON_MSG_TIMEOUT;
        }
    }

    return status;
}
  

// TODO -- document 
static on_message_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    on_msg_hdr_t msg_hdr;
    on_raw_did_t dst;
    
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;
    on_decode(dst ,*(pkt->enc_dst_did), ON_ENCODED_DID_LEN);
    
    
    if(status == ON_MSG_SUCCESS && *msg_type == ON_ADMIN_MSG)
    {
        switch(raw_pld[0])
        {
            case ON_FEATURES_RESP:
            {
                sent_features = TRUE;
                break;
            }
            
            case ON_ADD_DEV_RESP:
            case ON_REMOVE_DEV_RESP:
            case ON_CHANGE_SETTINGS_RESP:
            #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_CHANGE_FRAGMENT_DELAY_RESP:
            #endif
            {
                // success.  We'll check in immediately again.
                #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
                send_keep_alive(0, 0);
                #elif _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                send_keep_alive(0);
                #else
                send_keep_alive();
                #endif
                break;
            }
            
            case ON_KEEP_ALIVE_RESP:
            {
                // If we get anything but a new keep-alive interval, we'll
                // need to send another keep-alive message very soon, but
                // not immediately.  We're pausing and making everything
                // random because the master may be informing lots of clients
                // about a change, and if so, we could have a lot of
                // collisions.  The randomness and the slight pause will
                // hopefully cut down.
                
                // TODO / Note
                // How long and whether to pause and who should pause is
                // something that needs to be perfected.  It will also vary
                // greatly between networks based on...
                
                // 1.  Whether any devices are multi-hop.
                // 2.  Whether the network even bothers to inform all the
                //     devices when a device joins or is removed.  All devices
                //     need to notified of key changes, of course.
                // 3.  How many devices are "simple" devices.  "Simple"
                //     devices can be assumed to 1) not care about device
                //     additions, and 2) Cannot handle sophisticated decisions
                //     about how to stagger messages, when messages are to be
                //     sent in the future, when they should "expire", etc.
                // 4.  How many devices are in the network.
                // 5.  How many devices, if any, sleep.
                
                
                // All in all, this is a work in progress and it may be useful
                // to tweak this protocol to your own system's needs.  I was
                // trying to make a one-size-fits-all protocol and there just
                // isn't one, I don't think.
                
                
                // Setting a slight pause till the next check-in.  If there
                // is no additional check-in needed, we will receive a NEW
                // keep-alive interval while will override this one.  In fact
                // that is usually what is going to happen.
                
                
                #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                master->keep_alive_interval = MS_TO_TICK(750 +
                  one_net_prand(get_tick_count(), 2000));
                #else
                master->keep_alive_interval = 0;
                #endif
                
                               
                if(ack_nack->handle == ON_ACK_ADMIN_MSG)
                {
                    BOOL send_confirm_admin_msg = FALSE;
                    UInt8 admin_msg_type;
                    
                    switch(ack_nack->payload->admin_msg[0])
                    {
                        case ON_NEW_KEY_FRAGMENT:
                        {
                            // we MAY be using the wrong key.  The master is
                            // giving us the right device to use.  If we are
                            // already using it, copy it into the key and
                            // shift.
                            if(one_net_memcmp(
                              &(ack_nack->payload->admin_msg[1]),
                              &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                              ONE_NET_XTEA_KEY_FRAGMENT_SIZE) == 0)
                            {
                                // shift the current key left.
                                one_net_memmove(on_base_param->current_key,
                                  &(on_base_param->current_key[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                                  3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
                                  
                                // replace the last fragment with the one we
                                // just received
                                one_net_memmove(
                                  &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                                  &(ack_nack->payload->admin_msg[1]),
                                  ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
                            }                            
                            break;
                        }
                        
                        case ON_ADD_DEV:
                        {
                            BOOL this_device_added;
                            on_encoded_did_t* added_device =
                              (on_encoded_did_t*)
                              &(ack_nack->payload->admin_msg[1]);
                            on_raw_did_t raw_did_added;
                            if(on_decode(raw_did_added, *added_device,
                              ON_ENCODED_DID_LEN) != ONS_SUCCESS)
                            {
                                break;
                            }
                            
                            this_device_added = is_my_did(added_device);
                            
                            if(!this_device_added)
                            {
                                one_net_client_client_added(&raw_did_added);
                            }

                            if(this_device_added && !client_joined_network)
                            {
                                one_net_client_invite_result(&raw_did_added,
                                  ONS_SUCCESS);
                                client_joined_network = TRUE;
                                // TODO -- seems like this should have
                                // been set elsewhere?
                                master->flags |= ON_JOINED;
                                client_looking_for_invite = FALSE;
                            }
                            
                            #ifdef _ONE_NET_MULTI_HOP
                            on_base_param->num_mh_devices =
                              ack_nack->payload->admin_msg[3];
                            on_base_param->num_mh_repeaters =
                              ack_nack->payload->admin_msg[4];
                            #endif

                            send_confirm_admin_msg = TRUE;
                            admin_msg_type = ON_ADD_DEV_RESP;
                            break;
                        }
                        
                        case ON_RM_DEV:
                        {
                            on_raw_did_t raw_did_added;
                            if(on_decode(raw_did_added,
                              &(ack_nack->payload->admin_msg)[1],
                              ON_ENCODED_DID_LEN) != ONS_SUCCESS)
                            {
                                break;
                            }
                            one_net_client_client_removed(&raw_did_added,
                              is_my_did((on_encoded_did_t*)
                              &(ack_nack->payload->admin_msg)[1]));
                              
                            #ifdef _ONE_NET_MULTI_HOP
                            on_base_param->num_mh_devices =
                              ack_nack->payload->admin_msg[3];
                            on_base_param->num_mh_repeaters =
                              ack_nack->payload->admin_msg[4];
                            #endif                              
                              
                            send_confirm_admin_msg = TRUE;
                            admin_msg_type = ON_REMOVE_DEV_RESP;
                            break;
                        }
                        
                        case ON_CHANGE_SETTINGS:
                        {
                            master->flags = ack_nack->payload->admin_msg[1];
                            send_confirm_admin_msg = TRUE;
                            admin_msg_type = ON_CHANGE_SETTINGS_RESP;
                            break;
                        }
                    }
                    
                    if(send_confirm_admin_msg)
                    {
                        // we'll send a confirmation message here.  We want
                        // this to go out BEFORE the next keep-alive message
                        // so we'll set it to go out slightly before the
                        // next keep-alive check-in if we are able to pause.
                        one_net_client_send_single(ONE_NET_RAW_SINGLE_DATA,
                          ON_ADMIN_MSG, &admin_msg_type, 1,
                          ONE_NET_HIGH_PRIORITY, NULL, &MASTER_ENCODED_DID
                          #ifdef _PEER
                          , FALSE, ONE_NET_DEV_UNIT
                          #endif
                          #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                          , 0
                          #endif
                          #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
                          , 0
                          #endif
                          );
                        break;
                    }
                    
                    // success.  We'll check in immediately again.
                    #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
                    send_keep_alive(0, 0);
                    #elif _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                    send_keep_alive(0);
                    #else
                    send_keep_alive();
                    #endif

                    break;
                }
                
                // No admin messages within the keep-alive response from
                // the master.  We were sent a new interval.
                master->keep_alive_interval = ack_nack->payload->ack_time_ms;
                ont_set_timer(ONT_KEEP_ALIVE_TIMER, MS_TO_TICK(
                  master->keep_alive_interval));
                rcvd_keep_alive = TRUE;
                break;
            }
        }
    }

    #ifndef _ONE_NET_MULTI_HOP
    one_net_client_single_txn_status(status, (*txn)->retry,
      msg_hdr, raw_pld, &dst, ack_nack);
    #else
    one_net_client_single_txn_status(status, (*txn)->retry,
      msg_hdr, raw_pld, &dst, ack_nack, pkt->hops);
    #endif


    if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR || *msg_type !=
      ON_ADMIN_MSG)
    {
        // Reset the Keep-Alive Timer
        ont_set_timer(ONT_KEEP_ALIVE_TIMER,
          MS_TO_TICK(master->keep_alive_interval));
    }
    
    return ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR ? ON_MSG_SUCCESS :
      ON_MSG_FAIL;
}




#ifdef _BLOCK_MESSAGES_ENABLED
// TODO -- document
static on_message_status_t on_client_block_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}


// TODO -- document  
static on_message_status_t on_client_handle_block_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
  

// TODO -- document 
static on_message_status_t on_client_block_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
#endif




#ifdef _STREAM_MESSAGES_ENABLED
// TODO -- document
static on_message_status_t on_client_stream_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}


// TODO -- document  
static on_message_status_t on_client_handle_stream_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
  

// TODO -- document 
static on_message_status_t on_client_stream_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
#endif




/*!
    \brief Initializes internal data structures.

    This will also initialize the base one_net functionality.

    \param void

    \return ONS_SUCCESS if the internals were successfully initialized
            ONS_INTERNAL_ERR if initializing the net layer was not successful
*/
static one_net_status_t init_internal(void)
{
    pkt_hdlr.single_data_hdlr = &on_client_single_data_hdlr;
    pkt_hdlr.single_ack_nack_hdlr =
      &on_client_handle_single_ack_nack_response;
    pkt_hdlr.single_txn_hdlr = &on_client_single_txn_hdlr;
    pkt_hdlr.adj_recip_list_hdlr = &on_client_adjust_recipient_list; 
    
    #ifdef _BLOCK_MESSAGES_ENABLED
    pkt_hdlr.block_data_hdlr = &on_client_block_data_hdlr;
    pkt_hdlr.block_ack_nack_hdlr =
      &on_client_handle_block_ack_nack_response;
    pkt_hdlr.block_txn_hdlr = &on_client_block_txn_hdlr;
    #endif
    
    #ifdef _STREAM_MESSAGES_ENABLED
    pkt_hdlr.stream_data_hdlr = &on_client_stream_data_hdlr;
    pkt_hdlr.stream_ack_nack_hdlr =
      &on_client_handle_stream_ack_nack_response;
    pkt_hdlr.stream_txn_hdlr = &on_client_stream_txn_hdlr;
    #endif

    one_net_send_single = &one_net_client_send_single;
    get_sender_info = &sender_info;
    device_is_master = FALSE;
    one_net_init();
    return ONS_SUCCESS;
} // init_internal //


/*!
    \brief Finds the sender info (or a location for the sender info).

    Loops through and finds the information for the sending device.  If the
    device has not heard from the sender before, a new location shall be
    returned.

    The return value should be checked for 0.  The expected_nonce and last nonce
    should then be compared.  If these two values are equal, then it is a new
    location so a NACK should be sent to the sender, and the new nonce filled
    out.  The last nonce value should not be a valid nonce value and should be
    left unchanged for the time being.

    \param[in] DID The device id of the sender.

    \return Pointer to location that holds the sender information (should be
      checked for 0, and should be checked if a new location).
*/
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID)
{
    // indexes
    UInt8 i, match_idx, max_lru_idx;

    // either the lru of the matched device, or the max lru in the list
    UInt8 lru = 0;

    if(!DID)
    {
        return 0;
    } // if parameter is invalid //

    max_lru_idx = match_idx = 0;

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        return &master->device;
    } // if the MASTER is the sender //

    // loop through and find the sender's information
    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(sending_dev_list[i].sender.did)))
        {
            match_idx = i;
            lru = sending_dev_list[i].lru;
            break;
        } // if the sender info was found //

        if(lru < sending_dev_list[i].lru)
        {
            lru = sending_dev_list[i].lru;
            max_lru_idx = i;
        } // if the device has a higher lru than the current max //
    } // loop to find sender info //

    if(match_idx != i)
    {
        UInt8* ptr = on_base_param->sid;
        // replace the least recently used device
        match_idx = max_lru_idx;
        one_net_memmove(sending_dev_list[match_idx].sender.did, *DID,
          sizeof(sending_dev_list[match_idx].sender.did));
        sending_dev_list[match_idx].sender.expected_nonce =
          one_net_prand(get_tick_count(), ON_MAX_NONCE);
        sending_dev_list[match_idx].sender.send_nonce =
          one_net_prand(get_tick_count(), ON_MAX_NONCE);
        sending_dev_list[match_idx].sender.last_nonce =
          one_net_prand(get_tick_count(), ON_MAX_NONCE);
        sending_dev_list[match_idx].sender.features = FEATURES_UNKNOWN;
        sending_dev_list[match_idx].sender.msg_id =
          one_net_prand(get_tick_count(), ON_MAX_MSG_ID);
        
        #ifdef _ONE_NET_MULTI_HOP
        sending_dev_list[match_idx].sender.hops = 0;
        sending_dev_list[match_idx].sender.max_hops = ON_MAX_HOPS_LIMIT;
        #endif
    } // if the device was not found in the list //

    if(lru)
    {
        // update the lru's in the list
        for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
        {
            if(i == match_idx)
            {
                sending_dev_list[i].lru = 0;
            } // if it is the index that matched //
            else
            {
                sending_dev_list[i].lru++;
            } // else it is not the index that matched //
        } // loop through devices and update the lrus //
    } // if the device was not the least recently used //

    return &(sending_dev_list[match_idx].sender);
} // sender_info //


/*!
    \brief Sends a single data message.
    
    The message is either sent to the peer list or only to the specific device
    that is passed in.
    
    \param[in] raw_pid The raw pid of the message.
    \param[in] msg_type The message type of the message(admin, application, etc.)
    \param[in] data The data to send.
    \param[in] data_len The length of DATA (in bytes).
    \param[in] priority The priority of the transaction.
    \param[in] src_did The source of the message (if NULL, the source will be
      assumed to be this device).
    \param[in] enc_dst The device the message is destined for.  This can be
      NULL if the message is to be sent to the peer list.
    \param[in] send_to_peer_list If true, the message will be sent to.
    \param[in] src_unit The unit that the message originated from.  Relevant
      only if sending to the peer list.
	\param[in] send_time_from_now Time to pause before sending.  NULL is interpreted as "send immediately"
	\param[in] expire_time_from_now If after this time, don't bother sending.  NULL is interpreted as "no expiration"
    
    \return ONS_SUCCESS If the single data has been queued successfully.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_RSRC_FULL If no resources are currently available to handle the
              request.
*/
static one_net_status_t one_net_client_send_single(UInt8 raw_pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t expire_time_from_now
  #endif
  )
{
    if(push_queue_element(raw_pid, msg_type, raw_data, data_len, priority,
      src_did, enc_dst
      #ifdef _PEER
          , send_to_peer_list, src_unit
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , send_time_from_now
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , expire_time_from_now
      #endif
      ) == NULL)
    {
        return ONS_RSRC_FULL;
    }

    return ONS_SUCCESS;
}


#if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static one_net_status_t send_keep_alive(tick_t send_time_from_now,
  tick_t expire_time_from_now)
#elif _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
static one_net_status_t send_keep_alive(tick_t send_time_from_now)
#else
static one_net_status_t send_keep_alive(void)
#endif
{
    UInt8 raw_pld[5];
    raw_pld[0] = ON_KEEP_ALIVE_RESP;
    
    // copy the last fragment of the key into the message.  The master
    // will check to make sure we have the right key.  If not, it will
    // send back the correct last fragment.
    one_net_memmove(&raw_pld[1],
      &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
      ONE_NET_XTEA_KEY_FRAGMENT_SIZE);

    return one_net_client_send_single(ONE_NET_RAW_SINGLE_DATA,
      ON_ADMIN_MSG, raw_pld, 5, ONE_NET_LOW_PRIORITY,
      NULL, (on_encoded_did_t*) MASTER_ENCODED_DID
      #ifdef _PEER
      , FALSE,
      ONE_NET_DEV_UNIT
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , send_time_from_now
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
      // if it can't get out of the queue within five seconds, cancel it.
      , expire_time_from_now
      #endif
      );
}


/*!
    \brief Looks for the Invite packet when the device is being added to the
      network.

    Scans the channels (in the shared multitasking environment) for the MASTER
    Invite New CLIENT packet.  This packet is read, and parsed here.

    \param void

    \return TRUE If the invite was found
            FALSE If the invite was not found
*/
static BOOL look_for_invite(void)
{
    on_txn_t* this_txn = &invite_txn;
    on_pkt_t* this_pkt_ptrs = &data_pkt_ptrs;
    

    if(on_rx_packet(&MASTER_ENCODED_DID, &invite_txn, &this_txn, &this_pkt_ptrs,
      raw_payload_bytes) != ONS_PKT_RCVD)
    {
        #ifdef _ENHANCED_INVITE
        if(ont_expired(ONT_INVITE_TIMER))
    	{
            client_invite_timed_out = TRUE;
    	    client_looking_for_invite = FALSE;
            one_net_client_invite_result(NULL, ONS_TIME_OUT);
            on_state = ON_IDLE;
    	}
        else if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
        #else
        if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
        #endif
        {
            // need to try the next channel
            on_base_param->channel++;
            #ifndef _ENHANCED_INVITE
            if(on_base_param->channel > ONE_NET_MAX_CHANNEL)
            {
                on_base_param->channel = 0;
            } // if the channel has overflowed //
            #else
            if(on_base_param->channel > high_invite_channel)
            {
                on_base_param->channel = low_invite_channel;
            } // if the channel has overflowed //
            #endif
		
            one_net_set_channel(on_base_param->channel);
            ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_SCAN_CHANNEL_TIME);
        } // if the timer expired //    

        return FALSE;
    }

    // we have received an invitation.  First check the version.
    if(raw_payload_bytes[ON_INVITE_VERSION_IDX] != ON_INVITE_VERSION_IDX)
    {
        return FALSE;
    }
    
    // We'll fill in some information and return TRUE.
    one_net_memmove(on_base_param->sid, *(this_pkt_ptrs->enc_nid),
      ON_ENCODED_NID_LEN);
    on_encode(&(on_base_param->sid[ON_ENCODED_NID_LEN]),
      &raw_payload_bytes[ON_INVITE_ASSIGNED_DID_IDX], ON_ENCODED_DID_LEN);
    one_net_memmove(on_base_param->current_key,
      &raw_payload_bytes[ON_INVITE_KEY_IDX], ONE_NET_XTEA_KEY_LEN);
    master->device.features =  
      *((on_features_t*)(&raw_payload_bytes[ON_INVITE_FEATURES_IDX]));
    #ifdef _ONE_NET_MULTI_HOP
    master->device.max_hops = features_max_hops(master->device.features);
    #endif

    return TRUE;
} // look_for_invite //


/*!
    \brief Handles admin packets.

    \param[in] SRC_DID The sender of the admin packet.
    \param[in] DATA The admin packet.
    \param[in/out] txn The transaction of the admin packet.
    \param[out] ack_nack acknowledgement or negative acknowledgement
                of a message

    \return ON_MSG_CONTINUE if processing should continue
            ON_MSG_IGNORE if the message should be ignored
*/
static on_message_status_t handle_admin_pkt(const on_encoded_did_t * const
  SRC_DID, const UInt8 * const DATA, on_txn_t* txn, on_ack_nack_t* ack_nack)
{
    on_message_status_t status;
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;


    switch(DATA[0])
    {
        case ON_NEW_KEY_FRAGMENT:
        {
            // there has been a key change.  We may already have the new key
            // and we may not.  Check here.  If our last key fragment matches
            // what is in the message, we already have the key, so don't
            // replace anything.  If not, replace the last fragment with what
            // we received.
            
            if(one_net_memcmp(&DATA[1],
              &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              ONE_NET_XTEA_KEY_FRAGMENT_SIZE) != 0)
            {
                // we are not using the correct key.  Copy it.
                
                // first shift the current fragments left.
                one_net_memmove(on_base_param->current_key,
                  &(on_base_param->current_key[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                  3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
                
                // now copy in the new fragment we just received.
                one_net_memmove(
                  &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                  &DATA[1], ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
            }
            
            
            // we now have the right key.  Send it back in the ACK.
            ack_nack->handle = ON_ACK_KEY_FRAGMENT;
            one_net_memmove(ack_nack->payload->key_frag, &DATA[1],
              ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
              
            confirm_key_change = TRUE;

            // we'll want to check in again soon, but we'll do it at a random
            // time.  There may be a lot of traffic out there trying to check
            // in with the new key.  We'll ACK it now, then check in.  If we
            // are a device that sleeps, we get precedence.  If we don't
            // sleep, we'll wait a little longer so any devices that do sleep
            // can hopefully check in first.
            
            // TODO -- is this a good technique to prevent a lot of messages
            // crashing into each other?
            #ifdef _DEVICE_SLEEPS
            ont_set_timer(ONT_KEEP_ALIVE_TIMER, MS_TO_TICK(500 +
              one_net_prand(get_tick_count(), 1500)));
            #else
            ont_set_timer(ONT_KEEP_ALIVE_TIMER, MS_TO_TICK(2000 +
              one_net_prand(get_tick_count(), 2500)));
            #endif
            break;
        } // change key case //

        #ifdef _PEER
        case ON_ASSIGN_PEER:
        {
            switch(one_net_add_peer_to_list(DATA[1 + ON_PEER_SRC_UNIT_IDX],
              NULL, (const on_encoded_did_t * const)
              &DATA[1 + ON_PEER_DID_IDX], DATA[1 + ON_PEER_PEER_UNIT_IDX]))
            {
                case ONS_RSRC_FULL: ack_nack->nack_reason =
                       ON_NACK_RSN_RSRC_UNAVAIL_ERR; break;
                case ONS_SUCCESS: break;
                default: ack_nack->nack_reason = ON_NACK_RSN_INTERNAL_ERR;
            }
            
            break;
        } // assign peer case //
        
        case ON_UNASSIGN_PEER:
        {
            switch(one_net_remove_peer_from_list(DATA[1+ON_PEER_SRC_UNIT_IDX],
              NULL, (const on_encoded_did_t * const)
              &DATA[1 + ON_PEER_DID_IDX], DATA[1 + ON_PEER_PEER_UNIT_IDX]))
            {
                case ONS_SUCCESS: break;
                default: ack_nack->nack_reason = ON_NACK_RSN_INTERNAL_ERR;
            }
            
            break;
        } // unassign peer case //
        #endif
        
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_CHANGE_FRAGMENT_DELAY:
        {
            // changing both within one message.  If a value is 0, then, it
            // is irrelevant.
            UInt16 new_frag_low = one_net_byte_stream_to_int16(
              &DATA[1 + ON_FRAG_LOW_IDX]);
            UInt16 new_frag_high = one_net_byte_stream_to_int16(
              &DATA[1 + ON_FRAG_HIGH_IDX]);
              
            if(new_frag_low == 0)
            {
                new_frag_low = on_base_param->fragment_delay_low;
            }
            if(new_frag_high == 0)
            {
                new_frag_high = on_base_param->fragment_delay_high;
            }
            
            if(new_frag_low < new_frag_high)
            {
                // Invalid.  Low priority cannot be less than high-priority
                // delay.
                ack_nack->nack_reason = ON_NACK_RSN_BAD_DATA_ERR;
                break;
            }
            
            on_base_param->fragment_delay_low = new_frag_low;
            on_base_param->fragment_delay_high = new_frag_high;
            rcvd_fragment_delays = TRUE;
            break;
        } // update fragment delay(s) case //
        #endif

        case ON_CHANGE_KEEP_ALIVE:
        {
            master->keep_alive_interval = one_net_byte_stream_to_int32(
              &DATA[1]);
        } // change keep-alive case : intentional fall-through //
        case ON_KEEP_ALIVE_QUERY:
        {
            ack_nack->handle = ON_ACK_TIME_MS;
            ack_nack->payload->ack_time_ms = master->keep_alive_interval;
            rcvd_keep_alive = TRUE;
            break;
        } // keep alive query case //
        
        case ON_CHANGE_SETTINGS:
        {
            master->flags = DATA[1];
            ack_nack->handle = ON_ACK_VALUE;
            ack_nack->payload->ack_value.uint8 = master->flags;
            rcvd_settings = TRUE;
            break;
        } // update settings case //
        
        case ON_RM_DEV:
        {
            // first check if we are the one being removed.
            on_encoded_did_t* removed_device = (on_encoded_did_t*) &DATA[1];

            if(is_my_did(removed_device))
            {
                // we're the ones being removed
                removed = TRUE;
                
                // we'll stay alive for another 3 seconds to complete any
                // pending transactions so other devices won't miss any ACKs
                // or NACKs, then delete ourselves.  Just use the invite
                // timer.
                ont_set_timer(ONT_INVITE_TIMER,
                  MS_TO_TICK(3000));
            }
            else
            {
                // Remove the device from the list of sending devices,
                // if applicable.  Just make the features unknown and that
                // will force a revalidation next time any device tries to
                // use this did.
                tick_t tick_now = get_tick_count();
                on_sending_device_t* device = sender_info(removed_device);
                device->features = FEATURES_UNKNOWN;
                
                // for security, change the nonces and the message id too.
                device->send_nonce = one_net_prand(tick_now, ON_MAX_NONCE);
                device->expected_nonce = one_net_prand(tick_now, ON_MAX_NONCE);
                device->last_nonce = one_net_prand(tick_now, ON_MAX_NONCE);
                device->msg_id = one_net_prand(tick_now, ON_MAX_MSG_ID);

                #ifdef _PEER
                // delete any peer assignments for this device
                one_net_remove_peer_from_list(ONE_NET_DEV_UNIT, NULL,
                  removed_device, ONE_NET_DEV_UNIT);
                #endif
                {
                    on_raw_did_t raw_did;
                    on_decode(raw_did, *removed_device, ON_ENCODED_DID_LEN);
                    one_net_client_client_removed(&raw_did, FALSE);
                }
                
                #ifdef _ONE_NET_MULTI_HOP
                on_base_param->num_mh_devices =
                  ack_nack->payload->admin_msg[3];
                on_base_param->num_mh_repeaters =
                  ack_nack->payload->admin_msg[4];
                #endif
            }
            
            break;
        }
        
        case ON_ADD_DEV:
        {
            BOOL this_device_added;
            on_encoded_did_t* added_device = (on_encoded_did_t*) &DATA[1];
            on_raw_did_t raw_did;
            on_decode(raw_did, *added_device, ON_ENCODED_DID_LEN);
            this_device_added = is_my_did(added_device);
            if(this_device_added && !client_joined_network)
            {
                one_net_client_invite_result(&raw_did, ONS_SUCCESS);
                client_joined_network = TRUE;
                master->flags |= ON_JOINED; // TODO -- seems like this should have
                                    // been set elsewhere?
                client_looking_for_invite = FALSE;
            }
            else
            {
                one_net_client_client_added(&raw_did);
            }
            
            #ifdef _ONE_NET_MULTI_HOP
            on_base_param->num_mh_devices =
              ack_nack->payload->admin_msg[3];
            on_base_param->num_mh_repeaters =
              ack_nack->payload->admin_msg[4];
            #endif
            break;
        }

        case ON_KEEP_ALIVE_RESP:
            break;  // not sure why a client would get this, but ACK it.
        
        default:
        {
            ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            ack_nack->handle = ON_NACK_FEATURES;
            ack_nack->payload->features = THIS_DEVICE_FEATURES;
        } // default case //
    } // switch(DATA[ON_ADMIN_MSG_ID_IDX]) //

    return ON_MSG_CONTINUE;
}


/*!
    \brief Checks to see whether the device needs to send a check-in message
      to the master and if so, sends it.

    \return TRUE if a message was sent
            FALSE otherwise.
*/
static BOOL check_in_with_master(void)
{
    UInt8 raw_pld[5];
    
    if(!ont_inactive_or_expired(ONT_KEEP_ALIVE_TIMER))
    {
        return FALSE;
    }

    // we are part of the network already or are in the process of
    // joining the network and are sending our first keep-alive
    // message.
    raw_pld[0] = ON_KEEP_ALIVE_RESP;
    
    // copy the last fragment of the key into the message.  The master
    // will check to make sure we have the right key.  If not, it will
    // send back the correct last fragment.
    one_net_memmove(&raw_pld[1],
      &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
      ONE_NET_XTEA_KEY_FRAGMENT_SIZE);

    #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
    if(send_keep_alive(0, 0) == ONS_SUCCESS)
    #elif _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
    if(send_keep_alive(0) == ONS_SUCCESS)
    #else
    if(send_keep_alive() == ONS_SUCCESS)
    #endif
    {
        // this should get reset to something else in the transaction
        // handler long before this timer expires, but just in case it
        // doesn't reset it for 10 seconds here.
        ont_set_timer(ONT_KEEP_ALIVE_TIMER, MS_TO_TICK(10000));
        return TRUE;
    }
    
    return FALSE;
}


#ifndef _ONE_NET_SIMPLE_DEVICE
/*!
    \brief Allows for adjustment of the recipient list for a message

    This function is called after a single message has been popped
    from the queue and ready to send.  ONE-NET has set up a list of
    destination dids and destination units that the message will be sent to.
    The destination units are relevant only if the message type is ON_APP_MSG.
    
    This code can do one of four things.
    
    1) Do nothing.  In this instance the list remains unchanged and this will
       be the list that is sent.
    2) Cancel the message.  If this is desired, *recipient_list should be set
       to NULL.
    3) A new list can replace the old list.  In this case the appliation code
       should change *recipient_list to point to the list it wants to have
       sent.
    4) The existing list can be used, but the code can add to it,
       remove from it, or reorder it.
   

    Lists can be emptied by setting the "num_recipients" field to 0.  Elements
    can be added using the add_recipient_to_recipient_list function.  Elements
    can be removed using the "remove_recipient_from_recipient_list" function.

    
    \param[in] msg The message that is to be sent.
    \param[in/out] A pointer to a pointer to a list of recipients.  The list
                   itself can be changed by changing the pointer.  Change the
                   pointer to NULL to cancel the message.  See the main
                   description for how to adjust lists.
*/
static void on_client_adjust_recipient_list(const on_single_data_queue_t*
  const msg, on_recipient_list_t** recipient_send_list)
{
    one_net_adjust_recipient_list(msg, recipient_send_list);
}
#endif


#endif // if _ONE_NET_CLIENT is defined //


//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT
