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

#ifdef ONE_NET_MASTER

#if SINGLE_QUEUE_LEVEL < MED_SINGLE_QUEUE_LEVEL
    #error "SINGLE_QUEUE_LEVEL must be at least at level MED_SINGLE_QUEUE_LEVEL if ONE_NET_MASTER is defined"
#endif



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
#include "one_net.h"
#ifdef PEER
#include "one_net_peer.h"
#endif



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{


//! Number of bits to shift the initial CLIENT address to use as a 16-bit
//! or raw address value
#define ON_INIT_CLIENT_SHIFT RAW_DID_SHIFT


#ifdef ONE_NET_MULTI_HOP
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


//! the updates where everyone should be notified.
enum
{
    ON_CLIENT_UPDATE_ADD_DEVICE,
    ON_CLIENT_UPDATE_RM_DEVICE,
    ON_CLIENT_UPDATE_CHANGE_KEY,
    NUM_CLIENT_UPDATE_TYPES
};


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
  
//! Unique key of the device being invited into the network
one_net_xtea_key_t invite_key;



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

//! Flag to denote that a key update is in progress.
static BOOL key_update_in_progress = FALSE;

//! Flag to denote that a device has been removed and the master is in
//! the process of informing all of the devices.
static BOOL remove_device_update_in_progress = FALSE;

//! Flag to denote that a device has been added and the master is in
//! the process of informing all of the devices.
static BOOL add_device_update_in_progress = FALSE;

//! Flag to denote the device that should next be notified of any updates,
//! if any.  Generally denotes either the device being added or removed or
//! a normally-sleeping device that has woken up and we thus want to move
//! it to the head of the list.  If NULL, then no device takes precedence
static on_client_t* device_to_update = NULL;

//! The did of the device being removed.  Irrelevant if broadcast
static on_encoded_did_t remove_device_did = {0xB4, 0xB4};

//! The did of the device being removed.  Irrelevant if broadcast.
static on_encoded_did_t add_device_did = {0xB4, 0xB4};

//! Flag for whether the device being added has been notified of its
//! settings / flags.
static BOOL settings_sent;

#ifdef BLOCK_MESSAGES_ENABLED
//! Flag for whether the device being added has been notified of its fragment
//! delays.
static BOOL fragment_delay_sent;
#endif

//! The time that the remove device update started.
static tick_t remove_device_start_time = 0;

//! The time that the add device update started.
static tick_t add_device_start_time = 0;



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
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_master_handle_single_ack_nack_response(
  on_txn_t* txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);
static on_message_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);
  
  
static void admin_txn_hdlr(const UInt8* const raw_pld,
  const on_raw_did_t* const raw_did, const on_ack_nack_t* const ack_nack,
  on_client_t* client);

#ifdef BLOCK_MESSAGES_ENABLED
static on_message_status_t on_master_block_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* block_pkt, on_ack_nack_t* ack_nack);
static on_message_status_t on_master_handle_block_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack);
static on_message_status_t on_master_block_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);
#endif

#ifdef STREAM_MESSAGES_ENABLED
static on_message_status_t on_master_stream_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* stream_pkt, on_ack_nack_t* ack_nack);
static on_message_status_t on_master_handle_stream_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack);
static on_message_status_t on_master_stream_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack);
#endif

static one_net_status_t init_internal(void);
static one_net_status_t rm_client(const on_encoded_did_t * const CLIENT_DID);
// TODO -- June 2, 2012 - sort_client_list_by_encoded_did appears to not be
// used anywhere.  Commenting out for a clean compile
#if 0
static void sort_client_list_by_encoded_did(void);
#endif
static UInt16 find_lowest_vacant_did(void);
static SInt16 find_vacant_client_list_index(void);

static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);
static void check_updates_in_progress(void);


static one_net_status_t send_admin_pkt(const UInt8 admin_msg_id,
  const on_encoded_did_t* const did, const UInt8* const pld,
  tick_t send_time_from_now);
  
static on_message_status_t handle_admin_pkt(const on_encoded_did_t * const
  SRC_DID, const UInt8 * const DATA, on_client_t ** client,
  on_ack_nack_t* ack_nack);

static BOOL is_invite_did(const on_encoded_did_t* const encoded_did);
static on_client_t* get_invite_client(void);
static void on_master_adjust_recipient_list(const on_single_data_queue_t*
  const msg, on_recipient_list_t** recipient_send_list);
  
static void check_clients_for_missed_check_ins(void);



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

    \return ONS_SUCCESS if the network was created.
            ONS_BAD_PARAM if the parameter was invalid
*/
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY)
{
    if(!SID || !KEY)
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

#ifdef BLOCK_MESSAGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
#endif
    one_net_master_clear_client_memory();
    init_internal();
    new_channel_clear_time_out = ONE_NET_MASTER_NETWORK_CHANNEL_CLR_TIME;
    
    // see comment in the "case ON_JOIN_NETWORK" segment of the
    // one_net_master() loop to explain we are using these timers and what
    // we're doing.
    ont_set_timer(ONT_GENERAL_TIMER, MS_TO_TICK(new_channel_clear_time_out));
    ont_set_timer(ONT_UPDATE_TIMER,
      MS_TO_TICK(one_net_master_channel_scan_time));
    on_state = ON_JOIN_NETWORK;

    return ONS_SUCCESS;
} // one_net_master_create_network //


void one_net_master_clear_client_memory(void)
{
    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID;
    master_param->client_count = 0;
    one_net_master_condense_client_memory();
    #ifdef PEER
    one_net_reset_peers();
    #endif    
    #ifdef ONE_NET_MULTI_HOP
    on_base_param->num_mh_devices = 1; // for the master
    on_base_param->num_mh_repeaters = 0; // new network, no clients,
                                         // master is not a repeater
    #endif
}


void one_net_master_condense_client_memory(void)
{
    SInt16 i;
    UInt16 num_clients_encountered = 0;
    
    for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
    {
        if(num_clients_encountered >= master_param->client_count)
        {
            break;
        }
        if(is_broadcast_did((const on_encoded_did_t*)
          client_list[i].device.did))
        {
            // move everything up
            one_net_memmove(&client_list[i], &client_list[i+1],
              (ONE_NET_MASTER_MAX_CLIENTS - i - 1) * sizeof(on_client_t));
            i--;
        }
        else
        {
            num_clients_encountered++;
        }
    }
    
    for(i = master_param->client_count; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
    {
        one_net_memmove(client_list[i].device.did, ON_ENCODED_BROADCAST_DID,
          ON_ENCODED_DID_LEN);
    }
}



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
#ifndef PEER
one_net_status_t one_net_master_init(const UInt8 * PARAM, UInt16 PARAM_LEN)
#else
one_net_status_t one_net_master_init(const UInt8 * PARAM, UInt16 PARAM_LEN,
  memory_type_t memory_type)
#endif
{
    UInt8 i;
    one_net_status_t status;
    
    // The number of bytes in the non-volatile parameter buffer that have been
    // initialized so far.
    static UInt16 nv_param_size_needed = MAX_MASTER_NV_PARAM_SIZE_BYTES;
    #ifdef PEER
    static UInt8 peer_memory_size_needed = PEER_STORAGE_SIZE_BYTES;
    #endif
    
    // There are several options.  This function may be called with PARAM equal to NULL.
    // In this case, it is assumed that the application code has already copied the non-volatile
    // memory to the appropriate buffer.  Therefore this function will NOT attempt to do so.
    // If PARAM_LEN is equal to 0, it will be assumed that all remaining memory of type
    // memory_type has been passed to the function.  one_net_master_init will figure out how
    // much memory is needed.  If PARAM_LEN is non-zero, it will be assumed that the buffer length
    // passed to this function is PARAM_LEN bytes.  The number of bytes to be copied will be the lesser
    // of the PARAM_LEN and the number of bytes needed to finish the initialization.
    
    // Finally, and this is only relevant when PEER is enabled, memory_type denotes what type of memory
    // has been passed (peer or non-peer or undistinguished).  If the memory is undistinguished, it is
    // assumed to represent one big buffer with the non-peer memory first followed by the peer memory.
    
    if(PARAM_LEN == 0)
    {
        #ifndef PEER
        PARAM_LEN = nv_param_size_needed;
        #else
        switch(memory_type)
        {
            case MEMORY_GENERIC:
                PARAM_LEN = nv_param_size_needed + peer_memory_size_needed;
                break;
            case MEMORY_NON_PEER:
                PARAM_LEN = nv_param_size_needed;
                break;
            case MEMORY_PEER:
                PARAM_LEN = peer_memory_size_needed;
                break;
            default:
                return ONS_BAD_PARAM;
        }
        #endif
    }
    
    #ifdef PEER
    if(memory_type == MEMORY_GENERIC)
    {
        if(nv_param_size_needed == 0)
        {
            memory_type = MEMORY_PEER;
        }
        else if(nv_param_size_needed <= nv_param_size_needed)
        {
            memory_type = MEMORY_NON_PEER;
        }
        else
        {
            UInt16 temp = nv_param_size_needed;
            status = one_net_master_init(PARAM, nv_param_size_needed, MEMORY_NON_PEER);
            if(status != ONS_MORE)
            {
                return status;
            }
            
            if(PARAM)
            {
                PARAM += temp;
            }
            PARAM_LEN -= temp;
            memory_type = MEMORY_PEER;
        }
    }
    #endif    
    
    #ifdef PEER
    if(memory_type == MEMORY_NON_PEER)
    {
        if(PARAM_LEN > nv_param_size_needed)
        {
            PARAM_LEN = nv_param_size_needed;
        }

        if(PARAM && nv_param_size_needed > 0)
        {
            one_net_memmove(&nv_param[MAX_MASTER_NV_PARAM_SIZE_BYTES - nv_param_size_needed],
              PARAM, PARAM_LEN);
        }
        nv_param_size_needed -= PARAM_LEN;
    }
    else if(memory_type == MEMORY_PEER)
    {
        if(PARAM_LEN > peer_memory_size_needed)
        {
            PARAM_LEN = peer_memory_size_needed;
        }
        
        if(PARAM && peer_memory_size_needed > 0)
        {
            one_net_memmove(&peer_storage[PEER_STORAGE_SIZE_BYTES - peer_memory_size_needed],
              PARAM, PARAM_LEN);
        }
        peer_memory_size_needed -= PARAM_LEN;
    }
    else
    {
        return ONS_BAD_PARAM;
    }

    #else
    
    if(PARAM_LEN > nv_param_size_needed)
    {
        PARAM_LEN = nv_param_size_needed;
    }
    
    if(PARAM && nv_param_size_needed > 0)
    {
        one_net_memmove(&nv_param[MAX_MASTER_NV_PARAM_SIZE_BYTES - nv_param_size_needed],
          PARAM, PARAM_LEN);
    }
    nv_param_size_needed -= PARAM_LEN;
    
    #endif
    
    #ifndef PEER
    if(nv_param_size_needed)
    {
        return ONS_MORE;
    }
    #else
    if(nv_param_size_needed || peer_memory_size_needed)
    {
        return ONS_MORE;
    }
    #endif    

    // Last thing to check is the CRC
    #ifndef PEER
    if(on_base_param->crc != master_nv_crc(NULL))
    #else
    if(on_base_param->crc != master_nv_crc(NULL, NULL))
    #endif
    {
        return ONS_INVALID_DATA;
    }

    #ifdef ONE_NET_MULTI_HOP
    // check for repeater
    for(i = 0; i < master_param->client_count; i++)
    {
        if(features_mh_capable(client_list[i].device.features))
        {
            on_base_param->num_mh_devices++;
            break;
        } // if client is a multi-hop client //
        
        if(features_mh_repeat_capable(client_list[i].device.features))
        {
            on_base_param->num_mh_repeaters++;
            break;
        } // if client is a multi-hop repeater //
    } // loop to look for Multi-Hop and Multi-Hop repeaters //
    #endif
    
    on_state = ON_LISTEN_FOR_DATA;
    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status; // if initializing the internals failed //
    }
    return ONS_SUCCESS;
} // one_net_master_init //


one_net_status_t one_net_master_change_key_fragment(
  const one_net_xtea_key_fragment_t key_fragment)
{
    UInt8 i;
    if(key_update_in_progress)
    {
        return ONS_ALREADY_IN_PROGRESS;
    }
    
    // check to make sure the new key fragment doesn't match any of the old
    // ones.
    if(!new_key_fragment(
      (const one_net_xtea_key_fragment_t*)key_fragment, TRUE))
    {
        return ONS_BAD_KEY_FRAGMENT;
    }
    
    key_update_in_progress = TRUE;
    reset_msg_ids();
    for(i = 0; i < master_param->client_count; i++)
    {
        client_list[i].use_current_key = FALSE;
    }
    
    #ifdef AUTO_SAVE
    save = TRUE;
    #endif    
      
    return ONS_SUCCESS;
} // one_net_master_change_key_fragment //


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
    on_client_t* client;
    tick_t time_now = get_tick_count();
    on_raw_did_t raw_invite_did;
    SInt16 vacant_index;

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

    if((vacant_index = find_vacant_client_list_index()) < 0)
    {
        return ONS_DEVICE_LIMIT;
    } // if the MASTER has reached it's device limit //

    one_net_memmove(invite_key, *KEY, sizeof(invite_key));
    raw_invite[ON_INVITE_VERSION_IDX] = ON_INVITE_PKT_VERSION;
    one_net_uint16_to_byte_stream(master_param->next_client_did,
      &(raw_invite[ON_INVITE_ASSIGNED_DID_IDX]));
    one_net_memmove(&(raw_invite[ON_INVITE_KEY_IDX]),
      on_base_param->current_key, sizeof(on_base_param->current_key));
    one_net_memmove(&raw_invite[ON_INVITE_FEATURES_IDX],
      &THIS_DEVICE_FEATURES, sizeof(on_features_t));

    raw_invite[ON_INVITE_CRC_IDX] = (UInt8)one_net_compute_crc(
      &raw_invite[ON_INVITE_CRC_START_IDX],
      ON_INVITE_DATA_LEN, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);

    #ifdef STREAM_MESSAGES_ENABLED
    status = on_encrypt(FALSE, raw_invite,
      (const one_net_xtea_key_t * const)(&invite_key), ON_RAW_INVITE_SIZE);
    #else
    status = on_encrypt(raw_invite,
      (const one_net_xtea_key_t * const)(&invite_key), ON_RAW_INVITE_SIZE);
    #endif

    if(status != ONS_SUCCESS)
    {
        one_net_master_cancel_invite(
          (const one_net_xtea_key_t * const)&invite_key);
        return status;
    } // if the invite was not created successfully //


    // so far, so good.  Start building the packet and pick a random message id
    if(!setup_pkt_ptr(ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT, invite_txn.pkt,
      one_net_prand(time_now, 50), &data_pkt_ptrs))
    {
        return ONS_INTERNAL_ERR;
    }

    // fill in the addresses
    if((status = on_build_my_pkt_addresses(&data_pkt_ptrs,
      (const on_encoded_did_t*) ON_ENCODED_BROADCAST_DID,
      (const on_encoded_did_t*) MASTER_ENCODED_DID)) != ONS_SUCCESS)
    {
        return status;
    }
    
    // encode the payload
    if((status = on_encode(&(data_pkt_ptrs.packet_bytes[ON_ENCODED_PLD_IDX]),
      raw_invite, ON_ENCODED_INVITE_SIZE)) != ONS_SUCCESS)
    {
        return status;
    }
    
    // now finish building the packet.
    if((status = on_complete_pkt_build(&data_pkt_ptrs,
      ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT)) != ONS_SUCCESS)
    {
        return status;                          
    }

    // everything worked out fine.  Set the transactions and timers
    invite_txn.data_len = ON_ENCODED_INVITE_SIZE;
    invite_txn.response_timeout = one_net_master_invite_send_time;
    invite_txn.priority = ONE_NET_LOW_PRIORITY;
    ont_set_timer(invite_txn.next_txn_timer, 0);
    ont_set_timer(ONT_INVITE_TIMER, MS_TO_TICK(timeout));
    
    
    // now set up the next unused position in client_list for this client
    client = &client_list[vacant_index];
    client->flags = ONE_NET_MASTER_SEND_TO_MASTER ? ON_SEND_TO_MASTER : 0;
    client->flags |= (ONE_NET_MASTER_REJECT_INVALID_MSG_ID ?
      ON_REJECT_INVALID_MSG_ID : 0);
      
    #ifdef BLOCK_MESSAGES_ENABLED
    #ifdef DATA_RATE_CHANNEL
    client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_ELEVATE_DATA_RATE ?
      ON_BS_ELEVATE_DATA_RATE : 0); 
    #endif
    client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_CHANGE_CHANNEL ?
      ON_BS_CHANGE_CHANNEL : 0); 
    client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_HIGH_PRIORITY ?
      ON_BS_HIGH_PRIORITY : 0); 
    client->flags |= (ONE_NET_MASTER_CLIENT_ALLOW_LONG_BLOCK_STREAM ?
      ON_BS_ALLOWED : 0); 
    #endif
      
    client->use_current_key = TRUE;
    client->keep_alive_interval = ONE_NET_MASTER_DEFAULT_KEEP_ALIVE;
    client->device.data_rate = ONE_NET_DATA_RATE_38_4;
    client->device.msg_id = data_pkt_ptrs.msg_id;
    one_net_uint16_to_byte_stream(master_param->next_client_did,
      raw_invite_did);
    on_encode(client->device.did, raw_invite_did,
      ON_ENCODED_DID_LEN);
    client->device.features = FEATURES_UNKNOWN;
    
    one_net_memmove(add_device_did, client->device.did, ON_ENCODED_DID_LEN);
    
    settings_sent = FALSE;
    #ifdef BLOCK_MESSAGES_ENABLED
    fragment_delay_sent = FALSE;
    #endif
    
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
    invite_txn.priority = ONE_NET_NO_PRIORITY;
    ont_stop_timer(invite_txn.next_txn_timer);
    ont_stop_timer(ONT_INVITE_TIMER);
    
    // zero out invite_key_for good measure    
    one_net_memset(invite_key, 0, ONE_NET_XTEA_KEY_LEN);
    
    #ifdef COMPILE_WO_WARNINGS
    if(!KEY)
    {
        return ONS_SUCCESS;
    }
    #endif
    return ONS_SUCCESS;
} // one_net_master_cancel_invite //


