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
#include "one_net_port_const.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "one_net_client_port_specific.h"
#include "one_net_client_port_const.h"
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
on_sending_dev_list_item_t sending_dev_list[ONE_NET_RX_FROM_DEVICE_COUNT] = {0};

//! Set to true upon being deleted from the network.  There will be a slight
//! two second pause before this device actually removes itself to give any
//! pending transactions to complete.
static BOOL removed = FALSE;



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
static on_message_status_t on_client_block_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* block_pkt, on_ack_nack_t* ack_nack);
static on_message_status_t on_client_handle_block_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack);
static on_message_status_t on_client_block_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);
#endif
#ifdef _STREAM_MESSAGES_ENABLED
static on_message_status_t on_client_stream_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* stream_pkt, on_ack_nack_t* ack_nack);
static on_message_status_t on_client_handle_stream_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack);
static on_message_status_t on_client_stream_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);
#endif
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);
#ifndef _ONE_NET_SIMPLE_CLIENT
static on_sending_dev_list_item_t* get_sending_dev_list_item_t(
  const on_encoded_did_t* DID);
#endif
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);
static one_net_status_t init_internal(void);
#ifndef _ONE_NET_SIMPLE_CLIENT
static BOOL send_new_key_request(void);
#endif
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


#ifndef _ONE_NET_SIMPLE_CLIENT
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
    
    
#ifndef _ONE_NET_SIMPLE_CLIENT
/*!
    \brief Sets the "slideoff" flag in the client device list for a device.
    
    See the comments for the on_client_unlock_device_slideoff() function for
    more information.
    
    To prevent internal errors, replay attacks, and any other problems resulting
    from a client being bumped off of the client device list, a flag can be
    set to retain that client's position in the list.  If this flag is set, the device
    will not be removed from the list if a new device needs to be placed on the list.
    The flag is the "slideoff" element of the "on_sending_dev_list_item_t" structure.
    
    That flag can be one of three values.  See the "device_slideoff_t" structure for
    more details on the values of this flag.  This function sets that flag.
    
    If the flag is already set to ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK, this function does
    nothing.  If the device is not already on the table, this function does nothing.
    Otherwise the "slideoff" flag is set to the parameter passed to this function.

    \param[in] enc_did The DID of the device.
    \param[in] slideoff The value to set the device's slideoff flag to.

    \return none
*/
void on_client_set_device_slideoff(const on_encoded_did_t* enc_did,
  device_slideoff_t slideoff)
{
    on_sending_dev_list_item_t* item = get_sending_dev_list_item_t(enc_did);
    
    // if the devide's flag is "locked", you must call on_client_unlock_device_slideoff()
    // to unlock.
    if(!item || item->slideoff == ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK)
    {
        return;
    }
    
    item->slideoff = slideoff;
}


/*!
    \brief Removes a "lock" on a device in the client's device list.  This
    function should be called only from the application code.

    To prevent internal errors, replay attacks, and any other problems resulting
    from a client being bumped off of the client device list, a "lock" can be
    placed on that client's position in the list.  Examples of when it is important
    for a device to not be allowed to slide off of the device list are...
    
    1. We are in the middle of a transaction (generally a block or stream transaction)
       and a pointer has been set somewhere to point to another device.  If the device
       falls off of the list, that pointer may no longer be valid.
    2. We expect to communicate with this device a lot and we don't want to have to
       "re-sync" with it to get its features, the number of hops, etc.
    3. Security concerns involving replay attacks.  When a device falls off of the
       list, its pairwise message id is no longer stored and it is therefore more vulnerable
       to replay attacks because it may not recognize that a message id is being re-used.
       
    This function "unlocks" a lock that has already been placed on it.  If no "lock" is on
    the device, the function has no effect except that it might rearrange the
    least-recent-used values.  To actually set the flag so that the device can
    "slide off", a subsequent call to the on_client_set_device_slideoff() function must
    be called with a parameter of ON_DEVICE_ALLOW_SLIDEOFF.
    
    To LOCK a device, this function should not be called.  Instead, call
    on_client_set_device_slideoff() and pass it the ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK
    parameter.
    
    Note that the master will never slide off the devicelist under any circumstances.
    
    Also note that devices requiring high levels of protection against replay attacks
    should be locked by the application code and remain locked.  Developers should, if
    the device has enough RAM, make the device list large enough so that devices will never
    fall off.
    

    \param[in] enc_did The DID of the device to unlock.

    \return none
*/
void on_client_unlock_device_slideoff(const on_encoded_did_t* enc_did)
{
    on_sending_dev_list_item_t* item = get_sending_dev_list_item_t(enc_did);
    if(item && item->slideoff == ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK)
    {
        item->slideoff = ON_DEVICE_PROHIBIT_SLIDEOFF;
    }
}
#endif

/*!
    \brief Initializes a CLIENT to start looking for an invite message.

    This function should be called the first time a device starts up, or when
    the device should attempt to join another network.  Once a device has
    joined a network, one_net_client_init should be called to reinitialize the
    CLIENT.

    \param INVITE_KEY The unique key of this CLIENT to decrypt invite packets
      with.
    \param[in] min_channel lowest channel to scan on.
    \param[in] max_channel highest channel to scan on.
    \param[in] timeout_time Length of time in milliseconds to listen for the invite.
	  0 means indefinite.

    \return ONS_SUCCESS Successfully initialized the CLIENT.
            ONS_BAD_PARAM if the parameter is invalid.
*/
#if !defined(_ENHANCED_INVITE)
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY)
#else
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time)
#endif
{
    // copy some parameters over to the base parameters and the master
    // parameters.
    
    // initialize SID to invalid (all zeroes).
    one_net_memset(on_base_param->sid, 0xB4, ON_ENCODED_SID_LEN);
    on_base_param->version = ON_PARAM_VERSION;
    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    one_net_memmove(&(on_base_param->current_key), *INVITE_KEY,
      sizeof(on_base_param->current_key));
    #ifdef _BLOCK_MESSAGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
    #endif 
    master->flags = 0x00;
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
    #ifdef _PEER
    one_net_reset_peers();
    #endif    

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
        ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(
          ONE_NET_CLIENT_INVITE_DURATION));
    }
    #else
    on_base_param->channel = one_net_prand(get_tick_count(),
      ONE_NET_MAX_CHANNEL);
    ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(ONE_NET_CLIENT_INVITE_DURATION));
    #endif
    
    // set up packet handlers, etc.
    init_internal();

    ont_set_timer(ONT_GENERAL_TIMER, MS_TO_TICK(ONE_NET_SCAN_CHANNEL_TIME));
    on_state = ON_JOIN_NETWORK;
    
    #ifdef _AUTO_SAVE
    save = TRUE;
    #endif    
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
    
    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status;
    } // if initializing the internals failed //
    
    #ifdef _PEER
    if(peer_param != NULL)
    {
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
        #ifndef _ENHANCED_INVITE
        one_net_client_reset_client(one_net_client_get_invite_key());
        #else
        one_net_client_reset_client(one_net_client_get_invite_key(), 0,
          ONE_NET_MAX_CHANNEL, 0);
        #endif
        return 0;
    }

    // if we are not in a network yet, we only accept messages from the
    // master.
    if(!client_joined_network)
    {
        one_net_memmove(expected_src_did, MASTER_ENCODED_DID,
          ON_ENCODED_DID_LEN);
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
                master->flags = 0;
                #ifdef _AUTO_SAVE
                save = TRUE;
                #endif
                one_net_client_client_removed(NULL, TRUE);
                #ifndef _ENHANCED_INVITE
                one_net_client_reset_client(one_net_client_get_invite_key());
                #else
                one_net_client_reset_client(one_net_client_get_invite_key(), 0,
                  ONE_NET_MAX_CHANNEL, 0);
                #endif
                return 0;
            }
            
            if(!client_joined_network &&
              ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                // we started to accept an invite, but for whatever reason
                // we did not finish
                #ifdef _ENHANCED_INVITE
                one_net_client_reset_client(one_net_client_get_invite_key(), 0,
                  ONE_NET_MAX_CHANNEL, 0);
                #else
                one_net_client_reset_client(one_net_client_get_invite_key());
                #endif
                #ifdef _AUTO_SAVE
                save = TRUE;
                #endif
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
              MS_TO_TICK(invite_transaction_timeout));
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
    if(txn || on_state != ON_LISTEN_FOR_DATA)
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
    
    #ifdef _DEVICE_SLEEPS
    if(ont_get_timer(ONT_KEEP_ALIVE_TIMER))
    {
        sleep_time = 0;
    }
    #endif
    
    #ifdef _NON_VOLATILE_MEMORY
    // if we're going to do anything soon, don't save since things might change
    // soon.
    
    // TODO -- Do we want the sleep time check?
    if(sleep_time > MS_TO_TICK(1000) && save)
    {        
        one_net_client_save_settings();
        save = FALSE;
    }
    #endif
    
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
    
    if(param_len >= 0 && param_len != CLIENT_NV_PARAM_SIZE_BYTES)
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


#ifdef _BLOCK_MESSAGES_ENABLED
on_nack_rsn_t on_client_get_default_block_transfer_values(
  const on_encoded_did_t* dst, UInt32 transfer_size, UInt8* priority,
  UInt8* chunk_size, UInt16* frag_delay, UInt16* chunk_delay, UInt8* data_rate,
  UInt8* channel, UInt16* timeout, on_ack_nack_t* ack_nack)
{
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    on_sending_device_t* device = &master->device;
    BOOL dst_features_known;
    
    if(!is_master_did(dst))
    {
        device = sender_info(dst);
    }
    
    *timeout = DEFAULT_BLOCK_STREAM_TIMEOUT;
    
    dst_features_known = device && features_known(device->features);
    
    if(dst_features_known && !features_block_capable(
      device->features))
    {
        return ON_NACK_RSN_DEVICE_FUNCTION_ERR;
    }

    *nr = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;
    // Set to the current parameters first.
    *data_rate = ONE_NET_DATA_RATE_38_4;
    *channel = on_base_param->channel;
    *priority = ONE_NET_HIGH_PRIORITY;
    *chunk_size = DEFAULT_BS_CHUNK_SIZE;
    *chunk_delay = DEFAULT_BS_CHUNK_DELAY;
    
    
    // first see if this is a long transfer.  If it is not, there's not much to
    // do.
    if(transfer_size > 2000)
    {
        // this is a long transfer.  We may or may not want to switch data
        // rates
        if(!(master->flags & ON_BS_ALLOWED))
        {
            // long transfers not allowed at all.
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            return *nr;
        }

        #ifdef _DATA_RATE_CHANNEL
        // See if we are to switch channels.
        if(master->flags & ON_BS_CHANGE_CHANNEL)
        {
            SInt8 alternate_channel = one_net_get_alternate_channel();
            if(alternate_channel >= 0)
            {
                *channel = (UInt8) alternate_channel;
            }
        }
        
        // See if we are to switch data rates
        if(master->flags & ON_BS_ELEVATE_DATA_RATE)
        {
            if(!dst_features_known)
            {
                *data_rate = features_highest_data_rate(THIS_DEVICE_FEATURES);
            }
            else
            {
                *data_rate = features_highest_matching_data_rate(
                  THIS_DEVICE_FEATURES, device->features);
            }
        }
        #endif
        
        if(!(master->flags & ON_BS_HIGH_PRIORITY))
        {
            *priority = ONE_NET_LOW_PRIORITY;
        }
    }

    *frag_delay = (*priority == ONE_NET_LOW_PRIORITY) ?
      on_base_param->fragment_delay_low : on_base_param->fragment_delay_high;

    *nr = one_net_client_get_default_block_transfer_values(dst, transfer_size,
      priority, chunk_size, frag_delay, chunk_delay, data_rate, channel,
      timeout, ack_nack);

    return *nr;
}


on_nack_rsn_t on_client_initiate_block_msg(block_stream_msg_t* msg,
  on_ack_nack_t* ack_nack)
{
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    ack_nack->handle = ON_ACK;
    *nr = ON_NACK_RSN_NO_ERROR;
    
    if(bs_msg.transfer_in_progress)
    {
        *nr = ON_NACK_RSN_BUSY;
    }
    else
    {
        one_net_memmove(&bs_msg, msg, sizeof(block_stream_msg_t));  
        
        if(!msg->dst)
        {
            *nr = ON_NACK_RSN_INTERNAL_ERR;
            return *nr;
        }
        
        if(!features_data_rate_capable(THIS_DEVICE_FEATURES,
          bs_msg.data_rate))
        {
            *nr = ON_NACK_RSN_INVALID_DATA_RATE;
            return *nr;
        }        
        
        if(features_known(bs_msg.dst->features))
        {
            if(!features_block_capable(bs_msg.dst->features))
            {
                *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            }
        
            if(!features_data_rate_capable(bs_msg.dst->features,
              bs_msg.data_rate))
            { 
                *nr = ON_NACK_RSN_INVALID_DATA_RATE;
            }
        }
        
        set_bs_transfer_type(&bs_msg.flags, ON_BLK_TRANSFER);
        bs_msg.src = NULL;
        bs_msg.bs_on_state = ON_LISTEN_FOR_DATA;
        ont_set_timer(ONT_BS_TIMER, 0);
    }
    
    if(*nr == ON_NACK_RSN_NO_ERROR)
    {
        bs_msg.transfer_in_progress = TRUE;
    }
    return *nr;
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
on_nack_rsn_t on_client_get_default_stream_transfer_values(
  const on_encoded_did_t* dst, UInt32 time_ms, UInt8* priority,
  UInt16* frag_delay, UInt8* data_rate, UInt8* channel, UInt16* timeout,
  on_ack_nack_t* ack_nack)
{
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    on_sending_device_t* device = &master->device;
    BOOL dst_features_known;
    
    if(!is_master_did(dst))
    {
        device = sender_info(dst);
    }
    
    *timeout = DEFAULT_BLOCK_STREAM_TIMEOUT;
    
    dst_features_known = device && features_known(device->features);
    
    if(dst_features_known && !features_stream_capable(
      device->features))
    {
        return ON_NACK_RSN_DEVICE_FUNCTION_ERR;
    }

    *nr = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;
    // Set to the current parameters first.
    *priority = ONE_NET_HIGH_PRIORITY;
    *data_rate = ONE_NET_DATA_RATE_38_4;
    *channel = on_base_param->channel;
    
    // first see if this is a long transfer.  If it is not, there's nothing to
    // do.
    if(time_ms == 0 || time_ms > 2000)
    {
        // this is a long transfer.  We may or may not want to switch data
        // rates
        if(!(master->flags & ON_BS_ALLOWED))
        {
            // long transfers not allowed at all.
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            return *nr;
        }
        
        #ifdef _DATA_RATE_CHANNEL
        // See if we are to switch channels.
        if(master->flags & ON_BS_CHANGE_CHANNEL)
        {
            SInt8 alternate_channel = one_net_get_alternate_channel();
            if(alternate_channel >= 0)
            {
                *channel = (UInt8) alternate_channel;
            }
        }
        
        // See if we are to switch data rates
        if(master->flags & ON_BS_ELEVATE_DATA_RATE)
        {
            if(!dst_features_known)
            {
                *data_rate = features_highest_data_rate(THIS_DEVICE_FEATURES);
            }
            else
            {
                *data_rate = features_highest_matching_data_rate(
                  THIS_DEVICE_FEATURES, device->features);
            }
        }
        #endif
    }
    
    // Assign the frag delay.  Default for stream is always high-priority
    // fragment delay regardless of priority
    *frag_delay = on_base_param->fragment_delay_high;
    
    *nr = one_net_client_get_default_stream_transfer_values(dst, time_ms,
      priority, frag_delay, data_rate, channel, timeout, ack_nack);
    return *nr;
}


on_nack_rsn_t on_client_initiate_stream_msg(block_stream_msg_t* msg,
  on_ack_nack_t* ack_nack)
{
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    ack_nack->handle = ON_ACK;
    *nr = ON_NACK_RSN_NO_ERROR;
    
    if(bs_msg.transfer_in_progress)
    {
        *nr = ON_NACK_RSN_BUSY;
    }
    else
    {
        one_net_memmove(&bs_msg, msg, sizeof(block_stream_msg_t));  
        
        if(!msg->dst)
        {
            *nr = ON_NACK_RSN_INTERNAL_ERR;
            return *nr;
        }
        
        if(!features_data_rate_capable(THIS_DEVICE_FEATURES,
          bs_msg.data_rate))
        {
            *nr = ON_NACK_RSN_INVALID_DATA_RATE;
            return *nr;
        }        
        
        if(features_known(bs_msg.dst->features))
        {
            if(!features_stream_capable(bs_msg.dst->features))
            {
                *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            }
        
            if(!features_data_rate_capable(bs_msg.dst->features,
              bs_msg.data_rate))
            { 
                *nr = ON_NACK_RSN_INVALID_DATA_RATE;
            }
        }
        
        set_bs_transfer_type(&bs_msg.flags, ON_STREAM_TRANSFER);
        bs_msg.src = NULL;
        bs_msg.bs_on_state = ON_LISTEN_FOR_DATA;
        ont_set_timer(ONT_BS_TIMER, 0);
    }
    
    if(*nr == ON_NACK_RSN_NO_ERROR)
    {
        bs_msg.transfer_in_progress = TRUE;
    }
    return *nr;
}
#endif



//! @} ONE-NET_CLIENT_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{



/*!
    \brief Handles a single data packet

    \param[in/out] txn The single transaction being carried out
    \param[in] pkt The packet structure.
    \param[in] raw_pld The raw payload bytes in the message
    \param[in] msg_type The type of the datapacket (i.e. admin packet or application packet)
    \param[out] ack_nack The response that should be sent to the sending device
    
    \return 
            ON_MSG_ABORT If the message is to be discarded and the transaction aborted
            ON_MSG_CONTINUE if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/ 
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

    on_decode(raw_src_did, &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
      ON_ENCODED_DID_LEN);
    on_decode(raw_repeater_did, &(pkt->packet_bytes[ON_ENCODED_RPTR_DID_IDX]),
      ON_ENCODED_DID_LEN);
    
    msg_hdr.msg_type = *msg_type;
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    
    // we'll be sending it back to the souerce.
    if(!(device = sender_info(
      (on_encoded_did_t*)&(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]))))
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
            msg_status = handle_admin_pkt(
              (on_encoded_did_t*)&(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
              &raw_pld[ON_PLD_DATA_IDX], *txn, ack_nack);
            break;
        #ifdef _ROUTE
        case ON_ROUTE_MSG:
        {
            on_raw_did_t my_raw_did;
            msg_status = ON_MSG_CONTINUE;
            ack_nack->handle = ON_ACK_ROUTE;
            ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
            one_net_memmove(ack_nack->payload->ack_payload,
              &raw_pld[ON_PLD_DATA_IDX],
              ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN);
              
            if(on_decode(my_raw_did, &on_base_param->sid[ON_ENCODED_NID_LEN],
              ON_ENCODED_DID_LEN) != ONS_SUCCESS)
            {
                ack_nack->nack_reason = ON_NACK_RSN_INTERNAL_ERR;
            }
            else if(append_raw_did_to_route(ack_nack->payload->ack_payload,
              (const on_raw_did_t* const) my_raw_did) == -1)
            {
                ack_nack->nack_reason = ON_NACK_RSN_RSRC_UNAVAIL_ERR;
            }
            break;
        }
        #endif
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
            ONE_NET_HIGH_PRIORITY, NULL, (const on_encoded_did_t* const)
            &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX])
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
    

// normally we try not to use goto statements but this is embedded programming
// and it may save us a few bytes?
ocsdh_build_resp:
    stay_awake = device_should_stay_awake((const on_encoded_did_t* const)
      &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]));

    response_pid = get_single_response_pid(pkt->raw_pid,
      ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR, stay_awake);

    if(!setup_pkt_ptr(response_pid, response_txn.pkt, pkt->msg_id,
      &response_pkt_ptrs))
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
    
    if(on_complete_pkt_build(&response_pkt_ptrs, response_pid) != ONS_SUCCESS)
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    return ON_MSG_RESPOND;
}


/*!
    \brief Handles a response from the recipient of a single transaction to the originator(this device)
    
    \param[in/out] txn The transaction.
    \param[in] pkt The packet structure.
    \param[in] raw_pld The raw payload bytes that are being responded to.
    \param[in] msg_type The type of the message in process (i.e. admin message or applicaiton message)
    \param[in/out] ack_nack The ack or nack atttached to the response.
           
    \return ON_MSG_TIMEOUT if the message should time out.
            ON_MSG_CONTINUE if the message should continue.
            See on_message_status_t structure for more options.
*/   
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    on_raw_did_t src_did;
    on_message_status_t status = ON_MSG_DEFAULT_BHVR;
    on_msg_hdr_t msg_hdr;
    
    if(!ack_nack || !txn)
    {
        // not sure how we got here, but we can't do anything
        return status;
    }
    
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;

    on_decode(src_did, &(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]),
      ON_ENCODED_DID_LEN);
   
    #ifndef _ONE_NET_MULTI_HOP
    status = one_net_client_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &(txn->retry));
    #else
    status = one_net_client_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &(txn->retry), pkt->hops, &(pkt->max_hops));
    #endif
    

    if(status == ON_MSG_DEFAULT_BHVR || status == ON_MSG_CONTINUE)
    {
        SInt16 new_response_timeout = (SInt16)txn->response_timeout;

        switch(ack_nack->handle)
        {
            case ON_ACK_SLOW_DOWN_TIME_MS:
                new_response_timeout += (SInt16)ack_nack->payload->ack_time_ms;
                break;
            case ON_ACK_SPEED_UP_TIME_MS:
                new_response_timeout -= (SInt16)ack_nack->payload->ack_time_ms;
                break;
            case ON_ACK_RESPONSE_TIME_MS:
                new_response_timeout = (SInt16)ack_nack->payload->ack_time_ms;
                break;
            #ifdef _COMPILE_WO_WARNINGS
            // add default case that does nothing for clean compile
            default:
                break;
            #endif
        }
        
        if(new_response_timeout > 0)
        {
            txn->response_timeout = (UInt16)new_response_timeout;
        }
        
        if(ack_nack->nack_reason == ON_NACK_RSN_BAD_KEY)
        {
            // The other side thinks we have a bad key and have given us one to
            // use.  We need to find out whether that is true.  If it is from
            // the master and we are in fact using an old key, we will change
            // keys and report to the master.  If it's from a client, we won't
            // change keys.  The client may be right, but we'll check in with
            // the master to make sure before changing.  Regardless, we want to
            // abort this message and stick it back in the queue to re-send
            // after any key changes.
            BOOL to_master = is_master_did((const on_encoded_did_t*)
              txn->device->did);
              
            #ifndef _COMPILE_WO_WARNINGS  
            BOOL requeue_msg = FALSE; // TODO -- figure out whether this is
                                      // something that should / can be re-sent.
                                      // otherwise, delete this unused variable
            #endif
            
            BOOL new_key = new_key_fragment(
              (const one_net_xtea_key_fragment_t*) ack_nack->payload->key_frag,
              to_master);
              
            if(new_key)
            {
                ont_set_timer(ONT_KEEP_ALIVE_TIMER, 0); // check-in with master
            }
            
            // TODO -- requeue the message?
        
            #ifdef _DEVICE_SLEEPS
            // We're in the middle of something, so stay awake.
            ont_set_timer(ONT_STAY_AWAKE_TIMER, MS_TO_TICK(DEVICE_SLEEP_STAY_AWAKE_TIME));
            #endif
            
            txn = 0;  // cancel the current transaction.
            return ON_MSG_ABORT;
        }
        
        
        if(txn->retry >= ON_MAX_RETRY)
        {
            #ifdef _ONE_NET_MULTI_HOP
            // we may be able to re-send with a higher max hops if there are
            // any repeaters available.  If we aren't part of the network yet,
            // we'll do a multi-hop.  If we are a device that sleeps, we may
            // not have an accurate count of the number of repeaters, so we'll
            // try multi-hop.
            
            // TODO -- there really should be a better way to determine
            // the number of repeaters for a device that sleeps.
            
            BOOL try_mh = features_device_sleeps(THIS_DEVICE_FEATURES) ||
              !client_joined_network;
              
            if(!try_mh)
            {
                SInt8 num_repeat = (SInt8) on_base_param->num_mh_repeaters;
                if(features_known(txn->device->features) &&
                  features_mh_repeat_capable(txn->device->features) &&
                  !is_master_did((const on_encoded_did_t*)(txn->device->did)))
                {
                    num_repeat--; // don't count the destination as a repeater.
                }
                
                // do not count ourself either if we are one of the repeaters.
                if(features_mh_repeat_capable(THIS_DEVICE_FEATURES))
                {
                    num_repeat--;
                }
                
                try_mh = (num_repeat > 0); // give it a try if any are available
            }
            
            if(try_mh && txn->max_hops < txn->device->max_hops)
            {
                on_raw_did_t raw_did;
                on_decode(raw_did,
                  &(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]),
                  ON_ENCODED_DID_LEN);
                
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
                    #ifdef _COMPILE_WO_WARNINGS
                    // add default case that does nothing for clean compile
                    default:
                        break;
                    #endif
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
  

/*!
    \brief Handles the end of a single transaction

    \param[in/out] txn The transaction that has just completed
    \param[in] pkt The packet containing the packet bytes of the message that has just completed
    \param[in] raw_pld The raw payload of the message that has just completed
    \param[in] msg_type The type of the message that has just completed (i.e. admin message, application message)
    \param[in] status The status of the message that just completed
    \param[out] ack_nack The response attached to the message
    
    \return ON_MSG_FAIL If the message failed
            ON_MSG_SUCCESS If the message succeeded
*/
static on_message_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    on_msg_hdr_t msg_hdr;
    on_raw_did_t dst;
    
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;
    on_decode(dst , &(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]),
      ON_ENCODED_DID_LEN);
   
    #ifdef _BLOCK_MESSAGES_ENABLED
    if(*msg_type == ON_ADMIN_MSG && raw_pld[0] == ON_REQUEST_BLOCK_STREAM)
    {
        // prevent the device from "sliding off" the device list
        on_client_set_device_slideoff((on_encoded_did_t*)
          &pkt->packet_bytes[ON_ENCODED_DST_DID_IDX],
          ON_DEVICE_PROHIBIT_SLIDEOFF);
    }
    #endif
    
    if(status == ON_MSG_SUCCESS && *msg_type == ON_ADMIN_MSG)
    {
        switch(raw_pld[0])
        {
            case ON_FEATURES_RESP:
            {
                break;
            }            
            
            case ON_ADD_DEV_RESP:
            case ON_REMOVE_DEV_RESP:
            case ON_CHANGE_SETTINGS_RESP:
            #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_REQUEST_KEY_CHANGE:
            #endif
            #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_CHANGE_FRAGMENT_DELAY_RESP:
            #endif
            {
                // success.  We'll check in immediately again.
                master->keep_alive_interval = 1;
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
                
                
                // Jan. 8, 2012 -- changing it to an immediate send.  More
                // experimentation is needed, but the random pause now seems
                // more trouble than it's worth.  The comments above may be
                // largely obsolete, but keeping them in anyway for now.
                master->keep_alive_interval = 1;
                               
                if(ack_nack->handle == ON_ACK_ADMIN_MSG)
                {
                    BOOL send_confirm_admin_msg = FALSE;
                    UInt8 admin_msg_type;
                    
                    switch(ack_nack->payload->admin_msg[0])
                    {
                        case ON_NEW_KEY_FRAGMENT:
                        {
                            // we MAY be using the wrong key.  The master is
                            // giving us the right key to use.  If we are not
                            // already using it, copy it into the key and
                            // shift.
                            new_key_fragment(
                              (const one_net_xtea_key_fragment_t*)
                              &(ack_nack->payload->admin_msg[1]), TRUE);
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
                            
                            this_device_added = is_my_did(
                              (const on_encoded_did_t*)added_device);
                            
                            if(!this_device_added)
                            {
                                one_net_client_client_added(
                                  (const on_raw_did_t* const) &raw_did_added);
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
                            on_raw_did_t raw_did_removed;
                            if(on_decode(raw_did_removed,
                              &(ack_nack->payload->admin_msg)[1],
                              ON_ENCODED_DID_LEN) != ONS_SUCCESS)
                            {
                                break;
                            }
                            one_net_client_client_removed(
                              (const on_raw_did_t* const) &raw_did_removed,
                              is_my_did((const on_encoded_did_t*)
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
                        
                        #ifdef _BLOCK_MESSAGES_ENABLED
                        case ON_CHANGE_FRAGMENT_DELAY:
                        {
                            // changing both within one message.  If a value is 0, then, it
                            // is irrelevant.
                            UInt16 new_frag_low = one_net_byte_stream_to_int16(
                              &ack_nack->payload->admin_msg[1 + ON_FRAG_LOW_IDX]);
                            UInt16 new_frag_high = one_net_byte_stream_to_int16(
                              &ack_nack->payload->admin_msg[1 + ON_FRAG_HIGH_IDX]);
              
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
                            
                            send_confirm_admin_msg = TRUE;
                            admin_msg_type = ON_CHANGE_FRAGMENT_DELAY_RESP;                            
                            break;
                        } // update fragment delay(s) case //
                        #endif                        
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
                    }
                    
                    break;
                }
                
                // No admin messages within the keep-alive response from
                // the master.  We were sent a new interval.
                master->keep_alive_interval = ack_nack->payload->ack_time_ms;
                #ifdef _AUTO_SAVE
                save = TRUE;
                #endif
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
    
    #if defined(_BLOCK_MESSAGES_ENABLED) && defined(_ONE_NET_MULTI_HOP)
    if(bs_msg.transfer_in_progress)
    {
        ack_nack->handle = ON_ACK;
        if(*msg_type == ON_ROUTE_MSG)
        {
            UInt8 hops, return_hops;
            if(ack_nack->nack_reason ||
              !extract_repeaters_and_hops_from_route(&(bs_msg.dst->did),
              ack_nack->payload->ack_payload, &hops,
              &return_hops, &bs_msg.num_repeaters, bs_msg.repeaters))
            {
                on_message_status_t status = ON_MSG_FAIL;
                
                // route failed for some reason.
                ack_nack->nack_reason = ON_NACK_RSN_ROUTE_ERROR;
                #ifndef _STREAM_MESSAGES_ENABLED
                on_client_block_txn_hdlr(&bs_msg, NULL, &status, ack_nack);
                #else
                if(get_bs_transfer_type(bs_msg.flags) == ON_BLK_TRANSFER)
                {
                    on_client_block_txn_hdlr(&bs_msg, NULL, &status,
                      ack_nack);
                }
                else
                {
                    on_client_stream_txn_hdlr(&bs_msg, NULL, &status,
                      ack_nack);
                }
                #endif
            }
            else
            {
                set_bs_hops(&bs_msg.flags, hops > return_hops ? hops :
                  return_hops);
            }
        }
    }
    #endif    

    if(is_master_did((on_encoded_did_t*)
      &(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]))
      && ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
    {
        // We have checked in.  Reset the Keep-Alive Timer if we are sending
        // regular updates
        if(master->keep_alive_interval > 0)
        {
            ont_set_timer(ONT_KEEP_ALIVE_TIMER,
              MS_TO_TICK(master->keep_alive_interval));
        }
    }
    
    return ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR ? ON_MSG_SUCCESS :
      ON_MSG_FAIL;
}




#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Handles a block data packet

    \param[in/out] txn The block transaction being carried out
    \param[in/out] bs_msg The stream message in progress
    \param[in] block_pkt The data packet, including both the payload and some administration information. 
    \param[out] ack_nack The response that should be sent to the sending device
    
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/ 
static on_message_status_t on_client_block_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* block_pkt, on_ack_nack_t* ack_nack)
{
    return one_net_client_handle_block_pkt(txn, bs_msg, (block_pkt_t*)
      block_pkt, ack_nack);
}


/*!
    \brief Handles a response from the recipient of a block transfer to the originator(this device) of a block transfer
    
    \param[in/out] txn The transaction.
    \param[in/out] bs_msg The block message being responded to.
    \param[in] pkt The packet structure.
    \param[in] raw_payload_bytes The raw payload bytes that are being responded to.
    \param[in/out] ack_nack The ack or nack atttached to the response.
           
    \return ON_MSG_IGNORE to ignore the response.
            ON_MSG_TERMINATE to terminate the transaction
            ON_MSG_ACCEPT_PACKET If packet is good.
*/  
static on_message_status_t on_client_handle_block_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return one_net_client_handle_bs_ack_nack_response(txn, bs_msg, pkt,
      raw_payload_bytes, ack_nack);
}
  

/*!
    \brief Handles the end of a block transaction.
    
    \param[in] msg The block message that is being terminated.
    \param[in] terminating_device The device that terminated this transaction.
    \param[in/out] status The status of the message that was just completed.
    \param[in/out] ack_nack Any ACK or NACK associated with this termination.
    
    \return ON_MSG_RESPOND if this device should inform the other devices
              of the termination.
            All other return types abort immediately with no further messages.
*/
static on_message_status_t on_client_block_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack)
{
    // allow devices to "slide off" again.
    if(msg->dst)
    {
        on_client_set_device_slideoff(&msg->dst->did,
          ON_DEVICE_ALLOW_SLIDEOFF);
        #ifdef _ONE_NET_MULTI_HOP
        {
            UInt8 i;
            for(i = 0; i < msg->num_repeaters; i++)
            {
                on_client_set_device_slideoff(&msg->repeaters[i],
                  ON_DEVICE_ALLOW_SLIDEOFF);
            }
        }
        #endif
    }
    if(msg->src)
    {
        on_client_set_device_slideoff(&msg->src->did,
          ON_DEVICE_ALLOW_SLIDEOFF);
    }
    
    return one_net_client_block_txn_status(msg, terminating_device, status,
      ack_nack);
}
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
static on_message_status_t on_client_stream_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* stream_pkt, on_ack_nack_t* ack_nack)
{
    return one_net_client_handle_stream_pkt(txn, bs_msg, (stream_pkt_t*)
      stream_pkt, ack_nack);
}


/*!
    \brief Handles a response from the recipient of a stream transfer to the originator(this device) of a stream transfer

    \param[in/out] txn The stream transaction being carried out
    \param[in/out] bs_msg The stream message in progress
    \param[in] pkt The packet
    \param[in] raw_payload_bytes The raw payload bytes of the stream paylod that is being responded to.
    \param[in/out] ack_nack The ack or nack containied in the response

    \return ON_MSG_IGNORE to ignore the response.
            ON_MSG_TERMINATE to terminate the transaction
            ON_MSG_ACCEPT_PACKET If packet is good.
            If packet is rejected, the ack_nack reason and / or payload should be filled in.
*/ 
static on_message_status_t on_client_handle_stream_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return one_net_client_handle_bs_ack_nack_response(txn, bs_msg, pkt,
      raw_payload_bytes, ack_nack);
}
  

/*!
    \brief Handles the end of a stream transaction.

    \param[in] msg The stream message that has ended.
    \param[in] terminating_device The device that terminated the stream transfer(if NULL, then the terminating device is this device)
    \param[in/out] status The termination status of the stream message
    \param[in/out] ack_nack The ack or nack associated with the termination, if any

    \return ON_MSG_RESPOND if this device should inform the other devices
              of the termination.
            All other return types abort immediately with no further messages.
*/
static on_message_status_t on_client_stream_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack)
{
    return one_net_client_block_txn_status(msg, terminating_device, status,
      ack_nack);
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
    #ifndef _ONE_NET_SIMPLE_CLIENT
    pkt_hdlr.adj_recip_list_hdlr = &on_client_adjust_recipient_list;
    #endif
    
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

    get_sender_info = &sender_info;
    device_is_master = FALSE;
    one_net_init();
    return ONS_SUCCESS;
} // init_internal //


#ifndef _ONE_NET_SIMPLE_CLIENT
static on_sending_dev_list_item_t* get_sending_dev_list_item_t(
  const on_encoded_did_t* DID)
{
    UInt8 i;
    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)
          &(sending_dev_list[i].sender.did)))
        {
            return &sending_dev_list[i];
        }
    }
    return NULL;
}


/*!
    \brief Finds the sender info (or a location for the sender info).

    Loops through and finds the information for the sending device.  If the
    device has not heard from the sender before, a new location shall be
    returned.

    \param[in] DID The device id of the sender.

    \return Pointer to location that holds the sender information (should be
      checked for 0, and should be checked if a new location).
*/
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID)
{
    SInt8 i;
    SInt8 vacant_index = -1;
    SInt8 replace_index = -1;
    SInt8 device_index = -1;
    UInt8 device_lru = ONE_NET_RX_FROM_DEVICE_COUNT;

    if(!DID)
    {
        return 0;
    } // if parameter is invalid //

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        return &master->device;
    } // if the MASTER is the sender //



    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(sending_dev_list[i].lru)
        {
            if(on_encoded_did_equal(DID,
              (const on_encoded_did_t * const)
              &(sending_dev_list[i].sender.did)))
            {
                device_index = i;
                device_lru = sending_dev_list[i].lru;
            }
            else
            {
                if(sending_dev_list[i].slideoff == ON_DEVICE_ALLOW_SLIDEOFF)
                {
                    if(replace_index == -1 || sending_dev_list[i].lru >
                      sending_dev_list[replace_index].lru)
                    {
                        replace_index = i;
                    }
                }
            }
        }
        else
        {
            vacant_index = i;
        }
    }
    
    if(device_index == -1)
    {
        if(vacant_index != -1)
        {
            replace_index = vacant_index;
        }
        
        device_index = replace_index;
        if(device_index == -1)
        {
            goto adjust_lru_list; // no room on list
        }
        
        one_net_memmove(sending_dev_list[device_index].sender.did, *DID,
          sizeof(sending_dev_list[device_index].sender.did));
        sending_dev_list[device_index].sender.features = FEATURES_UNKNOWN;
        sending_dev_list[device_index].sender.msg_id =
          one_net_prand(get_tick_count(), 50);
        sending_dev_list[device_index].slideoff = ON_DEVICE_ALLOW_SLIDEOFF;
        
        #ifdef _ONE_NET_MULTI_HOP
        sending_dev_list[device_index].sender.hops = 0;
        sending_dev_list[device_index].sender.max_hops = ON_MAX_HOPS_LIMIT;
        #endif
    }
    sending_dev_list[device_index].lru = 1;
    
adjust_lru_list:
    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(i == device_index || sending_dev_list[i].lru == 0)
        {
            continue;
        }
        if(sending_dev_list[i].lru < device_lru)
        {
            sending_dev_list[i].lru++;
        }
    }

    if(device_index == -1)
    {
        return NULL;
    }
    return &(sending_dev_list[device_index].sender);
} // sender_info //
#else
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID)
{
    SInt8 i;
    SInt8 vacant_index = -1;
    SInt8 device_index = -1;

    if(!DID)
    {
        return 0;
    } // if parameter is invalid //

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        return &master->device;
    } // if the MASTER is the sender //



    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)
          &(sending_dev_list[i].sender.did)))
        {
            device_index = i;
        }
        else if(sending_dev_list[i].sender.did[0] == 0 &&
          sending_dev_list[i].sender.did[1] == 0)
        {
            vacant_index = i;
        }
    }
    
    if(device_index == -1)
    {
        if(vacant_index != -1)
        {
            device_index = vacant_index;
        }
        else
        {
            device_index = one_net_prand(get_tick_count(),
              ONE_NET_RX_FROM_DEVICE_COUNT);
        }
        
        one_net_memmove(sending_dev_list[device_index].sender.did, *DID,
          sizeof(sending_dev_list[device_index].sender.did));
        sending_dev_list[device_index].sender.features = FEATURES_UNKNOWN;
        sending_dev_list[device_index].sender.msg_id =
          one_net_prand(get_tick_count(), 50);
    }

    return &(sending_dev_list[device_index].sender);
} // sender_info //
#endif



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

    if(one_net_client_send_single(ONE_NET_RAW_SINGLE_DATA,
      ON_ADMIN_MSG, raw_pld, 5, ONE_NET_LOW_PRIORITY,
      NULL, (const on_encoded_did_t* const) MASTER_ENCODED_DID
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
      ))
    {
        return ONS_SUCCESS;
    }
    else
    {
        return ONS_RSRC_FULL;
    }
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
    
    #if defined(_BLOCK_MESSAGES_ENABLED) || defined(_ONE_NET_MH_CLIENT_REPEATER)
    if(on_rx_packet(&invite_txn, &this_txn, &this_pkt_ptrs, raw_payload_bytes)
      != ONS_PKT_RCVD)
    #else
    if(on_rx_packet(&this_txn, &this_pkt_ptrs, raw_payload_bytes)
      != ONS_PKT_RCVD)
    #endif
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
    one_net_memmove(on_base_param->sid,
      &(this_pkt_ptrs->packet_bytes[ON_ENCODED_NID_IDX]), ON_ENCODED_NID_LEN);
    on_encode(&(on_base_param->sid[ON_ENCODED_NID_LEN]),
      &raw_payload_bytes[ON_INVITE_ASSIGNED_DID_IDX], ON_ENCODED_DID_LEN);
    one_net_memmove(on_base_param->current_key,
      &raw_payload_bytes[ON_INVITE_KEY_IDX], ONE_NET_XTEA_KEY_LEN);
    master->device.features =  
      *((on_features_t*)(&raw_payload_bytes[ON_INVITE_FEATURES_IDX]));
    
    // Set up a random message id.  We have no idea what the master id is.
    reset_msg_ids();
    this_pkt_ptrs->msg_id = master->device.msg_id;
    
    #ifdef _ONE_NET_MULTI_HOP
    master->device.max_hops = features_max_hops(master->device.features);
    #endif
    #ifdef _AUTO_SAVE
    save = TRUE;
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
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;

    switch(DATA[0])
    {
        #ifdef _DATA_RATE_CHANNEL
        case ON_CHANGE_DATA_RATE_CHANNEL:
        {
            UInt16 pause_time_ms = one_net_byte_stream_to_int16(&DATA[3]);
            UInt16 dormant_time_ms = one_net_byte_stream_to_int16(&DATA[5]);
            ack_nack->nack_reason = on_change_dr_channel(NULL,
              pause_time_ms, dormant_time_ms, DATA[1], DATA[2]);
            break;
        }
        #endif
        
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_TERMINATE_BLOCK_STREAM:
        {
            on_ack_nack_t* received_ack_nack = (on_ack_nack_t*)
              &DATA[ON_ENCODED_DID_LEN+2];
            received_ack_nack->payload = (ack_nack_payload_t*)
              &DATA[ON_ENCODED_DID_LEN+4];
            terminate_bs_msg(&bs_msg, (const on_encoded_did_t*) &DATA[1],
              DATA[ON_ENCODED_DID_LEN+1], received_ack_nack);
            break;
        }
        
        case ON_REQUEST_BLOCK_STREAM:
        {
            #ifdef _STREAM_MESSAGES_ENABLED
            UInt8 is_block_transfer = (get_bs_transfer_type(
              DATA[BLOCK_STREAM_SETUP_FLAGS_IDX]) == ON_BLK_TRANSFER);
            #endif
            
            // check if we are already in the middle of a block transaction.  If
            // we are and the source is NOT the sending device of this message,
            // NACK it.  If the source IS the sending device of this message and
            // we're still in the setup stage, we're OK.  Otherwise, NACK it.            
            if(bs_msg.transfer_in_progress)
            {
                if(!bs_msg.src || !on_encoded_did_equal(
                  (const on_encoded_did_t* const)&(bs_msg.src->did), SRC_DID))
                {
                    ack_nack->nack_reason = ON_NACK_RSN_BUSY;
                    break;
                }
                
                #ifdef _STREAM_MESSAGES_ENABLED
                if(!is_block_transfer)
                {
                    if(bs_msg.bs.stream.last_response_time > 0)
                    {
                        // we have already started receiving data
                        ack_nack->nack_reason = ON_NACK_RSN_ALREADY_IN_PROGRESS;
                        break;                    
                    }
                }
                else
                #endif
                {
                    if(bs_msg.bs.block.byte_idx > 0)
                    {
                        // we have already started receiving data
                        ack_nack->nack_reason = ON_NACK_RSN_ALREADY_IN_PROGRESS;
                        break;                    
                    }
                }
            }
            
            bs_msg.src = sender_info(SRC_DID);
            if(!bs_msg.src)
            {
                ack_nack->nack_reason = ON_NACK_RSN_INTERNAL_ERR;
                break;
            }
            
            admin_msg_to_block_stream_msg_t(&DATA[0], &bs_msg, SRC_DID);
            
            if(bs_msg.dst)
            {
                // we are being requested as a repeater
                #ifndef _ONE_NET_MH_CLIENT_REPEATER
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
                #else
                // We are capable.  check application code
                one_net_client_repeater_requested(&bs_msg, ack_nack);
                #endif
            }
            
            if(!ack_nack->nack_reason)
            {
                one_net_block_stream_transfer_requested(&bs_msg, ack_nack);
            }
            
            if(!ack_nack->nack_reason)
            {
                #ifdef _STREAM_MESSAGES_ENABLED
                if(!is_block_transfer)
                {
                    bs_msg.bs.stream.last_response_time = 0;
                }
                else
                #endif
                {
                    bs_msg.bs.block.byte_idx = 0;
                }
                
                bs_msg.transfer_in_progress = TRUE;
                bs_msg.bs_on_state = ON_LISTEN_FOR_DATA;
                
                // Set the block / stream timer to the timeout
                ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(bs_msg.timeout));
                
                // prevent device list slideoff
                if(bs_msg.dst)
                {
                    on_client_set_device_slideoff(&bs_msg.dst->did,
                      ON_DEVICE_PROHIBIT_SLIDEOFF);
                }
                if(bs_msg.src)
                {
                    on_client_set_device_slideoff(&bs_msg.src->did,
                      ON_DEVICE_PROHIBIT_SLIDEOFF);
                }
            }
            break;
        }
        #endif
                    
        case ON_NEW_KEY_FRAGMENT:
        {
            // there has been a key change.  We may already have the new key
            // and we may not.  Check here.  If our last key fragment matches
            // what is in the message, we already have the key, so don't
            // replace anything.  If not, replace the last fragment with what
            // we received.
            
            if(new_key_fragment(
              (const one_net_xtea_key_fragment_t* const)(&DATA[1]), TRUE))
            {
                // We have changed a key.  We don't know why.  Some device,
                // possibly us, may have run out of message ids and there was a
                // key change to reset that.  Regardless of who wanted the key
                // change and why, we'll reset all of our message IDs.  We'll
                // assume someone is out of sync, so we'll sync back up later.
                reset_msg_ids();
                master->device.msg_id = one_net_prand(get_tick_count(), 50);
                  
                #ifdef _AUTO_SAVE
                save = TRUE;
                #endif
            }
            
            
            // we now have the right key.  Send it back in the ACK.  We'll
            // encrypt using the NEW key.
            ack_nack->handle = ON_ACK_KEY_FRAGMENT;
            one_net_memmove(ack_nack->payload->key_frag, &DATA[1],
              ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
            txn->key = &(on_base_param->current_key);
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
                case ONS_SUCCESS:
                    #ifdef _AUTO_SAVE
                    save = TRUE;
                    #endif
                    break;
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
                case ONS_SUCCESS:
                    #ifdef _AUTO_SAVE
                    save = TRUE;
                    #endif
                    break;
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
            #ifdef _AUTO_SAVE
            save = TRUE;
            #endif
            break;
        } // update fragment delay(s) case //
        #endif

        case ON_CHANGE_KEEP_ALIVE:
        {
            master->keep_alive_interval = one_net_byte_stream_to_int32(
              &DATA[1]);
            ont_set_timer(ONT_KEEP_ALIVE_TIMER, master->keep_alive_interval);
            #ifdef _AUTO_SAVE
            save = TRUE;
            #endif
        } // change keep-alive case : intentional fall-through //
        case ON_KEEP_ALIVE_QUERY:
        {
            ack_nack->handle = ON_ACK_TIME_MS;
            ack_nack->payload->ack_time_ms = master->keep_alive_interval;
            break;
        } // keep alive query case //
        
        case ON_CHANGE_SETTINGS:
        {
            master->flags = DATA[1];
            ack_nack->handle = ON_ACK_VALUE;
            ack_nack->payload->ack_value.uint8 = master->flags;
            #ifdef _AUTO_SAVE
            save = TRUE;
            #endif
            break;
        } // update settings case //
        
        case ON_RM_DEV:
        {
            // first check if we are the one being removed.
            on_encoded_did_t* removed_did = (on_encoded_did_t*) &DATA[1];

            if(is_my_did((const on_encoded_did_t*) removed_did))
            {
                // we're the ones being removed
                removed = TRUE;
                
                // we'll stay alive for another 3 seconds to complete any
                // pending transactions so other devices won't miss any ACKs
                // or NACKs, then delete ourselves.  Just use the invite
                // timer.
                
                // TODO -- why the invite timer?
                ont_set_timer(ONT_INVITE_TIMER,
                  MS_TO_TICK(3000));
            }
            // TODO -- #define guard here?  Do clients that sleep, don't
            // have multi-hop, and don't have block messages enabled get this
            // message?
            else
            {
                // Remove the device from the list of sending devices
                
                #ifndef _ONE_NET_SIMPLE_CLIENT
                on_sending_dev_list_item_t* removed_item =
                  get_sending_dev_list_item_t(removed_did);
                if(removed_item)
                {
                    removed_item->lru = 0;
                }
                #endif

                #ifdef _PEER
                // delete any peer assignments for this device
                one_net_remove_peer_from_list(ONE_NET_DEV_UNIT, NULL,
                  (const on_encoded_did_t * const) removed_did,
                  ONE_NET_DEV_UNIT);
                #endif
                {
                    on_raw_did_t raw_did;
                    on_decode(raw_did, *removed_did, ON_ENCODED_DID_LEN);
                    one_net_client_client_removed((const on_raw_did_t* const)
                      &raw_did, FALSE);
                }
                
                #ifdef _ONE_NET_MULTI_HOP
                on_base_param->num_mh_devices =
                  ack_nack->payload->admin_msg[3];
                on_base_param->num_mh_repeaters =
                  ack_nack->payload->admin_msg[4];
                #endif
                #ifdef _AUTO_SAVE
                save = TRUE;
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
            this_device_added = is_my_did((const on_encoded_did_t*)
              added_device);
            if(this_device_added && !client_joined_network)
            {
                one_net_client_invite_result((const on_raw_did_t* const)
                  &raw_did, ONS_SUCCESS);
                client_joined_network = TRUE;
                master->flags |= ON_JOINED; // TODO -- seems like this should have
                                    // been set elsewhere?
                client_looking_for_invite = FALSE;
            }
            else
            {
                one_net_client_client_added((const on_raw_did_t* const)
                  &raw_did);
            }
            
            #ifdef _ONE_NET_MULTI_HOP
            on_base_param->num_mh_devices =
              ack_nack->payload->admin_msg[3];
            on_base_param->num_mh_repeaters =
              ack_nack->payload->admin_msg[4];
            #endif            
            #ifdef _AUTO_SAVE
            save = TRUE;
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


#ifndef _ONE_NET_SIMPLE_CLIENT
// TODO -- not sure if it is OK to comment this out for Simple Clients.  If
// two simple clients are communicating, it could be a problem.  If either
// end is not a simple client, it won't be a problem.  Anyway, for now, we'll
// comment it out for simple clients.  We're right up against the 16K boundary.
/*!
    \brief Checks to see whether the device needs to send the master a request
      to change keys

    \return TRUE if a message was sent
            FALSE otherwise.
*/
static BOOL send_new_key_request(void)
{
    UInt8 admin_type = ON_REQUEST_KEY_CHANGE;

    if(!key_change_requested)
    {
        return FALSE;
    }

    if(one_net_client_send_single(ONE_NET_RAW_SINGLE_DATA,
      ON_ADMIN_MSG, &admin_type, 1, ONE_NET_LOW_PRIORITY,
      NULL, (const on_encoded_did_t* const) MASTER_ENCODED_DID
      #ifdef _PEER
      , FALSE,
      ONE_NET_DEV_UNIT
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
      , 0
      #endif
      ))
    {
        key_change_requested = FALSE;
        // to avoid duplicate requests, only request a key change at most every
        // 60 seconds.  This time will be checked elsewhere before setting the
        // key_change_requested flag.
        key_change_request_time = get_tick_count();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif


/*!
    \brief Checks to see whether the device needs to send a check-in message
      to the master and if so, sends it.

    \return TRUE if a message was sent
            FALSE otherwise.
*/
static BOOL check_in_with_master(void)
{
    UInt8 raw_pld[5];
    #ifndef _ONE_NET_SIMPLE_CLIENT
    if(send_new_key_request())
    {
        return TRUE;
    }
    #endif
    
    if(!ont_expired(ONT_KEEP_ALIVE_TIMER))
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


#ifndef _ONE_NET_SIMPLE_CLIENT
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
