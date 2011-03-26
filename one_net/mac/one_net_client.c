//! \addtogroup ONE-NET_CLIENT ONE-NET CLIENT device functionality
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

#include "one_net_client_net.h"
#include "one_net_client_port_specific.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_timer.h"
#include "one_net_prand.h"

#ifdef _ONE_NET_EVAL
    #include "oncli.h"
    extern one_net_raw_did_t client_did; // Derek_S 11/4/2010 for CLI "list" command
#endif // ifdef ONE_NET_EVAL //


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_const
//! \ingroup ONE-NET_CLIENT
//! @{

enum
{	
	// Derek_S : "Feature" is now ONE_NET_MAX_PEER_UNIT, not ONE_NET_PEER_PER_UNIT.
    // chaning everything to ONE_NET_MAX_PEER_UNIT
	
	// TO - DO : I'm a bit confused by the "|" operator below and why we're using it.
    // Need to figure out exactly where and how ON_FEATURES is used and confirm this
    // is correct.  The "|" operator seems incorrect to me, but I'm leaving it in for now.

    //! The features this client supports
    #ifdef _ONE_NET_SIMPLE_CLIENT
        #ifdef _PEER
            ON_FEATURES = ONE_NET_MAX_PEER_UNIT
		#else
			ON_FEATURES = 0
        #endif
    #else // ifdef _ONE_NET_SIMPLE_CLIENT //
        #ifdef _ONE_NET_MULTI_HOP
            #ifdef _ONE_NET_MH_CLIENT_REPEATER
                ON_FEATURES = ON_MAC_FEATURES | ON_MH_CAPABLE | ON_MH_REPEATER
                  | ONE_NET_MAX_PEER_UNIT
            #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
                ON_FEATURES = ON_MAC_FEATURES | ON_MH_CAPABLE
                  | ONE_NET_MAX_PEER_UNIT
            #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        #else // ifdef _ONE_NET_MULTI_HOP //
		    #ifdef _PEER
                ON_FEATURES = ON_MAC_FEATURES | ONE_NET_MAX_PEER_UNIT
			#else
			    ON_FEATURES = ON_MAC_FEATURES
			#endif
        #endif // else _ONE_NET_MULTI_HOP is not defined //
    #endif // else for ifdef _ONE_NET_SIMPLE_CLIENT //
};


/*!
    \brief Indexes for queued transactions.

    These indexes are used to store the information for a queued transaction
    (type, destination, payload), in the pkt field of the txn_t.  When the
    packet is about to be sent, the data will be copied to local variables,
    and the packet will then be built and the pkt field will then contain the
    encoded packet.

    The ONE_NET_ENCODED_PID_IDX is added to these values, to make sure the data
    does not overlap the ENCODED_PID_IDX for transactions who are ready to be
    sent (data rate & invites).  Otherwise the difference between valid data or
    one of these other PIDs can not be determined.
*/
enum
{
    //! The admin message type (if admin msg).
    ON_ADMIN_TYPE_IDX = ONE_NET_ENCODED_PID_IDX + 1,

    //! Start of where the data is stored.
    ON_DATA_IDX = ON_ADMIN_TYPE_IDX + ONE_NET_RAW_DID_LEN
};


enum
{
    //! Size of the parameters that need to be saved to non-volatile memory
    ON_NV_PARAM_SIZE = sizeof(on_base_param_t) + sizeof(on_master_t)
#ifdef _PEER
      + sizeof(on_peer_t)
#endif
};


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
typedef struct
{
    //! Transaction identifier to be returned to the network layer.
    UInt8 txn_id;

    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! Indicates if the single admin request for the transaction is being
        //! made.
        BOOL requesting;
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

    //! The transaction
    on_txn_t txn;
} client_txn_t;

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

//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
static UInt8 nv_param[ON_NV_PARAM_SIZE];

//! The MASTER device
on_master_t * const master
  = (on_master_t *)(nv_param + sizeof(on_base_param_t));

//! The ONE_NET_RX_FROM_DEVICE_COUNT devices that have most recently sent data
//! to this device.
static struct
{
    on_sending_device_t sender;     //!< did and expected nonce from sender.
    UInt8 lru;                      //!< least recently used value
} sending_dev_list[ONE_NET_RX_FROM_DEVICE_COUNT];

//! "Protected" state variable inherited from one_net.c
extern on_state_t on_state;

//! "Protected" base parameters variable inherited from one_net.c
extern on_base_param_t * on_base_param;

#ifdef _ONE_NET_MULTI_HOP
    //! location to store the encoded data for an ack/nack packet
        UInt8 on_client_response_pkt[ON_ACK_NACK_LEN + ON_ENCODED_HOPS_SIZE];

    //! location to store the encoded data for the single transaction
    UInt8 on_single_pkt[ON_ENCODED_SINGLE_DATA_LEN + ON_ENCODED_HOPS_SIZE];
#else // ifdef _ONE_NET_MULTI_HOP //
    //! location to store the encoded data for an ack/nack packet
        UInt8 on_client_response_pkt[ON_ACK_NACK_LEN];

    //! location to store the encoded data for the single transaction
    UInt8 on_single_pkt[ON_ENCODED_SINGLE_DATA_LEN];
#endif // else _ONE_NET_MULTI_HOP is not defined //

void one_net_copy_to_nv_param(const UInt8 *param, UInt16 len)
{
    one_net_memmove(nv_param, param, len);
}

// If any of transactions are added or removed from the list, the
// ON_CLIENT_TXN_COUNT enumeration in the one_net_client.h file needs to
// be adjusted.
#ifndef _ONE_NET_SIMPLE_CLIENT
    //! location to store the encoded data for a block transaction.
    UInt8 on_block_pkt[ONE_NET_MAX_ENCODED_PKT_LEN];

    //! location to store the encoded data for a stream transaction.
    UInt8 on_stream_pkt[ONE_NET_MAX_ENCODED_PKT_LEN];

    #ifdef _ONE_NET_MULTI_HOP
        //! Stores the current single transaction.
        client_txn_t on_single_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_SINGLE_TXN_TIMER_OFFSET, 0, 0, 0,
          sizeof(on_single_pkt), on_single_pkt}};

        //! The current block transaction
        client_txn_t on_block_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_BLOCK_TXN_TIMER_OFFSET, 0, 0, 0,
          sizeof(on_block_pkt), on_block_pkt}};

        //! The current stream transaction
        client_txn_t on_stream_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_STREAM_TXN_TIMER_OFFSET, 0, 0, 0,
          sizeof(on_stream_pkt), on_stream_pkt}};

        //! Used to send a response
        static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0,
          ON_INVALID_MSG_TYPE, FALSE, ONT_FIRST_TXN_TIMER
          + ON_CLIENT_RESPONSE_TXN_TIMER_OFFSET, 0, 0, 0,
          sizeof(on_client_response_pkt), on_client_response_pkt};
    #else // ifdef _ONE_NET_MULTI_HOP //
        //! Stores the current single transaction.
        client_txn_t on_single_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_SINGLE_TXN_TIMER_OFFSET, 0, 0,
          sizeof(on_single_pkt), on_single_pkt}};

        //! The current block transaction
        client_txn_t on_block_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_BLOCK_TXN_TIMER_OFFSET, 0, 0,
          sizeof(on_block_pkt), on_block_pkt}};

        //! The current stream transaction
        client_txn_t on_stream_txn = {SINGLE_DST_TXN_ID, FALSE,
          {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE, FALSE,
          ONT_FIRST_TXN_TIMER + ON_CLIENT_STREAM_TXN_TIMER_OFFSET, 0, 0,
          sizeof(on_stream_pkt), on_stream_pkt}};

        //! Used to send a response
        static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0,
          ON_INVALID_MSG_TYPE, FALSE, ONT_FIRST_TXN_TIMER
          + ON_CLIENT_RESPONSE_TXN_TIMER_OFFSET, 0, 0,
          sizeof(on_client_response_pkt), on_client_response_pkt};
    #endif // else _ONE_NET_MULTI_HOP is not defined //
#else
    client_txn_t on_single_txn = {SINGLE_DST_TXN_ID, {ONE_NET_NO_PRIORITY,
       0, 0, ON_INVALID_MSG_TYPE, 0, sizeof(on_single_pkt), on_single_pkt}};

    //! Used to send a response
    static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0,
      ON_INVALID_MSG_TYPE, 0, sizeof(on_client_response_pkt),
      on_client_response_pkt};
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! Flag to indicate settings need to be saved
static BOOL save = FALSE;

//! Flag to indicate the CLIENT needs to confirm the key update
static BOOL confirm_key_update = FALSE;

//! Flag to indicate that the results of the data rate test need to be reported
static BOOL report_data_rate_results = FALSE;

//! Flag to indicate that the device has been removed from the network.
static BOOL removed = FALSE;

//! Minimum channel to look for an invite
static one_net_channel_t minimum_invite_channel;

//! Maximum channel to look for an invite
static one_net_channel_t maximum_invite_channel;



//! @} ONE-NET_CLIENT_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================




//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{

static one_net_status_t init_internal(void);

one_net_status_t on_client_send_single(const UInt8 * const DATA,
  const UInt8 DATA_LEN, const UInt8 PRIORITY,
  const on_encoded_did_t * const DST, const UInt8 TXN_ID);

client_txn_t * get_client_txn(const on_txn_t * const txn);

static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);
static UInt8 device_txn_nonce(const on_encoded_did_t * const DID);
static void set_device_txn_nonce(const on_encoded_did_t * const DID,
  const UInt8 NEXT_NONCE);
static UInt8 data_rate_for_dst(const on_encoded_did_t * const DID);
#ifdef _ONE_NET_MULTI_HOP
    static UInt8 max_hops_for_dst(const on_encoded_did_t * const DID);
    static UInt8 * device_hops_field(const on_encoded_did_t * const DID);
    static void update_device_hops(const on_encoded_did_t * const DID,
      const UInt8 HOPS_TAKEN);
#endif // ifdef _ONE_NET_MULTI_HOP //

static one_net_status_t build_txn_data_pkt(const UInt8 TYPE,
  on_txn_t * const txn);

static void send_keep_alive(void);
static one_net_status_t send_status_resp(const on_encoded_did_t * const DST);
static one_net_status_t send_settings_resp(const on_encoded_did_t * const DST);

#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_single_data_hdlr(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn, const UInt8 HOPS_TAKEN);
    one_net_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS,
      const UInt8 HOPS_TAKEN);
    static one_net_status_t single_nack_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const UInt8 * const RAW_PLD_DATA,
      const UInt8 HOPS);
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_single_data_hdlr(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn);
    one_net_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS);
    static one_net_status_t single_nack_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const UInt8 * const RAW_PLD_DATA);
#endif // else _ONE_NET_MULTI_HOP is not defined //

static one_net_status_t single_success_hdlr(on_txn_t ** txn,
  const UInt8 * const RAW_PLD_DATA);
static one_net_status_t single_fail_hdlr(on_txn_t ** txn,
  const UInt8 * const RAW_PLD_DATA);

#ifndef _ONE_NET_SIMPLE_CLIENT
    #ifdef _ONE_NET_MULTI_HOP
        one_net_status_t on_client_b_s_data_hdlr(const UInt8 PID,
          const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
          on_txn_t ** txn, const UInt8 HOPS_TAKEN);
        one_net_status_t on_client_b_s_txn_hdlr(on_txn_t ** txn,
          const UInt8 NEXT_NONCE, const one_net_status_t STATUS,
          const UInt8 HOPS_TAKEN);
    #else // ifdef _ONE_NET_MULTI_HOP //
        one_net_status_t on_client_b_s_data_hdlr(const UInt8 PID,
          const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
          on_txn_t ** txn);
        one_net_status_t on_client_b_s_txn_hdlr(on_txn_t ** txn,
          const UInt8 NEXT_NONCE, const one_net_status_t STATUS);
    #endif // else _ONE_NET_MULTI_HOP is not defined //

    static one_net_status_t handle_extended_block_admin_msg(
      const UInt8 * const DATA, const client_txn_t * const BLOCK_TXN);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

void on_client_data_rate_hdlr(const UInt8 RATE,
  const on_encoded_did_t * const DID, const UInt8 RESULT);

static one_net_status_t handle_admin_pkt(const on_encoded_did_t * const SRC,
  const UInt8 * const DATA, const UInt8 DATA_LEN);

#ifndef _ONE_NET_SIMPLE_CLIENT
    static one_net_status_t handle_extended_single_admin_pkt(
      const on_encoded_did_t * const SRC, const UInt8 * const DATA,
      const UInt8 DATA_LEN);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

static BOOL look_for_invite(void);
  
static void save_param(void);

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
    one_net_status_t status;
	
#ifdef _ENHANCED_INVITE
    if(min_channel > max_channel ||
	   max_channel > ONE_NET_MAX_CHANNEL)
	{
		return ONS_BAD_PARAM;
	}
#endif

    client_joined_network = FALSE;

    #ifdef _STREAM_MESSAGES_ENABLED
        if(!INVITE_KEY
          || SINGLE_BLOCK_ENCRYPT_METHOD != ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32
          || STREAM_ENCRYPT_METHOD != ONE_NET_STREAM_ENCRYPT_XTEA8)
    #else // ifdef _STREAM_MESSAGES_ENABLED
        if(!INVITE_KEY
          || SINGLE_BLOCK_ENCRYPT_METHOD != ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32)
    #endif // else _STREAM_MESSAGES_ENABLED is not defined //
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    on_base_param = (on_base_param_t *)nv_param;

    on_base_param->version = ON_PARAM_VERSION;
    on_base_param->channel = one_net_prand(one_net_tick(), ONE_NET_MAX_CHANNEL);
    one_net_memmove(&(on_base_param->current_key), *INVITE_KEY,
      sizeof(on_base_param->current_key));
    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    on_base_param->single_block_encrypt = SINGLE_BLOCK_ENCRYPT_METHOD;
	
	// TODO - 2/10/2011 - Find out who where fragment delays are used.  Only for streams?
	#ifdef _STREAM_MESSAGES_ENABLED
        on_base_param->stream_encrypt = STREAM_ENCRYPT_METHOD;
	#endif
    #ifndef _ONE_NET_SIMPLE_CLIENT
        on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
        on_base_param->fragment_delay_high
          = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

    master->settings.flags = 0x00;
    master->settings.master_data_rate = ONE_NET_DATA_RATE_38_4;
    master->device.expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);
    master->device.last_nonce = one_net_prand(one_net_tick() - 1, ON_MAX_NONCE);
    master->device.send_nonce = 0;

    #ifdef _ONE_NET_MULTI_HOP
        master->device.max_hops = 0;
    #endif // ifdef _ONE_NET_MULTI_HOP //

    // this needs to be done first to set the on_base_param pointer
    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status;
    } // if initializing the internals failed //


    client_joined_network = FALSE;
    client_looking_for_invite = TRUE;

#ifdef _ENHANCED_INVITE
    client_invite_timed_out = FALSE;

	low_invite_channel = min_channel;
	high_invite_channel = max_channel;
    on_base_param->channel = low_invite_channel;
	
	if(timeout_time > 0)
	{
		ont_set_timer(ONT_INVITE_TIMER, timeout_time * 1000);
	}	
#endif
	
    ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_SCAN_CHANNEL_TIME);
    on_state = ON_JOIN_NETWORK;

    return ONS_SUCCESS;
} // one_net_client_look_for_invite //


/*!
    \brief Initializes the CLIENT to run in a network that it has previously
      joined.

    If the CLIENT has not yet joined a network, one_net_client_look_for_invite
    needs to be called instead of this function.

    \param[in] PARAM The parameters that were saved to non-volatile memory.
    \param[in] PARAM_LEN The sizeof PARAM in bytes.

    \return ONS_SUCCESS If initializing the CLIENT was successful
            ONS_BAD_PARAM If any of the parameters are invalid
*/
one_net_status_t one_net_client_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN)
{
    const on_base_param_t * const BASE_PARAM = (const on_base_param_t *)PARAM;
    const on_master_t * const MASTER_PARAM
      = (const on_master_t *)(PARAM + sizeof(on_base_param_t));

    one_net_status_t status;

    if(!PARAM || PARAM_LEN != ON_NV_PARAM_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(BASE_PARAM->version != ON_PARAM_VERSION
      || BASE_PARAM->crc != one_net_compute_crc(PARAM
      + sizeof(BASE_PARAM->crc), PARAM_LEN - sizeof(on_base_param->crc),
      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER))
    {
        return ONS_INVALID_DATA;
    } // if the parameter version does not match or crc's don't match //

    if(!(MASTER_PARAM->settings.flags & ON_JOINED))
    {
        return ONS_NOT_JOINED;
    } // if not connected //

    one_net_memmove(nv_param, PARAM, sizeof(nv_param));

    on_base_param = (on_base_param_t *)nv_param;

    if((status = init_internal()) != ONS_SUCCESS)
    {
        return status;
    } // if initializing the internals failed //

    on_state = ON_LISTEN_FOR_DATA;
    client_joined_network = TRUE;
	client_looking_for_invite = FALSE;
	
    //dje: wired around these for boards that do not have CLI
    // November 10, 2010
    #ifdef _ONE_NET_EVAL
        // Derek_S 10/25/2010, 11/4/2010
        //
        // save data for use by the CLI
        //
        save_param(); // Derek_S 11/4/2010 - needed for CLI
        return on_decode(&client_did[0], &(on_base_param->sid[ON_ENCODED_NID_LEN]), sizeof(one_net_raw_did_t));

    #else
        save_param(); // TO DO : Is this call to save_param() needed?
        return ONS_SUCCESS;
    #endif
} // one_net_client_init //



#ifdef _ONE_NET_EVAL
/*!
    \brief Returns the channel the CLIENT is on.  Only needed for printouts?

    If the CLIENT has not yet picked a channel, ONE_NET_NUM_CHANNELS is
    returned.

    \param void

    \return The channel the CLIENT is on or ONE_NET_NUM_CHANNELS
*/
UInt8 one_net_client_get_channel(void)
{
    if(!client_joined_network)
    {
        return ONE_NET_NUM_CHANNELS;
    } // if the channel has not yet been picked //

    return on_base_param->channel;
} // one_net_client_get_channel //
#endif


/*!
    \brief Returns the raw MASTER DID.

    \param[out] master_did Location to return raw MASTER DID

    \return void
*/
void one_net_client_raw_master_did(one_net_raw_did_t * const master_did)
{
    if(!master_did)
    {
        return;
    } // if the parameter is invalid //

    on_decode(*master_did, master->device.did, ON_ENCODED_DID_LEN);
} // one_net_client_raw_master_did //


#ifndef _ONE_NET_SIMPLE_CLIENT
    /*!
        \brief Interface to start a block transaction.

        Starts a block transaction (send or receive) if the resources are
        available.

        \param[in] TYPE The type of transaction to be requested.  This can be
                        either BLOCK or ON_STREAM.
        \param[in] SEND TRUE if this device is sending the data.
                        FALSE if this device is requesting to receive the data.
        \param[in] DATA_TYPE The type of data to transfer.
        \param[in] LEN The total number of bytes to transfer (if a block txn).
        \param[in] PRIORITY The priority of the transaction.
        \param[in] DID 0 If this device is initiating a transaction.
                       destination did if the device is sending this
                       transaction in response to a packet received from
                       this device.
        \param[in] SRC_UNIT The unit of this device making the request.  This
                            is used to send to the data to the peers if the data
                            is to be sent to the peers.

        \return ONS_SUCCESS If there are resources available and the request
                  will be made.
                ONS_BAD_PARAM If any of the parameters are invalid.
                ONS_RSRC_FULL If the resources are unavailable.
                ALREADY_IN_PROGRESS If a transaction of this type with the
                  device is already in progress.
                ONS_INTERNAL_ERR If something failed that should not have.
    */
    one_net_status_t one_net_client_block_stream_request(const UInt8 TYPE,
      const BOOL SEND, const UInt16 DATA_TYPE, const UInt16 LEN,
      const UInt8 PRIORITY, const one_net_raw_did_t * const DID,
      const UInt8 SRC_UNIT)
    {
        client_txn_t * client_txn;
        UInt8 admin_type;

        switch(TYPE)
        {
            case ON_BLOCK:
            {
                client_txn = &on_block_txn;
                client_txn->txn.remaining = LEN;

                // if this device is sending, then the other device is
                // receiving (that is why the admin types are reversed
                // from what might be expected).
                if(SEND)
                {
                    if(PRIORITY == ONE_NET_HIGH_PRIORITY)
                    {
                        admin_type = ON_RECV_BLOCK_HIGH;
                    } // if a high priority transaction //
                    else
                    {
                        admin_type = ON_RECV_BLOCK_LOW;
                    } // else a low priority transaction //
                } // if sending the transactin //
                else
                {
                    if(PRIORITY == ONE_NET_HIGH_PRIORITY)
                    {
                        admin_type = ON_SEND_BLOCK_HIGH;
                    } // if a high priority transaction //
                    else
                    {
                        admin_type = ON_SEND_BLOCK_LOW;
                    } // else a low priority transaction //
                } // else receiving the transaction //
                break;
            } // ON_BLOCK case //

            case ON_STREAM:
            {
                client_txn = &on_stream_txn;

                // when remaining is !0, it means for the stream to continue.
                // when remaining is 0, it means for the stream to end.
                client_txn->txn.remaining = 1;

                // if this device is sending, then the other device is
                // receiving (that is why the admin types are reversed
                // from what might be expected).
                if(SEND)
                {
                    if(PRIORITY == ONE_NET_HIGH_PRIORITY)
                    {
                        admin_type = ON_RECV_STREAM_HIGH;
                    } // if a high priority transaction //
                    else
                    {
                        admin_type = ON_RECV_STREAM_LOW;
                    } // else a low priority transaction //
                } // if sending the transactin //
                else
                {
                    if(PRIORITY == ONE_NET_HIGH_PRIORITY)
                    {
                        admin_type = ON_SEND_STREAM_HIGH;
                    } // if a high priority transaction //
                    else
                    {
                        admin_type = ON_SEND_STREAM_LOW;
                    } // else a low priority transaction //
                } // else receiving the transaction //
                break;
            } // ON_STREAM case //

            default:
            {
                return ONS_BAD_PARAM;
                break;
            } // default case //
        } // switch(TYPE) //

        if(!LEN || PRIORITY < ONE_NET_LOW_PRIORITY
          || PRIORITY > ONE_NET_HIGH_PRIORITY
          || (SRC_UNIT != ONE_NET_DEV_UNIT && SRC_UNIT >= ONE_NET_NUM_UNITS))
        {
            return ONS_BAD_PARAM;
        } // if any parameters are invalid //

        if(!(master->settings.flags & ON_JOINED))
        {
            return ONS_NOT_JOINED;
        } // if the device has not joined the network yet //

        if(client_txn->txn.priority != ONE_NET_NO_PRIORITY)
        {
            return ONS_RSRC_FULL;
        } // if the resource is in use //

        client_txn->txn.send = SEND;
        client_txn->requesting = TRUE;

        // even if receiving the stream, store the did into
        // ONE_NET_ENCODED_DST_DID_IDX so there is only 1 place to look later.
        // If receiving, this packet will not be overwritten, and if sending,
        // then the data is already in the correct spot, making it less work to
        // move later.
        if(DID)
        {
            on_encode(&(client_txn->txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]), *DID,
              ON_ENCODED_DID_LEN);
        } // if the device is responding to another transaction //
        else
        {
            one_net_memmove(&(client_txn->txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
              master->device.did, sizeof(master->device.did));
        } // else the device initiated the transaction //

        client_txn->txn.msg_type = ON_ADMIN_MSG;
        client_txn->txn.pkt[ON_ADMIN_TYPE_IDX] = admin_type;
        one_net_int16_to_byte_stream(DATA_TYPE,
          &(client_txn->txn.pkt[ON_DATA_IDX + ON_BLOCK_STREAM_DATA_TYPE_IDX]));
        one_net_int16_to_byte_stream(LEN,
          &(client_txn->txn.pkt[ON_DATA_IDX + ON_BLOCK_LEN_IDX]));
        client_txn->txn.data_len = ON_MAX_ADMIN_PLD_LEN;

        client_txn->txn.retry = 0;
        client_txn->txn.priority = PRIORITY;

        return ONS_SUCCESS;
    } // one_net_client_block_stream_request //


    /*!
        \brief Ends the stream transaction.

        \param void

        \return ONS_SUCCESS if the end stream is scheduled
                ONS_TXN_DOES_NOT_EXIST if the transaction does not exist.
    */
    one_net_status_t one_net_client_end_stream(void)
    {
        if(on_stream_txn.txn.priority == ONE_NET_NO_PRIORITY)
        {
            return ONS_TXN_DOES_NOT_EXIST;
        } // if the parameter is invalid //

        // mark the end of the stream
        on_stream_txn.txn.remaining = 0;

        return ONS_SUCCESS;
    } // one_net_client_end_stream //


    /*!
        \brief Sends the stream key query to the MASTER

        If the resource is not available, a timer is started to keep querying
        until the key is updated.

        \param void

        \return ONS_SUCCESS If setting up the query was successful
                ONS_RSRC_FULL If the resources are unavailable
    */
    void one_net_client_stream_key_query(void)
    {
        ont_set_timer(ONT_STREAM_KEY_TIMER, ONE_NET_STREAM_KEY_QUERY_INTERVAL);

        if(on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
        {
            return;
        } // if the resource is not available //

        on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
        on_single_txn.txn.retry = 0;

        on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

        on_single_txn.txn.msg_type = ON_ADMIN_MSG;
        on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_STREAM_KEY_QUERY;

        one_net_memmove(&(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
          master->device.did, sizeof(master->device.did));
    } // one_net_client_stream_key_query //

#endif // ifndef _ONE_NET_SIMPLE_CLIENT //


/*!
    \brief The main function for the ONE-NET CLIENT.

    \param void

    \return The number of ticks the device can sleep for.
*/
tick_t one_net_client(void)
{
    // The current transaction
    static on_txn_t * txn = 0;

    // The time the application can sleep for.
    tick_t sleep_time = 0;

    // Do the appropriate action for the state the device is in.
    switch(on_state)
    {
#ifdef _IDLE
		case ON_IDLE:
		{
			break;
		}
#endif
		
        case ON_LISTEN_FOR_DATA:
        {
            //
            // Listen for a new transaction.
            // Also check to see if there are any events
            // associated with timers that need attention
            //

            if(ont_inactive_or_expired(ONT_STAY_AWAKE_TIMER))
            {
                //
                // we are not waiting on ONT_STAY_AWAKE_TIME
                //
                #ifndef _ONE_NET_SIMPLE_CLIENT
                    BOOL requesting = FALSE;
                #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

                txn = 0;

                //
                // NO_PRIORITY for single txn means nothing is queued
                // nothing waiting to be sent, not waiting for ACK
                //
                if(on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
                {
                    //
                    // a transaction is active,
                    // see what we need to do next.
                    //
                    txn = &(on_single_txn.txn);

#ifdef _ONE_NET_MULTI_HOP
                    if(txn->pkt[ONE_NET_ENCODED_PID_IDX]
                      == ONE_NET_ENCODED_DATA_RATE_TEST
                      || txn->pkt[ONE_NET_ENCODED_PID_IDX]
                      == ONE_NET_ENCODED_MH_DATA_RATE_TEST)
#else
                    if(txn->pkt[ONE_NET_ENCODED_PID_IDX]
                      == ONE_NET_ENCODED_DATA_RATE_TEST)
#endif
                    {
                        if(on_encoded_did_equal(
                          (const on_encoded_did_t * const)
                          &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                          &ON_ENCODED_BROADCAST_DID))
                        {
                            // Since the broadcast address is set as the
                            // destination, this device is receiving the test.
                            on_state = ON_INIT_RX_DATA_RATE;
                            break;
                        } // if the destination is the broadcast address //
                        else
                        {
                            // This device is sending the test.
                            on_state = ON_INIT_SEND_DATA_RATE;
                            break;
                        } // else the dst is not the broadcast address //
                    } // if it's a data rate test //
                } // if a single transaction is waiting //
                #ifndef _ONE_NET_SIMPLE_CLIENT
                    else if(ont_active(ONT_STREAM_KEY_TIMER)
                      && ont_expired(ONT_STREAM_KEY_TIMER))
                    {
                        //
                        // we are waiting for the stream key
                        // and the timer has expired, so
                        // reset the stream key timer and reissue
                        // the query for the stream key
                        //
                        one_net_client_stream_key_query();
                    } // else if time to query for the stream key //

                    if(on_stream_txn.txn.priority != ONE_NET_NO_PRIORITY
                      && (!txn || (on_stream_txn.txn.priority > txn->priority
                      && ont_inactive_or_expired(
                      on_stream_txn.txn.next_txn_timer))))
                    {
                        txn = &(on_stream_txn.txn);
                        requesting = on_stream_txn.requesting;
                    } // if a stream transaction has higher priority //

                    if(on_block_txn.txn.priority != ONE_NET_NO_PRIORITY && (!txn
                      || (on_block_txn.txn.priority > txn->priority
                      && ont_inactive_or_expired(
                      on_block_txn.txn.next_txn_timer))))
                    {
                        txn = &(on_block_txn.txn);
                        requesting = on_block_txn.requesting;
                    } // if a block transaction has higher priority //
                #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

                #ifndef _ONE_NET_SIMPLE_CLIENT
                    if(txn && (txn == &(on_single_txn.txn) || requesting))
                #else // ifndef _ONE_NET_SIMPLE_CLIENT //
                    if(txn && txn == &(on_single_txn.txn))
                #endif // else _ONE_NET_SIMPLE_CLIENT is defined //
                {
                    // store the raw did in case the txn fails
                    one_net_raw_did_t raw_did;

                    if(on_decode(raw_did,
                      &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      ON_ENCODED_DID_LEN) == ONS_SUCCESS)
                    {
                        if(build_txn_data_pkt(ON_SINGLE, txn) == ONS_SUCCESS)
                        {
                            one_net_set_data_rate(data_rate_for_dst(
                              (const on_encoded_did_t * const)
                              &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX])));
                            on_state = ON_SEND_SINGLE_DATA_PKT;
                            break;
                        } // if building the packet was successful //

                        #ifndef _ONE_NET_SIMPLE_CLIENT
                            if(txn == &(on_single_txn.txn))
                            {
                                // The data is not reliable since building the
                                // txn failed, so don't return it.
                                #ifdef _ONE_NET_MULTI_HOP
                                    on_client_single_txn_hdlr(&txn,
                                      ON_MAX_NONCE + 1, ONS_SINGLE_FAIL, 0);
                                #else // ifdef _ONE_NET_MULTI_HOP //
                                    on_client_single_txn_hdlr(&txn,
                                      ON_MAX_NONCE + 1, ONS_SINGLE_FAIL);
                                #endif // else _ONE_NET_MULTI_HOP not defined //
                            } // if a single transaction //
                            else if(txn == &(on_block_txn.txn))
                            {
                                one_net_client_block_txn_status(ONS_BLOCK_FAIL,
                                  (const one_net_raw_did_t * const)&raw_did);
                            } // if block admin request //
                            else if(txn == &(on_stream_txn.txn))
                            {
                                one_net_client_stream_txn_status(
                                  ONS_STREAM_FAIL,
                                  (const one_net_raw_did_t * const)&raw_did);
                            } // else if stream admin request //
                            // else this should not have happened
                        #else // ifndef _ONE_NET_SIMPLE_CLIENT //
                            // The data is not reliable since building the txn
                            // failed, so don't return it.
                            #ifdef _ONE_NET_MULTI_HOP
                                on_client_single_txn_hdlr(&txn,
                                  ON_MAX_NONCE + 1, ONS_SINGLE_FAIL, 0);
                            #else // ifdef _ONE_NET_MULTI_HOP //
                                on_client_single_txn_hdlr(&txn,
                                  ON_MAX_NONCE + 1, ONS_SINGLE_FAIL);
                            #endif // else _ONE_NET_MULTI_HOP is not defined //
                        #endif // else _ONE_NET_SIMPLE_CLIENT is defined //
                    } // if decoding the dst did was successfull //
                    else
                    {
                        if(txn == &(on_single_txn.txn))
                        {
                            UInt8 priority = txn->priority;

                            txn->priority = ONE_NET_NO_PRIORITY;
                            on_client_net_single_txn_hdlr(ONS_INTERNAL_ERR,
                              0, &(txn->pkt[ON_DATA_IDX]), 0, priority,
                              on_single_txn.txn_id);
                        } // if a single transaction //
                        #ifndef _ONE_NET_SIMPLE_CLIENT
                            else if(txn == &(on_block_txn.txn))
                            {
                                one_net_client_block_txn_status(ONS_BLOCK_FAIL,
                                  0);
                            } // if block admin request //
                            else if(txn == &(on_stream_txn.txn))
                            {
                                one_net_client_stream_txn_status(
                                  ONS_STREAM_FAIL, 0);
                            } // else if stream admin request //
                        #endif // ifdef _ONE_NET_SIMPLE_CLIENT //
                    } // else decoding the dst did was not successful //

                    // building failed, so release the bad txn
                    txn->priority = ONE_NET_NO_PRIORITY;
                    txn = 0;
                } // if a single transaction is occuring //
                #ifndef _ONE_NET_SIMPLE_CLIENT
                    else if(txn && ont_inactive_or_expired(txn->next_txn_timer))
                    {
                        const UInt8 * DATA_PTR;

                        UInt16 len;
                        UInt8 type;

                        if(txn == &(on_block_txn.txn))
                        {
                            type = ON_BLOCK;
                        } // if a block transaction //
                        else if(txn == &(on_stream_txn.txn))
                        {
                            type = ON_STREAM;
                        } // else if it's a stream transaction //
                        else
                        {
                            break;
                        } // else there's a problem with the transaction //

                        if(txn->send)
                        {
                            if((!txn->retry && type != ON_STREAM)
                              || (type == ON_STREAM
                              && on_stream_txn.txn.remaining))
                            {
                                one_net_raw_did_t did;

                                if(on_decode(did,
                                  &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                                  ON_ENCODED_DID_LEN) != ONS_SUCCESS)
                                {
                                    if(type == ON_BLOCK)
                                    {
                                        one_net_client_block_txn_status(
                                          ONS_INTERNAL_ERR,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // if a block transaction //
                                    else
                                    {
                                        one_net_client_stream_txn_status(
                                          ONS_INTERNAL_ERR,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else a stream transaction //

                                    // clear the txn since it is no longer valid
                                    txn->priority = ONE_NET_NO_PRIORITY;
                                    txn = 0;
                                    break;
                                } // if decoding was not successful //

                                if(((DATA_PTR = one_net_client_next_payload(
                                  type, &len,
                                  (const one_net_raw_did_t * const) &did))
                                  == 0 || !len || len > ON_MAX_RAW_PLD_LEN
                                  || (type == ON_BLOCK
                                  && len > txn->remaining)))
                                {
                                    if(type == ON_BLOCK)
                                    {
                                        one_net_client_block_txn_status(
                                          ONS_INVALID_DATA,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // if a block transaction //
                                    else
                                    {
                                        one_net_client_stream_txn_status(
                                          ONS_INVALID_DATA,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else a stream transaction //

                                    txn->priority = ONE_NET_NO_PRIORITY;
                                    txn = 0;
                                    break;
                                } // if data returned from app is not valid //

                                txn->data_len = len;

                                if(type == ON_BLOCK)
                                {
                                    txn->remaining -= len;
                                } // if a block transaction //

                                one_net_memmove(&(txn->pkt[ON_DATA_IDX]),
                                  DATA_PTR, len);
                            } // if the next payload is needed //
                            else if(type == ON_STREAM)
                            {
                                // end the stream transaction

                                // send 0's since field is not used
                                UInt8 data[ON_MAX_ADMIN_PLD_LEN] = {0};

                                // Send the single to end the stream
                                type = ON_SINGLE;
                                on_stream_txn.txn.data_len = sizeof(data);
                                on_stream_txn.txn.pkt[ON_ADMIN_TYPE_IDX]
                                  = ON_END_STREAM;
                                one_net_memmove(
                                  &(on_stream_txn.txn.pkt[ON_DATA_IDX]),
                                  data, sizeof(data));
                            } // else if a stream txn //

                            if(type == ON_SINGLE)
                            {
                                // if it got here, that means a stream
                                // transaction is ending
                                txn->msg_type = ON_ADMIN_MSG;
                            } // if a single txn //

                            if(build_txn_data_pkt(type, txn) != ONS_SUCCESS)
                            {
                                one_net_raw_did_t did;

                                on_decode(did,
                                  &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                                  ON_ENCODED_DID_LEN);

                                if(type == ON_BLOCK)
                                {
                                    one_net_client_block_txn_status(
                                      ONS_INVALID_DATA,
                                      (const one_net_raw_did_t * const)&did);
                                } // if a block transaction //
                                else
                                {
                                    one_net_client_stream_txn_status(
                                      ONS_INVALID_DATA,
                                      (const one_net_raw_did_t * const)&did);
                                } // else a stream transaction //

                                // drop the invalid transaction
                                txn->priority = ONE_NET_NO_PRIORITY;
                                txn = 0;
                                break;
                            } // if building the packet was not successful //
                            else if(type == ON_BLOCK)
                            {
                                on_state = ON_SEND_BLOCK_DATA_PKT;
                            } // else if a block txn //
                            else if(type == ON_STREAM)
                            {
                                on_state = ON_SEND_STREAM_DATA_PKT;
                            } // else if a stream txn //
                            else if(type == ON_SINGLE)
                            {
                                on_state = ON_SEND_SINGLE_DATA_PKT;
                            } // else if a single txn //

                            one_net_set_data_rate(data_rate_for_dst(
                              (const on_encoded_did_t * const)
                              &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX])));
                            break;
                        } // if sending the transaction //
                        else
                        {
                            txn->retry++;
                            if(txn->retry >= ON_MAX_RETRY)
                            {
                                one_net_raw_did_t did;

                                // even though this is the receive section,
                                // the did was stored in the dst section instead
                                // of the sender to keep it in 1 location, and
                                // to lesson the amount of work that needs to be
                                // done when accessing the did.
                                if(on_decode(did,
                                  &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                                  ON_ENCODED_DID_LEN) == ONS_SUCCESS)
                                {
                                    if(txn == &(on_stream_txn.txn))
                                    {
                                        one_net_client_stream_txn_status(
                                          ONS_STREAM_FAIL,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // if a stream transaction //
                                    else if(txn->remaining)
                                    {
                                        one_net_client_block_txn_status(
                                          ONS_BLOCK_FAIL,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else if blocks remain //
                                    else
                                    {
                                        // no blocks remaining to be
                                        // received so the
                                        // ONE_NET_ENCODED_BLOCK_TXN_ACK
                                        // must have been missed.
                                        one_net_client_block_txn_status(
                                          ONS_BLOCK_END,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else rxing & no blocks remained //
                                } // if decoding did was successful //
                                else
                                {
                                    if(txn == &(on_stream_txn.txn))
                                    {
                                        one_net_client_stream_txn_status(
                                          ONS_INTERNAL_ERR,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // if a stream transaction //
                                    else if(txn->remaining)
                                    {
                                        one_net_client_block_txn_status(
                                          ONS_INTERNAL_ERR,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else if blocks remain //
                                    else
                                    {
                                        // no blocks remaining to be
                                        // received so the
                                        // ONE_NET_ENCODED_BLOCK_TXN_ACK
                                        // must have been missed.
                                        one_net_client_block_txn_status(
                                          ONS_INTERNAL_ERR,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else rxing & no blocks remained //
                                } // else decoding the did was not successful //

                                txn->priority = ONE_NET_NO_PRIORITY;
                                txn = 0;
                                break;
                            } // if the transaction failed //
                            else
                            {
                                on_update_next_txn_time(txn);
                            } // else try again //
                        } // else receiving the transaction //
                    } // else if a block or stream transaction is occuring //
                #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
            } // if device not waiting for something && single txn pending //

            #ifndef _ONE_NET_SIMPLE_CLIENT
                //
                // becasue we are not in the middle of a transfer,
                // time to actually listen for data pkts, ...
                //
                switch(on_rx_data_pkt(&ON_ENCODED_BROADCAST_DID, &txn))
                {
                    case ONS_BLOCK_FAIL:
                    {
                        on_block_txn.txn.priority = ONE_NET_NO_PRIORITY;
                        txn = 0;
                        break;
                    } // block end/fail case //

                    case ONS_STREAM_END:                // fall through
                    case ONS_STREAM_FAIL:
                    {
                        if(on_state != ON_SEND_SINGLE_DATA_PKT)
                        {
                            on_stream_txn.txn.priority = ONE_NET_NO_PRIORITY;
                            txn = 0;
                        } // if not sending the end stream admin pkt //
                        break;
                    } // block end/fail case //

                    default:
                    {
                        break;
                    } // default case //
                } // switch on rx_data_pkt result //
            #else // ifndef _ONE_NET_SIMPLE_CLIENT
                on_rx_data_pkt(&ON_ENCODED_BROADCAST_DID, &txn);
            #endif // else _ONE_NET_SIMPLE_CLIENT is defined //
            break;
        } // listen_for_data case //

        case ON_JOIN_NETWORK:
        {
            //TODO: waiting to join a network.

            if(look_for_invite())
            {
                ont_stop_timer(ONT_STAY_AWAKE_TIMER);
                txn = &(on_single_txn.txn);
                if(build_txn_data_pkt(ON_SINGLE, txn) == ONS_SUCCESS)
                {
                    on_state = ON_SEND_SINGLE_DATA_PKT;
                } // if building the packet was successful //
                else
                {
                    // Should never get here, but if it does, then an extremely
                    // serious error occured since this device cannot go back
                    // to listening for the invite since the key has been
                    // been changed, and if building the packet didn't work the
                    // first time, it won't work again, so there is no point in
                    // trying to send the status response again.  Don't store
                    // the old key since that will take too much memory for
                    // something that should never happen.
                } // else building the pkt was not successful //
            } // if the invite was found //
            break;
        } // join network case //

        default:
        {
            //
            // one_net() will handle the common state machine states based on
            // the transaction passed in and return true if that transactiom
            // completed.
            //
            if(one_net(&txn) && txn)
            {
                // Only clr the txn if the CLIENT joined the network.
                // If the CLIENT has not joined the network, the settings
                // response message must be sent to the MASTER again to
                // complete the join.
                if(master->settings.flags & ON_JOINED)
                {
                    #ifndef _ONE_NET_SIMPLE_CLIENT
                        if(txn == &(on_block_txn.txn)
                          && on_block_txn.requesting)
                        {
                            on_block_txn.requesting = FALSE;
                            on_update_next_txn_time(&(on_block_txn.txn));
                            on_block_txn.txn.retry = 0;
                        } // if block is requesting a transaction //
                        else if(txn == &(on_stream_txn.txn)
                          && on_stream_txn.requesting)
                        {
                            on_stream_txn.requesting = FALSE;
                            on_update_next_txn_time(&(on_stream_txn.txn));
                            on_stream_txn.txn.retry = 0;
                        } // else if stream is requesting a transaction //

                        if(!report_data_rate_results)
                        {
                            if(txn->msg_type != ON_APP_MSG)
                            {
                                txn->priority = ONE_NET_NO_PRIORITY;
                            } // if not an application message //
                            txn = 0;
                        } // else if the transaction is really over //
                        else
                        {
                            report_data_rate_results = FALSE;
                        } // else need to report the data rate results //
                    #else // infdef _ONE_NET_SIMPLE_CLIENT //
                        if(!report_data_rate_results)
                        {
                            if(txn->msg_type != ON_APP_MSG)
                            {
                                txn->priority = ONE_NET_NO_PRIORITY;
                            } // if not an application message //
                            txn = 0;
                        } // if not reporting data rate results //
                        else
                        {
                            report_data_rate_results = FALSE;
                        } // else reporting data rate results //
                    #endif // else _ONE_NET_SIMPLE_CLIENT was defined //
                } // if the CLIENT has joined the network //
                else
                {
                    // try sending the transaction again since the device needs
                    // to complete the join.  The packet may need to be rebuilt
                    // if the MASTER corrected the nonce to use
                    on_single_txn.txn.priority = ONE_NET_NO_PRIORITY;
                    send_status_resp(
                      (const on_encoded_did_t * const)&(master->device.did));
                    txn = &(on_single_txn.txn);
                    if(build_txn_data_pkt(ON_SINGLE, txn) != ONS_SUCCESS)
                    {
                        on_state = ON_JOIN_NETWORK;
                    } // if building the packet failed //
                    else
                    {
                        on_state = ON_SEND_SINGLE_DATA_PKT;
                    } // else building the packet was successful //
                } // else the device has not joined the network //
            } // if transaction is complete //
            break;
        } // default case //
    } // switch(on_state) //

    // TODO: JAY? calculate the sleep time? yes
    // Get the time the application can sleep for,
    // and see what house cleaning we can do.

    switch(on_state)
    {
#ifdef _IDLE
		case ON_IDLE:
		{
			break;
		}
#endif
		
        case ON_LISTEN_FOR_DATA:
        {
            // since we are not busy, see what we can do
            //TODO: JAY? what does saving settings have to do with calc'g sleep time?
            if(save)
            {
                save_param();
                save = FALSE;
            } // if need to save settings //

            if(removed)
            {
                one_net_xtea_key_t empty_key = {0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00};

                #ifdef _STREAM_MESSAGES_ENABLED // Stream Messages Enabled //
				    #ifdef _ENHANCED_INVITE
                        one_net_client_look_for_invite(&empty_key,
                          ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
                          ONE_NET_STREAM_ENCRYPT_XTEA8, 0,
					      ONE_NET_MAX_CHANNEL, 0);
				    #else
                        one_net_client_look_for_invite(&empty_key,
                          ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
                          ONE_NET_STREAM_ENCRYPT_XTEA8);				
				    #endif
                #else // Stream Messages Not Enabled //
				    #ifdef _ENHANCED_INVITE
                        one_net_client_look_for_invite(&empty_key,
                          ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32, 0,
					      ONE_NET_MAX_CHANNEL, 0);
				    #else
                        one_net_client_look_for_invite(&empty_key,
                         ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32);				
				    #endif
                #endif
				
				// reset the did to broadcast/unjoined
				one_net_memmove(&(on_base_param->sid[ON_ENCODED_NID_LEN]),
				    ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
				
                save_param();
                removed = FALSE;
                on_state = ON_INIT_STATE;

                //dje: wired around oncli stuff for non-eval boards
                //     since they don't have cli
                //     November 10, 2010
                #ifdef _ONE_NET_EVAL
                    // Derek_S 11/3/2010
                    oncli_send_msg("Removed from network by master.  No longer joined.\n");
                    oncli_send_msg("Now resetting the device and looking for an invite.\n");
                #endif

                one_net_client_client_remove_device();

                #ifdef _ONE_NET_EVAL
                    oncli_reset_client();
                    oncli_print_prompt();
                #endif
                save_param();
                break;
            } // if the device was removed from the network //

            if(ont_active(ONT_STAY_AWAKE_TIMER)
              || on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
            {
                sleep_time = 0;
            } // if stay awake time //
            else if(confirm_key_update
              || ont_inactive_or_expired(ONT_KEEP_ALIVE_TIMER))
            {
                send_keep_alive();
                sleep_time = 0;
            } // else if it is time to send a keep alive //
            else
            {
                sleep_time = ont_get_timer(ONT_KEEP_ALIVE_TIMER);

                #ifndef _ONE_NET_SIMPLE_CLIENT
                    if(on_block_txn.txn.priority != ONE_NET_NO_PRIORITY &&
                      ont_get_timer(on_block_txn.txn.next_txn_timer)
                      < sleep_time)
                    {
                        sleep_time
                          = ont_get_timer(on_block_txn.txn.next_txn_timer);
                    } // if a block happens first //

                    if(on_stream_txn.txn.priority != ONE_NET_NO_PRIORITY &&
                      ont_get_timer(on_stream_txn.txn.next_txn_timer)
                      < sleep_time)
                    {
                        sleep_time
                          = ont_get_timer(on_stream_txn.txn.next_txn_timer);
                    } // if a stream happens first //
                #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
            } // else wait for keep alive time //
            break;
        } // listen for data case //

        case ON_SEND_PKT:                               // fall through
        case ON_SEND_SINGLE_DATA_PKT:                   // fall through
        case ON_SEND_SINGLE_DATA_RESP:                  // fall through
        #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_SEND_BLOCK_DATA_PKT:                // fall through
            case ON_SEND_BLOCK_DATA_RESP:               // fall through
            case ON_SEND_STREAM_DATA_PKT:               // fall through
            case ON_SEND_STREAM_DATA_RESP:              // fall through
        #endif // _ONE_NET_SIMPLE_CLIENT //
        case ON_SEND_DATA_RATE:                         // fall through
        case ON_SEND_DATA_RATE_RESP:
        {
            sleep_time = ont_get_timer(ONT_GENERAL_TIMER);
            break;
        } // send packet, send single data, send single data resp case //

        case ON_SEND_PKT_WRITE_WAIT:                    // fall through
        case ON_SEND_SINGLE_DATA_WRITE_WAIT:            // fall through
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:       // fall through
        case ON_WAIT_FOR_SINGLE_DATA_RESP:              // fall through
        case ON_WAIT_FOR_SINGLE_DATA_END:               // fall through
        case ON_INIT_SEND_DATA_RATE:                    // fall through
        #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_SEND_BLOCK_DATA_WRITE_WAIT:         // fall through
            case ON_WAIT_FOR_BLOCK_DATA_RESP:           // fall through
            case ON_SEND_BLOCK_DATA_RESP_WRITE_WAIT:    // fall through
            case ON_WAIT_FOR_BLOCK_DATA_END:            // fall through
            case ON_SEND_STREAM_DATA_WRITE_WAIT:        // fall through
            case ON_WAIT_FOR_STREAM_DATA_RESP:          // fall through
            case ON_SEND_STREAM_DATA_RESP_WRITE_WAIT:   // fall through
        #endif
        case ON_SEND_DATA_RATE_WRITE_WAIT:              // fall through
        case ON_RX_DATA_RATE_RESP:                      // fall through
        case ON_INIT_RX_DATA_RATE:                      // fall through
        case ON_RX_DATA_RATE:                           // fall through
        case ON_SEND_DATA_RATE_RESP_WRITE_WAIT:
        {
            //TODO: JAY? does this mean don't sleep becasue we are waiting for a resp? where is look_for_pkt called?
            sleep_time = 0;
            break;
        } // wait for single data resp, wait for single data end case //

        case ON_JOIN_NETWORK:
        {
            sleep_time = 0;
            break;
        } // join network case //

        case ON_INIT_STATE:
        {
            // sleep as long as possible since device hasn't been initialized //
            sleep_time = (tick_t)-1;
            break;
        } // initial state case //

        default:
        {
            sleep_time = 0;
            on_state = ON_LISTEN_FOR_DATA;
            break;
        } // default case //
    } // switch(state) to get the time the app can sleep for //

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

/*!
    \brief Initializes internal data structures.

    This will also initialize the base one_net functionality.

    \param void

    \return ONS_SUCCESS if the internals were successfully initialized
            ONS_INTERNAL_ERR if initializing the net layer was not successful
*/
static one_net_status_t init_internal(void)
{
    on_pkt_hdlr_set_t pkt_hdlr;
    one_net_status_t status;

    UInt8 i;

#ifdef _PEER
    // initialize the network layer
    if((status = on_client_net_init(nv_param + sizeof(on_base_param_t)
      + sizeof(on_master_t), sizeof(nv_param) - sizeof(on_base_param_t)
      - sizeof(on_master_t))) != ONS_SUCCESS)
    {
        return ONS_INTERNAL_ERR;
    } // if initializing the net layer was not successful //
#endif

    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        sending_dev_list[i].lru = i;
        one_net_memmove(sending_dev_list[i].sender.did,
          ON_ENCODED_BROADCAST_DID, sizeof(sending_dev_list[i].sender.did));
        sending_dev_list[i].sender.expected_nonce
          = one_net_prand(one_net_tick(), ON_MAX_NONCE);
        sending_dev_list[i].sender.last_nonce = ON_MAX_NONCE + 1;
        sending_dev_list[i].sender.send_nonce = 0;

        #ifdef _ONE_NET_MULTI_HOP
            sending_dev_list[i].sender.max_hops = 0;
        #endif // ifdef _ONE_NET_MULTI_HOP //
    } // loop to initialize sending device list //

    // initialize ONE-NET
    pkt_hdlr.single_data_hdlr = &on_client_single_data_hdlr;
    pkt_hdlr.single_txn_hdlr = &on_client_single_txn_hdlr;
    #ifndef _ONE_NET_SIMPLE_CLIENT
        pkt_hdlr.block_data_hdlr = &on_client_b_s_data_hdlr;
        pkt_hdlr.block_txn_hdlr = &on_client_b_s_txn_hdlr;
        pkt_hdlr.stream_data_hdlr = &on_client_b_s_data_hdlr;
        pkt_hdlr.stream_txn_hdlr = &on_client_b_s_txn_hdlr;
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
    pkt_hdlr.data_rate_hdlr = &on_client_data_rate_hdlr;
    one_net_init(&pkt_hdlr);

    return ONS_SUCCESS;
} // init_internal //


/*!
    \brief Returns the encoded MASTER DID.

    \param[out] master_did Location to return encoded MASTER DID

    \return void
*/
void on_client_encoded_master_did(on_encoded_did_t * const master_did)
{
    if(!master_did)
    {
        return;
    } // if the parameter is invalid //

    one_net_memmove(*master_did, master->device.did, ON_ENCODED_DID_LEN);
} // on_client_encoded_master_did //


/*!
    \brief Returns TRUE if the device should send messages that are sent to
      peers to the MASTER as well.

    \param void

    \return TRUE if messages sent to peers should be sent to the MASTER as well.
            FALSE if messages sent to peers should not be sent to the MASTER.
*/
BOOL on_client_send_to_master(void)
{
    return (BOOL)(master->settings.flags & ON_SEND_TO_MASTER);
} // on_client_send_to_master //


/*!
    \brief Sends a single data.

    Sends a single data packet to the specified destination.

    \param[in] DATA The data to send.
    \param[in] DATA_LEN The length of data (in bytes).
    \param[in] PRIORITY The priority on the transaction.
    \param[in] DST The device to send DATA to.
    \param[in] TXN_ID The transaction id that will be returned to the network
      layer when the transaction completes.

    \return ONS_SUCCESS If the single data has been queued successfully.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_NOT_JOINED If the CLIENT has not joined the network yet
            ONS_RSRC_FULL If no resources are currently available to handle the
              request.
            See on_build_data_pkt for more possible return codes.
*/
one_net_status_t on_client_send_single(const UInt8 * const DATA,
  const UInt8 DATA_LEN, const UInt8 PRIORITY,
  const on_encoded_did_t * const DST, const UInt8 TXN_ID)
{
    one_net_status_t status;

    if(!DATA || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN
      || PRIORITY < ONE_NET_LOW_PRIORITY
      || PRIORITY > ONE_NET_HIGH_PRIORITY

      || !DST)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    if(!(master->settings.flags & ON_JOINED))
    {
        return ONS_NOT_JOINED;
    } // if the device has not joined the network yet //

    if(on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
    {
        //
        // this seems to be where we make sure that any single
        // data transaction (resceiving or sending) is complete
        // before we start sending a new transaction,
        //
        return ONS_RSRC_FULL;
    } // if the resource is in use //

    on_single_txn.txn_id = TXN_ID;
    on_single_txn.txn.priority = PRIORITY;
    on_single_txn.txn.retry = 0;
    #ifndef _ONE_NET_SIMPLE_CLIENT
        on_single_txn.requesting = FALSE;
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

    one_net_memmove(&(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
      *DST, ON_ENCODED_DID_LEN);

    on_single_txn.txn.msg_type = ON_APP_MSG;
    one_net_memmove(&(on_single_txn.txn.pkt[ON_DATA_IDX]), DATA, DATA_LEN);
    on_single_txn.txn.data_len = DATA_LEN;

    return ONS_SUCCESS;
} // on_client_send_single //


/*!
    \brief Returns the complete client transaction data associated with the
      transaction.

    \param[in] The transaction to retrieve the complete info for.

    \return The client transaction info for the given transaction.  0 Will be
      returned in the case of an error (bad parameter or the client transaction
      info can't be found).
*/
client_txn_t * get_client_txn(const on_txn_t * const txn)
{
    if(txn)
    {
        if(txn == &(on_single_txn.txn))
        {
            return &on_single_txn;
        } // if the single transaction //
        #ifndef _ONE_NET_SIMPLE_CLIENT
            else if(txn == &(on_block_txn.txn))
            {
                return &on_block_txn;
            } // else if the block transaction //
            else if(txn == &(on_stream_txn.txn))
            {
                return &on_stream_txn;
            } // else if the stream transaction //
        #endif // ifndef _SIMPLE_CLIENT //
    } // if the parameter is valid //

    return 0;
} // get_client_txn //


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
        // replace the least recently used device
        match_idx = max_lru_idx;
        one_net_memmove(sending_dev_list[match_idx].sender.did, *DID,
          sizeof(sending_dev_list[match_idx].sender.did));
        sending_dev_list[match_idx].sender.expected_nonce
          = one_net_prand(one_net_tick(), ON_MAX_NONCE);
        sending_dev_list[match_idx].sender.last_nonce = ON_INVALID_NONCE;
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
    \brief Get the transaction nonce to include in the packet sent to DID.

    \param[in] DID The encoded destination device id.

    \return The transaction nonce to send to DID.
*/
static UInt8 device_txn_nonce(const on_encoded_did_t * const DID)
{
    on_sending_device_t * sender;

    UInt8 nonce;

    if(!DID)
    {
        return 0;
    } // if parameter is invalid //

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        return master->device.send_nonce;
    } // if it is responding to the MASTER //

#ifdef _PEER
    if((nonce = on_client_net_txn_nonce_for_peer(DID)) <= ON_MAX_NONCE)
    {
        return nonce;
    } // if peer_dev //
#endif

    if((sender = sender_info(DID)) != NULL)
    {
        return sender->send_nonce;
    } // if sender //

    return 0;
} // device_txn_nonce //


/*!
    \brief Sets the transaction nonce to use for the next transaction sent to
      DID.

    \param[in] DID The encoded destination device id.
    \param[in] NEXT_NONCE The transaction noce to set for the device.

    \return void
*/
static void set_device_txn_nonce(const on_encoded_did_t * const DID,
  const UInt8 NEXT_NONCE)
{
    if(!DID || NEXT_NONCE > ON_MAX_NONCE)
    {
        return;
    } // if the parameters are invalid //

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        master->device.send_nonce = NEXT_NONCE;
    } // if it is responding to the MASTER //
#ifdef _PEER
    else if(!on_client_net_set_peer_txn_nonce(DID, NEXT_NONCE))
#else
    else
#endif
    {
        on_sending_device_t * sender;

        if((sender = sender_info(DID))!= NULL)
        {
            sender->send_nonce = NEXT_NONCE;
        } // if a device that sends to this one //
    } // if one of the peers //
} // set_device_txn_nonce //


#ifdef _ONE_NET_MULTI_HOP
    /*!
        \brief Returns a pointer to the hops field for the given device.

        The hops field is the max number of hops to allow a packet to take when
        sent to the device.

        \param[in] DID The device whose hops field is to be retrieved.

        \return A pointer to the hops field for the given device.  If the device
          could not be found, 0 is returned.
    */
    static UInt8 * device_hops_field(const on_encoded_did_t * const DID)
    {
        UInt8 * hops_field = 0;

        if(!DID)
        {
            return 0;
        } // if the parameter is invalid //

        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(master->device.did)))
        {
            hops_field = &(master->device.max_hops);
        } // if it is responding to the MASTER //
        else if(!(hops_field = on_client_net_peer_hops_field(DID)))
        {
            on_sending_device_t * sender;

            if((sender = sender_info(DID)) != NULL)
            {
                hops_field = &(sender->max_hops);
            } // if a device that sends to this one //
        } // if one of the peers //

        return hops_field;
    } // device_hops_field //


    /*!
        \brief Updates the number hops it takes for a packet to reach a device.

        The hops field is only updated if HOPS_TAKEN is less than the number of
        hops that is stored for the device.

        \param[in] DID The device whose hops field is to be updated.
        \param[in] HOPS_TAKEN The number of hops it took for the packet to reach
          the device.

        \return void
    */
    static void update_device_hops(const on_encoded_did_t * const DID,
      const UInt8 HOPS_TAKEN)
    {
        UInt8 * hops_field;

        if(!DID || HOPS_TAKEN > ON_MAX_HOPS_LIMIT)
        {
            return;
        } // if any of the parameters are invalid //

        if(((hops_field = device_hops_field(DID)) != NULL) && HOPS_TAKEN < *hops_field)
        {
            *hops_field = HOPS_TAKEN;
        } // if less hops taken //
    } // update_device_hops //
#endif // ifdef _ONE_NET_MULTI_HOP //


/*!
    \brief Get the data rate the destination device receives at.

    Returns the data rate DID receives at. or the data rate this device
    receives at if there was an error or DID could not be found (since this
    device may not normally send to DID, but DID sent something to this
    device that requires a response, in which case do it at this devices
    data rate since DID sent to this device).

    \param[in] DID The encoded destination device id.

    \return The data rate DID receives at.
*/
static UInt8 data_rate_for_dst(const on_encoded_did_t * const DID)
{
    if(DID)
    {
        UInt8 data_rate;

        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(master->device.did)))
        {
            return master->settings.master_data_rate;
        } // if it is responding to the MASTER //

#ifdef _PEER
        if((data_rate = on_client_net_data_rate_for_peer(DID))
          < ONE_NET_DATA_RATE_LIMIT)
        {
            return data_rate;
        } // if the data rate is valid //
#endif
    } // if the parameter is valid //

    // A device that this device does not send to could have sent something to
    // this device, at the data rate this device receives at, so use this
    // data rate to send to an unknown device.
    return on_base_param->data_rate;
} // data_rate_for_dst //


#ifdef _ONE_NET_MULTI_HOP
    /*!
        \brief Returns the maximum number of hops a packet can take when sending
          to the destination.

        \param[in] DID The device to request the maximum number of hops for.

        \return The maximum number of hops a packet can take when sending to
          DID.  ON_INVALID_HOPS will be returned if DID is not a valid
          destination.
    */
    static UInt8 max_hops_for_dst(const on_encoded_did_t * const DID)
    {
        UInt8 max_hops;
        on_sending_device_t * sender;

        if(!DID)
        {
            return ON_INVALID_HOPS;
        } // if the parameter is invalid //

        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(master->device.did)))
        {
            return master->device.max_hops;
        } // if it is responding to the MASTER //

        if((max_hops = on_client_net_max_hops_for_peer(DID))
          <= ON_MAX_HOPS_LIMIT)
        {
            return max_hops;
        } // if peer_dev //

        if((sender = sender_info(DID)) != NULL)
        {
            return sender->max_hops;
        } // if sender //

        return ON_INVALID_HOPS;
    } // max_hops_for_dst //
#endif // ifdef _ONE_NET_MULTI_HOP //


/*!
    \brief Builds the data packet for the given transaction.

    The raw data is stored in the pkt field of the transaction.  It is copied
    out of that field, the client info is found for the destination, and the
    packet is built.  This should be called when the transaction is actually
    being sent.

    \param[in] TYPE The type of transaction to be carried out.
    \param[in] txn  The transaction to build the data packet for.

    \return ONS_SUCCESS If the transaction was successfully built.
            ONS_BAD_PARAM If the parameter was invalid
            ONS_INTERNAL_ERR If there was a problem with the data size or
              msg_type
            See on_build_data_pkt for more possible return values.
*/
static one_net_status_t build_txn_data_pkt(const UInt8 TYPE,
  on_txn_t * const txn)
{
    const one_net_xtea_key_t * KEY = 0;

    one_net_status_t status;
    on_encoded_did_t dst;
    UInt8 pid, msg_type, admin_type, txn_nonce, data_len;
    UInt8 data[ON_MAX_RAW_PLD_LEN];

    #ifdef _ONE_NET_MULTI_HOP
        UInt8 max_hops;
    #endif // ifdef _ONE_NET_MULTI_HOP //

    if(!txn)
    {
        return ONS_BAD_PARAM;
    } // if the parameter was invalid //

    one_net_memmove(dst, &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
      ON_ENCODED_DID_LEN);

    #ifdef _ONE_NET_MULTI_HOP
        if((max_hops = max_hops_for_dst((const on_encoded_did_t * const)&dst))
          == ON_INVALID_HOPS)
        {
            return ONS_INTERNAL_ERR;
        } // if the hops field was invalid //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    switch(TYPE)
    {
        case ON_SINGLE:
        {
            #ifdef _ONE_NET_MULTI_HOP
                pid = max_hops ? ONE_NET_ENCODED_MH_SINGLE_DATA
                  : ONE_NET_ENCODED_SINGLE_DATA;
            #else // ifdef _ONE_NET_MULTI_HOP //
                pid = ONE_NET_ENCODED_SINGLE_DATA;
            #endif // else _ONE_NET_MULTI_HOP is not defined //

            KEY = &(on_base_param->current_key);
            break;
        } // ON_SINGLE case //

        #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_BLOCK:
            {
                if(!txn->retry)
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        pid = max_hops ? ONE_NET_ENCODED_MH_BLOCK_DATA
                          : ONE_NET_ENCODED_BLOCK_DATA;
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        pid = ONE_NET_ENCODED_BLOCK_DATA;
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                } // if this block has not been sent //
                else
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        pid = max_hops ? ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
                          : ONE_NET_ENCODED_REPEAT_BLOCK_DATA;
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        pid = ONE_NET_ENCODED_REPEAT_BLOCK_DATA;
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                } // else this block is being resent //

                KEY = &(on_base_param->current_key);
                break;
            } // ON_BLOCK case //

            case ON_STREAM:
            {
                #ifdef _ONE_NET_MULTI_HOP
                    pid = max_hops ? ONE_NET_ENCODED_MH_STREAM_DATA
                      : ONE_NET_ENCODED_STREAM_DATA;
                #else // ifdef _ONE_NET_MULTI_HOP //
                    pid = ONE_NET_ENCODED_STREAM_DATA;
                #endif // else _ONE_NET_MULTI_HOP is not defined //

                KEY = &(on_base_param->stream_key);
                break;
            } // ON_STREAM case //
        #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

        default:
        {
            return ONS_BAD_PARAM;
            break;
        } // default case //
    } // switch(TYPE) //

    if(txn->data_len > sizeof(data))
    {
        return ONS_INTERNAL_ERR;
    } // if an internal error occured //

    // get the information & raw fields that were stored in the packet
    msg_type = txn->msg_type;
    admin_type = txn->pkt[ON_ADMIN_TYPE_IDX];
    data_len = txn->data_len;
    one_net_memmove(data, &(txn->pkt[ON_DATA_IDX]), data_len);

    txn->data_len = txn->pkt_size;

    txn_nonce = device_txn_nonce((const on_encoded_did_t * const)&dst);
    txn->expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);

    switch(msg_type)
    {
        case ON_APP_MSG:
        {
            #ifdef _ONE_NET_MULTI_HOP
                status = on_build_data_pkt(txn->pkt, &(txn->data_len),
                  msg_type, pid, (const on_encoded_did_t * const)&dst,
                  txn_nonce, txn->expected_nonce, data, data_len, KEY,
                  max_hops);
            #else // ifdef _ONE_NET_MULTI_HOP //
                status = on_build_data_pkt(txn->pkt, &(txn->data_len),
                  msg_type, pid, (const on_encoded_did_t * const)&dst,
                  txn_nonce, txn->expected_nonce, data, data_len, KEY);
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            break;
        } // ON_APP_MSG case //

        case ON_ADMIN_MSG:          // fall through
        case ON_EXTENDED_ADMIN_MSG:
        {
            #ifdef _ONE_NET_MULTI_HOP
                status = on_build_admin_pkt(txn->pkt, &(txn->data_len),
                  txn->msg_type, admin_type,
                  (const on_encoded_did_t * const)&dst, txn_nonce,
                  txn->expected_nonce, data, data_len, KEY, max_hops);
            #else // ifdef _ONE_NET_MULTI_HOP //
                status = on_build_admin_pkt(txn->pkt, &(txn->data_len),
                  txn->msg_type, admin_type,
                  (const on_encoded_did_t * const)&dst, txn_nonce,
                  txn->expected_nonce, data, data_len, KEY);
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            break;
        } // ON_ADMIN_MSG & ON_EXTENDED_ADMIN_MSG case //

        default:
        {
            status = ONS_INTERNAL_ERR;
            break;
        } // default case //
    } // switch(msg_type) //

    #ifdef _ONE_NET_MULTI_HOP
        if(status == ONS_SUCCESS)
        {
            txn->max_hops = max_hops;
            if(on_encoded_did_equal((const on_encoded_did_t * const)dst,
              (const on_encoded_did_t * const)&master->device.did))
            {
                ont_set_timer(ONT_KEEP_ALIVE_TIMER,
                  master->keep_alive_interval);
            } // if sending to the MASTER //
        } // if successfully built the packet //
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(status == ONS_SUCCESS
          && on_encoded_did_equal((const on_encoded_did_t * const)dst,
          (const on_encoded_did_t * const)&master->device.did))
        {
            ont_set_timer(ONT_KEEP_ALIVE_TIMER, master->keep_alive_interval);
        } // if sending to the MASTER //
    #endif // else _ONE_NET_MULTI_HOP is not defined //

    return status;
} // build_txn_data_pkt //


/*!
    \brief Sends a keep alive message to the MASTER

    \param void

    \return void
*/
static void send_keep_alive(void)
{
    if(send_settings_resp((const on_encoded_did_t * const)&(master->device.did))
      == ONS_SUCCESS)
    {
        ont_set_timer(ONT_KEEP_ALIVE_TIMER, master->keep_alive_interval);
    } // if queueing the packet was successful //
} // send_keep_alive //


/*!
    \brief Sends a status response message.

    \param void

    \return ONS_SUCCESS If the packet was successfully built
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_RSRC_FULL If the resource is not available.
*/
static one_net_status_t send_status_resp(const on_encoded_did_t * const DST)
{
    if(!DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
    {
        return ONS_RSRC_FULL;
    } // if the resource is not available //

    on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
    on_single_txn.txn.retry = 0;

    on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

    on_single_txn.txn.msg_type = ON_ADMIN_MSG;
    on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_STATUS_RESP;

    one_net_memmove(&(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]), *DST,
      sizeof(*DST));

    on_single_txn.txn.pkt[ON_DATA_IDX + ON_STATUS_VER_IDX] = ON_VERSION;
    on_single_txn.txn.pkt[ON_DATA_IDX + ON_STATUS_MAX_DATA_RATE_IDX]
      = ONE_NET_MAX_DATA_RATE;
    on_single_txn.txn.pkt[ON_DATA_IDX + ON_STATUS_FEATURES_IDX] = ON_FEATURES;

    return ONS_SUCCESS;
} // send_status_resp //


/*!
    \brief Sends a settings response message.

    \param void

    \return ONS_SUCCESS If the packet was successfully built
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_RSRC_FULL If the resource is not available.
*/
static one_net_status_t send_settings_resp(const on_encoded_did_t * const DST)
{
    if(!DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(on_single_txn.txn.priority != ONE_NET_NO_PRIORITY)
    {
        return ONS_RSRC_FULL;
    } // if the resource is full //

    on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
    on_single_txn.txn.retry = 0;

    on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

    on_single_txn.txn.msg_type = ON_ADMIN_MSG;
    on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_SETTINGS_RESP;

    one_net_memmove(&(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]), *DST,
      sizeof(*DST));

    on_single_txn.txn.pkt[ON_DATA_IDX + ON_SETTINGS_DATA_RATE_IDX]
      = on_base_param->data_rate;
    on_single_txn.txn.pkt[ON_DATA_IDX + ON_SETTINGS_MASTER_DATA_RATE_IDX]
      = master->settings.master_data_rate;
    on_single_txn.txn.pkt[ON_DATA_IDX + ON_SETTINGS_FLAG_IDX]
      = master->settings.flags;

    return ONS_SUCCESS;
} // send_settings_resp //


/*!
    \brief Parses a single data packet.

    If the device is unable to parse the packet (or does not want to send a
    response), it clears *txn.  If the device is sending an ACK or NACK,
    it sets up a txn, and returns the pointer through txn.

    \param[in] PID The PID of the received packet.  Either
      ONE_NET_ENCODED_SINGLE_DATA or ONE_NET_ENCODED_REPEAT_SINGLE_DATA.
    \param[in] SRC_DID The encoded did of the sender of the packet.
    \param[in/out] pld The payload of the single data packet
    \param[out] txn If not 0, the transaction that contains the response to the
      single data packet.
    \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
      CLIENT.

    \return ONS_SUCCESS if the payload was successfully parsed.
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INTERNAL_ERR if something unexpected occured.
            ONS_BAD_PKT If data in the packet is invalid.
            See on_parse_pld for more possible return values.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_single_data_hdlr(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn, const UInt8 HOPS_TAKEN)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_single_data_hdlr(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn)
#endif // else _ONE_NET_MULTI_HOP is not defined //
{
    one_net_raw_did_t raw_src_did;
    one_net_status_t status;
    on_sending_device_t * sender = 0;

    UInt8 txn_nonce, resp_nonce, msg_type, response_pid, response_max_hops;
    const one_net_xtea_key_t* const key = &(on_base_param->current_key);

    #ifdef _ONE_NET_VERSION_2_X
/*        ona_msg_class_t msg_class;
		ona_msg_type_t type;
		BOOL useDefaultHandling = TRUE;
		UInt8 src_unit;
		UInt8 dst_unit;
		UInt16 msg_data;*/
        on_nack_rsn_t nack_reason;
	#endif
	
    #ifdef _ONE_NET_MULTI_HOP
        if((PID != ONE_NET_ENCODED_SINGLE_DATA
          && PID != ONE_NET_ENCODED_REPEAT_SINGLE_DATA
          && PID != ONE_NET_ENCODED_MH_SINGLE_DATA
          && PID != ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA)
          || ((PID == ONE_NET_ENCODED_SINGLE_DATA
          || PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA) && HOPS_TAKEN)
          || !SRC_DID || !pld || !txn || HOPS_TAKEN > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if((PID != ONE_NET_ENCODED_SINGLE_DATA
          && PID != ONE_NET_ENCODED_REPEAT_SINGLE_DATA) || !SRC_DID || !pld
          || !txn)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        if(txn)
        {
            *txn = 0;                   // not responding
        } // if the txn is non-NULL //

        #ifdef _ONE_NET_DEBUG
        #if 1
            one_net_debug(ONE_NET_DEBUG_ONS_BAD_PARAM, "1", 1);
        #endif
        #endif
        return ONS_BAD_PARAM;
    } // if any parameter is invalid //

    // decode the address 1. To make sure it's a valid encoding. 2. Since the
    // address may need to be passed up to the app
    if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
      == ONS_SUCCESS)
    {
        if((sender = sender_info(SRC_DID)) == 0)
        {
            #ifdef _ONE_NET_DEBUG
            #if 0
                one_net_debug(ONE_NET_DEBUG_ONS_INTERNAL_ERR, "1", 1);
            #endif
            #endif
            status = ONS_INTERNAL_ERR;
        } // if not able to get sender info //
    } // if decoding the address was successful //

    if(status == ONS_SUCCESS)
    {
        status = on_parse_pld(&txn_nonce, &resp_nonce, &msg_type, pld,
          ON_SINGLE, key);
    } // if successful so far //

    #ifdef _ONE_NET_MULTI_HOP	
        // if it's a repeat multi-hop packet, allow MAX_HOPS since it
        // may take more hops to get back than it took for the packet
        // to arrive.
        response_max_hops = (PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA
          ? ON_MAX_HOPS_LIMIT : HOPS_TAKEN);
    #endif

    if(status == ONS_SUCCESS && txn_nonce == sender->expected_nonce)
    {
        switch(msg_type)
        {
            case ON_APP_MSG:
            {
                #ifndef _ONE_NET_SIMPLE_CLIENT
                    if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP)
                    {
                        #ifdef _ONE_NET_DEBUG
                        #if 0
                            one_net_debug(ONE_NET_DEBUG_ONS_BAD_PKT_TYPE, &msg_type, 1);
                        #endif
                        #endif
                        return ONS_BAD_PKT_TYPE;
                    } // if waiting for a stream data response //
                #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

/*#ifndef _ONE_NET_VERSION_2_X*/
                if(!one_net_client_handle_single_pkt(&(pld[ON_PLD_DATA_IDX]),
                  ONE_NET_RAW_SINGLE_DATA_LEN,
                  (const one_net_raw_did_t * const)&raw_src_did))
                {
                    status = ONS_UNHANDLED_PKT;
                } // if client is not handling the packet //
/*#else			
                nack_reason = on_parse_single_app_pld(pld, &src_unit,
                   &dst_unit, &msg_class, &type, &msg_data);
				   
                if(nack_reason != ON_NACK_RSN_NO_ERROR ||
				   !one_net_client_handle_single_pkt(msg_class, type, 
                    src_unit, dst_unit, &msg_data,
                   (const one_net_raw_did_t * const)&raw_src_did,
				   &useDefaultHandling, &nack_reason))
                {
					// invalid packet.  We'll want to send out a NACK with
					// nack_reason as the reason
                    status = ONS_UNHANDLED_PKT;
                } // if master is not handling the packet //
#endif	*/			   

                break;
            } // application message case //

            case ON_ADMIN_MSG:
            {
                status = handle_admin_pkt(SRC_DID, &(pld[ON_PLD_DATA_IDX]),
                  ONE_NET_RAW_SINGLE_DATA_LEN);
                break;
            } // admin message case //

            #ifndef _ONE_NET_SIMPLE_CLIENT
                case ON_EXTENDED_ADMIN_MSG:
                {
                    status = handle_extended_single_admin_pkt(SRC_DID,
                      &(pld[ON_PLD_DATA_IDX]), ONE_NET_RAW_SINGLE_DATA_LEN);
                    #ifdef _ONE_NET_DEBUG
                    #if 0
                        one_net_debug(ONE_NET_DEBUG_HANDLE_EXTENDED_ADMIN, (UInt8 *) &status, 2);
                    #endif
                    #endif
                    break;
                } // extended admin message case //
            #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

            default:
            {
                status = ONS_BAD_PKT;
                #ifdef _ONE_NET_DEBUG
                #if 0
                    one_net_debug(ONE_NET_DEBUG_ONS_BAD_PKT, &msg_type, 1);
                #endif
                #endif
                break;
            } // default case //
        } // switch(msg_type) //

        #ifndef _ONE_NET_SIMPLE_CLIENT
            if(status == ONS_SUCCESS || status == ONS_STREAM_END)
        #else // ifndef _ONE_NET_SIMPLE_CLIENT //
            if(status == ONS_SUCCESS)
        #endif // else _ONE_NET_SIMPLE_CLIENT is defined //
        {
            #ifndef _ONE_NET_SIMPLE_CLIENT
                one_net_status_t saved_status = status;
            #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

            sender->last_nonce = sender->expected_nonce;
            sender->expected_nonce = one_net_prand(one_net_tick(),
              ON_MAX_NONCE);
            response_txn.data_len = response_txn.pkt_size;

            #ifdef _ONE_NET_MULTI_HOP			
                response_pid = (HOPS_TAKEN ? ONE_NET_ENCODED_MH_SINGLE_DATA_ACK :
                  ONE_NET_ENCODED_SINGLE_DATA_ACK);
				  
                status = on_build_response_pkt(response_txn.pkt,
                  &(response_txn.data_len), response_pid, SRC_DID, resp_nonce,
                  sender->expected_nonce, response_max_hops);
            #else // ifdef _ONE_NET_MULTI_HOP //
                status = on_build_response_pkt(response_txn.pkt,
                  &(response_txn.data_len), ONE_NET_ENCODED_SINGLE_DATA_ACK,
                  SRC_DID, resp_nonce, sender->expected_nonce);
            #endif // else _ONE_NET_MULTI_HOP is not defined //
                #ifdef _ONE_NET_DEBUG
                #if 0
                    one_net_debug(ONE_NET_DEBUG_BUILD_RESP_PKT, (UInt8 *) &status, 2);
                #endif
                #endif

            #ifndef _ONE_NET_SIMPLE_CLIENT
                if(saved_status == ONS_STREAM_END && status == ONS_SUCCESS)
                {
                    saved_status = status;
                } // if stream ended and building response was successful //
            #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
        } // if successful so far //
    } // if it's a new packet //
    else if(status == ONS_SUCCESS)
    {
        //
        // since the nonce in the packet is not equal to the exptected nonce,
        // this is not a new packet
        //
        response_txn.data_len = response_txn.pkt_size;
		
		
		#ifndef _ONE_NET_VERSION_2_X
        	#ifdef _ONE_NET_MULTI_HOP
	            if((PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA
	              || PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA)
	              && txn_nonce == sender->last_nonce)
	            {
					response_pid = (PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA
	                  ? ONE_NET_ENCODED_MH_SINGLE_DATA_ACK
	                  : ONE_NET_ENCODED_SINGLE_DATA_ACK);
					
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), response_pid, SRC_DID, resp_nonce,
	                  sender->expected_nonce, response_max_hops);	                  
	            } // if it is a repeat packet //
	            else
	            {
					response_pid = (HOPS_TAKEN ? ONE_NET_ENCODED_MH_SINGLE_DATA_NACK
	                  : ONE_NET_ENCODED_SINGLE_DATA_NACK);
					  
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), response_pid, SRC_DID, resp_nonce,
	                  sender->expected_nonce, response_max_hops);
	            } // else the nonce is wrong //
	        #else // ifdef _ONE_NET_MULTI_HOP //
	            if(PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA
	              && txn_nonce == sender->last_nonce)
	            {
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), ONE_NET_ENCODED_SINGLE_DATA_ACK,
	                  SRC_DID, resp_nonce, sender->expected_nonce);
	            } // if it is a repeat packet //
	            else
	            {
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), ONE_NET_ENCODED_SINGLE_DATA_NACK,
	                  SRC_DID, resp_nonce, sender->expected_nonce);
	            } // else the nonce is wrong //
	        #endif // else _ONE_NET_MULTI_HOP is not defined //
		#else // If ONE-NET Version 1.x		
        	#ifdef _ONE_NET_MULTI_HOP
	            if((PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA
	              || PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA)
	              && txn_nonce == sender->last_nonce)
	            {
					response_pid = (PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA
	                  ? ONE_NET_ENCODED_MH_SINGLE_DATA_ACK
	                  : ONE_NET_ENCODED_SINGLE_DATA_ACK);
					
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), response_pid, SRC_DID, resp_nonce,
	                  sender->expected_nonce, response_max_hops);
	            } // if it is a repeat packet //
	            else
	            {
					response_pid = (HOPS_TAKEN ? ONE_NET_ENCODED_MH_SINGLE_DATA_NACK
	                  : ONE_NET_ENCODED_SINGLE_DATA_NACK);
					
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), response_pid, SRC_DID, resp_nonce,
	                  sender->expected_nonce, response_max_hops);
	            } // else the nonce is wrong //
	        #else // ifdef _ONE_NET_MULTI_HOP //
	            if(PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA
	              && txn_nonce == sender->last_nonce)
	            {
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), ONE_NET_ENCODED_SINGLE_DATA_ACK,
	                  SRC_DID, resp_nonce, sender->expected_nonce);
	            } // if it is a repeat packet //
	            else
	            {
	                status = on_build_response_pkt(response_txn.pkt,
	                  &(response_txn.data_len), ONE_NET_ENCODED_SINGLE_DATA_NACK,
	                  SRC_DID, resp_nonce, sender->expected_nonce);
	            } // else the nonce is wrong //
	        #endif // else _ONE_NET_MULTI_HOP is not defined //		
		#endif // If ONE-NET Version 2.x
			
		
            #ifdef _ONE_NET_DEBUG
            #if 0
                one_net_debug(ONE_NET_DEBUG_BUILD_RESP_PKT, (UInt8 *) &status, 2);
            #endif
            #endif
    } // else if the packet was successfully parsed //

    if(status == ONS_SUCCESS)
    {
        *txn = &response_txn;
    } // if building the response was successful //
    else if(on_state != ON_WAIT_FOR_STREAM_DATA_RESP)
    {
        *txn = 0;
    } // else if not waiting for a response to a stream data pkt //

    return status;
} // on_client_single_data_hdlr //


/*!
    \brief Handles the end of a single data transaction.

    Updates nonces and hops.  Resends (or tries more hops) if necessary.

    \param[in/out] txn The transaction that ended.
    \param[in] NEXT_NONCE ON_MAX_NONCE + 1 if this field is to be ignored,
                otherwise the next nonce for the receiving device.
    \param[in] STATUS The result of the transaction.
    \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
      CLIENT.

    \return ONS_SUCCESS If handling the transaction was successful
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            TXN_QUEUED If there is another transaction queued for this client.
            ONS_INTERNAL_ERR If the transaction was not handled.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS,
      const UInt8 HOPS_TAKEN)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_single_txn_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS)
#endif // else _ONE_NET_MULTI_HOP is not defined //
{
    one_net_status_t rv;

    UInt8 pld[ON_RAW_SINGLE_PLD_SIZE];

    #ifdef _ONE_NET_MULTI_HOP
        if(!txn || !(*txn) || !(*txn)->pkt || HOPS_TAKEN > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(!txn || !(*txn) || !(*txn)->pkt)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // Check if the device is part of the network
    if(!(master->settings.flags & ON_JOINED))
    {
        // still need to join the network.
        // make sure the message is for the MASTER.  No other messages matter
        // since the device still needs to join.
        if(!on_encoded_did_equal((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
          (const on_encoded_did_t * const)&(master->device.did)))
        {
            return ONS_INCORRECT_ADDR;
        } // if the packet is not for the MASTER //

        // the CLIENT is not part of the network yet.
        if(STATUS == ONS_SUCCESS || STATUS == ONS_RX_STAY_AWAKE)
        {
            one_net_raw_did_t local_did, master_did;

            if(on_decode(local_did, &(on_base_param->sid[ON_ENCODED_NID_LEN]),
              sizeof(on_encoded_did_t)) != ONS_SUCCESS || on_decode(master_did,
              master->device.did, sizeof(on_encoded_did_t)) != ONS_SUCCESS)
            {
                // This really should never happen.
                // keep trying the transaction
                (*txn)->retry = 0;
                return ONS_INTERNAL_ERR;
            } // if decoding the did's failed //

            (*txn)->priority = ONE_NET_NO_PRIORITY;
            *txn = &response_txn;
            master->settings.flags |= ON_JOINED;
            master->device.send_nonce = NEXT_NONCE;

            #ifndef _ONE_NET_SIMPLE_CLIENT
                one_net_client_stream_key_query();
            #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

            one_net_client_joined_network(
              (const one_net_raw_did_t * const)&local_did,
              (const one_net_raw_did_t * const)&master_did);
			  
			client_joined_network = TRUE;
			client_looking_for_invite = FALSE;

            if(STATUS == ONS_RX_STAY_AWAKE)
            {
                ont_set_timer(ONT_STAY_AWAKE_TIMER, ONE_NET_STAY_AWAKE_TIME);
            } // if a stay awake was received //

            save = TRUE;
            return ONS_SUCCESS;
        } // if the transaction was successful //
        else
        {
            // keep trying the transaction
            (*txn)->retry = 0;

            // If a nack was received, we want to continue processing below
            // since the MASTER may now think that the CLIENT is part of the
            // network, but the CLIENT must have missed the single data ack, so
            // it is not sure that it is part of the network.  Because of this,
            // if a nack is received, fall through, otherwise we can exit here
            // since the packet is already set up.  If doing Multi-Hop, since we
            // are sending to the MASTER, we know the MASTER does Multi-Hop, so
            // it is possible the MASTER may not hear the packet, so try a
            // Multi-Hop version (fall through).  If already doing the
            // Multi-Hop packet, we can return here since the packet is already
            // set up.
            #ifdef _ONE_NET_MULTI_HOP
                if(STATUS != ONS_RX_NACK && (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                  == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if(STATUS != ONS_RX_NACK)
            #endif // else _ONE_NET_MULTI_HOP has not been defined //
            {
                return ONS_SUCCESS;
            } // if a nack wasn't received && already sending multi hop //
        } // else the device has not successfully joined the network //
    } // if the device has not joined the network yet //

    // set the transaction nonce for the device
    set_device_txn_nonce((const on_encoded_did_t * const)
      &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), NEXT_NONCE);

    // update the hops field
    #ifdef _ONE_NET_MULTI_HOP
        if(HOPS_TAKEN <= ON_MAX_HOPS_LIMIT && (STATUS == ONS_SUCCESS
          || STATUS == ONS_RX_STAY_AWAKE || STATUS == ONS_RX_NACK))
        {
            update_device_hops((const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), HOPS_TAKEN);
        } // if the hops field should be updated //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    on_decode(pld, &((*txn)->pkt[ON_PLD_IDX]), ON_ENCODED_SINGLE_PLD_SIZE);
    // Reuse txn->expected_nonce to save ram since neither of those nonces will
    // be used later on
    if((rv = on_parse_pld(&((*txn)->expected_nonce), &((*txn)->expected_nonce),
      &((*txn)->msg_type), pld, ON_SINGLE,
      (const one_net_xtea_key_t * const)&(on_base_param->current_key)))
      != ONS_SUCCESS)
    {
        return rv;
    } // if parsing the payload is not successful //

    if(STATUS == ONS_SUCCESS || STATUS == ONS_RX_STAY_AWAKE)
    {
        rv = single_success_hdlr(txn, &(pld[ON_PLD_DATA_IDX]));

        if(STATUS == ONS_RX_STAY_AWAKE)
        {
            ont_set_timer(ONT_STAY_AWAKE_TIMER, ONE_NET_STAY_AWAKE_TIME);
        } // if a stay awake was received //
    } // if the transaction was successful //
    else if(STATUS == ONS_RX_NACK)
    {
        #ifdef _ONE_NET_MULTI_HOP
            rv = single_nack_hdlr(txn, NEXT_NONCE, &(pld[ON_PLD_DATA_IDX]),
              HOPS_TAKEN);
        #else // ifdef _ONE_NET_MULTI_HOP //
            rv = single_nack_hdlr(txn, NEXT_NONCE, &(pld[ON_PLD_DATA_IDX]));
        #endif // else _ONE_NET_MULTI_HOP has not been defined //
    } // else if a nack was received //
    else if(STATUS == ONS_SINGLE_FAIL)
    {
        rv = single_fail_hdlr(txn, &(pld[ON_PLD_DATA_IDX]));
    } // else if the transaction failed //
    else
    {
        // this should not happen
        rv = ONS_INTERNAL_ERR;
    } // else an unhandled status //

    return rv;
} // on_client_single_txn_hdlr //


/*!
    \brief Handles receiving a NACK

    \param[in/out] txn The single transaction that the nack was received for.
    \param[in] NEXT_NONCE The nonce to use when sending to the device.
    \param[in] raw_pld_data The app/admin data that was sent (actual data,
      does not include the nonces, and other header information that gets
      encrypted with the data).
    \param[in] HOPS The max number of hops to allow when sending.

    \return ONS_SUCCESS If handling the completed transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
#ifdef _ONE_NET_MULTI_HOP
    static one_net_status_t single_nack_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const UInt8 * const RAW_PLD_DATA,
      const UInt8 HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    static one_net_status_t single_nack_hdlr(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const UInt8 * const RAW_PLD_DATA)
#endif // else _ONE_NET_MULTI_HOP is not defined //
{
    on_encoded_did_t dst;

    #ifdef _ONE_NET_MULTI_HOP
        if(!txn || !(*txn) || !(*txn)->pkt || !RAW_PLD_DATA
          || HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(!txn || !(*txn) || !(*txn)->pkt || !RAW_PLD_DATA)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    one_net_memmove(dst, &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
      sizeof(dst));

    // rebuild the packet
    (*txn)->expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);
    (*txn)->data_len = (*txn)->pkt_size;

    #ifdef _ONE_NET_MULTI_HOP
        return on_build_data_pkt((*txn)->pkt, &((*txn)->data_len),
          (*txn)->msg_type, HOPS ? ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA
          : ONE_NET_ENCODED_REPEAT_SINGLE_DATA,
          (const on_encoded_did_t * const)&dst, NEXT_NONCE,
          (*txn)->expected_nonce, RAW_PLD_DATA, ONE_NET_RAW_SINGLE_DATA_LEN,
          (const one_net_xtea_key_t * const)&(on_base_param->current_key),
          HOPS);
    #else // ifdef _ONE_NET_MULTI_HOP //
        return on_build_data_pkt((*txn)->pkt, &((*txn)->data_len),
          (*txn)->msg_type, ONE_NET_ENCODED_REPEAT_SINGLE_DATA,
          (const on_encoded_did_t * const)&dst, NEXT_NONCE,
          (*txn)->expected_nonce, RAW_PLD_DATA, ONE_NET_RAW_SINGLE_DATA_LEN,
          (const one_net_xtea_key_t * const)&(on_base_param->current_key));
    #endif // else _ONE_NET_MULTI_HOP is not defined //
} // single_nack_hdlr //


/*!
    \brief Handles a successful single transaction.

    \param[in/out] txn The single transaction that was completed
    \param[in] RAW_PLD_DATA The app/admin data that was sent (actual data,
      does not include the nonces, and other header information that gets
      encrypted with the data).

    \return ONS_SUCCESS If handling the completed transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
static one_net_status_t single_success_hdlr(on_txn_t ** txn,
  const UInt8 * const RAW_PLD_DATA)
{
    client_txn_t * client_txn = 0;

    if(!txn || !(*txn) || !(*txn)->pkt || !(client_txn = get_client_txn(*txn))
      || !RAW_PLD_DATA)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if((*txn)->msg_type == ON_APP_MSG)
    {
        UInt8 priority = (*txn)->priority;

        // Make sure the priority is cleared, so the app can send right away if
        // it needs to.
        (*txn)->priority = ONE_NET_NO_PRIORITY;
        on_client_net_single_txn_hdlr(ONS_SUCCESS,
          (const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), RAW_PLD_DATA,
          (*txn)->retry, priority, client_txn->txn_id);
        // The transaction may be new, so make sure it doesn't get changed.
        *txn = &response_txn;
    } // if an app message //
    else if((*txn)->msg_type == ON_ADMIN_MSG)
    {
        if(RAW_PLD_DATA[ON_ADMIN_MSG_ID_IDX] == ON_SETTINGS_RESP)
        {
            confirm_key_update = FALSE;
        } // else if it was a keep alive admin message //
        #ifndef _ONE_NET_SIMPLE_CLIENT
            else if(client_txn->requesting)
            {
                // Admin messages are used to set up application
                // transactions
                (*txn)->msg_type = ON_APP_MSG;
            } // if a block or stream transaction is being requested //
        #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
    } // else if an admin message //

    return ONS_SUCCESS;
} // single_success_hdlr //


/*!
    \brief Handles a failed transaction

    \param[in/out] txn The single transaction that failed
    \param[in] RAW_PLD_DATA The app/admin data that was sent (actual data,
      does not include the nonces, and other header information that gets
      encrypted with the data).

    \return ONS_SUCCESS If handling the completed transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
static one_net_status_t single_fail_hdlr(on_txn_t ** txn,
  const UInt8 * const RAW_PLD_DATA)
{
    client_txn_t * client_txn = 0;
    UInt8 * hops;
    on_encoded_did_t * dstpt;

    one_net_status_t rv = ONS_INTERNAL_ERR;

    if(!txn || !(*txn) || !(*txn)->pkt || !(client_txn = get_client_txn(*txn)))
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    #ifdef _ONE_NET_MULTI_HOP
        // dje: Don't go through this stuff if hops_field == NULL
        // index of dst did /get encoded de
        dstpt = (on_encoded_did_t *)((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]);
        if ((hops = device_hops_field(dstpt)) != NULL)
        {
            BOOL retry = FALSE;
            if(!*hops) // zero hops added so far
            {
                //dje: Don't do the multi-hop retry unless max_hops
                //for this did is non-zero
                UInt8 maxh = max_hops_for_dst(dstpt);
                if (maxh != 0)  // don't do it again for non-multihop
                {
                    retry = TRUE;
                    *hops = ON_FIRST_MH_MAX_HOPS_COUNT;
                }
            } // if hops is 0 //
            else if(*hops < ON_MAX_HOPS_LIMIT)
            {
                *hops <<= 1;
                if(*hops > ON_MAX_HOPS_LIMIT)
                {
                    *hops = ON_MAX_HOPS_LIMIT;
                } // if the max hops limit was exceeded //

                retry = TRUE;
            } // else if increase the max hops size //

            if(retry)
            {
                on_encoded_did_t dst;

                one_net_memmove(dst,
                  &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  ON_ENCODED_DID_LEN);

                if((*txn)->msg_type == ON_APP_MSG)
                {
                    (*txn)->data_len = ONE_NET_RAW_SINGLE_DATA_LEN;
                    one_net_memmove(&((*txn)->pkt[ON_DATA_IDX]),
                      RAW_PLD_DATA, (*txn)->data_len);
                } // if an app message //
                else
                {
                    (*txn)->data_len = ON_MAX_ADMIN_PLD_LEN;
                    (*txn)->pkt[ON_ADMIN_TYPE_IDX] = *RAW_PLD_DATA;
                    one_net_memmove(&((*txn)->pkt[ON_DATA_IDX]),
                      &(RAW_PLD_DATA[ON_ADMIN_DATA_IDX]),
                      (*txn)->data_len);
                } // else an admin or extended admin message //

                one_net_memmove(&((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  dst, ON_ENCODED_DID_LEN);

                (*txn)->retry = 0;
                if(master->settings.flags & ON_JOINED)
                {
                    // This is done so the single transaction does not
                    // get cleared later on
                    *txn = &response_txn;
                } // if the packet was successfully built //

                return rv;
            } // if the txn should be tried with more hops //
        } // if more hops should be tried //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    (*txn)->priority = ONE_NET_NO_PRIORITY;

    if((*txn)->msg_type == ON_APP_MSG)
    {
        UInt8 priority = (*txn)->priority;

        (*txn)->priority = ONE_NET_NO_PRIORITY;
        on_client_net_single_txn_hdlr(ONS_SINGLE_FAIL,
          (const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), RAW_PLD_DATA,
          (*txn)->retry, priority, client_txn->txn_id);
        *txn = &response_txn;
    } // if an application message //
    #ifndef _ONE_NET_SIMPLE_CLIENT
        else if((*txn) == &(on_block_txn.txn))
        {
            one_net_raw_did_t raw_did;

            client_txn->requesting = FALSE;
            if((rv = on_decode(raw_did,
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), ON_ENCODED_DID_LEN))
              != ONS_SUCCESS)
            {
                return rv;
            } // if decoding the CLIENT did was not successful //
            one_net_client_block_txn_status(ONS_BLOCK_FAIL,
              (const one_net_raw_did_t * const)&raw_did);
        } // if block transaction //
        else if((*txn) == &(on_stream_txn.txn))
        {
            one_net_raw_did_t raw_did;

            client_txn->requesting = FALSE;
            if((rv = on_decode(raw_did, &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
              ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
            {
                return rv;
            } // if decoding the CLIENT did was not successful //
            one_net_client_stream_txn_status(ONS_STREAM_FAIL,
              (const one_net_raw_did_t * const)&raw_did);
        } // else if stream transaction //
    #endif // _ONE_NET_SIMPLE_CLIENT //

    return ONS_SUCCESS;
} // single_fail_hdlr //


#ifndef _ONE_NET_SIMPLE_CLIENT
    /*!
        \brief Parses a block data packet.

        If the device is unable to parse the packet (or does not want to send a
        response), it clears *txn.  If the device is sending an ACK or NACK,
        it sets up a txn, and returns the pointer through txn.

        \param[in] PID The PID of the received packet.  Either
          ONE_NET_ENCODED_BLOCK_DATA or ONE_NET_ENCODED_REPEAT_BLOCK_DATA.
        \param[in] SRC_DID The encoded did of the sender of the packet.
        \param[in/out] pld The payload of the single data packet
        \param[out] txn If not 0, the transaction that contains the response to
          the single data packet.
        \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
          CLIENT.

        \return ONS_SUCCESS if the payload was successfully parsed.
                ONS_BAD_PARAM if any of the parameters are invalid.
                ONS_INTERNAL_ERR if something unexpected occured.
                ONS_BAD_PKT If data in the packet is invalid.
                ONS_TXN_DOES_NOT_EXIST If the txn does not exist.
                See on_parse_pld for more possible return values.
    */
    #ifdef _ONE_NET_MULTI_HOP
        one_net_status_t on_client_b_s_data_hdlr(const UInt8 PID,
          const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
          on_txn_t ** txn, const UInt8 HOPS_TAKEN)
    #else // ifdef _ONE_NET_MULTI_HOP //
        one_net_status_t on_client_b_s_data_hdlr(const UInt8 PID,
          const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
          on_txn_t ** txn)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        const one_net_xtea_key_t * KEY = 0;

        one_net_raw_did_t raw_src_did;
        one_net_status_t status;
        on_sending_device_t * sender = 0;
        client_txn_t * client_txn = 0;

        UInt8 type;
        UInt8 txn_nonce, resp_nonce, msg_type;

        #ifdef _ONE_NET_MULTI_HOP
            if(!SRC_DID || !pld || !txn || HOPS_TAKEN > ON_MAX_HOPS_LIMIT)
        #else // ifdef _ONE_NET_MULTI_HOP //
            if(!SRC_DID || !pld || !txn)
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        {
            return ONS_BAD_PARAM;
        } // if any of the parameters are not valid //

        // whatever this pointed to is not the current txn.
        *txn = 0;

        switch(PID)
        {
            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_BLOCK_DATA:     // fall through
                case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:
            {
                #ifdef _ONE_NET_MULTI_HOP
                    if((PID == ONE_NET_ENCODED_BLOCK_DATA
                      || PID == ONE_NET_ENCODED_REPEAT_BLOCK_DATA)
                      && HOPS_TAKEN)
                    {
                        return ONS_BAD_PARAM;
                    } // if parameters are invalid //
                #endif // ifdef _ONE_NET_MULTI_HOP //

                type = ON_BLOCK;
                client_txn = &on_block_txn;
                KEY = &(on_base_param->current_key);
                break;
            } // (repeat)block data case //

            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_STREAM_DATA:    // fall through
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_STREAM_DATA:
            {
                #ifdef _ONE_NET_MULTI_HOP
                    if(PID == ONE_NET_ENCODED_STREAM_DATA && HOPS_TAKEN)
                    {
                        return ONS_BAD_PARAM;
                    } // if parameters are invalid //
                #endif // ifdef _ONE_NET_MULTI_HOP //
                type = ON_STREAM;
                client_txn = &on_stream_txn;
                KEY = &(on_base_param->stream_key);
                break;
            } // ONE_NET_ENCODED_STREAM_DATA case //

            default:
            {
                return ONS_BAD_PARAM;
                break;
            } // default case //
        } // switch(PID) //

        if(client_txn->txn.priority == ONE_NET_NO_PRIORITY)
        {
            return ONS_TXN_DOES_NOT_EXIST;
        } // if the txn does not exist //

        if(!on_encoded_did_equal(SRC_DID, (const on_encoded_did_t * const)
          &(client_txn->txn.pkt[ONE_NET_ENCODED_DST_DID_IDX])))
        {
            return ONS_INCORRECT_ADDR;
        } // if the msg was rxed from a device not part of the txn //

        // decode the address 1. To make sure it's a valid encoding. 2. Since
        // the address may need to be passed up to the app
        if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
          == ONS_SUCCESS)
        {
            if((sender = sender_info(SRC_DID)) == 0)
            {
                status = ONS_INTERNAL_ERR;
            } // if not able to get sender info //
            else
            {
                status = on_parse_pld(&txn_nonce, &resp_nonce, &msg_type, pld,
                  type, KEY);
            } // else the sender info was found //
        } // if decoding the address was successful //

        if(status == ONS_SUCCESS && txn_nonce == sender->expected_nonce)
        {
            if(msg_type == ON_EXTENDED_ADMIN_MSG)
            {
                status = handle_extended_block_admin_msg(
                  &(pld[ON_PLD_DATA_IDX]), client_txn);
            } // if an extended admin message //
            else if(msg_type != ON_APP_MSG)
            {
                status = ONS_BAD_PKT;
            } // else if msg_type is not an app message //
            else if(type == ON_STREAM && !on_stream_txn.txn.remaining)
            {
                on_encoded_did_t did;

                // send 0's since field is not used
                UInt8 data[ONE_NET_RAW_SINGLE_DATA_LEN] = {0};
                UInt8 txn_nonce;

                one_net_memmove(did,
                  &(on_stream_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  sizeof(did));
                if((txn_nonce = device_txn_nonce(
                  (const on_encoded_did_t * const)&did)) > ON_MAX_NONCE)
                {
                    return ONS_INVALID_DATA;
                } // if the nonce is not valid //

                data[0] = ON_END_STREAM;
                on_stream_txn.txn.data_len = on_stream_txn.txn.pkt_size;
                on_stream_txn.txn.expected_nonce = one_net_prand(one_net_tick(),
                  ON_MAX_NONCE);

                #ifdef _ONE_NET_MULTI_HOP
                    if((status = on_build_data_pkt(on_stream_txn.txn.pkt,
                      &(on_stream_txn.txn.data_len), ON_ADMIN_MSG,
                      HOPS_TAKEN ? ONE_NET_ENCODED_MH_SINGLE_DATA
                      : ONE_NET_ENCODED_SINGLE_DATA,
                      (const on_encoded_did_t * const)&did, txn_nonce,
                      on_stream_txn.txn.expected_nonce, data, sizeof(data),
                      (const one_net_xtea_key_t * const)
                      &(on_base_param->current_key), HOPS_TAKEN))
                      == ONS_SUCCESS)
                #else // ifdef _ONE_NET_MULTI_HOP //
                    if((status = on_build_data_pkt(on_stream_txn.txn.pkt,
                      &(on_stream_txn.txn.data_len), ON_ADMIN_MSG,
                      ONE_NET_ENCODED_SINGLE_DATA,
                      (const on_encoded_did_t * const)&did,
                      txn_nonce, on_stream_txn.txn.expected_nonce, data,
                      sizeof(data), (const one_net_xtea_key_t * const)
                      &(on_base_param->current_key))) == ONS_SUCCESS)
                #endif // else _ONE_NET_MULTI_HOP is not defined //
                {
                    // using the response transaction to send a single data pkt
                    // since we know that it has the necessary space & is
                    // available
                    status = ONS_STREAM_END;
                } // if building the end stream packet was successful //
            } // if stream is done //
            else if(type == ON_BLOCK && !one_net_client_handle_block_pkt(
              &(pld[ON_PLD_DATA_IDX]),
              client_txn->txn.remaining <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN
              ? client_txn->txn.remaining : ONE_NET_RAW_BLOCK_STREAM_DATA_LEN,
              (const one_net_raw_did_t * const)&raw_src_did))
            {
                status = ONS_UNHANDLED_PKT;
            } // else if block message not handled //
            else if(type == ON_STREAM && !one_net_client_handle_stream_pkt(
              &(pld[ON_PLD_DATA_IDX]), ONE_NET_RAW_BLOCK_STREAM_DATA_LEN,
              (const one_net_raw_did_t * const)&raw_src_did))
            {
                status = ONS_UNHANDLED_PKT;
            } // else if stream message not handled //

            if(status == ONS_SUCCESS)
            {
                #ifdef _ONE_NET_MULTI_HOP
                    UInt8 pid, hops;

                    if(type == ON_BLOCK)
                    {
                        pid = HOPS_TAKEN ? ONE_NET_ENCODED_MH_BLOCK_DATA_ACK
                          : ONE_NET_ENCODED_BLOCK_DATA_ACK;

                        // if it's a repeat multi-hop packet, allow MAX_HOPS
                        // since it may take more hops to get back than it took
                        // for the packet to arrive.
                        hops = (PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
                          ? ON_MAX_HOPS_LIMIT : HOPS_TAKEN);
                    } // if it's a block transaction //
                    else
                    {
                        pid = HOPS_TAKEN ? ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE
                          : ONE_NET_ENCODED_STREAM_KEEP_ALIVE;
                        hops = HOPS_TAKEN;
                    } // else it's a stream transaction;
                #endif // ifdef _ONE_NET_MULTI_HOP //

                client_txn->txn.retry = 0;
                sender->last_nonce = sender->expected_nonce;
                sender->expected_nonce = one_net_prand(one_net_tick(),
                  ON_MAX_NONCE);
                response_txn.data_len = response_txn.pkt_size;

                #ifdef _ONE_NET_MULTI_HOP
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len), pid, SRC_DID, resp_nonce,
                      sender->expected_nonce, hops);
                #else // ifdef _ONE_NET_MULTI_HOP //
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len),
                      type == ON_BLOCK ? ONE_NET_ENCODED_BLOCK_DATA_ACK
                      : ONE_NET_ENCODED_STREAM_KEEP_ALIVE, SRC_DID, resp_nonce,
                      sender->expected_nonce);
                #endif // else _ONE_NET_MULTI_HOP is not defined //
            } // if successful so far //
        } // if it's a new packet //
        else if(status == ONS_SUCCESS)
        {
            client_txn->txn.retry++;

            if(client_txn->txn.retry >= ON_MAX_RETRY)
            {
                switch(type)
                {
                    case ON_BLOCK:
                    {
                        status = ONS_BLOCK_FAIL;
                        one_net_client_block_txn_status(status,
                          (const one_net_raw_did_t * const)&raw_src_did);
                        break;
                    } // ON_BLOCK case //

                    case ON_STREAM:
                    {
                        status = ONS_STREAM_FAIL;
                        one_net_client_stream_txn_status(status,
                          (const one_net_raw_did_t * const)&raw_src_did);
                        break;
                    } // ON_STREAM case //

                    default:
                    {
                        status = ONS_INTERNAL_ERR;
                        break;
                    } // default case //
                } // switch(type) //

                return status;
            } // if the transaction failed too many times //

            response_txn.data_len = response_txn.pkt_size;
            #ifdef _ONE_NET_MULTI_HOP
                if(PID == ONE_NET_ENCODED_STREAM_DATA
                  || PID == ONE_NET_ENCODED_MH_STREAM_DATA)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if(PID == ONE_NET_ENCODED_STREAM_DATA)
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            {
                if(!on_stream_txn.txn.remaining)
                {
                    on_encoded_did_t did;

                    // send 0's since field is not used
                    UInt8 data[ONE_NET_RAW_SINGLE_DATA_LEN] = {0};
                    UInt8 txn_nonce;

                    one_net_memmove(did,
                      &(on_stream_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      sizeof(did));
                    if((txn_nonce = device_txn_nonce(
                      (const on_encoded_did_t * const)&did)) > ON_MAX_NONCE)
                    {
                        return ONS_INVALID_DATA;
                    } // if the nonce is not valid //

                    data[0] = ON_END_STREAM;
                    on_stream_txn.txn.data_len = on_stream_txn.txn.pkt_size;
                    on_stream_txn.txn.expected_nonce
                      = one_net_prand(one_net_tick(), ON_MAX_NONCE);

                    #ifdef _ONE_NET_MULTI_HOP
                        if((status = on_build_data_pkt(on_stream_txn.txn.pkt,
                          &(on_stream_txn.txn.data_len), ON_ADMIN_MSG,
                          HOPS_TAKEN ? ONE_NET_ENCODED_MH_SINGLE_DATA
                          : ONE_NET_ENCODED_SINGLE_DATA,
                          (const on_encoded_did_t * const)&did, txn_nonce,
                          on_stream_txn.txn.expected_nonce, data, sizeof(data),
                          (const one_net_xtea_key_t * const)
                          &(on_base_param->current_key), HOPS_TAKEN))
                          == ONS_SUCCESS)
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        if((status = on_build_data_pkt(on_stream_txn.txn.pkt,
                          &(on_stream_txn.txn.data_len), ON_ADMIN_MSG,
                          ONE_NET_ENCODED_SINGLE_DATA,
                          (const on_encoded_did_t * const)&did,
                          txn_nonce, on_stream_txn.txn.expected_nonce, data,
                          sizeof(data), (const one_net_xtea_key_t * const)
                          &(on_base_param->current_key))) == ONS_SUCCESS)
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                    {
                        // using the response transaction to send a single data
                        // pkt since we know that it has the necessary space &
                        // is available
                        status = ONS_STREAM_END;
                    } // if building the end stream packet was successful //
                } // if stream is done //
                else
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        status = on_build_response_pkt(response_txn.pkt,
                          &(response_txn.data_len),
                          HOPS_TAKEN ? ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE
                          : ONE_NET_ENCODED_STREAM_KEEP_ALIVE, SRC_DID,
                          resp_nonce, sender->expected_nonce, HOPS_TAKEN);
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        status = on_build_response_pkt(response_txn.pkt,
                          &(response_txn.data_len),
                          ONE_NET_ENCODED_STREAM_KEEP_ALIVE, SRC_DID,
                          resp_nonce, sender->expected_nonce);
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                } // else handle the stream normally //
            } // if it's a stream packet //
            #ifdef _ONE_NET_MULTI_HOP
                else if((PID == ONE_NET_ENCODED_REPEAT_BLOCK_DATA
                  || PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA) && txn_nonce
                  == sender->last_nonce)
                {
                    // if it's a repeat multi-hop packet, allow MAX_HOPS since
                    // it may take more hops to get back than it took for the
                    // packet to arrive.
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len),
                      PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
                      ? ONE_NET_ENCODED_MH_BLOCK_DATA_ACK
                      : ONE_NET_ENCODED_BLOCK_DATA_ACK, SRC_DID, resp_nonce,
                      sender->expected_nonce,
                      PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
                      ? ON_MAX_HOPS_LIMIT : HOPS_TAKEN);
                } // else if it is a repeat packet //
                else
                {
                    // if it's a repeat multi-hop packet, allow MAX_HOPS since
                    // it may take more hops to get back than it took for the
                    // packet to arrive.
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len),
                      HOPS_TAKEN ? ONE_NET_ENCODED_MH_BLOCK_DATA_NACK
                      : ONE_NET_ENCODED_BLOCK_DATA_NACK, SRC_DID, resp_nonce,
                      sender->expected_nonce,
                      PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
                      ? ON_MAX_HOPS_LIMIT : HOPS_TAKEN);
                } // else the nonce is wrong //
            #else // ifdef _ONE_NET_MULTI_HOP //
                else if(PID == ONE_NET_ENCODED_REPEAT_BLOCK_DATA && txn_nonce
                  == sender->last_nonce)
                {
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len), ONE_NET_ENCODED_BLOCK_DATA_ACK,
                      SRC_DID, resp_nonce, sender->expected_nonce);
                } // else if it is a repeat packet //
                else
                {
                    status = on_build_response_pkt(response_txn.pkt,
                      &(response_txn.data_len), ONE_NET_ENCODED_BLOCK_DATA_NACK,
                      SRC_DID, resp_nonce, sender->expected_nonce);
                } // else the nonce is wrong //
            #endif // else _ONE_NET_MULTI_HOP is not defined //
        } // else if the packet was successfully parsed //

        if(status == ONS_SUCCESS)
        {
            if(type == ON_BLOCK && !on_block_txn.txn.retry)
            {
                if(on_block_txn.txn.remaining
                  <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
                {
                    on_block_txn.txn.remaining = 0;
                    status = ONS_BLOCK_END;
                } // if the block transaction is complete //
                else
                {
                    on_block_txn.txn.remaining
                      -= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
                } // else more blocks remain //
            } // if a block transaction //

            *txn = &response_txn;
        } // if building the response was successful //
        else if(status == ONS_STREAM_END)
        {
            *txn = &(on_stream_txn.txn);
        } // if ending the stream //
        else
        {
            *txn = 0;
        } // else building the response was not successful //

        return status;
    } // on_client_b_s_data_hdlr //


    /*!
        \brief Handles the status of a block or stream transaction.

        This is called whenever there is a status event for a block or stream
        transaction (sending or receiving).

        \param[in] txn The transaction whose status is being reported.
        \param[in] NEXT_NONCE ON_MAX_NONCE + 1 if this field is to be ignored,
                    otherwise the next nonce for the receiving device.
        \param[in] STATUS The result of the transaction.
        \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
          CLIENT

        \return ONS_SUCCESS If handling the transaction was successful
                ONS_BAD_PARAM if any of the parameters are invalid.
                ONS_INCORRECT_ADDR If the address is for a device not on this
                  network.
                ONS_BLOCK_END If the block transaction is complete.
                ONS_INTERNAL_ERR If the transaction was not handled.
    */
    #ifdef _ONE_NET_MULTI_HOP
        one_net_status_t on_client_b_s_txn_hdlr(on_txn_t ** txn,
          const UInt8 NEXT_NONCE, const one_net_status_t STATUS,
          const UInt8 HOPS_TAKEN)
    #else // ifdef _ONE_NET_MULTI_HOP //
        one_net_status_t on_client_b_s_txn_hdlr(on_txn_t ** txn,
          const UInt8 NEXT_NONCE, const one_net_status_t STATUS)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        const BOOL SEND_TXN = (STATUS == ONS_SUCCESS || STATUS == ONS_TIME_OUT
          || STATUS == ONS_RX_NACK || STATUS == ONS_BLOCK_FAIL
          || STATUS == ONS_STREAM_FAIL);

        one_net_status_t rv = ONS_INTERNAL_ERR;

        if((SEND_TXN && (!txn || !*txn || !(*txn)->send))
          || (!SEND_TXN && (*txn)->send)
          || (NEXT_NONCE <= ON_MAX_NONCE && !SEND_TXN))
        {
            return ONS_INTERNAL_ERR;
        } // if the txn not correct //

        set_device_txn_nonce((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), NEXT_NONCE);

        // update the hops field
        #ifdef _ONE_NET_MULTI_HOP
            if(HOPS_TAKEN <= ON_MAX_HOPS_LIMIT && (STATUS == ONS_SUCCESS
              || STATUS == ONS_RX_STAY_AWAKE || STATUS == ONS_RX_NACK))
            {
                update_device_hops((const on_encoded_did_t * const)
                  &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), HOPS_TAKEN);
            } // if the hops field should be updated //
        #endif // ifdef _ONE_NET_MULTI_HOP //

        switch(STATUS)
        {
            case ONS_SUCCESS:
            {
                (*txn)->retry = 0;
                if((*txn) == &(on_block_txn.txn) && (*txn)->remaining == 0)
                {
                    one_net_raw_did_t raw_did;

                    if((rv = on_decode(raw_did,
                      &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
                    {
                        return rv;
                    } // if decoding the destination did was not successful //

                    on_block_txn.txn.priority = ONE_NET_NO_PRIORITY;
                    *txn = &response_txn;

                    if(on_block_txn.txn.msg_type == ON_APP_MSG)
                    {
                        one_net_client_block_txn_status(ONS_BLOCK_END,
                          (const one_net_raw_did_t * const)&raw_did);
                    } // if an app layer message //

                    return ONS_BLOCK_END;
                } // if that was the last block //

                on_update_next_txn_time(*txn);
                break;
            } // ONS_SUCCESS case //

            case ONS_TIME_OUT:                          // fall through
            case ONS_RX_NACK:
            {
                // Need to resend the message
                UInt8 txn_nonce;
                UInt8 msg_type;
                UInt8 pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];

                if((*txn) != &(on_block_txn.txn) && (*txn)
                  != &(on_stream_txn.txn))
                {
                    return ONS_INTERNAL_ERR;
                } // if not carrying out a block or stream transaction //

                if((*txn) == &(on_block_txn.txn))
                {
                    on_decode(pld, &((*txn)->pkt[ON_PLD_IDX]),
                      ON_ENCODED_BLOCK_STREAM_PLD_SIZE);
                    if((rv = on_parse_pld(&txn_nonce, &((*txn)->expected_nonce),
                      &msg_type, pld, ON_BLOCK,
                      (const one_net_xtea_key_t * const)
                      &(on_base_param->current_key))) != ONS_SUCCESS)
                    {
                        return rv;
                    } // if parsing the payload is not successful //

                    (*txn)->data_len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
                    one_net_memmove(&((*txn)->pkt[ON_DATA_IDX]),
                      &(pld[ON_PLD_DATA_IDX]), (*txn)->data_len);
                } // if a block transaction //

                on_update_next_txn_time(*txn);
                break;
            } // ONS_TIME_OUT/ONS_RX_NACK case //

            case ONS_BLOCK_END:                         // fall through
            case ONS_BLOCK_FAIL:
            {
                one_net_raw_did_t raw_did;

                on_block_txn.txn.priority = ONE_NET_NO_PRIORITY;
                *txn = &response_txn;

                if((rv = on_decode(raw_did,
                  &(on_block_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
                {
                    return rv;
                } // if decoding the destination did was not successful //

                if(on_block_txn.txn.msg_type == ON_APP_MSG)
                {
                    one_net_client_block_txn_status(STATUS,
                      (const one_net_raw_did_t * const)&raw_did);
                } // if an app layer message //
                break;
            } // ONS_BLOCK_END/ONS_BLOCK_FAIL case //

            case ONS_STREAM_END:                        // fall through
            case ONS_STREAM_FAIL:
            {
                one_net_raw_did_t raw_did;

                if((*txn) != &(on_stream_txn.txn))
                {
                    return ONS_INTERNAL_ERR;
                } // if not sending //

                if((rv = on_decode(raw_did,
                  &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
                {
                    return rv;
                } // if decoding the destination did was not successful //

                one_net_client_stream_txn_status(STATUS,
                  (const one_net_raw_did_t * const)&raw_did);
                break;
            } // ONS_STREAM_END/ONS_STREAM_FAIL case //

            default:
            {
                rv = ONS_INTERNAL_ERR;
                break;
            } // default case //
        } // switch(STATUS) //

        return rv;
    } // on_client_b_s_txn_hdlr //


    /*!
        \brief Handles reception of a block fragment extended admin message.

        ON_CHANGE_STREAM_KEY is currently the only extended admin message.  This
        function copies the stream key to on_base_param.

        \param[in] DATA The data that was received in the block fragment
        \param[in] BLOCK_TXN The block transaction for which the data was
          received.

        \return ONS_SUCCESS If the data was successfully handled.
                ONS_BAD_PARAM If any of the parameters are invalid.
                ONS_INVALID_DATA If the data is not correct.
    */
    static one_net_status_t handle_extended_block_admin_msg(
      const UInt8 * const DATA, const client_txn_t * const BLOCK_TXN)
    {
        if(!DATA || !BLOCK_TXN)
        {
            return ONS_BAD_PARAM;
        } // if the parameters are invalid //

        if(BLOCK_TXN->txn.remaining != sizeof(on_base_param->stream_key))
        {
            return ONS_INVALID_DATA;
        } // if the data is invalid //

        #ifdef _ONE_NET_EVAL
            oncli_print_admin_msg(ON_EXTENDED_ADMIN_MSG, ON_BLOCK,
              ON_CHANGE_STREAM_KEY, DATA, sizeof(on_base_param->stream_key));
        #endif // ifdef ONE_NET_EVAL //

        one_net_memmove(on_base_param->stream_key, DATA,
          sizeof(on_base_param->stream_key));

        // done getting the stream key
        ont_stop_timer(ONT_STREAM_KEY_TIMER);
        return ONS_SUCCESS;
    } // handle_extended_block_admin_msg //
#endif // _ONE_NET_SIMPLE_CLIENT //


/*!
    \brief Handles the end of the data rate test.

    The CLIENT sends the results of the data rate test to the MASTER.

    \param[in] RATE The data rate that was tested.
    \param[in] DID The device the CLIENT tested the data rate with.
    \param[in] RESULT The number of successful response the CLIENT heard.

    \return void
*/
void on_client_data_rate_hdlr(const UInt8 RATE,
  const on_encoded_did_t * const DID, const UInt8 RESULT)
{
    on_single_txn.txn.priority = ONE_NET_HIGH_PRIORITY;
    on_single_txn.txn.retry = 0;

    on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

    // make sure the pid is not the data rate pid
    on_single_txn.txn.pkt[ONE_NET_ENCODED_PID_IDX]
      = ONE_NET_ENCODED_SINGLE_DATA;
    on_single_txn.txn.msg_type = ON_ADMIN_MSG;
    on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_DATA_RATE_RESULT;

    on_single_txn.txn.pkt[ON_DATA_IDX + ON_DATA_RATE_DATA_RATE_IDX] = RATE;
    one_net_memmove(
      &(on_single_txn.txn.pkt[ON_DATA_IDX + ON_DATA_RATE_DID_IDX]), *DID,
      ON_ENCODED_DID_LEN);
    on_single_txn.txn.pkt[ON_DATA_IDX + ON_DATA_RATE_RESULT_IDX] = RESULT;

    one_net_memmove(&(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
      master->device.did, ON_ENCODED_DID_LEN);

    report_data_rate_results = TRUE;
} // on_client_data_rate_hdlr //

/*!
 *     \brief Provide the information needed to allow this client to join a ONE-NET network.
 *
 *     For those cases where the security and other network configuration information will
 *     be provided to a client device by a means other than the normal invite/join
 *     process, this function may be used. The input argument is a pointer to a data
 *     structure that contains the information the client needs to operate in a
 *     ONE-NET network.
 *
 *     \param[in] DATA Pointer to network configuration information for this client.
 *     \return ONS_SUCCESS if the join worked, any other return code indicates that
 *     an error was encountered.
 */
one_net_status_t one_net_client_join_network(one_net_client_join_network_data_t *DATA)
{

    if (!DATA)
    {
        return(ONS_BAD_PARAM);
    }

    on_base_param = (on_base_param_t *)nv_param;

    // set the version number for the nv_param layout
    on_base_param->version = ON_PARAM_VERSION;

    // copy the values provided to the appropriate ONE-NET client data structures
    on_base_param->channel = DATA->channel;
    on_base_param->data_rate = DATA->data_rate;
    on_base_param->single_block_encrypt = DATA->single_block_encrypt_method;
    one_net_memmove(&(on_base_param->current_key[0]), &(DATA->current_key[0]), sizeof(one_net_xtea_key_t));
    if (on_encode(&(on_base_param->sid[0]), &(DATA->raw_sid[0]), sizeof(on_encoded_sid_t)) != ONS_SUCCESS)
    {
        return(ONS_BAD_PARAM);
    }
    if (on_encode(&(master->device.did[0]), &(DATA->raw_master_did[0]), sizeof(on_encoded_did_t))
      != ONS_SUCCESS)
    {
        return(ONS_BAD_PARAM);
    }
    master->keep_alive_interval = DATA->keep_alive_interval;
    master->settings.master_data_rate = DATA->master_data_rate;
#ifdef _ONE_NET_MULTI_HOP
    master->device.max_hops = DATA->max_hops;
#endif
#ifndef _ONE_NET_SIMPLE_CLIENT
    one_net_memmove(&(on_base_param->stream_key), DATA->current_key, sizeof(one_net_xtea_key_t));
    on_base_param->stream_encrypt = DATA->stream_encrypt_method;
    on_base_param->fragment_delay_low = DATA->fragment_delay_low;
    on_base_param->fragment_delay_high = DATA->fragment_delay_high;
#endif

    // initialize other values
    master->device.expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);
    master->device.last_nonce = one_net_prand(one_net_tick() - 1, ON_MAX_NONCE);
    master->device.send_nonce = 0;
    master->settings.flags = 0x00;
    master->settings.flags |= ON_JOINED;
    save_param();
    on_state = ON_LISTEN_FOR_DATA;

    return(ONS_SUCCESS);
} //one_net_client_join_network //



/*!
    \brief Handles admin packets.

    \param[in] SRC The sender of the admin packet.
    \param[in] DATA The admin packet.
    \param[in] DATA_LEN The length of the admin packet.

    \return ONS_SUCCESS If the packet was successfully parsed.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_RSRC_FULL If the resources to send an appropriate response are
              full.
            ONS_UNHANDLED_PKT If the admin message type is not handled by the
              device
            ONS_INVALID_DATA If the received data is not valid.
            ONS_INCORRECT_ADDR If a packet was received from an unexpected
              sender.
            ONS_INTERNAL_ERR If this is returned, something unexpected happened.
*/
static one_net_status_t handle_admin_pkt(const on_encoded_did_t * const SRC,
  const UInt8 * const DATA, const UInt8 DATA_LEN)
{
    one_net_status_t status = ONS_INTERNAL_ERR;

    if(!SRC || !DATA || DATA_LEN != ONE_NET_RAW_SINGLE_DATA_LEN)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    #ifdef _ONE_NET_EVAL
        oncli_print_admin_msg(ON_ADMIN_MSG, ON_SINGLE,
          DATA[ON_ADMIN_MSG_ID_IDX], &(DATA[ON_ADMIN_DATA_IDX]),
          ON_MAX_ADMIN_PLD_LEN);
    #endif // ifdef ONE_NET_EVAL //

    #ifndef _ONE_NET_SIMPLE_CLIENT
        if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP
          && DATA[ON_ADMIN_MSG_ID_IDX] != ON_END_STREAM)
        {
            return ONS_BAD_PKT_TYPE;
        } // if waiting for a stream data response //
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

    switch(DATA[ON_ADMIN_MSG_ID_IDX])
    {
        case ON_STATUS_QUERY:
        {
            status = send_status_resp(SRC);
            break;
        } // status query case //

        case ON_SETTINGS_QUERY:
        {
            status = send_settings_resp(SRC);
            break;
        } // settings query case //

        case ON_CHANGE_SETTINGS:
        {
            if(DATA[ON_SETTINGS_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]
              > ONE_NET_MAX_DATA_RATE
              || DATA[ON_SETTINGS_MASTER_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]
              > ONE_NET_MAX_DATA_RATE)
            {
                status = ONS_INVALID_DATA;
            } // if the settings are not valid //
            else
            {
                on_base_param->data_rate = DATA[ON_SETTINGS_DATA_RATE_IDX
                  + ON_ADMIN_DATA_IDX];
                master->settings.master_data_rate
                  = DATA[ON_SETTINGS_MASTER_DATA_RATE_IDX + ON_ADMIN_DATA_IDX];

                if(DATA[ON_SETTINGS_FLAG_IDX + ON_ADMIN_DATA_IDX]
                  & ON_SEND_TO_MASTER)
                {
                    master->settings.flags |= ON_SEND_TO_MASTER;
                } // if the MASTER should be updated //
                else
                {
                    master->settings.flags &= ~ON_SEND_TO_MASTER;
                } // else do not update the MASTER //

                save = TRUE;

                status = send_settings_resp(SRC);
            } // else store the new settings //
            break;
        } // change settings case //

        #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_FRAGMENT_DELAY_QUERY:
            {
                on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                on_single_txn.txn.retry = 0;

                on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

                on_single_txn.txn.msg_type = ON_ADMIN_MSG;
                on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX]
                  = ON_FRAGMENT_DELAY_RESP;

                one_net_memmove(
                  &(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  *SRC, sizeof(*SRC));

                // Check if quering the high or low priority value
                if(DATA[ON_ADMIN_DATA_IDX] == (ONE_NET_HIGH_PRIORITY
                  - ONE_NET_LOW_PRIORITY))
                {
                    one_net_int32_to_byte_stream(
                      one_net_tick_to_ms(on_base_param->fragment_delay_high),
                      &(on_single_txn.txn.pkt[ON_DATA_IDX]));
                } // if high priority is being queried //
                else if(DATA[ON_ADMIN_DATA_IDX] == (ONE_NET_LOW_PRIORITY
                  - ONE_NET_LOW_PRIORITY))
                {
                    one_net_int32_to_byte_stream(
                      one_net_tick_to_ms(on_base_param->fragment_delay_low),
                      &(on_single_txn.txn.pkt[ON_DATA_IDX]));
                } // else if a low priority txn //
                else
                {
                    status = ONS_INVALID_DATA;
                    break;
                } // else invalid data //

                status = ONS_SUCCESS;
                break;
            } // fragment delay query case //

            case ON_CHANGE_LOW_FRAGMENT_DELAY:          // fall through
            case ON_CHANGE_HIGH_FRAGMENT_DELAY:
            {
                tick_t delay
                  = one_net_ms_to_tick(
                  one_net_byte_stream_to_int32(&DATA[ON_ADMIN_DATA_IDX]));

                if(DATA[ON_ADMIN_MSG_ID_IDX] == ON_CHANGE_HIGH_FRAGMENT_DELAY)
                {
                    if(delay > on_base_param->fragment_delay_low)
                    {
                        status = ONS_INVALID_DATA;
                        break;
                    } // if high priority fragment delay longer than low //

                    on_base_param->fragment_delay_high = delay;
                } // if changing the high priority time //
                else
                {
                    if(delay < on_base_param->fragment_delay_high)
                    {
                        status = ONS_INVALID_DATA;
                        break;
                    } // if high priority fragment delay longer than low //

                    on_base_param->fragment_delay_low = delay;
                } // else changing the low priority time //

                one_net_int32_to_byte_stream(one_net_tick_to_ms(delay),
                  &(on_single_txn.txn.pkt[ON_DATA_IDX]));
                save = TRUE;

                on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                on_single_txn.txn.retry = 0;

                on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

                on_single_txn.txn.msg_type = ON_ADMIN_MSG;
                on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX]
                  = ON_FRAGMENT_DELAY_RESP;

                one_net_memmove(
                  &(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  *SRC, sizeof(*SRC));

                status = ONS_SUCCESS;
                break;
            } // change fragment delay case //
        #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

        case ON_KEEP_ALIVE_QUERY:
        {
            on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
            on_single_txn.txn.retry = 0;

            on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

            on_single_txn.txn.msg_type = ON_ADMIN_MSG;
            on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_KEEP_ALIVE_RESP;

            one_net_memmove(
              &(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
              *SRC, sizeof(*SRC));

            one_net_int32_to_byte_stream(
              one_net_tick_to_ms(master->keep_alive_interval),
              &(on_single_txn.txn.pkt[ON_DATA_IDX]));

            status = ONS_SUCCESS;
            break;
        } // keep alive query case //

        case ON_CHANGE_KEEP_ALIVE:
        {
            if(!on_encoded_did_equal(SRC,
              (const on_encoded_did_t * const)&(master->device.did)))
            {
                status = ONS_INCORRECT_ADDR;
            } // if it is not from the MASTER //
            else
            {
                master->keep_alive_interval
                  = one_net_ms_to_tick(
                  one_net_byte_stream_to_int32(&DATA[ON_ADMIN_DATA_IDX]));

                save = TRUE;

                on_single_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                on_single_txn.txn.retry = 0;

                on_single_txn.txn.data_len = ON_MAX_ADMIN_PLD_LEN;

                on_single_txn.txn.msg_type = ON_ADMIN_MSG;
                on_single_txn.txn.pkt[ON_ADMIN_TYPE_IDX] = ON_KEEP_ALIVE_RESP;

                one_net_memmove(
                  &(on_single_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  *SRC, sizeof(*SRC));

                one_net_int32_to_byte_stream(
                  one_net_tick_to_ms(master->keep_alive_interval),
                  &(on_single_txn.txn.pkt[ON_DATA_IDX]));

                status = ONS_SUCCESS;
            } // else it is from the MASTER //
            break;
        } // change keep alive case //

        case ON_NEW_KEY_FRAGMENT:
        {
            if(confirm_key_update)
            {
                // Since the key change has not been confirmed, this is likely
                // a repeat of the key change, so ignore it until it is
                // confirmed
                status = ONS_RSRC_FULL;
                break;
            } // if the key change has not been cofirmed //

            // shift the key
            one_net_memmove(on_base_param->current_key,
              &(on_base_param->current_key[ON_MAX_ADMIN_PLD_LEN]),
              sizeof(on_base_param->current_key) - ON_MAX_ADMIN_PLD_LEN);
            // copy in the new key fragment
            one_net_memmove(
              &(on_base_param->current_key[sizeof(on_base_param->current_key)
              - ON_MAX_ADMIN_PLD_LEN]), &(DATA[ON_ADMIN_DATA_IDX]),
              ON_MAX_ADMIN_PLD_LEN);

            confirm_key_update = TRUE;
            save = TRUE;
            status = ONS_SUCCESS;
            break;
        } // new key fragment case //
#ifdef _PEER
        case ON_ASSIGN_PEER:                            // fall through
#ifdef _ONE_NET_MULTI_HOP
        case ON_ASSIGN_MH_PEER:
        {
            #ifdef _ONE_NET_MULTI_HOP
                if((status = on_client_net_assign_peer(DATA[ON_PEER_SRC_UNIT_IDX
                  + ON_ADMIN_DATA_IDX], (const on_encoded_did_t * const)
                  &(DATA[ON_PEER_DID_IDX + ON_ADMIN_DATA_IDX]),
                  DATA[ON_PEER_PEER_UNIT_IDX + ON_ADMIN_DATA_IDX],
                  DATA[ON_ADMIN_MSG_ID_IDX] == ON_ASSIGN_MH_PEER))
                  == ONS_SUCCESS)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if((status = on_client_net_assign_peer(DATA[ON_PEER_SRC_UNIT_IDX
                  + ON_ADMIN_DATA_IDX], (const on_encoded_did_t * const)
                  &(DATA[ON_PEER_DID_IDX + ON_ADMIN_DATA_IDX]),
                  DATA[ON_PEER_PEER_UNIT_IDX + ON_ADMIN_DATA_IDX]))
                  == ONS_SUCCESS)
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            {
                save = TRUE;
                status = ONS_SUCCESS;
            } // if the peer was successfully assigned //
            break;
        } // assign peer case //
#endif

        case ON_UNASSIGN_PEER:
        {
            if((status = on_client_net_unassign_peer(DATA[ON_PEER_SRC_UNIT_IDX
              + ON_ADMIN_DATA_IDX], (const on_encoded_did_t * const)
              &DATA[ON_PEER_DID_IDX + ON_ADMIN_DATA_IDX],
              DATA[ON_PEER_PEER_UNIT_IDX + ON_ADMIN_DATA_IDX])) == ONS_SUCCESS)
            {
                save = TRUE;
            } // if successfully unassigned the peer //

            break;
        } // unassign peer case //

        // 12/10/2010 - If the master is running the Eval Board code, the client will
		// never receive this message.  I'm leaving it in anyway for now though.
        case ON_CHANGE_PEER_DATA_RATE:
        {
            UInt8 data_rate = DATA[ON_PEER_SETTING_DATA_RATE_IDX
              + ON_ADMIN_DATA_IDX];

            if(data_rate > ONE_NET_MAX_DATA_RATE)
            {
                status = ONS_INVALID_DATA;
                break;
            } // if the this device can't handle the data rate //

            if(on_client_net_set_peer_data_rate((const on_encoded_did_t * const)
              &(DATA[ON_PEER_DID_IDX + ON_ADMIN_DATA_IDX]), data_rate))
            {
                save = TRUE;
                status = ONS_SUCCESS;
            } // if the peer is invalid //
            else
            {
                status = ONS_INVALID_DATA;
            } // else the peer is valid //
            break;
        } // ON_CHANGE_PEER_DATA_RATE case //
#endif

        #ifndef _ONE_NET_SIMPLE_CLIENT
            case ON_SEND_BLOCK_LOW:                     // fall through
            case ON_RECV_BLOCK_LOW:                     // fall through
            case ON_SEND_BLOCK_HIGH:                    // fall through
            case ON_RECV_BLOCK_HIGH:
            {
                one_net_raw_did_t raw_src_did;
                UInt16 cmd_and_data_type;

                if((status = on_decode(raw_src_did, *SRC, sizeof(*SRC)))
                  != ONS_SUCCESS)
                {
                    break;
                } // if decoding the did failed //

                if(on_block_txn.txn.priority != ONE_NET_NO_PRIORITY)
                {
                    status = ONS_RSRC_FULL;
                    break;
                } // if the resource is in use //

                on_block_txn.requesting = FALSE;
                on_block_txn.txn.send = (DATA[ON_ADMIN_MSG_ID_IDX]
                  == ON_SEND_BLOCK_LOW || DATA[ON_ADMIN_MSG_ID_IDX]
                  == ON_SEND_BLOCK_HIGH) ? TRUE : FALSE;
                on_block_txn.txn.remaining
                  = one_net_byte_stream_to_int16(&(DATA[ON_BLOCK_LEN_IDX
                  + ON_ADMIN_DATA_IDX]));

                cmd_and_data_type = one_net_byte_stream_to_int16(
                  &(DATA[ON_BLOCK_STREAM_DATA_TYPE_IDX + ON_ADMIN_DATA_IDX]));
                if(one_net_client_txn_requested(ON_BLOCK, on_block_txn.txn.send,
                  cmd_and_data_type, on_block_txn.txn.remaining,
                  (const one_net_raw_did_t * const)&raw_src_did))
                {
                    one_net_memmove(
                      &(on_block_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      *SRC, ON_ENCODED_DID_LEN);
                    if(DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_BLOCK_HIGH
                      || DATA[ON_ADMIN_MSG_ID_IDX] == ON_RECV_BLOCK_HIGH)
                    {
                        on_block_txn.txn.priority = ONE_NET_HIGH_PRIORITY;
                        ont_set_timer(on_block_txn.txn.next_txn_timer,
                          on_base_param->fragment_delay_high);
                    } // if a high priority transaction //
                    else
                    {
                        on_block_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                        ont_set_timer(on_block_txn.txn.next_txn_timer,
                          on_base_param->fragment_delay_low);
                    } // else a low priority transaction //
                    on_block_txn.txn.retry = 0;
                    on_block_txn.txn.msg_type = ON_APP_MSG;
                    status = ONS_SUCCESS;
                } // if request is granted //
                else
                {
                    status = ONS_UNHANDLED_PKT;
                } // else request is not granted //
                break;
            } // send block case //

            case ON_SEND_STREAM_LOW:                    // fall through
            case ON_RECV_STREAM_LOW:                    // fall through
            case ON_SEND_STREAM_HIGH:                   // fall through
            case ON_RECV_STREAM_HIGH:
            {
                one_net_raw_did_t raw_src_did;
                UInt16 cmd_and_data_type;

                if((status = on_decode(raw_src_did, *SRC, sizeof(*SRC)))
                  != ONS_SUCCESS)
                {
                    break;
                } // if decoding the did failed //

                if(on_stream_txn.txn.priority != ONE_NET_NO_PRIORITY)
                {
                    status = ONS_RSRC_FULL;
                    break;
                } // if the resource is in use //

                on_stream_txn.requesting = FALSE;

                on_stream_txn.txn.send =
                  (DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_LOW
                  || DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_HIGH)
                  ? TRUE : FALSE;

                cmd_and_data_type = one_net_byte_stream_to_int16(
                  &(DATA[ON_BLOCK_STREAM_DATA_TYPE_IDX + ON_ADMIN_DATA_IDX]));
                if(one_net_client_txn_requested(ON_STREAM,
                  on_stream_txn.txn.send, cmd_and_data_type, 0,
                  (const one_net_raw_did_t * const)&raw_src_did))
                {
                    one_net_memmove(
                      &(on_stream_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      *SRC, ON_ENCODED_DID_LEN);
                    if(DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_HIGH
                      || DATA[ON_ADMIN_MSG_ID_IDX] == ON_RECV_STREAM_HIGH)
                    {
                        on_stream_txn.txn.priority = ONE_NET_HIGH_PRIORITY;
                        ont_set_timer(on_stream_txn.txn.next_txn_timer,
                          on_base_param->fragment_delay_high);
                    } // if a high priority transaction //
                    else
                    {
                        on_stream_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                        ont_set_timer(on_stream_txn.txn.next_txn_timer,
                          on_base_param->fragment_delay_low);
                    } // else a low priority transaction //
                    // mark to continue the stream
                    on_stream_txn.txn.remaining = 1;
                    on_stream_txn.txn.msg_type = ON_APP_MSG;
                    status = ONS_SUCCESS;
                } // if request is granted //
                else
                {
                    status = ONS_UNHANDLED_PKT;
                } // else request is not granted //
                break;
            } // send stream case //

            case ON_END_STREAM:
            {
                if(!on_encoded_did_equal(SRC, (const on_encoded_did_t * const)
                  &(on_stream_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX])))
                {
                    status = ONS_INCORRECT_ADDR;
                } // if the msg was rxed from a device not part of the txn //
                else
                {
                    one_net_raw_did_t raw_did;

                    // decode the address
                    if((status = on_decode(raw_did, *SRC, sizeof(*SRC)))
                      != ONS_SUCCESS)
                    {
                        break;
                    } // if decoding the address was not successful //

                    on_stream_txn.txn.priority = ONE_NET_NO_PRIORITY;
                    one_net_client_stream_txn_status(ONS_STREAM_END,
                      (const one_net_raw_did_t * const)&raw_did);
                    status = ONS_STREAM_END;
                } // else the transaction was found //
                break;
            } // end stream case //
        #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

        case ON_INIT_DATA_RATE_TEST:
        {
            if(DATA[ON_DATA_RATE_DATA_RATE_IDX
              + ON_ADMIN_DATA_IDX] > ONE_NET_MAX_DATA_RATE)
            {
                status = ONS_INVALID_DATA;
            } // if the data is invalid //
            else if(on_single_txn.txn.priority == ONE_NET_NO_PRIORITY)
            {
                #ifdef _ONE_NET_MULTI_HOP
                    UInt8 hops = 0;
                #endif // ifdef _ONE_NET_MULTI_HOP //
                on_single_txn.txn.data_len = on_single_txn.txn.pkt_size;

                #ifdef _ONE_NET_MULTI_HOP
                    if(DATA[ON_DATA_RATE_FLAG_IDX + ON_ADMIN_DATA_IDX]
                      & ON_MH_DATA_RATE_TEST_FLAG)
                    {
                        hops = ON_MAX_HOPS_LIMIT;
                    } // if doing a mh data rate test //

                    status = on_build_data_rate_pkt(on_single_txn.txn.pkt,
                      &(on_single_txn.txn.data_len),
                      (const on_encoded_did_t * const)
                      &(DATA[ON_DATA_RATE_DID_IDX + ON_ADMIN_DATA_IDX]),
                      DATA[ON_DATA_RATE_DATA_RATE_IDX + ON_ADMIN_DATA_IDX],
                      hops);
                #else // ifdef _ONE_NET_MULTI_HOP //
                    if(DATA[ON_DATA_RATE_FLAG_IDX + ON_ADMIN_DATA_IDX]
                      & ON_MH_DATA_RATE_TEST_FLAG)
                    {
                        status = ONS_INVALID_DATA;
                    } // if a mh data rate test should be performed //
                    else
                    {
                        status = on_build_data_rate_pkt(on_single_txn.txn.pkt,
                          &(on_single_txn.txn.data_len),
                          (const on_encoded_did_t * const)
                          &(DATA[ON_DATA_RATE_DID_IDX + ON_ADMIN_DATA_IDX]),
                          DATA[ON_DATA_RATE_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]);
                    } // else a non mh data rate test should be performed //
                #endif // else _ONE_NET_MULTI_HOP is not defined //

                if(status == ONS_SUCCESS)
                {
                    on_single_txn.txn.priority = ONE_NET_HIGH_PRIORITY;
                } // if building the packet was successful //
            } // else if the resource is available //
            else
            {
                status = ONS_RSRC_FULL;
            } // else the resource is not available //
            break;
        } // init data rate case //

        case ON_RM_DEV:
        {
            removed = TRUE;
            status = ONS_SUCCESS;
            break;
        } // remove device case //

        #ifdef _ONE_NET_SIMPLE_CLIENT
            case ON_FRAGMENT_DELAY_QUERY:               // fall through
            case ON_FRAGMENT_DELAY_RESP:                // fall through
            case ON_CHANGE_LOW_FRAGMENT_DELAY:          // fall through
            case ON_CHANGE_HIGH_FRAGMENT_DELAY:         // fall through
            case ON_SEND_BLOCK_LOW:                     // fall through
            case ON_RECV_BLOCK_LOW:                     // fall through
            case ON_SEND_STREAM_LOW:                    // fall through
            case ON_RECV_STREAM_LOW:                    // fall through
            case ON_SEND_BLOCK_HIGH:                    // fall through
            case ON_RECV_BLOCK_HIGH:                    // fall through
            case ON_SEND_STREAM_HIGH:                   // fall through
            case ON_RECV_STREAM_HIGH:                   // fall through
            case ON_END_STREAM:                         // fall through
        #endif
        case ON_STATUS_RESP:                            // fall through
        case ON_SETTINGS_RESP:                          // fall through
        case ON_KEEP_ALIVE_RESP:                        // fall through
        case ON_DATA_RATE_RESULT:
        {
            status = ONS_UNHANDLED_PKT;
            break;
        } // unhandled cases //

        default:
        {
            status = ONS_BAD_PKT_TYPE;
            break;
        } // default case //
    } // switch(DATA[ON_ADMIN_MSG_ID_IDX]) //

    return status;
} // handle_admin_pkt //


#ifndef _ONE_NET_SIMPLE_CLIENT

    /*!
        \brief Handles extended admin single data packets.

        The current implementation only allows REVC_BLOCK_LOW requests to
        receive the stream key update.

        \param[in] SRC The sender of the admin packet.
        \param[in] DATA The admin packet.
        \param[in] DATA_LEN The length of the admin packet.

        \return ONS_SUCCESS If the packet was successfully parsed.
                ONS_BAD_PARAM If the parameters are invalid.
                ONS_RSRC_FULL If the resources to send an appropriate response
                  are full.
                ONS_UNHANDLED_PKT If the admin message type is not handled by
                  the device
                ONS_INVALID_DATA If the received data is not valid.
                ONS_INCORRECT_ADDR If a packet was received from an unexpected
                  sender.
                ONS_INTERNAL_ERR If this is returned, something unexpected
                  happened.
    */
    static one_net_status_t handle_extended_single_admin_pkt(
      const on_encoded_did_t * const SRC, const UInt8 * const DATA,
      const UInt8 DATA_LEN)
    {
        one_net_status_t status = ONS_INTERNAL_ERR;

        if(!SRC || !DATA || DATA_LEN != ONE_NET_RAW_SINGLE_DATA_LEN)
        {
            return ONS_BAD_PARAM;
        } // if the parameters are invalid //

        #ifdef _ONE_NET_EVAL
            oncli_print_admin_msg(ON_EXTENDED_ADMIN_MSG, ON_SINGLE,
              DATA[ON_ADMIN_MSG_ID_IDX], &(DATA[ON_ADMIN_DATA_IDX]),
              ON_MAX_ADMIN_PLD_LEN);
        #endif // ifdef ONE_NET_EVAL //

        if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP
          && DATA[ON_ADMIN_MSG_ID_IDX] != ON_END_STREAM)
        {
            return ONS_BAD_PKT_TYPE;
        } // if waiting for a stream data response //

        switch(DATA[ON_ADMIN_MSG_ID_IDX])
        {
            case ON_RECV_BLOCK_LOW:
            {
                UInt16 cmd_and_data_type;

                if(on_block_txn.txn.priority != ONE_NET_NO_PRIORITY)
                {
                    status = ONS_RSRC_FULL;
                    break;
                } // if the resource is in use //

                on_block_txn.txn.remaining
                  = one_net_byte_stream_to_int16(&(DATA[ON_BLOCK_LEN_IDX
                  + ON_ADMIN_DATA_IDX]));

                if(one_net_byte_stream_to_int16(
                  &(DATA[ON_BLOCK_STREAM_DATA_TYPE_IDX + ON_ADMIN_DATA_IDX]))
                  != ON_CHANGE_STREAM_KEY || on_block_txn.txn.remaining
                  != sizeof(on_base_param->stream_key))
                {
                    status = ONS_INVALID_DATA;
                    break;
                } // if not the stream key update //

                on_block_txn.requesting = FALSE;
                on_block_txn.txn.send = FALSE;

                one_net_memmove(
                  &(on_block_txn.txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  *SRC, ON_ENCODED_DID_LEN);
                on_block_txn.txn.priority = ONE_NET_LOW_PRIORITY;
                ont_set_timer(on_block_txn.txn.next_txn_timer,
                  on_base_param->fragment_delay_low);
                on_block_txn.txn.retry = 0;
                on_block_txn.txn.msg_type = ON_EXTENDED_ADMIN_MSG;

                status = ONS_SUCCESS;
                break;
            } // receive low priority block case //

            default:
            {
                status = ONS_BAD_PKT_TYPE;
                break;
            } // default case //
        } // switch(DATA[ON_ADMIN_MSG_ID_IDX]) //

        return status;
    } // handle_extended_single_admin_pkt //

#endif // infdef _ONE_NET_SIMPLE_CLIENT //


/*!
    \brief Looks for the Invite packet when the device is being added to the
      network.

    Scans the channels (in the shared multitasking environment) for the MASTER
    Invite New CLIENT packet.  This packet is read, and parsed here.

    \param void

    \return TRUE If the invite was found
            FALSE If the invite was not found
*/
static BOOL look_for_invite()
{
    on_encoded_did_t rx_dst, rx_src;
    on_encoded_nid_t nid;

    UInt8 pid;

	
#ifdef _ENHANCED_INVITE
    if(ont_expired(ONT_INVITE_TIMER))
	{
        client_invite_timed_out = TRUE;
	    client_looking_for_invite = FALSE;
		on_state = ON_IDLE;
        one_net_client_invite_cancelled(CANCEL_INVITE_TIMEOUT);
	    return FALSE;
	}
#endif

    // get the start of frame and read in the destination DID, the NID, source
    // DID, and the PID, the rx_dst is the broadcast address, and the pid
    // is the MASTER Invite New CLIENT packet.
    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) == ONS_SUCCESS
      && one_net_read(rx_dst, sizeof(rx_dst)) == sizeof(rx_dst)
      && on_encoded_did_equal((const on_encoded_did_t * const)&rx_dst,
      &ON_ENCODED_BROADCAST_DID) && one_net_read(nid, sizeof(nid))
      == sizeof(nid) && one_net_read(rx_src, sizeof(rx_src)) == sizeof(rx_src)
      && one_net_read(&pid, sizeof(pid)) == sizeof(pid)
    #ifdef _ONE_NET_MULTI_HOP
          && (pid == ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT
          || pid == ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT))
    #else // ifdef _ONE_NET_MULTI_HOP //
          && pid == ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        UInt8 encoded_invite[ON_ENCODED_INVITE_SIZE];
        UInt8 raw_invite[ON_RAW_INVITE_SIZE];

        // read in, decode, and decrypt the invite field.  Verify the crc,
        // and version.
        if(one_net_read(encoded_invite, sizeof(encoded_invite))
          == ON_ENCODED_INVITE_SIZE && on_decode(raw_invite, encoded_invite,
          ON_ENCODED_INVITE_SIZE) == ONS_SUCCESS && on_decrypt(ON_INVITE,
            raw_invite,
          (const one_net_xtea_key_t * const)&(on_base_param->current_key))
          == ONS_SUCCESS && raw_invite[ON_INVITE_CRC_IDX]
          == (UInt8)one_net_compute_crc(raw_invite, ON_INVITE_DATA_LEN,
          ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER)
          && raw_invite[ON_INVITE_VERSION_IDX] == ON_INVITE_PKT_VERSION)
        {
            #ifdef _ONE_NET_MULTI_HOP
                if(pid == ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT)
                {
                    master->device.max_hops = on_hops_taken();

                    if(master->device.max_hops == ON_INVALID_HOPS)
                    {
                        master->device.max_hops = 0;
                        return FALSE;
                    } // if problem reading or processing the hops field //
                } // if a Multi-Hop invite was received //
            #endif // ifdef _ONE_NET_MULTI_HOP //

            // store the address for the device
            one_net_memmove(on_base_param->sid, nid, sizeof(nid));

            if(on_encode(&(on_base_param->sid[ON_ENCODED_NID_LEN]),
              &(raw_invite[ON_INVITE_ASSIGNED_DID_IDX]), ON_ENCODED_DID_LEN)
              != ONS_SUCCESS)
            {
                return FALSE;
            } // if encoding the did failed //

            // store the key
            one_net_memmove(on_base_param->current_key,
              &(raw_invite[ON_INVITE_KEY_IDX]),
              sizeof(on_base_param->current_key));

            // store the keep alive interval
            master->keep_alive_interval = one_net_ms_to_tick(
              one_net_byte_stream_to_int32(
              &raw_invite[ON_INVITE_KEEP_ALIVE_IDX]));

            // store the MASTERs DID
            one_net_memmove(master->device.did, rx_src,
              sizeof(master->device.did));

            // don't check the response since device has received the network
            // info, and no longer has the unique key stored.  It will keep
            // trying to send the status response.  Also, make sure the single
            // transaction is clear (the device shouldn't be trying to send
            // a single transaction since it's not part of the network).
            on_single_txn.txn.priority = ONE_NET_NO_PRIORITY;
            send_status_resp(
              (const on_encoded_did_t * const)&(master->device.did));
            return TRUE;
        } // if reading and decryption the invite field is successful //
    } // if SOF was received //

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
    \brief Saves the parameters that need it to non-volatile storage.

    \param void

    \return void
*/
static void save_param(void)
{
    const int DID_OFFSET = ON_ENCODED_NID_LEN;
	on_encoded_did_t* cli_did_ptr;
	
	cli_did_ptr = (on_encoded_did_t*) (&(on_base_param->sid[DID_OFFSET]));

	client_joined_network = FALSE;
	if(ONS_SUCCESS == on_decode(client_did, *cli_did_ptr, ON_ENCODED_DID_LEN))
	{
        if(!on_encoded_did_equal(cli_did_ptr, &ON_ENCODED_BROADCAST_DID))
		{
		    client_joined_network = TRUE;
		}
	}
	
	if(!client_joined_network)
	{
        one_net_memmove(*cli_did_ptr, ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
		client_did[0] = 0;
		client_did[1] = 0;
	}		
		
    on_base_param->crc = one_net_compute_crc((UInt8 *)on_base_param
      + sizeof(on_base_param->crc), sizeof(nv_param)
      - sizeof(on_base_param->crc),
      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
    one_net_client_save_settings(nv_param, sizeof(nv_param));
} // save_param //


#ifdef _ONE_NET_EVAL
    /*!
        \brief Forces the CLIENT to save it's parameters.

        This is a "protected" function used only by the eval project to force
        the CLLIENT to save it's parameters.  It is not even declared in the
        ONE-NET files.

        \param void

        \return void
    */
    void on_client_force_save(void)
    {
        save_param();
    } // on_client_force_save //
#endif // ifdef _ONE_NET_EVAL //

//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT
