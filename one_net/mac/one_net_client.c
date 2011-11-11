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
#include "one_net_client.h"
#include "one_net_client_port_const.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "tick.h"



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
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type); 
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
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
    return 0;
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
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type)
{
    return ON_MSG_CONTINUE;
}
 
  
// TODO -- document  
static on_message_status_t on_client_handle_single_ack_nack_response(
  on_txn_t** txn, on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}
  

// TODO -- document 
static on_message_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack)
{
    return ON_MSG_CONTINUE;
}



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
        sending_dev_list[match_idx].sender.expected_nonce = ON_INVALID_NONCE;
        sending_dev_list[match_idx].sender.send_nonce = ON_INVALID_NONCE;
        sending_dev_list[match_idx].sender.last_nonce = ON_INVALID_NONCE;
        sending_dev_list[match_idx].sender.features = FEATURES_UNKNOWN;
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
    return ONS_SUCCESS;
}



//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT
