//! \addtogroup ONE-NET
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
    \file one_net.c
    \brief Basis for ONE-NET implementation.

    This file is application independent.  The functionality implemented here
    is also independent of the device being a MASTER or CLIENT.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"

#include "one_net.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"
#include "one_net_xtea.h"

#include "one_net_status_codes.h"


#ifdef _ONE_NET_DEBUG
    #include "oncli.h"
    #include "oncli_str.h"
    #include "uart.h"
#endif

#ifdef _DEBUG_DELAY
    #include "oncli.h"
#endif

#ifdef _ONE_NET_DEBUG_STACK
    #include "uart.h"
#endif

#include "one_net_application.h"

#ifdef _ONE_NET_MH_CLIENT_REPEATER
    #ifndef _ONE_NET_MULTI_HOP
        #error "Need to define Multi_Hop if _ONE_NET_MH_CLIENT_REPEATER is defined!"
    #endif // ifndef _ONE_NET_MULTI_HOP //
    #ifdef _ONE_NET_SIMPLE_CLIENT
        #error "A Multi-Hop repeater cannot be a SIMPLE_CLIENT!"
    #endif // ifdef _ONE_NET_SIMPLE_CLIENT //
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{

#ifdef _ONE_NET_DEBUG_STACK
    UInt8 one_net_debug_stack_flags = 0xff;
#endif

//! Preamble
static const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
static const UInt8 SOF[] = {0x33};

enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};

#ifdef _ONE_NET_USE_ENCODING
const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0xB4, 0xB4};
#else
const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0x00, 0x00};
#endif


// Derek_S - adding a raw broadcase did constant
const one_net_raw_did_t ON_RAW_BROADCAST_DID = {0x00, 0x00};

//! @} ONE-NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_typedefs
//! \ingroup ONE-NET
//! @{

//! @} ONE-NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{

//! Packet Handlers
// Derek_S 6/19/2011 - since pkt_hdlr_t is STATIC, the initialization code is not needed.
// Removing, but keeping in as commented code for ease of reading.  See
// the on_pkt_hdlr_set_t struct for details.  Since static, everything is
// initialized to 0 automatically, so there are no longer three cases.  Just
// declare the variable.  We'll be initializing them elsewhere at boot-up anyway.
/*
#if defined(_STREAM_MESSAGES_ENABLED)
    static on_pkt_hdlr_set_t pkt_hdlr = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif defined(_BLOCK_MESSAGES_ENABLED)
    static on_pkt_hdlr_set_t pkt_hdlr = {0, 0, 0, 0, 0, 0, 0};
#else
    static on_pkt_hdlr_set_t pkt_hdlr = {0, 0, 0, 0, 0};
#endif*/
static on_pkt_hdlr_set_t pkt_hdlr;

#ifdef _IDLE
    //! Whether the current state is allowed to be changed
    static BOOL allow_set_state = TRUE;
#endif

//! The current state.  This is a "protected" variable.
on_state_t on_state = ON_INIT_STATE;

extern UInt8 nv_param[];

//! The base parameters for the device
on_base_param_t* const on_base_param = (on_base_param_t*) nv_param;

//! Keep track of the number of successful data rate test packets
static UInt8 data_rate_result;

//#ifdef _BLOCK_MESSAGES_ENABLED
    //! Indicates when a block transaction has been completed
    static BOOL block_complete = FALSE;
//#endif

#ifdef _STREAM_MESSAGES_ENABLED
    //! Points to the stream currently being sent.  This is incase the device is
    //! waiting for a response to the data it sent and has to send a single data
    //! nack, in which case we want it to go back to waiting for the stream
    //! response.
    static on_txn_t * cur_stream = 0;
#endif

#ifdef _ONE_NET_MH_CLIENT_REPEATER
    // fill in the preamble in the Multi-Hop packet to be sent.  The rest will
    // be filled in when the received Multi-Hop packet is read in over the rf
    // interface.
    static UInt8 mh_pkt[ONE_NET_MAX_ENCODED_PKT_LEN] = {0x55, 0x55, 0x55, 0x33};

    // Transaction for forwarding on MH packets.  If the number of MH packets
    // is changed, need to update the number of multi-hop packets value in
    // one_net_timer.h.
    static on_txn_t mh_txn = {ONE_NET_LOW_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
      #if defined(_ONE_NET_MASTER) || defined(_BLOCK_MESSAGES_ENABLED)
      TRUE, ONT_MH_TIMER,
      #endif
      #ifdef _BLOCK_MESSAGES_ENABLED
      0,
      #endif
      
      0, 0, sizeof(mh_pkt), mh_pkt, 0};
#endif


// variables for deciding whether a payload is new
static tick_t last_single_ack_time = 0;
static one_net_raw_did_t last_single_ack_did;
static UInt8 last_single_ack_payload[ONE_NET_RAW_SINGLE_DATA_LEN];


#ifdef _ONE_NET_MASTER
extern on_master_param_t* master_param;
#endif


//! @} ONE-NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_pub_var
//! \ingroup ONE-NET
//! @{


//! True if device is functioning as a master, false otehrwise.  It will
//! be res-set to true later during initialization if the device is
// functioning as a master.
BOOL deviceIsMaster = FALSE;

UInt8 single_data_queue_size = 0;
on_single_data_queue_t single_data_queue[SINGLE_DATA_QUEUE_SIZE];

//! Used when taking apart a transaction so we don't have to do
//! more than needed.  Needed when changing the payload of single
//! data packets.
BOOL parse_txn_payload = FALSE; // doesn't seem to be used anywhere?


//! @} ONE-NET_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================


//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{


#ifdef _BLOCK_MESSAGES_ENABLED
//! The number of bytes in the entire block transaction.  This value should be
//! set to 0 if there is no pending block transaction.
UInt16 block_data_len = 0;

//! The byte number of the start of the payload for the next block data transfer
UInt16 block_data_pos = 0;

//! Buffer to store the block data
UInt8* block_data;

//! did of device that the block transaction is being carried out with
one_net_raw_did_t block_did;

//! TRUE if sending the block transaction, false if receiving
BOOL send_block = FALSE;

//! The type of data being transfered in the block transaction
UInt8 block_data_type = NO_BLOCK_TXN;

//! The unit sending the block transaction
UInt8 block_src_unit;

//! The unit receiving the block transaction
UInt8 block_dst_unit;

//! If true, all of the block payload has been successfully transferred
BOOL block_byte_transfer_complete = FALSE;

//! If true, all of the block payload has been successfully transferred
BOOL block_txn_ack_rcvd = FALSE;
#endif


static BOOL check_for_clr_channel(void);

// packet reception functions
#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const SRC_DID, UInt8 * data);
#else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const SRC_DID);
#endif // else _ONE_NET_MH_CLIENT_REPEATER not defined //
static one_net_status_t rx_single_resp_pkt(on_txn_t ** txn);

#ifdef _TRIPLE_HANDSHAKE
static one_net_status_t rx_single_txn_ack(on_txn_t ** txn);
#endif

#ifndef _ONE_NET_SIMPLE_DEVICE
    static one_net_status_t rx_block_resp_pkt(on_txn_t ** txn);
    static one_net_status_t rx_block_txn_ack(on_txn_t ** txn);
	#ifdef _STREAM_MESSAGES_ENABLED
    static one_net_status_t rx_stream_resp_pkt(on_txn_t ** txn);
	#endif
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //
#ifdef _DATA_RATE
static one_net_status_t rx_data_rate(on_txn_t * const txn,
  const BOOL RECEIVER);
#endif

static one_net_status_t rx_single_data(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
#ifdef _BLOCK_MESSAGES_ENABLED
    static one_net_status_t rx_block_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
#endif
#ifdef _STREAM_MESSAGES_ENABLED
    static one_net_status_t rx_stream_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
#endif
static one_net_status_t rx_payload(UInt8 * const raw_pld,
  const UInt8 ENCODED_LEN);
static one_net_status_t rx_nonces(const UInt8* const encoded_pld,
  UInt8 * const txn_nonce, UInt8 * const next_nonce,
  on_nack_rsn_t* const nack_reason,
  on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload,
  const one_net_xtea_key_t * const key, const on_data_t data_type);


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t repeat_mh_pkt(on_txn_t ** txn);
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{

/*!
    \brief Parses the payload of an already existing data packet

    This function decodes, decrypts the payload of a data packet.  It will also
    compute and validate the crc, and return the sections of the header.

    \param[in/out] txn The transaction we are parsing
    \param[out] pld The pld portion of the payload.
    \param[in] DATA_TYPE Type of data being parsed (single, block, or stream)
    \param[in] KEY The xtea key to use to decrypt the packet.

    \return ONS_SUCCESS if parsing the payload was successful
            ONS_CRC_FAIL if the computed crc did not match the received crc.
            ONS_BAD_PARAM If the parameters are invalid.
            See on_decode and decrypt for more possible return values.
*/
one_net_status_t on_parse_txn_pld(on_txn_t * const txn, UInt8 * const pld,
  const UInt8 DATA_TYPE, const one_net_xtea_key_t * const KEY)
{
	UInt8 temp;
	one_net_status_t status;
	
    if(!txn || !pld)
	{
		return ONS_BAD_PARAM;
	}

	// Just handle single for new
	// TODO - handle more than just single
	
	// first decode
    if(ONS_SUCCESS != (status = on_decode(pld, &(txn->pkt[ON_PLD_IDX]), ON_ENCODED_SINGLE_PLD_SIZE)))
	{
		return status;
	}
    // expected_nonce is pretty much meaningless.  We don't know whehter we're sending
	// or receiving so we don't know the order we want.  Don't change the txn, just read it
	// into a temp variable and throw it away.
    return on_parse_pld(&temp, &temp, &(txn->msg_type), pld,
	         ON_SINGLE, KEY);
}


void single_ack_queued(const one_net_raw_did_t* const src_did,
    const UInt8* const raw_pld)
{
	one_net_memmove(last_single_ack_did, *src_did, ON_ENCODED_DID_LEN);
	one_net_memmove(last_single_ack_payload, raw_pld, ONE_NET_RAW_SINGLE_DATA_LEN);
	last_single_ack_time = one_net_tick();
}


BOOL single_data_is_repeat(const one_net_raw_did_t* const src_did,
    const UInt8* const raw_pld)
{
	if(one_net_tick() - last_single_ack_time > ONE_NET_DISTINCT_SINGLE_MESSAGE_PAUSE)
	{
		return FALSE;
	}
	
	return !((one_net_memcmp(*src_did, last_single_ack_did, ON_ENCODED_DID_LEN)) ||
	       (one_net_memcmp(raw_pld, last_single_ack_payload, ONE_NET_RAW_SINGLE_DATA_LEN)));
}


#if _ACK_NACK_LEVEL >= 50
BOOL nack_reason_is_fatal(const on_nack_rsn_t nack_reason)
{	
	switch(nack_reason)
	{
		case ON_NACK_RSN_DEVICE_FUNCTION_ERR:
		case ON_NACK_RSN_UNIT_FUNCTION_ERR:
		case ON_NACK_RSN_INVALID_UNIT_ERR:
		case ON_NACK_RSN_MISMATCH_UNIT_ERR:
		case ON_NACK_RSN_INVALID_LENGTH_ERR:
		case ON_NACK_RSN_BAD_DATA_ERR:
		case ON_NACK_RSN_TRANSACTION_ERR:
		case ON_NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED:
		case ON_NACK_RSN_BUSY:
        case ON_NACK_RSN_FATAL_ERR:
		    return TRUE;
		default:
		    break;
	}
	
	if(nack_reason >= ON_NACK_RSN_MIN_USR_FATAL && nack_reason <= ON_NACK_RSN_MAX_USR_FATAL)
	{
		return TRUE;
	}
	
	return FALSE;
}
#endif


#if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
/*!
    \ brief Adjusts the single data packet queue
	
	If there is a single data transaction in the queue and it's ready to
	be placed into a transaction, it will be placed into the single transaction spot
	
	Any unsent packets that have expired will be deleted.
	
	\param[out] queue_sleep_time The earliest time the queue could might add a transaction.
	
	\return TRUE if the queue is non-empty.
	       FALSE otherwise
*/
BOOL adjust_single_data_queue(tick_t* const queue_sleep_time)
{
	int i, j;
	one_net_status_t status;
	one_net_raw_did_t* raw_dst;
	tick_t sleep_time;
	UInt8 priority = ONE_NET_HIGH_PRIORITY;
	tick_t cur_tick = one_net_tick();
	BOOL queue_time_is_relevant = FALSE;
	on_single_data_queue_t* single_ptr = 0;
    BOOL raw_dst_is_null = FALSE;
	
	*queue_sleep_time = 0;
	
	// note that send_time equals 0 means send immediately, expire_time equals 0
	// means no expiration.

	// Go through and delete anything that has already expired.  If so, delete it.  Also
	// delete anything with no priority.  This actually should not be needed, but do it anyway.
	for(i = 0; i < single_data_queue_size; i++)
	{
		#if _SINGLE_QUEUE_LEVEL == MAX_SINGLE_QUEUE_LEVEL
		if(single_data_queue[i].priority == ONE_NET_NO_PRIORITY ||
		   (single_data_queue[i].expire_time > 0 && single_data_queue[i].expire_time < cur_tick))
		#else
		if(single_data_queue[i].priority == ONE_NET_NO_PRIORITY)
		#endif
		{
			if(i < single_data_queue_size - 1)
			{
				// expired.  Delete it
				one_net_memmove(&(single_data_queue[i]), &(single_data_queue[i+1]),
				    (single_data_queue_size - i - 1) * sizeof(on_single_data_queue_t));
			}
			
			single_data_queue_size--;
			i--;
		}
	}
	
	// now go through the list and see if any packets are ready.  Try high priority, then low priority.
	// Note that priority started initialized as HIGH_PRIORITY.  Go through the list twice, once for
	// each priority.  We'll change priority at the bottom of the outer loop
	for(j = 0; j < 2; j++)
	{
	    for(i = 0; i < single_data_queue_size; i++)
	    {
            raw_dst_is_null = FALSE;
		    single_ptr = &(single_data_queue[i]);

            #if _SINGLE_QUEUE_LEVEL >= MED_SINGLE_QUEUE_LEVEL
		    if((*single_ptr).send_time <= cur_tick)
		    {
			#endif
			    if((*single_ptr).priority == priority)
			    {
                    // it's ready and has the right priority.  Copy it into the single transaction.
					// Note: The last two parameters are NULL so there will be an attempt to place
					// it in the transaction.  If they weren't NULL, they'd be placed in THIS
					// queue.  They are already in this queue.  We'll place them in the actual
					// transaction or at least try to.
					
					// we stored the actual contents of the raw destination.  If it was passed to us
					// as NULL initially, we stored it as 0xFFFF(a nonsense value).  Check for 0xFFFF
					// first.  If it's equal to that, convert to a NULL pointer.
					raw_dst = &(single_ptr->raw_dst);
					if(((*raw_dst)[0] == 0xFF) && ((*raw_dst)[1] == 0xFF))
					{
                        raw_dst_is_null = TRUE;
					}
                    
				    status = (*one_net_send_single)(single_ptr->payload, single_ptr->data_len,
				        single_ptr->send_to_peer_list, priority, raw_dst_is_null ?
                        NULL: raw_dst, single_ptr->src_unit, NULL, NULL);
					
				    if(status == ONS_RSRC_FULL)
				    {
						*queue_sleep_time = 0;
						return TRUE; // busy.  None of the other attempts will work.  Should check the queue
						              // immediately again though.
					}
						
					// delete this from the list even if there was an error.
			        if(i < single_data_queue_size - 1)
			        {
				        one_net_memmove(&(single_data_queue[i]), &(single_data_queue[i+1]),
				            (single_data_queue_size - i - 1) * sizeof(on_single_data_queue_t));
			        }

			
			        single_data_queue_size--;
					i--;
					
				    if(status == ONS_SUCCESS)
				    {
						*queue_sleep_time = 0;
						return TRUE; // successful.  Return true.
					}
					 
					// some error occurred, so we deleted it.  But nothing's going to be
					// sent, so let's try the next one.  Don't return yet.
				}
			#if _SINGLE_QUEUE_LEVEL >= MED_SINGLE_QUEUE_LEVEL
			}
			else
			{
				sleep_time = (*single_ptr).send_time - cur_tick;
				if(!queue_time_is_relevant)
				{
					queue_time_is_relevant = TRUE;
					*queue_sleep_time = sleep_time;
				}
				else
				{
					if(sleep_time < *queue_sleep_time)
					{
						*queue_sleep_time = sleep_time;
					}
				}
			}
			#endif
		}
		 
		priority = ONE_NET_LOW_PRIORITY;
	}
	
	return queue_time_is_relevant; // nothing ready and able to send.
}


one_net_status_t place_in_single_queue(const UInt8* const data,
  const UInt8 data_len, const BOOL send_to_peer_list, const UInt8 priority,
  const one_net_raw_did_t* const raw_dst, const UInt8 src_unit,
  const tick_t* const send_time_from_now, const tick_t* const expire_time_from_now)
{
	tick_t cur_tick = one_net_tick();
    tick_t queue_sleep_time = 0;
	tick_t send_time_absolute = 0;
	tick_t expire_time_absolute = 0;
	on_single_data_queue_t* single_ptr = 0;

	// first check a few parameters
    if(!data || data_len > ONE_NET_RAW_SINGLE_DATA_LEN
      || priority < ONE_NET_LOW_PRIORITY || priority > ONE_NET_HIGH_PRIORITY
      || ((src_unit != ONE_NET_DEV_UNIT) && (src_unit != 0) &&
          ((SInt8) src_unit >= ONE_NET_NUM_UNITS))
      || (src_unit == ONE_NET_DEV_UNIT && !raw_dst))
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

	// check a couple more
	if(!send_time_from_now || !expire_time_from_now)
	{
		return ONS_BAD_PARAM;
	}

    #if _SINGLE_QUEUE_LEVEL == MAX_SINGLE_QUEUE_LEVEL
	// make sure the send_time is not after the expire time
	if((*send_time_from_now > *expire_time_from_now) && (*expire_time_from_now > 0))
	{
		return ONS_BAD_PARAM;
	}
	#endif
	
	#if _SINGLE_QUEUE_LEVEL >= MED_SINGLE_QUEUE_LEVEL
	if(*send_time_from_now > 0)
	{
		send_time_absolute = *send_time_from_now + cur_tick;
	}
	#endif
	
    #if _SINGLE_QUEUE_LEVEL == MAX_SINGLE_QUEUE_LEVEL	
	if(*expire_time_from_now > 0)
	{
		expire_time_absolute = *expire_time_from_now + cur_tick;
	}
	#endif

    // might as well update the queue now.  What could it hurt, plus it might
	// actually get this packet in there a little more quickly.
	adjust_single_data_queue(&queue_sleep_time);
	
	if(single_data_queue_size >= SINGLE_DATA_QUEUE_SIZE)
	{
		return ONS_RSRC_FULL;
	}

	single_ptr = &(single_data_queue[single_data_queue_size]);
	
	// copy everything into single_ptr.
	one_net_memmove(single_ptr->payload, data, ONE_NET_RAW_SINGLE_DATA_LEN);
	single_ptr->data_len = data_len;
	single_ptr->send_to_peer_list = send_to_peer_list;
	single_ptr->priority = priority;
	
	if(raw_dst == NULL)
	{
		// store as 0xFFFF, which signifies NULL.
		(single_ptr->raw_dst)[0] = 0xFF;
		(single_ptr->raw_dst)[1] = 0xFF;
	}
	else
	{
		// store the raw did's contents.
		(single_ptr->raw_dst)[0] = (*raw_dst)[0];
		(single_ptr->raw_dst)[1] = (*raw_dst)[1];     
	}

    single_ptr->src_unit = src_unit;
	
	#if _SINGLE_QUEUE_LEVEL >= MED_SINGLE_QUEUE_LEVEL
	single_ptr->send_time = send_time_absolute;
	#endif
	#if _SINGLE_QUEUE_LEVEL == MAX_SINGLE_QUEUE_LEVEL	
	single_ptr->expire_time = expire_time_absolute;
	#endif
	
	single_data_queue_size++;
	
	// everything is copied.  Why not try adjusting the queue again?  We might
	// be able to send right away.
    adjust_single_data_queue(&queue_sleep_time);
	return ONS_SUCCESS;
}
#endif


/*!
    \brief Initializes ONE-NET.

    \param[in] PKT_HDLR Contains the necessary callbacks ONE-NET will need
      to make.

    \return void
*/
void one_net_init(const on_pkt_hdlr_set_t * const PKT_HDLR)
{
    one_net_set_channel(on_base_param->channel);
    on_base_param->features = ON_FEATURES;
    one_net_memmove(&pkt_hdlr, PKT_HDLR, sizeof(pkt_hdlr));
} // one_net_init //


#ifdef _IDLE
BOOL set_on_state(UInt8 new_on_state)
{
	if(!allow_set_state)
	{
		return FALSE;
	}
	on_state = new_on_state;
	return TRUE;
}


void set_allow_set_state(BOOL allow)
{
	allow_set_state = allow;
}
#endif


/*!
    \brief Compares the NID passed in to the devices own NID

    \param[in] NID The encoded NID to check if it is the device's.

    \return TRUE if the NIDs match.
            FALSE if the NIDs do not match.
*/
BOOL on_is_my_NID(const on_encoded_nid_t * const NID)
{
    if(!NID)
    {
        return FALSE;
    } // if parameter is invalid //

    return !one_net_memcmp(*NID, on_base_param->sid, ON_ENCODED_NID_LEN);
} // on_is_my_NID //


/*!
    \brief Compares the DID passed in to the devices own DID

    \param[in] DID The encoded DID to check if it is the device's.

    \return TRUE if the DIDs match.
            FALSE if the DIDs do not match.
*/
BOOL on_is_my_DID(const on_encoded_did_t * const DID)
{
    if(!DID)
    {
        return FALSE;
    } // if parameter is invalid //

    return !one_net_memcmp(*DID, &(on_base_param->sid[ON_ENCODED_NID_LEN]),
        ON_ENCODED_DID_LEN);
} // on_is_my_DID //


/*!
    \brief Checks whether a did is a broadcast did / uninitialized
	
	Note that the DID may either be raw or encoded, hence the parameter is UInt8*
	rather than on_encoded_did_t* or one_net_raw_did_t*

    \param[in] did The did to check

    \return TRUE if did is a broadcast did
            FALSE if did is not a broadcast did
*/
BOOL did_is_broadcast(const UInt8* const did)
{
    // sometimes "encoded" dids are stored as uninitialized ({0, 0}).  We want to
	// make sure that these are interpreted as "not taken"/ unitialized / broadcast
	if(!did)
	{
        return FALSE;
	}
	if(did[0] == ON_ENCODED_BROADCAST_DID[0] && did[1] == ON_ENCODED_BROADCAST_DID[1])
	{
        return TRUE;
	}
	if(did[0] == ON_RAW_BROADCAST_DID[0] && did[1] == ON_RAW_BROADCAST_DID[1])
	{
        return TRUE;
	}
	
	return FALSE;
}


/*!
    \brief Compares two encoded dids and sees which is "smaller"

    A broadcast did is considered a "large" id.
	One DID is smaller than the other if the raw did is smaller.
	
    \param[in] enc_did1 First encoded did to compare
    \param[in] enc_did2 Second encoded did to compare
    
    \return negative number if did1 is "smaller" than did2
            postive number if did1 is "larger" than did2
            0 if did1 and did2 have the same values
*/
int enc_did_cmp(const on_encoded_did_t* const enc_did1, const on_encoded_did_t* const enc_did2)
{
	// TODO - check for valid parameters, check for return values
    one_net_raw_did_t raw_did1, raw_did2;
	BOOL did1IsBroadcast, did2IsBroadcast;
	
	if(!enc_did1 || !enc_did2)
	{
        // TODO - Return value of this should be what?  Just return 0 for now
		return 0;
	}
	
	did1IsBroadcast = did_is_broadcast(*enc_did1);
	did2IsBroadcast = did_is_broadcast(*enc_did2);

    if(did1IsBroadcast && did2IsBroadcast)
	{
	    return 0;
	}
	else if(did1IsBroadcast)
	{
		return 1;
	}
	else if(did2IsBroadcast)
	{
		return -1;
	}
		
	// TODO - what if these don't decode?
    on_decode(raw_did1, *enc_did1, ON_ENCODED_DID_LEN);
    on_decode(raw_did2, *enc_did2, ON_ENCODED_DID_LEN);

	if(raw_did1[0] == raw_did2[0])
	{
		return (int) raw_did1[1] - (int) raw_did2[1];
	}

    return (int) raw_did1[0] -  (int) raw_did2[0];
}

/*!
    \brief Compares two encoded Device IDs.

    \param[in] LHS The left hand side of the compare equation.
    \param[in] RHS The right hand side of the compare equation.

    \return TRUE if the DIDs match.
            FALSE if the DIDs do not match.
*/
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS)
{
	if(!LHS || !RHS)
	{
        return FALSE;
	}
    return (enc_did_cmp(LHS, RHS) == 0);
} // on_encoded_did_equal //



/*!
    \brief Checks the DID to see if it is one that this device listens to.

    \param[in] The encoded DID to check.

    \return ONS_SUCCESS if DID is for this device.
            ONS_BROADCAST_ADDR if DID is the broadcast address
            ONS_DID_FAILED if the DID is unknown
*/
one_net_status_t on_validate_dst_DID(const on_encoded_did_t * const DID)
{
    if(on_encoded_did_equal(DID, (const on_encoded_did_t * const)
      &(on_base_param->sid[ON_ENCODED_NID_LEN])))
    {
        return ONS_SUCCESS;
    } // if DID == my_did //

    if(on_encoded_did_equal(DID, &ON_ENCODED_BROADCAST_DID))
    {
        return ONS_BROADCAST_ADDR;
    } // if DID == broadcast //

    return ONS_DID_FAILED;
} // on_valid_dst_DID //


/*!
    \brief Encrypt the data passed in.

    data should be formatted such that the first byte is the location where
    the 8-bit crc is going to go, then the next N bytes are the data that is
    being encrypted, and there should be room for 1 extra byte on the end
    for the encryption type.  In short, data should be the format of the
    payload field for the appropriate data type.

    \param[in] DATA_TYPE Type of data to be sent.  (see on_data_t in one_net.h)
    \param[in/out] data The data to encrypt
    \param[in] KEY The XTEA key used to encrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
one_net_status_t on_encrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    #ifdef _STREAM_MESSAGES_ENABLED
    // get the number of XTEA rounds
    if(DATA_TYPE != ON_STREAM)
    {
	#endif
        switch(on_base_param->single_block_encrypt)
        {
            case ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE:
            {
                rounds = 0;
                break;
            } // no encryption //

            case ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32:
            {
                rounds = ON_XTEA_32_ROUNDS;
                break;
            } // xtea with 32 rounds //

            default:
            {
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch on encryption type //

        data[payload_len - 1] = on_base_param->single_block_encrypt;
	#ifdef _STREAM_MESSAGES_ENABLED
    } // if not stream //
        else
        {
            switch(on_base_param->stream_encrypt)
            {
                case ONE_NET_STREAM_ENCRYPT_NONE:
                {
                    rounds = 0;
                    break;
                } // no encryption //

                case ONE_NET_STREAM_ENCRYPT_XTEA8:
                {
                    rounds = ON_XTEA_8_ROUNDS;
                    break;
                } // xtea with 8 rounds //

                default:
                {
                    return ONS_INTERNAL_ERR;
                    break;
                } // default //
            } // switch on encryption type //

            data[payload_len - 1] = on_base_param->stream_encrypt;
        } // else stream //
    #endif // if _STREAM_MESSAGES_ENABLED is not defined //

#ifdef _ONE_NET_USE_ENCRYPTION
    if(rounds)
    {
        UInt8 i;

        // -1 since we're not enciphering the byte that has the 2 bits for
        // the encryption type used.
        for(i = 0; i < payload_len - 1; i += ONE_NET_XTEA_BLOCK_SIZE)
        {
            one_net_xtea_encipher(rounds, &(data[i]), KEY);
        } // process 8 bytes at a time //
    } // if  rounds //
#endif

    return ONS_SUCCESS;
} // on_encrypt //


/*!
    \brief Decrypt the data passed in.

    The last 2 bits of the data should contain the method used to decrypt the
    packet.  These 2 bits are the high 2 bits of the last byte, as not all of
    the bits in the last byte are used.

    \param[in] DATA_TYPE Type of data to decrypted.  (see on_data_t in
      one_net.h)
    \param[in/out] data The data to decrypt
    \param[in] key The XTEA key used to decrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
one_net_status_t on_decrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data)
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    // get the number of XTEA rounds
	#ifdef _STREAM_MESSAGES_ENABLED
    if(DATA_TYPE != ON_STREAM)
    {
	#endif
        switch(data[payload_len - 1])
        {
            case ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE:
            {
                rounds = 0;
                break;
            } // no encryption //

            case ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32:
            {
                rounds = ON_XTEA_32_ROUNDS;
                break;
            } // xtea with 32 rounds //

            default:
            {
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch on encryption type //
	#ifdef _STREAM_MESSAGES_ENABLED
    } // if not stream //
       else
        {
            switch(data[payload_len - 1])
            {
                case ONE_NET_STREAM_ENCRYPT_NONE:
                {
                    rounds = 0;
                    break;
                } // no encryption //

                case ONE_NET_STREAM_ENCRYPT_XTEA8:
                {
                    rounds = ON_XTEA_8_ROUNDS;
                    break;
                } // xtea with 8 rounds //

                default:
                {
                    return ONS_INTERNAL_ERR;
                    break;
                } // default //
            } // switch on encryption type //
        } // else stream //
    #endif // ifdef _STREAM_MESSAGES_ENABLED //

#ifdef _ONE_NET_USE_ENCRYPTION    
    if(rounds)
    {
        UInt8 i;

        // -1 since we're not enciphering the byte that has the 2 bits for
        // the encryption type used.
        for(i = 0; i < payload_len - 1; i += ONE_NET_XTEA_BLOCK_SIZE)
        {
            one_net_xtea_decipher(rounds, &(data[i]), KEY);
        } // process 8 bytes at a time //
    } // if  rounds //
#endif

    return ONS_SUCCESS;
} // on_decrypt //


/*!
    \brief Receives data packets only.

    This function is to only receive Single, Repeat Single, Block, Repeat Block,
    Stream, and Data Rate Test packets.  All other packets are discarded.

    \param[in] EXPECTED_SRC_DID The encoded DID of the device that this device
      expects to receive a packet from.  If this is set to the broadcast
      address, this device does not expect a packet from anyone in particular.
    \param[in/out] The current transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized properly.
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_READ_ERR If there was an error while reading the packet.
            ONS_BAD_PKT_TYPE If a packet type that was not expected was
              received.
            ONS_INTERNAL_ERR if control reaches the end of the function.
            ONS_INVALID_DATA If data received is not valid.
            For more return codes, see rx_pkt_addr.
*/
one_net_status_t on_rx_data_pkt(const on_encoded_did_t * const EXPECTED_SRC_DID,
  on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 pid;

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!EXPECTED_SRC_DID || !txn)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //



    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr(EXPECTED_SRC_DID, &src_did, mh_pkt))
          != ONS_SUCCESS)
        {
            if(status == ONS_DID_FAILED)
            {
                status = repeat_mh_pkt(txn);
            } // if receiving a Multi-Hop packet //

            return status;
        } // if the packet is not for this device //
    #else
        if((status = rx_pkt_addr(EXPECTED_SRC_DID, &src_did)) != ONS_SUCCESS)
        {
            return status;
        } // if the packet is not for this device //
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //


    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    switch(pid)
    {
        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            status = rx_single_data(pid,
              (const on_encoded_did_t * const)&src_did, txn);
            break;
        } // single data case //

        #ifdef _BLOCK_MESSAGES_ENABLED
            case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:
            {
                status = rx_block_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);
                break;
            } // block data case //
			#ifdef _STREAM_MESSAGES_ENABLED
            case ONE_NET_ENCODED_STREAM_DATA:
            {
                status = rx_stream_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);
                break;
            } // stream data case //
			#endif
        #endif // if _BLOCK_MESSAGES_ENABLED is defined //

        default:
        {
            return ONS_BAD_PKT_TYPE;
        } // default //
    } // switch(pid) //

    return status;
} // on_rx_data_pkt //