/*!
    \brief Starts the process to remove a device from the network.

    \param[in] RAW_DID The device to remove from the network

    \return ONS_SUCCESS If the process to remove the device was started
            ONS_BAD_PARAM If the parameter was invalid
*/
one_net_status_t one_net_master_remove_device(
  const on_raw_did_t * const RAW_DID)
{
    one_net_status_t status;
    UInt8 i;
    on_client_t* client;
    UInt8 admin_pld[4];
    
    
    if(remove_device_update_in_progress)
    {
        return ONS_ALREADY_IN_PROGRESS;
    }

    if(!RAW_DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //
    
    // first check if we're in the middle of a transaction.  If so, we're
    // busy and can't do this.
    // TODO - we want a better solution?
    if(single_txn.priority != ONE_NET_NO_PRIORITY || response_txn.priority !=
      ONE_NET_NO_PRIORITY)
    {
        return ONS_BUSY;
    }
    
    #ifdef BLOCK_MESSAGES_ENABLED
    if(bs_txn.priority != ONE_NET_NO_PRIORITY)
    {
        return ONS_BUSY;
    }
    #endif
    
    if((status = on_encode(remove_device_did, *RAW_DID, ON_ENCODED_DID_LEN))
      != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(!(client = client_info((const on_encoded_did_t*)
      &remove_device_did)))
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //
    
    #ifdef ONE_NET_MULTI_HOP
    if(features_mh_capable(client->device.features))
    {
        on_base_param->num_mh_devices--;
    }
    if(features_mh_repeat_capable(client->device.features))
    {
        on_base_param->num_mh_repeaters--;
    }
    #endif
    
    remove_device_update_in_progress = TRUE;
    remove_device_start_time = get_tick_count();

    
    for(i = 0; i < master_param->client_count; i++)
    {
        client_list[i].send_remove_device_message = TRUE;
    }
    
    #ifdef PEER
    // remove any peers of this device.
    one_net_remove_peer_from_list(ONE_NET_DEV_UNIT, NULL,
      (const on_encoded_did_t* const) &remove_device_did, ONE_NET_DEV_UNIT);
    #endif
    
    admin_pld[0] = remove_device_did[0];
    admin_pld[1] = remove_device_did[1];
    #ifdef ONE_NET_MULTI_HOP
    admin_pld[2] = on_base_param->num_mh_devices;
    admin_pld[3] = on_base_param->num_mh_repeaters;
    #else
    // TODO -- should we ban multi-hop just because the master isn't capable?
    admin_pld[2] = 0;
    admin_pld[3] = 0;
    #endif
    
    #ifdef AUTO_SAVE
    save = TRUE;
    #endif    

    // all client peer assignments to this did are removed.  Now remove the device itself.
    // When that's done, the other devices will also be informed of this deletion.
    return send_admin_pkt(ON_RM_DEV, (const on_encoded_did_t * const)&remove_device_did,
      admin_pld, 0);
} // one_net_master_remove_device //


/*!
    \brief Returns the encryption key used by a client(or by the master
      itself).  This is a master-specific function added so that one_net.c
      can get the correct key for a client.  It is declared in one_net.h
      rather than one_net_master.h so that one_net.h does not have to
      include one_net_master.h

    \param[in] did of the device

    \return The key to use.  NULL if type is invalid or the device is not
            part of the network
*/
one_net_xtea_key_t* master_get_encryption_key(
  const on_encoded_did_t* const did)
{
    on_client_t* client;
    
    if(!did)
    {
        return NULL;
    }
    
    if(on_encoded_did_equal(did, &MASTER_ENCODED_DID))
    {
        // the device is this master device.  Master is using the current key.
        return (one_net_xtea_key_t*)(on_base_param->current_key);
    }
    
    client = client_info(did);
    if(client == NULL)
    {
        if(is_invite_did(did))
        {
            return (one_net_xtea_key_t*)(on_base_param->current_key);
        }
        return NULL; // not in the network
    }
    
    return client->use_current_key ?
      (one_net_xtea_key_t*)(on_base_param->current_key) :
      (one_net_xtea_key_t*)(on_base_param->old_key);
} // master_get_encryption_key //


/*! \brief Determines whether a DID is a DID that is currently being

    \param[in] encoded_did The did to check
    
    \return TRUE if there is an invite pending and this is the DID
            FALSE if no invite is pending or the DIDs do not match
*/
static BOOL is_invite_did(const on_encoded_did_t* const encoded_did)
{
    on_raw_did_t raw_did;
    if(!encoded_did || on_decode(raw_did, *encoded_did, ON_ENCODED_DID_LEN)
      != ONS_SUCCESS || invite_txn.priority == ONE_NET_NO_PRIORITY)
    {
        return FALSE;
    }
    if(did_to_u16((const on_raw_did_t*) raw_did) ==
      (master_param->next_client_did >> RAW_DID_SHIFT))
    {
        return TRUE;
    }
    return FALSE;
}


/*! \brief Returns a pointer to the client that is currently being invited
    
    \return A pointer to the client that is currently being invited, or NULL
            if no client is being invited.
*/
static on_client_t* get_invite_client(void)
{
    UInt8 i;
    for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
    {
        on_client_t* client = &client_list[i];
        if(is_invite_did((const on_encoded_did_t* const)
          client_list[i].device.did))
        {
            return client;
        }
    }
    return NULL;
}


/*!
    \brief The main function for the ONE-NET MASTER.

    \param void

    \return void
*/
void one_net_master(void)
{
    // The current transaction
    static on_txn_t * txn = 0;
    tick_t queue_sleep_time;
    

    // Do the appropriate action for the state the device is in.
    switch(on_state)
    {
#ifdef IDLE
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
            // We'll use the general timer and the update timer.  Note that
            // they were initially set in the one_net_master_create_network()
            // function.
            if(one_net_channel_is_clear())
            {
                if(ont_expired(ONT_GENERAL_TIMER))
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    ont_stop_timer(ONT_UPDATE_TIMER);
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
                if(ont_inactive_or_expired(ONT_UPDATE_TIMER))
                {
                    new_channel_clear_time_out >>= 1;
                    ont_set_timer(ONT_UPDATE_TIMER,
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
                UInt16 raw_pid = ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT |
                  (3 << ONE_NET_RAW_PID_SIZE_SHIFT);
                on_client_t* invite_client;
                
                if(ont_expired(ONT_INVITE_TIMER))
                {
                    one_net_master_invite_result(ONS_TIME_OUT, &invite_key, 0);
                    one_net_master_cancel_invite(
                      (const one_net_xtea_key_t * const)&invite_key);
                    break;
                } // if trying to add device timed out //
                
                if((invite_client = get_invite_client()) == NULL)
                {
                    // should never get here?
                    break;
                }

                
                txn = &invite_txn;
                
                #ifdef ONE_NET_MULTI_HOP
                txn->retry++;
                if(on_base_param->num_mh_repeaters &&
                  txn->retry > ON_INVITES_BEFORE_MULTI_HOP)
                {
                    txn->retry = 0;
                    raw_pid |= ONE_NET_RAW_PID_MH_MASK;
                } // if time to send a multi hop packet //
                #endif

                // set up the message id.
                if(!setup_pkt_ptr(raw_pid, invite_txn.pkt,
                  invite_client->device.msg_id, &data_pkt_ptrs))
                {
                    break; // we should never get here
                }
                
                #ifdef ONE_NET_MULTI_HOP
                if(raw_pid | ONE_NET_RAW_PID_MH_MASK)
                {
                    on_build_hops(&data_pkt_ptrs, 0,
                      features_max_hops(THIS_DEVICE_FEATURES));
                }
                if(txn->retry < 2)
                {
                    // we're either switching from multi-hop to non-multi-hop
                    // or vice-versa, so we need to re-calculate the message
                    // crc.
                    UInt8 msg_crc = calculate_msg_crc(&data_pkt_ptrs);
                    data_pkt_ptrs.packet_bytes[ON_ENCODED_MSG_CRC_IDX] =
                      decoded_to_encoded_byte(msg_crc, TRUE);
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
    
    // one_net() has had a chance to load any messages it wanted from the
    // queue.  If we are in state ON_LISTEN_FOR_DATA and there are no messages
    // ready to send within a second and there is no active message,
    // we'll try doing some admin stuff like checking for any key changes.
    if(on_state == ON_LISTEN_FOR_DATA && single_msg_ptr == NULL &&
      single_data_queue_ready_to_send(&queue_sleep_time) == -1 &&
      queue_sleep_time < MS_TO_TICK(500))
    {
        check_updates_in_progress();
        #ifdef NON_VOLATILE_MEMORY
        if(save)
        {
            one_net_master_condense_client_memory();
            one_net_master_save_settings();
            save = FALSE;
        }
        #endif
        check_clients_for_missed_check_ins();
    }
} // one_net_master //


/*!
    \brief Add a client to the current network.

    This function can be used when you need to add a client
    to the current network and you do not want to use the normal
    invite/join process. A client is added with the information
    supplied and the information that the client will need to
    communicate on the current network is returned.  It is also used by the
    master in the ONE-NET invite process

    \param[in] features The client's features / capabilities.
    \param[out] out_base_param Pointer to the base parameters the new client should use.
    \param[out] out_master_param Pointer to the master parameters the new client should use.
    \param[in] send_update_to_network If true, the network should be notified
               of the addition of this client.

    \return ONS_SUCCESS if the client was added.
            ONS_BAD_PARAM if the parameter was invalid.
            ONS_DEVICE_LIMIT if there is no room to hold another client.
*/
one_net_status_t one_net_master_add_client(const on_features_t features,
  on_base_param_t* out_base_param, on_master_t* out_master_param,
  BOOL send_update_to_network)
{
    on_raw_did_t raw_did;
    on_client_t * client;
    
    
    SInt16 vacant_index = find_vacant_client_list_index();
    
    if(vacant_index < 0)
    {
        return ONS_DEVICE_LIMIT;
    }
    
    #ifdef ONE_NET_MULTI_HOP
    if(features_mh_capable(features))
    {
        on_base_param->num_mh_devices++;
    }
    
    if(features_mh_repeat_capable(features))
    {
        on_base_param->num_mh_repeaters++;
    }
    #endif    

    // a device is being added, place it in the next available client_t
    // structure
    client = &client_list[vacant_index];

    //
    // initialize the fields in the client_t structure for this new client
    //    
    client->device.msg_id = one_net_prand(get_tick_count(), 50);
    client->flags = ONE_NET_MASTER_SEND_TO_MASTER ? ON_SEND_TO_MASTER : 0;
    client->flags |= (ONE_NET_MASTER_REJECT_INVALID_MSG_ID ?
      ON_REJECT_INVALID_MSG_ID : 0);
    #ifdef BLOCK_MESSAGES_ENABLED
    if(features_block_capable(client->device.features))
    {
        #ifdef DATA_RATE_CHANNEL
        client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_ELEVATE_DATA_RATE ?
          ON_BS_ELEVATE_DATA_RATE : 0); 
        #endif
        client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_CHANGE_CHANNEL ?
          ON_BS_CHANGE_CHANNEL : 0); 
        client->flags |= (ONE_NET_MASTER_CLIENT_BLOCK_STREAM_HIGH_PRIORITY ?
          ON_BS_HIGH_PRIORITY : 0); 
        client->flags |= (ONE_NET_MASTER_CLIENT_ALLOW_LONG_BLOCK_STREAM ?
          ON_BS_ALLOWED : 0); 
    }
    #endif
    client->flags |= ON_JOINED;
    client->device.data_rate = ONE_NET_DATA_RATE_38_4;
    client->device.features = features;
    client->send_remove_device_message = FALSE;
    client->use_current_key = TRUE;
    client->keep_alive_interval = ONE_NET_MASTER_DEFAULT_KEEP_ALIVE;
    // give it 5 extra seconds.
    client->next_check_in_time = get_tick_count() +
      MS_TO_TICK(5000 + client->keep_alive_interval);
      
    #ifdef ONE_NET_MULTI_HOP
    client->device.max_hops = features_max_hops(features);
    client->device.hops = 0;
    #endif
    one_net_uint16_to_byte_stream(master_param->next_client_did, raw_did);
    on_encode(client->device.did, raw_did, ON_ENCODED_DID_LEN);
    
    
    // if these are not NULL, the master is passing these parameters to the
    // client somehow, most likely as part of a process which is NOT part of
    // a ONE-NET protocol invite (an example of this might be if a device is
    // temporarily plugged in / attached to the master in a way that the
    // master is in direct communication with the client rather than
    // communicating through ONE-NET messages.  Note that this very often
    // means that the client is directly attached to the master physically
    // through a physical plug, but the two devices could also be on opposite
    // sides of the world and each side is attached to a computer via the
    // serial port and the information is transmitted via the internet.
    // Regardless, the point is that this function can be used for any invite
    // process that is not using a ONE-NET message protocol.  If this is the
    // case, out_base_param and out_master_param must be non-NULL.  For ONE-NET
    // invite processes, they should be NULL.
    if(out_base_param && out_master_param)
    {
        // TODO -- what about the number of multi-hop and multi-hop repeaters?
        // The device added possibly needs to know.  Note that this is
        // irrelevant which are added with the wireless ONE-NET invite process.
        
        one_net_memmove(&(out_base_param->sid[ON_ENCODED_NID_LEN]),
          client->device.did, ON_ENCODED_DID_LEN);
        one_net_memmove(out_base_param->sid, on_base_param->sid, ON_ENCODED_NID_LEN);
        out_master_param->device.features = THIS_DEVICE_FEATURES;
        out_master_param->device.msg_id = one_net_prand(get_tick_count(), 50);
        #ifdef ONE_NET_MULTI_HOP
        out_master_param->device.max_hops = features_max_hops(THIS_DEVICE_FEATURES);
        out_master_param->device.hops = 0;
        #endif
        one_net_memmove(out_master_param->device.did, MASTER_ENCODED_DID,
          ON_ENCODED_DID_LEN);
        one_net_memmove(out_base_param->current_key, on_base_param->current_key,
          sizeof(one_net_xtea_key_t));    
        out_master_param->keep_alive_interval = ONE_NET_MASTER_DEFAULT_KEEP_ALIVE;
        out_base_param->channel = on_base_param->channel;
        #ifdef BLOCK_MESSAGES_ENABLED
        out_base_param->fragment_delay_low = on_base_param->fragment_delay_low;
        out_base_param->fragment_delay_high = on_base_param->fragment_delay_high;
        #endif
    }

    master_param->client_count++;
    master_param->next_client_did = find_lowest_vacant_did();
    
    if(send_update_to_network)
    {
        UInt8 i;
        add_device_update_in_progress = TRUE;
        add_device_did[0] = client->device.did[0];
        add_device_did[1] = client->device.did[1];
        add_device_start_time = get_tick_count();
        device_to_update = client; // update the device being added
                                   // first. 
        
        for(i = 0; i < master_param->client_count; i++)
        {
            client_list[i].send_add_device_message = TRUE;
            
            // some clients WILL NOT get updates.
            if(client == &client_list[i])
            {
                continue; // this is the client being added. It needs one.
            }
            
            if(features_device_sleeps(client_list[i].device.features))
            {
                // no update if it sleeps.
                client_list[i].send_add_device_message = FALSE;
            }
            
            // TODO -- what about extended single?  They need to be updated too?
            if(!features_mh_capable(client_list[i].device.features) &&
               !features_block_capable(client_list[i].device.features))
            {
                client_list[i].send_add_device_message = FALSE;
            }
        }
    }

    #ifdef AUTO_SAVE
    save = TRUE;
    #endif
    return ONS_SUCCESS;
} // one_net_master_add_client //


#ifdef PEER
/*!
    \brief (Un)Assigns a peer for a given client.

    Assigns or unassigns a peer unit and device to the client at DST_DID.

    \param[in] ASSIGN TRUE if the peer is being assigned
                      FALSE if the peer is being unassigned
    \param[in] SRC_DID The raw did of the device being (un)assigned the peer.
    \param[in] SRC_UNIT The unit on the device that is having the peer
    \param[in] PEER_DID The raw did of the peer being (un)assigned to the
      client.
    \param[in] PEER_UNIT The unit on the peer device being (un)assigned to the
      client.
      (un)assigned.

    \return ONS_SUCCESS if the operation was successful
            ONS_BAD_PARAM if any of the parameters are invalid
            ONS_INCORRECT_ADDR if either the peer or desination device is not
              part of the network.
            See send_admin_pkt for more return values.
*/
one_net_status_t one_net_master_peer_assignment(const BOOL ASSIGN,
  const on_raw_did_t * const SRC_DID, const UInt8 SRC_UNIT,
  const on_raw_did_t * const PEER_DID, const UInt8 PEER_UNIT)
{
    on_encoded_did_t enc_src_did;
    UInt8 pld[ONA_SINGLE_PACKET_PAYLOAD_LEN - 1];
    on_encoded_did_t* enc_dst_did = (on_encoded_did_t*)&pld[ON_PEER_DID_IDX];
    
    // note -- broadcast peer did indicates wildcard
    BOOL src_is_master, dst_is_master, dst_is_broadcast;
    

    if(!SRC_DID || !PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //
    
    if(on_encode(enc_src_did, *SRC_DID, sizeof(enc_src_did)) != ONS_SUCCESS
      || on_encode(*enc_dst_did, *PEER_DID, ON_ENCODED_DID_LEN)
      != ONS_SUCCESS)
    {
        return ONS_INCORRECT_ADDR;
    } // if the encode failed //
    
    src_is_master = is_my_did((const on_encoded_did_t*) &enc_src_did);
    dst_is_master = is_my_did((const on_encoded_did_t*) enc_dst_did);
    dst_is_broadcast = is_broadcast_did((const on_encoded_did_t*) enc_dst_did);
    
    // make sure that the devices are both part of the network and are
    // not the same device
    if(!src_is_master && !client_info((const on_encoded_did_t*)
      &enc_src_did))
    {
        // source devices is in the network.
        return ONS_INCORRECT_ADDR;
    }
    if(!dst_is_master && !dst_is_broadcast && !client_info(
      (const on_encoded_did_t*) enc_dst_did))
    {
        // dest. device is not part of the network.
        return ONS_INCORRECT_ADDR;
    }
    if(on_encoded_did_equal((const on_encoded_did_t* const) &enc_src_did,
      (const on_encoded_did_t* const) enc_dst_did))
    {
        // devices are the same.
        return ONS_INCORRECT_ADDR;
    }
    
    // one last check -- broadcast dids are valid for unassigning but not
    // assigning
    if(dst_is_broadcast && ASSIGN)
    {
        return ONS_INCORRECT_ADDR;
    }

    
    // first see if we're assigning to ourself, in which case no ONE-NET
    // message will be needed.
    if(src_is_master)
    {
        if(ASSIGN)
        {
            return one_net_add_peer_to_list(SRC_UNIT, NULL,
              (const on_encoded_did_t* const) enc_dst_did, PEER_UNIT);
        }
        else
        {
            return one_net_remove_peer_from_list(SRC_UNIT, NULL,
              (const on_encoded_did_t* const) enc_dst_did, PEER_UNIT);
        }
        
        #ifdef AUTO_SAVE
        save = TRUE;
        #endif        
    }


    pld[ON_PEER_SRC_UNIT_IDX] = SRC_UNIT;
    pld[ON_PEER_PEER_UNIT_IDX] = PEER_UNIT;

    return send_admin_pkt(ASSIGN ? ON_ASSIGN_PEER : ON_UNASSIGN_PEER,
        (const on_encoded_did_t* const) &enc_src_did, pld, 0);
} // one_net_master_peer_assignment //
#endif


/*!
    \brief Changes a CLIENT's keep alive interval.

    \param[in] RAW_DST The CLIENT to update.
    \param[in] KEEP_ALIVE The new keep alive interval (in ms) the CLIENT should
      report at.

    \return ONS_SUCCESS if queueing the transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INCORRECT_ADDR If the address is for a device not in the
              network.
*/
one_net_status_t one_net_master_change_client_keep_alive(
  const on_raw_did_t * const RAW_DST, const UInt32 KEEP_ALIVE)
{
    on_encoded_did_t dst;
    one_net_status_t status;

    UInt8 pld[4];

    if(!RAW_DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = on_encode(dst, *RAW_DST, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(!client_info((const on_encoded_did_t*)&dst))
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

    one_net_uint32_to_byte_stream(KEEP_ALIVE, pld);

    return send_admin_pkt(ON_CHANGE_KEEP_ALIVE, 
      (const on_encoded_did_t * const)&dst, pld, 0);
} // one_net_master_change_client_keep_alive //


#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Changes the fragment delay of a device.

    \param[in] RAW_DST The device to update
    \param[in] LOW_DELAY The fragment delay for low-priority (0 means irrelevant)
    \param[in] HIGH_DELAY The fragment delay for high-priority (0 means irrelevant)
    \param[in] DELAY The new [low/high] fragment delay (in ms)

    \return ONS_SUCCESS if queueing the transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INCORRECT_ADDR If the address is for a device not in the
              network.
            ONS_INVALID_DATA If the data is not valid (such as the high priority
              having a delay longer than the low priority, or vise-versa).
            see ONE-NET status codes for other return values.
*/
one_net_status_t one_net_master_change_frag_dly(
  const on_raw_did_t * const RAW_DST, const UInt16 LOW_DELAY,
  const UInt16 HIGH_DELAY)
{
    on_encoded_did_t dst;
    one_net_status_t status;
    on_client_t* client;
    UInt8 pld[4];
    
    
    if(!RAW_DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(LOW_DELAY != 0 && HIGH_DELAY != 0 && LOW_DELAY < HIGH_DELAY)
    {
        return ONS_INVALID_DATA; // low priority delay cannot be less than
                                 // high priority delay.
    }

    if((status = on_encode(dst, *RAW_DST, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(is_my_did((const on_encoded_did_t * const)&dst))
    {
        // change the MASTER's fragment delay        
        UInt16 new_low = on_base_param->fragment_delay_low;
        UInt16 new_high = on_base_param->fragment_delay_high;

        if(LOW_DELAY != 0)
        {
            new_low = LOW_DELAY;
        }
        if(HIGH_DELAY != 0)
        {
            new_high = HIGH_DELAY;
        }
        
        if(new_low < new_high)
        {
            return ONS_INVALID_DATA; // low priority delay cannot be less than
                                     // high priority delay.
        }
        
        on_base_param->fragment_delay_low = new_low;
        on_base_param->fragment_delay_high = new_high;
        #ifdef AUTO_SAVE
        save = TRUE;
        #endif
        return ONS_SUCCESS;
    } // if the MASTER device //
    
    client = client_info((const on_encoded_did_t*)&dst);

    if(!client)
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //
    
    if(!features_block_capable(client->device.features))
    {
        return ONS_DEVICE_NOT_CAPABLE;
    }

    one_net_uint16_to_byte_stream(LOW_DELAY, &pld[ON_FRAG_LOW_IDX]);
    one_net_uint16_to_byte_stream(HIGH_DELAY, &pld[ON_FRAG_HIGH_IDX]);

    return send_admin_pkt(ON_CHANGE_FRAGMENT_DELAY,
      (const on_encoded_did_t * const)&dst, pld, 0);
} // one_net_master_change_frag_dly //
#endif


/*!
    \brief Sets the MASTER flags in the CLIENT.

    \param[in] client The CLIENT to update.
    \param[in] flags the new flags for the client

    \return ONS_SUCCESS if the command has successfully been queued.
            ONS_BAD_PARAM if any of the parameters are invalid.
            See the ONE-NET status codes for other possibilities for more
            return codes.
*/
one_net_status_t one_net_master_set_flags(on_client_t* client, UInt8 flags)
{
    UInt8 pld[ONA_SINGLE_PACKET_PAYLOAD_LEN - 1];

    if(!client)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //
    
    client->flags = flags;
    pld[0] = client->flags;
    return send_admin_pkt(ON_CHANGE_SETTINGS,
      (const on_encoded_did_t * const)client->device.did, pld, 0);
} // one_net_master_set_flags //


/*!
    \brief Calculate CRC over the master parameters.
    
    \param[in] param pointer to non-volatile parameters.  If NULL,
               on_base_param is used.
    \param[in] peer_param pointer to peer parameters.  If NULL,
               peer is used.
    \return 8-bit CRC of the master parameters if valid
            -1 if invalid
*/
#ifndef PEER
int master_nv_crc(const UInt8* param)
#else
int master_nv_crc(const UInt8* param, const UInt8* peer_param)
#endif
{
    UInt16 starting_crc = ON_PLD_INIT_CRC;
    const UInt8 CRC_LEN = sizeof(UInt8);
    on_master_param_t* mast_param;
    
    #ifdef PEER
    if(!peer_param)
    {
        peer_param = peer_storage;
    }
    #endif
    
    if(!param)
    {
        param = nv_param;
    }
    
    mast_param = (on_master_param_t*) (param + sizeof(on_base_param_t));

    if(mast_param->client_count > ONE_NET_MASTER_MAX_CLIENTS)
    {
        return -1; // error
    }
    
    #ifdef PEER
    // crc over peer parameters
    starting_crc = one_net_compute_crc(peer_param, PEER_STORAGE_SIZE_BYTES,
      starting_crc, ON_PLD_CRC_ORDER);
    #endif
    
    return one_net_compute_crc(&param[CRC_LEN],
      MAX_MASTER_NV_PARAM_SIZE_BYTES - CRC_LEN, starting_crc, ON_PLD_CRC_ORDER);
} // master_nv_crc //


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] CLIENT_DID The encoded device id of the CLIENT to retrieve the
      information for.

    \return The CLIENT information if the information was found
            0 If an error occured.
*/
on_client_t* client_info(const on_encoded_did_t* CLIENT_DID)
{
    UInt16 i;

    if(!CLIENT_DID)
    {
        return 0;
    } // if the parameter is invalid //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(CLIENT_DID,
          (const on_encoded_did_t * const)&client_list[i].device.did))
        {
            return &(client_list[i]);
        } // if the CLIENT was found //
    } // loop to find the CLIENT //

    // check to see if this is a device currently accepting an invite.
    // If it is, then assign it the next DID
    if(is_invite_did(CLIENT_DID))
    {
        return &client_list[master_param->client_count];
    }

    return 0;
} // client_info //


#ifdef BLOCK_MESSAGES_ENABLED
on_nack_rsn_t on_master_get_default_block_transfer_values(
  on_client_t* src_client, on_client_t* dst_client, UInt32 transfer_size,
  UInt8* priority, UInt8* chunk_size, UInt16* frag_delay, UInt16* chunk_delay,
  UInt8* data_rate, UInt8* channel, UInt16* timeout, on_ack_nack_t* ack_nack)
{    
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    ack_nack->handle = ON_ACK;
    
    *timeout = DEFAULT_BLOCK_STREAM_TIMEOUT;
    *chunk_size = DEFAULT_BS_CHUNK_SIZE;
    
    if(src_client == dst_client)
    {
        *nr = ON_NACK_RSN_SENDER_AND_DEST_ARE_SAME;
        return *nr;
    }
    
    *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
    if(src_client)
    {
        if(!features_block_capable(src_client->device.features))
        {
            return *nr;
        }
    }
    if(dst_client)
    {
        if(!features_block_capable(dst_client->device.features))
        {
            return *nr;
        }
    }
    
    *priority = ONE_NET_HIGH_PRIORITY;
    
    *nr =  ON_NACK_RSN_NO_ERROR;
    
    *data_rate = ONE_NET_DATA_RATE_38_4;
    *channel = on_base_param->channel;    
    
    if(transfer_size > ON_SHORT_BLOCK_TRANSFER_MAX_SIZE)
    {
        // if it's <= 2000 bytes, use the base parameters no matter what
        UInt8 src_flags, dst_flags;
        on_features_t src_features, dst_features;
        
        if(src_client)
        {
            src_flags = src_client->flags;
            src_features = src_client->device.features;
        }
        else
        {
            src_flags = master_param->block_stream_flags;
            src_features = THIS_DEVICE_FEATURES;
        }
        if(dst_client)
        {
            dst_flags = dst_client->flags;
            dst_features = dst_client->device.features;
        }
        else
        {
            dst_flags = master_param->block_stream_flags;
            dst_features = THIS_DEVICE_FEATURES;
        }

        if(!(src_flags & ON_BS_ALLOWED) || !(dst_flags & ON_BS_ALLOWED))
        {
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            return *nr;
        }
        
        if(!(src_flags & ON_BS_HIGH_PRIORITY))
        {
            *priority = ONE_NET_LOW_PRIORITY;
        }        
        
        #ifdef DATA_RATE_CHANNEL
        if((src_flags & ON_BS_ELEVATE_DATA_RATE) && (dst_flags & 
          ON_BS_ELEVATE_DATA_RATE))
        {
            *data_rate = features_highest_matching_data_rate(src_features,
              dst_features);
        }

        if((src_flags & ON_BS_CHANGE_CHANNEL) && (dst_flags &
          ON_BS_CHANGE_CHANNEL))
        {
            SInt8 alternate_channel = one_net_get_alternate_channel();
            if(alternate_channel >= 0)
            {
                *channel = (UInt8) alternate_channel;
            }
        }
        #endif
    }
    
    if(!src_client || !dst_client)
    {
        // master is involved, so assign the frag delay.
        *frag_delay = (*priority == ONE_NET_HIGH_PRIORITY) ?
          on_base_param->fragment_delay_high :
          on_base_param->fragment_delay_low;
    }

    *nr = one_net_master_get_default_block_transfer_values(src_client,
      dst_client, transfer_size, priority, chunk_size, frag_delay, chunk_delay,
      data_rate, channel, timeout, ack_nack);
    return *nr;
}


on_nack_rsn_t on_master_initiate_block_msg(block_stream_msg_t* msg,
  on_ack_nack_t* ack_nack)
{
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    ack_nack->handle = ON_ACK;
    *nr = ON_NACK_RSN_NO_ERROR;
    
    if(!msg->dst)
    {
        *nr = ON_NACK_RSN_INTERNAL_ERR;
        return *nr;
    }    
    
    if(bs_msg.transfer_in_progress)
    {
        *nr = ON_NACK_RSN_BUSY;
    }
    else
    {
        on_client_t* client = client_info(
          (const on_encoded_did_t* const) &(msg->dst->did));
        if(!client)
        {
            *nr = ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
        }
        one_net_memmove(&bs_msg, msg, sizeof(block_stream_msg_t));    

        if(!features_block_capable(client->device.features))
        {
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        }
        
        if(!features_data_rate_capable(THIS_DEVICE_FEATURES,
          bs_msg.data_rate) || !features_data_rate_capable(
          client->device.features, bs_msg.data_rate))
        { 
            *nr = ON_NACK_RSN_INVALID_DATA_RATE;
        }
        
        set_bs_transfer_type(&bs_msg.flags, ON_BLK_TRANSFER);
        bs_msg.src = NULL;
        bs_msg.dst = &(client->device);
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


#ifdef STREAM_MESSAGES_ENABLED
on_nack_rsn_t on_master_get_default_stream_transfer_values(
  const on_client_t* src_client, const on_client_t* dst_client, UInt32 time_ms,
  UInt8* priority, UInt16* frag_delay, UInt8* data_rate, UInt8* channel,
  UInt16* timeout, on_ack_nack_t* ack_nack)
{    
    on_nack_rsn_t* nr = &ack_nack->nack_reason;
    ack_nack->handle = ON_ACK;  
    
    *timeout = DEFAULT_BLOCK_STREAM_TIMEOUT;
    
    if(src_client == dst_client)
    {
        *nr = ON_NACK_RSN_SENDER_AND_DEST_ARE_SAME;
        return *nr;
    }
    
    *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
    if(src_client)
    {
        if(!features_stream_capable(src_client->device.features))
        {
            return *nr;
        }
    }
    if(dst_client)
    {
        if(!features_stream_capable(dst_client->device.features))
        {
            return *nr;
        }
    }  
    
    *nr =  ON_NACK_RSN_NO_ERROR;
    *data_rate = ONE_NET_DATA_RATE_38_4;
    *priority = ONE_NET_HIGH_PRIORITY;
    *channel = on_base_param->channel;
    
    if(time_ms > 0 && time_ms < 2000)
    {
        // if it's known and less than 2 seconds, use the base parameters no
        // matter what        
    }
    else
    {
        UInt8 src_flags, dst_flags;
        on_features_t src_features, dst_features;
        
        if(src_client)
        {
            src_flags = src_client->flags;
            src_features = src_client->device.features;
        }
        else
        {
            src_flags = master_param->block_stream_flags;
            src_features = THIS_DEVICE_FEATURES;
        }
        if(dst_client)
        {
            dst_flags = dst_client->flags;
            dst_features = dst_client->device.features;
        }
        else
        {
            dst_flags = master_param->block_stream_flags;
            dst_features = THIS_DEVICE_FEATURES;
        }

        if(!(src_flags & ON_BS_ALLOWED) || !(dst_flags & ON_BS_ALLOWED))
        {
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            return *nr;
        }
        
        if(!(src_flags & ON_BS_HIGH_PRIORITY))
        {
            *priority = ONE_NET_LOW_PRIORITY;
        }
        
        #ifdef DATA_RATE_CHANNEL
        if(!(src_flags & ON_BS_ELEVATE_DATA_RATE) || !(dst_flags & 
          ON_BS_ELEVATE_DATA_RATE))
        {
            *data_rate = features_highest_matching_data_rate(src_features,
              dst_features);
        }
        
        if(!(src_flags & ON_BS_CHANGE_CHANNEL) || !(dst_flags &
          ON_BS_CHANGE_CHANNEL))
        {
            SInt8 alternate_channel = one_net_get_alternate_channel();
            if(alternate_channel >= 0)
            {
                *channel = (UInt8) alternate_channel;
            }
        }
        #endif
    }

    if(!src_client || !dst_client)
    {
        // master is involved, so assign the frag delay.  Default for stream is
        // always high-priority fragment delay regardless of priority
        *frag_delay = on_base_param->fragment_delay_high;
    }

    *nr = one_net_master_get_default_stream_transfer_values(src_client,
      dst_client, time_ms, priority, frag_delay, data_rate, channel, timeout,
      ack_nack);
    return *nr;
}


on_nack_rsn_t on_master_initiate_stream_msg(block_stream_msg_t* msg,
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
        on_client_t* client = client_info(
          (const on_encoded_did_t* const) &(msg->dst->did));
        one_net_memmove(&bs_msg, msg, sizeof(block_stream_msg_t));        
        if(!client)
        {
            *nr = ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
        }
        
        if(!features_stream_capable(client->device.features))
        {
            *nr = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        }
        
        if(!features_data_rate_capable(THIS_DEVICE_FEATURES,
          bs_msg.data_rate) || !features_data_rate_capable(
          client->device.features, bs_msg.data_rate))
        { 
            *nr = ON_NACK_RSN_INVALID_DATA_RATE;
        }
        
        set_bs_transfer_type(&bs_msg.flags, ON_STREAM_TRANSFER);
        bs_msg.src = NULL;
        bs_msg.dst = &(client->device);
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



//! @} ONE-NET_MASTER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
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
static on_message_status_t on_master_single_data_hdlr(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    BOOL stay_awake;
    on_message_status_t msg_status;
    on_msg_hdr_t msg_hdr;
    on_raw_did_t raw_src_did, raw_repeater_did;
    UInt8 response_pid;
    on_sending_device_t* device;
    on_client_t* client;
    client = client_info((const on_encoded_did_t* const)
      &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]));

    on_decode(raw_src_did, &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
      ON_ENCODED_DID_LEN);
    on_decode(raw_repeater_did, &(pkt->packet_bytes[ON_ENCODED_RPTR_DID_IDX]),
      ON_ENCODED_DID_LEN);
    
    msg_hdr.msg_type = *msg_type;
    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    
    // we'll be sending it back to the source
    if(!(device = sender_info((const on_encoded_did_t* const)
      &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]))))
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
        goto omsdh_build_resp;
    }
    
    switch(*msg_type)
    {
        case ON_ADMIN_MSG:
            msg_status = handle_admin_pkt((const on_encoded_did_t* const)
              &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
              &raw_pld[ON_PLD_DATA_IDX], &client, ack_nack);
            break;
        #ifdef ROUTE
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
            #ifndef ONE_NET_MULTI_HOP
            msg_status = one_net_master_handle_single_pkt(
              &raw_pld[ON_PLD_DATA_IDX], &msg_hdr, (const on_raw_did_t* const)
              &raw_src_did, (const on_raw_did_t* const) &raw_repeater_did,
              ack_nack);
            #else
            msg_status = one_net_master_handle_single_pkt(
              &raw_pld[ON_PLD_DATA_IDX], &msg_hdr, (const on_raw_did_t* const)
              &raw_src_did, (const on_raw_did_t* const) &raw_repeater_did,
              ack_nack, (*txn)->hops, &((*txn)->max_hops));
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
    if(ack_nack->handle == ON_ACK_APP_MSG && get_msg_class(
      ack_nack->payload->app_msg) == ONA_STATUS_QUERY_RESP)
    {
        one_net_master_send_single(ONE_NET_RAW_SINGLE_DATA, ON_APP_MSG,
            ack_nack->payload->app_msg, ONA_SINGLE_PACKET_PAYLOAD_LEN,
            ONE_NET_HIGH_PRIORITY, NULL,
            (const on_encoded_did_t* const)
            &(pkt->packet_bytes[ON_ENCODED_SRC_DID_IDX])
        #ifdef PEER
            , FALSE,
            get_src_unit(ack_nack->payload->app_msg)
        #endif
            , 0
        #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	        , 0
        #endif
        );
        
        // now change the handle to ON_ACK
        ack_nack->handle = ON_ACK;
    }
    

// normally we try not to use goto statements but this is embedded programming
// and it may save us a few bytes?
omsdh_build_resp:
    stay_awake = FALSE;
    if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
    {
        // device has checked in, so reset the next check-in time
        // give it 5 extra seconds.
        client->next_check_in_time = get_tick_count() +
          MS_TO_TICK(5000 + client->keep_alive_interval);
        stay_awake = one_net_master_device_is_awake(FALSE,
          (const on_raw_did_t * const)&raw_src_did);
    }
    
    stay_awake = stay_awake || device_should_stay_awake(
      (const on_encoded_did_t* const) &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]));
      
    // now check to make sure we're not in the middle of a key change
    if(!(client->use_current_key))
    {
        stay_awake = TRUE;
        // TODO -- actually queue the key change?
    }


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

    response_txn.key = (*txn)->key;
    #ifdef ONE_NET_MULTI_HOP
    response_txn.hops = (*txn)->hops;
    response_txn.max_hops = (*txn)->max_hops;
    #endif
    *txn = &response_txn;

    if(on_build_response_pkt(ack_nack, &response_pkt_ptrs, *txn, FALSE) !=
      ONS_SUCCESS)
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
static on_message_status_t on_master_handle_single_ack_nack_response(
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
    
    #ifndef ONE_NET_MULTI_HOP
    status = one_net_master_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, &src_did, NULL, &(txn->retry));
    #else
    status = one_net_master_handle_ack_nack_response(raw_pld, &msg_hdr, NULL,
      ack_nack, (const on_raw_did_t* const) &src_did, NULL, &(txn->retry),
      pkt->hops, &(pkt->max_hops));
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
            #ifdef COMPILE_WO_WARNINGS
            // add default case that does nothing for clean compile
            default:
                break;
            #endif
        }
        
        if(new_response_timeout > 0)
        {
            txn->response_timeout = (UInt16)new_response_timeout;
        }
        
        if(txn->retry >= ON_MAX_RETRY)
        {
            #ifdef ONE_NET_MULTI_HOP
            // we may be able to re-send with a higher max hops if there are
            // any multi-hop repeaters.  If there are repeaters out there
            // (and the repeater isn't the device we're sending to), we'll
            // give it a shot
            SInt8 num_repeat = (SInt8) on_base_param->num_mh_repeaters;
            if(features_known(txn->device->features) &&
              features_mh_repeat_capable(txn->device->features))
            {
                num_repeat--; // don't count the destination as a repeater.
            }
            
            if(num_repeat > 0 && txn->max_hops < txn->device->max_hops)
            {
                // we have repeaters available, so we'll give it a shot
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
                switch(one_net_adjust_hops((const on_raw_did_t* const) &raw_did,
                  &txn->max_hops))
                {
                    case ON_MSG_ABORT: return ON_MSG_ABORT;
                    #ifdef COMPILE_WO_WARNINGS
                    // add default case that does nothing for clean compile
                    default: break;
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
    \brief Handles the end of an admin transaction.

    \param[in] raw_pld The raw payload of the packet that was sent.
    \param[in] raw_did The raw did of the CLIENT the packet was sent to.
    \param[in] ack_nack The CLIENT the pkt was sent to.
    \param[out] client the client that was the recipient of this packet.
*/
static void admin_txn_hdlr(const UInt8* const raw_pld,
  const on_raw_did_t* const raw_did, const on_ack_nack_t* const ack_nack,
  on_client_t* client)
{
    UInt8 admin_type = raw_pld[0];
    one_net_mac_update_t update = ONE_NET_UPDATE_NOTHING;
    on_encoded_did_t enc_did;
    on_encode(enc_did, *raw_did, ON_ENCODED_DID_LEN);
    
    switch(admin_type)
    {
#ifdef PEER
        case ON_ASSIGN_PEER:
        {
            update = ONE_NET_UPDATE_ASSIGN_PEER;
            break;
        } // assign peer case //

        case ON_UNASSIGN_PEER:
        {
            update = ONE_NET_UPDATE_UNASSIGN_PEER;
            break;
        } // unassign peer case //
#endif

        case ON_CHANGE_KEEP_ALIVE:
        {
            if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
            {
                client->keep_alive_interval = ack_nack->payload->ack_time_ms;
            }           
            update = ONE_NET_UPDATE_KEEP_ALIVE;
            break;
        } // change keep-alive case //
        
        #ifdef BLOCK_MESSAGES_ENABLED
        case ONE_NET_UPDATE_FRAGMENT_DELAY:
        {
            update = ONE_NET_UPDATE_FRAGMENT_DELAY;
            break;
        } // change one or both fragment delays case //
        #endif

        case ON_CHANGE_SETTINGS:
        {
            update = ONE_NET_UPDATE_SETTINGS;
            break;
        } // change keep-alive case //
        
        case ON_ADD_DEV:
        {
            if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
            {
                client->send_add_device_message = FALSE;
                #ifdef AUTO_SAVE
                save = TRUE;
                #endif
            }
            update = ONE_NET_UPDATE_ADD_DEVICE;
            break;
        } // add device case //
        
        case ON_RM_DEV:
        {
            if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
            {
                client->send_remove_device_message = FALSE;
                #ifdef AUTO_SAVE
                save = TRUE;
                #endif
            }
            update = ONE_NET_UPDATE_REMOVE_DEVICE;
            break;
        } // remove device case //
        
        case ON_NEW_KEY_FRAGMENT:
        {
            if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
            {
                if(one_net_memcmp(&(on_base_param->current_key[3 *
                  ONE_NET_XTEA_KEY_FRAGMENT_SIZE]), ack_nack->payload->key_frag,
                  ONE_NET_XTEA_KEY_FRAGMENT_SIZE) == 0)
                {
                    client->use_current_key = TRUE;
                    #ifdef AUTO_SAVE
                    save = TRUE;
                    #endif
                }
                update = ONE_NET_UPDATE_NETWORK_KEY;
            }
            break;
        }

        default: return;
    }
    
    one_net_master_update_result(update, raw_did, ack_nack);
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
static on_message_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    on_msg_hdr_t msg_hdr;
    on_raw_did_t dst;
    on_client_t* client = client_info((const on_encoded_did_t* const)
      &(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]));
    
    if(!client)
    {
        return ON_MSG_INTERNAL_ERR;
    }
    

    msg_hdr.raw_pid = pkt->raw_pid;
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;
    on_decode(dst ,&(pkt->packet_bytes[ON_ENCODED_DST_DID_IDX]),
      ON_ENCODED_DID_LEN);
    
    
    if(*msg_type == ON_ADMIN_MSG)
    {
        admin_txn_hdlr(raw_pld, (const on_raw_did_t* const) &dst,
          ack_nack, client);
    }

    #ifndef ONE_NET_MULTI_HOP
    one_net_master_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, (const on_raw_did_t*) &dst, ack_nack);
    #else
    one_net_master_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, (const on_raw_did_t*) &dst, ack_nack, pkt->hops);
    #endif
    
    #if defined(BLOCK_MESSAGES_ENABLED) && defined(ONE_NET_MULTI_HOP)
    if(bs_msg.transfer_in_progress)
    {
        ack_nack->handle = ON_ACK;
        if(*msg_type == ON_ROUTE_MSG)
        {
            UInt8 hops, return_hops;
            if(ack_nack->nack_reason ||
              !extract_repeaters_and_hops_from_route(
              (const on_encoded_did_t* const) &(bs_msg.dst->did),
              ack_nack->payload->ack_payload, &hops,
              &return_hops, &bs_msg.num_repeaters, bs_msg.repeaters))
            {
                on_message_status_t status = ON_MSG_FAIL;
                
                // route failed for some reason.
                ack_nack->nack_reason = ON_NACK_RSN_ROUTE_ERROR;
                #ifndef STREAM_MESSAGES_ENABLED
                on_master_block_txn_hdlr(&bs_msg, NULL, &status, ack_nack);
                #else
                if(get_bs_transfer_type(bs_msg.flags) == ON_BLK_TRANSFER)
                {
                    on_master_block_txn_hdlr(&bs_msg, NULL, &status,
                      ack_nack);
                }
                else
                {
                    on_master_stream_txn_hdlr(&bs_msg, NULL, &status,
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
    
    if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
    {
        // device has checked in, so reset the next check-in time
        // give it 5 extra seconds.
        client->next_check_in_time = get_tick_count() +
          MS_TO_TICK(5000 + client->keep_alive_interval);
        one_net_master_device_is_awake(TRUE,
          (const on_raw_did_t * const)&dst);
    }
    
    return ON_MSG_DEFAULT_BHVR;
}




#ifdef BLOCK_MESSAGES_ENABLED
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
static on_message_status_t on_master_block_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* block_pkt, on_ack_nack_t* ack_nack)
{
    return one_net_master_handle_block_pkt(txn, bs_msg, (block_pkt_t*)
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
static on_message_status_t on_master_handle_block_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return one_net_master_handle_bs_ack_nack_response(txn, bs_msg, pkt,
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
static on_message_status_t on_master_block_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack)
{
    return one_net_master_block_txn_status(msg, terminating_device, status,
      ack_nack);
}
#endif




#ifdef STREAM_MESSAGES_ENABLED
/*!
    \brief Handles a stream data packet

    \param[in/out] txn The stream transaction being carried out
    \param[in/out] bs_msg The stream message in progress
    \param[in] stream_pkt The data packet, including both the payload and some administration information. 
    \param[out] ack_nack The response that should be sent to the sending device
    
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/ 
static on_message_status_t on_master_stream_data_hdlr(on_txn_t* txn,
  block_stream_msg_t* bs_msg, void* stream_pkt, on_ack_nack_t* ack_nack)
{
    return one_net_master_handle_stream_pkt(txn, bs_msg, (stream_pkt_t*)
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
static on_message_status_t on_master_handle_stream_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return one_net_master_handle_bs_ack_nack_response(txn, bs_msg, pkt,
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
static on_message_status_t on_master_stream_txn_hdlr(
  const block_stream_msg_t* msg, const on_encoded_did_t* terminating_device,
  on_message_status_t* status, on_ack_nack_t* ack_nack)
{
    return one_net_master_stream_txn_status(msg, terminating_device, status,
      ack_nack);
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
    one_net_master_cancel_invite((const one_net_xtea_key_t* const) &invite_key);
    invite_txn.pkt = &encoded_pkt_bytes[2 * ON_MAX_ENCODED_DATA_PKT_SIZE];
    pkt_hdlr.single_data_hdlr = &on_master_single_data_hdlr;
    pkt_hdlr.single_ack_nack_hdlr =
      &on_master_handle_single_ack_nack_response;
    pkt_hdlr.single_txn_hdlr = &on_master_single_txn_hdlr;
    pkt_hdlr.adj_recip_list_hdlr = &on_master_adjust_recipient_list;
    
    #ifdef BLOCK_MESSAGES_ENABLED
    pkt_hdlr.block_data_hdlr = &on_master_block_data_hdlr;
    pkt_hdlr.block_ack_nack_hdlr =
      &on_master_handle_block_ack_nack_response;
    pkt_hdlr.block_txn_hdlr = &on_master_block_txn_hdlr;
    #endif
    
    #ifdef STREAM_MESSAGES_ENABLED
    pkt_hdlr.stream_data_hdlr = &on_master_stream_data_hdlr;
    pkt_hdlr.stream_ack_nack_hdlr =
      &on_master_handle_stream_ack_nack_response;
    pkt_hdlr.stream_txn_hdlr = &on_master_stream_txn_hdlr;
    #endif    

    get_sender_info = &sender_info;
    device_is_master = TRUE;
    one_net_init();
    
    #ifdef BLOCK_MESSAGES_ENABLED
    #ifdef DATA_RATE_CHANNEL
    master_param->block_stream_flags |= (ONE_NET_MASTER_MASTER_BLOCK_STREAM_ELEVATE_DATA_RATE ?
      ON_BS_ELEVATE_DATA_RATE : 0); 
    #endif
    master_param->block_stream_flags |= (ONE_NET_MASTER_MASTER_BLOCK_STREAM_CHANGE_CHANNEL ?
      ON_BS_CHANGE_CHANNEL : 0); 
    master_param->block_stream_flags |= (ONE_NET_MASTER_MASTER_BLOCK_STREAM_HIGH_PRIORITY ?
      ON_BS_HIGH_PRIORITY : 0); 
    master_param->block_stream_flags |= (ONE_NET_MASTER_MASTER_ALLOW_LONG_BLOCK_STREAM ?
      ON_BS_ALLOWED : 0); 
    #endif    
    
    return ONS_SUCCESS;
} // init_internal //


/*!
    \brief Removes the CLIENT from the network (by removing it from the table) 

    \param[in] DID The device ID of the CLIENT to remove

    \return ONS_SUCCESS if the client was deleted.
            ONS_NOT_JOINED if the client to delete is not in the network
            ONS_INVALID_DATA if the client was not deleted.
            ONS_INTERNAL_ERR if some other problem occurred
*/
static one_net_status_t rm_client(const on_encoded_did_t * const DID)
{
    on_client_t* client;
    
    if(!DID)
    {
        return ONS_INVALID_DATA;
    } // if the parameter is invalid //

    client = client_info(DID);
    if(!client)
    {
        return ONS_NOT_JOINED; // there's no client to delete.  This may be an
                               // error or it may just be that the device has
                               // already been deleted.
    }
    
    
    // make this slot vacant
    one_net_memmove(client->device.did, ON_ENCODED_BROADCAST_DID,
      ON_ENCODED_DID_LEN);
      
    // subtract 1 from the count
    (master_param->client_count)--;
    
    // now fill in the next client did to assign
    master_param->next_client_did = find_lowest_vacant_did();
    #ifdef AUTO_SAVE
    save = TRUE;
    #endif
	return ONS_SUCCESS;
} // rm_client //


// TODO -- June 2, 2012 - sort_client_list_by_encoded_did appears to not be
// used anywhere.  Commenting out for a clean compile
#if 0
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
        on_decode(raw_did1, client_list[i-1].device.did,
          ON_ENCODED_DID_LEN);
        on_decode(raw_did2, client_list[i].device.did,
          ON_ENCODED_DID_LEN);
          
        if(one_net_byte_stream_to_uint16(raw_did1) >
          one_net_byte_stream_to_uint16(raw_did2))
        {
            // swap.
            on_client_t temp = client_list[i-1];
            client_list[i-1] = client_list[i];
            client_list[i] = temp;
            i = 0; // make it 0, not 1 because the loop increments i.
        }
    }
}
#endif


/*!
    \brief Find the lowest vacant did that can be assigned to the next client.

    \return the next available client did that can be assigned in raw form
            stored as a UInt16 (i.e. 0020, 0030, 0040, etc.).  Returns 0
            if the list is full.
*/
static UInt16 find_lowest_vacant_did(void)
{
    UInt16 i;
    BOOL found;
    on_raw_did_t raw_did;
    UInt16 vacant_did = ONE_NET_INITIAL_CLIENT_DID;
    
    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return 0; // list is full.
    }
    
    do
    {
        found = FALSE;
        for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
        {
            on_decode(raw_did, client_list[i].device.did, ON_ENCODED_DID_LEN);
            if(one_net_byte_stream_to_uint16(raw_did) == vacant_did)
            {
                // this did is already taken, so increment
                vacant_did += ON_CLIENT_DID_INCREMENT;
                found = TRUE;
            }
        }
    } while(found);
    
    return vacant_did;
}


/*!
    \brief Finds the lowest unused index in the client list.

    \return the lowest unused index in the client list.
            -1 if the list is full.
*/
static SInt16 find_vacant_client_list_index(void)
{
    UInt16 i;
    for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
    {
        if(is_broadcast_did((const on_encoded_did_t*)
          client_list[i].device.did))
        {
            return i;
        }
    }
    
    return -1;
}


/*!
    \brief Finds the sender info.

    Finds the sender info.

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
    
    return &(client->device);
}


static void check_updates_in_progress(void)
{
    static tick_t last_send_time = 0;
    tick_t time_now = get_tick_count();
    const tick_t SEND_INTERVAL = MS_TO_TICK(5000);
    
    // The time to stop trying to update any devices which have not been updated
    // TODO -- should this be a port constant?  Should it exist at all?
    // Note -- Does NOT apply to devices that sleep with key changes
    // And should this value be defined locally or globally.
    // located here?
    const tick_t UPDATE_TIME_LIMIT = MS_TO_TICK(60000);

    UInt16 i;
    UInt8 admin_payload[4];
    on_ack_nack_t ack;
    UInt8 admin_msg_id = 0xFF; // garbage argument.  Will be written over if
                               // any real message is to be sent.

    ack.nack_reason = ON_NACK_RSN_NO_ERROR;

    // now go through the update types and see if any messages need sending
    if(remove_device_update_in_progress)
    {
        if(time_now > remove_device_start_time + UPDATE_TIME_LIMIT)
        {
            // time's up.  Set everyone's flag to "sent" even if they have not
            // sent.  This includes devices that sleep.
            for(i = 0; i < master_param->client_count; i++)
            {
                client_list[i].send_remove_device_message = FALSE;
            }
        }        
        
        remove_device_update_in_progress = FALSE;
        
        // now check if we're done with this update.
        for(i = 0; i < master_param->client_count; i++)
        {
            if(client_list[i].send_remove_device_message)
            {
                admin_msg_id = ON_RM_DEV;
                admin_payload[0] = remove_device_did[0];
                admin_payload[1] = remove_device_did[1];
                #ifdef ONE_NET_MULTI_HOP
                admin_payload[2] = on_base_param->num_mh_devices;
                admin_payload[3] = on_base_param->num_mh_repeaters;
                #else
                // TODO -- should we ban multi-hop just because the master
                // isn't capable.
                admin_payload[2] = 0;
                admin_payload[3] = 0;
                #endif
                remove_device_update_in_progress = TRUE;
                break; // we have one.
            }
        }
        
        if(!remove_device_update_in_progress)
        {
            // we don't have any more updates for this, so notify the
            // application code
            one_net_master_update_result(ONE_NET_UPDATE_REMOVE_DEVICE, NULL,
              &ack);
            // now actually remove the client
            rm_client((const on_encoded_did_t* const) &remove_device_did);
            return;
        }
    }

    else if(add_device_update_in_progress)
    {
        if(time_now > add_device_start_time + UPDATE_TIME_LIMIT)
        {
            // time's up.  Set everyone's flag to "sent" even if they have not
            // sent.  This includes devices that sleep.
            for(i = 0; i < master_param->client_count; i++)
            {
                client_list[i].send_add_device_message = FALSE;
            }
        }
        
        
        add_device_update_in_progress = FALSE;
        
        // now check if we're done with this update.
        for(i = 0; i < master_param->client_count; i++)
        {
            if(client_list[i].send_add_device_message)
            {
                admin_msg_id = ON_ADD_DEV;
                admin_payload[0] = add_device_did[0];
                admin_payload[1] = add_device_did[1];
                #ifdef ONE_NET_MULTI_HOP
                admin_payload[2] = on_base_param->num_mh_devices;
                admin_payload[3] = on_base_param->num_mh_repeaters;
                #else
                // TODO -- should we ban multi-hop just because the master
                // isn't capable.
                admin_payload[2] = 0;
                admin_payload[3] = 0;
                #endif
                add_device_update_in_progress = TRUE;
                break; // we have one.
            }
        }

        if(!add_device_update_in_progress)
        {
            // we don't have any more updates for this, so notify the
            // application code
            one_net_master_update_result(ONE_NET_UPDATE_ADD_DEVICE, NULL,
              &ack);
            one_net_memmove(add_device_did, ON_ENCODED_BROADCAST_DID,
              ON_ENCODED_DID_LEN);
            return;
        }
    }

    else if(key_update_in_progress)
    {
        key_change_requested = FALSE; // already changing it.  No need to
                                      // flag it to change again.
        key_update_in_progress = FALSE;

        // now check if we're done with this update.
        for(i = 0; i < master_param->client_count; i++)
        {
            if(!client_list[i].use_current_key)
            {
                key_update_in_progress = TRUE; // at least one device has not
                                               // been updated
                                               
                // if it doesn't sleep, send it out.  Otherwise we'll have to
                // wait till it checks in.                
                if(!features_device_sleeps(client_list[i].device.features))
                {
                    admin_msg_id = ON_NEW_KEY_FRAGMENT;
                    one_net_memmove(admin_payload, &(on_base_param->current_key[
                      3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                      ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
                    break; // we have an update to send.
                }
            }
        }

        if(!key_update_in_progress)
        {
            // we don't have any more updates for this, so notify the
            // application code
            one_net_master_update_result(ONE_NET_UPDATE_NETWORK_KEY, NULL,
              &ack);
            return;
        }
    }
    
    else if(key_change_requested)
    {
        UInt32 rand_num;
        one_net_xtea_key_fragment_t key_fragment;
        while(!key_update_in_progress)
        {
            rand_num = one_net_prand(get_tick_count(),
              0xFFFFFFFE);
            one_net_memmove(key_fragment, &rand_num,
              ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
            one_net_master_change_key_fragment(key_fragment);
        }
        return;
    }


    if(time_now < last_send_time + SEND_INTERVAL || admin_msg_id == 0xFF)
    {
        return; // nothing is being updated.
    }
    
    // don't worry about filling in any destination addresses.  This message's
    // recipient list will be adjusted when the message is actually popped.
    if(send_admin_pkt(admin_msg_id, NULL, admin_payload,
      MS_TO_TICK(2500)) == ONS_SUCCESS)
    {
        last_send_time = time_now;
    }
}


/*!
    \brief Sends an admin packet (single transaction).

    Sets up an admin packet and queues the transaction to send it.

    \param[in] admin_msg_id the type of admin message being sent.
    \param[in] did The device to send to.  NULL means broadcast.
    \param[in] pld The  admin data to be sent.
    \param[in] send_time_from_now The time to pause before sending.

    \return ONS_SUCCESS If the packet was built and queued successfully
            ONS_RSRC_FULL If there are no resources available
*/
static one_net_status_t send_admin_pkt(const UInt8 admin_msg_id,
  const on_encoded_did_t* const did, const UInt8* const pld,
  tick_t send_time_from_now)
{
    UInt8 admin_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];
    admin_pld[0] = admin_msg_id;
    one_net_memmove(&admin_pld[1], pld, ONA_SINGLE_PACKET_PAYLOAD_LEN - 1);

    if(one_net_master_send_single(ONE_NET_RAW_SINGLE_DATA, ON_ADMIN_MSG,
      admin_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN, ONE_NET_LOW_PRIORITY,
      NULL, did
      #ifdef PEER
      , FALSE,
      ONE_NET_DEV_UNIT
      #endif
      , send_time_from_now
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , 0
      #endif
      ))
    {
        return ONS_SUCCESS;
    }
    else
    {
        return ONS_RSRC_FULL;
    }
} // send_admin_pkt //


/*!
    \brief Handles admin packets.

    \param[in] SRC The sender of the admin packet.
    \param[in] DATA The admin packet.
    \param[in/out] client The CLIENT the message is from if the CLIENT is
      already part of the network.  If the CLIENT is not yet part of the
      network and it is a new CLIENT being added to the network, the new CLIENT
      will be returned in client.
    \param[out] acknowledgement or negative acknowledgement of a message

    \return ON_MSG_CONTINUE if processing should continue
            ON_MSG_IGNORE if the message should be ignored
*/
static on_message_status_t handle_admin_pkt(const on_encoded_did_t * const
  SRC_DID, const UInt8 * const DATA, on_client_t ** client,
  on_ack_nack_t* ack_nack)
{
    on_raw_did_t raw_did;

    // TODO -- we need some named constants
    if(!SRC_DID || !DATA || !client || (DATA[1] != ON_FEATURES_RESP &&
      !(*client)))
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    on_decode(raw_did, *SRC_DID, ON_ENCODED_DID_LEN);
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;


    switch(DATA[0])
    {
        #ifdef DATA_RATE_CHANNEL
        case ON_CHANGE_DATA_RATE_CHANNEL:
        {
            UInt16 pause_time_ms = one_net_byte_stream_to_uint16(&DATA[3]);
            UInt16 dormant_time_ms = one_net_byte_stream_to_uint16(&DATA[5]);
            ack_nack->nack_reason = on_change_dr_channel(NULL,
              pause_time_ms, dormant_time_ms, DATA[1], DATA[2]);
            break;
        }
        #endif
        
        #ifdef BLOCK_MESSAGES_ENABLED
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
            #ifdef STREAM_MESSAGES_ENABLED
            UInt8 is_block_transfer = (get_bs_transfer_type(
              DATA[BLOCK_STREAM_SETUP_FLAGS_IDX]) == ON_BLK_TRANSFER);
            #endif
            const on_encoded_did_t* dst_did = (const on_encoded_did_t*)
              &DATA[BLOCK_STREAM_SETUP_DST_IDX];
            on_client_t* recipient = client_info(dst_did);
            BOOL master_is_recipient = (is_my_did(dst_did));
            block_stream_msg_t proposed_msg;
            block_stream_msg_t* bs_ptr = &bs_msg;
              
            if(!master_is_recipient)
            {
                bs_ptr = &proposed_msg; // we don't want to write our own
                                        // message, if any.
                if(!recipient)
                {
                    ack_nack->nack_reason = ON_NACK_RSN_BAD_ADDRESS_ERR;
                    break;
                }
            }
            
            // Check if we are already in the middle of a block transaction.  If
            // we are and the source is NOT the sending device of this message,
            // NACK it.  If the source IS the sending device of this message and
            // we're still in the setup stage, we're OK.  Otherwise, NACK it.            
            if(master_is_recipient && bs_msg.transfer_in_progress)
            {
                if(!bs_msg.src || !on_encoded_did_equal(
                  (const on_encoded_did_t* const) &(bs_msg.src->did), SRC_DID))
                {
                    ack_nack->nack_reason = ON_NACK_RSN_BUSY;
                    break;
                }
                
                #ifdef STREAM_MESSAGES_ENABLED
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
            
            admin_msg_to_block_stream_msg_t(&DATA[0], bs_ptr,
              (const on_encoded_did_t*) (*client)->device.did);
            
            if(!ack_nack->nack_reason)
            {
                one_net_block_stream_transfer_requested(bs_ptr, ack_nack);
            }
            
            // we could check other things like features, but the client
            // device will check that anyway.
            
            if(master_is_recipient && !ack_nack->nack_reason)
            {
                #ifdef STREAM_MESSAGES_ENABLED
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
            }
            break;
        }
        #ifdef ONE_NET_MULTI_HOP
        case ON_REQUEST_REPEATER:
        {
            UInt32 estimated_time = one_net_byte_stream_to_uint32(&DATA[7]);
            on_client_t* rptr_client = client_info(
              (const on_encoded_did_t*) &DATA[1]);
            on_client_t* src_client = client_info(
              (const on_encoded_did_t*) &DATA[3]);
            on_client_t* dst_client = client_info(
              (const on_encoded_did_t*) &DATA[5]);
            on_encoded_did_t* invalid_device_did = NULL;
            
            // we need to make sure the source and the destinations are clients
            // in the network and are different and that the repeater is
            // capable.  The destination will eiher be this device, the master,
            // in which case we need to have already set up our block / stream
            // meassage and we are the recipient.  If it's not us, we also want
            // to make sure everyone involved is capable.
            
            // TODO -- there are more things to check too.
            
            if(!rptr_client)
            {
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
                invalid_device_did = (on_encoded_did_t*) &DATA[1];
            }
            else if(!features_mh_repeat_capable(rptr_client->device.features))
            {
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
                invalid_device_did = (on_encoded_did_t*) &DATA[1];
            }
            else if(!src_client)
            {
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
                invalid_device_did = (on_encoded_did_t*) &DATA[3];
            }
            else if(!features_mh_capable(src_client->device.features))
            {
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
                invalid_device_did = (on_encoded_did_t*) &DATA[3];
            }
            else if(!dst_client)
            {
                invalid_device_did = (on_encoded_did_t*) &DATA[5];
                if(is_master_did((const on_encoded_did_t* const)
                  invalid_device_did))
                {
                    // this is to us.
                    if(!bs_msg.transfer_in_progress)
                    {
                        // they should have set up the block transfer with us
                        // prior to asking us for permission for a repeater.
                        // We will deny permission, but make it non-fatal since
                        // there may have been some mistake on the requesting
                        // side.  We are not denying this request, but merely
                        // NACKing it because it may have come in the wrong
                        // wrong order.  The sending device, getting this error,
                        // can hopefully re-request everything in the correct
                        // order and that may allow approval.  We can't approve
                        // this because the request makes no sense.
                        ack_nack->nack_reason =
                          ON_NACK_RSN_NOT_ALREADY_IN_PROGRESS;
                    }
                    else
                    {
                        // make sure the source and destinations match what we
                        // think they should.  If they don't, NACK with an
                        // "already in progress" reason.
                        if(&(src_client->device) != bs_msg.src)
                        {
                            ack_nack->nack_reason =
                              ON_NACK_RSN_ALREADY_IN_PROGRESS;
                        }
                    }
                }
                else
                {
                    ack_nack->nack_reason = ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
                }
            }
            else if(!features_mh_capable(dst_client->device.features))
            {
                ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            }
            
            if(ack_nack->nack_reason)
            {
                ack_nack->handle = ON_NACK_DATA;
                one_net_memmove(ack_nack->payload->nack_payload,
                  *invalid_device_did, ON_ENCODED_DID_LEN);
                break;
            }
            
            // So far, so good, now inform / ask the application code and give
            // it a chance to veto this request.
            one_net_master_repeater_requested(src_client, dst_client,
              rptr_client, DATA[11], DATA[12], DATA[13], estimated_time,
              ack_nack);
            break;
        }
        #endif
        #endif
        
        case ON_REQUEST_KEY_CHANGE:
        {
            if(!key_update_in_progress)
            {
                key_change_requested = TRUE;
            }
            break;
        }
        
        case ON_ADD_DEV_RESP:
        {
            // this is sent when a client has received a message that a device
            // has been added.
            if((*client)->send_add_device_message)
            {
                one_net_master_update_result(ONE_NET_UPDATE_ADD_DEVICE,
                  (const on_raw_did_t*) &raw_did, ack_nack);
            }
            
            (*client)->send_add_device_message = FALSE;
            #ifdef AUTO_SAVE
            save = TRUE;
            #endif
            break;
        }
        
        case ON_REMOVE_DEV_RESP:
        {
            // this is sent when a client has received a message that a device
            // has been removed.
            if((*client)->send_remove_device_message)
            {
                one_net_master_update_result(ONE_NET_UPDATE_REMOVE_DEVICE,
                  (const on_raw_did_t*) &raw_did, ack_nack);
            }
            
            (*client)->send_remove_device_message = FALSE;
            #ifdef AUTO_SAVE
            save = TRUE;
            #endif
            break;
        }
        
        case ON_CHANGE_SETTINGS_RESP:
        {
            if(is_invite_did(SRC_DID))
            {
                settings_sent = TRUE;
            }
            break;
        }
        
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_CHANGE_FRAGMENT_DELAY_RESP:
        {
            if(is_invite_did(SRC_DID))
            {
                fragment_delay_sent = TRUE;
            }
            break;
        }
        #endif
        
        case ON_KEEP_ALIVE_RESP:
        {
            // Several things need to be considered here.
            // One, we could be in the middle of an event that requires that
            // we update all devices.  In particular, we could have added a
            // device, we could have removed a device, or we could have
            // changed a key.  It would be nice to simply send back an ACK
            // with the update message, so that's what we'll do.  We can never
            // assume, however, that the client will get this ACK (more below).
            // It's not a big problem if the device gets extra
            // "device removed" or "device added" messages.  It is a potential
            // problem if it gets multiple key fragment changes and tries to
            // change it more than once.  To rectify that problem, ONE-NET
            // stipulates that a new key fragmet cannot match any of the 4
            // existing key fragments.  Before a client changes key fragments,
            // it compares the new fragment to what it has now.  If there's a
            // match, it already has the new key fragment and there was a
            // a missed ACK or the timing synchronization was off, or something
            // else occurred.  In any case, in this case it will not replace
            // the key fragment.
            
            // Every time a Keep-Alive message is sent, the last key fragment
            // is sent.  The master compares the key fragments to make sure
            // the client has the current key.  If it does not, an admin
            // message will be sent back in the ACK containing the new key
            // fragment.  Upon receiving this message, the client should
            // send another keep-alive message containing the current key.
            // The master used this to keep track of when a key change is
            // complete.
            
            // If the device has the right key, the master then makes sure it
            // is informed of any pending additions / removals of devices.
            // when the client is informed of such an occurrence, it will
            // send either an ON_ADD_DEV_RESP message or an ON_REMOVE_DEV_RESP
            // message as confirmation, at which point the master will add the
            // client to the list of devices that have been informed.
            // The client should then send another keep-alive reponse message.
            
            
            // In addition, the device might be in the middle of adding itself
            // to the network.  In that case, it needs to be informed of the
            // following...
            
            // 1. Fragment delays (if block enabled)
            // 2. Settings / Flags

            // For a device just joining, the master's features and the key
            // were included in the invite message.  The keep-alive time is
            // not known yet, but will be known with the first keep-alive
            // message AFTER the device has been fully added.
            
            
            // If there are no more administrative tasks to finish up, the
            // master will send back a new keep-alive interval.  Then and only
            // then may a client go to sleep.  This is regardless of whether
            // the ACK or NACK sent back was of the "keep awake" type.  A
            // client has fulfilled its obligation to check in with the master
            // only when it has received this new keep-alive time.  That
            // signifies that the master has no more messages for this client.
            on_encoded_did_t* device_change_did = NULL;
            
            ack_nack->handle = ON_ACK_ADMIN_MSG;
            
            // the client has attached its key.  We want to make sure it's
            // using the right one.
            if(one_net_memcmp(&DATA[1],
              &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              ONE_NET_XTEA_KEY_FRAGMENT_SIZE) != 0)
            {
                // they are not using the correct key.  We'll attach it.
                ack_nack->payload->admin_msg[0] = ON_NEW_KEY_FRAGMENT;
                one_net_memmove(&(ack_nack->payload->admin_msg[1]),
                  &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                  ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
                key_update_in_progress = TRUE;
                (*client)->use_current_key = FALSE;
                break;
            }
            else
            {
                if(!((*client)->use_current_key))
                {
                    one_net_master_update_result(ONE_NET_UPDATE_NETWORK_KEY,
                        (const on_raw_did_t*) &raw_did, ack_nack);
                }
                (*client)->use_current_key = TRUE;
                #ifdef AUTO_SAVE
                save = TRUE;
                #endif
            }
            
            if(is_invite_did(SRC_DID))
            {
                // the settings_sent and fragment_delay_sent flags
                // will be set to true when the joining client sends an
                // admin message confirming it got these values.
                
                if(!settings_sent)
                {
                    ack_nack->payload->admin_msg[0] = ON_CHANGE_SETTINGS;
                    ack_nack->payload->admin_msg[1] = (*client)->flags;
                    break;
                }                
                #ifdef BLOCK_MESSAGES_ENABLED
                else if(features_block_capable((*client)->device.features)
                  && !fragment_delay_sent)
                {
                    ack_nack->payload->admin_msg[0] = ON_CHANGE_FRAGMENT_DELAY;
                    one_net_uint16_to_byte_stream(
                      on_base_param->fragment_delay_low,
                      &(ack_nack->payload->admin_msg[1]));
                    one_net_uint16_to_byte_stream(
                      on_base_param->fragment_delay_high,
                      &(ack_nack->payload->admin_msg[3]));
                    break;
                }
                #endif
                else
                {
                    // device is now added.  Adjust the client count,
                    // next Device DID, and notify everyone.
                    one_net_master_cancel_invite(
                      (const one_net_xtea_key_t* const) &invite_key);
                    // TODO -- check return value.  What do we do if this
                    // is not a success?
                    
                    // one_net_master_add_client function will figure out
                    // what DID to assign.  It will be the same one as what
                    // is in there now, but we'll need to blank it out to
                    // prevent problems.
                    one_net_memmove((*client)->device.did,
                      ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
                    one_net_master_add_client((*client)->device.features,
                      NULL, NULL, TRUE);
                }
            }
            
            if(remove_device_update_in_progress && 
              (*client)->send_remove_device_message)
            {
                device_change_did = &remove_device_did;
                ack_nack->payload->admin_msg[0] = ON_RM_DEV;
            }
            
            if(add_device_update_in_progress && 
              (*client)->send_add_device_message)
            {
                device_change_did = &add_device_did;
                ack_nack->payload->admin_msg[0] = ON_ADD_DEV;
            }
            
            if(device_change_did)
            {
                ack_nack->payload->admin_msg[1] = (*device_change_did)[0];
                ack_nack->payload->admin_msg[2] = (*device_change_did)[1];
                #ifdef ONE_NET_MULTI_HOP
                ack_nack->payload->admin_msg[3] = on_base_param->num_mh_devices;
                ack_nack->payload->admin_msg[4] = on_base_param->num_mh_repeaters;
                #else
                // TODO - should we ban multi-hop just because the master
                // isn't multi-hop?
                ack_nack->payload->admin_msg[3] = 0;
                ack_nack->payload->admin_msg[4] = 0;
                #endif
                break;
            }
            
            // they have the right key and no other admin messages need to
            // go out.  We'll send back the keep-alive interval they should
            // use.
            (*client)->use_current_key = TRUE;
            ack_nack->handle = ON_ACK_TIME_MS;
            ack_nack->payload->ack_time_ms = (*client)->keep_alive_interval;
            break;
        }
        
        case ON_FEATURES_RESP:
        {
            one_net_memmove(&((*client)->device.features),
              &DATA[1], sizeof(on_features_t));
            #ifdef ONE_NET_MULTI_HOP
            (*client)->device.max_hops = features_max_hops(
              (*client)->device.features);
            #endif
            ack_nack->handle = ON_ACK_FEATURES;
            ack_nack->payload->features = THIS_DEVICE_FEATURES;
            break;
        } // features response case //

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
static void on_master_adjust_recipient_list(const on_single_data_queue_t*
  const msg, on_recipient_list_t** recipient_send_list)
{
    int i, index;
    on_did_unit_t did_unit;
    on_client_t* client;
    
    
    // it's possible that this message was queued at a time when we had
    // clients, but we no longer have any.  If that's the case, cancel this
    // message.
    
    // TODO -- is this really possible?  Perhaps during a deletion?  Is this
    // checked anywhere else?
    if(master_param->client_count == 0)
    {
        *recipient_send_list = NULL;
        return;
    }
    
    // first see if this is an admin message of type ON_NEW_KEY_FRAGMENT,
    // ON_ADD_DEV, or ON_RM_DEV.  If not, we aren't interested in it here, but
    // the application code might be.
    if(msg->msg_type != ON_ADMIN_MSG || (msg->payload[0] != ON_NEW_KEY_FRAGMENT
      && msg->payload[0] != ON_ADD_DEV && msg->payload[0] != ON_RM_DEV))
    {
        one_net_adjust_recipient_list(msg, recipient_send_list);
        return;
    }
    
    did_unit.unit = ONE_NET_DEV_UNIT; // all of these updates go to the device
                                      // as a whole.
                                      
    // first we'll check to see if this is a message for the addition or
    // removal of a device.  If it is, then the device being added or removed
    // needs to be informed first.  If this is a device that is being ADDED and
    // it hasn't been informed yet, we'll cancel this message.  It will be
    // informed with an ACK to a message that IT initiates.  Note that
    // the only devices that are informed of a device being added or removed
    // are
    
    // 1) The device being added or removed.
    // 2) Any devices that have block, multi-hop, or extended single capability
    //    AND which do not sleep.  Any devices which sleep and need to know
    //    about a network update can query the master.
    
    // The REMOVAL of a sleeping device is sort of complicated.  We can't hold
    // everything up waiting for a sleeping device to check-in, so we won't
    // bother informing that device when we remove it.  We'll inform everyone
    // else who needs to know.  How to handle sleeping devices is always a
    // little complex and very applicaiton dependent, so a client-to-client
    // protocol involving device(s) that sleep will have to be largely dealt
    // with at the application level, not the ONE-NET level.
    

    // Jan. 11, 2011 -- Some of the code below might be unnecessary.  However
    // we seem to be getting some collisions.  When we STOP getting collisons,
    // we should revisit some of this anti-collision code to see if there are
    // any redundancies that can be removed.  For now, I'll take redundancy
    // over collisions!
    if(msg->payload[0] == ON_ADD_DEV || msg->payload[0] == ON_RM_DEV)
    {
        BOOL adding = (msg->payload[0] == ON_ADD_DEV);
        on_client_t* add_or_remove_client = NULL;
        
        if(adding && !add_device_update_in_progress)
        {
            // looks like we queued a message in the past and since then,
            // everything has either been updated or for whatever other
            // reason, we should not send this message, so cancel it.  If it
            // turns out that not everyone has been updated, this flag will be
            // reset to true.  We'll just assume that whoever set this flag
            // knows what he / she /it is doing. :)  Cancel the message!
            *recipient_send_list = NULL;
            return;
        }
        else if(!adding && !remove_device_update_in_progress)
        {
            // looks like we queued a message in the past and since then,
            // everything has either been updated or for whatever other
            // reason, we should not send this message, so cancel it.  If it
            // turns out that not everyone has been updated, this flag will be
            // reset to true.  We'll just assume that whoever set this flag
            // knows what he / she /it is doing. :)  Cancel the message!
            *recipient_send_list = NULL;
            return;
        }
        
        
        
        if((add_or_remove_client = client_info((const on_encoded_did_t*)
          &(msg->payload[1]))) == NULL)
        {
            // internal error.  This message has been corrupted somehow.
            // Cancel any notifications.
            on_raw_did_t raw_did;
            UInt8 i;
            on_ack_nack_t ack_nack;
            ack_nack.nack_reason = ON_NACK_RSN_INTERNAL_ERR;

            for(i = 0; i < master_param->client_count; i++)
            {
                on_decode(raw_did, client_list[i].device.did, ON_ENCODED_DID_LEN);
                
                if(adding)
                {
                    if(client_list[i].send_add_device_message)
                    {
                        one_net_master_update_result(ONE_NET_UPDATE_ADD_DEVICE,
                          (const on_raw_did_t*) &raw_did, &ack_nack);
                        client_list[i].send_add_device_message = FALSE;
                    }
                }
                else
                {
                    if(client_list[i].send_remove_device_message)
                    {
                        one_net_master_update_result(ONE_NET_UPDATE_REMOVE_DEVICE,
                          (const on_raw_did_t*) &raw_did, &ack_nack);
                        client_list[i].send_remove_device_message = FALSE;
                    }
                }
            }
            
            if(adding)
            {
                one_net_master_update_result(ONE_NET_UPDATE_ADD_DEVICE,
                    NULL, &ack_nack);
                add_device_update_in_progress = FALSE;
            }
            else
            {
                one_net_master_update_result(ONE_NET_UPDATE_REMOVE_DEVICE,
                    NULL, &ack_nack);
                remove_device_update_in_progress = FALSE;
            }
            
            // now cancel this message.
            *recipient_send_list = NULL;
            return;
        }
        
        if(adding && add_or_remove_client->send_add_device_message)
        {
            // TODO -- if we get here, I think we have an inefficiency.  The
            // client being added needs to be informed via an ACK.  How did
            // the "send_add_device_message" get flagged in the first place?
            // Did it need to be?  Can this be improved?  Regardless, this WILL
            // work and until we're sure this is NOT needed and / or desired and
            // working as far as an anti-collision test, we'll keep it in.
            
            // cancel the message.  This client has not been informed yet that
            // it is now a member of the network.  We want to inform it when it
            // checks in with us.
            *recipient_send_list = NULL;
            return;
        }
        else if(!adding && add_or_remove_client->send_remove_device_message)
        {
            if(features_device_sleeps(add_or_remove_client->device.features))
            {
                // this client is not going to be informed of its own removal.
                // TODO -- see the other TODO comments.  Is this wise /
                // necessary?
                
                // set the flag so that we don't send again and inform the
                // master that this client was not informed of its removal.
                add_or_remove_client->send_remove_device_message = FALSE;
                
                // cancel this message.
                *recipient_send_list = NULL;
                return;                
            }
            
            // the device being removed hasn't been informed yet.  Whoever
            // the original intended recipient of this message was, the device
            // being removed needs to be informed first, so we'll change the
            // recipient.
            
            // first clear the list.
            (*recipient_send_list)->num_recipients = 0;
            
            // now add the new recipient.
            did_unit.did[0] = msg->payload[1];
            did_unit.did[1] = msg->payload[2];
            add_recipient_to_recipient_list(*recipient_send_list, &did_unit);
            return;
        }
    }

    
    // Pick a random index.  This will be overridden if we find a match when traversing
    // the client list in the loop below.
    client = NULL;
    index = one_net_prand(get_tick_count(), master_param->client_count - 1);

    if((*recipient_send_list)->num_recipients)
    {
        client = client_info((const on_encoded_did_t*)
          &(*recipient_send_list)->recipient_list[0].did);
          
        if(client)
        {
            // iterate through the clients till we find the one we're queued
            // to send.
            for(i = 0; i < master_param->client_count; i++)
            {
                if(client == &client_list[i])
                {
                    index = i;
                    break; // found it.
                }
                client = NULL;
            }
        }
    }
    
    
    // clear the list.  If there was something on it and it still needs to be
    // sent, we'll add it back in
    (*recipient_send_list)->num_recipients = 0;
    
    for(i = 0; i < master_param->client_count; i++)
    {
        if(index >= master_param->client_count)
        {
            index = 0; // wraparound.
        }
        client = &client_list[index];
        index++;
        
        // see if we need to send this message fo this client.  We don't need
        // to send it if...
        // 1) It's already been sent and the device has verified that it has
        //    been sent.
        // 2) The device sleeps.  Sleeping devices don't receive outgoing
        //    messages for these particular updates.  They get them when they
        //    check in with keep-alive messages.
        
        // TODO -- do we need to check this one?  Presumably it was checked
        //         before it was queued in the first place?
        if(features_device_sleeps(client->device.features))
        {
            continue;
        }
        
        if(msg->payload[0] == ON_NEW_KEY_FRAGMENT && client->use_current_key)
        {
            // already sent
            continue;
        }
        if(msg->payload[0] == ON_ADD_DEV &&
          !(client->send_add_device_message))
        {
            // already sent
            continue;
        }
        if(msg->payload[0] == ON_RM_DEV &&
          !(client->send_remove_device_message))
        {
            // already sent
            continue;
        }
        
        
        // looks like we should send this one.
        did_unit.did[0] = client->device.did[0];
        did_unit.did[1] = client->device.did[1];
        add_recipient_to_recipient_list(*recipient_send_list, &did_unit);
        return;
    }
    
    // looks like nothing should be sent.  Abort this message.
    *recipient_send_list = NULL;
}


static void check_clients_for_missed_check_ins(void)
{
    UInt8 i;
    tick_t time_now = get_tick_count();
    UInt8 pld[4];
    for(i = 0; i < master_param->client_count; i++)
    {
        if(client_list[i].keep_alive_interval == 0)
        {
            continue; // this client is not expected to check in.
        }
        if(client_list[i].next_check_in_time < time_now)
        {
            if(one_net_master_client_missed_check_in(&client_list[i]) &&
              !features_device_sleeps(client_list[i].device.features))
            {
                send_admin_pkt(ON_KEEP_ALIVE_QUERY,
                  (const on_encoded_did_t* const) &(client_list[i].device.did),
                  pld, 0);
            }
            client_list[i].next_check_in_time = time_now + 
              MS_TO_TICK(5000 + client_list[i].keep_alive_interval);
        }
    }
}




//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE-NET_MASTER


#endif // if ONE_NET_MASTER defined //
