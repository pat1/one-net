#ifndef _ONE_NET_CLIENT_H
#define _ONE_NET_CLIENT_H

#include <one_net/port_specific/config_options.h>


//! \defgroup ONE-NET_CLIENT ONE-NET CLIENT device functionality
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
    \file one_net_client.h
    \brief ONE-NET CLIENT functionality declarations.

    Derives from ONE-NET.  CLIENT dependent functionality.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include <one_net/port_specific/one_net_client_port_const.h>
#include <one_net/port_specific/one_net_port_const.h>
#include <one_net/one_net.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_const
//! \ingroup ONE-NET_CLIENT
//! @{

enum
{
    // This is necessary for the one_net_timer to initialize the correct number
    // of timers.  This value counts the number of transactions defined by the
    // one_net_client (single_txn, block_txn, stream_txn, etc).
    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! How many txn_t the CLIENT has declared that need a timer.
        // single_txn, block_txn, stream_txn, response_txn
        ON_CLIENT_TXN_COUNT = 4,
        
        //! Offset from ONT_FIRST_TXN_TIMER for single transaction timer
        ON_CLIENT_SINGLE_TXN_TIMER_OFFSET = 0,
        
        //! Offset from ONT_FIRST_TXN_TIMER for block transaction timer
        ON_CLIENT_BLOCK_TXN_TIMER_OFFSET,
        
        //! Offset from ONT_FIRST_TXN_TIMER for stream transaction timer
        ON_CLIENT_STREAM_TXN_TIMER_OFFSET,
        
        //! Offset from ONT_FIRST_TXN_TIMER for the response transaction timer
        ON_CLIENT_RESPONSE_TXN_TIMER_OFFSET
    #else // ifndef _ONE_NET_SIMPLE_CLIENT //
        //! How many txn_t the CLIENT has declared that need a timer.
        // transaction timers are not needed for simple CLIENTs
        ON_CLIENT_TXN_COUNT = 0
    #endif // else _ONE_NET_SIMPLE_CLIENT is defined //
};

//! @} ONE-NET_CLIENT_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_CLIENT_typedefs
//! \ingroup ONE-NET_CLIENT
//! @{

//! Data needed to communicate with the MASTER
typedef struct
{
    //! Contains the MASTER did, and nonce expected to receive from the MASTER
    on_sending_device_t device;

    //! Interval at which the device must communicate with the MASTER
    tick_t keep_alive_interval;

    struct
    {
        //! Data rate the MASTER receives at
        UInt8 master_data_rate;

        //! Bitmap of communication and MASTER settable flags
        //! (sent in the SETTINGS admin message).
        UInt8 flags;
    } settings;
} on_master_t;

//! The structure that holds the fields needed by one_net_client_join_network
typedef struct
{
    one_net_channel_t   channel; 
    one_net_xtea_key_t  current_key;
    UInt8               single_block_encrypt_method;
    on_data_rate_t      data_rate;
    one_net_raw_sid_t   raw_sid;
    one_net_raw_did_t   raw_master_did;
    on_data_rate_t      master_data_rate;
    tick_t              keep_alive_interval;
#ifdef _ONE_NET_MULTI_HOP
    UInt8               max_hops;
#endif
#ifndef _ONE_NET_SIMPLE_CLIENT
    tick_t              fragment_delay_low;
    tick_t              fragment_delay_high;
    one_net_xtea_key_t  stream_key;
    UInt8               stream_encrypt_method;
#endif

} one_net_client_join_network_data_t;


//! @} ONE-NET_CLIENT_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_CLIENT_pub_var
//! \ingroup ONE-NET_CLIENT
//! @{

// see one_net_client.h for declarations and descriptions of the variables below.

#if defined(_ENHANCED_INVITE) && !defined(_IDLE)
    #error "ERROR : _IDLE must be defined if _ENHANCED_INVITE is defined.  Please adjust the #define values in the config_options.h file."
#endif

extern BOOL client_joined_network;
extern BOOL client_looking_for_invite;

#ifdef _ENHANCED_INVITE
    extern BOOL client_invite_timed_out;
	extern one_net_channel_t low_invite_channel;
	extern one_net_channel_t high_invite_channel;	
#endif


//! @} ONE-NET_CLIENT_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_pub_func
//! \ingroup ONE-NET_CLIENT
//! @{

#if !defined(_ENHANCED_INVITE)
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD);
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD);
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#else
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time);
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time);
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#endif

one_net_status_t one_net_client_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN);

void one_net_copy_to_nv_param(const UInt8 *param, UInt16 len);

void one_net_client_raw_master_did(one_net_raw_did_t * const master_did);

one_net_status_t one_net_client_send_single(UInt8 *data,
  UInt8 data_len, UInt8 dst_unit_idx, UInt8 priority,
  const one_net_raw_did_t *raw_dst, UInt8 src_unit);

#ifndef _ONE_NET_SIMPLE_CLIENT
    one_net_status_t one_net_client_block_stream_request(const UInt8 TYPE,
      const BOOL SEND, const UInt16 DATA_TYPE, const UInt16 LEN,
      const UInt8 PRIORITY, const one_net_raw_did_t * const DID,
      const UInt8 SRC_UNIT);
    one_net_status_t one_net_client_end_stream(void);
    void one_net_client_stream_key_query(void);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //
one_net_status_t one_net_client_join_network(one_net_client_join_network_data_t *DATA);

tick_t one_net_client(void);

#ifdef _ONE_NET_EVAL
    UInt8 one_net_client_get_channel(void);
#endif


//! @} ONE-NET_CLIENT_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_CLIENT

#endif // _ONE_NET_CLIENT_H //
