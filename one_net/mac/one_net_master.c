//! \addtogroup ONE-NET_MASTER ONE-NET MASTER device functionality
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
    \file one_net_master.c
    \brief ONE-NET MASTER functionality implementation

    Derives from ONE-NET.  MASTER dependent functionality.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


#include "config_options.h"

#ifdef _ONE_NET_MASTER

#include "one_net_master.h"
#include "one_net_master_port_const.h"
#include "one_net_timer.h"
#include "tick.h"
#include "tal.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_master_port_specific.h"
#include "one_net_status_codes.h"
#include "one_net_prand.h"
#include "one_net_crc.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{


//! Number of bits to shift the initial CLIENT address to use as a 16-bit
//! or raw address value
#define ON_INIT_CLIENT_SHIFT RAW_DID_SHIFT


#ifdef _ONE_NET_MULTI_HOP
    #define ON_INVITES_BEFORE_MULTI_HOP 5
#endif


//! @} ONE-NET_MASTER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MASTER_typedefs
//! \ingroup ONE-NET_MASTER
//! @{

//! @} ONE-NET_MASTER_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MASTER_pub_var
//! \ingroup ONE-NET_MASTER
//! @{


//! MASTER specific parameters.  These need to be saved in non-volatile memory.
on_master_param_t * const master_param =
  (on_master_param_t * const)(&nv_param[0] + sizeof(on_base_param_t));
  
//! List of the CLIENTS
on_client_t * const client_list = (on_client_t * const)(&nv_param[0] +
  sizeof(on_base_param_t) + sizeof(on_master_param_t));



//! @} ONE-NET_MASTER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================



//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_MASTER_pri_var
//! \ingroup ONE-NET_MASTER
//! @{



/*!
    The length of time in ticks that the channel must be clear for before
    determining that the channel is ok for the network to operate on.
*/
static tick_t new_channel_clear_time_out = 0;


//! @} ONE-NET_MASTER_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
//! @{



// packet handlers
static on_message_status_t on_master_single_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type);
static on_message_status_t on_master_handle_single_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);

#ifdef _BLOCK_MESSAGES_ENABLED
static on_message_status_t on_master_block_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type);
static on_message_status_t on_master_handle_block_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_master_block_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);
#endif

#ifdef _STREAM_MESSAGES_ENABLED
static on_message_status_t on_master_stream_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type);
static on_message_status_t on_master_handle_stream_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_master_stream_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);
#endif

static one_net_status_t init_internal(void);
static on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID);
static one_net_status_t rm_client(const on_encoded_did_t * const CLIENT_DID);
static void sort_client_list_by_encoded_did(void);
static UInt16 find_lowest_vacant_did(void);


static one_net_status_t one_net_master_send_single(UInt8 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t* send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t* expire_time_from_now
  #endif
  );
  
  
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);



//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MASTER_pub_func
//! \ingroup ONE-NET_MASTER
//! @{

/*!
    \brief Starts a new ONE-NET network.

    This should be called the very first time the MASTER starts up.  It creates
    a new ONE-NET network.  Once the network has been created, call
    one_net_master_init to initialize the MASTER with the network created when
    this function is called.

    \param[in] SID The raw SID of the MASTER.
    \param[in] KEY The xtea key to use for single and block transactions.
    \param[in] SINGLE_BLOCK_ENCRYPT_METHOD The method to use to encrypt single
      and block packets when they are sent.
    \param[in] STREAM_KEY The xtea key to use for stream transactions.
    \param[in] STREAM_ENCRYPT_METHOD The method to use to encrypt stream packets
      when they are sent.

    \return ONS_SUCCESS if the network was created.
            ONS_BAD_PARAM if the parameter was invalid
*/

#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
  const one_net_xtea_key_t * const STREAM_KEY,
  const UInt8 STREAM_ENCRYPT_METHOD)
#else
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD)
#endif  
{
	UInt8 i;

#ifdef _STREAM_MESSAGES_ENABLED
    if(!SID || !KEY
      || SINGLE_BLOCK_ENCRYPT_METHOD != ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32
      || !STREAM_KEY || STREAM_ENCRYPT_METHOD != ONE_NET_STREAM_ENCRYPT_XTEA8)
#else
    if(!SID || !KEY
      || SINGLE_BLOCK_ENCRYPT_METHOD != ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32)
#endif
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    on_base_param->version = ON_PARAM_VERSION;
    on_encode(on_base_param->sid, *SID, sizeof(on_base_param->sid));

    on_base_param->channel = one_net_prand(get_tick_count(),
      ONE_NET_MAX_CHANNEL);
    // randomly pick any channel //
    
    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    one_net_memmove(on_base_param->current_key, *KEY,
      sizeof(on_base_param->current_key));
    on_base_param->single_block_encrypt = SINGLE_BLOCK_ENCRYPT_METHOD;
	
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_memmove(on_base_param->stream_key, *STREAM_KEY,
      sizeof(on_base_param->stream_key));
    on_base_param->stream_encrypt = STREAM_ENCRYPT_METHOD;
#endif
#ifdef _BLOCK_MESSAGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
#endif

    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID;
    master_param->client_count = 0;

    init_internal();
    new_channel_clear_time_out = ONE_NET_MASTER_NETWORK_CHANNEL_CLR_TIME;
    
    // see comment in the "case ON_JOIN_NETWORK" segment of the
    // one_net_master() loop to explain we are using these timers and what
    // we're doing.
    ont_set_timer(ONT_GENERAL_TIMER, MS_TO_TICK(new_channel_clear_time_out));
    ont_set_timer(ONT_CHANGE_KEY_TIMER,
      MS_TO_TICK(one_net_master_channel_scan_time));
    on_state = ON_JOIN_NETWORK;

    return ONS_SUCCESS;
} // one_net_master_create_network //


/*!
    \brief MASTER initializer

    This needs to be called before the MASTER is run.  Due to memory constraints
    of embedded systems, this function can be repeatedly called with only a
    subset of the parameters.  The calls must preserve the byte order of the
    parameters.

    \param[in] PARAM The parameters (or part) that were saved.  If NULL, then
                     the caller has already initialized the base memory.
    \param[in] PARAM_LEN The size in bytes of the parameters being loaded.

    \return ONS_SUCCESS If loading all of the parameters have completed
              successfully.
            ONS_MORE If not done initializing the parameters and this
              function needs to be called again.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_INVALID_DATA If the data passed in is not valid (including being
              too long).  Initialization is reset and the parameters must be
              passed in from the beginning.
*/
one_net_status_t one_net_master_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN)
{
    UInt8 i;
    one_net_status_t status;
    
    if(PARAM != NULL)
    {
        // code here to initalize things from PARAM and PARAM_LEN
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    // check for repeater
    for(i = 0; i < master_param->client_count; i++)
    {
        if(features_mh_repeat_capable(
          client_list[i].device_send_info.features))
        {
            mh_repeater_available = TRUE;
            break;
        } // if a mh repeater capable CLIENT was found //
    } // loop to look for any Multi-Hop repeaters //
    #endif
    
    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status;
    } // if initializing the internals failed //
    
    on_state = ON_LISTEN_FOR_DATA;
   
    return ONS_SUCCESS;
} // one_net_master_init //


/*!
    \brief Resets the master to a new channel and wipes out all clients

    \param[in] channel The new channel to set the network to
    
    \return void
*/
void one_net_reset_master_with_channel(UInt8 channel)
{
    one_net_master_cancel_invite(&invite_key);
    on_base_param->channel = channel;
    one_net_set_channel(channel);
    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID;
    master_param->client_count = 0;
    on_state = ON_LISTEN_FOR_DATA;
    one_net_init();
}


one_net_status_t one_net_master_change_key(
  const one_net_xtea_key_fragment_t KEY_FRAGMENT)
{
    return ONS_SUCCESS;
} // one_net_master_change_key //


#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_change_stream_key(
  const one_net_xtea_key_t * const NEW_STREAM_KEY)
{
    return ONS_SUCCESS;
} // one_net_master_change_stream_key //
#endif


/*!
    \brief Starts the network process to invite a CLIENT to join the network.

    \param[in] KEY The unique key of the device to invite to join the network.
    \param[in] timeout The length of time, in milliseconds, that the master
                   should attempt to invite the device before giving up.

    \return ONS_SUCCESS if the process was successfully started
            ONS_BAD_PARAM if the parameter is invalid
            ONS_NOT_INIT The network has not been fully created.
            ONS_DEVICE_LIMIT If the MASTER cannot handle adding another device.
            ONS_RSRC_FULL if there is not an available transaction to send the
              invite.
            See on_encrypt & on_build_pkt for more return codes
*/
one_net_status_t one_net_master_invite(const one_net_xtea_key_t * const KEY,
  UInt32 timeout)
{
    one_net_status_t status;
    UInt8 raw_invite[ON_RAW_INVITE_SIZE];

    if(!KEY || timeout == 0)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(on_state == ON_INIT_STATE || on_state == ON_JOIN_NETWORK)
    {
        return ONS_NOT_INIT;
    } // if the network has not been created yet //

    if(invite_txn.priority != ONE_NET_NO_PRIORITY)
    {
        return ONS_ALREADY_IN_PROGRESS;
    } // if already in the process of inviting //

    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return ONS_DEVICE_LIMIT;
    } // if the MASTER has reached it's device limit //

    one_net_memmove(invite_key, *KEY, sizeof(invite_key));
    raw_invite[ON_INVITE_VERSION_IDX] = ON_INVITE_PKT_VERSION;
    one_net_int16_to_byte_stream(master_param->next_client_did,
      &(raw_invite[ON_INVITE_ASSIGNED_DID_IDX]));
    one_net_memmove(&(raw_invite[ON_INVITE_KEY_IDX]),
      on_base_param->current_key, sizeof(on_base_param->current_key));
    one_net_memmove(&raw_invite[ON_INVITE_FEATURES_IDX],
      &THIS_DEVICE_FEATURES, sizeof(on_features_t));

    raw_invite[ON_INVITE_CRC_IDX] = (UInt8)one_net_compute_crc(
      &raw_invite[ON_INVITE_CRC_START_IDX],
      ON_INVITE_DATA_LEN, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);

    status = on_encrypt(ON_INVITE, raw_invite,
      (const one_net_xtea_key_t * const)(&invite_key), ON_RAW_INVITE_SIZE);


    if(status != ONS_SUCCESS)
    {
        one_net_master_cancel_invite(
          (const one_net_xtea_key_t * const)&invite_key);
        return status;
    } // if the invite was not created successfully //


    // so far, so good.  Start building the packet.
    if(!setup_pkt_ptr(ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT, invite_pkt,
      &data_pkt_ptrs))
    {
        return ONS_INTERNAL_ERR;
    }

    // pick a random message id
    data_pkt_ptrs.msg_id = one_net_prand(get_tick_count(), ON_MAX_MSG_ID);

    // fill in the addresses
    if((status = on_build_my_pkt_addresses(&data_pkt_ptrs,
      (on_encoded_did_t*) ON_ENCODED_BROADCAST_DID,
      (on_encoded_did_t*) MASTER_ENCODED_DID)) != ONS_SUCCESS)
    {
        return status;
    }
    
    // encode the payload
    if(status = on_encode(data_pkt_ptrs.payload, raw_invite,
      ON_ENCODED_INVITE_SIZE) != ONS_SUCCESS)
    {
        return status;
    }
    
    // now finish building the packet.
    if(status = on_complete_pkt_build(&data_pkt_ptrs,
      data_pkt_ptrs.msg_id, ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT)
      != ONS_SUCCESS)
    {
        return status;                          
    }

    // everything worked out fine.  Set the transactions and timers
    invite_txn.data_len = ON_ENCODED_INVITE_SIZE;
    invite_txn.response_timeout = one_net_master_invite_send_time;
    invite_txn.priority = ONE_NET_LOW_PRIORITY;
    ont_set_timer(invite_txn.next_txn_timer, 0);
    ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(timeout));

    return ONS_SUCCESS;
} // one_net_master_invite //


/*!
    \brief Cancels an invite request

    Even though there can only be 1 outstanding invite request at a time, pass
    in the KEY in case later on there can be multiple outstanding invite
    requests.  This will prevent the interface from changing later on if the
    implementation is changed.

    \param[in] KEY The unique key of the device to cancel the invite request for

    \return ONS_SUCCESS if the invite request was canceled.
*/
one_net_status_t one_net_master_cancel_invite(
  const one_net_xtea_key_t* const KEY)
{
    UInt8 i;
    invite_txn.priority = ONE_NET_NO_PRIORITY;
    ont_stop_timer(invite_txn.next_txn_timer);
    ont_stop_timer(ONT_INVITE_TIMER);
    
    // zero out invite_key_for good measure    
    one_net_memset(invite_key, 0, ONE_NET_XTEA_KEY_LEN);
    return ONS_SUCCESS;
} // one_net_master_cancel_invite //


/*!
    \brief Starts the process to remove a device from the network.

    \param[in] RAW_PEER_DID The device to remove from the network

    \return ONS_SUCCESS If the process to remove the device was started
            ONS_BAD_PARAM If the parameter was invalid
*/
one_net_status_t one_net_master_remove_device(
  const on_raw_did_t * const RAW_PEER_DID)
{
    return ONS_SUCCESS;
} // one_net_master_remove_device //


/*!
    \brief Returns the encryption key used by a client(or by the master
      itself).  This is a master-specific function added so that one_net.c
      can get the correct key for a client.  It is declared in one_net.h
      rather than one_net_master.h so that one_net.h does not have to
      include one_net_master.h

    \param[in] type Stream, Block, or Single
    \param[in] did of the device

    \return The key to use.  NULL if type is invalid or the device is not
            part of the network
*/
#ifdef _STREAM_MESSAGES_ENABLED
one_net_xtea_key_t* master_get_encryption_key(on_data_t type,
  const on_encoded_did_t* const did)
#else
one_net_xtea_key_t* master_get_encryption_key(
  const on_encoded_did_t* const did)
#endif
{
    on_client_t* client;
    
    if(!did)
    {
        return NULL;
    }
    
    if(on_encoded_did_equal(did, &MASTER_ENCODED_DID))
    {
        // the device is this master device.  Master is using the current key.
        #ifdef _STREAM_MESSAGES_ENABLED
        if(type == ON_STREAM)
        {
            return (one_net_xtea_key_t*)(on_base_param->stream_key);
        }
        #endif
        return (one_net_xtea_key_t*)(on_base_param->current_key);
    }
    
    client = client_info(did);
    if(client == NULL)
    {
        return NULL; // not in the network
    }
    
    #ifdef _STREAM_MESSAGES_ENABLED
    if(type == ON_STREAM)
    {
        return client->use_current_stream_key ?
          (one_net_xtea_key_t*)(on_base_param->stream_key) :
          (one_net_xtea_key_t*)(master_param->old_stream_key);
    }
    #endif
    
    return client->use_current_key ?
      (one_net_xtea_key_t*)(on_base_param->current_key) :
      (one_net_xtea_key_t*)(master_param->old_key);
} // master_get_encryption_key //


/*!
    \brief The main function for the ONE-NET MASTER.

    \param void

    \return void
*/
void one_net_master(void)
{
    // The current transaction
    static on_txn_t * txn = 0;

    // Do the appropriate action for the state the device is in.
    switch(on_state)
    {
#ifdef _IDLE
		case ON_IDLE:
		{
			break;
		}
#endif

        case ON_JOIN_NETWORK:
        {
            // If we are in ON_JOIN_NETWORK state, we aren't doing anything
            // except cruising for channels.  Nothing else occurs until we
            // find one.  We don't check transactions and we don't check any
            // timers except for timers involved in finding the clear channel.
            // Therefore we don't need to create a new timer for this process.
            // Instead we'll just use two that we already have since we know
            // that they aren't currently being used for their normal purposes.
            // We'll use the general timer and the change key timer.  Note that
            // they were initially set in the one_net_master_create_network()
            // function.
            if(one_net_channel_is_clear())
            {
                if(ont_expired(ONT_GENERAL_TIMER))
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    ont_stop_timer(ONT_CHANGE_KEY_TIMER);
                } // if channel has been clear for enough time //
            } // if channel is clear //
            else
            {
                // TODO - should this be random or should we simply increment?
                on_base_param->channel++;
                if(on_base_param->channel > ONE_NET_MAX_CHANNEL)
                {
                    on_base_param->channel = 0;
                }

                one_net_set_channel(on_base_param->channel);
                ont_set_timer(ONT_GENERAL_TIMER, new_channel_clear_time_out);

                // check if it's been long enough where the device thinks that
                // there is traffic on all the channels.  If that is the case
                // lower the time in hopes of finding the least busy channel.
                if(ont_inactive_or_expired(ONT_CHANGE_KEY_TIMER))
                {
                    new_channel_clear_time_out >>= 1;
                    ont_set_timer(ONT_CHANGE_KEY_TIMER,
                      ONE_NET_MASTER_CHANNEL_SCAN_TIME);
                } // if time to lower the channel clear time //
            } // else channel is not clear //
            break;
        } // ON_JOIN_NETWORK case //
        
        case ON_LISTEN_FOR_DATA:
        {
            if(invite_txn.priority != ONE_NET_NO_PRIORITY &&
              ont_inactive_or_expired(invite_txn.next_txn_timer))
            {
                UInt8 pid = ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT;
                
                if(ont_expired(ONT_INVITE_TIMER))
                {
                    one_net_master_invite_result(ONS_TIME_OUT, &invite_key, 0);
                    one_net_master_cancel_invite(
                      (const one_net_xtea_key_t * const)&invite_key);
                    break;
                } // if trying to add device timed out //
                
                txn = &invite_txn;
                
                #ifdef _ONE_NET_MULTI_HOP
                txn->retry++;
                if(mh_repeater_available && txn->retry > ON_INVITES_BEFORE_MULTI_HOP)
                {
                    txn->retry = 0;
                    pid = ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT;
                } // if time to send a multi hop packet //
                #endif

                if(!setup_pkt_ptr(pid, invite_pkt, &data_pkt_ptrs))
                {
                    break; // we should never get here
                }
                
                #ifdef _ONE_NET_MULTI_HOP
                if(pid == ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT)
                {
                    on_build_hops(data_pkt_ptrs.enc_hops_field,
                      features_max_hops(THIS_DEVICE_FEATURES), 0);
                }
                #endif

                on_state = ON_SEND_INVITE_PKT;
                break;
            }
        }

        default:
        {
            one_net(&txn);
            break;
        } // default case //
    } // switch(on_state) //
} // one_net_master //


/*!
    \brief Add a client to the current network.

    This function can be used when you need to add a client
    to the current network and you do not want to use the normal
    invite/join process. A client is added with the information
    supplied and the information that the client will need to
    communicate on the current network is returned.

    \param[in] features The client's features / capabilities.
    \param[out] out_base_param Pointer to the base parameters the new client should use.
    \param[out] out_master_param Pointer to the master parameters the new client should use.

    \return ONS_SUCCESS if the client was added.
            ONS_BAD_PARAM if the parameter was invalid.
            ONS_DEVICE_LIMIT if there is no room to hold another client.
*/
one_net_status_t one_net_master_add_client(const on_features_t features,
  on_base_param_t* out_base_param, on_master_t* out_master_param)
{
    // instead of separate arguments for values returned.
    one_net_status_t status;
    on_raw_did_t raw_did;

    on_client_t * client;
    
    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return ONS_DEVICE_LIMIT;
    }

    // a device is being added, place it in the next available client_t
    // structure
    client = &client_list[master_param->client_count];

    //
    // initialize the fields in the client_t structure for this new client
    //
    client->device_send_info.expected_nonce = ON_INVALID_NONCE;
    client->device_send_info.last_nonce = ON_INVALID_NONCE;
    client->device_send_info.send_nonce = 0;
    client->device_send_info.data_rate = ONE_NET_DATA_RATE_38_4;
    client->device_send_info.features = features;
    client->use_current_key = TRUE;
#ifdef _ONE_NET_MULTI_HOP
    client->device_send_info.max_hops = features_max_hops(features);
    client->device_send_info.hops = 0;
#endif
    one_net_int16_to_byte_stream(master_param->next_client_did, raw_did);
    on_encode(client->device_send_info.did, raw_did, ON_ENCODED_DID_LEN);
    
    one_net_memmove(&(out_base_param->sid[ON_ENCODED_NID_LEN]),
      client->device_send_info.did, ON_ENCODED_DID_LEN);
    one_net_memmove(out_base_param->sid, on_base_param->sid, ON_ENCODED_NID_LEN);
    out_master_param->device.features = THIS_DEVICE_FEATURES;
    out_master_param->device.expected_nonce = ON_INVALID_NONCE;
    out_master_param->device.last_nonce = ON_INVALID_NONCE;
    out_master_param->device.send_nonce = 0;
#ifdef _ONE_NET_MULTI_HOP
    out_master_param->device.max_hops = features_max_hops(THIS_DEVICE_FEATURES);
    out_master_param->device.hops = 0;
#endif
    one_net_memmove(out_master_param->device.did, MASTER_ENCODED_DID,
      ON_ENCODED_DID_LEN);
    one_net_memmove(out_base_param->current_key, on_base_param->current_key,
      sizeof(one_net_xtea_key_t));
    out_master_param->keep_alive_interval = ONE_NET_MASTER_DEFAULT_KEEP_ALIVE;
    out_base_param->single_block_encrypt = on_base_param->single_block_encrypt;
    out_base_param->channel = on_base_param->channel;
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_memmove(out_base_param->stream_key, on_base_param->stream_key,
      sizeof(one_net_xtea_key_t));
    out_base_param->stream_encrypt = on_base_param->stream_encrypt;
#endif
#ifdef _BLOCK_MESSAGES_ENABLED
    out_base_param->fragment_delay_low = on_base_param->fragment_delay_low;
    out_base_param->fragment_delay_high = on_base_param->fragment_delay_high;
#endif

    master_param->client_count++;
    master_param->next_client_did = find_lowest_vacant_did();
    
    #ifdef _ONE_NET_MULTI_HOP
    if(features_mh_repeat_capable(features))
    {
        mh_repeater_available = TRUE;
    }
    #endif

    return ONS_SUCCESS;
} // one_net_master_add_client //



//! @} ONE-NET_MASTER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
//! @{



// TODO -- document
#include "oncli.h"
static on_message_status_t on_master_single_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type)
{
    // TODO -- 11/28/2011
    // Look at the rx_single_data function in one_net.c.  It has
    // a lot of what is in here, but it appears is never called.
    // Figure out what code shoul go where and if we aren't going to
    // call rx_single_data from anywhere, delete it.
    
    on_message_status_t msg_status;
    one_net_status_t response_status;
    on_ack_nack_t ack_nack;
    on_msg_hdr_t msg_hdr;
    on_raw_did_t raw_src_did, raw_repeater_did;
    UInt8 response_pid;
    UInt8* resp_pid_grp;
    #ifndef _ONE_NET_SIMPLE_CLIENT
    UInt8 pid_type_idx;
    #endif
    on_sending_device_t* device;
    
    #ifndef _ONE_NET_SIMPLE_CLIENT
    enum
    {
        NUM_SINGLE_PIDS = 3
    };
    
    static const UInt8 single_pids[NUM_SINGLE_PIDS] =
    {
        ONE_NET_ENCODED_SINGLE_DATA,
        ONE_NET_ENCODED_LARGE_SINGLE_DATA,
        ONE_NET_ENCODED_EXTENDED_SINGLE_DATA
    };
    
    static const UInt8 response_pids[NUM_SINGLE_PIDS][2] =
    {
        {ONE_NET_ENCODED_SINGLE_DATA_ACK,
           ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN},
        {ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK,
           ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_RSN},
        {ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK,
           ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_RSN}
    };
    #else
    static const UInt response_pids[2] =
    {
        {ONE_NET_ENCODED_SINGLE_DATA_ACK,
           ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN}
    };
    #endif

    switch(*(pkt->pid))
    {
        case ONE_NET_ENCODED_SINGLE_DATA:
            #ifndef _ONE_NET_SIMPLE_CLIENT
            pid_type_idx = 0;
            #endif
            break;
        #ifndef _ONE_NET_SIMPLE_CLIENT
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
            pid_type_idx = 1;
            break;
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA:
            pid_type_idx = 2;
            break;        
        #endif
        default:
            // invalid PID -- abort
            *txn = 0;
            return ON_MSG_ABORT;
    }
    
    #ifndef _ONE_NET_SIMPLE_CLIENT
    resp_pid_grp = (UInt8*) response_pids[pid_type_idx];
    #else
    resp_pid_grp = (UInt8*) response_pids;
    #endif
    

    on_decode(raw_src_did, *(pkt->enc_src_did), ON_ENCODED_DID_LEN);
    on_decode(raw_repeater_did, *(pkt->enc_repeater_did), ON_ENCODED_DID_LEN);
    
    msg_hdr.msg_type = *msg_type;
    msg_hdr.pid = *(pkt->pid);
    msg_hdr.msg_id = pkt->msg_id;
    ack_nack.payload = (ack_nack_payload_t*) raw_pld;
    ack_nack.nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack.handle = ON_ACK;

    oncli_send_msg("Rcv'd packet : Source ");
    oncli_print_did(pkt->enc_src_did);
    oncli_send_msg(" : Rptr ");
    oncli_print_did(pkt->enc_repeater_did);
    oncli_send_msg(" : Msg type = %d : PID=%02X : ", *msg_type, *(pkt->pid));
    {
        UInt8 i;
        UInt8 len = get_raw_payload_len(*(pkt->pid)) -  1 - ON_PLD_DATA_IDX;
        oncli_send_msg("Raw payload length : %d\n", len);
        for(i = 0; i < len; i++)
        {
            oncli_send_msg("%02X ", raw_pld[i]);
        }
        
        oncli_send_msg("\n");
    }
    
    oncli_send_msg("Calling one_net_master_handle_single_pkt\n");
    
    #ifndef _ONE_NET_MULTI_HOP
    msg_status =  one_net_master_handle_single_pkt(raw_pld, &msg_hdr,
      &raw_src_did, &raw_repeater_did, &ack_nack);
    #else
    msg_status = one_net_master_handle_single_pkt(raw_pld, &msg_hdr,
      &raw_src_did, &raw_repeater_did, &ack_nack, (*txn)->hops,
      &((*txn)->max_hops));
    #endif
    

    oncli_send_msg("Return from one_net_master_handle_single_pkt\n");
    oncli_send_msg("msg_status=%d\n", msg_status);
    if(msg_status == ON_MSG_CONTINUE)
    {
        if(ack_nack.nack_reason == ON_NACK_RSN_NO_ERROR)
        {
            oncli_send_msg("Message ACK'd\n");
        }
        else
        {
            oncli_send_msg("Message NACK'd with reason %02X\n",
              ack_nack.nack_reason);
        }
        
        oncli_send_msg("Handle = %d\n", ack_nack.handle);
        if(ack_nack.handle == ON_ACK_STATUS)
        {
            UInt8 src_unit, dst_unit;
            ona_msg_class_t msg_class;
            UInt16 msg_type, msg_data;
            UInt8* pld = ack_nack.payload->status_resp;
            
            on_parse_app_pld(pld, &src_unit,
              &dst_unit, &msg_class, &msg_type, &msg_data);

            oncli_send_msg("ocsdh: payload:%02X%02X%02X%02X%02X ", pld[0],
              pld[1], pld[2], pld[3], pld[4]);

            oncli_send_msg("Src:%02X Dst:%02X ", src_unit, dst_unit);
            oncli_send_msg("Class:%04X Type:%04X ", msg_class, msg_type);
            oncli_send_msg("Data:%04X\n", msg_data);
        }
    }

    if(msg_status != ON_MSG_CONTINUE)
    {
        *txn = 0;
        return msg_status;
    }
    
    // we'll be sending it back to the souerce.
    if(!(device = sender_info(pkt->enc_src_did)))
    {
        // I think we should have solved this problem before now, but abort if
        // we have not.
        
        // TODO -- solve this better or confirm it is in fact solved earlier.
        *txn = 0;
        return ON_MSG_ABORT;
    }
    
    response_pid = (ack_nack.nack_reason == ON_NACK_RSN_NO_ERROR ?
      resp_pid_grp[0] : resp_pid_grp[1]);

    if(!setup_pkt_ptr(response_pid, response_txn.pkt, pkt))
    {
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    // the response destination will be the transaction's source
    on_build_my_pkt_addresses(pkt, (const on_encoded_did_t* const)
      &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]), NULL);

    response_txn.key = (*txn)->key;
    *txn = &response_txn;

    // TODO -- what about the hops?  We allowed the application code to
    // change them.  We need to pass that along.  Should we change "device"?
    
    // TODO -- harness the return value
    response_status = on_build_response_pkt(&ack_nack, pkt, *txn, device);
    response_status = on_complete_pkt_build(pkt, msg_hdr.msg_id, response_pid);
    return ON_MSG_CONTINUE;
}

  
// TODO -- document  
static on_message_status_t on_master_handle_single_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    on_raw_did_t src_did;
    UInt8 raw_hops_field;
    on_message_status_t status = ON_MSG_DEFAULT_BHVR;
    on_msg_hdr_t msg_hdr;
    
    if(!ack_nack || !txn || !(*txn))
    {
        // not sure how we got here, but we can't do anything
        return status;
    }    

    msg_hdr.pid = *(pkt->pid);
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;

    on_decode(src_did, *(pkt->enc_dst_did), ON_ENCODED_DID_LEN);
    
    #ifndef _ONE_NET_MULTI_HOP
    status = one_net_master_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &((*txn)->retry));
    #else
    status = one_net_master_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &((*txn)->retry), pkt->hops, &(pkt->max_hops));
    #endif    
    
    if(status == ON_MSG_DEFAULT_BHVR || status == ON_MSG_CONTINUE)
    {
        (*txn)->response_timeout = ack_nack->payload->nack_time_ms;
        
        if((*txn)->retry >= ON_MAX_RETRY)
        {
            #ifdef _ONE_NET_MULTI_HOP
            // we may be able to re-send with a higher max hops.
            
            if(mh_repeater_available && (*txn)->max_hops <
              (*txn)->device->max_hops)
            {
                on_raw_did_t raw_did;
                on_decode(raw_did, *(pkt->enc_dst_did), ON_ENCODED_DID_LEN);
                
                if((*txn)->max_hops == 0)
                {
                    (*txn)->max_hops = 1;
                }
                else
                {
                    ((*txn)->max_hops) *= 2;
                }
                
                if((*txn)->max_hops > (*txn)->device->max_hops)
                {
                    (*txn)->max_hops = (*txn)->device->max_hops;
                }
                
                // give the application code a chance to override if it
                // wants to.
                switch(one_net_adjust_hops(&raw_did, &(*txn)->max_hops))
                {
                    case ON_MSG_ABORT: return ON_MSG_ABORT;
                }             
                
                (*txn)->hops = 0;
                (*txn)->retry = 0;
                pkt->hops = (*txn)->hops;
                pkt->max_hops = (*txn)->max_hops;

                // change the pid if needed
                if((*txn)->max_hops)
                {
                    switch(*(pkt->pid))
                    {
                        case ONE_NET_ENCODED_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_MH_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA;
                          break;
                    }
                }
                else
                {
                    switch(*(pkt->pid))
                    {
                        case ONE_NET_ENCODED_MH_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_LARGE_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA:
                          *(pkt->pid) = ONE_NET_ENCODED_EXTENDED_SINGLE_DATA;
                          break;
                    }
                }
                
                if(on_complete_pkt_build(pkt, pkt->msg_id, *(pkt->pid)) !=
                  ONS_SUCCESS)
                {
                    return ON_MSG_TIMEOUT; // should never get here?
                }
                
                return ON_MSG_CONTINUE;
            }
            #endif
            return ON_MSG_TIMEOUT;
        }
    }

    return status;
}
  

// TODO -- document 
static on_message_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    on_msg_hdr_t msg_hdr;
    on_raw_did_t dst;
    
    msg_hdr.pid = *(pkt->pid);
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;
    on_decode(dst ,*(pkt->enc_dst_did), ON_ENCODED_DID_LEN);

    #ifndef _ONE_NET_MULTI_HOP
    one_net_master_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, &dst, ack_nack);
    #else
    one_net_master_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, &dst, ack_nack, pkt->hops);
    #endif  
}




#ifdef _BLOCK_MESSAGES_ENABLED
// TODO -- document
static on_message_status_t on_master_block_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type)
{
    return ON_MSG_CONTINUE;
}


// TODO -- document  
static on_message_status_t on_master_handle_block_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
  

// TODO -- document 
static on_message_status_t on_master_block_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
#endif




#ifdef _STREAM_MESSAGES_ENABLED
// TODO -- document
static on_message_status_t on_master_stream_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type)
{
    return ON_MSG_CONTINUE;
}


// TODO -- document  
static on_message_status_t on_master_handle_stream_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
  

// TODO -- document 
static on_message_status_t on_master_stream_txn_hdlr(on_txn_t ** txn,
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

    \return ONS_SUCCESS upon success.
*/
static one_net_status_t init_internal(void)
{
    pkt_hdlr.single_data_hdlr = &on_master_single_data_hdlr;
    pkt_hdlr.single_ack_nack_hdlr =
      &on_master_handle_single_ack_nack_response;
    pkt_hdlr.single_txn_hdlr = &on_master_single_txn_hdlr;
    
    #ifdef _BLOCK_MESSAGES_ENABLED
    pkt_hdlr.block_data_hdlr = &on_master_block_data_hdlr;
    pkt_hdlr.block_ack_nack_hdlr =
      &on_master_handle_block_ack_nack_response;
    pkt_hdlr.block_txn_hdlr = &on_master_block_txn_hdlr;
    #endif
    
    #ifdef _STREAM_MESSAGES_ENABLED
    pkt_hdlr.stream_data_hdlr = &on_master_stream_data_hdlr;
    pkt_hdlr.stream_ack_nack_hdlr =
      &on_master_handle_stream_ack_nack_response;
    pkt_hdlr.stream_txn_hdlr = &on_master_stream_txn_hdlr;
    #endif    

    one_net_send_single = &one_net_master_send_single;
    get_sender_info = &sender_info;
    device_is_master = TRUE;
    one_net_init();
    return ONS_SUCCESS;
} // init_internal //


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] CLIENT_DID The enocded device id of the CLIENT to retrieve the
      information for.

    \return The CLIENT information if the information was found
            0 If an error occured.
*/
static on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID)
{
    UInt16 i;

    if(!CLIENT_DID)
    {
        return 0;
    } // if the parameter is invalid //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(CLIENT_DID,
          (const on_encoded_did_t * const)&client_list[i].device_send_info.did))
        {
            return &(client_list[i]);
        } // if the CLIENT was found //
    } // loop to find the CLIENT //

    return 0;
} // client_info //


/*!
    \brief Removes the CLIENT from the network (by removing it from the table) 

    \param[in] DID The device ID of the CLIENT to remove

    \return ONS_SUCCESS if the client was deleted.
            ONS_INVALID_DATA if the client was not deleted.
*/
static one_net_status_t rm_client(const on_encoded_did_t * const DID)
{
	return ONS_SUCCESS;
} // rm_client //


/*!
    \brief Sorts the client list by did for a cleaner printout.
*/
static void sort_client_list_by_encoded_did(void)
{
    UInt16 i;
    on_raw_did_t raw_did1;
    on_raw_did_t raw_did2;
    
    // the client list will almost always be in order already.  When it's not,
    // generally only one element will be out of order, so instead of having a
    // nested loop, if we ever find an element out of order, we'll just iterate
    // through the whole list again.
    for(i = 1; i < master_param->client_count; i++)
    {
        on_decode(raw_did1, client_list[i-1].device_send_info.did,
          ON_ENCODED_DID_LEN);
        on_decode(raw_did2, client_list[i].device_send_info.did,
          ON_ENCODED_DID_LEN);
          
        if(one_net_byte_stream_to_int16(raw_did1) >
          one_net_byte_stream_to_int16(raw_did2))
        {
            // swap.
            on_client_t temp = client_list[i-1];
            client_list[i-1] = client_list[i];
            client_list[i] = temp;
            i = 0; // make it 0, not 1 because the loop increments i.
        }
    }
}


/*!
    \brief Find the lowest vacant did that can be assigned to the next client.

    \return the next available client did that can be assigned in raw form
            stored as a UInt16 (i.e. 0020, 0030, 0040, etc.).  Returns 0
            if the list is full.
*/
static UInt16 find_lowest_vacant_did(void)
{
    UInt16 i;
    on_raw_did_t raw_did;
    UInt16 vacant_did = ONE_NET_INITIAL_CLIENT_DID;
    
    sort_client_list_by_encoded_did();
    
    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return 0; // list is full.
    }
    
    // Note that the list is already sorted.  We'll go through it and see if
    // there are any gaps.  If not, we'll add ONE_NET_INITIAL_CLIENT_DID to
    // the last client and return that.
    for(i = 0; i < master_param->client_count; i++)
    {
        on_decode(raw_did, client_list[i].device_send_info.did,
          ON_ENCODED_DID_LEN);
        if(one_net_byte_stream_to_int16(raw_did) != vacant_did)
        {
            // a vacant did has been found
            return vacant_did;
        }
        
        vacant_did += ON_CLIENT_DID_INCREMENT;
    }
    
    // no gaps were found.
    return vacant_did;
}


/*!
    \brief Sends a single data message.
    
    The message is either sent to the peer list or only to the specific device
    that is passed in.
    
    \param[in] pid The pid of the message.
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
static one_net_status_t one_net_master_send_single(UInt8 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t* send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t* expire_time_from_now
  #endif
  )
{    
    if(push_queue_element(pid, msg_type, raw_data, data_len, priority, src_did,
      enc_dst
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


/*!
    \brief Finds the sender info.

    The return value should be checked for 0.  The expected_nonce and last nonce
    should then be compared.  If these two values are equal, then it is a new
    location so a NACK should be sent to the sender, and the new nonce filled
    out.  The last nonce value should not be a valid nonce value and should be
    left unchanged for the time being.

    \param[in] DID The device id of the device.

    \return Pointer to location that holds the sender information (should be
      checked for 0, and should be checked if a new location).
*/
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID)
{
    on_client_t* client = client_info(DID);
    if(client == NULL)
    {
        return NULL;
    }
    
    return &(client->device_send_info);
}



//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE-NET_MASTER


#endif // if _ONE_NET_MASTER defined //