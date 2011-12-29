//! \addtogroup ONE-NET_MASTER ONE-NET MASTER device functionality
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
    \file one_net_master.c
    \brief ONE-NET MASTER functionality implementation

    Derives from ONE-NET.  MASTER dependent functionality.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"

#include "one_net_master.h"

#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_master_port_const.h"
#include "one_net_master_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"

#include "one_net_peer.h"


#ifdef _ONE_NET_EVAL
    #include "one_net_eval.h"
    #include "oncli.h"
#endif // ifdef ONE_NET_EVAL


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{

enum
{
    //! Value to increment the next CLIENT did by each time a device is added
    //! to the network
    ON_CLIENT_DID_INCREMENT = 0x0010,

    //! Number of bits to shift the initial CLIENT address to use as a 16-bit
    //! or raw address value
    ON_INIT_CLIENT_SHIFT = 4,
};


enum
{
    //! The number of invites sent before a Multi-hop invite is sent
    ON_INVITES_BEFORE_MULTI_HOP = 5
};


/*!
    \brief Indexes for queued transactions.

    These indexes are used to store the raw information for a queued transaction
    (type, destination, payload), in the pkt field of the txn_t.  When the
    packet is about to be sent, the raw data will be copied to local variables,
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
    ON_DATA_IDX
};

//! @} ONE-NET_MASTER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MASTER_typedefs
//! \ingroup ONE-NET_MASTER
//! @{

//! Represents an element in the transaction list.
typedef struct
{
    //! Type of txn.  See on_data_t in one_net.h
    UInt8 type;

    //! The did of the other device involved in the transaction.
    on_encoded_did_t did;

    //! Pointer to txn (if sending)
    on_txn_t txn;

    //! Index that contains which packet is being pointed to by the pkt pointer
    //! in txn.
    UInt8 pkt_idx;

    //! Pointer to next queued transaction
    UInt8 next;
} txn_info_t;


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Block/stream request info.

    This information needs to be stored when the single data admin packet for a
    block or stream request is made.
*/
typedef struct
{
    //! type of request being made
    UInt8 type;

    //! TRUE if this device is the sending the data for the transaction.
    BOOL send;

    //! The total number of bytes to be transfered (if a block transaction).
    UInt16 len;
} block_stream_req_t;
#endif


//! Location to store packet data
typedef struct
{
    //! flag to indicate if the packet is being used
    BOOL in_use;

    //! The memory to store packets to send
    UInt8 pkt_data[ONE_NET_MAX_ENCODED_PKT_LEN];
} pkt_mgr_t;



//! @} ONE-NET_MASTER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_MASTER_pri_var
//! \ingroup ONE-NET_MASTER
//! @{

//------------------------------------------------------------------------------
// If any txn_t variables are added or removed, may need to update
// ON_MASTER_TXN_COUNT
//------------------------------------------------------------------------------

//! "Protected" state variable inherited from one_net.c
extern on_state_t on_state;

//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
UInt8 nv_param[sizeof(on_base_param_t) + sizeof(on_master_param_t)
  + ONE_NET_MASTER_MAX_CLIENTS * sizeof(on_client_t)];

const int MASTER_NV_PARAM_SIZE_BYTES = sizeof(nv_param);

on_master_param_t* master_param
  = (on_master_param_t *)(nv_param + sizeof(on_base_param_t));

//! List of the CLIENTS
on_client_t * client_list = (on_client_t *)(nv_param
  + sizeof(on_base_param_t) + sizeof(on_master_param_t));

//! "Protected" base parameters variable inherited from one_net.c
extern on_base_param_t * on_base_param;


#ifdef _NONCES_MATTER
extern BOOL nonces_matter;
#endif


//! Locations to be used to store packets that are queued to be sent.
static pkt_mgr_t pkt_list[ONE_NET_MASTER_MAX_SEND_TXN] =
{
    {FALSE, {0}}, {FALSE, {0}}, {FALSE, {0}},
    {FALSE, {0}}, {FALSE, {0}}, {FALSE, {0}}
};

//! Total number of elements from txn_data_list being used.
static UInt8 txn_data_count = 0;

//! The number of elements from txn_data_list used for single transactions.
static UInt8 single_txn_data_count = 0;

//! The available resources for single transactions.  Also acts as the queue
//! for single transactions.
static txn_info_t txn_list[ONE_NET_MASTER_MAX_TXN] =
{
#if defined(_BLOCK_MESSAGES_ENABLED)
    {ON_NO_TXN, {0xB4, 0xB4}, {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
      FALSE, 0, 0, FALSE, 0, 0, 0}, ONE_NET_MASTER_MAX_SEND_TXN,
      ONE_NET_MASTER_MAX_TXN}
#elif defined(_ONE_NET_MASTER)
    {ON_NO_TXN, {0xB4, 0xB4}, {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
      0, 0, FALSE, 0, 0, 0}, ONE_NET_MASTER_MAX_SEND_TXN,
      ONE_NET_MASTER_MAX_TXN}
#else
    {ON_NO_TXN, {0xB4, 0xB4}, {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
      0, 0, FALSE, 0, 0, 0}, ONE_NET_MASTER_MAX_SEND_TXN,
      ONE_NET_MASTER_MAX_TXN}
#endif
};


//! The head of the queued transactions
static UInt8 txn_head = ONE_NET_MASTER_MAX_TXN;

//! The current transaction being carried out.
static UInt8 cur_txn_info = ONE_NET_MASTER_MAX_TXN;

//! the index into txn_list of the transaction used to send the invite message
static UInt8 invite_idx = ONE_NET_MASTER_MAX_TXN;

/*!
    \brief Location used to send response packets

    This will also be used to send a single data stream end packet.
*/
UInt8 on_master_response_pkt[ONE_NET_MAX_ENCODED_PKT_LEN];

//! Transaction used to send a response
// if this is removed, need to update ON_MASTER_TXN_COUNT
#if defined(_BLOCK_MESSAGES_ENABLED)
static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
  FALSE, ON_MASTER_RESPONSE_TXN_TIMER_OFFSEST, 0, 0, // no hops field
  sizeof(on_master_response_pkt), on_master_response_pkt};
#elif defined(_ONE_NET_MASTER)
static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
  FALSE, ON_MASTER_RESPONSE_TXN_TIMER_OFFSEST, 0, // no hops field
  sizeof(on_master_response_pkt), on_master_response_pkt};
#else
static on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
  FALSE, ON_MASTER_RESPONSE_TXN_TIMER_OFFSEST, 0, // no hops field
  sizeof(on_master_response_pkt), on_master_response_pkt};
#endif

#ifdef _BLOCK_MESSAGES_ENABLED
//! Information stored when a block/stream request admin packet is built and
//! sent.
static block_stream_req_t b_s_req = {ON_NO_TXN, FALSE, 0};
#endif

/*!
    The length of time in ticks that the channel must be clear for before
    determining that the channel is ok for the network to operate on.
*/
static tick_t new_channel_clear_time_out = 0;

//! Unique key of the device being invited into the network
static one_net_xtea_key_t invite_key;

//! Index used to try and update keys without waiting for a keep alive from
//! a CLIENT.
static UInt16 key_change_idx = 0;

#ifdef _STREAM_MESSAGES_ENABLED
//! Index used to try and update the stream key without waiting for a keep alive
//! from a CLIENT.
static UInt16 stream_key_change_idx = 0;
#endif

//! Keeps track of the data rate test being carried out
static struct
{
    //! Indexes to single transactions being used to set up the data rate test.
    UInt8 recv_idx, send_idx;

    //! DIDs of the two devices involved in the data rate test.
    on_encoded_did_t recv_did, send_did;

    //! Indicates the timeout period in which the a response should be heard in.
    UInt8 timer;

    //! The data rate being tested
    UInt8 rate;

    //! Flag to indicate if it a Multi-Hop data rate test was performed.
    BOOL multi_hop;
} data_rate_test;

//! flag that indicates if any Multi-Hop Repeaters have joined the network
static BOOL mh_repeater_available = FALSE;

//! flag to indicate if the MASTER should save settings
static BOOL save = FALSE;


// June 18, 2011 - making this a PUBLIC variable so it can be changed
// by application code.
// TODO - stick it in the PUBLIC varaibles section.  Should still be
// accessible as an extern though since it doesn't have a "static" modifier
//! flag to indicate how long to try inviting a client before giving up.
tick_t one_net_master_invite_duration = ONE_NET_MASTER_INVITE_DURATION;


//! @} ONE-NET_MASTER_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
//! @{

static void init_internal(void);

one_net_status_t on_master_single_data_hdlr(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, UInt8 * const pld, on_txn_t ** txn);

one_net_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  const UInt8 NEXT_NONCE, const one_net_status_t STATUS);

#ifdef _BLOCK_MESSAGES_ENABLED
static one_net_status_t b_s_request(const UInt8 TXN_TYPE, const UInt8 MSG_TYPE,
  const BOOL SEND, const UInt8 DATA_TYPE, const UInt16 LEN,
  const UInt8 PRIORITY, const on_encoded_did_t * const DID,
  const UInt8 SRC_UNIT, const UInt8 DST_UNIT);

one_net_status_t on_master_b_s_data_hdlr(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, UInt8 * const pld, on_txn_t ** txn);

one_net_status_t on_master_b_s_txn_hdlr(on_txn_t ** txn, const UInt8 NEXT_NONCE,
  const one_net_status_t STATUS, const UInt8 HOPS_TAKEN);
#endif

void on_master_data_rate_hdlr(const UInt8 RATE,
  const on_encoded_did_t * const DID, const UInt8 RESULT);

static one_net_status_t setup_data_rate_test(
  const on_encoded_did_t * const SENDER,
  const on_encoded_did_t * const RECEIVER, const UInt8 DATA_RATE,
  const BOOL MH);

static BOOL retry_data_rate_test(const UInt8 RATE);

static void report_data_rate_result(const UInt8 DATA_RATE, const UInt8 RESULT,
  const UInt8 ATTEMPTS);

static one_net_status_t handle_admin_pkt(const on_encoded_did_t * const SRC_DID,
  const UInt8 * const DATA, const UInt8 DATA_LEN, on_client_t ** client,
  on_nack_rsn_t* const nack_reason, on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload);

static one_net_status_t admin_txn_hdlr(const UInt8 ADMIN_ID,
  const UInt8 * const DATA, on_txn_t ** txn, const one_net_status_t STATUS,
  on_client_t * const client, const one_net_raw_did_t * const DID);
#ifdef _STREAM_MESSAGES_ENABLED
static one_net_status_t extended_admin_single_txn_hdlr(const UInt8 ADMIN_ID,
  const UInt8 * const DATA, on_txn_t ** txn, const one_net_status_t STATUS,
  on_client_t * const client, const one_net_raw_did_t * const DID);
#endif

static one_net_status_t send_change_settings(const on_encoded_did_t * const DST,
  const UInt8 CLIENT_DATA_RATE, const UInt8 FLAGS);

static void check_key_update(const BOOL ONLY_IF_ACTIVE);
static void key_update_confirmed(on_client_t * const client);
static one_net_status_t send_key_update(const on_encoded_did_t * const DST);
#ifdef _STREAM_MESSAGES_ENABLED
static void check_stream_key_update(const BOOL ONLY_IF_ACTIVE);
static void stream_key_update_confirmed(on_client_t * const client);
static one_net_status_t send_stream_key_update(
  const on_encoded_did_t * const DST);
#endif

static one_net_status_t send_admin_pkt(const UInt8 ADMIN_MSG_ID,
  const UInt8 MSG_TYPE, const on_encoded_did_t * const DST,
  const UInt8 PRIORITY, const UInt8 * const PLD, const UInt8 PLD_LEN);

//#ifdef _BLOCK_MESSAGES_ENABLED
static BOOL get_next_b_s_pld(const UInt8 TXN_INFO);
static on_nack_rsn_t get_b_s_app_data(const UInt8 TXN_INFO);
#ifdef _STREAM_MESSAGES_ENABLED
static BOOL get_b_s_extended_admin_data(const UInt8 TXN_INFO);
#endif

static void check_b_s_req(void);

static BOOL b_s_in_progress(const UInt8 TYPE,
  const on_encoded_did_t * const DID);
//#endif

static one_net_status_t build_txn_data_pkt(const UInt8 TXN);

static one_net_status_t rm_client(const on_encoded_did_t * const CLIENT_DID);
static on_client_t* one_net_master_add_new_client_to_list(void);
static int adjust_client_list(void);

static UInt8 get_free_txn(const UInt8 TYPE, const BOOL SEND);

static void free_txn(const UInt8 TXN);

static BOOL enqueue_txn(const UInt8 TXN);

static void dequeue_txn(const UInt8 TXN);

#ifdef _BLOCK_MESSAGES_ENABLED
static UInt8 dequeue_b_s_txn(const UInt8 TYPE,
  const on_encoded_did_t * const DID);
#endif

static BOOL next_txn_queued_for_client(const on_encoded_did_t * const DID);

static void data_rate_txn(UInt8 * const txn_idx);

static UInt8 txn_nonce_for_client(const on_encoded_did_t * const DID);

static void save_param(void);

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
  const one_net_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
  const one_net_xtea_key_t * const STREAM_KEY,
  const UInt8 STREAM_ENCRYPT_METHOD)
#else
one_net_status_t one_net_master_create_network(
  const one_net_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
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
    #if defined(_ONE_NET_EVAL) && defined(_US_CHANNELS)
        // the eval board contains both US and European channels, but the MASTER
        // defaults to creating the network on the US channel.
        on_base_param->channel = one_net_prand(one_net_tick(),
          ONE_NET_MAX_US_CHANNEL);
    #else // pick any U.S. Channel //
        on_base_param->channel = one_net_prand(one_net_tick(),
          ONE_NET_MAX_CHANNEL);
    #endif // else pick any channel //
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
    ont_set_timer(ONT_GENERAL_TIMER, new_channel_clear_time_out);
    ont_set_timer(ONT_CHANGE_KEY_TIMER, ONE_NET_MASTER_CHANNEL_SCAN_TIME);
    on_state = ON_JOIN_NETWORK;

    return ONS_SUCCESS;
} // one_net_master_create_network //


/*!
    \brief MASTER initializer

    This needs to be called before the MASTER is run.  Due to memory constraints
    of embedded systems, this function can be repeatedly called with only a
    subset of the parameters.  The calls must preserve the byte order of the
    parameters.

    \param[in] PARAM The parameters (or part) that were saved.
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

    // The number of bytes in the non-volatile parameter buffer that have been
    // initialized so far.
    static UInt16 param_size_initialized = 0;

    // The number of bytes expected to initialize the parameters
    UInt16 param_size_expected;
    one_net_send_single = &one_net_master_send_single;

    if(!PARAM || !PARAM_LEN)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    if(sizeof(nv_param) < param_size_initialized + PARAM_LEN)
    {
        param_size_initialized = 0;
        return ONS_INVALID_DATA;
    } // if more data passed in than expected //

    one_net_memmove(&(nv_param[param_size_initialized]), PARAM, PARAM_LEN);
    param_size_initialized += PARAM_LEN;
    param_size_expected = sizeof(on_base_param_t) + sizeof(on_master_param_t)
      + master_param->client_count * sizeof(on_client_t);

    // Need the base_parameters and master parameters to know how many CLIENTS
    // are in the network (and thus the total size to be initialized)
    if(param_size_initialized < sizeof(on_base_param_t)
      + sizeof(on_master_param_t)
      || param_size_initialized < param_size_expected)
    {
        return ONS_MORE;
    } // if not done initializing the parameters //
    else if(param_size_initialized > param_size_expected)
    {
        param_size_initialized = 0;
        return ONS_INVALID_DATA;
    } // if too much data was given //

#ifdef _REJECT_BAD_VERSION
    if(on_base_param->version != ON_PARAM_VERSION)
    {
        param_size_initialized = 0;
        return ONS_INVALID_DATA;
    } // if the parameter version does not match //
#endif

#ifdef _ONE_NET_MASTER_NV_DATA_CRC_CHECK
      if(on_base_param->crc != one_net_compute_crc((UInt8 *)on_base_param
      + sizeof(on_base_param->crc), param_size_initialized
      - sizeof(on_base_param->crc), ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER))
    {
        param_size_initialized = 0;
        return ONS_INVALID_DATA;
    } // if the parameter version does not match or crc's don't match //
#endif

    for(i = 0; i < master_param->client_count; i++)
    {
        if((client_list[i].features & ON_MH_CAPABLE) &&
          (client_list[i].features & ON_MH_REPEATER))
        {
            mh_repeater_available = TRUE;
            break;
        } // if a mh repeater capable CLIENT was found //
    } // loop to look for any Multi-Hop repeaters //

    init_internal();

    on_state = ON_LISTEN_FOR_DATA;

    // check to see if in the middle of changing the key
    if(master_param->client_count)
    {
        check_key_update(FALSE);
		#ifdef _STREAM_MESSAGES_ENABLED
        check_stream_key_update(FALSE);
		#endif
    } // if there are devices in the network //

    param_size_initialized = 0;

	// Derek_S 10/25/2010 added call to save_param() below.  "list" command
	// does not work without it after a "save" and power cycle.
	// TO-DO : Is this the proper place for this call to save_param() and is
	// the if condition below necessary (i.e. is there a downside/upside to calling
	// save_param() when there are no clients?)
	if(master_param->client_count > 0)
	{
		save_param(); // This function changes a NULL pointer to a real pointer.
		              // "list" needs the real pointer to be accurate.
	}

    return ONS_SUCCESS;
} // one_net_master_init //


/*!
    \brief Returns the channel the MASTER is on.

    If the MASTER has not yet picked a channel, ONE_NET_NUM_CHANNELS is
    returned.

    \param void

    \return The channel the MASTER is on or ONE_NET_NUM_CHANNELS
*/
UInt8 one_net_master_get_channel(void)
{
    if(on_state == ON_JOIN_NETWORK || on_state == ON_INIT_STATE)
    {
        return ONE_NET_NUM_CHANNELS;
    } // if the channel has not yet been picked //

    return on_base_param->channel;
} // one_net_master_get_channel //


/*!
    \brief Sends a single data packet.

    Queues the single transaction based on the priority of the transaction.
    The transaction is stored in it's raw form using the location that will
    later contain the encoded packet.  The first byte will contain whether
    it is a single, block, or stream transaction, the next 2 bytes will contain
    the raw destination address, while the remaining bytes will be left to
    contain the raw data (not all bytes will be used as the raw size is less
    than the encoded size).

    This function may not use all the parameters.  The parameter list is
    identical to the client send single function so that the application
    layer can use function pointers.

    \param[in] data The data to send.
    \param[in] DATA_LEN The length of data (in bytes).
    \param[in] send_to_peer_list True if this message should be sent through the
               peer list
    \param[in] PRIORITY The priority on the transaction.
    \param[in] RAW_DST 0 If this device is initiating a transaction.
      destination did if the device is sending this transaction in response to a
      packet received from this device.
    \param[in] SRC_UNIT The unit sending the message.
	\param[in] send_time_from_now Time to pause before sending.  NULL is interpreted as "send immediately"
	\param[in] expire_time_from_now If after this time, don't bother sending.  NULL is interpreted as "no expiration"

    \return ONS_SUCCESS If the single data has been queued successfully.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_RSRC_FULL If no resources are currently available to handle the
              request.
            ONS_BAD_ADDR If RAW_DST is not an address that is part of this
              network.
*/
one_net_status_t one_net_master_send_single(UInt8 * data,
  UInt8 DATA_LEN, const BOOL send_to_peer_list, UInt8 PRIORITY,
  const one_net_raw_did_t *RAW_DST, UInt8 SRC_UNIT,
  tick_t* send_time_from_now, tick_t* expire_time_from_now)
{
    one_net_status_t status;
    UInt8 txn;

    if(!data || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN
      || PRIORITY < ONE_NET_LOW_PRIORITY || PRIORITY > ONE_NET_HIGH_PRIORITY
      #ifdef _PEER
      || (!send_to_peer_list && !RAW_DST))
      #else
      || !RAW_DST)
      #endif
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

	if(RAW_DST && (*RAW_DST)[0] == 0x00 && (*RAW_DST)[1] == 0x10)
	{
        // TODO - for the moment, reject any message to myself.  But this might
        // be considered a legitimate message too, so we might wantt o allow it.
        // Consider calling the single data handler directly. Right now we allow
        // peer assignments from one unit to another unit on the same device.
		return ONS_DID_FAILED;
	}

    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
	if(send_time_from_now != NULL && expire_time_from_now != NULL)
	{
		return place_in_single_queue(data, DATA_LEN, send_to_peer_list,
		    PRIORITY, RAW_DST, SRC_UNIT, send_time_from_now, expire_time_from_now);
	}
    #endif

    #ifdef _PEER
    if(RAW_DST == NULL || send_to_peer_list)
    {
        return master_send_to_peer_list(data, DATA_LEN, PRIORITY, SRC_UNIT);
    }
    #endif

    if((txn = get_free_txn(ON_SINGLE, TRUE)) >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_RSRC_FULL;
    } // if there are no available resources //

    if((status = on_encode(txn_list[txn].did, *RAW_DST, ON_ENCODED_DID_LEN))
      == ONS_SUCCESS)
    {
        // make sure the address is valid //
        if(client_info((const on_encoded_did_t * const)&(txn_list[txn].did))
          == 0)
        {
            status = ONS_BAD_ADDR;
        } // the client is not part of the network //
        else
        {
            txn_list[txn].type = ON_SINGLE;
            txn_list[txn].txn.data_len = DATA_LEN;
            txn_list[txn].txn.msg_type = ON_APP_MSG;

            one_net_memmove(&(txn_list[txn].txn.pkt[ON_DATA_IDX]), data,
              DATA_LEN);
            txn_list[txn].txn.priority = PRIORITY;
            enqueue_txn(txn);
        } // else successfully found the CLIENT //
    } // if encoding the packet was successful //

    if(status != ONS_SUCCESS)
    {
        free_txn(txn);
    } // else encoding failed //

    return status;
} // one_net_master_send_single //


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Interface to start a block transaction.

    Starts a block transaction (send or receive) if the resources are available.

    \param[in] TYPE The type of transaction to be requested.  This can be either
                    ON_BLOCK or ON_STREAM.
    \param[in] SEND TRUE if this device is sending the data.
                    FALSE if this device is requesting to receive the data.
    \param[in] DATA_TYPE The type of data to transfer.
    \param[in] LEN The total number of bytes to transfer (if a block txn).
    \param[in] DID The device id of the other device involved in the transaction
    \param[in] SRC_UNIT The unit sending the message.
    \param[in] DST_UNIT The unit receiving the message.

    \return ONS_SUCCESS If there are resources available and the request will be
                    made.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_RSRC_FULL If the resources are unavailable.
            ONS_BAD_ADDR If DID does not point to an address of a device in the
                    network.
            ONS_ALREADY_IN_PROGRESS If a transaction of this type with the
                    device is already in progress.
*/
one_net_status_t one_net_master_block_stream_request(UInt8 TYPE, BOOL SEND,
  UInt8 DATA_TYPE, UInt16 LEN, UInt8 PRIORITY, const one_net_raw_did_t * DID,
  UInt8 SRC_UNIT, UInt8 DST_UNIT)
{
    one_net_status_t status;
    on_encoded_did_t dst;

    if(!DID)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if((status = on_encode(dst, *DID, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the destination (of the admin packet) was not successful //

    // make sure the address is valid //
    if(client_info((const on_encoded_did_t * const)&dst) == 0)
    {
        return ONS_BAD_ADDR;
    } // the client is not part of the network //

    return b_s_request(TYPE, ON_ADMIN_MSG, SEND, DATA_TYPE, LEN, PRIORITY,
      (const on_encoded_did_t * const)&dst, SRC_UNIT, DST_UNIT);
} // one_net_master_block_stream_request //
#endif


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Ends a stream transaction with a given device

    \param[in] DID The device to end the stream with.

    \return ONS_SUCCESS if the end stream is scheduled
            ONS_BAD_PARAM if DID is invalid.
            ONS_TXN_DOES_NOT_EXIST if the transaction does not exist.
*/
one_net_status_t one_net_master_end_stream(const one_net_raw_did_t * const DID)
{
    one_net_status_t rv;

    on_encoded_did_t encoded_did;
    UInt8 stream;

    if(!DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if((rv = on_encode(encoded_did, (*DID), sizeof(encoded_did)))
      != ONS_SUCCESS)
    {
        return rv;
    } // if encoding the did was not successful //

    // don't go through the active list since the stream may be active and
    // will not be in the list
    for(stream = 0; stream < ONE_NET_MASTER_MAX_TXN; stream++)
    {
        if(txn_list[stream].type == ON_STREAM && on_encoded_did_equal(
          (const on_encoded_did_t * const)&encoded_did,
          (const on_encoded_did_t * const)&(txn_list[stream].did)))
        {
            break;
        } // if the stream address matches //
    } // loop to find the stream //

    if(stream >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_TXN_DOES_NOT_EXIST;
    } // if the transaction does not exist //

    // indicate that the stream is to end by setting remaining to 0.
    txn_list[stream].txn.remaining = 0;
    return rv;
} // one_net_master_end_stream //
#endif


/*!
    \brief Changes the data rate a CLIENT device receives at.

    \param[in] RAW_DID The device whose data rate is to be changed.
    \param[in] DATA_RATE The data rate to change to.

    \return ONS_SUCCESS If the transaction was successfully queued
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            ONS_INVALID_DATA If the CLIENT does not support the given data rate
              See encoded, and send_change_settings for more possible return
              results.
*/
one_net_status_t one_net_master_change_client_data_rate(
  const one_net_raw_did_t * const RAW_DID, const UInt8 DATA_RATE)
{
    const on_client_t * CLIENT = 0;

    on_encoded_did_t dst;
    one_net_status_t status;

    if(!RAW_DID || DATA_RATE > ONE_NET_MAX_DATA_RATE)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if((status = on_encode(dst, *RAW_DID, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if((CLIENT = client_info((const on_encoded_did_t * const)&dst)) == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

    if(DATA_RATE > CLIENT->max_data_rate)
    {
        return ONS_INVALID_DATA;
    } // if the data rate is higher than the CLIENT supports //

    return send_change_settings((const on_encoded_did_t * const)&dst, DATA_RATE,
      CLIENT->flags);
} // one_net_master_change_client_data_rate //


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
  const one_net_raw_did_t * const RAW_DST, const UInt32 KEEP_ALIVE)
{
    on_encoded_did_t dst;
    one_net_status_t status;

    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    if(!RAW_DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = on_encode(dst, *RAW_DST, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(!client_info((const on_encoded_did_t * const)&dst))
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

    one_net_int32_to_byte_stream(KEEP_ALIVE, pld);

    return send_admin_pkt(ON_CHANGE_KEEP_ALIVE, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)&dst, ONE_NET_HIGH_PRIORITY, pld,
      sizeof(pld));
} // one_net_master_change_client_keep_alive //


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Changes the fragment delay of the MASTER or a CLIENT device depending
      on RAW_DST.

    \param[in] RAW_DST The device to update
    \param[in] PRIORITY The fragment delay priority to update (high or low).
    \param[in] DELAY The new [low/high] fragment delay (in ms)

    \return ONS_SUCCESS if queueing the transaction was successful
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INCORRECT_ADDR If the address is for a device not in the
              network.
            ONS_INVALID_DATA If the data is not valid (such as the high priority
              having a delay longer than the low priority, or vise-versa).
*/
one_net_status_t one_net_master_change_frag_dly(
  const one_net_raw_did_t * const RAW_DST, const UInt8 PRIORITY,
  const UInt32 DELAY)
{
    const UInt8 admin_type = PRIORITY == ONE_NET_HIGH_PRIORITY
      ? ON_CHANGE_HIGH_FRAGMENT_DELAY : ON_CHANGE_LOW_FRAGMENT_DELAY;

    on_encoded_did_t dst;
    one_net_status_t status;

    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    if(!RAW_DST || PRIORITY < ONE_NET_LOW_PRIORITY
      || PRIORITY > ONE_NET_HIGH_PRIORITY)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = on_encode(dst, *RAW_DST, sizeof(dst))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(on_encoded_did_equal((const on_encoded_did_t * const)&dst,
      (const on_encoded_did_t * const)
      &(on_base_param->sid[ON_ENCODED_NID_LEN])))
    {
        // change the MASTER's fragment delay
        if(PRIORITY == ONE_NET_LOW_PRIORITY
          && DELAY > on_base_param->fragment_delay_high)
        {
            on_base_param->fragment_delay_low = DELAY;
            save_param();
            return ONS_SUCCESS;
        } // if setting the low priority fragment delay //
        else if(PRIORITY == ONE_NET_HIGH_PRIORITY
          && DELAY < on_base_param->fragment_delay_low)
        {
            on_base_param->fragment_delay_high = DELAY;
            save_param();
            return ONS_SUCCESS;
        } // else if setting the high priority fragment delay //

        return ONS_INVALID_DATA;
    } // if the MASTER device //

    if(!client_info((const on_encoded_did_t * const)&dst))
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

    one_net_int32_to_byte_stream(DELAY, pld);

    return send_admin_pkt(admin_type, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)&dst, ONE_NET_HIGH_PRIORITY, pld,
      sizeof(pld));
} // one_net_master_change_frag_dly //
#endif


/*!
    \brief Changes the key.

    The key is changed by removing the most significant 32 bits of the key and
    appending the 32 bits passed in.

    \param[in] KEY_FRAGMENT The new key fragment.

    \return ONS_SUCCESS If the key was accepted
            ONS_NOT_INIT If the network has not been initialized.
            ONS_ALREADY_IN_PROGRESS If still changing the key from the last
              time it was updated.
*/
one_net_status_t one_net_master_change_key(
  const one_net_xtea_key_fragment_t KEY_FRAGMENT)
{
    UInt16 i;

    if(on_state == ON_INIT_STATE || on_state == ON_JOIN_NETWORK)
    {
        return ONS_NOT_INIT;
    } // if the network has not been created yet //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(!client_list[i].use_current_key)
        {
            return ONS_ALREADY_IN_PROGRESS;
        } // if still updating the key //
    } // loop to see if still updating keys //

    for(i = 0; i < master_param->client_count; i++)
    {
        client_list[i].use_current_key = FALSE;
    } // loop to clear all the flags //

    one_net_memmove(master_param->old_key, on_base_param->current_key,
      sizeof(master_param->old_key));
    one_net_memmove(on_base_param->current_key,
      &(on_base_param->current_key[sizeof(one_net_xtea_key_fragment_t)]),
      sizeof(on_base_param->current_key) - sizeof(one_net_xtea_key_fragment_t));
    one_net_memmove(
      &(on_base_param->current_key[sizeof(on_base_param->current_key)
      - sizeof(one_net_xtea_key_fragment_t)]), KEY_FRAGMENT,
      sizeof(one_net_xtea_key_fragment_t));

    save_param();

    if(!master_param->client_count)
    {
        one_net_master_update_result(ONE_NET_UPDATE_NETWORK_KEY, 0, TRUE);
    } // if there are no CLIENTs //
    else
    {
        check_key_update(FALSE);
    } // else there are CLIENTS //

    return ONS_SUCCESS;
} // one_net_master_change_key //


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Changes the stream key.

    \param[in] NEW_STREAM_KEY The new stream key.

    \return ONS_SUCCESS If the key was accepted
            ONS_NOT_INIT If the network has not been initialized.
            ONS_ALREADY_IN_PROGRESS If still changing the key from the last
              time it was updated.
*/
one_net_status_t one_net_master_change_stream_key(
  const one_net_xtea_key_t * const NEW_STREAM_KEY)
{
    UInt16 i;

    if(!NEW_STREAM_KEY)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(on_state == ON_INIT_STATE || on_state == ON_JOIN_NETWORK)
    {
        return ONS_NOT_INIT;
    } // if the network has not been created yet //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(!client_list[i].use_current_stream_key)
        {
            return ONS_ALREADY_IN_PROGRESS;
        } // if still updating the key //
    } // loop to see if still updating stream keys //

    for(i = 0; i < master_param->client_count; i++)
    {
        client_list[i].use_current_stream_key = FALSE;
    } // loop to clear all the flags //

    one_net_memmove(master_param->old_stream_key,
      on_base_param->stream_key, sizeof(master_param->old_stream_key));
    one_net_memmove(on_base_param->stream_key, *NEW_STREAM_KEY,
      sizeof(on_base_param->stream_key));

    save_param();
    if(!master_param->client_count)
    {
        one_net_master_update_result(ONE_NET_UPDATE_STREAM_KEY, 0, TRUE);
    } // if there are no CLIENTs //
    else
    {
        check_stream_key_update(FALSE);
    } // else there are CLIENTS //

    return ONS_SUCCESS;
} // one_net_master_change_stream_key //
#endif


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
#ifdef _PEER
one_net_status_t one_net_master_peer_assignment(const BOOL ASSIGN,
  const one_net_raw_did_t * const SRC_DID, const UInt8 SRC_UNIT,
  const one_net_raw_did_t * const PEER_DID, const UInt8 PEER_UNIT)
{
    on_client_t * src_client = 0, * peer_client = 0;
    on_encoded_did_t enc_src_did;
    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];
    UInt8 admin_type;

    BOOL master_is_peer = FALSE;
	BOOL did_is_broadcast;

    if(!SRC_DID || !PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    if(on_encode(enc_src_did, *SRC_DID, sizeof(enc_src_did)) != ONS_SUCCESS
      || on_encode(&(pld[ON_PEER_DID_IDX]), *PEER_DID, ON_ENCODED_DID_LEN)
      != ONS_SUCCESS)
    {
        return ONS_INCORRECT_ADDR;
    } // if the encode failed //

    master_is_peer = on_encoded_did_equal(
      (const on_encoded_did_t * const)&(pld[ON_PEER_DID_IDX]),
      (const on_encoded_did_t * const)
      &(on_base_param->sid[ON_ENCODED_NID_LEN]));

    // if we're unassigning and the encoded peer did is BROADCAST, flag it here.  We
    // don't want a broadcast did rejected because it is now a legitimate message.  It
    // will be checked below.
    did_is_broadcast = (!ASSIGN) && on_encoded_did_equal(
      (const on_encoded_did_t *const) (&(&pld[ON_PEER_DID_IDX])), &ON_ENCODED_BROADCAST_DID);

    // verify peer did
    if((!master_is_peer && (!did_is_broadcast && (peer_client
      = client_info((const on_encoded_did_t * const)&(pld[ON_PEER_DID_IDX])))
        == 0)) || (src_client
      = client_info((const on_encoded_did_t * const)&enc_src_did))
        == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // if the peer or dst is not part of the network //

    if(ASSIGN)
    {
        admin_type = ON_ASSIGN_PEER;
    } // if assigning the peer //
    else
    {
        admin_type = ON_UNASSIGN_PEER;
    } // else unassigning the peer //

    pld[ON_PEER_SRC_UNIT_IDX] = SRC_UNIT;
    pld[ON_PEER_PEER_UNIT_IDX] = PEER_UNIT;

    return send_admin_pkt(admin_type, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)&enc_src_did, ONE_NET_LOW_PRIORITY, pld,
      sizeof(pld));
} // one_net_master_peer_assignment //
#endif


/*!
    \brief Sets the update MASTER flag in the CLIENT.

    \param[in] UPDATE_MASTER TRUE if the device should update the MASTER when an
                               event or status change occurs.
                             FALSE if the MASTER should not be notified when an
                               event or status change occurs when a peer has
                               been defined.
    \param[in] DST_DID The CLIENT to update.

    \return ONS_SUCCESS if the command has successfully been queued.
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INCORRECT_ADDR if the device does not exist.
            See encode and send_change_settings for more return codes.
*/
one_net_status_t one_net_master_set_update_master_flag(const BOOL UPDATE_MASTER,
  const one_net_raw_did_t * const DST_DID)
{
    on_encoded_did_t did;
    one_net_status_t status;
    on_client_t * client;
    UInt8 flags = 0x00;

    if(!DST_DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    if((status = on_encode(did, *DST_DID, sizeof(did))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding was not successful //

    if((client = client_info((const on_encoded_did_t * const)&did)) == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // if getting the client info was not successful //

    if(UPDATE_MASTER)
    {
        flags |= ON_SEND_TO_MASTER;
    } // if the MASTER should be updated //
    else
    {
        flags &= ~ON_SEND_TO_MASTER;
    } // else the MASTER does not want to be updated //

    return send_change_settings((const on_encoded_did_t * const)&did,
      client->data_rate, flags);
} // one_net_master_set_update_master_flag //


#if !defined(_ONE_NET_EVAL) && defined(_PEER)
/*!
    \brief Updates a CLIENT that the data rate for one of it's peer devices
      has changed.

    \param[in] RAW_DST The CLIENT to update the peer data rate setting in.
    \param[in] RAW_PEER The peer device whose data rate has been updated.
    \param[in] DATA_RATE The data rate RAW_PEER receives at.

    \return ONS_SUCCESS if the command has successfully been queued
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INCORRECT_ADDR If RAW_DST or RAW_PEER are not part of the
              network.
            ONS_INVALID_DATA If the peer is not operating at the data rate
              passed in or the destination device
*/
one_net_status_t one_net_master_change_peer_data_rate(
  const one_net_raw_did_t * const RAW_DST,
  const one_net_raw_did_t * const RAW_PEER, const UInt8 DATA_RATE)
{
    const on_client_t * DST_CLIENT = 0, * PEER_CLIENT = 0;
    on_encoded_did_t dst, peer;
    one_net_status_t status;

    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    if(!RAW_DST || !RAW_PEER || DATA_RATE > ONE_NET_MAX_DATA_RATE)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if((status = on_encode(dst, *RAW_DST, sizeof(dst))) != ONS_SUCCESS
      || (status = on_encode(peer, *RAW_PEER, sizeof(peer))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if((DST_CLIENT = client_info((const on_encoded_did_t * const)&dst)) == 0
      || (PEER_CLIENT = client_info((const on_encoded_did_t * const)&peer))
      == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

    if(DATA_RATE != PEER_CLIENT->data_rate
      || DATA_RATE > DST_CLIENT->max_data_rate)
    {
        return ONS_INVALID_DATA;
    } // if the data rate is higher than the CLIENT supports //

    one_net_memmove(&(pld[ON_PEER_DID_IDX]), peer, sizeof(peer));
    pld[ON_PEER_SETTING_DATA_RATE_IDX] = DATA_RATE;

    return send_admin_pkt(ON_CHANGE_PEER_DATA_RATE, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)dst, ONE_NET_HIGH_PRIORITY, pld,
      sizeof(pld));
} // one_net_master_change_peer_data_rate //
#endif


/*!
    \brief Starts the data rate test between 2 devices.

    \param[in] SENDER The device that is to send the data rate test.  Pass in 0
      if the MASTER is to send the data rate test.
    \param[in] RECEIVER The device receiving the data rate test.  This must be
      a CLIENT device.
    \param[in] DATA_RATE The data rate to perform the test at.

    \return ONS_SUCCESS If the data rate test has been set up.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            ONS_INVALID_DATA If the data rate is beyond the max data rate that
              either device supports
            See encode, send_admin_pkt for more return results.
*/
one_net_status_t one_net_master_start_data_rate_test(
  const one_net_raw_did_t * const SENDER,
  const one_net_raw_did_t * const RECEIVER, const UInt8 DATA_RATE)
{
    const on_client_t * SENDING_DEVICE = 0;
    const on_client_t * RECEIVING_DEVICE = 0;

    on_encoded_did_t sender, receiver;
    one_net_status_t status;

    BOOL mh = FALSE;

    if(!RECEIVER)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(SENDER)
    {
        if((status = on_encode(sender, *SENDER, sizeof(sender))) != ONS_SUCCESS)
        {
            return status;
        } // if encodeding the sender failed //
    } // if sender is specified //
    else
    {
        one_net_memmove(sender, &(on_base_param->sid[ON_ENCODED_NID_LEN]),
          sizeof(sender));
    } // else the MASTER is going to be the sender //

    if((status = on_encode(receiver, *RECEIVER, sizeof(receiver)))
      != ONS_SUCCESS)
    {
        return status;
    } // if encodeding failed //

    if(on_encoded_did_equal((const on_encoded_did_t * const)&sender,
      (const on_encoded_did_t * const)&receiver))
    {
        return ONS_INCORRECT_ADDR;
    }  // if addrs are equal //

    if(!(RECEIVING_DEVICE = client_info(
      (const on_encoded_did_t * const)&receiver))
      || (!on_encoded_did_equal((const on_encoded_did_t * const)&sender,
      (const on_encoded_did_t * const)&(on_base_param->sid[ON_ENCODED_NID_LEN]))
      && !(SENDING_DEVICE
      = client_info((const on_encoded_did_t * const)&sender))))
    {
        return ONS_INCORRECT_ADDR;
    } // if one of the devices does not exist //

    if(DATA_RATE > RECEIVING_DEVICE->max_data_rate
      || ((SENDING_DEVICE && DATA_RATE > SENDING_DEVICE->max_data_rate)
      || DATA_RATE > ONE_NET_MAX_DATA_RATE))
    {
        return ONS_INVALID_DATA;
    } // if one of the devices can't support the data rate //

    mh = FALSE;

    return setup_data_rate_test((const on_encoded_did_t * const)&sender,
      (const on_encoded_did_t * const)&receiver, DATA_RATE, mh);
} // one_net_master_start_data_rate_test //


/*!
    \brief Starts the network process to invite a CLIENT to join the network.

    \param[in] KEY The unique key of the device to invite to join the network.

    \return ONS_SUCCESS if the process was successfully started
            ONS_BAD_PARAM if the parameter is invalid
            ONS_NOT_INIT The network has not been fully created.
            ONS_DEVICE_LIMIT If the MASTER cannot handle adding another device.
            ONS_RSRC_FULL if there is not an available transaction to send the
              invite.
            See on_encrypt & on_build_pkt for more return codes
*/
one_net_status_t one_net_master_invite(const one_net_xtea_key_t * const KEY)
{
    one_net_status_t status;
    UInt8 raw_invite[ON_RAW_INVITE_SIZE];

    if(!KEY)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(on_state == ON_INIT_STATE || on_state == ON_JOIN_NETWORK)
    {
        return ONS_NOT_INIT;
    } // if the network has not been created yet //

    if(invite_idx < ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_ALREADY_IN_PROGRESS;
    } // if already in the process of inviting //

    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return ONS_DEVICE_LIMIT;
    } // if the MASTER has reached it's device limit //

    invite_idx = get_free_txn(ON_INVITE, TRUE);
    if(invite_idx >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_RSRC_FULL;
    } // if a transaction resource is not available //

    one_net_memmove(invite_key, *KEY, sizeof(invite_key));

    raw_invite[ON_INVITE_VERSION_IDX] = ON_INVITE_PKT_VERSION;
    one_net_int16_to_byte_stream(master_param->next_client_did,
      &(raw_invite[ON_INVITE_ASSIGNED_DID_IDX]));
    one_net_memmove(&(raw_invite[ON_INVITE_KEY_IDX]),
      on_base_param->current_key, sizeof(on_base_param->current_key));
    one_net_int32_to_byte_stream(ONE_NET_MASTER_DEFAULT_KEEP_ALIVE,
      &raw_invite[ON_INVITE_KEEP_ALIVE_IDX]);
    raw_invite[ON_INVITE_CRC_IDX] = (UInt8)one_net_compute_crc(raw_invite,
      ON_INVITE_DATA_LEN, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);

    if((status = on_encrypt(ON_INVITE, raw_invite,
      (const one_net_xtea_key_t * const)(&invite_key), ON_RAW_INVITE_SIZE))
      == ONS_SUCCESS)
    {
        txn_list[invite_idx].txn.data_len = txn_list[invite_idx].txn.pkt_size;
        status = on_build_pkt(txn_list[invite_idx].txn.pkt,
          &(txn_list[invite_idx].txn.data_len),
          ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT, &ON_ENCODED_BROADCAST_DID,
          raw_invite, ON_ENCODED_INVITE_SIZE);
    } // if encrypting was not successful //

    if(status == ONS_SUCCESS)
    {
        txn_list[invite_idx].type = ON_INVITE;
        txn_list[invite_idx].txn.priority = ONE_NET_HIGH_PRIORITY;
        ont_set_timer(txn_list[invite_idx].txn.next_txn_timer,
          ONE_NET_MASTER_INVITE_SEND_TIME);
        ont_set_timer(ONT_INVITE_TIMER, one_net_master_invite_duration);
    } // if the invite has successfully been created //

    if(status != ONS_SUCCESS)
    {
        one_net_master_cancel_invite(
          (const one_net_xtea_key_t * const)&invite_key);
    } // if the invite was not created successfully //

    return status;
} // one_net_master_invite //


/*!
    \brief Cancels an invite request

    Even though there can only be 1 outstanding invite request at a time, pass
    in the KEY in case later on there can be multiple outstanding invite
    requests.  This will prevent the interface from changing later on if the
    implementation is changed.

    \param[in] KEY The unique key of the device to cancel the invite request for

    \return ONS_SUCCESS if the invite request was canceled.
            ONS_BAD_PARAM if the parameter is invalid.
*/
one_net_status_t one_net_master_cancel_invite(
  const one_net_xtea_key_t * const KEY)
{
    UInt8 i;

    if(!KEY)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    free_txn(invite_idx);
    ont_stop_timer(ONT_INVITE_TIMER);
    invite_idx = ONE_NET_MASTER_MAX_TXN;

    for(i = 0; i < ONE_NET_XTEA_KEY_LEN; i++)
    {
        invite_key[i] = 0x00;
    } // loop to clear key for good security measure //

    return ONS_SUCCESS;
} // one_net_master_cancel_invite //


/*!
    \brief Starts the process to remove a device from the network.

    \param[in] RAW_PEER_DID The device to remove from the network

    \return ONS_SUCCESS If the process to remove the device was started
            ONS_BAD_PARAM If the parameter was invalid
*/
one_net_status_t one_net_master_remove_device(
  const one_net_raw_did_t * const RAW_PEER_DID)
{
    // Derek_s 11/4/2010
	UInt8 i;
	on_encoded_did_t*  client_enc_did;
	one_net_raw_did_t client_raw_did;
    on_encoded_did_t encoded_peer_did;
    one_net_status_t status;
    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    if(!RAW_PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = on_encode(encoded_peer_did, *RAW_PEER_DID,
      sizeof(encoded_peer_did))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the dst did failed //

    if(!client_info((const on_encoded_did_t * const)&encoded_peer_did))
    {
        return ONS_INCORRECT_ADDR;
    } // the CLIENT is not part of the network //

#ifdef _ONE_NET_EVAL
#ifdef _PEER
	oncli_send_msg("Deleting did %03x.  First removing all relevant peer assignments to did %03x.\n",
	    did_to_u16(RAW_PEER_DID), did_to_u16(RAW_PEER_DID));
#else
	oncli_send_msg("Deleting did %03x.\n", did_to_u16(RAW_PEER_DID));
#endif
#endif

#ifdef _PEER
	for(i = 0; i < master_param->client_count; i++)
	{
		client_enc_did = &(client_list[i].did);

		if(on_encoded_did_equal(client_enc_did, &encoded_peer_did))
		{
			// it's the device we're removing anyway, so don't bother with any peer un-assignments
			continue;
		}

		if(on_decode(client_raw_did, *client_enc_did, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
		{
			// error.  couldn't decode.  Skip this one.
			continue;
		}

		// send out an unassign peer message to the client.  For the unit numbers, use ONE_NET_DEV_UNIT,
		// which will remove ALL peeer assignments where the did we are deleting is the target.
		one_net_master_peer_assignment(FALSE, &client_raw_did, ONE_NET_DEV_UNIT, RAW_PEER_DID,
		    ONE_NET_DEV_UNIT);
	}

    // Now remove any master peer assignments to the device being deleted.
    master_unassigned_peer(ONE_NET_DEV_UNIT, &encoded_peer_did, ONE_NET_DEV_UNIT, TRUE);
#endif

    // all client peer assignments to this did are removed.  Now remove the device itself
    return send_admin_pkt(ON_RM_DEV, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)&encoded_peer_did, ONE_NET_LOW_PRIORITY, pld,
      sizeof(pld));
} // one_net_master_remove_device //


/*!
    \brief The main function for the ONE-NET MASTER.

    \param void

    \return void
*/
void one_net_master(void)
{
    // The current transaction
    static on_txn_t * txn = 0;
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
	tick_t queue_sleep_time;
    #endif

    // Derek_S 11/10/2010 - Call to save_param() seems to now be commented
	// out.  Not sure what happened, but apparently there was a change and
	// things seem to work.

	// TO-DO : Find out what replaced the call to save_param() and if the
	// comment below is truly obsolete, as it appears to be, delete it so
	// it doesn't confuse things.

	// Derek_S 10/25/2010 - fixing problem with CLI "list" command
	// force a "save" (note that "save" DOES NOT save to non-volatile
	// memory.  It is separate from the CLI "save" command) so that
	// the "list" command will have an appropriate flag.  If you type
	// in "list" before anything "interesting" happens(i.e. key change, new
	// client, etc., "list" won't perform properly upon a power cycle after
	// a save.  So we call save_param() as sort of a memory jog, not because
	// we are saving things.  "list" requires things to be "saved" at least once
	// to work
	//#ifdef _ONE_NET_EVAL
	//    save_param();
    //#endif

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
            if(invite_idx < ONE_NET_MASTER_MAX_TXN
              && ont_inactive_or_expired(
              txn_list[invite_idx].txn.next_txn_timer))
            {
                if(ont_expired(ONT_INVITE_TIMER))
                {
                    one_net_master_invite_result(ONS_TIME_OUT, invite_key, 0);
                    one_net_master_cancel_invite(
                      (const one_net_xtea_key_t * const)&invite_key);
                    break;
                } // if trying to add device timed out //

                txn = &(txn_list[invite_idx].txn);
                txn->retry++;

                ont_set_timer(txn_list[invite_idx].txn.next_txn_timer,
                  ONE_NET_MASTER_INVITE_SEND_TIME);
                one_net_set_data_rate(ONE_NET_DATA_RATE_38_4);
                on_state = ON_SEND_PKT;
                break;
            } // if time to send an invite //
            else if(txn_head < ONE_NET_MASTER_MAX_TXN
              && ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                txn = &(txn_list[txn_head].txn);

                // update head of the list since the transaction is being sent
                cur_txn_info = txn_head;
                txn_head = txn_list[txn_head].next;
                txn_list[cur_txn_info].next = ONE_NET_MASTER_MAX_TXN;

                if(txn_list[cur_txn_info].type == ON_DATA_RATE_TXN)
                {
                    on_state = ON_INIT_SEND_DATA_RATE;
                    break;
                } // if the MASTER is performing a data rate test //

                else
                {
                    if(txn_list[cur_txn_info].txn.send
                      && (txn_list[cur_txn_info].type == ON_SINGLE ||
                      ont_inactive_or_expired(
                      txn_list[cur_txn_info].txn.next_txn_timer)))
                    {
                        switch(txn_list[cur_txn_info].type)
                        {
                            case ON_SINGLE:
                            {
                                #ifdef _BLOCK_MESSAGES_ENABLED
                                check_b_s_req();
                                #endif
                                on_state = ON_SEND_SINGLE_DATA_PKT;
                                break;
                            } // ON_SINGLE CASE //

                            #ifdef _BLOCK_MESSAGES_ENABLED
                            case ON_BLOCK:
                            {
                                if(txn_list[cur_txn_info].txn.retry == 0)
                                {
                                    on_nack_rsn_t nack_reason;

                                    if((nack_reason = get_next_b_s_pld(cur_txn_info)) !=
                                       ON_NACK_RSN_NO_ERROR)
                                    {
                                        one_net_raw_did_t raw_did;

                                        if(on_decode(raw_did,
                                          txn_list[cur_txn_info].did,
                                          sizeof(txn_list[cur_txn_info].did))
                                          == ONS_SUCCESS)
                                        {
                                            one_net_master_block_txn_status(
                                              ONS_BLOCK_FAIL,
                                              (const one_net_raw_did_t * const)
                                              &raw_did);
                                        } // if decoding was successful //
                                        else
                                        {
                                            one_net_master_block_txn_status(
                                              ONS_INTERNAL_ERR,
                                              (const one_net_raw_did_t * const)
                                              &raw_did);
                                        } // if decoding failed //

                                        cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                                        break;
                                    } // if getting next payload unsuccessful //
                                } // if a block transaction & retry == 0 //

                                on_state = ON_SEND_BLOCK_DATA_PKT;
                                break;
                            } // ON_BLOCK case //
                            #endif

                            #ifdef _STREAM_MESSAGES_ENABLED
                            case ON_STREAM:
                            {
                                if(txn_list[cur_txn_info].txn.remaining)
                                {
                                    if(!get_next_b_s_pld(cur_txn_info))
                                    {
                                        break;
                                    } // if getting next payload unsuccessful //

                                    on_state = ON_SEND_STREAM_DATA_PKT;
                                } // if continuing with stream //
                                else
                                {
                                    // send 0's since field is not used
                                    UInt8 data[ON_MAX_ADMIN_PLD_LEN] = {0};

                                    // Send the single to end the stream
                                    txn_list[cur_txn_info].type = ON_SINGLE;
                                    txn_list[cur_txn_info].txn.data_len
                                      = sizeof(data);
                                    txn_list[cur_txn_info].txn.msg_type
                                      = ON_ADMIN_MSG;
                                    txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
                                      = ON_END_STREAM;
                                    one_net_memmove(
                                      &(txn_list[cur_txn_info].txn.pkt[ON_DATA_IDX]),
                                      data, sizeof(data));
                                    single_txn_data_count++;

                                    on_state = ON_SEND_SINGLE_DATA_PKT;
                                } // else ending the stream //
                                break;
                            } // ON_STREAM case //
							#endif

                            default:
                            {
                                return;
                                break;
                            } // default case //
                        } // switch(txn type) //

                        if(build_txn_data_pkt(cur_txn_info) == ONS_SUCCESS)
                        {
                            // get the device to set the data rate the device
                            // receives at
                            const on_client_t * CLIENT = 0;

                            one_net_raw_did_t raw_dst;
                            one_net_status_t status = ONS_INTERNAL_ERR;

                            if((status = on_decode(raw_dst,
                              &(txn_list[cur_txn_info].txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                              ON_ENCODED_DID_LEN)) != ONS_SUCCESS || (CLIENT =
                              client_info((const on_encoded_did_t * const)
                              &(txn_list[cur_txn_info].txn.pkt[ONE_NET_ENCODED_DST_DID_IDX])))
                              == 0)
                            {
                                #ifdef _BLOCK_MESSAGES_ENABLED
                                if(txn_list[cur_txn_info].type == ON_BLOCK
                                  || b_s_req.type == ON_BLOCK)
                                {
                                    one_net_master_block_txn_status(
                                      ONS_INTERNAL_ERR,
                                      (const one_net_raw_did_t * const)
                                      &raw_dst);
                                } // if a block (or block request) //
								#ifdef _STREAM_MESSAGES_ENABLED
                                else if(txn_list[cur_txn_info].type
                                  == ON_STREAM || b_s_req.type == ON_STREAM)
                                {
                                    one_net_master_stream_txn_status(
                                      ONS_INTERNAL_ERR,
                                      (const one_net_raw_did_t * const)
                                      &raw_dst);
                                } // else if a stream (or stream request) //
								#endif
                                else
                                {
                                    #endif
                                    one_net_master_single_txn_status(
                                      ONS_INTERNAL_ERR, 0, 0,
                                      (const one_net_raw_did_t * const)
                                      &raw_dst);
                                    #ifdef _BLOCK_MESSAGES_ENABLED
                                } // else it's a single transaction //

                                b_s_req.type = ON_NO_TXN;
                                #endif
                                free_txn(cur_txn_info);
                                cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                            } // if getting the client info failed //
                            else
                            {
                                one_net_set_data_rate(CLIENT->data_rate);
                                break;
                            } // else getting the CLIENT info was successful //
                        } // if building the packet was successful //
                        else
                        {
                            one_net_raw_did_t raw_dst;

                            on_decode(raw_dst,
                              &(txn_list[cur_txn_info].txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                              ON_ENCODED_DID_LEN);

                            #ifdef _BLOCK_MESSAGES_ENABLED
                            if(txn_list[cur_txn_info].type == ON_BLOCK
                              || b_s_req.type == ON_BLOCK)
                            {
                                one_net_master_block_txn_status(
                                  ONS_INTERNAL_ERR,
                                  (const one_net_raw_did_t * const)&raw_dst);
                            } // if a block (or block request) //
							#ifdef _STREAM_MESSAGES_ENABLED
                            else if(txn_list[cur_txn_info].type
                              == ON_STREAM || b_s_req.type == ON_STREAM)
                            {
                                one_net_master_stream_txn_status(
                                  ONS_INTERNAL_ERR,
                                  (const one_net_raw_did_t * const)&raw_dst);
                            } // else if a stream (or stream request) //
							#endif
                            else
                            {
                                #endif
                                one_net_master_single_txn_status(
                                  ONS_INTERNAL_ERR, 0, 0,
                                  (const one_net_raw_did_t * const)&raw_dst);
                                #ifdef _BLOCK_MESSAGES_ENABLED
                            } // else it's a single transaction //

                            b_s_req.type = ON_NO_TXN;
                            #endif
                            free_txn(cur_txn_info);
                            cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                        } // else building the txn was not successful //

                        on_state = ON_LISTEN_FOR_DATA;
                    } // if sending the transaction and it's time to send //
                    #ifdef _BLOCK_MESSAGES_ENABLED
                    else if(!txn_list[cur_txn_info].txn.send)
                    {
                        if(ont_inactive_or_expired(
                          txn_list[cur_txn_info].txn.next_txn_timer))
                        {
                            txn_list[cur_txn_info].txn.retry++;
                            if(txn_list[cur_txn_info].txn.retry >= ON_MAX_RETRY)
                            {
                                one_net_raw_did_t did;

                                if(on_decode(did, txn_list[cur_txn_info].did,
                                  sizeof(txn_list[cur_txn_info].did))
                                  == ONS_SUCCESS)
                                {
									#ifdef _STREAM_MESSAGES_ENABLED
                                    if(txn_list[cur_txn_info].type == ON_STREAM)
                                    {
                                        one_net_master_stream_txn_status(
                                          ONS_STREAM_FAIL,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // if a stream transaction //
                                    else if(
									#else
									if(
									#endif
                                      txn_list[cur_txn_info].txn.remaining)
                                    {
                                        one_net_master_block_txn_status(
                                          ONS_BLOCK_FAIL,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else if blocks remain //
                                    else
                                    {
                                        // no blocks remaining to be received
                                        // so the ONE_NET_ENCODED_BLOCK_TXN_ACK
                                        // must have been missed.
                                        one_net_master_block_txn_status(
                                          ONS_BLOCK_END,
                                          (const one_net_raw_did_t * const)
                                          &did);
                                    } // else receiving & no blocks remained //
                                } // if decoding did was successful //

                                free_txn(cur_txn_info);
                            } // if the transaction failed //
                            else
                            {
                                on_update_next_txn_time(
                                  &(txn_list[cur_txn_info].txn));
                                enqueue_txn(cur_txn_info);
                            } // queue it to try again //
                        } // if the transaction time passed //
                        else
                        {
                            // it is not time for the txn yet
                            enqueue_txn(cur_txn_info);
                        } // else the transaction time has not passed //
                    } // else if receiving the transaction //
                    else
                    {
                        // not time for the txn yet, so put it back in the q
                        enqueue_txn(cur_txn_info);
                    } // else must not be time for the txn //
                    #endif
                } // else it is a single/block/stream transaction //
            } // else if a transaction is queued //


            // not carrying out a transaction at this time
            cur_txn_info = ONE_NET_MASTER_MAX_TXN;

            switch(on_rx_data_pkt(&ON_ENCODED_BROADCAST_DID, &txn))
            {
				#ifdef _BLOCK_MESSAGES_ENABLED
                case ONS_BLOCK_FAIL:
                {
                    if(cur_txn_info < ONE_NET_MASTER_MAX_TXN
                      && txn_list[cur_txn_info].type == ON_BLOCK)
                    {
                        free_txn(cur_txn_info);
                        cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                    } // if the txn was set in the call //
                    break;
                } // block fail case //
				#endif

                #ifdef _STREAM_MESSAGES_ENABLED
                case ONS_STREAM_END:                    // fall through
                case ONS_STREAM_FAIL:
                {
                    if(cur_txn_info < ONE_NET_MASTER_MAX_TXN
                      && txn_list[cur_txn_info].type == ON_STREAM
                      && on_state != ON_SEND_SINGLE_DATA_PKT)
                    {
                        free_txn(cur_txn_info);
                        cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                    } // if the txn should be freed //
                    break;
                } // stream end/fail case //
				#endif

                default:
                {
                    break;
                } // default case //
            } // switch on on_rx_data_pkt result //

            if(ont_active(data_rate_test.timer)
              && ont_expired(data_rate_test.timer))
            {
                report_data_rate_result(data_rate_test.rate, 0, 0);
            } // if the data rate timer expired //

            check_key_update(TRUE);
			#ifdef _STREAM_MESSAGES_ENABLED
            check_stream_key_update(TRUE);
			#endif
            break;
        } // listen_for_data case //

        case ON_JOIN_NETWORK:
        {
            if(one_net_channel_is_clear())
            {
                if(ont_expired(ONT_GENERAL_TIMER))
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    ont_stop_timer(ONT_CHANGE_KEY_TIMER);
                    save_param();
                } // if channel has been clear for enough time //
            } // if channel is clear //
            else
            {
                on_base_param->channel++;
                #if defined(_ONE_NET_EVAL) && defined(_US_CHANNELS)
                    // the eval board contains both US and European channels,
                    // but the MASTER defaults to creating the network on the
                    // US channel.
                    on_base_param->channel %= (ONE_NET_MAX_US_CHANNEL + 1);
                #else // if picking only a U.S. Channel //
                    on_base_param->channel %= ONE_NET_NUM_CHANNELS;
                #endif // else picking any channel //
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

        default:
        {
            if(one_net(&txn) && cur_txn_info < ONE_NET_MASTER_MAX_TXN
              && txn_list[cur_txn_info].type != ON_INVITE)
            {
				#ifdef _BLOCK_MESSAGES_ENABLED
                if(txn_list[cur_txn_info].type == ON_SINGLE
				#ifdef _STREAM_MESSAGES_ENABLED
                  && (b_s_req.type == ON_BLOCK || b_s_req.type == ON_STREAM))
				#else
                  && (b_s_req.type == ON_BLOCK))
				#endif
                {
                    // reusing the txn_list element for the block transaction.
                    txn_list[cur_txn_info].type = b_s_req.type;
                    txn_list[cur_txn_info].txn.remaining = b_s_req.len;
                    txn_list[cur_txn_info].txn.send = b_s_req.send;
                    txn_list[cur_txn_info].txn.retry = 0;

                    if(!txn_list[cur_txn_info].txn.send)
                    {
                        // receiving the transaction, so release the
                        // transaction data back to the pool
                        txn_list[cur_txn_info].txn.data_len = 0;
                        txn_list[cur_txn_info].txn.pkt_size = 0;
                        txn_list[cur_txn_info].txn.pkt = 0;

                        if(txn_list[cur_txn_info].pkt_idx
                          < ONE_NET_MASTER_MAX_SEND_TXN)
                        {
                            pkt_list[txn_list[cur_txn_info].pkt_idx].in_use
                              = FALSE;
                            txn_list[cur_txn_info].pkt_idx
                              = ONE_NET_MASTER_MAX_SEND_TXN;
                        } // if the pkt_idx is valid //

                        txn_data_count--;
                    } // if receiving the transaction //

                    single_txn_data_count--;
                    b_s_req.type = ON_NO_TXN;

                    if(txn_list[cur_txn_info].txn.msg_type == ON_ADMIN_MSG)
                    {
                        // if an admin message was used to set up the
                        // transaction, then the transaction was set up to send
                        // block or stream data.
                        txn_list[cur_txn_info].txn.msg_type = ON_APP_MSG;
                    } // if an admin message //

                    on_update_next_txn_time(&(txn_list[cur_txn_info].txn));
                    enqueue_txn(cur_txn_info);
                } // if a block request was made //
                else
                {
                    free_txn(cur_txn_info);
                } // else not an admin request for a block transaction //
				#else  // if block messages are enabled. //
				    free_txn(cur_txn_info);
				#endif // if block messages are not enabled. //
                cur_txn_info = ONE_NET_MASTER_MAX_TXN;

            } // if the transaction was completed successfully //

            break;
        } // default case //
    } // switch(on_state) //

   // check to see if we need to save parameters
    if(save)
    {
        save = FALSE;
        save_param();
    } // if the parameters should be saved //

    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
	// we don't care about the sleep time for a master, but the function wants it.
	adjust_single_data_queue(&queue_sleep_time);
    #endif
} // one_net_master //


/*!
    \brief Add a client to the current network.

    This function can be used when you need to add a client
    to the current network and you do not want to use the normal
    invite/join process. A client is added with the information
    supplied and the information that the client will need to
    communicate on the current network is returned.

    \param[in] CAPABILITIES Pointer to a structure holding the client's capabilities.
    \param[out] config Pointer to a structure used to return networl configuration information.

    \return ONS_SUCCESS if the client was added.
            ONS_BAD_PARAM if the parameter was invalid.
            ONS_DEVICE_LIMIT if there is no room to hold another client.
            See on_decode error returns for the other values that may
            be returned.
*/
// Derek_S 11/3/2010 - this function is a bit hard to test since we've been testing it by
// inviting/deleting it the normal way.
// TO -DO : test this to make sure nothing was missed.
one_net_status_t one_net_master_add_client(
  const one_net_master_add_client_in_t * CAPABILITIES,
  one_net_master_add_client_out_t * config)
{
    // instead of separate arguments for values returned.
    one_net_status_t status;

    on_client_t * client;
    UInt16 new_client_did;
    UInt16 raw_master_did;
    UInt8 features = 0;

    new_client_did = master_param->next_client_did;

    // a device is being added, find the next available client_t
    // structure
    client = one_net_master_add_new_client_to_list();

    if(!client)
    {
        // the master can't handle any more devices
        status = ONS_DEVICE_LIMIT;
        return status;
    } // if the pointer is 0 //

    //
    // initialize the fields in the client_t structure for this new client
    //
    client->expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);
    client->last_nonce = ON_MAX_NONCE + 1;
    client->send_nonce = 0;
    client->data_rate = on_base_param->data_rate;
    client->max_data_rate = CAPABILITIES->max_data_rate;
    client->use_current_key = TRUE;

    //
    // fill in the network configuration values this new client
    // will need to operate in this ONE-NET network.
    //

    // the master's DID is just past the NID within the encoded SID, copy it to master_did
    one_net_memmove(config->master_did, on_base_param->sid+ON_ENCODED_NID_LEN, sizeof(on_encoded_did_t));

    // decode the SID to get a raw SID for the master
    if((status = on_decode(config->raw_sid, on_base_param->sid, sizeof(on_encoded_sid_t)))
      != ONS_SUCCESS)
    {
        return status;
    }

    // extract the raw master DID from the end of the raw SID, before replacing it
    raw_master_did = config->raw_sid[ONE_NET_RAW_NID_LEN-1]<<8;// pick up the two byte DID
    raw_master_did |= config->raw_sid[ONE_NET_RAW_NID_LEN];
    raw_master_did <<= 4;                                   // and shift out the end of the NID
    config->master_did[0] = (raw_master_did>>8) & 0xff;     // store DID as a big endian
    config->master_did[1] = raw_master_did & 0xff;          // 16 bit value left justified

    // replace the DID field in this raw SID with this new client's DID
    config->raw_sid[ONE_NET_RAW_NID_LEN-1] &= 0xf0;
    config->raw_sid[ONE_NET_RAW_NID_LEN-1] |= (new_client_did>>12) & 0x0f; //TODO: MR2: is there a ONE-NET const for 12?
    config->raw_sid[ONE_NET_RAW_NID_LEN] = (new_client_did>>4) & 0xff; //TODO: MR2: is there a ONE-NET const for 0xff?

    one_net_memmove(config->current_key, on_base_param->current_key, sizeof(one_net_xtea_key_t));
    config->keep_alive_interval = ONE_NET_MASTER_DEFAULT_KEEP_ALIVE;
    config->single_block_encrypt_method = on_base_param->single_block_encrypt;
    config->master_data_rate = on_base_param->data_rate;
    config->data_rate = on_base_param->data_rate;
    config->channel = on_base_param->channel;
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_memmove(config->stream_key, on_base_param->stream_key, sizeof(one_net_xtea_key_t));
    config->stream_encrypt_method = on_base_param->stream_encrypt;
#endif
#ifdef _BLOCK_MESSAGES_ENABLED
    config->fragment_delay_low = on_base_param->fragment_delay_low;
    config->fragment_delay_high = on_base_param->fragment_delay_high;
#endif

    // save the new client
    save = TRUE;

    return ONS_SUCCESS;
} // one_net_master_add_client //


/*!
    \brief Delete a client.

    This function has been retained for historiacal purposes.  There is no attempt made
    to ensure that the DID provided is the LAST client added.

    \param[in] raw_client_did Pointer to the raw DID of the client to be deleted.

    \return ONS_SUCCESS if the client was deleted.
            ONS_INVALID_DATA if the client was not deleted.
*/
one_net_status_t one_net_master_delete_last_client (one_net_raw_did_t * raw_client_did)
{
    on_encoded_did_t encoded_client_did;

    on_encode(encoded_client_did, (UInt8 *)raw_client_did, sizeof(on_encoded_did_t));
    return rm_client(&encoded_client_did);
} // one_net_master_delete_last_client //


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] CLIENT_DID The enocded device id of the CLIENT to retrieve the
      information for.

    \return The CLIENT information if the information was found
            0 If an error occured.
*/
on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID)
{
    UInt16 i;

    if(!CLIENT_DID)
    {
        return 0;
    } // if the parameter is invalid //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(CLIENT_DID,
          (const on_encoded_did_t * const)&client_list[i].did))
        {
            return &(client_list[i]);
        } // if the CLIENT was found //
    } // loop to find the CLIENT //

    return 0;
} // client_info //


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] current_key TRUE if the function should return the current key,
	                   FALSE if the function should return the old key
    \param[in] stream_key TRUE if the function should return the stream key,
	                   FALSE if the function should return the single/block key

    \return The encryption key that matches the parameters
*/
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_xtea_key_t* get_encryption_key(const BOOL current_key, const BOOL stream_key)
#else
    one_net_xtea_key_t* get_encryption_key(const BOOL current_key)
#endif
{
    const one_net_xtea_key_t* new_key = &(on_base_param->current_key);
    const one_net_xtea_key_t* old_key = &(master_param->old_key);

    #ifdef _STREAM_MESSAGES_ENABLED
	    if(stream_key)
		{
            new_key = &(on_base_param->stream_key);
            old_key = &(master_param->old_stream_key);
		}
	#endif

	if(current_key)
	{
		return new_key;
	}

	return old_key;
}


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] client pointer to a client's settings in the network
    \param[in] stream_key TRUE if the function should return the stream key,
	                   FALSE if the function should return the single/block key

    \return The encryption key that this client uses
*/
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_xtea_key_t* get_client_encryption_key(const on_client_t* const client, const BOOL stream_key)
#else
    one_net_xtea_key_t* get_client_encryption_key(const on_client_t* const client)
#endif
{
    BOOL current_key;

    if(client == NULL)
	{
		return NULL;
	}

	current_key = client->use_current_key;
    #ifdef _STREAM_MESSAGES_ENABLED
        if(stream_key)
		{
			current_key = client->use_current_stream_key;
		}
		return get_encryption_key(current_key, stream_key);
    #else
	    return get_encryption_key(current_key);
    #endif
}


#ifdef _PEER
UInt8 master_get_num_free_send_txn(void)
{
    UInt8 i;
    UInt8 num_free_txn = 0;

    for(i = 0; i < ONE_NET_MASTER_MAX_TXN; i++)
    {
        if(i != invite_idx && txn_list[i].type == ON_NO_TXN)
        {
            num_free_txn++;
        } // if a free transaction was found //
    } // loop through to find free transactions //

    return num_free_txn;
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
    \brief Initializes internal data structures.

    This will also initialize the base one_net functionality.

    \param void

    \return void
*/
static void init_internal(void)
{
    on_pkt_hdlr_set_t pkt_hdlr;

    UInt8 i;

    data_rate_test.recv_idx = ONE_NET_MASTER_MAX_TXN;
    data_rate_test.send_idx = ONE_NET_MASTER_MAX_TXN;
    one_net_memmove(data_rate_test.recv_did, ON_ENCODED_BROADCAST_DID,
      sizeof(data_rate_test.recv_did));
    one_net_memmove(data_rate_test.send_did, ON_ENCODED_BROADCAST_DID,
      sizeof(data_rate_test.send_did));
    data_rate_test.timer = ONT_DATA_RATE_TEST_TIMER;

    txn_head = ONE_NET_MASTER_MAX_TXN;
    invite_idx = ONE_NET_MASTER_MAX_TXN;
    new_channel_clear_time_out = 0;

    // Assign the timers that each transaction uses
    for(i = 0; i < ONE_NET_MASTER_MAX_TXN; i++)
    {
        txn_list[i].txn.next_txn_timer = ONT_FIRST_TXN_TIMER + i;
    } // loop to assign timers to transactions //

    pkt_hdlr.single_data_hdlr = &on_master_single_data_hdlr;
    pkt_hdlr.single_ack_nack_hdlr = &on_master_handle_ack_nack_response;
    pkt_hdlr.status_msg_hdlr = &one_net_master_handle_status_msg;
    pkt_hdlr.single_txn_hdlr = &on_master_single_txn_hdlr;
    pkt_hdlr.data_rate_hdlr = &on_master_data_rate_hdlr;
	#ifdef _BLOCK_MESSAGES_ENABLED
    pkt_hdlr.block_data_hdlr = &on_master_b_s_data_hdlr;
    pkt_hdlr.block_txn_hdlr = &on_master_b_s_txn_hdlr;
	#endif
	#ifdef _STREAM_MESSAGES_ENABLED
    pkt_hdlr.stream_data_hdlr = &on_master_b_s_data_hdlr;
    pkt_hdlr.stream_txn_hdlr = &on_master_b_s_txn_hdlr;
	#endif

    deviceIsMaster = TRUE;

    one_net_init(&pkt_hdlr);

    #ifdef _PEER
    on_base_param->features += NUM_MASTER_PEER;
    #endif
} // init_internal //


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

    \return ONS_SUCCESS if the payload was successfully parsed.
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INTERNAL_ERR if something unexpected occured.
            ONS_BAD_PKT If data in the packet is invalid.
            See on_parse_pld for more possible return values.
*/
one_net_status_t on_master_single_data_hdlr(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, UInt8 * const pld, on_txn_t ** txn)
{
    one_net_status_t status = ONS_INTERNAL_ERR;
    on_client_t * client;
    one_net_raw_did_t raw_src_did;
    UInt8 txn_nonce, resp_nonce;
	UInt8 single_msg_type; // Application message, Admin message, extended admin message
    BOOL tried_new_key = FALSE;
    const one_net_xtea_key_t* key = 0;

    ona_msg_class_t msg_class;
	ona_msg_type_t msg_type; // ONA_QUERY, ONA_COMMAND, etc.
	BOOL useDefaultHandling = TRUE;
	UInt8 src_unit;
	UInt8 dst_unit;
	UInt16 msg_data;
    on_nack_rsn_t nack_reason = ON_NACK_RSN_NO_ERROR;
	on_pid_t resp_pid  = ONE_NET_ENCODED_SINGLE_DATA_ACK;
	on_ack_nack_handle_t ack_nack_handle = ON_ACK; // also corresponds to ON_NACK;
	ack_nack_payload_t ack_nack_payload;

	UInt8 response_payload[ONA_SINGLE_PACKET_PAYLOAD_LEN]; // for status responses to query
	UInt8* resp_pld_ptr = response_payload; // for status responses to query
    tick_t time_from_now = 0; // for status responses to query



	// June 2, 2011 - Ignoring all Block and Stream Code.
	// Also ignoring all key update code.
	// TODO - Go back to Version 1.5 and put that back in when we get everything
	// sorted out.


    if((PID != ONE_NET_ENCODED_SINGLE_DATA
      && PID != ONE_NET_ENCODED_REPEAT_SINGLE_DATA)
      || !SRC_DID || !pld || !txn)
    {
        if(txn)
        {
            *txn = 0;               // not responding
        } // if txn //

        return ONS_BAD_PARAM;
    } // if any parameter is invalid //

    *txn = &response_txn;

    if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
      != ONS_SUCCESS)
    {
        return ONS_INTERNAL_ERR;
    } // if decoding failed //

    if((client = client_info(SRC_DID)) != NULL)
    {
        one_net_master_device_is_awake(
          (const one_net_raw_did_t * const)&raw_src_did);
    } // if the CLIENT exists //

    if(!client || client->use_current_key)
    {
        key = &(on_base_param->current_key);
        status = on_parse_pld(&txn_nonce, &resp_nonce, &single_msg_type, pld,
          ON_SINGLE, key);

        if(!client)
        {
            if(single_msg_type != ON_ADMIN_MSG)
            {
                status = ONS_INTERNAL_ERR;
            } // if it is not an admin message //
        } // if the device is not part of the network //
    } // if null client using current key //


    // June 2, 2011 - TODO - The entire key change protocol will very likely
    // be changed.  I'm commenting out the code in for now, but ignoring it since it
    // will be revisited.  For now, this will be unhandled.  See the new temporary "else"
	// below.  No key changes allowed till we get the new scheme.
	// TODO - update this.
	else
	{
		*txn = 0;
		return ONS_BAD_PKT;
	}

// TODO - Keeping the code below for when we come back to key changes.
//        Go back to 1.5. and 1.6 and see what was happening.
#if 0
    else
    {

        // copy the payload just in case the CLIENT really is using the
        // current key (since on_parse_pld modifies the data)
        UInt8 pld_cpy[ON_RAW_BLOCK_STREAM_PLD_SIZE];

        one_net_memmove(pld_cpy, pld, sizeof(pld_cpy));
        key = &(master_param->old_key);
        if((status = on_parse_pld(&txn_nonce, &resp_nonce, &single_msg_type, pld,
          ON_SINGLE, key)) != ONS_SUCCESS)
        {
            key = &(on_base_param->current_key);
            // try the new key
            one_net_memmove(pld, pld_cpy, ON_RAW_BLOCK_STREAM_PLD_SIZE);
            status = on_parse_pld(&txn_nonce, &resp_nonce, &single_msg_type, pld,
              ON_SINGLE, key);
            tried_new_key = TRUE;
        } // if decoding with the old key is not successful //
    } // else try the old key //
#endif


	// if we've made it this far, we might possibly send back a response.
	// The most likely error is a bad nonce.  At least three possibilities.
	// 1) txn_nonce is equal to client->expected_nonce -->  This is the good case.
	//        Nonces check out.  We'll handle the packet.
	// 2) txn_nonce is equal to client->last_nonce and the pid is
	//        ONE_NET_ENCODED_REPEAT_SINGLE_DATA.  Something went wrong here and
	//        there was a packet sent with a good nonce, but it was an expired
	//        good nonce.  Maybe it just got here a little late, so we should
	//        handle this packet anyway.
	//
	// TODO: Look at Multi-Hop, Block, Stream, and make this adjustment.
	//
	// 3) Neither 1 or 2.  Give a NACK here and adjust the nonces
	//        so they'll hopefully repeat things correctly next time.


	// We also want to check whether we've already ACK'd this recently.  If so, just ACK
	// it again.
	if(client != 0 && single_data_is_repeat(&raw_src_did, &(pld[ON_PLD_DATA_IDX])))
	{
		// send another ACK.  Skip all other handling.
		goto omsdh_build_resp;
	}

	// If we have a null client, we'll skip the nonces check.
	if(client != 0 && (txn_nonce != client->expected_nonce)  &&
	       !(PID == ONE_NET_ENCODED_REPEAT_SINGLE_DATA && txn_nonce == client->last_nonce))
	{
		// bad nonce error.
		nack_reason = ON_NACK_RSN_NONCE_ERR;
        resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
		status = ONS_INCORRECT_NONCE;
		goto omsdh_build_resp;
	}


    // if the client is 0, then the packet must be an admin packet which we
    // want to handle in case it is a Status Response for a CLIENT joining
    // the network.  If it is a new CLIENT, client will be returned from
    // handle_admin_pkt so the proper response can be sent.
    switch(single_msg_type)
    {
        case ON_APP_MSG:
        {
			#ifdef _STREAM_MESSAGES_ENABLED
            // only handle if we're not waiting for a stream response
            if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP)
            {
				*txn = 0;
                return ONS_BAD_PKT_TYPE;
            } // if waiting for the stream data response //
			#endif

            nack_reason = on_parse_single_app_pld(&(pld[ON_PLD_DATA_IDX]), &src_unit,
                &dst_unit, &msg_class, &msg_type, &msg_data);

			if(nack_reason != ON_NACK_RSN_NO_ERROR)
			{
                resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
		        status = ONS_TX_NACK;
				goto omsdh_build_resp;
			}


            // I've been debating how to handle status messages and whether we need a status
			// message handler at all and whether and when to call it.  For now I think we should
			// call not call it here.  Instead we'll call the regular old single data handler
			// and IT can call the status message handler if it likes.  The status manager will
			// be called directly from ONE-NET only in responses to fast queries.  I may take
			// that out and have that handled in the single handler as well.
			// TODO - revisit this.  For now, commenting out the call.

            /*if(msg_class == ONA_STATUS_QUERY_RESP)
			{
				if(!one_net_master_handle_status_msg(&(pld[ON_PLD_DATA_IDX]), msg_class, msg_type, src_unit, dst_unit,
				    msg_data, &raw_src_did, &nack_reason, &ack_nack_handle, &ack_nack_payload))
                {
				    // application layer code does not want to send a response.
                    *txn = 0;
				    return ONS_SINGLE_CANCELED;
                } // if application code layer wants to cancel handling //
			}	*/


			// set useDefaultHandling to true.  Application code can set it to false
			// if it wishes.  Note: There could already be a nack reason.  Send it to
			// the application code to handle anyway.  Chances are that it will decide to
			// keep it as is, but we'll give it the opportunity to change it or see it if
			// it wants to.  But if there's still a nack reason when we get BACK from the function,
			// we'll still handle it.
			useDefaultHandling = TRUE;

            if(!one_net_master_handle_single_pkt(&(pld[ON_PLD_DATA_IDX]), msg_class,
			       &msg_type, src_unit, dst_unit, &msg_data, &raw_src_did,
				   &useDefaultHandling, &nack_reason, &ack_nack_handle, &ack_nack_payload))
            {
				// application layer code does not want to send a response.
                *txn = 0;
				return ONS_SINGLE_CANCELED;
            } // if application code layer wants to cancel handling //


			if(nack_reason != ON_NACK_RSN_NO_ERROR)
			{
				resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
				status = ONS_TX_NACK;
				goto omsdh_build_resp;
			}

			else if(useDefaultHandling)
			{
				// see if there is any default handling we need to do
				// Note: If we're using default handling, any changes the application code
				// made to ack_nack_handle and ack_nack_payload will be have no effect
				// since we're re-initializing.  If the application code needs to make
				// changes to these values, it should set useDefaultHandling to false.
				ona_msg_class_t resp_msg_class = ONA_STATUS_QUERY_RESP;
				ack_nack_handle =  ON_ACK;

                // if this was a query or a command, send back a response
				switch(msg_class)
				{
					case ONA_POLL: case ONA_COMMAND:// these cases fall through
					    resp_msg_class = ((msg_class == ONA_POLL)? ONA_STATUS_FAST_QUERY_RESP
						                                         : ONA_STATUS_ACK_RESP);
						resp_pld_ptr = ack_nack_payload.status_resp; // status will be part of the ACK
					case ONA_QUERY:
					    put_msg_hdr(resp_msg_class | msg_type, resp_pld_ptr);
						put_src_unit(dst_unit, resp_pld_ptr); // source and destination units are reversed
						put_dst_unit(src_unit, resp_pld_ptr); // source and destination units are reversed
						put_msg_data(msg_data, resp_pld_ptr); // msg_data was filled in by application code

						if(resp_msg_class == ONA_STATUS_QUERY_RESP)
						{
							// stick this in the queue to be sent.  Note: dest and source are reversed
							// so the parameters below are correct.  raw_src_did is not OUR did, it is
							// the did that the query came from.  We are not sending to the peer list
                            // so the third parameter is false.
							status = one_net_master_send_single(resp_pld_ptr, ONA_SINGLE_PACKET_PAYLOAD_LEN,
                                FALSE, ONE_NET_SEND_SINGLE_PRIORITY, &raw_src_did, dst_unit,
						        &time_from_now, &time_from_now);

						    if(status != ONS_SUCCESS)
							{
								// Can't send a status response.  we're sending a NACK
								resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
								nack_reason = ON_NACK_RSN_RSRC_UNAVAIL_ERR;
								ack_nack_handle = ON_NACK;
								status = ONS_UNHANDLED_PKT; // TODO:change this to "send nack" or something
							}
						}
				}
			}

            break;
        } // application message case //

        case ON_ADMIN_MSG:
        {
            status = handle_admin_pkt(SRC_DID, &(pld[ON_PLD_DATA_IDX]),
              ONE_NET_RAW_SINGLE_DATA_LEN, &client, &nack_reason,
              &ack_nack_handle, &ack_nack_payload);

            if(status != ONS_SUCCESS)
            {

                if(nack_reason == ON_NACK_RSN_NO_ERROR)
                {
                    nack_reason = ON_NACK_RSN_GENERAL_ERR;
                }
            }
            if(nack_reason != ON_NACK_RSN_NO_ERROR)
            {
                resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
            }
            break;
        } // admin message case //

        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_EXTENDED_ADMIN_MSG:
        {
            // MASTER does not currently receive any extended admin packets
            status = ONS_UNHANDLED_PKT;
			resp_pid = resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
			nack_reason = ON_NACK_RSN_GENERAL_ERR;
            break;
        } // extended admin message case //
		#endif

        default:
        {
            status = ONS_BAD_PKT;
            break;
        } // default case //
    } // switch(single_msg_type) //

    if(status == ONS_SUCCESS || status == ONS_TXN_QUEUED)
    {
        if(status == ONS_TXN_QUEUED)
        {
            resp_pid = ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE;
        } // if a transaction is queued //
        else
        {
            resp_pid = ONE_NET_ENCODED_SINGLE_DATA_ACK;
        } // else a transaction is not queued //
    } // if successful so far //
	else
	{
		resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
	}


    // build the response(if txn is still non-NULL)
    // TODO - Look at 1.5 and Multi-Hop, as well as Stream and Block.
    // particularly look for statuses like ON_WAIT_FOR_STREAM_DATA_RESP,
    // make sure all is well.  It almost certainly isn't, but look at 1.5
    // to try to get a grip on things.
omsdh_build_resp:
    if(*txn == 0)
	{
		return status;
	}

	// these three checks hopefully aren't necessary, but we'll do them anyway.
	// TODO - verify that they AREN'T necessary and take them out or if they are
	// necessary, see if they should have been caught before here.

	// make sure the client isn't still null
	if(!client)
	{
		*txn = 0;
		return ONS_BAD_PKT;
	}

	// stick a nack reason if it's a nack and we don't have a reason
	if(resp_pid == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN &&
	    nack_reason == ON_NACK_RSN_NO_ERROR)
	{
		nack_reason = ON_NACK_RSN_GENERAL_ERR;
	}

	// if there is a nack reason filled in, make sure we're sending a nack
	if(nack_reason != ON_NACK_RSN_NO_ERROR)
	{
		resp_pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
	}

	// update nonces
	client->last_nonce = client->expected_nonce;
	client->expected_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);

	// now build the response packet
    response_txn.data_len = response_txn.pkt_size;
	*txn = &response_txn;
	status = on_build_response_pkt(response_txn.pkt,
        &(response_txn.data_len), resp_pid,
        resp_pid == ONE_NET_ENCODED_SINGLE_DATA_ACK ? NULL : &nack_reason,
		&ack_nack_handle, &ack_nack_payload, SRC_DID, resp_nonce, client->expected_nonce,
	    key, ON_SINGLE);

    // if this is an ACK, we want to make sure that if for some reason the other device
	// continues to send the message not realizing we've already received and ACK'd it,
	// we don't interpret that repeated message as a NEW instance of the same message.
	// Thus we'll set a timer.  If we see the same message from the same device, we'll
	// ACK it even if it has bad nonces and we won't "handle" it since we already have
	// handled it.  Note: Since we're doing away with the transaction ACK, we can't wait
	// for it.
	if(resp_pid == ONE_NET_ENCODED_SINGLE_DATA_ACK)
	{
	    single_ack_queued(&raw_src_did, &(pld[ON_PLD_DATA_IDX]));
	}

    return status;
} // on_master_single_data_hdlr //


/*!
    \brief Callback for the application to handle an ack or a nack.

    This function is application dependent.  It is called by ONE-NET when a
    MASTER receives an ACK or a NACK.  ONE-NET provides default handling for
	query responses.  In addition, the application code can change a NACK reason or
	change an ACK to a NACK or vice versa.  It can also change the number of retries
	to either prolong the number of retries or decrease them.  The application can
	provide its own handling by returning ONS_CANCELED.  If it wants to abort the transaction
	for whatever reason, either because it has its own handling or because it does not
	want to handle the message at all, it should return ONS_CANCELED.


    \param[in/out] txn The current transaction
	\param[in] src_did the did of the device we received this ACK or NACK from.
	\param[in/out] nack_reason The reason for the NACK.  If this is null or equals ON_NACK_RSN_NO_ERROR
	               then we received an ACK, not a NACK.
	\param[in/out] ack_nack_handle The way the data(if any) provided by the ACk or NACK should be
	               interpreted.
	\param[in/out] ack_nack_payload The data(if any) sent with the ACK/NACK message.

    \return ONS_SUCCESS If ONE-NET should proceed with any further handling of the transaction
            ONS_CANCELED If ONE-NET should consider the transaction complete for whatever reason.
			See one_net_status_t for error codes
*/
one_net_status_t on_master_handle_ack_nack_response(on_txn_t* const txn,
	const one_net_raw_did_t* const src_did, on_nack_rsn_t* const nack_reason,
	on_ack_nack_handle_t* const ack_nack_handle,
	ack_nack_payload_t* const ack_nack_payload)
{
	one_net_status_t rv = ONS_SUCCESS;
	UInt8 i;

	ona_msg_type_t msg_type, orig_msg_type;

	#if _ACK_NACK_LEVEL >= 200
	UInt8 raw_pld[ON_RAW_SINGLE_PLD_SIZE]; // payload of 9
	UInt8 orig_raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN]; // payload of 5
	#endif

	BOOL abort_txn, need_txn_payload;

	on_encoded_did_t enc_src_did;
	on_client_t* client;
	one_net_xtea_key_t* key;

	on_encode(enc_src_did, *src_did, ON_ENCODED_DID_LEN);

	// first assume that one_net_master_ack_nack_response does not need the raw payload.  If it does,
	// it will set a flag and we'll re-send with that payload.
    abort_txn = !one_net_master_handle_ack_nack_response(txn->msg_type, NULL, 0,
        &need_txn_payload, &(txn->retry), src_did, nack_reason, ack_nack_handle,
		ack_nack_payload);

	if(abort_txn)
	{
		return ONS_CANCELED;
	}

	#if _ACK_NACK_LEVEL >= 200
	// check the flag
	if(need_txn_payload)
	{
		// we'll need the key and we'll have to extract the 5 bit raw payload
	    client = client_info(&enc_src_did);
	    if(client == 0)
	    {
		    return ONS_DID_FAILED; // bad did.  We should never get this far if the did is bad.
	    }

	    key = (client->use_current_key) ? (&(on_base_param->current_key)) :
                  (&(master_param->old_key));

		// we have the key.  Let's get the raw payload of 9.  Bytes 3 through 7 are
		// what are relevant to the application code.
        if((rv = on_parse_txn_pld(txn, raw_pld, ON_SINGLE, key)) != ONS_SUCCESS)
	    {
		    return rv;
	    }

		// we'll need to know whether the application code changes this, so make a
		// copy so we can compare later
		one_net_memmove(orig_raw_pld, &(raw_pld[ON_PLD_DATA_IDX]), ONA_SINGLE_PACKET_PAYLOAD_LEN);

		// now call the application code and see if it wants to change anything
        abort_txn = !one_net_master_handle_ack_nack_response(txn->msg_type,
		  &(raw_pld[ON_PLD_DATA_IDX]), ONA_SINGLE_PACKET_PAYLOAD_LEN, NULL,
		  &(txn->retry), src_did, nack_reason, ack_nack_handle, ack_nack_payload);

		if(abort_txn)
		{
			return ONS_CANCELED;
		}

		// let's see if it changed the raw payload.  If it did, we need to re-build.
		if(one_net_memcmp(&(raw_pld[ON_PLD_DATA_IDX]), orig_raw_pld, ON_RAW_SINGLE_PLD_SIZE) != 0)
		{
			// the application code changed the raw payload, so we'll re-build.
			// We don't care if we screw up when building the response nonce
			// because if it needs to be re-sent, the other device has already
			// sent us what needs to be used so it'll be overwritten.  We have the
			// same nonce twice in the packet build below, which is wrong, but
			// it's going to get re-built with the right one.  We're only re-building
			// because the raw payload changed.

			// TODO - we're alreaady taking apart and rebuilding this packet here
			// and we may do it again later too.  For efficiency sake, we may
			// be able to do this once instead of twice.  Right now it's a bit
			// inefficient.
            rv = on_build_data_pkt(txn->pkt, &(txn->data_len), txn->msg_type,
	          ONE_NET_ENCODED_REPEAT_SINGLE_DATA, &enc_src_did, txn->expected_nonce,
              txn->expected_nonce, &(raw_pld[ON_PLD_DATA_IDX]),
			  ONE_NET_RAW_SINGLE_DATA_LEN, key);

			if(rv != ONS_SUCCESS)
			{
				return rv;
			}
		}
	}
	#endif

    return ONS_SUCCESS;
} // on_master_handle_ack_nack_response



/*!
    \brief Handles the response to a sent single data transaction.

    \param[in] txn The transaction whose status is being reported.
    \param[in] NEXT_NONCE ON_MAX_NONCE + 1 if this field is to be ignored,
                otherwise the next nonce for the receiving device.
    \param[in] STATUS The result of the transaction.
    \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
      MASTER

    \return ONS_SUCCESS If handling the transaction was successful
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            ONS_TXN_QUEUED If there is another transaction queued for this
              client.
            ONS_INTERNAL_ERR If the transaction was not handled.
*/
one_net_status_t on_master_single_txn_hdlr(on_txn_t ** txn,
  const UInt8 NEXT_NONCE, const one_net_status_t STATUS)
{
    on_client_t * client;
    one_net_raw_did_t raw_did;
    one_net_status_t rv = ONS_INTERNAL_ERR;

    UInt8 txn_nonce;
    UInt8 msg_type;
    UInt8 pld[ON_RAW_SINGLE_PLD_SIZE];

    one_net_status_t parameter_status;

	tick_t send_time_from_now;
	tick_t expire_time_from_now = 0;


    if(!txn || !*txn || !(*txn)->pkt)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(cur_txn_info >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_INTERNAL_ERR;
    } // if an internal error occured //

    // use the dst address since txn has the single data packet sent to the
    // device (it is not the packet just received).
    if((rv = on_decode(raw_did, &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
      ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
    {
        return rv;
    } // if decoding the CLIENT did was not successful //

    if((client = client_info((const on_encoded_did_t * const)
      &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]))) == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // if the CLIENT could not be found //

    if(NEXT_NONCE <= ON_MAX_NONCE)
    {
        client->send_nonce = NEXT_NONCE;
    } // if the next nonce is valid //

    if((rv = on_parse_txn_pld(*txn, pld, ON_SINGLE, client->use_current_key
      ? (const one_net_xtea_key_t * const)&(on_base_param->current_key)
      : (const one_net_xtea_key_t * const)&(master_param->old_key)))
      != ONS_SUCCESS)
	{
		return rv;
	}
    // if parsing the payload is not successful //

	msg_type = (*txn)->msg_type;

    switch(STATUS)
    {
        case ONS_SUCCESS:
        case ONS_RX_STAY_AWAKE:
        case ONS_SINGLE_FAIL:
		case ONS_SINGLE_CANCELED:
        {
			parameter_status = STATUS;
			if(STATUS == ONS_RX_STAY_AWAKE)
			{
				parameter_status = ONS_SUCCESS;
			}

            if(msg_type == ON_APP_MSG)
            {
                one_net_master_single_txn_status(STATUS, (*txn)->retry,
                  &(pld[ON_PLD_DATA_IDX]),
                  (const one_net_raw_did_t * const)&raw_did);
            } // if an application message //
            else if(msg_type == ON_ADMIN_MSG)
            {
                rv = admin_txn_hdlr(pld[ON_PLD_DATA_IDX + ON_ADMIN_MSG_ID_IDX],
                  &(pld[ON_PLD_DATA_IDX + ON_ADMIN_DATA_IDX]), txn,
                  parameter_status, client,
                  (const one_net_raw_did_t * const)&raw_did);
            } // else if an admin message //
			#ifdef _STREAM_MESSAGES_ENABLED
            else if(msg_type == ON_EXTENDED_ADMIN_MSG)
            {
                rv = extended_admin_single_txn_hdlr(
                  pld[ON_PLD_DATA_IDX + ON_ADMIN_MSG_ID_IDX],
                  &(pld[ON_PLD_DATA_IDX + ON_ADMIN_DATA_IDX]), txn,
                  parameter_status, client,
                  (const one_net_raw_did_t * const)&raw_did);
            } // else if an extended admin message //
			#endif
            else
            {
                rv = ONS_INVALID_DATA;
                break;
            } // else the data is invalid //

            // The pid may have changed to ONE_NET_ENCODED_DATA_RATE_TEST in the
            // admin_txn_hdlr function if the MASTER is supposed to send a data
            // rate test, in which case, don't send any queued packets since the
            // CLIENT is listening for the data rate test.
            if(rv == ONS_SUCCESS && STATUS == ONS_SUCCESS
              && (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
              != ONE_NET_ENCODED_DATA_RATE_TEST)
            {
                // check to see if there is another packet queued for this
                // device (with priority == to the highest queued priority)
                if(next_txn_queued_for_client((const on_encoded_did_t * const)
                  &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]))
                  == TRUE)
                {
                    rv = ONS_TXN_QUEUED;
                } // if there is another transaction for the CLIENT //
            } // if txn was successful & data rate test not being sent //
            break;
        } // ONS_SUCCESS, ONS_RX_STAY_AWAKE, ONS_SINGLE_FAIL, ONS_SINGLE_CANCELED cases //

        #if _ACK_NACK_LEVEL >= 225
        case ONS_RETRY:
		    (*txn)->retry = 0;  // do we really want to reset this to 0?
			send_time_from_now = (*txn)->time;
			(*txn)->time = 0;

			on_state = ON_SEND_SINGLE_DATA_PKT;

			if(send_time_from_now <= ONE_NET_RESPONSE_TIME_OUT)
			{
				// keep this transaction, set the timer, and fall through to the case
				// below to build the packet
				ont_set_timer(ONT_GENERAL_TIMER, send_time_from_now);
				// TODO - don't we need to rebuild the transaction with the new nonce here?
				// Maybe don't bother since there's a delay and the nonces may change?
				// Come back tho this and make sure we're doing the same in the client.
			}
			else
			{
				// delay time is too long.  Cancel this transaction and add the
				// payload to the single queue.  Don't send it through peer?

				// we need to set a few flags here too.  They may be set elsewhere
				// and overridden or they may not.  We'll set them again anyway.
				(*txn)->priority = ONE_NET_NO_PRIORITY;
				ont_set_timer(ONT_GENERAL_TIMER, 0);
				on_state = ON_LISTEN_FOR_DATA;

                if(ONS_SUCCESS != one_net_master_send_single(&(pld[ON_PLD_DATA_IDX]),
			         ONE_NET_RAW_SINGLE_DATA_LEN, 0, ONE_NET_HIGH_PRIORITY,
                     &raw_did, ONE_NET_DEV_UNIT, &send_time_from_now,
				     &expire_time_from_now))
			    {
					// OK, we've failed.  Just make this a failure and abort
					return on_master_single_txn_hdlr(txn, NEXT_NONCE, ONS_SINGLE_FAIL);
			    }
				else
				{
					// New request is in the queue for later.  Cancel the current one.
					return on_master_single_txn_hdlr(txn, NEXT_NONCE, ONS_SINGLE_CANCELED);
				}
			}
		#endif
        case ONS_RX_NACK: case ONS_REBUILD_PKT:
        {
            // need to rebuild the packet, so take apart the current one to
            // rebuild with differect nonces
            on_encoded_did_t dst;

            // rebuild the packet
            one_net_memmove(dst, &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
              sizeof(dst));
            (*txn)->expected_nonce = one_net_prand(one_net_tick(),
              ON_MAX_NONCE);
            (*txn)->data_len = (*txn)->pkt_size;

            rv = on_build_data_pkt((*txn)->pkt, &((*txn)->data_len), (*txn)->msg_type,
              ONE_NET_ENCODED_REPEAT_SINGLE_DATA,
              (const on_encoded_did_t * const)&dst, NEXT_NONCE,
              (*txn)->expected_nonce, &(pld[ON_PLD_DATA_IDX]),
              ONE_NET_RAW_SINGLE_DATA_LEN, client->use_current_key
              ? (const one_net_xtea_key_t * const)&(on_base_param->current_key)
              : (const one_net_xtea_key_t * const)&(master_param->old_key));
            break;
        } // ONS_RX_NACK, and ONS_REBUILD_PKT cases //

        default:
        {
            rv = ONS_INTERNAL_ERR;
            break;
        } // default case //
    } // switch(STATUS) //

    return rv;
} // on_master_single_txn_hdlr //


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Interface to start a block transaction.

    Starts a block or stream transaction (send or receive) if the resources are
    available.

    \param[in] TYPE The type of transaction to be requested.  This can be either
                    ON_BLOCK or ON_STREAM.
    \param[in] MSG_TYPE The message type (msg_type_t).  This can be
      ON_ADMIN_MSG, or ON_EXTENDED_ADMIN_MSG.
    \param[in] SEND TRUE if this device is sending the data.
                    FALSE if this device is requesting to receive the data.
    \param[in] DATA_TYPE The type of data to transfer.
    \param[in] LEN The total number of bytes to transfer (if a block txn).
    \param[in] DID The device id of the other device involved in the transaction
    \param[in] SRC_UNIT The unit sending the message.
    \param[in] DST_UNIT The unit receiving the message.

    \return ONS_SUCCESS If there are resources available and the request will be
                    made.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_RSRC_FULL If the resources are unavailable.
            ONS_BAD_ADDR If DID does not point to an address of a device in the
                    network.
            ONS_ALREADY_IN_PROGRESS If a transaction of this type with the
                    device is already in progress.
*/
static one_net_status_t b_s_request(const UInt8 TXN_TYPE, const UInt8 MSG_TYPE,
  const BOOL SEND, const UInt8 DATA_TYPE, const UInt16 LEN,
  const UInt8 PRIORITY, const on_encoded_did_t * const DID,
  const UInt8 SRC_UNIT, UInt8 DST_UNIT)
{
    on_nack_rsn_t nack_reason = ON_NACK_RSN_NO_ERROR;
    on_ack_nack_handle_t ack_nack_handle;
    ack_nack_payload_t ack_nack_payload;

    #ifdef _STREAM_MESSAGES_ENABLED
    const UInt8 SEND_LOW_TYPE = TXN_TYPE == ON_BLOCK ? ON_SEND_BLOCK_LOW
      : ON_SEND_STREAM_LOW;
    const UInt8 RECV_LOW_TYPE = TXN_TYPE == ON_BLOCK ? ON_RECV_BLOCK_LOW
      : ON_RECV_STREAM_LOW;
    const UInt8 SEND_HIGH_TYPE = TXN_TYPE == ON_BLOCK ? ON_SEND_BLOCK_HIGH
      : ON_SEND_STREAM_HIGH;
    const UInt8 RECV_HIGH_TYPE = TXN_TYPE == ON_BLOCK ? ON_RECV_BLOCK_HIGH
      : ON_RECV_STREAM_HIGH;
	#endif

    one_net_status_t status;
    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];
    UInt8 txn_type;

	#ifdef _STREAM_MESSAGES_ENABLED
    if((TXN_TYPE != ON_BLOCK && TXN_TYPE != ON_STREAM) || !LEN
      || (MSG_TYPE != ON_ADMIN_MSG && MSG_TYPE != ON_EXTENDED_ADMIN_MSG)
	#else
    if((TXN_TYPE != ON_BLOCK) || !LEN  || (MSG_TYPE != ON_ADMIN_MSG)
	#endif
      || PRIORITY < ONE_NET_LOW_PRIORITY || PRIORITY > ONE_NET_HIGH_PRIORITY
      || !DID)
    {
        return ONS_BAD_PARAM;
    } // if any parameters are invalid //

    if(b_s_in_progress(TXN_TYPE, DID))
    {
        return ONS_ALREADY_IN_PROGRESS;
    } // if the transaction is already in progress with DID //

    if(MSG_TYPE == ON_BLOCK && SEND)
    {
        if(!one_net_master_txn_requested(ON_BLOCK, TRUE,
          DATA_TYPE, LEN, DID, SRC_UNIT, DST_UNIT,
          &nack_reason, &ack_nack_handle, &ack_nack_payload) || nack_reason)
        {
            return ONS_RSRC_FULL;
        }
    }

    pld[ON_BLOCK_STREAM_SRC_DST_IDX] =
      ((SRC_UNIT << ONA_BLK_DATA_HDR_SRC_UNIT_SHIFT) &
        ONA_BLK_DATA_HDR_SRC_UNIT_MASK) + (DST_UNIT &
        ONA_BLK_DATA_HDR_DST_UNIT_MASK);
    pld[ON_BLOCK_STREAM_DATA_TYPE_IDX] = DATA_TYPE;
    one_net_int16_to_byte_stream(LEN, &(pld[ON_BLOCK_LEN_IDX]));

    // if this device is sending, then the other device is receiving (that is
    // why the admin types are reversed from what might be expected).
	#ifdef _STREAM_MESSAGES_ENABLED
    if(SEND)
    {
        if(PRIORITY == ONE_NET_HIGH_PRIORITY)
        {
            txn_type = RECV_HIGH_TYPE;
        } // if high priority transaction
        else
        {
            txn_type = RECV_LOW_TYPE;
        } // else low priority txn //
    } // if sending //
    else
    {
        if(PRIORITY == ONE_NET_HIGH_PRIORITY)
        {
            txn_type = SEND_HIGH_TYPE;
        } // if high priority transaction
        else
        {
            txn_type = SEND_LOW_TYPE;
        } // else low priority txn //
    } // else receiving //
	#else
    if(SEND)
    {
        if(PRIORITY == ONE_NET_HIGH_PRIORITY)
        {
            txn_type = ON_RECV_BLOCK_HIGH;
        } // if high priority transaction
        else
        {
            txn_type = ON_RECV_BLOCK_LOW;
        } // else low priority txn //
    } // if sending //
    else
    {
        if(PRIORITY == ONE_NET_HIGH_PRIORITY)
        {
            txn_type = ON_SEND_BLOCK_HIGH;
        } // if high priority transaction
        else
        {
            txn_type = ON_SEND_BLOCK_LOW;
        } // else low priority txn //
    } // else receiving //
	#endif

    // SEND represents whether this device should send or receive.  SEND_TYPE
    // & RECV_TYPE represent what the other device should do, meaning if this
    // device is sending the txn, then RECV_TYPE is sent to the other device
    // since it will be receiving the transaction.
    return send_admin_pkt(txn_type, MSG_TYPE, DID, PRIORITY, pld, sizeof(pld));
} // b_s_request //


/*!
    \brief handles receiving a block or stream data packet

    \param[in] PID The received packet id.
    \param[in] SRC_DID The device ID of the sender.
    \param[in] pld The data that was sent.
    \param[out] txn If responding, the response transaction to send, 0
      otherwise.
    \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
      MASTER

    \return ONS_SUCCESS if handling the packet was successful
            ONS_BAD_PARAM If any of the parameters are not valid.
            ONS_TXN_DOES_NOT_EXIST a transaction with the sending device does
              not exist.
*/
one_net_status_t on_master_b_s_data_hdlr(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, UInt8 * const pld, on_txn_t ** txn)
{
    const one_net_xtea_key_t * KEY = 0;
	on_nack_rsn_t nack_reason;
    on_ack_nack_handle_t ack_nack_handle;
    ack_nack_payload_t ack_nack_payload;

    one_net_raw_did_t raw_src_did;
    one_net_status_t status;
    on_client_t * client = 0;
    UInt8 type, txn_nonce, resp_nonce, msg_type, pid;
    UInt16 byte_position_rcvd = 0;
    UInt16 num_bytes_rcvd = 0;

	UInt16 LEN;

    if(!SRC_DID || !pld || !txn)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are not valid //

    *txn = 0;

    // If the packet is not a Multi-Hop packet, then HOPS_TAKEN must == 0
    if(PID == ONE_NET_ENCODED_BLOCK_DATA
      || PID == ONE_NET_ENCODED_REPEAT_BLOCK_DATA)
    {
        type = ON_BLOCK;
        KEY = &(on_base_param->current_key);
    } // if a block transaction //
	#ifdef _STREAM_MESSAGES_ENABLED
    else if(PID == ONE_NET_ENCODED_STREAM_DATA)
    {
        type = ON_STREAM;
        KEY = &(on_base_param->stream_key);
    } // else if a stream transaction //
	#endif
    else
    {
        return ONS_BAD_PARAM;
    } // else the parameter is invalid //

    // decode the address
    if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
      != ONS_SUCCESS)
    {
        return ONS_INCORRECT_ADDR;
    } // if decoding the address was successful //

    if((client = client_info(SRC_DID)) == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // if not able to get sender info //

    if((cur_txn_info = dequeue_b_s_txn(type, SRC_DID))
      >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_TXN_DOES_NOT_EXIST;
    } // if the transaction was not found //

    status = on_parse_pld(&txn_nonce, &resp_nonce, &msg_type, pld, type,
      type != ON_BLOCK || client->use_current_key
      ? KEY : (const one_net_xtea_key_t * const)&(master_param->old_key));

    if(status == ONS_SUCCESS && txn_nonce == client->expected_nonce)
    {
		#ifdef _STREAM_MESSAGES_ENABLED
        if(type == ON_STREAM && !txn_list[cur_txn_info].txn.remaining)
        {
            // send 0's since field is not used
            UInt8 data[ONE_NET_RAW_SINGLE_DATA_LEN] = {0};

            if((txn_nonce = txn_nonce_for_client(
              (const on_encoded_did_t * const)&(txn_list[cur_txn_info].did)))
              > ON_MAX_NONCE)
            {
                return ONS_INVALID_DATA;
            } // if the nonce is not valid //

            data[0] = ON_END_STREAM;
            response_txn.data_len = response_txn.pkt_size;
            response_txn.expected_nonce = one_net_prand(one_net_tick(),
              ON_MAX_NONCE);

            KEY = (client->use_current_key
              ? (const one_net_xtea_key_t * const)&(on_base_param->current_key)
              : (const one_net_xtea_key_t * const)&(master_param->old_key));

            if((status = on_build_data_pkt(response_txn.pkt,
              &(response_txn.data_len), ON_ADMIN_MSG,
              ONE_NET_ENCODED_SINGLE_DATA,
              (const on_encoded_did_t * const)&(txn_list[cur_txn_info].did),
              txn_nonce, response_txn.expected_nonce, data, sizeof(data),
              KEY)) == ONS_SUCCESS)
            {
                // using the response transaction to send a single data pkt
                // since we know that it has the necessary space & is available
                status = ONS_STREAM_END;
            } // if building the end stream packet was successful //
        } // if the stream is marked to end //
        else if(type == ON_BLOCK && !txn_list[cur_txn_info].txn.remaining)
		#else
        if(type == ON_BLOCK && !txn_list[cur_txn_info].txn.remaining)
		#endif
        {
            // Received new data for a block transaction that should have ended.
            // Something went wrong, so fail the transaction
            status = ONS_BLOCK_FAIL;
            one_net_master_block_txn_status(status,
              (const one_net_raw_did_t * const)&raw_src_did);
            free_txn(cur_txn_info);
            return status;
        } // if block transaction that should have ended //

        else if(type == ON_BLOCK && !one_net_master_handle_block_pkt(
          num_bytes_rcvd, (const one_net_raw_did_t * const)&raw_src_did,
          &nack_reason, &ack_nack_handle, &ack_nack_payload))
        {
            status = ONS_UNHANDLED_PKT;
        } // else if a block packet that is not being handled //
		#ifdef _STREAM_MESSAGES_ENABLED
        else if(type == ON_STREAM
          && !one_net_master_handle_stream_pkt(&(pld[ON_PLD_DATA_IDX]),
          ONE_NET_RAW_BLOCK_STREAM_DATA_LEN,
          (const one_net_raw_did_t * const)&raw_src_did))
        {
            status = ONS_UNHANDLED_PKT;
        } // if a stream packet and it is not being handled //
		#endif
        else
        {
			#ifdef _STREAM_MESSAGES_ENABLED
            if(type == ON_BLOCK)
            {
                pid = ONE_NET_ENCODED_BLOCK_DATA_ACK;
            } // if it's a block transaction //
            else
            {
               pid = ONE_NET_ENCODED_STREAM_KEEP_ALIVE;
            } // else it's a stream transaction;
			#else
			pid = ONE_NET_ENCODED_BLOCK_DATA_ACK;
			#endif

            client->last_nonce = client->expected_nonce;

            // loop to make sure the expected nonce and last nonce don't match
            do
            {
                client->expected_nonce = one_net_prand(one_net_tick(),
                  ON_MAX_NONCE);
            } while(client->expected_nonce == client->last_nonce);

            response_txn.data_len = response_txn.pkt_size;

            status = on_build_response_pkt(response_txn.pkt,
              &(response_txn.data_len), pid, NULL, NULL, NULL, SRC_DID, resp_nonce,
              client->expected_nonce, KEY, type);

            txn_list[cur_txn_info].txn.retry = 0;
        } // else the packet was handled //
    } // if it's a new packet //
    else if(type == ON_BLOCK && status == ONS_SUCCESS)
    {
        txn_list[cur_txn_info].txn.retry++;
        if(txn_list[cur_txn_info].txn.retry >= ON_MAX_RETRY)
        {
            status = ONS_BLOCK_FAIL;
            one_net_master_block_txn_status(status,
              (const one_net_raw_did_t * const)&raw_src_did);
            free_txn(cur_txn_info);
            return status;
        } // if the transaction failed //

        response_txn.data_len = response_txn.pkt_size;

        if(PID == ONE_NET_ENCODED_REPEAT_BLOCK_DATA
          && txn_nonce == client->last_nonce)
        {
            status = on_build_response_pkt(response_txn.pkt,
              &(response_txn.data_len), ONE_NET_ENCODED_BLOCK_DATA_ACK,
              NULL, NULL, NULL, SRC_DID, resp_nonce, client->expected_nonce,
			  KEY, ON_BLOCK);
        } // if it is a repeat packet //
    } // else if the block packet was successfully parsed //
	#ifdef _STREAM_MESSAGES_ENABLED
    else if(status == ONS_SUCCESS)
    {
        if(!txn_list[cur_txn_info].txn.remaining)
        {
            // send 0's since field is not used
            UInt8 data[ONE_NET_RAW_SINGLE_DATA_LEN] = {0};

            if((txn_nonce = txn_nonce_for_client(
              (const on_encoded_did_t * const)&(txn_list[cur_txn_info].did)))
              > ON_MAX_NONCE)
            {
                return ONS_INVALID_DATA;
            } // if the nonce is not valid //

            data[0] = ON_END_STREAM;
            response_txn.data_len = response_txn.pkt_size;
            response_txn.expected_nonce = one_net_prand(one_net_tick(),
              ON_MAX_NONCE);

            KEY = (client->use_current_key
              ? (const one_net_xtea_key_t * const)&(on_base_param->current_key)
              : (const one_net_xtea_key_t * const)&(master_param->old_key));

            if((status = on_build_data_pkt(response_txn.pkt,
              &(response_txn.data_len), ON_ADMIN_MSG, ONE_NET_ENCODED_SINGLE_DATA,
              (const on_encoded_did_t * const)&(txn_list[cur_txn_info].did),
              txn_nonce, response_txn.expected_nonce, data, sizeof(data),
              KEY)) == ONS_SUCCESS)
            {
                // using the response transaction to send a single data pkt
                // since we know that it has the necessary space & is available
                status = ONS_STREAM_END;
                *txn = &response_txn;
            } // if building the end stream packet was successful //
        } // if the transaction has been marked to end //
        else
        {
            // It's possible the ONE_NET_ENCODED_STREAM_KEEP_ALIVE did not make
            // it back, so check the last nonce.  Send the
            // ONE_NET_ENCODED_STREAM_KEEP_ALIVE even if the last nonce does not
            // match since it is possible that several
            // ONE_NET_ENCODED_STREAM_KEEP_ALIVE packets in a row have been lost.
            // Only do this the max number of retries before failing the
            // transaction.
            txn_list[cur_txn_info].txn.retry++;
            if(txn_list[cur_txn_info].txn.retry >= ON_MAX_RETRY)
            {
                status = ONS_STREAM_FAIL;
                one_net_master_stream_txn_status(status,
                  (const one_net_raw_did_t * const)&raw_src_did);
                free_txn(cur_txn_info);
                return status;
            } // if the transaction failed //

            status = on_build_response_pkt(response_txn.pkt,
              &(response_txn.data_len), ONE_NET_ENCODED_STREAM_KEEP_ALIVE,
	          NULL, NULL, NULL, SRC_DID, resp_nonce, client->expected_nonce, KEY,
			  ON_STREAM);
        } // else process as normal //
    } // else if the stream packet was successfully parsed //
	#endif

    #ifdef _STREAM_MESSAGES_ENABLED
    if(status == ONS_SUCCESS || status == ONS_STREAM_END)
    #else
    if(status == ONS_SUCCESS)
	#endif
    {
        *txn = &response_txn;

        on_update_next_txn_time(&(txn_list[cur_txn_info].txn));

        if(txn_list[cur_txn_info].type == ON_BLOCK
          && !txn_list[cur_txn_info].txn.retry)
        {
            if(txn_list[cur_txn_info].txn.remaining
              <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
            {
                txn_list[cur_txn_info].txn.remaining = 0;
                status = ONS_BLOCK_END;
            } // if the block transaction is complete //
            else
            {
                txn_list[cur_txn_info].txn.remaining
                  -= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
            } // else more blocks remain //
        } // if block transaction //
    } // if building the response was successful //

    if(!client->use_current_key && (status == ONS_SUCCESS
    #ifdef _STREAM_MESSAGES_ENABLED
      || status == ONS_BLOCK_END || status == ONS_STREAM_END))
	#else
      || status == ONS_BLOCK_END))
	#endif
    {
        send_key_update(SRC_DID);
    } // if a key update should be sent //

    enqueue_txn(cur_txn_info);
	#ifdef _STREAM_MESSAGES_ENABLED
    if(status != ONS_BLOCK_END && status != ONS_STREAM_END)
	#else
    if(status != ONS_BLOCK_END)
	#endif
    {
        // no longer current txn
        cur_txn_info = ONE_NET_MASTER_MAX_TXN;
    } // if the transactin has not completed //

    return status;
} // on_master_b_s_data_hdlr //


/*!
    \brief Handles the status of a block or stream transaction.

    This is called whenever there is a status event for a block or stream
    transaction (sending or receiving).

    \param[in] txn The transaction whose status is being reported.
    \param[in] NEXT_NONCE ON_MAX_NONCE + 1 if this field is to be ignored,
                otherwise the next nonce for the receiving device.
    \param[in] STATUS The result of the transaction.
    \param[in] HOPS_TAKEN The number of hops it took the packet to reach the
      MASTER

    \return ONS_SUCCESS If handling the transaction was successful
            ONS_BAD_PARAM if any of the parameters are invalid.
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            ONS_BLOCK_END If the block transaction is complete.
            ONS_INTERNAL_ERR If the transaction was not handled.
*/
one_net_status_t on_master_b_s_txn_hdlr(on_txn_t ** txn, const UInt8 NEXT_NONCE,
  const one_net_status_t STATUS, const UInt8 HOPS_TAKEN)
{
    #ifdef _STREAM_MESSAGES_ENABLED
    const BOOL SEND_TXN = (STATUS == ONS_SUCCESS || STATUS == ONS_TIME_OUT
      || STATUS == ONS_RX_NACK || STATUS == ONS_BLOCK_FAIL
      || STATUS == ONS_STREAM_FAIL);
    #else
    const BOOL SEND_TXN = (STATUS == ONS_SUCCESS || STATUS == ONS_TIME_OUT
      || STATUS == ONS_RX_NACK || STATUS == ONS_BLOCK_FAIL);
    #endif

    on_client_t * client;
    one_net_raw_did_t raw_did;
    one_net_status_t rv = ONS_INTERNAL_ERR;
    const one_net_xtea_key_t* KEY = 0;

    if(SEND_TXN && (!txn || !*txn || !(*txn)->pkt))
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(cur_txn_info >= ONE_NET_MASTER_MAX_TXN || (SEND_TXN
      && (!txn_list[cur_txn_info].txn.send
      || *txn != &(txn_list[cur_txn_info].txn)))
      || (!SEND_TXN && txn_list[cur_txn_info].txn.send))
    {
        return ONS_INTERNAL_ERR;
    } // if the txn not correct //

    // use the dst address since txn has the single data packet sent to the
    // device (it is not the packet just received).
    if((rv = on_decode(raw_did, txn_list[cur_txn_info].did, ON_ENCODED_DID_LEN))
      != ONS_SUCCESS)
    {
        return rv;
    } // if decoding the CLIENT did was not successful //

    if((client = client_info((const on_encoded_did_t * const)
      &(txn_list[cur_txn_info].did))) == 0)
    {
        return ONS_INCORRECT_ADDR;
    } // if the CLIENT could not be found //

    if(NEXT_NONCE <= ON_MAX_NONCE)
    {
        client->send_nonce = NEXT_NONCE;
    } // if the next nonce is not valid //

    switch(STATUS)
    {
        case ONS_SUCCESS:
        {
            (*txn)->retry = 0;
            if(txn_list[cur_txn_info].type == ON_BLOCK
              && txn_list[cur_txn_info].txn.remaining == 0)
            {
                if((*txn)->msg_type == ON_APP_MSG)
                {
                    one_net_master_block_txn_status(ONS_BLOCK_END,
                      (const one_net_raw_did_t * const)&raw_did);
                } // if an app message //
				#ifdef _STREAM_MESSAGES_ENABLED
                else if((*txn)->msg_type == ON_EXTENDED_ADMIN_MSG)
                {
                    // Only have 1 exteneded admin message, so handle it here
                    stream_key_update_confirmed(client);
                } // else if an extended admin message //
				#endif
                else
                {
                    one_net_master_block_txn_status(ONS_INTERNAL_ERR,
                      (const one_net_raw_did_t * const)&raw_did);
                } // else invalid message type //

                return ONS_BLOCK_END;
            } // if that was the last block //

            on_update_next_txn_time(*txn);
            enqueue_txn(cur_txn_info);
            break;
        } // ONS_SUCCESS case //

        case ONS_TIME_OUT:                              // fall through
        case ONS_RX_NACK:
        {
            if(txn_list[cur_txn_info].type == ON_BLOCK)
            {
                // Need to resend the message
                UInt8 txn_nonce;
                UInt8 msg_type;
                UInt8 pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];

                on_decode(pld, &((*txn)->pkt[ON_PLD_IDX]),
                  ON_ENCODED_BLOCK_STREAM_PLD_SIZE);

                KEY = (client->use_current_key ?
				  (const one_net_xtea_key_t * const) &(on_base_param->current_key)
                  : (const one_net_xtea_key_t * const)&(master_param->old_key));

                if((rv = on_parse_pld(&txn_nonce, &((*txn)->expected_nonce),
                  &msg_type, pld, ON_BLOCK, KEY)) != ONS_SUCCESS)
                {
                    return rv;
                } // if parsing the payload is not successful //

                // store only the app part of the message
                (*txn)->data_len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
                one_net_memmove(&((*txn)->pkt[ON_DATA_IDX]),
                  &(pld[ON_PLD_DATA_IDX]), (*txn)->data_len);
            } // if a block transaction //

            on_update_next_txn_time(&(txn_list[cur_txn_info].txn));
            enqueue_txn(cur_txn_info);
            break;
        } // ONS_TIME_OUT / ONS_RX_NACK case //

        case ONS_BLOCK_END:                             // fall through
        case ONS_BLOCK_FAIL:
        {
            if(txn_list[cur_txn_info].type != ON_BLOCK)
            {
                return ONS_INTERNAL_ERR;
            } // if not sending //

            one_net_master_block_txn_status(STATUS,
              (const one_net_raw_did_t * const)&raw_did);
            break;
        } // ONS_BLOCK_END/ONS_BLOCK_FAIL case //

        #ifdef _STREAM_MESSAGES_ENABLED
        case ONS_STREAM_END:                            // fall through
        case ONS_STREAM_FAIL:
        {
            if(txn_list[cur_txn_info].type != ON_STREAM)
            {
                return ONS_INTERNAL_ERR;
            } // if not sending //

            one_net_master_stream_txn_status(STATUS,
              (const one_net_raw_did_t * const)&raw_did);
            break;
        } // ONS_STREAM_END/ONS_STREAM_FAIL case //
		#endif

        default:
        {
            rv = ONS_INTERNAL_ERR;
            break;
        } // default case //
    } // switch(STATUS) //

    return rv;
} // on_master_b_s_txn_hdlr //
#endif


/*!
    \brief Handles the end of the data rate test.

    \param[in] RATE The data rate that was tested.
    \param[in] DID The device the MASTER tested the data rate with.
    \param[in] RESULT The number of successful response the MASTER heard.

    \return void
*/
void on_master_data_rate_hdlr(const UInt8 RATE,
  const on_encoded_did_t * const DID, const UInt8 RESULT)
{
    if(!ont_active(data_rate_test.timer) || !DID || !on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(data_rate_test.recv_did)))
    {
        return;
    } // if not carrying out a data rate test //

    report_data_rate_result(RATE, RESULT, ON_MAX_DATA_RATE_TRIES);
} // on_master_data_rate_hdlr //


/*!
    \brief Sets up the data rate test between 2 devices.

    \param[in] SENDER The device that is to send the data rate test.  Pass in 0
      if the MASTER is to send the data rate test.
    \param[in] RECEIVER The device receiving the data rate test.  This must be
      a CLIENT device.
    \param[in] DATA_RATE The data rate to perform the test at.
    \param[in] MH TRUE if it is a Multi-Hop data rate test.
                  FALSE if it is a normal data rate test (no hops).

    \return ONS_SUCCESS If the data rate test has been set up.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_INCORRECT_ADDR If the address is for a device not on this
              network.
            See encode, send_admin_pkt for more return results.
*/
static one_net_status_t setup_data_rate_test(
  const on_encoded_did_t * const SENDER,
  const on_encoded_did_t * const RECEIVER, const UInt8 DATA_RATE, const BOOL MH)
{
    one_net_status_t status;

    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    if(!SENDER || !RECEIVER || DATA_RATE > ONE_NET_MAX_DATA_RATE)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(ont_active(data_rate_test.timer))
    {
        return ONS_ALREADY_IN_PROGRESS;
    } // if the MASTER is already sending a data rate test //

    one_net_memmove(data_rate_test.recv_did, *RECEIVER,
      sizeof(data_rate_test.recv_did));
    one_net_memmove(data_rate_test.send_did, *SENDER,
      sizeof(data_rate_test.send_did));

    data_rate_test.rate = DATA_RATE;
    pld[ON_DATA_RATE_DATA_RATE_IDX] = DATA_RATE;

    // set up the single txn to set up the receiver of the data rate test
    one_net_memmove(&(pld[ON_DATA_RATE_DID_IDX]), ON_ENCODED_BROADCAST_DID,
      ON_ENCODED_DID_LEN);
    pld[ON_DATA_RATE_FLAG_IDX] = MH ? ON_MH_DATA_RATE_TEST_FLAG : 0x00;

    if((status = send_admin_pkt(ON_INIT_DATA_RATE_TEST, ON_ADMIN_MSG,
      (const on_encoded_did_t * const)&(data_rate_test.recv_did),
      ONE_NET_HIGH_PRIORITY, pld, sizeof(pld))) != ONS_SUCCESS)
    {
        return status;
    } // if sending the receiver admin pkt not successful //

    // need to find txn the initiate recv data rate admin pkt got assigned
    data_rate_test.recv_idx = txn_head;
    data_rate_txn(&(data_rate_test.recv_idx));
    if(data_rate_test.recv_idx >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_INTERNAL_ERR;
    } // if the txn could not be found //

    // set up sender
    if(on_validate_dst_DID(SENDER) != ONS_SUCCESS)
    {
        one_net_memmove(&(pld[ON_DATA_RATE_DID_IDX]), data_rate_test.recv_did,
          ON_ENCODED_DID_LEN);

        if((status = send_admin_pkt(ON_INIT_DATA_RATE_TEST, ON_ADMIN_MSG,
          (const on_encoded_did_t * const)&data_rate_test.send_did,
          ONE_NET_HIGH_PRIORITY, pld, sizeof(pld))) != ONS_SUCCESS)
        {
            return status;
        } // if sending the sender admin pkt not successful //

        // Since the receiver transaction is queued first, start looking at
        // the transaction queued after the receiver.
        data_rate_test.send_idx = txn_list[data_rate_test.recv_idx].next;
        data_rate_txn(&(data_rate_test.send_idx));
        if(data_rate_test.send_idx >= ONE_NET_MASTER_MAX_TXN)
        {
            free_txn(data_rate_test.recv_idx);
            data_rate_test.recv_idx = ONE_NET_MASTER_MAX_TXN;
            return ONS_INTERNAL_ERR;
        } // if the txn could not be found //
    } // if the master is not the sender of the data rate test //
    else
    {
        data_rate_test.send_idx = ONE_NET_MASTER_MAX_TXN;
    } // else the MASTER is the sender //

    data_rate_test.multi_hop = MH;
    ont_set_timer(data_rate_test.timer, ONE_NET_STAY_AWAKE_TIME << 1);
    return ONS_SUCCESS;
} // setup_data_rate_test //


/*!
    \brief Tries a different variation of the data rate test.

    If the data rate test failed, will try sending a Multi-Hop data rate test.
    If that fails, will check if the receiver is a repeater, and if the MASTER
    sends Multi-Hop packets to the sender.  If that is the case, the MASTER will
    try the test again with the roles reversed.

    \note This should only be called if the data rate test failed with no
      successful transactions.

    \param[in] RATE The data rate that was tested.

    \return TRUE if the test was attempted again.
            FALSE If the test was not attempted again.
*/
static BOOL retry_data_rate_test(const UInt8 RATE)
{
    on_encoded_did_t sender, receiver;
    on_client_t * send_client = 0, * recv_client = 0;

    if(!mh_repeater_available || data_rate_test.multi_hop)
    {
        return FALSE;
    } // if no repeater in the network or the variants had been tried //

    if(!(recv_client = client_info((const on_encoded_did_t * const)
      &(data_rate_test.recv_did))) || !(recv_client->features & ON_MH_CAPABLE))
    {
        return FALSE;
    } // if the CLIENT can't be found or doesn't support Multi-Hop //

    if(on_validate_dst_DID((const on_encoded_did_t * const)
      &(data_rate_test.send_did)) != ONS_SUCCESS && (!(send_client
      = client_info((const on_encoded_did_t * const)&(data_rate_test.send_did)))
      || !(send_client->features & ON_MH_CAPABLE)))
    {
        return FALSE;
    } // if the SENDER could not be found or doesn't support Multi-Hop //

    if(!data_rate_test.multi_hop)
    {
        one_net_memmove(sender, data_rate_test.send_did, sizeof(sender));
        one_net_memmove(receiver, data_rate_test.recv_did, sizeof(receiver));
    } // if a Multi-Hop data rate test should be tried //
    else
    {
        return FALSE;
    } // else noting else to try //

    if(setup_data_rate_test((const on_encoded_did_t * const)&sender,
      (const on_encoded_did_t * const)&receiver, RATE,
      !data_rate_test.multi_hop) == ONS_SUCCESS)
    {
        if(data_rate_test.recv_idx == txn_head)
        {
            // need to wait for the CLIENT to timeout listening for the
            // failed data rate test before trying to set up the new test.
            const UInt32 DELAY = ONE_NET_STAY_AWAKE_TIME >> 1;

            ont_set_timer(ONT_GENERAL_TIMER, DELAY);

            // add the ONE_NET_STAY_AWAKE_TIME to the data rate timer since the
            // start of the test is being delayed
            ont_set_timer(data_rate_test.timer,
              ont_get_timer(data_rate_test.timer) + DELAY);
        } // if the data rate test is the only queued txn //

        return TRUE;
    } // if setting up the data rate test was successful //

    return FALSE;
} // retry_data_rate_test //


/*!
    \brief Reports the results of the data rate test to the application.

    \param[in] DATA_RATE The data rate the test was performed at.
    \param[in] RESULT The number of successful data rate test transactions.
    \param[in] ATTEMPTS The number of data rate transactions that were attempted

    \return void
*/
static void report_data_rate_result(const UInt8 DATA_RATE, const UInt8 RESULT,
  const UInt8 ATTEMPTS)
{
    one_net_raw_did_t raw_sender, raw_receiver;

    if(DATA_RATE > ONE_NET_MAX_DATA_RATE)
    {
        return;
    } // if the parameters are invalid //

    ont_stop_timer(data_rate_test.timer);
    if(!RESULT && retry_data_rate_test(DATA_RATE))
    {
        return;
    } // check to see if the test should be tried in a different way //

    if(on_decode(raw_sender, data_rate_test.send_did, ON_ENCODED_DID_LEN)
      != ONS_SUCCESS
      || on_decode(raw_receiver, data_rate_test.recv_did, ON_ENCODED_DID_LEN)
      != ONS_SUCCESS)
    {
        on_decode(raw_sender, ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
        on_decode(raw_receiver, ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);

        one_net_master_data_rate_result(
          (const one_net_raw_did_t * const)&raw_sender,
          (const one_net_raw_did_t * const)&raw_receiver, DATA_RATE, 0, 0);
    } // if decoding an address failed //
    else
    {
        one_net_master_data_rate_result(
          (const one_net_raw_did_t * const)&raw_sender,
          (const one_net_raw_did_t * const)&raw_receiver, DATA_RATE, RESULT,
          ATTEMPTS);
    } // else decoding addresses successful //
} // report_data_rate_result //


/*!
    \brief Handles admin packets.

    \param[in] SRC The sender of the admin packet.
    \param[in] DATA The admin packet.
    \param[in] DATA_LEN The length of the admin packet.
    \param[in/out] client The CLIENT the message is from if the CLIENT is
      already part of the network.  If the CLIENT is not yet part of the
      network and it is a new CLIENT being added to the network, the new CLIENT
      will be returned in client.
	\param[out] nack_reason The "reason" that should be given if a NACK needs to be sent
	\param[out] Filled in if sending data with the ACK/NACK message
	\param[out] ack_nack_payload Filled in if sending data with the ACK/NACK message


    \return ONS_SUCCESS If the packet was successfully parsed.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_INTERNAL_ERR If an internal error occured.
            ONS_UNHANDLED_VER If the version in the packet is not handled by
              this device.
            ONS_RSRC_FULL If the resources to send an appropriate response are
              full.
            ONS_UNHANDLED_PKT If the admin message type is not handled by the
              device or the packet is being ignored.
            ONS_INVALID_DATA If the received data is not valid.
            ONS_INCORRECT_ADDR If a packet was received from an unexpected
              sender.
*/
static one_net_status_t handle_admin_pkt(const on_encoded_did_t * const SRC_DID,
  const UInt8 * const DATA, const UInt8 DATA_LEN, on_client_t ** client,
  on_nack_rsn_t* const nack_reason, on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload)
{
    one_net_status_t status;

    if(!SRC_DID || !DATA || !nack_reason || !ack_nack_handle ||
      !ack_nack_payload || DATA_LEN != ONE_NET_RAW_SINGLE_DATA_LEN || !client
      || (DATA[ON_ADMIN_MSG_ID_IDX] != ON_STATUS_RESP && !(*client)))
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    #ifdef _ONE_NET_EVAL
        oncli_print_admin_msg(ON_ADMIN_MSG, ON_SINGLE,
          DATA[ON_ADMIN_MSG_ID_IDX], &(DATA[ON_ADMIN_DATA_IDX]),
          ON_MAX_ADMIN_PLD_LEN);
    #endif // ifdef ONE_NET_EVAL //

    #ifdef _STREAM_MESSAGES_ENABLED
    if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP
      && DATA[ON_ADMIN_MSG_ID_IDX] != ON_END_STREAM)
    {
        return ONS_BAD_PKT_TYPE;
    } // if waiting for a stream data response, & it was not received //
	#endif

    switch(DATA[ON_ADMIN_MSG_ID_IDX])
    {
        case ON_STATUS_RESP:
        {
            // Make sure the version is something the MASTER handles
            if(DATA[ON_STATUS_VER_IDX + ON_ADMIN_DATA_IDX] != ON_VERSION)
            {
                status = ONS_UNHANDLED_VER;
                break;
            } // if the version did not match //

            if(!(*client))
            {
                one_net_raw_did_t raw_src_did;

                // The raw source address to be used as an index
                UInt16 src;

                if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
                  != ONS_SUCCESS)
                {
                    break;
                } // if decoding the did failed //

                src = one_net_byte_stream_to_int16(raw_src_did);

                if(invite_idx >= ONE_NET_MASTER_MAX_TXN
                  || src != master_param->next_client_did)
                {
                    status = ONS_INCORRECT_ADDR;
                    break;
                } // if not inviting a CLIENT //

                // the device was added

                *client = one_net_master_add_new_client_to_list();

                if(!(*client))
                {
                    status = ONS_INTERNAL_ERR;
                    break;
                } // if the pointer is 0 //

                one_net_master_invite_result(ONS_SUCCESS, invite_key,
                  &raw_src_did);
                one_net_master_cancel_invite(
                  (const one_net_xtea_key_t * const)&invite_key);

                // the step below isn't necessary because it was already handled
                // in one_net_master_add_new_client_to_list().  Commenting out.
                /*one_net_memmove((*client)->did, *SRC_DID,
                  sizeof(on_encoded_did_t));*/

                (*client)->expected_nonce = one_net_prand(one_net_tick(),
                  ON_MAX_NONCE);
                (*client)->last_nonce = ON_MAX_NONCE + 1;
                (*client)->send_nonce = 0;
                (*client)->data_rate = on_base_param->data_rate;
                (*client)->use_current_key = TRUE;

                // save the new client
                save = TRUE;
            } // if the client was not found in the list //

            (*client)->max_data_rate
              = DATA[ON_STATUS_MAX_DATA_RATE_IDX + ON_ADMIN_DATA_IDX];
            (*client)->features = DATA[ON_STATUS_FEATURES_IDX
              + ON_ADMIN_DATA_IDX];

            if(((*client)->features & ON_MH_CAPABLE)
              && ((*client)->features & ON_MH_REPEATER))
            {
                mh_repeater_available = TRUE;
            } // if the CLIENT is a Multi-Hop Repeater //

            status = ONS_SUCCESS;
            break;
        } // status response case //

        case ON_SETTINGS_RESP:
        {
            if((*client)->data_rate
              != DATA[ON_SETTINGS_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]
              || (*client)->flags != DATA[ON_SETTINGS_FLAG_IDX
              + ON_ADMIN_DATA_IDX])
            {
                (*client)->data_rate
                  = DATA[ON_SETTINGS_DATA_RATE_IDX + ON_ADMIN_DATA_IDX];
                (*client)->flags = DATA[ON_SETTINGS_FLAG_IDX
                  + ON_ADMIN_DATA_IDX];
                save = TRUE;
            } // if the settings have changed //

            if(DATA[ON_SETTINGS_MASTER_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]
              != on_base_param->data_rate)
            {
                if((status = send_change_settings(
                  SRC_DID, (*client)->data_rate,
                  DATA[ON_SETTINGS_FLAG_IDX + ON_ADMIN_DATA_IDX]))
                  == ONS_SUCCESS)
                {
                    status = ONS_TXN_QUEUED;
                } // if sending the change was successful //
            } // if data rate CLIENT thinks the MASTER is at is not correct //
            else
            {
                status = ONS_SUCCESS;
            } // else everything is ok //
            break;
        } // settings response case //

        case ON_KEEP_ALIVE_RESP:
        {
            // do nothing for now
            status = ONS_SUCCESS;
            break;
        } // keep alive response case //

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_FRAGMENT_DELAY_RESP:
        {
            // do nothing for now
            status = ONS_SUCCESS;
            break;
        } // fragment delay response case //

        case ON_SEND_BLOCK_LOW:                         // fall through
        case ON_RECV_BLOCK_LOW:                         // fall through
        case ON_SEND_BLOCK_HIGH:                        // fall through
        case ON_RECV_BLOCK_HIGH:
        {
            const BOOL SEND = (DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_BLOCK_LOW
              || DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_BLOCK_HIGH);

            one_net_raw_did_t raw_src_did;
            UInt8 data_type, src_dst_unit, src_unit, dst_unit;
            UInt8 txn;

            if(b_s_in_progress(ON_BLOCK, SRC_DID))
            {
                status = ONS_ALREADY_IN_PROGRESS;
                break;
            } // if the transaction is already in progress //

            if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
              != ONS_SUCCESS)
            {
                break;
            } // if decoding the did failed //

            // getting the resource, and checking with the app is a catch 22.
            // If the rsrc is checked first, and the app does not want to
            // honor the txn, then checking the rsrc was a waste.  If the
            // app was checked, and the rsrc is unavailable, then the app
            // needs to be notified, and checking with the app in the first
            // place was a waste.  Since we won't have to notify the app if the
            // resource is not available, check the resource first.
            txn = get_free_txn(ON_BLOCK, SEND);

            if(txn >= ONE_NET_MASTER_MAX_TXN)
            {
                status = ONS_RSRC_FULL;
                break;
            } // if the resource is not available //

            txn_list[txn].type = ON_BLOCK;
            txn_list[txn].txn.remaining = one_net_byte_stream_to_int16(
              &(DATA[ON_BLOCK_LEN_IDX + ON_ADMIN_DATA_IDX]));
            one_net_memmove(txn_list[txn].did, *SRC_DID, sizeof(*SRC_DID));
            txn_list[txn].txn.send = SEND;
            txn_list[txn].txn.priority =
              (DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_BLOCK_HIGH
              || DATA[ON_ADMIN_MSG_ID_IDX] == ON_RECV_BLOCK_HIGH)
              ? ONE_NET_HIGH_PRIORITY : ONE_NET_LOW_PRIORITY;

            src_dst_unit = DATA[ON_BLOCK_STREAM_SRC_DST_IDX +
              ON_ADMIN_DATA_IDX];
            src_unit = (src_dst_unit & ONA_BLK_DATA_HDR_SRC_UNIT_MASK) >>
              ONA_BLK_DATA_HDR_SRC_UNIT_SHIFT;
            dst_unit = (src_dst_unit & ONA_BLK_DATA_HDR_DST_UNIT_MASK);

            data_type = DATA[ON_BLOCK_STREAM_DATA_TYPE_IDX + ON_ADMIN_DATA_IDX];

            if(one_net_master_txn_requested(ON_BLOCK, txn_list[txn].txn.send,
              data_type, txn_list[txn].txn.remaining,
              (const one_net_raw_did_t * const)&raw_src_did, src_unit, dst_unit,
              nack_reason, ack_nack_handle, ack_nack_payload))
            {
                on_update_next_txn_time(&(txn_list[txn].txn));
                enqueue_txn(txn);
                status = ONS_SUCCESS;
            } // if request is granted //
            else
            {
                free_txn(txn);
                status = ONS_UNHANDLED_PKT;
            } // else request is not granted //
            break;
        } // SEND_BLOCK, RECV_BLOCK case //
		#endif

        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_SEND_STREAM_LOW:                        // fall through
        case ON_RECV_STREAM_LOW:                        // fall through
        case ON_SEND_STREAM_HIGH:                       // fall through
        case ON_RECV_STREAM_HIGH:
        {
            const BOOL SEND = (DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_LOW
              || DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_HIGH);

            one_net_raw_did_t raw_src_did;
            UInt16 cmd_and_data_type;
            UInt8 txn;

            if(b_s_in_progress(ON_STREAM, SRC_DID))
            {
                status = ONS_ALREADY_IN_PROGRESS;
                break;
            } // if the transaction is already in progress //

            if((status = on_decode(raw_src_did, *SRC_DID, sizeof(*SRC_DID)))
              != ONS_SUCCESS)
            {
                break;
            } // if decoding the did failed //

            // getting the resource, and checking with the app is a catch 22.
            // If the rsrc is checked first, and the app does not want to
            // honor the txn, then checking the rsrc was a waste.  If the
            // app was checked, and the rsrc is unavailable, then the app
            // needs to be notified, and checking with the app in the first
            // place was a waste.  Since we won't have to notify the app if the
            // resource is not available, check the resource first.
            txn = get_free_txn(ON_STREAM, SEND);

            if(txn >= ONE_NET_MASTER_MAX_TXN)
            {
                status = ONS_RSRC_FULL;
                break;
            } // if the resource is not available //

            txn_list[txn].type = ON_STREAM;
            // mark that the stream is to continue (this is set to 0 when the
            // stream is ended
            txn_list[txn].txn.remaining = 1;
            one_net_memmove(txn_list[txn].did, *SRC_DID, sizeof(*SRC_DID));
            txn_list[txn].txn.send = SEND;
            txn_list[txn].txn.priority =
              (DATA[ON_ADMIN_MSG_ID_IDX] == ON_SEND_STREAM_HIGH
              || DATA[ON_ADMIN_MSG_ID_IDX] == ON_RECV_STREAM_HIGH)
              ? ONE_NET_HIGH_PRIORITY : ONE_NET_LOW_PRIORITY;

            cmd_and_data_type = one_net_byte_stream_to_int16(
              &(DATA[ON_BLOCK_STREAM_DATA_TYPE_IDX + ON_ADMIN_DATA_IDX]));
            if(one_net_master_txn_requested(ON_STREAM, txn_list[txn].txn.send,
              cmd_and_data_type, 0, (const one_net_raw_did_t * const)
              &raw_src_did))
            {
                on_update_next_txn_time(&(txn_list[txn].txn));
                enqueue_txn(txn);
                status = ONS_SUCCESS;
            } // if request is granted //
            else
            {
                free_txn(txn);
                status = ONS_UNHANDLED_PKT;
            } // else request is not granted //
            break;
        } // send/recv stream case //

        case ON_END_STREAM:
        {
            one_net_raw_did_t raw_did;

            // decode the address
            if((status = on_decode(raw_did, *SRC_DID, sizeof(*SRC_DID)))
              != ONS_SUCCESS)
            {
                break;
            } // if decoding the address was not successful //

            if((txn_list[cur_txn_info].type != ON_STREAM
              || !txn_list[cur_txn_info].txn.send
              || !on_encoded_did_equal(
              (const on_encoded_did_t * const)&(txn_list[cur_txn_info].did),
              SRC_DID)) && (cur_txn_info = dequeue_b_s_txn(ON_STREAM, SRC_DID))
              >= ONE_NET_MASTER_MAX_TXN)
            {
                status = ONS_TXN_DOES_NOT_EXIST;
            } // if the transaction was not found //
            else
            {
                free_txn(cur_txn_info);
                one_net_master_stream_txn_status(ONS_STREAM_END,
                  (const one_net_raw_did_t * const)&raw_did);
                status = ONS_STREAM_END;
            } // else the transaction was found //
            break;
        } // end stream case //
		#endif

        case ON_DATA_RATE_RESULT:
        {
            if(DATA[ON_DATA_RATE_DATA_RATE_IDX + ON_ADMIN_DATA_IDX]
              != data_rate_test.rate
              || !on_encoded_did_equal(SRC_DID,
              (const on_encoded_did_t * const)data_rate_test.send_did)
              || !on_encoded_did_equal((const on_encoded_did_t * const)
              &(DATA[ON_DATA_RATE_DID_IDX + ON_ADMIN_DATA_IDX]),
              (const on_encoded_did_t * const)data_rate_test.recv_did)
              || DATA[ON_DATA_RATE_RESULT_IDX + ON_ADMIN_DATA_IDX]
              > ON_MAX_DATA_RATE_TRIES)
            {
                // The data does not match what it should be, so ignore
                // the pkt
                status = ONS_INVALID_DATA;
            } // if any of the data is invalid //
            else
            {
                report_data_rate_result(data_rate_test.rate,
                  DATA[ON_DATA_RATE_RESULT_IDX + ON_ADMIN_DATA_IDX],
                  ON_MAX_DATA_RATE_TRIES);

                status = ONS_SUCCESS;
            } // else the data is valid //
            break;
        } // data rate response case //

        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_STREAM_KEY_QUERY:
        {
            status = send_stream_key_update(SRC_DID);
            break;
        } // stream key query case //
		#endif

        case ON_STATUS_QUERY:                           // fall through
        case ON_SETTINGS_QUERY:                         // fall through
        case ON_CHANGE_SETTINGS:                        // fall through
        case ON_FRAGMENT_DELAY_QUERY:                   // fall through
        case ON_CHANGE_LOW_FRAGMENT_DELAY:              // fall through
        case ON_KEEP_ALIVE_QUERY:                       // fall through
        case ON_CHANGE_KEEP_ALIVE:                      // fall through
        case ON_NEW_KEY_FRAGMENT:                       // fall through
#ifdef _PEER
        case ON_ASSIGN_PEER:                            // fall through
        case ON_UNASSIGN_PEER:                          // fall through
#endif
        case ON_INIT_DATA_RATE_TEST:                    // fall through
        case ON_RM_DEV:
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


/*!
    \brief Handles the end of an admin transaction.

    \param[in] ADMIN_ID The type of admin transaction that just ended.
    \param[in] DATA The admin data that was sent.
    \param[in/out] txn The transaction that just ended.
    \param[in] STATUS The status of the transaction.  This should only be
      ONS_SUCCESS or ONS_SINGLE_FAIL.  All other values will return an error.
    \param[in] client The CLIENT the pkt was sent to.
    \param[in] DID The raw did of the CLIENT the packet was sent to.

    \return ONS_SUCCESS if the processing was successful.
            ONS_BAD_PARAM if any of the parameters are invalid
            ONS_INTERNAL_ERR if something unexpected happened
*/
static one_net_status_t admin_txn_hdlr(const UInt8 ADMIN_ID,
  const UInt8 * const DATA, on_txn_t ** txn, const one_net_status_t STATUS,
  on_client_t * const client, const one_net_raw_did_t * const DID)
{
    one_net_status_t rv = ONS_INTERNAL_ERR;
    one_net_mac_update_t update = ONE_NET_UPDATE_NOTHING;

    if(!txn || !(*txn) || !(*txn)->pkt || (STATUS != ONS_SUCCESS
      && STATUS != ONS_SINGLE_FAIL) || !client || !DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    switch(ADMIN_ID)
    {
        case ON_CHANGE_SETTINGS:
        {
            if(client->flags != DATA[ON_SETTINGS_FLAG_IDX + ON_ADMIN_DATA_IDX])
            {
                client->flags = DATA[ON_SETTINGS_FLAG_IDX + ON_ADMIN_DATA_IDX];
                save = TRUE;
            } // if the flags have changed //

            if(client->data_rate != DATA[ON_SETTINGS_DATA_RATE_IDX])
            {
                update = ONE_NET_UPDATE_DATA_RATE;

                if(STATUS == ONS_SUCCESS)
                {
                    client->data_rate = DATA[ON_SETTINGS_DATA_RATE_IDX];
                    save = TRUE;
                } // if the txn was successful //
            } // if the data rate was being updated //
            else
            {
                update = ONE_NET_UPDATE_REPORT_TO_MASTER;
            } // else update report status changes to the MASTER //
            break;
        } // change settings case //

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_CHANGE_LOW_FRAGMENT_DELAY:
        {
            update = ONE_NET_UPDATE_LOW_FRAGMENT_DELAY;
            break;
        } // change low fragment delay case //

        case ON_CHANGE_HIGH_FRAGMENT_DELAY:
        {
            update = ONE_NET_UPDATE_HIGH_FRAGMENT_DELAY;
            break;
        } // change high fragment delay case //
		#endif

        case ON_CHANGE_KEEP_ALIVE:
        {
            update = ONE_NET_UPDATE_KEEP_ALIVE;
            break;
        } // change keep alive case //

        case ON_NEW_KEY_FRAGMENT:
        {
            if(STATUS == ONS_SUCCESS)
            {
                key_update_confirmed(client);
                save = TRUE;
            } // if changing the key fragment was successful //

            rv = ONS_SUCCESS;
            break;
        } // new key fragment case //

#ifdef _PEER
        case ON_ASSIGN_PEER:        // fall through
        {
            update = ONE_NET_UPDATE_ASSIGN_PEER;
            rv = ONS_SUCCESS;
            break;
        } // assign peer case //

        case ON_UNASSIGN_PEER:
        {
            update = ONE_NET_UPDATE_UNASSIGN_PEER;
            rv = ONS_SUCCESS;
            break;
        } // unassign peer case //
#endif
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_SEND_BLOCK_LOW:     // fall through
        case ON_RECV_BLOCK_LOW:     // fall through
        case ON_SEND_BLOCK_HIGH:    // fall through
        case ON_RECV_BLOCK_HIGH:
        {
            if(STATUS == ONS_SINGLE_FAIL && b_s_req.type == ON_BLOCK)
            {
                one_net_master_block_txn_status(ONS_BLOCK_FAIL, DID);

                b_s_req.type = ON_NO_TXN;
            } // if setting up the block txn failed //
            rv = ONS_SUCCESS;
            break;
        } // send/receive block low case //
		#endif

        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_SEND_STREAM_LOW:    // fall through
        case ON_RECV_STREAM_LOW:    // fall through
        case ON_SEND_STREAM_HIGH:   // fall through
        case ON_RECV_STREAM_HIGH:
        {
            if(STATUS == ONS_SINGLE_FAIL && b_s_req.type == ON_STREAM)
            {
                one_net_master_stream_txn_status(ONS_STREAM_FAIL, DID);
            } // if setting up the stream txn failed //
            rv = ONS_SUCCESS;
            break;
        } // send/receive stream low case //
		#endif

        case ON_INIT_DATA_RATE_TEST:
        {
            if(cur_txn_info == data_rate_test.recv_idx)
            {
                if(STATUS == ONS_SINGLE_FAIL)
                {
                    if(data_rate_test.send_idx < ONE_NET_MASTER_MAX_TXN)
                    {
                        free_txn(data_rate_test.send_idx);
                    } // if the MASTER is not the sender //

                    ont_stop_timer(data_rate_test.timer);
                    report_data_rate_result(data_rate_test.rate, 0, 0);
                } // if setting up the receiver failed //
                else if(STATUS == ONS_SUCCESS
                  && data_rate_test.send_idx >= ONE_NET_MASTER_MAX_TXN)
                {
                    (*txn)->data_len = (*txn)->pkt_size;
                    if((rv = on_build_data_rate_pkt((*txn)->pkt,
                      &((*txn)->data_len), (const on_encoded_did_t * const)
                      &(data_rate_test.recv_did), data_rate_test.rate))
                      == ONS_SUCCESS)
                    {
                        txn_list[cur_txn_info].type = ON_DATA_RATE_TXN;
                        single_txn_data_count--;
                        enqueue_txn(cur_txn_info);
                        // don't clr the txn when complete, since it is now
                        // used to send the data rate test
                        cur_txn_info = ONE_NET_MASTER_MAX_TXN;
                        *txn = &response_txn;
                    } // if building the data rate pkt was successful //
                    else
                    {
                        report_data_rate_result(data_rate_test.rate, 0, 0);
                    }
                    break;
                } // else if txn successful && MASTER is the sender //

                rv = ONS_SUCCESS;
            } // if status is for setting up receiver in a data rate test //
            else if(cur_txn_info == data_rate_test.send_idx)
            {
                if(STATUS == ONS_SINGLE_FAIL)
                {
                    report_data_rate_result(data_rate_test.rate, 0, 0);
                } // if setting up the data rate test sender failed //

                rv = ONS_SUCCESS;
            } // if txn successfully set up a data rate test //
            else
            {
                rv = ONS_INTERNAL_ERR;
            } // else an internal error occured //
            break;
        } // init data rate case //

        case ON_CHANGE_PEER_DATA_RATE:
        {
            update = ONE_NET_UPDATE_PEER_DATA_RATE;
            break;
        } // change peer data rate //

        case ON_RM_DEV:
        {
            if(one_net_master_remove_device_result(DID,
              STATUS == ONS_SUCCESS ? TRUE : FALSE))
            {
                rm_client((const on_encoded_did_t * const)client->did);
            } // if the app wants the device removed from the table //
            break;
        } // remove device case //

        default:
        {
            // handling of this type is not needed
            rv = ONS_SUCCESS;
            break;
        } // default case //
    } // switch(ADMIN_ID) //

    if(update < ONE_NET_UPDATE_NOTHING)
    {
        one_net_master_update_result(update, DID, STATUS == ONS_SUCCESS ? TRUE
          : FALSE);
        rv = ONS_SUCCESS;
    } // if reporting the results of a device update //

    return rv;
} // admin_txn_hdlr //


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Handles the end of a single data extended admin transaction.

    \param[in] ADMIN_ID The type of admin transaction that just ended.  Only

    \param[in] DATA The admin data that was sent.
    \param[in/out] txn The transaction that just ended.
    \param[in] STATUS The status of the transaction.  This should only be
      ONS_SUCCESS or ONS_SINGLE_FAIL.  All other values will return an error.
    \param[in] client The CLIENT the pkt was sent to.
    \param[in] DID The raw did of the CLIENT the packet was sent to.

    \return ONS_SUCCESS if the processing was successful.
            ONS_BAD_PARAM if any of the parameters are invalid
            ONS_INTERNAL_ERR if something unexpected happened
*/
static one_net_status_t extended_admin_single_txn_hdlr(const UInt8 ADMIN_ID,
  const UInt8 * const DATA, on_txn_t ** txn, const one_net_status_t STATUS,
  on_client_t * const client, const one_net_raw_did_t * const DID)
{
    one_net_status_t rv = ONS_INTERNAL_ERR;

    if(!txn || !(*txn) || !(*txn)->pkt || (STATUS != ONS_SUCCESS
      && STATUS != ONS_SINGLE_FAIL) || !client || !DID)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    switch(ADMIN_ID)
    {
        case ON_RECV_BLOCK_LOW:
        {
            if(STATUS == ONS_SINGLE_FAIL && b_s_req.type == ON_BLOCK)
            {
                b_s_req.type = ON_NO_TXN;
            } // if setting up the block txn failed //
            rv = ONS_SUCCESS;
            break;
        } // send/receive block low case //

        default:
        {
            // Should not have gotten here
            break;
        } // default case //
    } // switch(ADMIN_ID) //

    return rv;
} // one_net_status_t extended_admin_single_txn_hdlr //
#endif


/*!
    \brief Send an Admin packet to change a CLIENT's settings.

    \param[in] DST The encoded did of the CLIENT whose settings are being
      changed.
    \param[in] DATA_RATE The data rate to set the CLIENT to receive at.
    \param[in] FLAGS The flags the CLIENT should have set.

    \return See send_admin_pkt for possible return values.
*/
static one_net_status_t send_change_settings(const on_encoded_did_t * const DST,
  const UInt8 CLIENT_DATA_RATE, const UInt8 FLAGS)
{
    UInt8 pld[ON_MAX_ADMIN_PLD_LEN];

    pld[ON_SETTINGS_DATA_RATE_IDX] = CLIENT_DATA_RATE;
    pld[ON_SETTINGS_MASTER_DATA_RATE_IDX] = on_base_param->data_rate;
    pld[ON_SETTINGS_FLAG_IDX] = FLAGS;

    return send_admin_pkt(ON_CHANGE_SETTINGS, ON_ADMIN_MSG, DST,
      ONE_NET_HIGH_PRIORITY, pld, sizeof(pld));
} // send_change_settings //


/*!
    \brief Checks to see if a key update needs to be sent.

    \param[in] ONLY_IF_ACTIVE TRUE if the check should be done only if the
      key update process is already active.

    \return void
*/
static void check_key_update(const BOOL ONLY_IF_ACTIVE)
{
    const BOOL ACTIVE = ont_active(ONT_CHANGE_KEY_TIMER);

    if(!master_param->client_count || (ONLY_IF_ACTIVE && !ACTIVE)
      || (ACTIVE && !ont_expired(ONT_CHANGE_KEY_TIMER)))
    {
        // timer hasn't expired, so don't try the next CLIENT yet
        return;
    } // if the timer is active and has not expired //

    if(txn_data_count < ONE_NET_MASTER_LOW_LOAD_LIMIT)
    {
        UInt16 start = key_change_idx;

        BOOL loop = TRUE;

        // find a CLIENT that has not had it's key updated
        do
        {
            key_change_idx++;
            if(key_change_idx >= master_param->client_count)
            {
                key_change_idx = 0;
            } // if beyond the last CLIENT //

            if(!client_list[key_change_idx].use_current_key)
            {
                one_net_raw_did_t raw_did;
                on_encoded_did_t dst;

                one_net_int16_to_byte_stream((key_change_idx
                  << ON_INIT_CLIENT_SHIFT) + ONE_NET_INITIAL_CLIENT_DID,
                  raw_did);
                if(on_encode(dst, raw_did, sizeof(dst)) == ONS_SUCCESS)
                {
                    send_key_update((const on_encoded_did_t * const)&dst);
                } // if encoding was successful //

                loop = FALSE;
                break;
            } // if using the old key //
        } while(loop && key_change_idx != start);

        if(!loop)
        {
            ont_set_timer(ONT_CHANGE_KEY_TIMER,
              ONE_NET_MASTER_CHANGE_KEY_TIMEOUT);
        } // if sending a key change //
        else if(ACTIVE)
        {
            // everyone's key is updated
            ont_stop_timer(ONT_CHANGE_KEY_TIMER);
            one_net_master_update_result(ONE_NET_UPDATE_NETWORK_KEY, 0, TRUE);
        } // else if the key was being updated //
    } // if the load is low //
    else
    {
        // keep checking
        ont_set_timer(ONT_CHANGE_KEY_TIMER, 0);
    } // else too many queued transactions //
} // check_key_update //


/*!
    \brief Finishes up a key update for the given device id.

    Removes any queued key updates since the device just confirmed the update
    being sent.

    \param[in] client The CLIENT whose key has been confirmed

    \return void
*/
static void key_update_confirmed(on_client_t * const client)
{
    // current pointer
    UInt8 cur;

    if(!client)
    {
        return;
    } // if the parameter is invalid //

    client->use_current_key = TRUE;

    // It is possible the change key got through to the CLIENT but the ack did
    // not make it back to update the nonce.  The CLIENT transaction to validate
    // that it received the new key may still succeed, so the next time the
    // MASTER sends a packet, it will be with the last nonce.  This may cause
    // the CLIENT to believe that it had already received the next packet and
    // send a false ack, thus never receiving the new data.  To avoid this,
    // change to an incorrect nonce.
    cur = client->send_nonce;
    do
    {
        client->send_nonce = one_net_prand(one_net_tick(), ON_MAX_NONCE);
    }while(cur == client->send_nonce);

    cur = txn_head;
    while(cur != ONE_NET_MASTER_MAX_TXN)
    {
        if(txn_list[cur].type == ON_SINGLE
          && txn_list[cur].txn.msg_type == ON_ADMIN_MSG
          && txn_list[cur].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_NEW_KEY_FRAGMENT
          && on_encoded_did_equal(
          (const on_encoded_did_t * const)&(txn_list[cur].did),
          (const on_encoded_did_t * const)&(client->did)))
        {
            UInt8 txn_to_free = cur;

            cur = txn_list[cur].next;
            free_txn(txn_to_free);
            continue;
        } // if a new key fragment msg to DID was queued //

        cur = txn_list[cur].next;
    } // loop through the queued transactions //

    // move on to another device
    ont_set_timer(ONT_CHANGE_KEY_TIMER, 0);
} // key_update_confirmed //


/*!
    \brief Queues the txn to send a ON_NEW_KEY_FRAGMENT to DST.

    This sends the 32 least significant bits of the new key which is contained
    in on_base_param->current_key.

    \param[in] DST The device to send the new key fragment to.

    \return ONS_SUCCESS if the transaction was successfully queued.
            ONS_BAD_PARAM If the parameter is invalid
*/
static one_net_status_t send_key_update(const on_encoded_did_t * const DST)
{
    if(!DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    return send_admin_pkt(ON_NEW_KEY_FRAGMENT, ON_ADMIN_MSG, DST,
      ONE_NET_HIGH_PRIORITY,
      &(on_base_param->current_key[sizeof(on_base_param->current_key)
      - ONE_NET_XTEA_KEY_FRAGMENT_SIZE]), ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
} // send_key_update //


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Checks to see if a stream key update needs to be sent.

    \param[in] ONLY_IF_ACTIVE TRUE if the check should be done only if the
      key update process is already active.

    \return void
*/
static void check_stream_key_update(const BOOL ONLY_IF_ACTIVE)
{
    const BOOL ACTIVE = ont_active(ONT_CHANGE_STREAM_KEY_TIMER);

    #ifdef _DISABLE_BLOCK_STREAM
	return;
	#endif

    if(!master_param->client_count || (ONLY_IF_ACTIVE && !ACTIVE)
      || (ACTIVE && !ont_expired(ONT_CHANGE_STREAM_KEY_TIMER)))
    {
        // timer hasn't expired, so don't try the next CLIENT yet
        return;
    } // if the timer is active and has not expired //

    if(txn_data_count < ONE_NET_MASTER_LOW_LOAD_LIMIT)
    {
        UInt16 start = stream_key_change_idx;

        BOOL loop = TRUE;

        // find a CLIENT that has not had it's key updated
        do
        {
            stream_key_change_idx++;
            if(stream_key_change_idx >= master_param->client_count)
            {
                stream_key_change_idx = 0;
            } // if beyond the last CLIENT //

            // if bits 4 and 7 are both 1, then client is stream capable,
            // so do a mask with ON_STREAM_CAPABLE, which is 10010000
            // to see if they are -- note high-order bit is considered bit 7.
            if((client_list[stream_key_change_idx].features &
                    ON_STREAM_CAPABLE == ON_STREAM_CAPABLE)
              && !client_list[stream_key_change_idx].use_current_stream_key)
            {
                one_net_raw_did_t raw_did;
                on_encoded_did_t dst;

                one_net_int16_to_byte_stream((key_change_idx
                  << ON_INIT_CLIENT_SHIFT) + ONE_NET_INITIAL_CLIENT_DID,
                  raw_did);
                if(on_encode(dst, raw_did, sizeof(dst)) == ONS_SUCCESS)
                {
                    send_stream_key_update((const on_encoded_did_t * const)&dst);
                } // if encoding was successful //

                loop = FALSE;
                break;
            } // if using the old key //
        } while(loop && stream_key_change_idx != start);

        if(!loop)
        {
            ont_set_timer(ONT_CHANGE_STREAM_KEY_TIMER,
              ONE_NET_MASTER_CHANGE_KEY_TIMEOUT);
        } // if sending a key change //
        else if(ACTIVE)
        {
            // everyone's key is updated
            ont_stop_timer(ONT_CHANGE_STREAM_KEY_TIMER);
            one_net_master_update_result(ONE_NET_UPDATE_STREAM_KEY, 0, TRUE);
        } // else if the key was being updated //
    } // if the load is low //
    else
    {
        // keep checking
        ont_set_timer(ONT_CHANGE_STREAM_KEY_TIMER, 0);
    } // else too many queued transactions //
} // check_stream_key_update //


/*!
    \brief Finishes up a stream key update for the given device id.

    \param[in] client The CLIENT whose key has been confirmed

    \return void
*/
static void stream_key_update_confirmed(on_client_t * const client)
{
    if(!client)
    {
        return;
    } // if the parameter is invalid //

    client->use_current_stream_key = TRUE;

    // move on to another device
    ont_set_timer(ONT_CHANGE_STREAM_KEY_TIMER, 0);
} // stream_key_update_confirmed //


/*!
    \brief Queues the txn to send a ON_CHANGE_STREAM_KEY to DST.

    \param[in] DST The device to send the stream key to.

    \return ONS_SUCCESS if the transaction was successfully queued.
            ONS_BAD_PARAM If the parameter is invalid
*/
static one_net_status_t send_stream_key_update(
  const on_encoded_did_t * const DST)
{
    return b_s_request(ON_BLOCK, ON_EXTENDED_ADMIN_MSG, TRUE,
      ON_CHANGE_STREAM_KEY, sizeof(on_base_param->stream_key),
      ONE_NET_LOW_PRIORITY, DST, ONE_NET_DEV_UNIT);
} // send_stream_key_update //
#endif


/*!
    \brief Sends an admin packet (single transaction).

    Finds the available resource, sets it up, and queues the transaction to
    send an admin packet.

    \param[in] ADMIN_MSG_ID The type of admin message being sent.
    \param[in] MSG_TYPE The type of message being sent.  Valid values are
      ON_ADMIN_MSG, and ON_EXTENDED_ADMIN_MSG
    \param[in] DST The packet destination.
    \param[in] PRIORITY The priority of the transaction.
    \param[in] PLD The  admin data to be sent.
    \param[in] PLD_LEN The number of bytes in PLD

    \return ONS_SUCCESS If the packet was built and queued successfully
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_RSRC_FULL If there are no resources available
*/
static one_net_status_t send_admin_pkt(const UInt8 ADMIN_MSG_ID,
  const UInt8 MSG_TYPE, const on_encoded_did_t * const DST,
  const UInt8 PRIORITY, const UInt8 * const PLD, const UInt8 PLD_LEN)
{
    UInt8 txn;

    #ifdef _STREAM_MESSAGES_ENABLED
    if((MSG_TYPE != ON_ADMIN_MSG && MSG_TYPE != ON_EXTENDED_ADMIN_MSG)
    #else
    if((MSG_TYPE != ON_ADMIN_MSG)
	#endif
	|| !DST || !PLD || PLD_LEN != ON_MAX_ADMIN_PLD_LEN)
    {
        return ONS_BAD_PARAM;
    } // the parameter is invalid //

    txn = get_free_txn(ON_SINGLE, TRUE);

    if(txn >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_RSRC_FULL;
    } // if the resource is not available //

    txn_list[txn].type = ON_SINGLE;
    txn_list[txn].txn.data_len = PLD_LEN;
    txn_list[txn].txn.msg_type = MSG_TYPE;
    txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] = ADMIN_MSG_ID;
    one_net_memmove(txn_list[txn].did, *DST, sizeof(*DST));
    one_net_memmove(&(txn_list[txn].txn.pkt[ON_DATA_IDX]), PLD, PLD_LEN);
    txn_list[txn].txn.priority = PRIORITY;

    enqueue_txn(txn);

    return ONS_SUCCESS;
} // send_admin_pkt //


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Handles retrieving the next payload for block or stream transactions.

    \param[in] TXN_INFO The transaction to get the next payload for.

    \return TRUE if the next payload was retrieved.
            FALSE if the parameter was invalid, or the transaction was aborted.
*/
static BOOL get_next_b_s_pld(const UInt8 TXN_INFO)
{
    if(TXN_INFO >= ONE_NET_MASTER_MAX_TXN)
    {
        return FALSE;
    } // if the parameter is invalid //

    if(txn_list[TXN_INFO].txn.msg_type == ON_APP_MSG)
    {
        return get_b_s_app_data(TXN_INFO);
    } // if an admin message //
	#ifdef _STREAM_MESSAGES_ENABLED
    else if(txn_list[TXN_INFO].txn.msg_type == ON_EXTENDED_ADMIN_MSG)
    {
        return get_b_s_extended_admin_data(TXN_INFO);
    } // else if an extended admin message //
	#endif

    // Should not get here.
    return FALSE;
} // get_next_b_s_pld //


/*!
    \brief Handles retrieving the next payload for application block or stream
      transactions.

    This verifies the address is valid, and that the application returns a
    valid pointer.  If the address or pointer returned by the app is invalid
    (or the app returns too much data), the transaction will be aborted (the
    app will be notified).

    \param[in] TXN_INFO The transaction to get the next payload for.

    \return A non-Error nack reason if the block transaction should proceed
            An error nack reason if the block transaction should not proceed.
*/
on_nack_rsn_t get_b_s_app_data(const UInt8 TXN_INFO)
{
    one_net_raw_did_t dst;
    const UInt8 * DATA_PTR = 0;
    UInt16 len = 0;
    BOOL block = FALSE;
    on_nack_rsn_t nack_reason;
    UInt16 position_to_send = 0;

    if(txn_list[TXN_INFO].type == ON_BLOCK)
    {
        block = TRUE;
    } // if a block transaction //
	#ifdef _STREAM_MESSAGES_ENABLED
    else if(txn_list[TXN_INFO].type == ON_STREAM)
    {
        block = FALSE;
    } // else if a stream transaction //
	#endif
    else
    {
        return ON_NACK_RSN_BAD_DATA_ERR;
    } // else not a block or stream transaction //

    // decode address
    if(on_decode(dst, txn_list[TXN_INFO].did, sizeof(txn_list[TXN_INFO].did))
      != ONS_SUCCESS
      || client_info((const on_encoded_did_t * const)&(txn_list[TXN_INFO].did))
      == 0)
    {
        // since the address is not valid, alert the app and free the txn.
		#ifdef _STREAM_MESSAGES_ENABLED
        if(block)
        {
            one_net_master_block_txn_status(ONS_BAD_ADDR,
              (const one_net_raw_did_t * const)&dst);
        } // if a block transaction //
        else
        {
            one_net_master_stream_txn_status(ONS_BAD_ADDR,
              (const one_net_raw_did_t * const)&dst);
        } // else a stream transaction //
		#else
        one_net_master_block_txn_status(ONS_BAD_ADDR,
          (const one_net_raw_did_t * const)&dst);
		#endif

        free_txn(TXN_INFO);
        return ON_NACK_RSN_BAD_ADDRESS_ERR;
    } // if the address is not valid //

    // get data from app
    if((nack_reason = one_net_master_next_payload(txn_list[TXN_INFO].type, &len,
      (const one_net_raw_did_t * const)&dst)) != ON_NACK_RSN_NO_ERROR
      || len > ON_MAX_RAW_PLD_LEN
      || (block && len > txn_list[TXN_INFO].txn.remaining))
    {
		#ifdef _STREAM_MESSAGES_ENABLED
        if(block)
        {
            one_net_master_block_txn_status(ONS_INVALID_DATA,
              (const one_net_raw_did_t * const)&dst);
        } // if a block transaction //
        else
        {
            one_net_master_stream_txn_status(ONS_INVALID_DATA,
              (const one_net_raw_did_t * const)&dst);
        } // else a stream transaction //
		#else
            one_net_master_block_txn_status(ONS_INVALID_DATA,
              (const one_net_raw_did_t * const)&dst);
        #endif

        free_txn(TXN_INFO);
        return FALSE;
    } // if the data returned from the app is not valid //

    // adjust length if it is too long.
    if(block && len > txn_list[TXN_INFO].txn.remaining)
    {
        len = txn_list[TXN_INFO].txn.remaining;
    }
    if(len > ONE_NET_RAW_BLOCK_STREAM_DATA_LEN - 4)
    {
        len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN - 4;
    }

    if(block_data_pos >= block_data_len)
    {
        len = 0;
        position_to_send = 0;
    }
    else
    {
        position_to_send = block_data_pos + 1;
        one_net_memmove(&(txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX + 4]),
            block_data, len);
    }

    txn_list[TXN_INFO].txn.data_len = len + 4;
    txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX] =
        (UInt8) ((position_to_send & 0xFF00) >> 8);
    txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX + 1] =
        (UInt8) (position_to_send & 0x00FF);
    txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX + 2] =
        (UInt8) ((len & 0xFF00) >> 8);
    txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX + 3] =
        (UInt8) (len & 0x00FF);

    return ON_NACK_RSN_NO_ERROR;
} // get_b_s_app_data //


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Handles retrieving the next payload for extended admin block or
      stream transactions.

    \param[in] TXN_INFO The transaction to get the next payload for.

    \return TRUE if the next payload was retrieved.
            FALSE if the parameter was invalid, or the transaction was aborted.
*/
static BOOL get_b_s_extended_admin_data(const UInt8 TXN_INFO)
{
    // Currently, there is only 1 extended admin message - Update Stream Key.
    // For now, this function sets up the block fragment to send the new stream
    // key.  When more extended admin messages are added later, this function
    // will need to be refactored.
    txn_list[TXN_INFO].txn.data_len = sizeof(one_net_xtea_key_t);
    txn_list[TXN_INFO].txn.remaining -= txn_list[TXN_INFO].txn.data_len;
    one_net_memmove(&(txn_list[TXN_INFO].txn.pkt[ON_DATA_IDX]),
      on_base_param->stream_key, txn_list[TXN_INFO].txn.data_len);

    return TRUE;
} // get_b_s_extended_admin_data //
#endif


/*!
    \brief Checks if a block or stream transaction is being requested.

    \param void

    \return void
*/
static void check_b_s_req(void)
{
	#ifdef _STREAM_MESSAGES_ENABLED
    if(txn_list[cur_txn_info].txn.msg_type != ON_ADMIN_MSG
      && txn_list[cur_txn_info].txn.msg_type != ON_EXTENDED_ADMIN_MSG)
	#else
    if(txn_list[cur_txn_info].txn.msg_type != ON_ADMIN_MSG)
	#endif
    {
        return;
    } // if not an admin message //

    if(txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_SEND_BLOCK_LOW
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_RECV_BLOCK_LOW
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_SEND_BLOCK_HIGH
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
      == ON_RECV_BLOCK_HIGH)
    {
        b_s_req.type = ON_BLOCK;
        // this device sends (b_s_req.send == TRUE) if the other device receives
        b_s_req.send = txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
          == ON_RECV_BLOCK_LOW
          || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
          == ON_RECV_BLOCK_HIGH;
        b_s_req.len = one_net_byte_stream_to_int16(
          &(txn_list[cur_txn_info].txn.pkt[ON_BLOCK_LEN_IDX + ON_DATA_IDX]));
    } // if sending a block admin request //
	#ifdef _STREAM_MESSAGES_ENABLED
    else if(txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
      == ON_SEND_STREAM_LOW
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_RECV_STREAM_LOW
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_SEND_STREAM_HIGH
      || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_RECV_STREAM_HIGH)
    {
        b_s_req.type = ON_STREAM;
        // this device sends (b_s_req.send == TRUE) if the other device receives
        b_s_req.send = txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
          == ON_RECV_STREAM_LOW
          || txn_list[cur_txn_info].txn.pkt[ON_ADMIN_TYPE_IDX]
          == ON_RECV_STREAM_HIGH;
        // mark for the stream to continue
        b_s_req.len = 1;
    } // else if sending a stream admin request //
	#endif
    else
    {
        b_s_req.type = ON_NO_TXN;
        return;
    } // else not a block or stream request //
} // check_b_s_req //


/*!
    \brief Checks if a block or stream transaction is already in progress with
      DID.

    \param[in] TYPE The type of transaction to check for (ON_BLOCK or
      ON_STREAM).
    \param[in] DID The device to check if a block or stream is already in
      progress with it.

    \return TRUE if a transaction of TYPE is already in progress with DID.
            FALSE if there is no transaction of TYPE in progress with DID.
*/
static BOOL b_s_in_progress(const UInt8 TYPE,
  const on_encoded_did_t * const DID)
{
    #ifdef _STREAM_MESSAGES_ENABLED
    const UInt8 SEND_LOW_TYPE = TYPE == ON_BLOCK ? ON_SEND_BLOCK_LOW
      : ON_SEND_STREAM_LOW;
    const UInt8 RECV_LOW_TYPE = TYPE == ON_BLOCK ? ON_RECV_BLOCK_LOW
      : ON_RECV_STREAM_LOW;
    const UInt8 SEND_HIGH_TYPE = TYPE == ON_BLOCK ? ON_SEND_BLOCK_HIGH
      : ON_SEND_STREAM_HIGH;
    const UInt8 RECV_HIGH_TYPE = TYPE == ON_BLOCK ? ON_RECV_BLOCK_HIGH
      : ON_RECV_STREAM_HIGH;
	#endif

    UInt8 txn;
    #ifdef _STREAM_MESSAGES_ENABLED
    if((TYPE != ON_BLOCK && TYPE != ON_STREAM) || !DID)
	#else
    if((TYPE != ON_BLOCK) || !DID)
	#endif
    {
        // Return TRUE since we don't want to assume there is no transaction
        // when the parameters are invalid.  Deny allowing a transaction since
        // there is no gaurantee the txn is not being carried out.
        return TRUE;
    } // if one of the parameters is invalid //

    // loop through all queued transactions and make sure a block transaction
    // with this device is not already in progress
    for(txn = txn_head; txn < ONE_NET_MASTER_MAX_TXN; txn = txn_list[txn].next)
    {
        if(txn_list[txn].type == TYPE && on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(txn_list[txn].did)))
        {
            return TRUE;
        } // if a block transaction w/DID is already in progress //
        else if(txn_list[txn].type == ON_SINGLE && on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(txn_list[txn].did)))
        {
            // make sure a request for a block txn is not already being made
            // Transactions in the queue are not built until they are sent.
			#ifdef _STREAM_MESSAGES_ENABLED
            if((txn_list[txn].txn.msg_type == ON_ADMIN_MSG
              || txn_list[txn].txn.msg_type == ON_EXTENDED_ADMIN_MSG)
              && (txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == SEND_LOW_TYPE
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == RECV_LOW_TYPE
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == SEND_HIGH_TYPE
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == RECV_HIGH_TYPE))
			#else
            if((txn_list[txn].txn.msg_type == ON_ADMIN_MSG)
              && (txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_SEND_BLOCK_LOW
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_RECV_BLOCK_LOW
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_SEND_BLOCK_HIGH
              || txn_list[txn].txn.pkt[ON_ADMIN_TYPE_IDX] == ON_RECV_BLOCK_HIGH))
			#endif
            {
                return TRUE;
            } // if a block transaction request is already queued //
        } // if single transaction to dst device //
    } // loop through the queued transactions //

    return FALSE;
} // b_s_in_progress //
#endif // if block messages are enabled //


/*!
    \brief Builds the data packet for the given transaction.

    The raw data is stored in the pkt field of the transaction.  It is copied
    out of that field, the client info is found for the destination, and the
    packet is built.  This should be called when the transaction is actually
    being sent.

    \param[in] TXN  The transaction to build the data packet for.

    \return ONS_SUCCESS If the transaction was successfully built.
            ONS_BAD_PARAM If the parameter was invalid
            ONS_INTERNAL_ERR If there was a problem with the data size or
              msg_type
            ONS_BAD_ADDR If the CLIENT address was invalid.
            See on_build_data_pkt for more possible return values.
*/
static one_net_status_t build_txn_data_pkt(const UInt8 TXN)
{
    const on_client_t * CLIENT = 0;
    const one_net_xtea_key_t * KEY = 0;

    one_net_raw_did_t raw_did;
    one_net_status_t status;
    UInt8 pid, admin_type, txn_nonce, data_len;
    UInt8 data[ON_MAX_RAW_PLD_LEN];

    if(TXN >= ONE_NET_MASTER_MAX_TXN)
    {
        return ONS_BAD_PARAM;
    } // if the parameter was invalid //

    if(txn_list[TXN].txn.data_len > sizeof(data))
    {
        return ONS_INTERNAL_ERR;
    } // if an internal error occured //

    if(on_decode(raw_did, txn_list[TXN].did, ON_ENCODED_DID_LEN) != ONS_SUCCESS
      || (CLIENT = client_info((const on_encoded_did_t * const)
      &(txn_list[TXN].did))) == 0)
    {
        return ONS_INTERNAL_ERR;
    } // if decoding the did failed || the CLIENT could not be found //

    // get the information & raw fields that were stored in the packet
    switch(txn_list[TXN].type)
    {
        case ON_SINGLE:
        {
            pid = ONE_NET_ENCODED_SINGLE_DATA;
            KEY = &(on_base_param->current_key);
            break;
        } // ON_SINGLE case //

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_BLOCK:
        {
            if(!txn_list[TXN].txn.retry)
            {
                pid = ONE_NET_ENCODED_BLOCK_DATA;
            } // if first time the block is sent //
            else
            {
                pid = ONE_NET_ENCODED_REPEAT_BLOCK_DATA;
            } // else this is a repeat block //

            KEY = &(on_base_param->current_key);
            break;
        } // ON_BLOCK case //
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_STREAM:
        {
            pid = ONE_NET_ENCODED_STREAM_DATA;
            KEY = &(on_base_param->stream_key);
            break;
        } // ON_STREAM case //
		#endif

        default:
        {
            return ONS_INTERNAL_ERR;
            break;
        } // default case //
    } // switch on transactin type //

    admin_type = txn_list[TXN].txn.pkt[ON_ADMIN_TYPE_IDX];
    data_len = txn_list[TXN].txn.data_len;
    one_net_memmove(data, &(txn_list[TXN].txn.pkt[ON_DATA_IDX]), data_len);

    txn_nonce = CLIENT->send_nonce;

    txn_list[TXN].txn.data_len = txn_list[TXN].txn.pkt_size;
    txn_list[TXN].txn.expected_nonce = one_net_prand(one_net_tick(),
      ON_MAX_NONCE);

    switch(txn_list[TXN].txn.msg_type)
    {
        case ON_APP_MSG:
        {
            status = on_build_data_pkt(txn_list[TXN].txn.pkt,
              &(txn_list[TXN].txn.data_len), txn_list[TXN].txn.msg_type, pid,
              (const on_encoded_did_t * const)&(txn_list[TXN].did), txn_nonce,
              txn_list[TXN].txn.expected_nonce, data, data_len,
			  #ifdef _STREAM_MESSSAGES_ENABLED
              txn_list[TXN].type == ON_STREAM || CLIENT->use_current_key ? KEY
              : (const one_net_xtea_key_t * const)&(master_param->old_key));
			  #else
              CLIENT->use_current_key ? KEY
              : (const one_net_xtea_key_t * const)&(master_param->old_key));
			  #endif
            break;
        } // ON_APP_MSG case //

        case ON_ADMIN_MSG:
        {
            status = on_build_admin_pkt(txn_list[TXN].txn.pkt,
              &(txn_list[TXN].txn.data_len), txn_list[TXN].txn.msg_type,
              admin_type, (const on_encoded_did_t * const)&(txn_list[TXN].did),
              txn_nonce, txn_list[TXN].txn.expected_nonce, data, data_len,
			  #ifdef _STREAM_MESSAGES_ENABLED
              txn_list[TXN].type == ON_STREAM || CLIENT->use_current_key ? KEY
              : (const one_net_xtea_key_t * const)&(master_param->old_key));
			  #else
              CLIENT->use_current_key ? KEY
              : (const one_net_xtea_key_t * const)&(master_param->old_key));
			  #endif
            break;
        } // ON_ADMIN_MSG case //

        #ifdef _STREAM_MESSAGES_ENABLED
        case ON_EXTENDED_ADMIN_MSG:
        {
            if(txn_list[TXN].type == ON_SINGLE)
            {
                status = on_build_admin_pkt(txn_list[TXN].txn.pkt,
                  &(txn_list[TXN].txn.data_len), txn_list[TXN].txn.msg_type,
                  admin_type,
                  (const on_encoded_did_t * const)&(txn_list[TXN].did),
                  txn_nonce, txn_list[TXN].txn.expected_nonce, data, data_len,
                  CLIENT->use_current_key ? KEY
                  : (const one_net_xtea_key_t * const)&(master_param->old_key));
            } // if a single transaction //
            else if(txn_list[TXN].type == ON_BLOCK)
            {
                status = on_build_data_pkt(txn_list[TXN].txn.pkt,
                  &(txn_list[TXN].txn.data_len), txn_list[TXN].txn.msg_type,
                  pid, (const on_encoded_did_t * const)&(txn_list[TXN].did),
                  txn_nonce, txn_list[TXN].txn.expected_nonce, data, data_len,
                  CLIENT->use_current_key ? KEY
                  : (const one_net_xtea_key_t * const)&(master_param->old_key));
            } // else if a block transaction //
            else
            {
                status = ONS_INTERNAL_ERR;
            } // else an internal error //
            break;
        } // ON_EXTENDED_ADMIN_MSG case //
		#endif

        default:
        {
            status = ONS_INTERNAL_ERR;
            break;
        } // default case //
    } // switch(msg_type) //

    return status;
} // build_txn_data_pkt //


/*!
    \brief Removes the CLIENT from the network (by removing it from the table)

    \param[in] DID The device ID of the CLIENT to remove

    \return ONS_SUCCESS if the client was deleted.
            ONS_INVALID_DATA if the client was not deleted.
*/
static one_net_status_t rm_client(const on_encoded_did_t * const DID)
{
    UInt16 i;

    if(!DID)
    {
        return ONS_INVALID_DATA;
    } // if the parameter is invalid //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&client_list[i].did))
        {
            one_net_memmove(client_list[i].did, ON_ENCODED_BROADCAST_DID,
                ON_ENCODED_DID_LEN);
			adjust_client_list();

			return ONS_SUCCESS;
        } // if the CLIENT was found //
    } // loop to find the CLIENT //

	return ONS_INVALID_DATA;
} // rm_client //



/*!
    \brief Cleans up client list

    \return the index in the client list where the new client's settings should
    be stored.  -1 means no room left.
*/
static int adjust_client_list()
{
    on_encoded_did_t enc_did;
    int i, index;
    one_net_raw_did_t raw_next_did;
    BOOL found_next_did = FALSE;

    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID;
    index = 0;

	// go through and look for the first hole(if any) in the list
	for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(&ON_ENCODED_BROADCAST_DID, &client_list[i].did))
        {
            // found an empty spot
            (master_param->client_count)--;
            if(i < master_param->client_count)
            {
                one_net_memmove(&(client_list[i]), &(client_list[i + 1]),
                  sizeof(on_client_t) * (master_param->client_count - i));
            }

            return adjust_client_list();
		}

        one_net_int16_to_byte_stream(master_param->next_client_did, raw_next_did);
        on_encode(enc_did, raw_next_did, ONE_NET_RAW_DID_LEN);

        if(!on_encoded_did_equal(&enc_did, &client_list[i].did))
        {
			found_next_did = TRUE;
		}

		if(!found_next_did)
        {
			master_param->next_client_did += ON_CLIENT_DID_INCREMENT;
            index++;
		}
    }

	if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
		index = -1;
    }

    return index;
}


/*!
    \brief Returns an unused location to store CLIENT information.

    \return an unused location to store CLIENT information if there is room
            NULL if there is no room.
*/
static on_client_t* one_net_master_add_new_client_to_list()
{
    one_net_raw_did_t new_raw_did;

    int index = adjust_client_list();

    if(index == -1)
    {
        return 0;
    }

    if(index < master_param->client_count)
    {
        one_net_memmove(&client_list[index+1], &client_list[index],
            (master_param->client_count - index) * sizeof(on_client_t));
    }

    one_net_int16_to_byte_stream(master_param->next_client_did, new_raw_did);
    on_encode(client_list[index].did, new_raw_did, ONE_NET_RAW_DID_LEN);
    (master_param->client_count)++;
    adjust_client_list();
    return &client_list[index];
}


/*!
    \brief Returns a pointer to a free transaction.

    \param[in] TYPE The type of transaction.
    \param[in] SEND TRUE if sending
                        FALSE if not sending

    \return A pointer to a free transaction.
*/
static UInt8 get_free_txn(const UInt8 TYPE, const BOOL SEND)
{
    UInt8 free_txn_info;

    if(SEND && TYPE != ON_SINGLE &&
      (ONE_NET_MASTER_MAX_SEND_TXN - txn_data_count + single_txn_data_count)
      <= ONE_NET_MASTER_MIN_SINGLE_TXN)
    {
        return ONE_NET_MASTER_MAX_TXN;
    } // if the resources are full //

    for(free_txn_info = 0; free_txn_info < ONE_NET_MASTER_MAX_TXN;
      free_txn_info++)
    {
        if(free_txn_info != invite_idx
          && txn_list[free_txn_info].type == ON_NO_TXN)
        {
            break;
        } // if a free transaction was found //
    } // loop through to find a free transaction //

    if(free_txn_info < ONE_NET_MASTER_MAX_TXN && SEND)
    {
        if(SEND)
        {
            UInt8 free_txn_data;

            for(free_txn_data = 0; free_txn_data < ONE_NET_MASTER_MAX_SEND_TXN;
              free_txn_data++)
            {
                if(!pkt_list[free_txn_data].in_use)
                {
                    break;
                } // if a free data txn was found //
            } // loop to find a free transaction //

            if(free_txn_data >= ONE_NET_MASTER_MAX_SEND_TXN)
            {
                return ONE_NET_MASTER_MAX_TXN;
            } // if a free txn data element was not found //

            pkt_list[free_txn_data].in_use = TRUE;
            txn_list[free_txn_info].txn.pkt = pkt_list[free_txn_data].pkt_data;
            txn_list[free_txn_info].txn.pkt_size
              = sizeof(pkt_list[free_txn_data].pkt_data);
            txn_list[free_txn_info].pkt_idx = free_txn_data;

            if(TYPE == ON_SINGLE)
            {
                single_txn_data_count++;
            } // if it's a single transaction //

            txn_data_count++;
        } // if SENDing //

        txn_list[free_txn_info].txn.send = SEND;
    } // if there is a free txn info, and the txn needs memory for a pkt //

    txn_list[free_txn_info].txn.retry = 0;
    return free_txn_info;
} // get_free_txn //


/*!
    \brief Frees a transaction.

    \param[in] TXN The transaction to be freed

    \return void
*/
static void free_txn(const UInt8 TXN)
{
    if(TXN >= ONE_NET_MASTER_MAX_TXN || txn_list[TXN].type == ON_NO_TXN)
    {
        return;
    } // if the parameter is not valid //

    dequeue_txn(TXN);

    if(txn_list[TXN].txn.send)
    {
        txn_data_count--;

        if(txn_list[TXN].type == ON_SINGLE)
        {
            single_txn_data_count--;
        } // if single //

        if(txn_list[TXN].pkt_idx < ONE_NET_MASTER_MAX_SEND_TXN)
        {
            pkt_list[txn_list[TXN].pkt_idx].in_use = FALSE;
        } // if the idx is valid //

        txn_list[TXN].pkt_idx = ONE_NET_MASTER_MAX_SEND_TXN;
        txn_list[TXN].txn.pkt_size = 0;
        txn_list[TXN].txn.pkt = 0;
    } // if sending //

    txn_list[TXN].txn.priority = ONE_NET_NO_PRIORITY;
    txn_list[TXN].type = ON_NO_TXN;
} // free_txn //


/*!
    \brief Adds a transaction to the queue.

    Transactions are added to the queue based on priority.

    \param[in] TXN The transaction to enqueue.

    \return TRUE if adding the transaction to the queue was successful.
            FALSE if unable to add the transaction to the queue.
*/
static BOOL enqueue_txn(const UInt8 TXN)
{
    const UInt8 NEW_PRIORITY = txn_list[TXN].txn.priority;

    UInt8 queued_priority;
    UInt8 cur, prev;

    if(TXN >= ONE_NET_MASTER_MAX_TXN)
    {
        return FALSE;
    } // if the parameter is invalid //

    for(prev = ONE_NET_MASTER_MAX_TXN, cur = txn_head;
      cur < ONE_NET_MASTER_MAX_TXN; prev = cur, cur = txn_list[cur].next)
    {
        queued_priority = txn_list[txn_head].txn.priority;

        // if the new transaction has a greater priority the the queued
        // transaction or if the 2 transactions have the same priority and
        // the queued transaction is not a single transaction, and the new
        // transaction is a single transaction or the new transaction is
        // scheduled before the queued transaction.
        if(NEW_PRIORITY > queued_priority
          || (NEW_PRIORITY == queued_priority
          && txn_list[cur].type != ON_SINGLE
          && (txn_list[TXN].type == ON_SINGLE
          || ont_get_timer(txn_list[TXN].txn.next_txn_timer)
          < ont_get_timer(txn_list[cur].txn.next_txn_timer))))
        {
            break;
        } // if the slot in the queue for the new transaction was found //
    } // loop through the queued transactions //

    txn_list[TXN].next = cur;
    if(prev < ONE_NET_MASTER_MAX_TXN && prev != TXN)
    {
        txn_list[prev].next = TXN;
    } // if prev points to a valid transaction //

    if(txn_head >= ONE_NET_MASTER_MAX_TXN || cur == txn_head)
    {
        txn_head = TXN;
    } // if the queue is empty or the new txn was put at the head //

    return TRUE;
} // enqueue_txn //


/*!
    \brief Removes a transaction from the queue

    \param[in] TXN The transaction to remove from the queue.

    \return void
*/
static void dequeue_txn(const UInt8 TXN)
{
    UInt8 ptr = txn_head;

    if(TXN >= ONE_NET_MASTER_MAX_TXN)
    {
        return;
    } // if the parameter is invalid //

    if(ptr == TXN)
    {
        txn_head = txn_list[ptr].next;
        txn_list[ptr].next = ONE_NET_MASTER_MAX_TXN;
        return;
    } // if the head is the TXN to be removed //

    for(; ptr < ONE_NET_MASTER_MAX_TXN; ptr = txn_list[ptr].next)
    {
        if(txn_list[ptr].next == TXN)
        {
            txn_list[ptr].next = txn_list[TXN].next;
            txn_list[TXN].next = ONE_NET_MASTER_MAX_TXN;
            return;
        } // if a match was found //
    } // loop through the queued transactions //
} // dequeue_txn //


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Dequeues a block or stream transaction.

    Dequeues a block or stream transaction based on the type and the DID of the
    other device involved in the transaction.

    \param[in] TYPE The type of transaction (either ON_BLOCK or ON_STREAM).
    \param[in] DID Device ID of the other device involved in the transaction.

    \return The transaction that was dequeued if found.
            ONE_NET_MASTER_MAX_TXN if the transaction was not found.
*/
static UInt8 dequeue_b_s_txn(const UInt8 TYPE,
  const on_encoded_did_t * const DID)
{
    UInt8 ptr;

    #ifdef _STREAM_MESSAGES_ENABLED
    if((TYPE != ON_BLOCK && TYPE != ON_STREAM) || !DID)
	#else
    if((TYPE != ON_BLOCK) || !DID)
	#endif
    {
        return ONE_NET_MASTER_MAX_TXN;
    } // if the parameter is invalid //

    for(ptr = txn_head; ptr < ONE_NET_MASTER_MAX_TXN; ptr = txn_list[ptr].next)
    {
        if(TYPE == txn_list[ptr].type && on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(txn_list[ptr].did)))
        {
            break;
        } // if the transaction matches //
    } // loop through the queued transactions

    return ptr;
} // dequeue_b_s_txn //
#endif


/*!
    \brief Returns TRUE if there are more single transactions queued for DID.

    TRUE is only returned if there is not a transaction of higher priority.
    If a valid transaction is found, it is moved to the head of the list.

    \param[in] DID The destinatin device id to search for.

    \return TRUE if there are more transactions for DID.  These transactions
                 must be of the highest priority level in the list.
            FALSE if there are no more queued transactions for DID or the
                  queued transactions are not at the highest priority level
                  in the list.
*/
static BOOL next_txn_queued_for_client(const on_encoded_did_t * const DID)
{
    // previous and current pointers
    UInt8 prev, cur;

    UInt8 head_priority, cur_priority;

    if(!DID || txn_head == ONE_NET_MASTER_MAX_TXN)
    {
        return FALSE;
    } // if the parameter is invalid or there are no queued transactions //

    cur = txn_head;
    prev = ONE_NET_MASTER_MAX_TXN;

    head_priority = txn_list[txn_head].txn.priority;

    while(cur != ONE_NET_MASTER_MAX_TXN)
    {
        cur_priority = txn_list[cur].txn.priority;

        // make sure a lower priority transaction is not being bumped in front
        // of higher priority transactions.
        if(cur_priority != head_priority)
        {
            return FALSE;
        } // if the priority is less than the transactions that preceed it //

        if(txn_list[cur].type == ON_SINGLE && txn_list[cur].txn.send
          && on_encoded_did_equal(
          (const on_encoded_did_t * const)&(txn_list[cur].did), DID))
        {
            if(cur != txn_head)
            {
                txn_list[prev].next = txn_list[cur].next;
                txn_list[cur].next = txn_head;
                txn_head = cur;
            } // if it is at the head of the queue //

            return TRUE;
        } // if another transaction was found //

        prev = cur;
        cur = txn_list[cur].next;
    } // loop through the queued transactions //

    return FALSE;
} // next_txn_queued_for_client //


/*!
    \brief Returns the index to the transaction for the Initiate Data Rate admin
      packet being sent.

    \param[in/out] txn_idx On input, contains the location of where to start
      looking for the transaction.  On output, contains the transaction if
      found, else contains ONE_NET_MASTER_MAX_TXN.

    \return void
*/
static void data_rate_txn(UInt8 * const txn_idx)
{
    if(!txn_idx)
    {
        return;
    } // if the parameter is invalid //

    for(; *txn_idx < ONE_NET_MASTER_MAX_TXN; *txn_idx = txn_list[*txn_idx].next)
    {
        if(txn_list[*txn_idx].type == ON_SINGLE
          && txn_list[*txn_idx].txn.msg_type == ON_ADMIN_MSG
          && txn_list[*txn_idx].txn.pkt[ON_ADMIN_TYPE_IDX]
          == ON_INIT_DATA_RATE_TEST)
        {
            return;
        } // if the pkt was found //
    } // loop to find initiate data rate test pkt //

    *txn_idx = ONE_NET_MASTER_MAX_TXN;
} // data_rate_txn //


/*!
    \brief Returns the next nonce to use when sending to a specific device.

    \param[in] DID The device ID of the device being sent the txn.

    \return ON_MAX_NONCE + 1 if the parameter is invalid or the DID is not a
      valid device, otherwise the next nonce to use when sending to DID.
*/
static UInt8 txn_nonce_for_client(const on_encoded_did_t * const DID)
{
    const on_client_t * CLIENT;

    if(!DID)
    {
        return ON_MAX_NONCE + 1;
    } // if the parameter is invalid //

    if(on_encoded_did_equal(DID, &ON_ENCODED_BROADCAST_DID))
    {
        // make up a random nonce since it is a broadcast //
        return one_net_prand(one_net_tick(), ON_MAX_NONCE);
    } // if the packet is being sent to the broadcast address //

    // make sure the address is valid //
    if((CLIENT = client_info(DID) ) == 0)
    {
        return ON_MAX_NONCE + 1;
    } // the CLIENT is not part of the network //

    return CLIENT->send_nonce;
} // txn_nonce_for_client //


/*!
    \brief Saves the parameters that need it to non-volatile storage.

    \param void

    \return void
*/
static void save_param(void)
{
    on_base_param->crc = one_net_compute_crc((UInt8 *)on_base_param
      + sizeof(on_base_param->crc), sizeof(on_base_param_t)
      + sizeof(on_master_param_t) + master_param->client_count
      * sizeof(on_client_t) - sizeof(on_base_param->crc), ON_PARAM_INIT_CRC,
      ON_PARAM_CRC_ORDER);

	// Derek_S 10/25/2010 - The call to one_net_master_save_settings()
	// changes a UInt8* called ONE_NET_PARAM from NULL (when uninitialized)
	// to a data block containing some master settings, including client count.
	// The CLI "list" command needs this pointer to be accurate, so that is one
	// reason to call one_net_master_save_settings, but not the only reason.
	// note that the call to the one_net_save_master_save_settings does
	// NOT actually save the settings to non-volatile memory.  That is done
	// with the "save" command.
    one_net_master_save_settings(nv_param, sizeof(on_base_param_t)
      + sizeof(on_master_param_t) + master_param->client_count
      * sizeof(on_client_t));
} // save_param //


#ifdef _ONE_NET_EVAL
    /*!
        \brief Forces the MASTER to save its parameters.

        This is a "protected" function used only by the eval project to force
        the MASTER to save it's parameters.  It is not even declared in the
        ONE-NET files.

        \param void

        \return void
    */
    void on_master_force_save(void)
    {
        save_param();
    } // on_master_force_save //
#endif // ifdef _ONE_NET_EVAL //

//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE-NET_MASTER