/*!
    \brief Builds an admin packet.

    \param[out] pkt Returns the ready to send packet.
    \param[in/out] pkt_size On input, contains the size of pkt.
                            On output, contains the size of the packet.
    \param[in] MSG_TYPE The type of message being sent.  This can be either
      ON_ADMIN_MSG or ON_EXTENDED_ADMIN_MSG
    \param[in] MSG_ID The type of admin packet being sent.
    \param[in] ENCODED_DST The encoded DID of the device receiving the packet.
    \param[in] TXN_NONCE The nonce the receiver expects.
    \param[in] RESP_NONCE The nonce expected from the receiver when the
                 response is received.
    \param[in] RAW_DATA The data that goes along with the MSG_ID.
    \param[in] DATA_LEN The length of RAW_DATA in bytes.
    \param[in] KEY The key to use to encrypt the data.

    \return ONS_SUCCESS If building the packet was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_build_admin_pkt(UInt8 * pkt, UInt8 * const pkt_size,
  const UInt8 MSG_TYPE, const UInt8 MSG_ID,
  const on_encoded_did_t * const ENCODED_DST,
  const UInt8 TXN_NONCE, const UInt8 RESP_NONCE,
  const UInt8 * const RAW_DATA, const UInt8 DATA_LEN,
  const one_net_xtea_key_t * const KEY)
{
    UInt8 admin_pkt[ONE_NET_RAW_SINGLE_DATA_LEN];

    if(!pkt || !pkt_size
      || *pkt_size < ON_ENCODED_SINGLE_DATA_LEN
	  #ifdef _STREAM_MESSAGES_ENABLED
      || (MSG_TYPE != ON_ADMIN_MSG && MSG_TYPE != ON_EXTENDED_ADMIN_MSG)
	  #else
      || (MSG_TYPE != ON_ADMIN_MSG)
	  #endif
      || !ENCODED_DST || !RAW_DATA || DATA_LEN > ON_MAX_ADMIN_PLD_LEN
      || !KEY) 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    admin_pkt[ON_ADMIN_MSG_ID_IDX] = MSG_ID;
    one_net_memmove(&(admin_pkt[ON_ADMIN_DATA_IDX]), RAW_DATA, DATA_LEN);

    return on_build_data_pkt(pkt, pkt_size, MSG_TYPE,
      ONE_NET_ENCODED_SINGLE_DATA, ENCODED_DST, TXN_NONCE, RESP_NONCE,
      admin_pkt, sizeof(admin_pkt), KEY);
} // on_build_admin_pkt //


/*!
    \brief Builds a data packet for single, block, or stream transaction.

    \param[out] pkt Pointer to location to store the packet
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] MSG_TYPE The type of message (see msg_type_t)
    \param[in] PID The encoded packet id (must be single, block, or stream,
                     or one of the repeat packets).
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] TXN_NONCE The nonce the receiver expects.
    \param[in] RESP_NONCE The nonce expected from the receiver when the
                 response is received.
    \param[in] RAW_DATA Any packet specific data that must be sent with the pkt.
    \param[in] DATA_LEN The length of RAW_DATA (in bytes).
    \param[in] KEY The key used to encrypt the data.

    \return See on_build_pkt for return types.
*/
one_net_status_t on_build_data_pkt(UInt8 * pkt, UInt8 * const pkt_size,
  const UInt8 MSG_TYPE, const UInt8 PID,
  const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
  const UInt8 RESP_NONCE, const UInt8 * const RAW_DATA,
  const UInt8 DATA_LEN, const one_net_xtea_key_t * const KEY) 
{
    one_net_status_t status;

    // + 1 to include byte needed for 2 bits for encryption method.
    UInt8 raw_pld[ON_MAX_RAW_PLD_LEN + 1];
    UInt8 data_type;                // The type of data being sent    
    UInt8 pld_word_size;            // Size of the payload in 6 or 8 bit words
    UInt8 raw_pld_len = 0;          // size of the raw payload in bytes

    if(!pkt || !pkt_size || !ENCODED_DST || !RAW_DATA || !DATA_LEN || !KEY)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    switch(PID)
    {
        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            if(*pkt_size < ON_ENCODED_SINGLE_DATA_LEN
              || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN)
            {
                return ONS_BAD_PARAM;
            } // if parameter is invalid //

            data_type = ON_SINGLE;
            raw_pld_len = ONE_NET_RAW_SINGLE_DATA_LEN;
            pld_word_size = ON_ENCODED_SINGLE_PLD_SIZE;
            break;
        } // single data case //

        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:     // fall through
			#ifdef _STREAM_MESSAGES_ENABLED
            case ONE_NET_ENCODED_STREAM_DATA:
			#endif
            {
                if(*pkt_size < ONE_NET_MAX_ENCODED_PKT_LEN
                  || DATA_LEN > ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
                {
                    return ONS_BAD_PARAM;
                } // if parameter is invalid //
                #ifdef _STREAM_MESSAGES_ENABLED
                if(PID == ONE_NET_ENCODED_STREAM_DATA)
                {
                    data_type = ON_STREAM;
                } // if a stream packet //
                else
				#endif
                {
                    data_type = ON_BLOCK;
                } // else a block packet //
                raw_pld_len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
                pld_word_size = ON_ENCODED_BLOCK_STREAM_PLD_SIZE;
                break;
            } // single data case //
        #endif // _ONE_NET_SIMPLE_DEVICE //

        default:
        {
            return ONS_BAD_PARAM;
            break;
        } // default case //
    } // switch(PID) //

    // build the packet
    raw_pld[ON_PLD_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    raw_pld[ON_PLD_RESP_NONCE_HIGH_IDX] |= (RESP_NONCE
      >> ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK;
    raw_pld[ON_PLD_RESP_NONCE_LOW_IDX] = (RESP_NONCE << ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_BUILD_LOW_MASK;
    raw_pld[ON_PLD_MSG_TYPE_IDX] |= MSG_TYPE;
    one_net_memmove(&(raw_pld[ON_PLD_DATA_IDX]), RAW_DATA, DATA_LEN);

    // compute the crc
    raw_pld[0] = (UInt8)one_net_compute_crc(&(raw_pld[ON_PLD_TXN_NONCE_IDX]),
      raw_pld_len + ON_RAW_PLD_HDR_SIZE - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC,
      ON_PLD_CRC_ORDER);

    if((status = on_encrypt(data_type, raw_pld, KEY, data_type == ON_SINGLE ?
      ON_RAW_SINGLE_PLD_SIZE : ON_RAW_BLOCK_STREAM_PLD_SIZE)) == ONS_SUCCESS)
    {
        status = on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_pld,
          pld_word_size);
    } // if encrypting was not successful //

    return status;
} // on_build_data_pkt //


/*!
    \brief Builds a response packet.

    \param[out] pkt Pointer to location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] PID The encoded packet id.
    \param[in] nack_reason If non-NULL the reason for the NACK we're sending
	\param[in] ack_nack_handle How the ACK/NACK "payload" (if any) should be interpreted
	\param[in] ack_nack_payload The "payload"(if any) for this ACK or NACK
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] TXN_NONCE The transaction nonce associated with this packet.
    \param[in] EXPECTED_NONCE The nonce the receiver should use the next time it
      sends to this device.
    \param[in] KEY Key to use to encrypt this packet
    \param[in] data_type ON_SINGLE, ON_BLOCK, ON_STREAM, etc.  Used to figure out
	                           how many rounds of encryption we need. 

    \return ONS_BAD_PARAM if any of the parameters are invalid.
            See on_build_pkt for more return types.
*/
one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
   const UInt8 PID, const on_nack_rsn_t* const nack_reason, const on_ack_nack_handle_t* const ack_nack_handle,
   const ack_nack_payload_t* const ack_nack_payload,
   const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
   const UInt8 EXPECTED_NONCE, const one_net_xtea_key_t * const KEY,
   const on_data_t data_type)
{
	UInt8 i;
	UInt8 raw_data[ON_RAW_ACK_NACK_PLD_SIZE];
	UInt32 uint32;
	one_net_status_t status;
    
    // TODO - Any other relevant ACK packets?  Stay-alive?
    #ifdef _BLOCK_MESSAGES_ENABLED
	BOOL isACK = (PID == ONE_NET_ENCODED_SINGLE_DATA_ACK ||
      PID == ONE_NET_ENCODED_BLOCK_DATA_ACK);
    #else
	BOOL isACK = (PID == ONE_NET_ENCODED_SINGLE_DATA_ACK);
    #endif
	on_ack_nack_handle_t handle = ON_ACK; // this also corresponds to ON_NACK
	
    if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
      || TXN_NONCE > ON_MAX_NONCE || EXPECTED_NONCE > ON_MAX_NONCE)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

	
	if(ack_nack_payload != 0)
	{
	    if(ack_nack_handle != 0)
    	{
		    // override handle with the passed value
		    handle = *ack_nack_handle;
	    }
	}

	// random padding for more security
	// TODO - more efficiency.  Only random pad if there's no ack/nack payload
	for(i = 3; i < ON_RAW_ACK_NACK_PLD_SIZE - 1; i++)
	{
		raw_data[i] = one_net_prand(one_net_tick(), 255);
	}

	// TODO - save some code space and use the sizeof variable when dealing with the
	// ack_nack_payload?  Some clever math might save a few bytes.
	
	
	switch(handle) /* This may be a nack, but the valid nack values correspond to
	                  their ack counterparts so it's OK */
	{
		case ON_ACK: // also corresponds to ON_NACK
		    break; // do nothing.  We already have the random padding here.  There's no data.
		#if _ACK_NACK_LEVEL	>= 95
		case ON_ACK_DATA: // also corresponds to ON_NACK_DATA
		    // it's a payload
			if(isACK)
			    one_net_memmove(&raw_data[3], (*ack_nack_payload).ack_payload, 5);
			else
			    one_net_memmove(&raw_data[4], (*ack_nack_payload).nack_payload, 4);			
			break;
		#endif
		#if _ACK_NACK_LEVEL	>= 90
		case ON_ACK_VALUE: // also corresponds to ON_NACK_VALUE
		    // Bytes 4 through 7 are the 32-bit integer, MSB first
			if(isACK)
			{
				uint32 = (*ack_nack_payload).ack_value.uint32;
				raw_data[3] = (*ack_nack_payload).ack_value.uint8;
			}
			else
			{
				uint32 = (*ack_nack_payload).nack_value;
			}
			
			one_net_int32_to_byte_stream(uint32, &(raw_data[4]));
			break;
		#endif
		#if _ACK_NACK_LEVEL == 255
		case ON_ACK_TIME_MS: // also corresponds to ON_NACK_TIME_MS
		    // Bytes 4 through 7 are the 32-bit integer, MSB first
			uint32 = (*ack_nack_payload).ack_time_ms;
			one_net_int32_to_byte_stream(uint32, &(raw_data[4]));
			break;
		#endif
		#if _ACK_NACK_LEVEL	>= 80
		case ON_ACK_STATUS: // this is usually a response to a query/fast poll
		   // this had better be an ACK.  Therefore there should be no nack_reason
		   if(nack_reason != 0)
		   {
			   return ONS_BAD_RAW_DATA;
		   }
		   
		   one_net_memmove(&raw_data[ON_PLD_DATA_IDX], (*ack_nack_payload).status_resp, ONE_NET_RAW_SINGLE_DATA_LEN);
		   break;
		#endif
		default:
		   return ONS_BAD_RAW_DATA;
	}
	
	// fill in nonces
    raw_data[1] = (TXN_NONCE << 2);
	raw_data[1] |= ((EXPECTED_NONCE & 0x30) >> 4);
	raw_data[2] = (EXPECTED_NONCE & 0x0F) << 4;
	
	// fill in the ack/nack handle (The 4 LSB of raw data byte 2)
	raw_data[2] |= handle;
	
	// if there is a NACK reason, put it in the 6 most significant bits of raw_data[3]
	if(nack_reason != 0)
	{
		if(*nack_reason > ON_NACK_RSN_MAX_NACK_RSN_VALUE)
		{
			return ONS_BAD_PARAM;
		}
		
		raw_data[3] = (*nack_reason) << 2;
	}
	
	// crc
	raw_data[0] = (UInt8)one_net_compute_crc(&(raw_data[1]),
        ON_RAW_ACK_NACK_PLD_SIZE - 2, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
	
	if((status = on_encrypt(data_type, raw_data, KEY, ON_RAW_ACK_NACK_PLD_SIZE))
      != ONS_SUCCESS)
	{
		return status;
	}

    return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_data,
               ON_ENCODED_ACK_NACK_PLD_SIZE);
} // on_build_response_pkt //


#ifdef _DATA_RATE
/*!
    \brief Builds a data rate packet.

    If Multi-Hop functionality is enabled, then a ONE_NET_ENCODED_DATA_RATE_TEST
    packet will be sent if MAX_HOPS is 0, otherwise a
    ONE_NET_ENCODED_MH_DATA_RATE_TEST packet will be sent.

    \param[out] pkt Pointer to the location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] data_rate The raw data rate.

    \return ONS_SUCCESS if the packet was successfully built
            ONS_BAD_PARAM If any of the parameters are invalid.
            See on_build_pkt for more possible return values.
*/
one_net_status_t on_build_data_rate_pkt(UInt8 * pkt, UInt8 * const pkt_size,
  const on_encoded_did_t * const ENCODED_DST, UInt8 data_rate)
{
    one_net_status_t status;
    UInt8 i;

    if(!pkt || !pkt_size || *pkt_size < ON_DATA_RATE_PKT_LEN
      || !ENCODED_DST)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    data_rate <<= ON_DATA_RATE_SHIFT;

    if((status = on_build_pkt(pkt, pkt_size, ONE_NET_ENCODED_DATA_RATE_TEST,
      ENCODED_DST, &data_rate, sizeof(data_rate))) != ONS_SUCCESS)
    {
        return status;
    } // if building the packet was not successful //

    // The test pattern still needs to be added
    for(i = 0; i < ON_TEST_PATTERN_SIZE; i++)
    {
        pkt[(*pkt_size)++] = ON_TEST_PATTERN;
    } // loop to add the test pattern //

    return ONS_SUCCESS;
} // on_build_data_rate_pkt //
#endif


/*!
    \brief Builds a complete packet that is ready to be sent out the rf channel.

    If Multi-Hop functionality is enabled, the HOPS field is added to the end of
    the packet if MAX_HOPS is non-zero, otherwise the HOPS field is omitted.

    \param[out] pkt Pointer to the location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] PID The encoded packet id.
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] RAW_DATA Any packet specific data that must be sent with the
      packet.
    \param[in] DATA_WORD_SIZE The size of RAW_DATA in 6 or 8 bit words.

    \return ONS_SUCCESS if the packet was successfully built
            ONS_BAD_PARAM if any of the parameters are invalid
            See on_encode for more return types.
*/
one_net_status_t on_build_pkt(UInt8 * pkt, UInt8 * const pkt_size,
  const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
  const UInt8 * const RAW_DATA, const UInt8 DATA_WORD_SIZE)
{
	UInt8 i;
	
    enum
    {
        //! size of the header (Preamble, SOF, addresses, PID)
        HDR_SIZE = 15
    };

    one_net_status_t rv = ONS_SUCCESS;

    if(!pkt || !pkt_size || !ENCODED_DST
      || *pkt_size < HDR_SIZE + DATA_WORD_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    // copy the preamble to the packet
    one_net_memmove(pkt, PREAMBLE, sizeof(PREAMBLE));
    pkt += sizeof(PREAMBLE);

    // copy the start of frame to the packet
    one_net_memmove(pkt, SOF, sizeof(SOF));
    pkt += sizeof(SOF);

    // add the destination address
    one_net_memmove(pkt, *ENCODED_DST, ON_ENCODED_DID_LEN);
    pkt += ON_ENCODED_DID_LEN;

    // add the NID and SRC DID (which happens to be this devices SID)
    one_net_memmove(pkt, on_base_param->sid, sizeof(on_base_param->sid));
    pkt += sizeof(on_base_param->sid);

    // add the pid
    *pkt++ = PID;

    if(RAW_DATA)
    {
        rv = on_encode(pkt, RAW_DATA, DATA_WORD_SIZE);
        #ifdef _ONE_NET_DEBUG
        #if 0
            if (PID == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN)
            {
                one_net_debug(ONE_NET_DEBUG_NACK_WITH_RSN2_TEST, pkt, DATA_WORD_SIZE);
            }
        #endif
        #endif
    } // if there is data associated with this packet //

    *pkt_size = HDR_SIZE + DATA_WORD_SIZE;
    return rv;
} // on_build_pkt //


/*!
    \brief Parses the payload field of a raw data packet.

    This function decodes, decrypts the payload of a data packet.  It will also
    compute and validate the crc, and return the sections of the header.

    \param[out] txn_nonce The transaction nonce.
    \param[out] resp_nonce The nonce to be used in the response packet.
    \param[out] msg_type The type of message contained in the packet.
    \param[out] pld The pld portion of the payload.
    \param[in] DATA_TYPE Type of data being parsed (single, block, or stream)
    \param[in] KEY The xtea key to use to decrypt the packet.

    \return ONS_SUCCESS if parsing the payload was successful
            ONS_CRC_FAIL if the computed crc did not match the received crc.
            ONS_BAD_PARAM If the parameters are invalid.
            See on_decode and decrypt for more possible return values.
*/
one_net_status_t on_parse_pld(UInt8 * const txn_nonce, UInt8 * const resp_nonce,
  UInt8 * const msg_type, UInt8 * const pld, const UInt8 DATA_TYPE,
  const one_net_xtea_key_t * const KEY)
{
    one_net_status_t status;

    UInt8 raw_pld_len;

    if(!txn_nonce || !resp_nonce || !msg_type || !pld || !KEY)
    {
        return ONS_BAD_PARAM;
    } // the parameters are invalid //

    switch(DATA_TYPE)
    {
        case ON_SINGLE:
        {
            // -1 for byte that contains the encryption bits
            raw_pld_len = ON_RAW_SINGLE_PLD_SIZE - 1;
            break;
        } // single case //

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ON_BLOCK:
		#ifdef _STREAM_MESSAGES_ENABLED
        case ON_STREAM:
		#endif
        {
            // -1 for byte that contains the encryption bits
            raw_pld_len = ON_RAW_BLOCK_STREAM_PLD_SIZE - 1;
            break;
        } // block, stream case //
        #endif

        default:
        {
            return ONS_BAD_PARAM;
            break;
        } // default case //
    } // switch(DATA_TYPE)

    if((status = on_decrypt(DATA_TYPE, pld, KEY, raw_pld_len + 1))
      != ONS_SUCCESS)
    {
        return status;
    } // if decrypting the packet is not successful //

    if((UInt8)one_net_compute_crc(&(pld[ON_PLD_CRC_SIZE]),
      raw_pld_len - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER)
      != pld[ON_PLD_CRC_IDX])
    {
        return ONS_CRC_FAIL;
    } // if the crc's don't match //

    // get the transaction nonce
    *txn_nonce = (pld[ON_PLD_TXN_NONCE_IDX] >> ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_PARSE_MASK;

    // get the response nonce
    *resp_nonce = (pld[ON_PLD_RESP_NONCE_HIGH_IDX]
      << ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_PARSE_HIGH_MASK;
    *resp_nonce |= (pld[ON_PLD_RESP_NONCE_LOW_IDX] >> ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_PARSE_LOW_MASK;

    // get the message type
    *msg_type = pld[ON_PLD_MSG_TYPE_IDX] & ON_PLD_MSG_TYPE_MASK;

    return ONS_SUCCESS;
} // on_parse_pld //


#ifdef _BLOCK_MESSAGES_ENABLED
    /*!
        \brief Updates the time the transaction is supposed to occur at.

        \param[in] txn The transaction to update.

        \return void
    */
    void on_update_next_txn_time(on_txn_t * const txn)
    {
        if(!txn)
        {
            return;
        } // if the parameter is invalid //
        
        if(block_txn_ack_rcvd)
        {
            ont_set_timer(txn->next_txn_timer, 0);
            return;
        }

        if(txn->priority == ONE_NET_HIGH_PRIORITY)
        {
            ont_set_timer(txn->next_txn_timer,
              on_base_param->fragment_delay_high);
        } // if a high priority transaction //
        else
        {
            ont_set_timer(txn->next_txn_timer,
              on_base_param->fragment_delay_low);
        } // else a low priority transaction //
    } // on_update_next_txn_time //
#endif // if block messages are enabled //


/*!
    \brief The main ONE-NET function.

    This is the main state machine for ONE-NET.  It handles sending/receiving
    the various transactions.

    \param[in/out] txn The transaction currently being carried out

    \return TRUE if the current transaction is completed.  This should coincide
              with the priority for the transaction being set to
              ONE_NET_NO_PRIORITY unless another transaction resulted from the
              one that just finished.
            FALSE if the current transaction is not complete.
*/
BOOL one_net(on_txn_t ** txn)
{
    BOOL rv = FALSE;
	one_net_status_t rx_status;
 
#ifdef _ONE_NET_DEBUG_STACK
    if (one_net_debug_stack_flags & 0x01)
    {
        uart_write("\nIn one_net, stack is ", 22);
        uart_write_int8_hex( (((UInt16)(&rv))>>8) & 0xff );
        uart_write_int8_hex( ((UInt16)(&rv)) & 0xff );
        uart_write("\n", 1);
        one_net_debug_stack_flags &= ~0x01;
    }
#endif

    // Only need to check one of the handlers since it's all or nothing //
    if(!txn || !(*txn) || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE
      || !pkt_hdlr.single_data_hdlr)
    {
        return FALSE;
    } // if parameter is invalid //

    switch(on_state)
    {
        case ON_SEND_PKT:
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                on_state = ON_SEND_PKT_WRITE_WAIT;
            } // if channel is clear //
            break;
        } // send packet on_state //

        case ON_SEND_PKT_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // if write is complete //
            break;
        } // send packet write wait on_state //

        case ON_SEND_SINGLE_DATA_PKT:                   // fall through
        #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_SEND_BLOCK_DATA_PKT:                // fall through
		#endif
		#ifdef _STREAM_MESSAGES_ENABLED
            case ON_SEND_STREAM_DATA_PKT:
		#endif
        {
            if(ont_inactive_or_expired(ONT_GENERAL_TIMER)
              && check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                // general timer is now the response timer
                ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_RESPONSE_TIME_OUT);
                on_state++;
            } // if the channel is clear //
            
            break;
        } // send single data on_state //

        case ON_SEND_SINGLE_DATA_WRITE_WAIT:            // fall through
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:       // fall through
        #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_SEND_BLOCK_DATA_WRITE_WAIT:         // fall through
		#endif
		#ifdef _STREAM_MESSAGES_ENABLED
            case ON_SEND_STREAM_DATA_WRITE_WAIT:        // fall through
        #endif
        #ifdef _DATA_RATE
        case ON_SEND_DATA_RATE_WRITE_WAIT:
        #endif
        {
            if(one_net_write_done())
            {
                #ifdef _STREAM_MESSAGES_ENABLED
                    if(cur_stream)
                    {
                        *txn = cur_stream;
                        cur_stream = 0;
                        on_state = ON_WAIT_FOR_STREAM_DATA_RESP;
                    } // if cur_stream //
                    else
                #endif // ifdef _STREAM_MESSAGES_ENABLED //
                {
					on_state++;

					#ifndef _TRIPLE_HANDSHAKE
					if(on_state == ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT + 1)
					{
						on_state = ON_LISTEN_FOR_DATA;
					}
					#endif
                } // else not waiting for a stream response //
            } // if write is complete //
            break;
        } // send single data write wait case //

        case ON_WAIT_FOR_SINGLE_DATA_RESP:
        {
            // if the packet was not received, check the response timeout.
            // if timer expired, check retry, and either send the data again,
            // or fail the transaction.
			rx_status = rx_single_resp_pkt(txn);
            if(rx_status == ONS_SINGLE_FAIL || rx_status == ONS_SINGLE_CANCELED)
            {
                rv = TRUE;
                break;
            } // if the transaction failed //

            if(on_state == ON_LISTEN_FOR_DATA)
            {
                rv = TRUE;
            } // if the on_state changed to ON_LISTEN_FOR_DATA //
            else if(on_state == ON_WAIT_FOR_SINGLE_DATA_RESP
              && ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                // on_state has not changed, and the packet was not received
                // in time
                (*txn)->retry++;

                if((*txn)->retry >= ON_MAX_RETRY)
                {
                    pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
                      ONS_SINGLE_FAIL);
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                } // if the transaction has been tried too many times //
                else
                {
                    // make sure a repeat packet is sent
                    (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                      = ONE_NET_ENCODED_REPEAT_SINGLE_DATA;

                    if((*txn)->priority == ONE_NET_HIGH_PRIORITY)
                    {
                        ont_set_timer(ONT_GENERAL_TIMER,
                          one_net_prand(one_net_tick(),
                          ONE_NET_RETRANSMIT_HIGH_PRIORITY_TIME));
                    } // if it's a high priority transaction //
                    else
                    {
                        ont_set_timer(ONT_GENERAL_TIMER,
                          one_net_prand(one_net_tick(),
                          ONE_NET_RETRANSMIT_LOW_PRIORITY_TIME));
                    } // else it's a low priority transaction //
                    on_state = ON_SEND_SINGLE_DATA_PKT;
                }
            } // else if on_state hasn't changed & timed out waiting for resp //
            break;
        } // wait for single data response case //

        case ON_SEND_SINGLE_DATA_RESP:                  // fall through
        #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_SEND_BLOCK_DATA_RESP:               // fall through
        #endif // ifdef _BLOCK_MESSAGES_ENABLED //
        #ifdef _STREAM_MESSAGES_ENABLED
            case ON_SEND_STREAM_DATA_RESP:
        #endif // ifdef _STREAM_MESSAGES_ENABLED //
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);

                #ifdef _STREAM_MESSAGES_ENABLED
                    // don't update the timer if waiting for a stream data resp
                    if(!cur_stream)
                #endif // ifdef _ONE_NET_SIMPLE_DEVICE //
                {
                    // general timer is now the response timer

                    // TODO: RWM: maybe we should only set the general timer to 
                    // ONE_NET_TRN_END_TIME_OUT for the ON_SEND_SINGLE_DATA_RESP case
                    // and use ONE_NET_RESPONSE_TIME_OUT for the other cases as it was
                    // before 3/11/09 when we changed to ONE_NET_TRN_END_TIME_OUT?
                    
					#ifdef _TRIPLE_HANDSHAKE
                    ont_set_timer(ONT_GENERAL_TIMER,
                      ONE_NET_TRN_END_TIME_OUT);
					#endif
                } // else not waiting for a stream response //

                on_state++;
            } // if the channel is clear //
            break;
        } // send single/block data response case //

        #ifdef _TRIPLE_HANDSHAKE
        case ON_WAIT_FOR_SINGLE_DATA_END:
        {
            rx_single_txn_ack(txn);

            // if the on_state has not changed, then the packet was not received
            // in time
            if(on_state == ON_WAIT_FOR_SINGLE_DATA_END
              && ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // if on_state has not changed & timed out waiting for response //
            break;
        } // wait for the end of the single data transaction case //
		#endif

        #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_WAIT_FOR_BLOCK_DATA_RESP:
            {
                if(rx_block_resp_pkt(txn) == ONS_BLOCK_FAIL)
                {
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                    break;
                } // if the transaction failed //

                if(on_state == ON_WAIT_FOR_BLOCK_DATA_RESP
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    // the packet was not received in time
                    (*txn)->retry++;

                    if((*txn)->retry >= ON_MAX_RETRY)
                    {
                        pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_BLOCK_FAIL);
                        rv = TRUE;
                    } // if the transaction has been tried too many times //
                    else
                    {
                        pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_TIME_OUT);
                    } // else try some more //

                    on_state = ON_LISTEN_FOR_DATA;
                } // if timed out waiting for response //
                else if(block_complete)
                {
                    block_complete = FALSE;
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                } // if the transaction is compolete //
                break;
            } // wait for block data response case //

            case ON_SEND_BLOCK_DATA_RESP_WRITE_WAIT:
            {
                if(one_net_write_done())
                {
                    if(block_complete)
                    {
                        block_complete = FALSE;
                        ont_set_timer(ONT_GENERAL_TIMER,
                          ONE_NET_RESPONSE_TIME_OUT);
                        on_state = ON_WAIT_FOR_BLOCK_DATA_END;
                    } // if the block transaction is complete //
                    else
                    {
                        rv = TRUE;  // done sending response
                        on_state = ON_LISTEN_FOR_DATA;
                    } // else the block transaction is not complete //
                } // if write is complete //
                break;
            } // SEND_BLOCK_DATA_RESP_WRITE_WAIT case //

            case ON_WAIT_FOR_BLOCK_DATA_END:
            {
                if(rx_block_txn_ack(txn) == ONS_SUCCESS)
                {
                    rv = TRUE;
                } // if the txn ack has been received //

                // if the state has not changed, then the packet was not
                // received in time
                if(on_state == ON_WAIT_FOR_BLOCK_DATA_END
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    on_state = ON_LISTEN_FOR_DATA;
                } // if timed out waiting for response //
                break;
            } // WAIT_FOR_BLOCK_DATA_END case //

            #ifdef _STREAM_MESSAGES_ENABLED
            case ON_WAIT_FOR_STREAM_DATA_RESP:
            {
                if(rx_stream_resp_pkt(txn) == ONS_STREAM_FAIL)
                {
                    cur_stream = 0;
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                    break;
                } // if the transaction failed //

                if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    // the packet was not received in time
                    (*txn)->retry++;

                    if((*txn)->retry >= ON_MAX_RETRY)
                    {
                        cur_stream = 0;
                        pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_STREAM_FAIL);
                        rv = TRUE;
                    } // if the transaction has been tried too many times //
                    else
                    {
                        pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_TIME_OUT);
                    } // else try some more //

                    on_state = ON_LISTEN_FOR_DATA;
                } // else if timed out waiting for response //
                break;
            } // wait for block data response case //

            case ON_SEND_STREAM_DATA_RESP_WRITE_WAIT:
            {
                if(one_net_write_done())
                {
                    on_state = ON_LISTEN_FOR_DATA;
                } // if write is complete //
                break;
            } // SEND_STREAM_DATA_RESP_WRITE_WAIT case //
			#endif // if stream messages are enabled //
        #endif // ifdef _BLOCK_MESSAGES_ENABLED //

        #ifdef _DATA_RATE
        case ON_INIT_SEND_DATA_RATE:
        {
            UInt8 data_rate;

            if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
              ON_DATA_RATE_WORD_SIZE) != ONS_SUCCESS
              || data_rate >= ONE_NET_DATA_RATE_LIMIT)
            {
                // drop the transaction since the data rate is invalid
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
                break;
            } // if the data rate is invalid //

            one_net_set_data_rate(data_rate);
            (*txn)->retry = 0;
            data_rate_result = 0;
            on_state = ON_SEND_DATA_RATE;
            break;
        } // init sending data rate test case //

        case ON_SEND_DATA_RATE:
        {
            if((*txn)->retry < ON_MAX_DATA_RATE_TRIES)
            {
                if(check_for_clr_channel())
                {
                    one_net_write((*txn)->pkt, (*txn)->data_len);

                    // general timer is now the response timer
                    ont_set_timer(ONT_GENERAL_TIMER,
                      ONE_NET_RESPONSE_TIME_OUT);
                    on_state = ON_SEND_DATA_RATE_WRITE_WAIT;
                } // if the channel is clear //
            } // if not finished //
            else
            {
                UInt8 data_rate;

                one_net_set_data_rate(on_base_param->data_rate);
                (*txn)->priority = ONE_NET_NO_PRIORITY;

                if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
                  ON_DATA_RATE_WORD_SIZE) == ONS_SUCCESS)
                {
                    pkt_hdlr.data_rate_hdlr(data_rate,
                      (const on_encoded_did_t * const)
                      &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                      data_rate_result);
                } // if decoding was successful //

                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // else the data rate test is complete //
            break;
        } // send data rate case //

        case ON_RX_DATA_RATE_RESP:
        {
            if(rx_data_rate(*txn, FALSE) == ONS_SUCCESS)
            {
                data_rate_result++;
                (*txn)->retry++;
                on_state = ON_SEND_DATA_RATE;
            } // if receiving the response was successful //
            else if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                (*txn)->retry++;
                on_state = ON_SEND_DATA_RATE;
            } // if the device timed out waiting for the response //
            break;
        } // receive data rate response case //

        case ON_INIT_RX_DATA_RATE:
        {
            UInt8 data_rate;

            if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
              ON_DATA_RATE_WORD_SIZE) != ONS_SUCCESS
              || data_rate >= ONE_NET_DATA_RATE_LIMIT)
            {
                // drop the transaction since the data rate is invalid
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
                break;
            } // if the data rate is invalid //

            one_net_set_data_rate(data_rate);
            (*txn)->retry = 0;
            ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_STAY_AWAKE_TIME);
            on_state = ON_RX_DATA_RATE;
            break;
        } // init receiving data rate test case //

        case ON_RX_DATA_RATE:
        {
            if((*txn)->retry >= ON_MAX_RETRY ||
              ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                one_net_set_data_rate(on_base_param->data_rate);
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;

                // dje: 23 September, 2008
                //
                // The following two statements were added to make 
                // sure there are no further transmissions since
                // this is the receiving node of the data rate test
                //
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                (*txn)->pkt[ONE_NET_ENCODED_PID_IDX] = 
                    ONE_NET_ENCODED_SINGLE_DATA;
            } // if we had enough successes or timed out //
            else if(rx_data_rate(*txn, TRUE) == ONS_SUCCESS)
            {
                (*txn)->retry++;
                // try to send response write away
                ont_set_timer(ONT_GENERAL_TIMER, 0);
                on_state = ON_SEND_DATA_RATE_RESP;
            } // if receiving the response was successful //
            break;
        } // receive data rate case //

        case ON_SEND_DATA_RATE_RESP:
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                on_state = ON_SEND_DATA_RATE_RESP_WRITE_WAIT;
            } // if the channel is clear //
            break;
        } // send data rate response case //

        case ON_SEND_DATA_RATE_RESP_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                // update timer since a pkt was successfully received (and used
                // the timer when the response was sent)
                ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_STAY_AWAKE_TIME);
                on_state = ON_RX_DATA_RATE;
            } // if write is complete //
            break;
        } // send data rate response write wait case //
        #endif

        case ON_INIT_STATE:
        {
            // if the code gets here, it means the device was not initialized
            // properly.  Stay in this state until the device is initialized.
            break;
        } // initial state //

        default:
        {
            on_state = ON_LISTEN_FOR_DATA;
            break;
        } // default //
    } // switch(on_state) //

    if(on_state == ON_LISTEN_FOR_DATA)
    {
        one_net_set_data_rate(on_base_param->data_rate);
        //rv = TRUE;
    } // if going to the ON_LISTEN_FOR_DATA state //

    return rv;
} // one_net //

//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{

/*!
    \brief Checks if the channel is clear.

    If the channel is not clear, this function will set the clear channel time
    and will not check the channel again until the timer has expired.

    \param void

    \return TRUE If the channel is clear
            FALSE If the channel is not clear.
*/
static BOOL check_for_clr_channel(void)
{
    if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
    {
        ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_CLR_CHANNEL_TIME);
        return one_net_channel_is_clear();
    } // if it is time to check the channel //

    return FALSE;
} // check_for_clr_channel //


/*!
    \brief Receives a packet.

    This function only receives the address portion of the packet.  It verifies
    that the dst is correct, and if this device is expecting a packet from a
    specific sender, it verifies that the sender is correct.  If the address
    checks are all correct, this function calls the function pointed to by
    rx_specific_pkts to finish receiving the packets the device is allowed to
    receive based on the state it is in.

    \param[in] EXPECTED_SRC_DID Encoded address that the device expects to
      receive a packet from.  Set to the encoded form of the broadcast address
      if the device is willing to receive a packet from anybody.
    \param[in] src_did The device ID of the sender.
    \param[out] pkt Only valid if this device is a Multi-Hop repeater.  If this
      field is not NULL and if this device is not the intended destination, the
      received address fields (DST DID, NID, SRC DID) will be returned if the
      sending device is on the same network as this device (NIDs equal).  pkt
      must be at least the size of the longest packet, and represents the
      Multi-Hop packet that will be sent on.

    \return ONS_SUCCESS If the addresses were read correctly, and were what was
              expected.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_READ_ERR If there was a problem reading in the packet.
            ONS_NID_FAIL If the received NID does not match the NID for this
              device.
            ONS_INCORRECT_ADDR If this device is expecting a packet from
              someone, and the received packet is from someone other than the
              expected device.
            For more return values, see one_net_look_for_pkt, and
              on_validate_dst_DID
*/
#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const src_did, UInt8 * pkt)
#else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const src_did)
#endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
{
    on_encoded_nid_t rx_nid;
    on_encoded_did_t rx_dst;
    one_net_status_t status;

    if(!EXPECTED_SRC_DID || !src_did)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    if((status = one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME))
      != ONS_SUCCESS)
    {
        return status;
    } // if SOF was not received //

    // get the destination DID
    if(one_net_read(rx_dst, sizeof(rx_dst)) != sizeof(rx_dst))
    {
        return ONS_READ_ERR;
    } // if reading the destination DID failed //

    if((status = on_validate_dst_DID((const on_encoded_did_t * const)&rx_dst))
      != ONS_SUCCESS && status != ONS_BROADCAST_ADDR)
    {
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if(!pkt)
            {
                return status;
            } // if not repeating Multi-Hop packets.

            one_net_memmove(&(pkt[ONE_NET_ENCODED_DST_DID_IDX]), rx_dst,
              sizeof(rx_dst));
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            return status;
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    } // if the packet is not for this device //

    // get the NID
    if(one_net_read(rx_nid, sizeof(rx_nid)) != sizeof(rx_nid))
    {
        return ONS_READ_ERR;
    } // if reading the NID failed //

    if(!on_is_my_NID((const on_encoded_nid_t * const)&rx_nid))
    {
        return ONS_NID_FAILED;
    } // if NID does not match the NID for this device //

    // read the source address
    if(one_net_read(*src_did, sizeof(on_encoded_did_t))
      != sizeof(on_encoded_did_t))
    {
        return ONS_READ_ERR;
    } // if reading the source DID failed //

    // check to see if the device expects something from a specific source
    if(!on_encoded_did_equal(EXPECTED_SRC_DID, &ON_ENCODED_BROADCAST_DID))
    {
        if(!on_encoded_did_equal(EXPECTED_SRC_DID,
          (const on_encoded_did_t * const)src_did))
        {
            // don't bother with the Multi-Hop since the device is looking for
            // a specefic packet
            return ONS_INCORRECT_ADDR;
        } // if the source is not who the device is expecting //
    } // if this device is expecting a packet //

    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if(status == ONS_DID_FAILED)
        {
            one_net_memmove(&(pkt[ON_ENCODED_NID_IDX]), rx_nid, sizeof(rx_nid));
            one_net_memmove(&(pkt[ON_ENCODED_SRC_DID_IDX]), *src_did,
              sizeof(*src_did));
        } // if the this device is not the destination for the pkt //
    #endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

    return status;
} // rx_pkt_addr //


/*!
    \brief Receive the possible responses to a single data packet.

    This function is called from the ON_WAIT_FOR_SINGLE_DATA_RESP state.
    It only receives Single Data ACK, Single Data NACK, or Single
    Data ACK Stay Awake Packets.  These are expected in response to a Single
    Data Packet this device sent.<p>

    This function may change on_state:<br>
    If the nonce check fails and the maximum retries has been exeeded
    on_state is set to ON_LISTEN_FOR_DATA.<br>
    For ONE_NET_ENCODED_SINGLE_DATA_NACK and ONE_NET_ENCODED_MH_SINGLE_DATA_NACK 
    packet types, on_state is set to ON_SEND_SINGLE_DATA_PKT.

    \param[in] txn The transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized<br>
            ONS_BAD_PARAM If the parameter is not valid<br>
            ONS_BAD_PKT_TYPE If a packet type was received that this device is
              not looking for.<br>
            ONS_SINGLE_FAIL If the transaction has been tried too many times.<br>
            ONS_INCORRECT_NONCE If the received transaction nonce is not what is
              expected.<br>
            ONS_INVALID_DATA If received data was not correct<br>
            For more response values, see rx_pkt_addr, on_decode,
              single_resp_hdlr.
*/
static one_net_status_t rx_single_resp_pkt(on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 encoded_pld[ON_ENCODED_ACK_NACK_PLD_SIZE];
    one_net_raw_did_t raw_src_did;
    UInt8 pid;
    UInt8 txn_nonce, next_nonce;
	BOOL valid_pkt_pid = TRUE;
	BOOL isACK = FALSE;
	
	one_net_status_t abort_txn;
	
	on_nack_rsn_t nack_reason;
	on_ack_nack_handle_t ack_nack_handle;
    ack_nack_payload_t ack_nack_payload;
	#ifdef _ONE_NET_MASTER
    const on_client_t* client = 0;
    #endif
	const one_net_xtea_key_t * key = 0;		

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!txn || !*txn || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    // pkt in txn contains the single data packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from.
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
          != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    switch(pid)
	{
		case ONE_NET_ENCODED_SINGLE_DATA_ACK:
		case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
		    isACK = TRUE;
		case ONE_NET_ENCODED_SINGLE_DATA_NACK:
		case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
		    break;
		default:
		    valid_pkt_pid = FALSE;
	}
		
    if(!valid_pkt_pid)
    {
    #ifdef _ONE_NET_DEBUG
        one_net_debug(ONE_NET_DEBUG_ONS_BAD_PKT_TYPE, &pid, 1);
    #endif
        return ONS_BAD_PKT_TYPE;
    } // if the packet is not what the device is expecting //
    
    // read the payload containing the nonces.
    // TODO - find a named constant for 11.
    if(one_net_read(encoded_pld, ON_ENCODED_ACK_NACK_PLD_SIZE) !=
           ON_ENCODED_ACK_NACK_PLD_SIZE)
    {
        return ONS_READ_ERR;;
		
    }

	// Decode the source did.  We already know it's good or we would not
    // have made it this far.
	on_decode(raw_src_did, src_did, ON_ENCODED_DID_LEN);
    
    #ifdef _ONE_NET_MASTER
    if(deviceIsMaster)
    {
        client = client_info(&src_did);
	    if(client == 0)
    	{
		    return ONS_DID_FAILED;
	    }
    }
    #endif

    // we'll need the key in order to parse the payload.  First try the
    // current key
    key = (one_net_xtea_key_t*) (&(on_base_param->current_key));   
    status = rx_nonces(encoded_pld, &txn_nonce, &next_nonce, isACK ? NULL :
      &nack_reason, &ack_nack_handle, &ack_nack_payload, key, ON_SINGLE);
    
    #ifdef _ONE_NET_MASTER
    if(deviceIsMaster)
    {
        // if it didn't work and we are the master and we think the client is
        // using an old key, let's try the old key and see if it works.
        if(status != ONS_SUCCESS && client && !client->use_current_key)
        {
            key = &(master_param->old_key);
            status = rx_nonces(encoded_pld, &txn_nonce, &next_nonce, isACK ? NULL :
              &nack_reason, &ack_nack_handle, &ack_nack_payload, key, ON_SINGLE);
        }
    }
    #endif
    
    if(status != ONS_SUCCESS)
    {
        return status;
    } // if reading the nonces failed //
	
	// change any NACKs to NACK With Reason.  Make the reason generic.
	if(pid == ONE_NET_ENCODED_SINGLE_DATA_NACK)
	{
		pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
		nack_reason = ON_NACK_RSN_GENERAL_ERR;
	}
	
	// shouldn't need to do the step below, but if we got an ACK, we want to make sure
	// there is no nack_reason.
	// TODO - will this ever happen?
	if(isACK)
	{
		nack_reason = ON_NACK_RSN_NO_ERROR;
	}

	// first check the nonces.  Make sure we received the correct nonce back and
	// make sure that the other device like OUR nonce that we gave it.  Handle these two
	// first.  If they are wrong, then don't bother alerting the applciation code.
	// It doesn't want to deal with nonces.
	if((txn_nonce != (*txn)->expected_nonce) || (pid == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN
	        && nack_reason == ON_NACK_RSN_NONCE_ERR))
    {
		// this one failed.  Increment counter
        (*txn)->retry++;
		// rebuild packet with fresh nonces and a "Repeat" PID
        if((*txn)->retry >= ON_MAX_RETRY)
        {
            pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
              ONS_SINGLE_FAIL);
            on_state = ON_LISTEN_FOR_DATA;
            return ONS_SINGLE_FAIL;
        } // if the transaction has been tried too many times //

        status = pkt_hdlr.single_txn_hdlr(txn, next_nonce, ONS_RX_NACK);

        // try to send right away
        ont_set_timer(ONT_GENERAL_TIMER, 0);
        on_state = ON_SEND_SINGLE_DATA_PKT;
        return ONS_INCORRECT_NONCE;
    } // if the nonce did not match //
	
	// now we'll give the application code a chance to do whatever it wants to do
	// with the ACK/NACK, including handle it itself.  If it wants to handle everything
	// itself and cancel this transaction, it should return false.  If this is a
	// fast query response and the application code DOES NOT want the fast query response
	// handler, it should change the ack handle.  It can do a variety of other things,
	// including changing the nack to an ack or vice versa, change the reason, change the
	// number of retries remainingm, change the payload, etc.
	
	// first check if the return value is false.  If so, don't bother with anything else
	// and just cancel the transaction.
	
	// now call the application code ack/nack handler and see if it wants to do anything	
	abort_txn = pkt_hdlr.single_ack_nack_hdlr(*txn, &raw_src_did, &nack_reason,
	                 &ack_nack_handle, &ack_nack_payload);
					 
	#if _ACK_NACK_LEVEL >= 225						 
	// There is one thing to handle here.  If the nack reason is ON_NACK_RSN_BUSY_TRY_AGAIN
	// or ON_NACK_RSN_BUSY_TRY_AGAIN_TIME, we should either set a timer and try the
	// transaction again or put it in the queue and cancel it.
    if(!abort_txn && (nack_reason == ON_NACK_RSN_BUSY_TRY_AGAIN ||
	       nack_reason == ON_NACK_RSN_BUSY_TRY_AGAIN_TIME))
	{
	    (*txn)->time = 0;  // retry immediately
	    on_state = ON_SEND_SINGLE_DATA_PKT;
			
	    #if _ACK_NACK_LEVEL == 255
	    if(nack_reason == ON_NACK_RSN_BUSY_TRY_AGAIN_TIME &&
		   ack_nack_handle == ON_NACK_TIME_MS)
	    {
	        (*txn)->time = ack_nack_payload.nack_time_ms;
	    }		
		#endif
		
		status = pkt_hdlr.single_txn_hdlr(txn, next_nonce, ONS_RETRY);
	}
	#endif

					 
	// OK, we've called the application level ACK/NACK handler.  It will have changed anything
	// it wants to.  We could have an ACK payload that contains a response to a query we made.
	// Possibly it was already handled in then ACK/NACK handler, in which case if the application
	// code DOES NOT want us to handle it here, it would have returned FALSE or changed the
	// ack_nack_handle to something other than ON_ACK_STATUS.  If we have an ACK and no
	// nack reason and a handle of ON_ACK_STATUS, we'll assume that any query responses
	// have not been handled yet.
	
	if(abort_txn == ONS_SUCCESS && pid == ONE_NET_ENCODED_SINGLE_DATA_ACK &&
	        nack_reason == ON_NACK_RSN_NO_ERROR && ack_nack_handle == ON_ACK_STATUS)
	{
		// we have a status message in the ACK.  Let's parse it as a single data app message and send it to
		// the status message handler.
		
		UInt8 src_unit, dst_unit;
		ona_msg_class_t msg_class;
		ona_msg_type_t msg_type;
		UInt16 msg_data;
		
		nack_reason = on_parse_single_app_pld(ack_nack_payload.status_resp, &src_unit,
            &dst_unit, &msg_class, &msg_type, &msg_data);
			
		if(nack_reason == ON_NACK_RSN_NO_ERROR)
		{
            #ifdef _POLL
			if(msg_class != ONA_STATUS_QUERY_RESP && msg_class != ONA_STATUS_FAST_QUERY_RESP &&
			       msg_class != ONA_STATUS && msg_class != ONA_STATUS_ACK_RESP)
            #else
			if(msg_class != ONA_STATUS_QUERY_RESP &&
			       msg_class != ONA_STATUS && msg_class != ONA_STATUS_ACK_RESP)
            #endif
			{
				nack_reason = ON_NACK_RSN_TRANSACTION_ERR; // The packet handle says it's
				// a status message, but the message class says otherwise.
			}
		}

		if(nack_reason == ON_NACK_RSN_NO_ERROR)
		{
			// it parsed OK.  Let's pass it to the status handler
			if(!pkt_hdlr.status_msg_hdlr(ack_nack_payload.status_resp, msg_class, msg_type, src_unit,
                dst_unit, msg_data, &raw_src_did, &nack_reason, &ack_nack_handle,
				&ack_nack_payload))
			{
				abort_txn = ONS_CANCELED; // cancelled by application code
			}
		}
				
		if(abort_txn == ONS_SUCCESS && nack_reason != ON_NACK_RSN_NO_ERROR)
		{
            // if we get a nack reason, we'll change the PID and let it be handled
		    // like any other nack with reason
			pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
		}
	}
					 
	if(abort_txn == ONS_CANCELED)
	{
		// application code has cancelled this transaction for whatever reason
        pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
            ONS_CANCELED);
            on_state = ON_LISTEN_FOR_DATA;
            return ONS_CANCELED;        
	}
	
	if(abort_txn != ONS_SUCCESS)
	{
		// some error occurred somewhere.  We'll call it a single fail
		// and abort.
        pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1, ONS_SINGLE_FAIL);
        on_state = ON_LISTEN_FOR_DATA;
        return ONS_SINGLE_FAIL; 		
	}

    if(pid == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN)
    {
		// this one failed.  Increment counter
        (*txn)->retry++;
		
		// now check whether there have been too many retries or if there was a
		// fatal nack.  If either, we abort with a failure.
        #if _ACK_NACK_LEVEL >= 50		
        if(nack_reason_is_fatal(nack_reason) || (*txn)->retry >= ON_MAX_RETRY)
		#else
		if((*txn)->retry >= ON_MAX_RETRY)
		#endif
        {
            pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
              ONS_SINGLE_FAIL);
            on_state = ON_LISTEN_FOR_DATA;
            return ONS_SINGLE_FAIL;
        } // if the transaction has been tried too many times //

		// rebuild packet with fresh nonces and a "Repeat" PID
        status = pkt_hdlr.single_txn_hdlr(txn, next_nonce, ONS_RX_NACK);

        // try to send right away
        ont_set_timer(ONT_GENERAL_TIMER, 0);
        on_state = ON_SEND_SINGLE_DATA_PKT;
		
		// it wasn't fatal.  Return to main loop.  Re-send
		// TODO - the nack reason was not an incorrect nonce, but I'm not
		// sure how else to handle a nack on the sending end.
		// What about the single_txn_hdlr return value?
		// What about combining with the one above?  Same code.  Could save
		// some bytes.
		return ONS_INCORRECT_NONCE;
    } // if a single data nack was received //
    else
    {
        #ifndef _TRIPLE_HANDSHAKE
		// not sure if this is necessary
		on_state = ON_LISTEN_FOR_DATA;
		(*txn)->priority = ONE_NET_LOW_PRIORITY;
		#endif
		
        status = pkt_hdlr.single_txn_hdlr(txn, next_nonce,
          pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
          ? ONS_RX_STAY_AWAKE : ONS_SUCCESS);

        #ifdef _TRIPLE_HANDSHAKE
        if(pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
          || status == ONS_SUCCESS)
        {

            // Received a stay awake, or there is not another transaction being
            // sent to that device, so end the transaction.
            (*txn)->data_len = (*txn)->pkt_size;
            
            // try to send right away
            ont_set_timer(ONT_GENERAL_TIMER, 0);

            if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
              ONE_NET_ENCODED_SINGLE_TXN_ACK,
              (const on_encoded_did_t * const)&src_did, 0, 0))
                != ONS_SUCCESS)
            {
                on_state = ON_LISTEN_FOR_DATA;
                return status;
            } // if building the transaction ack failed //
            on_state = ON_SEND_PKT;
        }
        else if(status == ONS_TXN_QUEUED)
        {
            // There is another single transaction to send to the receiving
            // device. Try to send right away since not sending a transaction
            // ACK (the default state is ON_LISTEN_FOR_DATA, so that is where
            // the next packet should be dequeued from.
            ont_set_timer(ONT_GENERAL_TIMER, 0);
            on_state = ON_LISTEN_FOR_DATA;
        } // else if another transaction is queued //
        // else the transaction was an error, so time out and retry //
		#endif
    } // if a single data ack or single data ack stay awake was received //

    return status;
} // rx_single_resp_pkt //


#ifdef _TRIPLE_HANDSHAKE
/*!
    \brief Receives the single data transaction ack.

    This is called by the receiving device of a single data transaction.  It
    will also receive a single data (for a new transaction) or repeat single
    data packet.

    \param[in] txn The transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized properly.
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_READ_ERR If there was an error while reading the packet.
            ONS_BAD_PKT_TYPE If a packet type that was not expected was
              received.
            For more return codes, see rx_pkt_addr.
*/
static one_net_status_t rx_single_txn_ack(on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 pid;

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!txn || !(*txn) || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    //
    // pkt in txn contains the ack packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from. rx_pkt_addr will filter on this 
    // address.
    //
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
          != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    switch(pid)
    {
        case ONE_NET_ENCODED_SINGLE_TXN_ACK:
        {
            (*txn)->priority = ONE_NET_NO_PRIORITY;
            on_state = ON_LISTEN_FOR_DATA;
            break;
        } // single transaction ack case //

        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            status = rx_single_data(pid,
              (const on_encoded_did_t * const)&src_did, txn);
            break;
        } // single data case //

        default:
        {
            return ONS_BAD_PKT_TYPE;
        } // default //
    } // switch(pid) //

    // received the end of the transaction (or start of a new one), so clear
    // the general timer as it is not needed
    ont_stop_timer(ONT_GENERAL_TIMER);
    return status;
} // rx_single_txn_ack //
#endif



#ifdef _BLOCK_MESSAGES_ENABLED
    /*!
        \brief Receive the possible responses to a block data packet.

        This function only receives Block Data ACK, or Block Data NACK Packets.
        These are expected in response to a Block Data Packet this device sent.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized
                ONS_BAD_PARAM If the parameter is not valid
                ONS_BAD_PKT_TYPE If a packet type was received that this device
                  is not looking for.
                ONS_BLOCK_FAIL If the transaction has been tried too many times.
                ONS_INCORRECT_NONCE If the received transaction nonce is not
                  what is expected.
                For more response values, see rx_pkt_addr, on_decode.
    */
    static one_net_status_t rx_block_resp_pkt(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;
        UInt8 encoded_pld[ON_ENCODED_ACK_NACK_PLD_SIZE];
        UInt8 txn_nonce, next_nonce;
	    BOOL valid_pkt_pid = TRUE;
	    BOOL isACK = FALSE;		
		on_nack_rsn_t nack_reason;
	    on_ack_nack_handle_t ack_nack_handle;
        ack_nack_payload_t ack_nack_payload;
		
		#ifdef _ONE_NET_MASTER
        const on_client_t* client = 0;
		#endif
		const one_net_xtea_key_t * key = 0;	

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.block_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !*txn || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the block data packet that was sent which
        // contains the encoded address (the destination did in the packet)
        // that the response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr((const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        switch(pid)
    	{
    		case ONE_NET_ENCODED_BLOCK_DATA_ACK:
    		    isACK = TRUE;
    		case ONE_NET_ENCODED_BLOCK_DATA_NACK:
    		case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:
    		    break;
    		default:
    		    valid_pkt_pid = FALSE;
    	}

        if(!valid_pkt_pid)
        {
            return ONS_BAD_PKT_TYPE;
        } // if the packet is not what the device is expecting //

        #ifdef _ONE_NET_MASTER
        client = client_info(&src_did);
		if(client == 0)
		{
			return ONS_DID_FAILED;
		}
        #endif

        if(one_net_read(encoded_pld, ON_ENCODED_ACK_NACK_PLD_SIZE) !=
               ON_ENCODED_ACK_NACK_PLD_SIZE)
        {
            return ONS_READ_ERR;;		
        }

        // we'll need the key in order to parse the payload.  First try the
        // current key
        key = (one_net_xtea_key_t*) (&(on_base_param->current_key));
        status = rx_nonces(encoded_pld, &txn_nonce, &next_nonce, isACK ? NULL :
          &nack_reason, &ack_nack_handle, &ack_nack_payload, key, ON_BLOCK);
    
        #ifdef _ONE_NET_MASTER
        // if it didn't work and we are the master and we think the client is
        // using an old key, let's try the old key and see if it works.
        if(status != ONS_SUCCESS && client && !client->use_current_key)
        {
            key = &(master_param->old_key);
            status = rx_nonces(encoded_pld, &txn_nonce, &next_nonce, isACK ? NULL :
              &nack_reason, &ack_nack_handle, &ack_nack_payload, key, ON_BLOCK);
        }
        #endif
        
        if(status != ONS_SUCCESS)
        {
            return status;
        } // if reading the nonces failed //
		
	    // change any NACKs to NACK With Reason.  Make the reason generic.
	    if(pid == ONE_NET_ENCODED_BLOCK_DATA_NACK)
	    {
	    	pid = ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN;
		    nack_reason = ON_NACK_RSN_GENERAL_ERR;
	    }

        if(txn_nonce != (*txn)->expected_nonce
          || pid == ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN)
        {
            (*txn)->retry++;

            if((*txn)->retry >= ON_MAX_RETRY)
            {
                pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                  ONS_BLOCK_FAIL);
                return ONS_BLOCK_FAIL;
            } // if the transaction has been tried too many times //

            if(txn_nonce != (*txn)->expected_nonce)
            {
                return ONS_INCORRECT_NONCE;
            } // if the nonce did not match //
        } // if the nonce did not match or it was a NACK //

        on_state = ON_LISTEN_FOR_DATA;
        if(pid == ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN)
        {
            if((status = pkt_hdlr.block_txn_hdlr(txn, next_nonce,
              ONS_RX_NACK)) == ONS_SUCCESS)
            {
                status = ONS_RX_NACK;
            } // if txn_hdlr successful //
        } // if a block data nack was received //
        else
        {
            // adjust the block data position since we got an ACK
            UInt16 orig_block_data_pos = block_data_pos;
            if(ack_nack_handle == ON_ACK_VALUE)
            {
                if(ack_nack_payload.ack_value.uint32 == 0)
                {
                    block_data_pos = block_data_len + 1; // client has received
                                                         // all
                    if(block_byte_transfer_complete)
                    {
                        block_txn_ack_rcvd = TRUE;
                        (*txn)->remaining = 0;
                    }
                    block_byte_transfer_complete = TRUE;
                }
                else
                {
                    block_data_pos =
                        (UInt16) (ack_nack_payload.ack_value.uint32) - 1;
                    block_byte_transfer_complete = FALSE;
                }
            }
            status = pkt_hdlr.block_txn_hdlr(txn, next_nonce, ONS_SUCCESS);

            if(status == ONS_BLOCK_END)
            {
                // The block is completed
                (*txn)->data_len = (*txn)->pkt_size;

                // Try to send right away?
                ont_set_timer(ONT_GENERAL_TIMER, 0);

                #if 0
                if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
                  ONE_NET_ENCODED_BLOCK_TXN_ACK,
                  (const on_encoded_did_t * const)&src_did, 0, 0))
                  != ONS_SUCCESS)
                {
                    block_complete = TRUE;
                    return status;
                } // if building the transaction ack failed //

                on_state = ON_SEND_PKT;
                #endif
                on_state = ON_LISTEN_FOR_DATA;
            } // if the block transaction is complete //
            else
            {
                ont_stop_timer(ONT_GENERAL_TIMER);
            } // else the block transaction is not yet complete //
        } // else a block data ack was received //

        return status;
    } // rx_block_resp_pkt //


    /*!
        \brief Receives the block data transaction ack.

        This is called by the receiving device of a block transaction when the
        transaction is complete.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized properly.
                ONS_BAD_PARAM If the parameter is invalid.
                ONS_READ_ERR If there was an error while reading the packet.
                ONS_BAD_PKT_TYPE If an unexpected packet type was received.
                For more return codes, see rx_pkt_addr.
    */
    static one_net_status_t rx_block_txn_ack(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.block_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !(*txn) || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the ack packet that was sent which contains
        // the encoded address (the destination did in the packet) that the
        // response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        if(pid == ONE_NET_ENCODED_REPEAT_BLOCK_DATA)
        {
            // don't bother reading the data since to be here, this device must
            // have already received the last block data packet, therefore the
            // sending device must not have received the response, so resend the
            // response (which happens to still be txn).
            on_state = ON_SEND_BLOCK_DATA_RESP;
        } // if block or repeat block packet //
        else if(pid == ONE_NET_ENCODED_BLOCK_TXN_ACK)
        {
            // received the end of the transaction, so clear the general
            // timer as it is not needed
            pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1, ONS_BLOCK_END);
            ont_stop_timer(ONT_GENERAL_TIMER);
            (*txn)->priority = ONE_NET_NO_PRIORITY;
            on_state = ON_LISTEN_FOR_DATA;
        } // else if a block txn ack //
        else
        {
            return ONS_BAD_PKT_TYPE;
        } // if an unexpected packet was received //

        return status;
    } // rx_block_txn_ack //


    #ifdef _STREAM_MESSAGES_ENABLED
    /*!
        \brief Receive the response to a stream data packet.

        This function only receives Stream Keep Alive Packets.  Thise are
        expected in response to a Stream Data Packet this device sent.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized
                ONS_BAD_PARAM If the parameter is not valid
                ONS_BAD_PKT_TYPE If a packet type was received that this device
                  is not looking for.
                ONS_STREAM_FAIL If the transaction has been tried too many
                  times.
                ONS_INCORRECT_NONCE If the received transaction nonce is not
                  what is expected.
                For more response values, see rx_pkt_addr, on_decode.
    */
    static one_net_status_t rx_stream_resp_pkt(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;
        UInt8 encoded_pld[ON_ENCODED_ACK_NACK_PLD_SIZE];
        UInt8 txn_nonce, next_nonce;
		on_nack_rsn_t nack_reason;
	    on_ack_nack_handle_t ack_nack_handle;
        ack_nack_payload_t ack_nack_payload;

		#ifdef _ONE_NET_MASTER
        const on_client_t* client = 0;
		#endif
		const one_net_xtea_key_t * key = 0;

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.stream_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !*txn || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the stream data packet that was sent which
        // contains the encoded address (the destination did in the packet)
        // that the response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        switch(pid)
        {
            case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
            {
                // handled below
                break;
            } // ONE_NET_ENCODED_STREAM_KEEP_ALIVE case //

            case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
            case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
            {
                // remember the stream in case we need to come back to it.
                cur_stream = *txn;

                status = rx_single_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);

                if(*txn != cur_stream)
                {
                    if(status == ONS_STREAM_END)
                    {
                        // the stream ended so it does not need to be saved
                        cur_stream = 0;
                    } // if the stream has ended //

                    on_state = ON_SEND_SINGLE_DATA_RESP; 
                } // if a response is being sent back //

                return status;
                break;
            } // single data case //

            default:
            {
                return ONS_BAD_PKT_TYPE;
                break;
            } // default case //
        } // switch(pid) //

        // we'll need the key in order to read the nonces
		#if defined(_ONE_NET_MASTER) && defined(_ONE_NET_CLIENT)
		if(deviceIsMaster)
		{
			client = client_info(&src_did);
			if(client == 0)
			{
				return ONS_DID_FAILED;
			}
			key = get_client_encryption_key(client, TRUE);
		}
		else
		{
			key = &(on_base_param->stream_key);
		}
		#elif defined(_ONE_NET_MASTER)
		client = client_info(&src_did);
		if(client == 0)
		{
			return ONS_DID_FAILED;
		}
		key = get_client_encryption_key(client, TRUE);		
		#else
		key = on_base_param->stream_key;
		#endif

        // read the nonces -  not sure what we're expecting as far as keys,
		// acks/nacks, etc.
		// TODO - figure out the packet types, figure out what responses we
		// can/should expect and need to handle.
		// TODO - if we can get NACKs, do we have a reason?  Change to "NACK
		// with Reason"?  And should nack_reason be NULL?  Not sure, but just
		// in case, putting in some handling.
        if(one_net_read(encoded_pld, ON_ENCODED_ACK_NACK_PLD_SIZE) !=
               ON_ENCODED_ACK_NACK_PLD_SIZE)
        {
            return ONS_READ_ERR;;
		
         } // if reading the transaction nonce failed //
        if((status = rx_nonces(encoded_pld, &txn_nonce, &next_nonce, &nack_reason,
		      &ack_nack_handle, &ack_nack_payload, key, ON_STREAM)) != ONS_SUCCESS)
        {
            return status;
        } // if reading the nonces failed //
		
		// TODO : does the below make sense?
		switch(pid)
		{
			case ONE_NET_ENCODED_SINGLE_DATA_NACK:
			    pid = ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN;
				nack_reason = ON_NACK_RSN_GENERAL_ERR;
				break;
			case ONE_NET_ENCODED_BLOCK_DATA_NACK:
			    pid = ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN;
				nack_reason = ON_NACK_RSN_GENERAL_ERR;
				break;
		}
				
        if(txn_nonce != (*txn)->expected_nonce)
        {
            (*txn)->retry++;

            if((*txn)->retry >= ON_MAX_RETRY)
            {
                cur_stream = 0;
                pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                  ONS_STREAM_FAIL);
                return ONS_STREAM_FAIL;
            } // if the transaction has been tried too many times //

            return ONS_INCORRECT_NONCE;
        } // if the nonce did not match //

        on_state = ON_LISTEN_FOR_DATA;
        status = pkt_hdlr.stream_txn_hdlr(txn, next_nonce, ONS_SUCCESS);
        ont_stop_timer(ONT_GENERAL_TIMER);

        return status;
    } // rx_stream_resp_pkt //
	#endif // if stream messages are enabled //
#endif // ifdef _BLOCK_MESSAGES_ENABLED //


#ifdef _DATA_RATE
/*!
    \brief Receives a data rate packet.

    Verifies the contents of the packet to make sure there were no bit errors.

    \param[in/out] txn If sending, contains the address the response data rate
      packet should be coming from.  If receiving the transaction, returns the
      data rate packet to send back (if the received packet was correct).
    \param[in] RECEIVER TRUE if this device is the receiver in the transaction.
                      FALSE if this device is the sender in the transaction.

    \return ONS_SUCCESS if the received packet was correct.
            ONS_BAD_PARAM if at least one parameter is invalid.
            ONS_BAD_PKT_TYPE if the packet type is not what is expected.
            ONS_INVALID_DATA if the received data was not correct.
            See rx_pkt_addr for more possible return codes.
*/
static one_net_status_t rx_data_rate(on_txn_t * const txn,
  const BOOL RECEIVER)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 i;
    UInt8 pid, data_rate;
    UInt8 test_pattern[ON_TEST_PATTERN_SIZE];

    if(!txn || !(txn->pkt) || txn->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(RECEIVER)
    {
        // the receiver isn't expecting the packet from anyone in particular.
        one_net_memmove(&(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
          ON_ENCODED_BROADCAST_DID, sizeof(ON_ENCODED_BROADCAST_DID));
    } // if the device is the receiver for the transaction //

    // pkt in txn contains the ack packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from.
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did)) != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    if(pid != ONE_NET_ENCODED_DATA_RATE_TEST)
    {
        return ONS_BAD_PKT_TYPE;
    } // not the expected pid //

    // read the data rate
    if(one_net_read(&data_rate, sizeof(data_rate)) != sizeof(data_rate))
    {
        return ONS_READ_ERR;
    } // if reading the data rate failed //

    if(data_rate != txn->pkt[ON_DATA_RATE_IDX])
    {
        return ONS_INVALID_DATA;
    } // if the received data rate is not as expected //

    // read the test pattern
    if(one_net_read(test_pattern, sizeof(test_pattern)) != sizeof(test_pattern))
    {
        return ONS_READ_ERR;
    } // if reading the test pattern failed //

    for(i = 0; i < ON_TEST_PATTERN_SIZE; i++)
    {
        if(test_pattern[i] != ON_TEST_PATTERN)
        {
            return ONS_INVALID_DATA;
        } // if the test pattern does not match //
    } // loop to verify the test pattern //

    if(RECEIVER)
    {
        // add the senders did as the destination did to complete the
        // response
        one_net_memmove(&(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), src_did,
          sizeof(src_did));
    } // if this device is receiving during the test //

    return status;
} // rx_data_rate //
#endif


/*!
    \brief Finishes reception of a single data pkt

    \param[in] PID The PID that was received.  Should be
      ONE_NET_ENCODED_SINGLE_DATA, ONE_NET_ENCODED_REPEAT_SINGLE_DATA,
      ONE_NET_ENCODED_MH_SINGLE_DATA, or ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA.
    \param[in] SRC_DID Pointer to the did of the sender.
    \param[in/out] txn The transaction that is being carried out.

    \return ONS_READ_ERR If the hops field could not be read.
            ONS_INTERNAL_ERR If something unexpexted happened.
            See rx_payload & single_data_hdlr for more options.
*/
static one_net_status_t rx_single_data(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
{
    one_net_status_t status = ONS_INTERNAL_ERR;
    UInt8 raw_pld[ON_RAW_SINGLE_PLD_SIZE];

    if((status = rx_payload(raw_pld, ON_ENCODED_SINGLE_PLD_SIZE))
      != ONS_SUCCESS)
    {
        return status;
    } // if receiving the packet was not successful //
	
    status = (*pkt_hdlr.single_data_hdlr)(PID, SRC_DID, raw_pld, txn);

    if(*txn)
    {
        on_state = ON_SEND_SINGLE_DATA_RESP;
    } // if a response is being sent back //
    else
    {
        on_state = ON_LISTEN_FOR_DATA;
    } // else if the device does not want to send a response back //

    return status;
} // rx_single_data //


#ifdef _BLOCK_MESSAGES_ENABLED
    /*!
        \brief Finishes reception of a block data pkt

        \param[in] PID The PID that was received.  Should be
          ONE_NET_ENCODED_BLOCK_DATA, ONE_NET_ENCODED_REPEAT_BLOCK_DATA,
          ONE_NET_ENCODED_MH_BLOCK_DATA, or ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
        \param[in] SRC_DID Pointer to the did of the sender.
        \param[in/out] txn The transaction that is being carried out.

        \return ONS_READ_ERR If the hops field could not be read.
                ONS_INTERNAL_ERR If something unexpexted happened.
                See rx_payload & block_data_hdlr for more options.
    */
    static one_net_status_t rx_block_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
    {
        one_net_status_t status = ONS_INTERNAL_ERR;
        UInt8 raw_pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];

        if((status = rx_payload(raw_pld, ON_ENCODED_BLOCK_STREAM_PLD_SIZE))
          != ONS_SUCCESS)
        {
            return status;
        } // if receiving the packet was not successful //

        status = (*pkt_hdlr.block_data_hdlr)(PID, SRC_DID, raw_pld, txn);

        if(status == ONS_BLOCK_END)
        {
            block_complete = TRUE;
        } // if the block transaction has ended //

        if((status == ONS_SUCCESS || status == ONS_BLOCK_END) && *txn)
        {
            on_state = ON_SEND_BLOCK_DATA_RESP;
        } // if a response is being sent back //

        return status;
    } // rx_block_data //


    #ifdef _STREAM_MESSAGES_ENABLED
    /*!
        \brief Finishes reception of a stream data pkt

        \param[in] PID The PID that was received.  Should be
          ONE_NET_ENCODED_STREAM_DATA or ONE_NET_ENCODED_MH_STREAM_DATA.
        \param[in] SRC_DID Pointer to the did of the sender.
        \param[in/out] txn The transaction that is being carried out.

        \return ONS_READ_ERR If the hops field could not be read.
                ONS_INTERNAL_ERR If something unexpexted happened.
                See rx_payload & stream_data_hdlr for more options.
    */
    static one_net_status_t rx_stream_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
    {
        one_net_status_t status = ONS_INTERNAL_ERR;
        UInt8 raw_pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];

        if((status = rx_payload(raw_pld, ON_ENCODED_BLOCK_STREAM_PLD_SIZE))
          != ONS_SUCCESS)
        {
            return status;
        } // if receiving the packet was not successful //

        status = (*pkt_hdlr.stream_data_hdlr)(PID, SRC_DID, raw_pld, txn);

        if(*txn)
        {
            if(status == ONS_SUCCESS)
            {
                on_state = ON_SEND_STREAM_DATA_RESP;
            } // if successful //
            else if(status == ONS_STREAM_END)
            {
                cur_stream = 0;
                on_state = ON_SEND_SINGLE_DATA_PKT;
            } // else if ending the stream //
        } // if a response is being sent back //
 
        return status;
    } // rx_stream_data //
	#endif
#endif // ifdef _BLOCK_MESSAGES_ENABLED //


/*!
    \brief Reads in and decodes the payload portion of data packets.

    \param[out] raw_pld The raw payload of the packet that was received.  The
      size of raw_pld must be ENCODED_LEN * 6 / 8.
    \param[in] ENCODED_LEN The number of bytes to read from the rf interface.

    \return ONS_BAD_PARAM If a parameter passed in is not valid
            ONS_READ_ERR If reading the encoded payload failed.
            For more return values, see on_decode.
*/
static one_net_status_t rx_payload(UInt8 * const raw_pld,
  const UInt8 ENCODED_LEN)
{
    UInt8 rx_encoded_pld[ON_ENCODED_BLOCK_STREAM_PLD_SIZE];

    if(!raw_pld || ENCODED_LEN > sizeof(rx_encoded_pld))
    {
        return ONS_BAD_PARAM;
    } // if parameter is invalid //

    // read the encoded payload
    if(one_net_read(rx_encoded_pld, ENCODED_LEN) != ENCODED_LEN)
    {
        return ONS_READ_ERR;
    } // if reading the encoded payload failed //

    // decode the payload
    return on_decode(raw_pld, rx_encoded_pld, ENCODED_LEN);
} // rx_payload //


/*!
    \brief Receives the nonces from the rf interface.

    Receives the txn_nonce and resp_nonce from the rf_interface.  The decoded
    and parsed nonces are returned.

    \param[in] encoded_pld The encoded payload that is to be parsed
    \param[out] txn_nonce The decoded & parsed transaction nonce.
    \param[out] next_nonce The decoded & parsed next nonce.
    \param[out] nack_reason reason for the nack(if relevant).  If non-NULL, assumes NACK.  Otherwise, assumes ACK.
    \param[out] ack_nack_handle the "handle" for the payload, if any.
    \param[out] ack_nack_payload the payload, if any.
	\param[in] key encryption key to use to decrypt this packet
	\param[in] data_type data type of the packet (i.e. single, block., stream)

    \return ONS_SUCCESS If the nonces were successfully received and parsed.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_READ_ERR If either nonce could not be read from the rf
              interface.
*/
static one_net_status_t rx_nonces(const UInt8* const encoded_pld,
  UInt8 * const txn_nonce, UInt8 * const next_nonce,
  on_nack_rsn_t* const nack_reason,
  on_ack_nack_handle_t* const ack_nack_handle,
  ack_nack_payload_t* const ack_nack_payload,
  const one_net_xtea_key_t * const key, const on_data_t data_type)
{
    one_net_status_t status;
    UInt8 encoded_nonce, crc;
	UInt8 raw_pld[ON_RAW_ACK_NACK_PLD_SIZE];
	UInt8 i;
	UInt32 uint32;

    if(!encoded_pld || !txn_nonce || !next_nonce || !ack_nack_handle ||
         !ack_nack_payload)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
	
	if((status = on_decode(raw_pld, encoded_pld, ON_ENCODED_ACK_NACK_PLD_SIZE))
      != ONS_SUCCESS)
	{
		return ONS_INVALID_DATA;
	}
	
	if((status = on_decrypt(data_type, raw_pld, key, ON_RAW_ACK_NACK_PLD_SIZE))
      != ONS_SUCCESS)
	{
		return status;
	}
	
    crc = (UInt8)one_net_compute_crc(&(raw_pld[1]),
        ON_RAW_ACK_NACK_PLD_SIZE - 2, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
	  
	if(raw_pld[0] != crc)
	{
		return ONS_CRC_FAIL;
	}

    // TODO - replace the the shitfts and masks below with named constants.
    // nonces
    *txn_nonce = (raw_pld[1] & 0xFC) >> 2;
	*next_nonce = ((raw_pld[1] & 0x03) << 4) + ((raw_pld[2] & 0xF0) >> 4);

    // nack reason (if any)
	if(nack_reason != 0)
	{
		*nack_reason = raw_pld[3] >> 2;
	}
	
	// handle --> how to handle the ack/nack data (if any)
	*ack_nack_handle = raw_pld[2] & 0x0F;
	
	// check for validity and read.
	switch(*ack_nack_handle)
	{
		case ON_ACK: /* ON_NACK has same value */
		    break; // no ACK/NACK data
		#if _ACK_NACK_LEVEL	>= 95
		case ON_ACK_DATA: /* ON_NACK_DATA has same value */
		    if(nack_reason == 0)
			{
		        one_net_memmove((*ack_nack_payload).ack_payload, &raw_pld[3], 5);
			}
			else
			{
		        one_net_memmove((*ack_nack_payload).nack_payload, &raw_pld[4], 4);
			}			    
			break;
		#endif
		#if _ACK_NACK_LEVEL	>= 90
		case ON_ACK_VALUE: /* ON_NACK_VALUE has same value */
		    uint32 = one_net_byte_stream_to_int32(&(raw_pld[4]));
			
			if(nack_reason == 0)
			{
				(*ack_nack_payload).ack_value.uint32 = uint32;
				(*ack_nack_payload).ack_value.uint8 = raw_pld[3];
			}
			else
			{
				(*ack_nack_payload).nack_value = uint32;
			}
			break;
		#endif
		#if _ACK_NACK_LEVEL == 255
		case ON_ACK_TIME_MS: /* ON_NACK_TIME_MS has same value */
		    uint32 = one_net_byte_stream_to_int32(&(raw_pld[4]));
				(*ack_nack_payload).ack_time_ms = uint32; // no need to do one for the nack
				                                          // nack_time_ms is at raw_pld[4] too.
			break;
		#endif

        #if _ACK_NACK_LEVEL	>= 80		
		case ON_ACK_STATUS: /* This should only accompany an ACK */
		                            /* Therefore make sure nack_reason is null */
			if(nack_reason != NULL)
			{
				return ONS_BAD_PKT; // invalid handling for a NACK
			}
		    one_net_memmove((*ack_nack_payload).status_resp, &raw_pld[3], 5);
			break;
		#endif
		default:
		    return ONS_BAD_PKT; // invalid value.		
	}

    return ONS_SUCCESS;
} // rx_nonces // 


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    /*!
        \brief Reads in the rest of a Multi-Hop packet (after the address),
          changes the hops field, and sends the packet back out.

        \param txn[out] Returns the pointer to the mh_hop txn to be sent.

        \return ONS_SUCCESS If the packet was successfully handled either by
                  sending the packet on, or droping the packet because hops left
                  reached 0.
                ONS_READ_ERR if the data from the rf interface could not be
                  read.
                ONS_UNHANDLED_PKT If it was not a Multi-Hop packet.
    */
    static one_net_status_t repeat_mh_pkt(on_txn_t ** txn)
    {
        // temporarily commenting the real function out so things compile
        #if 1
        return ONS_SUCCESS;
        #else // trivial function above, real function below
        one_net_status_t status;
        UInt8 max_hops, hops_left;

        // read the pid
        if(one_net_read(&(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX]),
          ON_ENCODED_PID_SIZE) != ON_ENCODED_PID_SIZE)
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        switch(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX])
        {
            case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
            {
                mh_txn.data_len = ON_ENCODED_INVITE_PKT_LEN;
                break;
            } // ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT case //

            case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:    // fall through
            case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
            case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK:   // fall through
            case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:     // fall through
            case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK:    // fall through
            case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
            {
                mh_txn.data_len = ON_ACK_NACK_LEN;
                break;
            } // ACK/NACK cases //

            #ifdef _TRIPLE_HANDSHAKE
            case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
            case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
            {
                mh_txn.data_len = ON_TXN_ACK_LEN;
                break;
            } // multi-hop single/block transaction ack case //
			#endif

            case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
            case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
            {
                mh_txn.data_len = ON_ENCODED_SINGLE_DATA_LEN;
                break;
            } // (repeat)single data case //

            case ONE_NET_ENCODED_MH_BLOCK_DATA:         // fall through
            case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
            case ONE_NET_ENCODED_MH_STREAM_DATA:
            {
                // don't read in the hops field (which is included in the
                // length of this packet).
                mh_txn.data_len = ONE_NET_MAX_ENCODED_PKT_LEN
                  - ON_ENCODED_HOPS_SIZE;
                break;
            } // block and stream data case //

            case ONE_NET_ENCODED_MH_DATA_RATE_TEST:
            {
                mh_txn.data_len = ON_DATA_RATE_PKT_LEN;
                break;
            } // ONE_NET_ENCODED_MH_DATA_RATE_TEST case //

            default:
            {
                return ONS_UNHANDLED_PKT;
                break;
            } // default case //
        } // switch(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX]) //

        // read the rest of the packet (except for the hops field).  +1 since
        // 1 more byte than current index has been read.
        if(one_net_read(&(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX
          + ON_ENCODED_PID_SIZE]), mh_txn.data_len
          - (ONE_NET_ENCODED_PID_IDX + 1)) != mh_txn.data_len
          - (ONE_NET_ENCODED_PID_IDX + 1))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        if((status = on_read_and_parse_hops(&max_hops, &hops_left))
          != ONS_SUCCESS)
        {
            return status;
        } // if reading and parsing the hops field failed //

        if(hops_left)
        {
            hops_left--;
            if((status = on_build_hops(&(mh_txn.pkt[mh_txn.data_len++]),
              max_hops, hops_left)) == ONS_SUCCESS)
            {
                // successfully received and rebuilt the hops field of a
                // Multi-Hop packet, so now send it back out.
                *txn = &mh_txn;
                on_state = ON_SEND_PKT;
            } // if on_build_hops was successful //
        } // if hops remain //

        return status;
        #endif // temporarily commenting function out
    } // repeat_mh_pkt //
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //


#ifdef _ONE_NET_DEBUG
    void one_net_debug(UInt8 debug_type, UInt8 * data, UInt16 length)
    {
        tick_t ticks;
        UInt16 low_tick_count;
        UInt8 i;

        ticks = get_tick_count();
        low_tick_count = (UInt16) ticks;

        // print the debug message prefix
        oncli_send_msg(ONCLI_DEBUG_PREFIX_STR, low_tick_count, debug_type);
        oncli_send_msg(ONCLI_DEBUG_LENGTH_STR, length);

        //
        // display data from data as needed
        //
#if 1   
        if (length < 10)
        {
            //
            // it must not be a full message, just print the debug data
            //
            for (i=0; i<length; i++)
            {
                oncli_send_msg(ONCLI_DEBUG_BYTE_STR, data[i]);
            }
        }
        else
        {
            //
            // this must be a message, print the interesting parts to 
            // save time
            //
            for (i=4; i<=17; i++)
            {
                if (i < length)
                {
                    oncli_send_msg(ONCLI_DEBUG_BYTE_STR, data[i]);
                }
                else
                {
                    break;
                }
            }
        }
#endif

        // print the debug message suffix
        oncli_send_msg(ONCLI_DEBUG_SUFFIX_STR);

        // UART output seems to interfere with RF output
        while(uart_tx_bytes_free() < uart_tx_buffer_size());

    } // one_net_debug //
#endif


//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET

