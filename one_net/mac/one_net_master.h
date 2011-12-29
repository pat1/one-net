#ifndef _ONE_NET_MASTER_H
#define _ONE_NET_MASTER_H

#include "config_options.h"



//! \defgroup ONE-NET_MASTER ONE-NET MASTER device functionality
//! \ingroup ONE-NET
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
    \file one_net_master.h
    \brief ONE-NET MASTER functionality declarations.

    Derives from ONE-NET.  MASTER dependent functionality.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net.h"
#include "one_net_port_const.h"
#include "one_net_master_port_const.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{

enum
{
    //! The initial CLIENT DID.  Since a DID is only 12 bits, only the upper 12
    //! bits of this value is used.  Note the order this is stored in memory may
    //! not be the proper order for being sent.
    ONE_NET_INITIAL_CLIENT_DID = 0x0020,
    
    //! The number of transactions declared by the MASTER that need a
    //! one_net_timer
    // +1 is for the response transaction.  If any txn_t's are added or
    // removed, this number may need to be changed.
    ON_MASTER_TXN_COUNT = ONE_NET_MASTER_MAX_TXN + 1,
    
    //! The offset from ONT_FIRST_TXN_TIMER for the timer to use with the
    //! response transaction
    ON_MASTER_RESPONSE_TXN_TIMER_OFFSEST = ONE_NET_MASTER_MAX_TXN
};

//! @} ONE-NET_MASTER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MASTER_typedefs
//! \ingroup ONE-NET_MASTER
//! @{

//! The various MAC settings that can be updated.  These are used to report the
//! results of changing a devices parameters.
typedef enum
{
    //! Update a device's data rate
    ONE_NET_UPDATE_DATA_RATE = 0x00,

    //! Updating a device's setting for the data rate of a peer device.
    ONE_NET_UPDATE_PEER_DATA_RATE = 0x01,
    
    //! Updating the network key is complete
    ONE_NET_UPDATE_NETWORK_KEY = 0x02,

    //! Assigning a peer
    ONE_NET_UPDATE_ASSIGN_PEER = 0x03,

    //! Unassigning a peer
    ONE_NET_UPDATE_UNASSIGN_PEER = 0x04,

    //! Updating reporting status changes to the MASTER
    ONE_NET_UPDATE_REPORT_TO_MASTER = 0x05,

    //! Low Priority Fragment delay update
    ONE_NET_UPDATE_LOW_FRAGMENT_DELAY = 0x06,

    //! High Priority Fragment delay update
    ONE_NET_UPDATE_HIGH_FRAGMENT_DELAY = 0x07,

    //! Updates the keep alive interval for a device
    ONE_NET_UPDATE_KEEP_ALIVE = 0x08,

    //! Indicates an attempt to remove a device from the network
    ONE_NET_UPDATE_REMOVE_DEVICE = 0x09,
    
    //! Updating the stream key is complete
    ONE_NET_UPDATE_STREAM_KEY = 0x0A,

    //! This is to mark nothing was updated.  This item should ALWAYS be
    //! LAST IN THE LIST
    ONE_NET_UPDATE_NOTHING
} one_net_mac_update_t;

//! The one_net_master_add_client function needs several
//! data elements that the new client's capabilities.
//! A pointer to this structure
//! is provided to the one_net_master_add_client function so that
//! all of the client's capablilities are available to the master 
//! as the client is added to the network.
typedef struct
{
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    BOOL multi_hop_repeater;                //!< TRUE if the client operates as a repeater
    #endif
    #ifdef _ONE_NET_MULTI_HOP
    BOOL multi_hop_capable;                 //!< TRUE if the client will generate or receive mulit-hop msgs
    #endif
    #ifdef _BLOCK_MESSAGES_ENABLED
    BOOL block_capable;                     //!< TRUE if the client is block capable
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    BOOL stream_capable;                    //!< TRUE if the client is stream capable
    #endif
    BOOL data_rate_capable;                 //!< TRUE if the client is data rate capable
    #ifdef _PEER
    UInt8 max_peers;                        //!< the maximum number of peers this device can support
    #endif
    on_data_rate_t max_data_rate;           //!< the maximum data rate this client supports
} one_net_master_add_client_in_t;


//! The one_net_master_add_client function needs to return many
//! data elements that the new client needs in order to 
//! operate in the ONE-NET network. A pointer to this structure
//! is provided to the one_net_master_add_client function so that
//! all of the data elements can be returned.
typedef struct
{
    one_net_raw_sid_t raw_sid;              //!< raw sid for the client (NID plus client's DID)
    one_net_xtea_key_t current_key;         //!< current xtea key for the client
    #ifdef _STREAM_MESSAGES_ENABLED
    one_net_xtea_key_t stream_key;          //!< stream xtea key for the client
    #endif
    one_net_raw_did_t master_did;           //!< raw master did
    tick_t keep_alive_interval;             //!< specifies how often the client needs to check in with master
    #ifdef _BLOCK_MESSAGES_ENABLED
    tick_t fragment_delay_low;              //!< fragment delay for low priority
    tick_t fragment_delay_high;             //!< fragment delay for high priority
    #endif
    UInt8 single_block_encrypt_method;      //!< the encryption method for single and block
    #ifdef _STREAM_MESSAGES_ENABLED
    UInt8 stream_encrypt_method;            //!< the encryption method for streams
    #endif
    on_data_rate_t master_data_rate;        //!< the data rate of the master
    on_data_rate_t data_rate;               //!< this clients normal data rate
    one_net_channel_t channel;              //!< the channel being used by this master
    BOOL update_master;                     //!< TRUE if the master wants to be informed of application level state change
} one_net_master_add_client_out_t;


//! Parameters specific to the MASTER
typedef struct
{
    //! The next available DID to be handed to a CLIENT that joins the network.
    UInt16 next_client_did;
	
    //! The last key used
    one_net_xtea_key_t old_key;
    
    #ifdef _STREAM_MESSAGES_ENABLED
    //! The last stream key used
    one_net_xtea_key_t old_stream_key;
    #endif
    
    //! The number of CLIENTs currently in the network
    UInt16 client_count;
} on_master_param_t;


//! Structure to keep track of the CLIENT
typedef struct
{
    //! The encoded device address of the CLIENT
    on_encoded_did_t did;

    //! The nonce the MASTER expects to receive from the CLIENT
    UInt8 expected_nonce;

    //! The last nonce the MASTER received from the CLIENT
    UInt8 last_nonce;

    //! The nonce the CLIENT expects to receive from the MASTER
    UInt8 send_nonce;

    //! The current data rate the device is listening at
    UInt8 data_rate;

    //! The max data rate the device can operate at
    UInt8 max_data_rate;

    //! The features the device supports
    UInt8 features;

    //! Keeps track of the communication and MASTER setable flags in the CLIENT
    //! (the ones sent in the SETTINGS admin message).
    UInt8 flags;

    //! Indicates if using the current key or the old key.
    BOOL use_current_key;

    #ifdef _STREAM_MESSAGES_ENABLED
    //! Indicates if using the current stream key, or the old stream key
    BOOL use_current_stream_key;
	#endif

    // TODO - get rid of _MAX_HOPS_CLIENT_T_COMPATIBLE
    #if defined(_ONE_NET_MULTI_HOP)
    UInt8 max_hops;
    #endif
} on_client_t;


//! @} ONE-NET_MASTER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MASTER_pub_var
//! \ingroup ONE-NET_MASTER
//! @{

extern const int MASTER_NV_PARAM_SIZE_BYTES;

//! @} ONE-NET_MASTER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MASTER_pub_func
//! \ingroup ONE-NET_MASTER
//! @{

#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_create_network(
  const one_net_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
  const one_net_xtea_key_t * const STREAM_KEY,
  const UInt8 STREAM_ENCRYPT_METHOD);
#else
one_net_status_t one_net_master_create_network(
  const one_net_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD);
#endif

one_net_status_t one_net_master_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN);

UInt8 one_net_master_get_channel(void);

one_net_status_t one_net_master_send_single(UInt8 * data,
  UInt8 data_len, const BOOL send_to_peer_list, UInt8 priority,
  const one_net_raw_did_t *raw_dst, UInt8 src_unit,
  tick_t* send_time_from_now, tick_t* expire_time_from_now);

one_net_status_t one_net_master_block_stream_request(UInt8 TYPE, BOOL SEND,
  UInt8 DATA_TYPE, UInt16 LEN, UInt8 PRIORITY, const one_net_raw_did_t * DID,
  UInt8 SRC_UNIT, UInt8 DST_UNIT);

#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_end_stream(const one_net_raw_did_t * const DID);
#endif

one_net_status_t one_net_master_change_client_data_rate(
  const one_net_raw_did_t * const RAW_DID, const UInt8 DATA_RATE);
one_net_status_t one_net_master_change_client_keep_alive(
  const one_net_raw_did_t * const RAW_DST, const UInt32 KEEP_ALIVE);
#ifdef _BLOCK_MESSAGES_ENABLED
one_net_status_t one_net_master_change_frag_dly(
  const one_net_raw_did_t * const RAW_DST, const UInt8 PRIORITY,
  const UInt32 DELAY);
#endif
one_net_status_t one_net_master_change_key(
  const one_net_xtea_key_fragment_t KEY_FRAGMENT);
#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_change_stream_key(
  const one_net_xtea_key_t * const NEW_STREAM_KEY);
#endif
#ifdef _PEER
one_net_status_t one_net_master_peer_assignment(const BOOL ASSIGN,
  const one_net_raw_did_t * const SRC_DID, const UInt8 SRC_UNIT,
  const one_net_raw_did_t * const PEER_DID, const UInt8 PEER_UNIT);
#endif
one_net_status_t one_net_master_set_update_master_flag(const BOOL UPDATE_MASTER,
  const one_net_raw_did_t * const DST_DID);

#if !defined(_ONE_NET_EVAL) && defined(_PEER)
one_net_status_t one_net_master_change_peer_data_rate(
  const one_net_raw_did_t * const RAW_DST,
  const one_net_raw_did_t * const RAW_PEER, const UInt8 DATA_RATE);
#endif

one_net_status_t one_net_master_start_data_rate_test(
  const one_net_raw_did_t * const SENDER,
  const one_net_raw_did_t * const RECEIVER, const UInt8 DATA_RATE);

one_net_status_t one_net_master_invite(const one_net_xtea_key_t * const KEY);
one_net_status_t one_net_master_cancel_invite(
  const one_net_xtea_key_t * const KEY);

one_net_status_t one_net_master_remove_device(
  const one_net_raw_did_t * const RAW_PEER_DID);

void one_net_master(void);

one_net_status_t one_net_master_add_client(
  const one_net_master_add_client_in_t * CAPABILITIES,
  one_net_master_add_client_out_t * config);
  
on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID);

#ifdef _STREAM_MESSAGES_ENABLED
    one_net_xtea_key_t* get_encryption_key(const BOOL current_key, const BOOL stream_key);
#else
    one_net_xtea_key_t* get_encryption_key(const BOOL current_key);
#endif
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_xtea_key_t* get_client_encryption_key(const on_client_t* const client, const BOOL stream_key);
#else
    one_net_xtea_key_t* get_client_encryption_key(const on_client_t* const client);
#endif


one_net_status_t on_master_handle_ack_nack_response(on_txn_t* const txn,
	const one_net_raw_did_t* const src_did, on_nack_rsn_t* const nack_reason,
	on_ack_nack_handle_t* const ack_nack_handle,
	ack_nack_payload_t* const ack_nack_payload);



// Derek_S 11/2/2010 - I don't see anywhere where this function is ever called.
one_net_status_t one_net_master_delete_last_client(one_net_raw_did_t* raw_client_did);

#ifdef _PEER
UInt8 master_get_num_free_send_txn(void);
#endif


// Derek_S 11/2/2010
// TO-DO : We don't actually need to pass this the master_param parameter, do we?
// But it doesn't really hurt anything.
//void update_client_count(on_master_param_t* master_param);


//! @} ONE-NET_MASTER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MASTER

#endif // _ONE_NET_MASTER_H //
