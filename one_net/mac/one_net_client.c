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

#ifdef _STREAM_MESSAGES_ENABLED
//! Set to true if a confirmation of a stream key change is needed.
static BOOL confirm_stream_key_change = FALSE;
#endif


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

static one_net_status_t one_net_client_send_single(UInt8 pid,
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
  
static BOOL look_for_invite(void);

static on_message_status_t handle_admin_pkt(const on_encoded_did_t * const
  SRC_DID, const UInt8 * const DATA, on_txn_t* txn, on_ack_nack_t* ack_nack);
  
static BOOL check_in_with_master(void);


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
    return ONS_SUCCESS;
} // one_net_client_look_for_invite //


/*!
    \brief Initializes the CLIENT to run in a network that it has previously
      joined.

    If the CLIENT has not yet joined a network, one_net_client_look_for_invite
    needs to be called instead of this function.

    \param[in] PARAM The parameters (or part) that were saved.  If NULL, then
                     the caller has already initialized the base memory.
    \param[in] PARAM_LEN The sizeof PARAM in bytes.

    \return ONS_SUCCESS If initializing the CLIENT was successful
            ONS_BAD_PARAM If any of the parameters are invalid
*/
one_net_status_t one_net_client_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN)
{
    one_net_status_t status;
    
    if(PARAM != NULL)
    {
        // code here to initalize things from PARAM and PARAM_LEN
    }

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
        if(one_net_client_init(0, 0) != ONS_SUCCESS)
        {
            init_internal();
            client_joined_network = FALSE;
            client_looking_for_invite = TRUE;
        }

        on_state = ON_LISTEN_FOR_DATA;
    }
    

    switch(on_state)
    {
        case ON_IDLE:
            break;
            
        case ON_LISTEN_FOR_DATA:
        {
            //
            // Listen for a new transaction.
            // Also check to see if there are any events
            // associated with timers that need attention
            //
            break;
        }
        
        case ON_JOIN_NETWORK:
        {
            break;
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
    on_message_status_t msg_status;
    on_msg_hdr_t msg_hdr;
    on_raw_did_t raw_src_did, raw_repeater_did;
    UInt8 response_pid;
    on_sending_device_t* device;

    on_decode(raw_src_did, *(pkt->enc_src_did), ON_ENCODED_DID_LEN);
    on_decode(raw_repeater_did, *(pkt->enc_repeater_did), ON_ENCODED_DID_LEN);
    
    msg_hdr.msg_type = *msg_type;
    msg_hdr.pid = *(pkt->pid);
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
    
    
    // change the nonce we want.
    device->last_nonce = device->expected_nonce;
    device->expected_nonce = one_net_prand(get_tick_count(), ON_MAX_NONCE);
    

// normally we try not to use goto statements but this is embedded programming
// and it may save us a few bytes?
ocsdh_build_resp:
    response_pid = get_single_response_pid(*(pkt->pid),
      ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR, FALSE);

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
    
    msg_hdr.pid = *(pkt->pid);
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
            // we may be able to re-send with a higher max hops.
            
            if(mh_repeater_available && txn->max_hops <
              txn->device->max_hops)
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
    
    msg_hdr.pid = *(pkt->pid);
    msg_hdr.msg_id = pkt->msg_id;
    msg_hdr.msg_type = *msg_type;
    on_decode(dst ,*(pkt->enc_dst_did), ON_ENCODED_DID_LEN);

    #ifndef _ONE_NET_MULTI_HOP
    one_net_client_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, &dst, ack_nack);
    #else
    one_net_client_single_txn_status(status, (*txn)->retry, msg_hdr,
      raw_pld, &dst, ack_nack, pkt->hops);
    #endif  
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
static one_net_status_t one_net_client_send_single(UInt8 pid,
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
    UInt8* pid = &invite_pkt[ONE_NET_ENCODED_PID_IDX];
    UInt8 bytes_read = ONE_NET_ENCODED_PID_IDX + 1;
    UInt8 bytes_left_to_read;

	
#ifdef _ENHANCED_INVITE
    if(ont_expired(ONT_INVITE_TIMER))
	{
        client_invite_timed_out = TRUE;
	    client_looking_for_invite = FALSE;
        one_net_client_invite_result(NULL, ONS_TIME_OUT);
	    return FALSE;
	}
#endif

    // try to read in a packet
    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) == ONS_SUCCESS
      && one_net_read(invite_pkt, bytes_read) == bytes_read &&
      packet_is_invite(*pid))
    {
        #ifdef _ONE_NET_MULTI_HOP
        bytes_left_to_read = ON_INVITE_ENCODED_PLD_LEN +
          packet_is_multihop(*pid) ? ON_ENCODED_HOPS_SIZE : 0;
        #else
        bytes_left_to_read = ON_INVITE_ENCODED_PLD_LEN;
        #endif
        if(one_net_read(&invite_pkt[bytes_read], bytes_left_to_read) ==
          bytes_left_to_read)
        {
            UInt8 raw_invite[ON_RAW_INVITE_SIZE];
            
            // verify message CRC and addresses
            if(setup_pkt_ptr(*pid, invite_pkt, &data_pkt_ptrs) &&
              is_broadcast_did(data_pkt_ptrs.enc_dst_did) &&
              is_master_did(data_pkt_ptrs.enc_src_did) &&
              verify_msg_crc(&data_pkt_ptrs))
            {
                if(on_decode(raw_invite, data_pkt_ptrs.payload,
                  ON_INVITE_ENCODED_PLD_LEN) == ONS_SUCCESS)
                {
                    if(on_decrypt(ON_INVITE, raw_invite, &invite_key,
                      ON_RAW_INVITE_SIZE) == ONS_SUCCESS &&
                      verify_payload_crc(*(data_pkt_ptrs.pid), raw_invite))
                    {
                        // TODO parse the invite packet and respond
                    }
                }
            }
        }
    } // if a packet was received //

    if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
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
            one_net_memmove(&(on_base_param->current_key[0]),
              &(on_base_param->current_key[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
            one_net_memmove(
              &(on_base_param->current_key[3*ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              &DATA[1], ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
              
            confirm_key_change = TRUE; 
            ont_set_timer(ONT_KEEP_ALIVE_TIMER, 0);
            break;
        } // change key case //

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
    tick_t keep_alive_time;
    UInt8 raw_pld[5];
    
    if(!ont_inactive_or_expired(ONT_KEEP_ALIVE_TIMER))
    {
        return FALSE;
    }

    if(confirm_key_change)
    {
        raw_pld[0] = ON_KEY_CHANGE_CONFIRM;
        raw_pld[1] = one_net_compute_crc((UInt8*) on_base_param->current_key,
          ONE_NET_XTEA_KEY_LEN, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
        keep_alive_time = MS_TO_TICK(5000);
    }
    #ifdef _STREAM_MESSAGES_ENABLED
    else if(confirm_stream_key_change)
    {
        raw_pld[0] = ON_STREAM_KEY_CHANGE_CONFIRM;
        raw_pld[1] = one_net_compute_crc((UInt8*) on_base_param->stream_key,
          ONE_NET_XTEA_KEY_LEN, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
        keep_alive_time = MS_TO_TICK(5000);
    }
    #endif
    else
    {
        raw_pld[0] = ON_FEATURES_RESP;
        one_net_memmove(&raw_pld[1], &THIS_DEVICE_FEATURES,
          sizeof(on_features_t));
        keep_alive_time = MS_TO_TICK(30000);
    }
    
    if(one_net_client_send_single(ONE_NET_ENCODED_SINGLE_DATA,
      ON_ADMIN_MSG, raw_pld, 5, ONE_NET_LOW_PRIORITY,
      NULL, (on_encoded_did_t*) MASTER_ENCODED_DID
      #ifdef _PEER
      , FALSE,
      ONE_NET_DEV_UNIT
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , NULL
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
      , NULL
      #endif
      ) == ONS_SUCCESS)
    {
        ont_set_timer(ONT_KEEP_ALIVE_TIMER, keep_alive_time);
        return TRUE;
    }
    
    return FALSE;
}


#endif // if _ONE_NET_CLIENT is defined //


//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT
