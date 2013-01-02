#ifndef ONE_NET_CLIENT_H
#define ONE_NET_CLIENT_H

#include "config_options.h"


//! \defgroup ONE-NET_CLIENT ONE-NET CLIENT device functionality
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
    \file one_net_client.h
    \brief ONE-NET CLIENT functionality declarations.

    Derives from ONE-NET.  CLIENT dependent functionality.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net.h"


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



//! @} ONE-NET_CLIENT_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_CLIENT_pub_var
//! \ingroup ONE-NET_CLIENT
//! @{


extern BOOL client_joined_network;
extern BOOL client_looking_for_invite;
extern on_master_t * const master;

#ifdef ENHANCED_INVITE
    extern BOOL client_invite_timed_out;
	extern one_net_channel_t low_invite_channel;
	extern one_net_channel_t high_invite_channel;	
#endif


extern on_sending_dev_list_item_t sending_dev_list[];


//! @} ONE-NET_CLIENT_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_pub_func
//! \ingroup ONE-NET_CLIENT
//! @{


#if !defined(ENHANCED_INVITE)
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY);
#else
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time);
#endif

#ifndef PEER
one_net_status_t one_net_client_init(const UInt8 * const param,
  const UInt16 param_len);
#else
one_net_status_t one_net_client_init(const UInt8 * const param,
  const UInt16 param_len, const UInt8* const peer_param,
  const UInt16 peer_param_len);
#endif

tick_t one_net_client(void);


#ifndef PEER
int client_nv_crc(const UInt8* param);
#else
int client_nv_crc(const UInt8* param, const UInt8* peer_param);
#endif

#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Fills in some default values determined by ONE-NET for a proposed block transfer.
    
    \param[in] dst The destination of the transfer.
    \param[in] transfer_size The number of bytes to be transferred.
    \param[out] priority The priority of the transfer.
    \param[out] chunk_size The "chunk size" involved in the transfer.
    \param[out] frag_delay The time to wait between packet sends.
    \param[out] chunk_delay The time to pause between "chunks" of the message.
    \param[out] data_rate The data rate to use for the transfer.
    \param[out] channel The channel to use for the transfer.
    \param[out] timeout The time to wait for a response before assuming that
                   communication has been lost.
    \param[out] ack_nack If rejecting the transfer and there is an ack or nack associated
                   with it, this value will be filled in.
    
    \return The nack reason if rejecting the transfer.
*/
on_nack_rsn_t on_client_get_default_block_transfer_values(
  const on_encoded_did_t* dst, UInt32 transfer_size, UInt8* priority,
  UInt8* chunk_size, UInt16* frag_delay, UInt16* chunk_delay, UInt8* data_rate,
  UInt8* channel, UInt16* timeout, on_ack_nack_t* ack_nack);
  

/*!
    \brief Called when this device desires to send another device a block message
           AFTER the block message parameters have been loaded.
    
    This function is called (usually by the by the application code) after the
    parameters have been filled in.  In this function, ONE-NET sets up the
    state machine to find a route to the source, request permissions and inform
    any other devices of the transfers, switch data rates, then start sending
    the actual packets.  This function is called by the SOURCE of the message.
    
    \param[in] msg The parameters of the block message that is to be
                   transferred.  These parameters should be filled in before
                   this function is called.
    \param[out] ack_nack If rejecting the transfer and there is an ack or nack associated
                   with it, this value will be filled in.
    
    \return The nack reason if rejecting the transfer.
*/
on_nack_rsn_t on_client_initiate_block_msg(block_stream_msg_t* msg,
  on_ack_nack_t* ack_nack);
#endif


#ifdef STREAM_MESSAGES_ENABLED
/*!
    \brief Fills in some default values determined by ONE-NET for a proposed stream transfer.
    
    \param[in] dst The destination of the transfer.
    \param[in] time_ms Proposed duration of the stream transfer.  If time is 0, then the time is unknown.
    \param[out] priority The priority of the transfer.
    \param[out] frag_delay The time to wait between packet sends.
    \param[out] data_rate The data rate to use for the transfer.
    \param[out] channel The channel to use for the transfer.
    \param[out] timeout The time to wait for a response before assuming that
                   communication has been lost.
    \param[out] ack_nack If rejecting the transfer and there is an ack or nack associated
                   with it, this value will be filled in.
    
    \return The nack reason if rejecting the transfer.
            
*/
on_nack_rsn_t on_client_get_default_stream_transfer_values(
  const on_encoded_did_t* dst, UInt32 time_ms, UInt8* priority,
  UInt16* frag_delay, UInt8* data_rate, UInt8* channel, UInt16* timeout,
  on_ack_nack_t* ack_nack);


/*!
    \brief Called when this device desires to send another device a stream message
           AFTER the block message parameters have been loaded.
    
    This function is called (usually by the by the application code) after the
    parameters have been filled in.  In this function, ONE-NET sets up the
    state machine to find a route to the source, request permissions and inform
    any other devices of the transfers, switch data rates, then start sending
    the actual packets.  This function is called by the SOURCE of the message.
    
    \param[in] msg The parameters of the stream message that is to be
                   transferred.  These parameters should be filled in before
                   this function is called.
    \param[out] ack_nack If rejecting the transfer and there is an ack or nack associated
                   with it, this value will be filled in.
    
    \return The nack reason if rejecting the transfer.
*/  
on_nack_rsn_t on_client_initiate_stream_msg(block_stream_msg_t* msg,
  on_ack_nack_t* ack_nack);
#endif

#ifndef ONE_NET_SIMPLE_CLIENT
void on_client_set_device_slideoff(const on_encoded_did_t* enc_did,
  device_slideoff_t slideoff);
void on_client_unlock_device_slideoff(const on_encoded_did_t* enc_did);
#endif



//! @} ONE-NET_CLIENT_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_CLIENT

#endif // ONE_NET_CLIENT_H //
