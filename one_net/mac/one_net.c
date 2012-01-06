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
#include "one_net_packet.h"
#include "one_net.h"
#include "tal.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"
#include "one_net_xtea.h"
#include "one_net_acknowledge.h"
#ifdef _PEER
#include "one_net_peer.h"
#endif
#ifdef _ONE_NET_CLIENT
#include "one_net_client_port_specific.h"
#endif



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Header(Preamble and SOF)
const UInt8 HEADER[] = {0x55, 0x55, 0x55, 0x33};


enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};


const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0xB4, 0xB4};
const on_raw_did_t ON_RAW_BROADCAST_DID = {0x00, 0x00};
const on_raw_did_t MASTER_RAW_DID = {0x00, 0x10};
const on_encoded_did_t MASTER_ENCODED_DID = {0xB4, 0xBC};

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
//                              PUBLIC VARIABLES



//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
UInt8 nv_param[NV_PARAM_SIZE_BYTES];

//! The base parameters for the device
on_base_param_t* const on_base_param = (on_base_param_t* const) nv_param;


//! The set of packet handlers
on_pkt_hdlr_set_t pkt_hdlr;

//! a function to retrieve the sender information
one_net_get_sender_info_func_t get_sender_info;

//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, 0, NULL,
  NULL, NULL};

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  0, NULL, NULL, NULL};

#ifdef _BLOCK_MESSAGES_ENABLED
    //! The current block transaction
    on_txn_t block_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BLOCK_TIMER, 0, 0, NULL, NULL, NULL};

    #ifdef _STREAM_MESSAGES_ENABLED
    
    //! The current stream transaction
    on_txn_t stream_txn = {ON_STREAM, ONE_NET_NO_PRIORITY, 0,
      ONT_STREAM_TIMER, 0, 0, NULL, NULL, NULL};    
    #endif
#endif // if block messages are not enabled //



//! true if device is functioning as a master, false otherwise
#ifndef _ONE_NET_MASTER
BOOL device_is_master = FALSE;
#else
BOOL device_is_master = TRUE; // if device cvan be master OR client, the
                              // initialization code will need to set this
                              // value
#endif


one_net_startup_status_t startup_status = ON_STARTUP_IN_PROGRESS;


//! an on_pkt_t structure for data packets
on_pkt_t data_pkt_ptrs;

//! an on_pkt_t structure for response packets
on_pkt_t response_pkt_ptrs;


//! A place to store a single message with payload.
on_single_data_queue_t single_msg;

//! A place to store the single message raw payload.
UInt8 single_data_raw_pld[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN];

//! Pointer to the current single message being sent.  If none, this will be
//! NULL.  Generally this will point to single_msg.
on_single_data_queue_t* single_msg_ptr = NULL;


//! A place to store the raw packet bytes when encrypting, decrypting, etc.
//! so that it will not have to be declared inside of functions and risk a
//! overflow.
UInt8 raw_payload_bytes[ON_MAX_RAW_PLD_LEN + 1];

#ifdef _ONE_NET_CLIENT
extern BOOL client_joined_network; // declared extern in one_net_client.h but
     // we do not want to include one_net_client.h so we declare it here too.
#endif

//! The current invite transaction
on_txn_t invite_txn = {ON_INVITE, ONE_NET_NO_PRIORITY, 0,
#ifdef _ONE_NET_MASTER
  ONT_INVITE_SEND_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL};
#else
  ONT_INVITE_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL};
#endif
  
#ifdef _ONE_NET_CLIENT
extern BOOL client_looking_for_invite;
#endif

//! A buffer containing all encoded bytes for transmitting and receiving
UInt8 encoded_pkt_bytes[ENCODED_BYTES_BUFFER_LEN];


//                              PUBLIC VARIABLES
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{


//! The current state.
on_state_t on_state = ON_INIT_STATE;


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    // Transaction for forwarding on MH packets.
    static on_txn_t mh_txn = {ON_NO_TXN, ONE_NET_LOW_PRIORITY, 0,
      ONT_MH_TIMER, 0, 0, encoded_pkt_bytes};
#endif


#ifdef _RANGE_TESTING
//! Stores the DIDs of the devices which are in range.
static on_encoded_did_t range_test_did_array[RANGE_TESTING_ARRAY_SIZE];

//! If true, trange test.  If false, do not.
static BOOL range_testing_on = FALSE;
#endif


#ifdef _ONE_NET_CLIENT
//! If true and sending a single response, wthis flag signifies that we
// should instead send our features.
static BOOL features_override = FALSE;
#endif


//! @} ONE-NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{



static BOOL check_for_clr_channel(void);
static on_message_status_t rx_single_resp_pkt(on_txn_t** const txn,
  on_txn_t** const this_txn, on_pkt_t* const pkt,
  UInt8* const raw_payload_bytes, on_ack_nack_t* const ack_nack);



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{
    
    
#ifdef _ONE_NET_MULTI_HOP
/*!
    \brief Builds the encoded hops field for the packet.

    \param[out] enc_hops_field The hops field to be sent with the pkt.
    \param[in] hops The number of hops taken so far.
    \param[in] max_hops maximum number of hops the packet can take.

    \return ONS_SUCCESS If building the hops field was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_build_hops(UInt8 * enc_hops_field, UInt8 hops,
  UInt8 max_hops)
{
    UInt8 raw_hops;

    if(!enc_hops_field || max_hops > ON_MAX_HOPS_LIMIT || hops > max_hops)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    raw_hops = ((max_hops << ON_MAX_HOPS_BUILD_SHIFT) &
      ON_MAX_HOPS_BUILD_MASK) | ((hops << ON_HOPS_BUILD_SHIFT)
      & ON_HOPS_BUILD_MASK);

    on_encode(enc_hops_field, &raw_hops, ON_ENCODED_HOPS_SIZE);

    return ONS_SUCCESS;
} // on_build_hops //


/*!
    \brief Parses the encoded hops field for the packet.

    \param[in] enc_hops_field The hops field to be sent with the pkt.
    \param[out] hops The number of hops taken so far.
    \param[out] max_hops maximum number of hops the packet can take.

    \return ONS_SUCCESS If parsing the hops field was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_parse_hops(UInt8 enc_hops_field, UInt8* hops,
  UInt8* max_hops)
{
    UInt8 raw_hops_field;
    one_net_status_t status;

    if(!hops || !max_hops)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if((status = on_decode(&raw_hops_field, &enc_hops_field,
      ON_ENCODED_HOPS_SIZE)) != ONS_SUCCESS)
    {
        return status;
    }

    *hops = (raw_hops_field >> ON_PARSE_HOPS_SHIFT) &
      ON_PARSE_RAW_HOPS_FIELD_MASK;
    *max_hops = (raw_hops_field >> ON_PARSE_MAX_HOPS_SHIFT) &
      ON_PARSE_RAW_HOPS_FIELD_MASK;

    return ONS_SUCCESS;
} // on_parse_hops //
#endif // ifdef _ONE_NET_MULTI_HOP //


// TODO -- document
one_net_status_t on_parse_response_pkt(UInt8 raw_pid, UInt8* raw_bytes,
  on_ack_nack_t* const ack_nack)
{
    BOOL is_ack = packet_is_ack(raw_pid);
    if(!is_ack && !packet_is_nack(raw_pid))
    {
        return ONS_BAD_PKT_TYPE;
    }
    
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = get_ack_nack_handle(raw_bytes);
    raw_bytes += ON_PLD_DATA_IDX;
    
    if(!is_ack)
    {
        ack_nack->nack_reason = *raw_bytes;
        raw_bytes++;
    }
    
    ack_nack->payload = (ack_nack_payload_t*) raw_bytes;

    // fill in the payload based on the handle
    {
        BOOL val_present = FALSE;
        switch(ack_nack->handle)
        {
            case ON_ACK_FEATURES:
            case ON_ACK_KEY_FRAGMENT:
            case ON_ACK_STATUS:
	        case ON_ACK_DATA:
            case ON_ACK_ADMIN_MSG:
                // nothing to do with these.
                break;
	        case ON_ACK_VALUE:
                val_present = TRUE;
                if(is_ack)
                {
                    raw_bytes++;  // first byte is UInt8, no endian conversion
                }
                
                break;
	        case ON_ACK_TIME_MS:
            case ON_ACK_TIMEOUT_MS:
            case ON_ACK_SLOW_DOWN_TIME_MS:
            case ON_ACK_SPEED_UP_TIME_MS:
                val_present = TRUE;
                break;
        }
        
        if(val_present)
        {              
            // reverse the bytes if necessary
            // assign it to nack_value.  Doesn't matter.  They all point
            // to the same place
            ack_nack->payload->nack_value = one_net_byte_stream_to_int32(
              raw_bytes);
        }
    } 
    
    return ONS_SUCCESS;
}


one_net_status_t on_build_response_pkt(on_ack_nack_t* ack_nack,
  on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device,
  BOOL stay_awake)
{
    UInt8 status;
    SInt8 raw_pld_len = get_raw_payload_len(pkt_ptrs->raw_pid);
    SInt8 num_words = get_encoded_payload_len(pkt_ptrs->raw_pid);
    BOOL is_ack = (ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR);
    UInt8* ack_nack_pld_ptr = &raw_payload_bytes[ON_PLD_DATA_IDX];
    UInt8 ack_nack_pld_len = raw_pld_len - 1 - ON_PLD_DATA_IDX;
    
    // change pid if necessary
    pkt_ptrs->raw_pid = get_single_response_pid(pkt_ptrs->raw_pid, is_ack,
      stay_awake);
    *(pkt_ptrs->enc_pid) = decoded_to_encoded_byte(pkt_ptrs->raw_pid, FALSE);
    

    // for all we know, ack_nack->payload is located at the same address
    // as the raw_payload_bytes[] array.  We don't want to overwrite without
    // knowing, so we'll copy the payload to a buffer that is going to be
    // overwritten anyway to make sure.
    one_net_memmove(pkt_ptrs->payload, ack_nack->payload, ack_nack_pld_len);
    
    // now move the pointer.
    ack_nack->payload = (ack_nack_payload_t*) pkt_ptrs->payload;

    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
    }
    
    // make the payload portion all random first for extra security
    {
        UInt8 i;
        for(i = ON_PLD_DATA_IDX; i < raw_pld_len - 1; i++)
        {
            raw_payload_bytes[i] = one_net_prand(get_tick_count(), 255);
        }
    }
    
    // build the ack and nack
    if(!is_ack)
    {
        *ack_nack_pld_ptr = ack_nack->nack_reason;
        ack_nack_pld_ptr++;
        ack_nack_pld_len--;
    }
    
    
    // fill in the payload based on the handle
    {
        UInt32 val;
        BOOL val_present = FALSE;
        switch(ack_nack->handle)
        {
            case ON_ACK_FEATURES:
            case ON_ACK_KEY_FRAGMENT:
                // both features and key fragments are 4 bytes long.
                one_net_memmove(ack_nack_pld_ptr, ack_nack->payload, 4);
                break;
            case ON_ACK_STATUS:
	        case ON_ACK_DATA:
            case ON_ACK_ADMIN_MSG:
                one_net_memmove(ack_nack_pld_ptr, ack_nack->payload,
                  ack_nack_pld_len);
                break;
	        case ON_ACK_VALUE:
                val_present = TRUE;
                val = ack_nack->payload->nack_value;
                if(is_ack)
                {
                    *ack_nack_pld_ptr = ack_nack->payload->ack_value.uint8;
                    ack_nack_pld_ptr++;
                    val = ack_nack->payload->ack_value.uint32;
                }
                break;
	        case ON_ACK_TIME_MS:
            case ON_ACK_TIMEOUT_MS:
            case ON_ACK_SLOW_DOWN_TIME_MS:
            case ON_ACK_SPEED_UP_TIME_MS:
                val_present = TRUE;
                val = ack_nack->payload->ack_time_ms;
                break;
        }
        
        if(val_present)
        {
            one_net_int32_to_byte_stream(val, ack_nack_pld_ptr);
        }
    }

    #ifdef _ONE_NET_MULTI_HOP
    // change between multi-hop and non-multi-hop depending on whether 
    // txn->max_hops is positive.
    set_multihop_pid(&(pkt_ptrs->raw_pid), txn->max_hops > 0);
    *(pkt_ptrs->enc_pid) = decoded_to_encoded_byte(pkt_ptrs->raw_pid, FALSE);
    
    if(txn->max_hops > 0)
    {
        // build hops
        if((status = on_build_hops(pkt_ptrs->enc_hops_field, 0,
          txn->max_hops)) != ONS_SUCCESS)
        {
            return status;
        }
        
        // put it back into the packet pointers.
        on_parse_hops(*(pkt_ptrs->enc_hops_field), &(pkt_ptrs->hops),
          &(pkt_ptrs->max_hops));        
    }
    #endif


    // build the packet
    put_payload_txn_nonce(device->send_nonce, raw_payload_bytes);
    put_payload_resp_nonce(device->expected_nonce, raw_payload_bytes);
      
    // fill in the ack/nack handle (The 4 LSB of raw data byte 2)
	put_ack_nack_handle(ack_nack->handle, raw_payload_bytes);

    // compute the crc
    raw_payload_bytes[0] = (UInt8)one_net_compute_crc(
      &raw_payload_bytes[ON_PLD_CRC_SIZE], (raw_pld_len - 1) - ON_PLD_CRC_SIZE,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
      
    if((status = on_encrypt(txn->txn_type, raw_payload_bytes, txn->key,
      raw_pld_len)) == ONS_SUCCESS)
    {
        status = on_encode(pkt_ptrs->payload, raw_payload_bytes,
          num_words);
    } // if encrypting was successful //

    return status;
}


one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  const on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device)
{
    UInt8 status;
    SInt8 raw_pld_len = get_raw_payload_len(pkt_ptrs->raw_pid);
    SInt8 num_words = get_encoded_payload_len(pkt_ptrs->raw_pid);
    
    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
    }

    
    #ifdef _ONE_NET_MULTI_HOP
    // change between multi-hop and non-multi-hop depending on whether 
    // max_hops is positive.
    set_multihop_pid(&(pkt_ptrs->raw_pid), pkt_ptrs->max_hops > 0);
    *(pkt_ptrs->enc_pid) = decoded_to_encoded_byte(pkt_ptrs->raw_pid, FALSE);
    
    if(pkt_ptrs->max_hops > 0)
    {
        // build hops
        if((status = on_build_hops(pkt_ptrs->enc_hops_field, pkt_ptrs->hops,
          pkt_ptrs->max_hops)) != ONS_SUCCESS)
        {
            return status;
        }
    }
    #endif

    // build the packet
    put_payload_txn_nonce(device->send_nonce, raw_payload_bytes);
    put_payload_resp_nonce(device->expected_nonce, raw_payload_bytes);
    
    #ifdef _ONE_NET_CLIENT
    // If features_override is true, the other device needs our features or we
    // need theirs, so we'll send ours, which will cause them to send theirs
    // back, then the next time around, we'll send the real message.
    if(features_override)
    {
        // sending features
        put_payload_msg_type(ON_FEATURE_MSG, raw_payload_bytes);
        one_net_memmove(&raw_payload_bytes[ON_PLD_DATA_IDX],
          &THIS_DEVICE_FEATURES, sizeof(on_features_t));
        features_override = FALSE; // set false.  It will be set again next
                                   // time if it needs to be.
    }
    else
    #endif
    {
        // sending the real message
        put_payload_msg_type(msg_type, raw_payload_bytes);
        one_net_memmove(&raw_payload_bytes[ON_PLD_DATA_IDX], raw_pld,
          (raw_pld_len - 1) - ON_PLD_DATA_IDX);
    }
      
    // compute the crc
    raw_payload_bytes[0] = (UInt8)one_net_compute_crc(
      &raw_payload_bytes[ON_PLD_CRC_SIZE], (raw_pld_len - 1) - ON_PLD_CRC_SIZE,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
      
    if((status = on_encrypt(txn->txn_type, raw_payload_bytes, txn->key,
      raw_pld_len)) == ONS_SUCCESS)
    {
        status = on_encode(pkt_ptrs->payload, raw_payload_bytes,
          num_words);
    } // if encrypting was successful //

    return status;
}


one_net_status_t on_build_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_nid_t* nid, const on_encoded_did_t* repeater_did,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did)
{
    if(!pkt_ptrs || !nid || !repeater_did || !dst_did || !src_did)
    {
        return ONS_BAD_PARAM;
    }
    
    one_net_memmove(pkt_ptrs->enc_nid, *nid, ON_ENCODED_NID_LEN);
    one_net_memmove(pkt_ptrs->enc_repeater_did, *repeater_did,
      ON_ENCODED_DID_LEN);
    one_net_memmove(pkt_ptrs->enc_dst_did, *dst_did,
      ON_ENCODED_DID_LEN);
    one_net_memmove(pkt_ptrs->enc_src_did, *src_did,
      ON_ENCODED_DID_LEN);
      
    return ONS_SUCCESS;
}


one_net_status_t on_build_my_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did)
{
    on_encoded_nid_t* nid = (on_encoded_nid_t*) on_base_param->sid;
    on_encoded_did_t* repeater_did = (on_encoded_did_t*)
      (&on_base_param->sid[ON_ENCODED_NID_LEN]);
    if(src_did == NULL)
    {
        // source must be the same as repeater (i.e. we are originating)
        src_did = repeater_did;
    }
    
    return on_build_pkt_addresses(pkt_ptrs, nid, repeater_did, dst_did,
      src_did);
}


one_net_status_t on_complete_pkt_build(on_pkt_t* pkt_ptrs,
  UInt8 msg_id, UInt8 pid)
{
    UInt8 msg_crc_calc_len;
    UInt8* msg_crc_start;
    
    if(!pkt_ptrs)
    {
        return ONS_BAD_PARAM;
    }
    
    // A quick check of the payload length to make sure it's been set.
    // Should not be necessary, but check anyway.
    if(pkt_ptrs->payload_len > ON_MAX_ENCODED_PLD_LEN_WITH_TECH)
    {
        return ONS_INTERNAL_ERR;
    }
    
    // message CRC calculation length includes everything past the message CRC
    // and stops immediately BEFORE the hops field, if any, which is NOT part
    // of the message CRC.
    msg_crc_start = pkt_ptrs->enc_msg_crc + ONE_NET_ENCODED_MSG_CRC_LEN;
    msg_crc_calc_len = (pkt_ptrs->payload + pkt_ptrs->payload_len) -
      msg_crc_start;
    
    // stick the message id into pkt_ptrs if it isn't already there.
    // TODO - If we have the message ID and the CRC and the payload length
    // in the packet pointers and the payload length, why not the pid?
    // There are some redundancies and confusion about what structure should
    // store what.  Whenever two structures store the same thing, you have to
    // decide whether that is
    // worth it and be careful to update BOTH structures when needed.
    pkt_ptrs->msg_id = msg_id;
    
    // we shift in order to encode.  We saved the unshifted message id before
    // shifting.
    msg_id <<= 2;
    on_encode(pkt_ptrs->enc_msg_id, &msg_id, ONE_NET_ENCODED_MSG_ID_LEN);
    
    #ifdef _ONE_NET_MULTI_HOP
    // fill in hops if needed
    if(packet_is_multihop(pid))
    {
        one_net_status_t status;
        if((status = on_build_hops(pkt_ptrs->enc_hops_field, pkt_ptrs->hops,
          pkt_ptrs->max_hops) != ONS_SUCCESS))
        {
            return status;
        }
    }
    #endif
    
    // preamble and start of frame
    one_net_memmove(pkt_ptrs->packet_header, HEADER, sizeof(HEADER));

    // we have everything filled in but the the msg_crc, so we can calculate
    // it now.
    pkt_ptrs->msg_crc = (UInt8) one_net_compute_crc(msg_crc_start,
      msg_crc_calc_len, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    // we are only interested in the 6 most significant bits, so mask them
    pkt_ptrs->msg_crc &= 0xFC; // 11111100 -- six most significant bits.
    // now encode for the message
    on_encode(pkt_ptrs->enc_msg_crc, &(pkt_ptrs->msg_crc),
      ONE_NET_ENCODED_MSG_ID_LEN);
      
    return ONS_SUCCESS;
}


/*!
    \brief Calculates the decoded message CRC of a packet

    \param[in] pkt_ptrs the filled-in packet

    \return the message crc
*/
UInt8 calculate_msg_crc(const on_pkt_t* pkt_ptrs)
{
    // message CRC calculation length includes everything past the message CRC
    // and stops immediately BEFORE the hops field, if any, which is NOT part
    // of the message CRC.
    UInt8* msg_crc_start = pkt_ptrs->enc_msg_crc + ONE_NET_ENCODED_MSG_CRC_LEN;
    UInt8 msg_crc_calc_len = (pkt_ptrs->payload + pkt_ptrs->payload_len) -
      msg_crc_start;
      
    UInt8 msg_crc = (UInt8) one_net_compute_crc(msg_crc_start,
      msg_crc_calc_len, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
      
    // we are only interested in the 6 most significant bits, so mask them
    return msg_crc & 0xFC; // 11111100 -- six most significant bits.
}


/*!
    \brief Verify that the message CRC is valid

    \param[in] pkt_ptrs the filled-in packet

    \return TRUE if the message CRC is valid, FALSE otherwise
*/
BOOL verify_msg_crc(const on_pkt_t* pkt_ptrs)
{
    UInt8 enc_crc;
    UInt8 raw_crc = calculate_msg_crc(pkt_ptrs);
    
    if(on_encode(&enc_crc, &raw_crc, ONE_NET_ENCODED_MSG_CRC_LEN) !=
      ONS_SUCCESS)
    {
        return FALSE;
    }
    
    return (*(pkt_ptrs->enc_msg_crc) == enc_crc);
}


/*!
    \brief Verify that the payload CRC is valid

    \param[in] raw_pid The raw pid of the packet
    \param[in] decrypted The decrypted bytes

    \return TRUE if the message CRC is valid, FALSE otherwise
*/
BOOL verify_payload_crc(UInt8 raw_pid, const UInt8* decrypted)
{
    const UInt8* crc_calc_start;
    UInt8 crc_calc, crc_calc_len;
    SInt8 num_encoded_blocks = get_num_payload_blocks(raw_pid);
    
    if(num_encoded_blocks == 0)
    {
        return TRUE; // no CRC to check.
    }
    else if(num_encoded_blocks < 0)
    {
        return FALSE; // invalid CRC
    }
    
    if(!decrypted)
    {
        return FALSE; // bad parameter
    }
    
    crc_calc_len = (UInt8) (num_encoded_blocks * ONE_NET_XTEA_BLOCK_SIZE -
      ON_PLD_CRC_SIZE);
    crc_calc_start = &decrypted[ON_PLD_CRC_SIZE];
    crc_calc = (UInt8) one_net_compute_crc(crc_calc_start, crc_calc_len,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    
    return (crc_calc == decrypted[0]); // CRC is always the very first byte
}


/*!
    \brief Sets the pointers of an on_pkt_t structure.

    \param[in] raw_pid the raw pid of the packet
    \param[in] pkt_bytes The array holding the packet bytes
    \param[out] pkt The on_pkt_t structure to fill

    \return TRUE if the on_pkt structure was set up successfully.
            FALSE upon error.
*/
BOOL setup_pkt_ptr(UInt8 raw_pid, UInt8* pkt_bytes, on_pkt_t* pkt)
{
    SInt8 len = get_encoded_payload_len(raw_pid);
    if(len < 0)
    {
        return FALSE; // bad pid
    }
    
    if(!pkt_bytes || !pkt)
    {
        return FALSE;
    }
    
    pkt->packet_header    = &pkt_bytes[0];
    pkt->enc_pid          = &pkt_bytes[ONE_NET_ENCODED_PID_IDX];
    pkt->raw_pid          = raw_pid;
    *(pkt->enc_pid)       = decoded_to_encoded_byte(raw_pid, FALSE);
    pkt->enc_msg_id       = &pkt_bytes[ONE_NET_ENCODED_MSG_ID_IDX];
    pkt->enc_msg_crc      = &pkt_bytes[ONE_NET_ENCODED_MSG_CRC_IDX];
    pkt->enc_src_did      = (on_encoded_did_t*) &pkt_bytes[ON_ENCODED_SRC_DID_IDX];
    pkt->enc_dst_did      = (on_encoded_did_t*) &pkt_bytes[ONE_NET_ENCODED_DST_DID_IDX];
    pkt->enc_repeater_did = (on_encoded_did_t*) &pkt_bytes[ONE_NET_ENCODED_RPTR_DID_IDX];
    pkt->enc_nid          = (on_encoded_nid_t*) &pkt_bytes[ON_ENCODED_NID_IDX];
    pkt->payload          = &pkt_bytes[ON_PLD_IDX];
    pkt->payload_len      = (UInt8) len;
    
    #ifdef _ONE_NET_MULTI_HOP
    pkt->enc_hops_field = pkt->payload + pkt->payload_len;
    #endif
    
    return TRUE;
}


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
        rounds = ON_XTEA_32_ROUNDS;
        data[payload_len - 1] = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;
	#ifdef _STREAM_MESSAGES_ENABLED
    } // if not stream //
        else
        {
            rounds = ON_XTEA_8_ROUNDS;
            data[payload_len - 1] = ONE_NET_STREAM_ENCRYPT_XTEA8;
        } // else stream //
    #endif // if _STREAM_MESSAGES_ENABLED is not defined //

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

    return ONS_SUCCESS;
} // on_decrypt //


/*!
    \brief Initializes ONE-NET.

    \return void
*/
void one_net_init(void)
{
    one_net_set_channel(on_base_param->channel);
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    empty_queue();
    #endif
    #ifdef _PEER
    one_net_reset_peers();
    #endif
    single_msg_ptr = NULL;
    single_txn.priority = ONE_NET_NO_PRIORITY;
    #ifdef _RANGE_TESTING
    reset_range_test_did_array();
    #endif
} // one_net_init //
    


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
    one_net_status_t status;
    on_txn_t* this_txn;
    on_pkt_t* this_pkt_ptrs;
    #ifndef _ONE_NET_SIMPLE_DEVICE
    static BOOL at_least_one_response = FALSE;
    #endif
    
    on_ack_nack_t ack_nack;
    ack_nack_payload_t ack_nack_payload;
    ack_nack.payload = &ack_nack_payload;

    
    switch(on_state)
    {
        case ON_LISTEN_FOR_DATA:
        {
            #ifdef _ONE_NET_MULTI_HOP
            on_raw_did_t raw_did;
            #endif
            
            // we are listening for data.  Make sure we have nothing
            // pending
            #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
            if(*txn == NULL && single_txn.priority == ONE_NET_NO_PRIORITY
              && single_msg_ptr == NULL)
            #else
            if(*txn == NULL && single_txn.priority == ONE_NET_NO_PRIORITY
              && single_msg_ptr != NULL)
            #endif
            {
                on_sending_device_t* device;
                #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                tick_t next_pop_time = 0;
                #endif
                
                // first see if we're in the middle of a message.
                if(!load_next_recipient(&single_msg, recipient_send_list_ptr))
                {
                    // we are not in the middle of a message.  We might have
                    // something ready to pop though.
                    
                    #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                    int index = single_data_queue_ready_to_send(
                        &next_pop_time);
                    #else
                    int index = single_data_queue_ready_to_send();
                    #endif
                    if(index >=  0)
                    {
                        #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
                        if(pop_queue_element(&single_msg, single_data_raw_pld,
                          (UInt8) index))
                        #else
                        if(pop_queue_element())
                        #endif
                        {
                            on_did_unit_t first_recipient;


                            // we have just popped a message.  Set things up
                            // with the recipient list, then next time the
                            // load_next_recipient will take it from there.
                            recipient_send_list_ptr = &recipient_send_list;
                            

                            one_net_memmove(first_recipient.did, single_msg.dst_did,
                              ON_ENCODED_DID_LEN);
                              
                            // this will only be relevant for ON_APP_MSG
                            // message types.  Fill it in anyway.  If it's
                            // irrelevant, it will be ignroed later on it the
                            // process.
                            first_recipient.unit = get_dst_unit(single_msg.payload);

                            // first clear the list and set up the direct
                            // recipient                            
                            recipient_send_list_ptr->num_recipients = 0;
                            
                            add_recipient_to_recipient_list(
                              recipient_send_list_ptr, &first_recipient);
                            
                            #ifdef _PEER
                            // now add any peers, if they are relevant
                            add_peers_to_recipient_list(&single_msg,
                              recipient_send_list_ptr, peer);
                            #endif
                            
                            #ifdef _ONE_NET_CLIENT
                            // now add the master if needed
                            if(must_send_to_master(&single_msg))
                            {
                                on_did_unit_t master_unit;
                                one_net_memmove(master_unit.did,
                                  &MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);
                                master_unit.unit = ONE_NET_DEV_UNIT;
                                add_recipient_to_recipient_list(
                                  recipient_send_list_ptr, &master_unit);
                            }
                            #endif
                            
                            #ifndef _ONE_NET_SIMPLE_DEVICE
                            (*pkt_hdlr.adj_recip_list_hdlr)(&single_msg, 
                              &recipient_send_list_ptr);
                            #endif

                            if(recipient_send_list_ptr)
                            {
                                #ifndef _ONE_NET_SIMPLE_DEVICE
                                at_least_one_response = FALSE;
                                #endif
                                recipient_send_list_ptr->recipient_index = -1;
                            
                                // we have a message.  We'll take it further
                                // the next trip through.  If the nmber of 
                                return FALSE; // we have no transaction, but
                                              // we have something to send.
                                              // Return false.
                            }
                        }
                    }
                    
                    // we are either done or we never had anything to send in
                    // the first place.  Clear it and return TRUE.
                    single_msg_ptr = NULL;
                    
                    
                    // nothing popped, so look for a packet.  Any packet we
                    // get should be a single packet or possibly, if we are
                    // a repeater and the packet wasn't for us, a repeat
                    // packet.  What we should NOT get is a response, block,
                    // or stream packet.
                    response_txn.pkt =
                      &encoded_pkt_bytes[ON_MAX_ENCODED_DATA_PKT_SIZE];
                    single_txn.pkt = encoded_pkt_bytes;
                    this_txn = &single_txn;
                    *txn = NULL;
                    this_pkt_ptrs = &data_pkt_ptrs;

                    // TODO -- clean this up so there is some variable we can
                    // set for who we expect as the source.
                    #ifdef _ONE_NET_CLIENT
                    if(!device_is_master && !client_joined_network)
                    {
                        status = on_rx_packet(&MASTER_ENCODED_DID,
                          (const on_txn_t* const) *txn, &this_txn, &this_pkt_ptrs,
                          raw_payload_bytes);
                    }
                    else 
                    #endif
                    {
                        status = on_rx_packet(&ON_ENCODED_BROADCAST_DID,
                          (const on_txn_t* const) *txn, &this_txn, &this_pkt_ptrs,
                          raw_payload_bytes);
                    }
            
                    if(status == ONS_PKT_RCVD)
                    {
                        #ifdef _ONE_NET_MH_CLIENT_REPEATER
                        if(this_txn == &mh_txn)
                        {
                            *txn = &mh_txn;
                            mh_txn.priority = ONE_NET_LOW_PRIORITY;
            
                            // copy the preamble / header just in case it isn't there already
                            one_net_memmove(mh_txn.pkt, HEADER, ONE_NET_PREAMBLE_HEADER_LEN);

                            // set the timer to send right away.
                            ont_set_timer(mh_txn.next_txn_timer, 0);
                    
                            on_state = ON_SEND_PKT;
                            return FALSE;
                        }
                        #endif
                        
                        if(this_txn == &single_txn)
                        {
                            on_message_status_t msg_status;
                            
                            UInt8 msg_type =
                              get_payload_msg_type(raw_payload_bytes);
                              
                            ack_nack.payload = (ack_nack_payload_t*)
                              &raw_payload_bytes[ON_PLD_DATA_IDX];
                              
                            // first check for nonces, message ids, features,
                            // etc.  We'll take care of that level of
                            // processing in rx_single_data().  Anything
                            // beyond that will take place in the single data
                            // handler function.
                            msg_status = rx_single_data(&this_txn,
                              this_pkt_ptrs, raw_payload_bytes, &ack_nack);

                            if(msg_status == ON_MSG_CONTINUE)
                            {
                                (*pkt_hdlr.single_data_hdlr)(&this_txn,
                                  this_pkt_ptrs, raw_payload_bytes, &msg_type,
                                  &ack_nack);
                            }

                            if(this_txn == &response_txn)
                            {
                                // we'll send back a reply.
                                *txn = &response_txn;
                                on_state = ON_SEND_SINGLE_DATA_RESP;
                                response_txn.priority = ONE_NET_HIGH_PRIORITY;
                                
                                // send the response immediately.
                                ont_set_timer((*txn)->next_txn_timer, 0);
                            }
                            else
                            {
                                *txn = 0;
                            }
                        }
                    }
                    
                    break;
                }

                // we have a message.  Let's create the packet.
                single_txn.pkt =
                  &encoded_pkt_bytes[ON_MAX_ENCODED_DATA_PKT_SIZE];
                response_txn.pkt = encoded_pkt_bytes;
                
                // first get the sending device info.
                device = (*get_sender_info)((on_encoded_did_t*)
                  single_msg.dst_did);
                if(device == NULL)
                {
                    // an error occurred somewhere.  Abort.
                    return TRUE; // no pending transaction since we're
                                 // aborting
                }
                
                // signifies the time we verified this message with this
                // client to prevent replay attacks.
                device->verify_time = 0;
                

                // pick a message id.  Increment the last one.  If it
                // rolls over, make it 0.
                (device->msg_id)++;
                if(device->msg_id > ON_MAX_MSG_ID)
                {
                    device->msg_id = 0;
                }
                
                if(!setup_pkt_ptr(single_msg.raw_pid, single_txn.pkt,
                  &data_pkt_ptrs))
                {
                    // an error of some sort occurred.  We likely have
                    // a bad pid.  Unrecoverable.  Just abort.
                    return TRUE; // no outstanding transaction
                }
                
                #ifdef _ONE_NET_MULTI_HOP
                on_decode(raw_did, device->did, ON_ENCODED_DID_LEN);
                single_txn.hops = 0;
                single_txn.max_hops = device->hops;

                // give the application code a chance to override if it
                // wants to.
                switch(one_net_adjust_hops(&raw_did, &(single_txn.max_hops)))
                {
                    case ON_MSG_ABORT: return TRUE; // aborting
                }
                
                data_pkt_ptrs.hops = single_txn.hops;
                data_pkt_ptrs.max_hops = single_txn.max_hops;
                #endif

                
                // now fill in the packet
                
                // fill in the addresses
                if(on_build_my_pkt_addresses(&data_pkt_ptrs,
                  (on_encoded_did_t*) single_msg.dst_did,
                  (on_encoded_did_t*) single_msg.src_did) != ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return TRUE; // no outstanding transaction
                }

                // we'll need to fill in the key.  We're dealing with
                // a single transaction here.  Fill in the key.
                #ifdef _ONE_NET_CLIENT
                single_txn.key =
                  (one_net_xtea_key_t*) on_base_param->current_key;
                #endif
                #ifdef _ONE_NET_MASTER
                if(device_is_master)
                {
                    // we're the master.  We may or may not be dealing
                    // with a client using the old key.
                    #ifdef _STREAM_MESSAGES_ENABLED
                    single_txn.key = master_get_encryption_key(ON_SINGLE,
                      (on_encoded_did_t*) single_msg.dst_did);
                    #else
                    single_txn.key = master_get_encryption_key(
                      (on_encoded_did_t*) single_msg.dst_did);
                    #endif
                }
                #endif
                
                // It's a data packet.  Fill in the data portion
                #ifdef _ONE_NET_CLIENT
                features_override = features_override ||
                  !features_known(device->features);
                #endif
                if(on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn,
                  device)!= ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return TRUE; // no outstanding transaction
                }

                // now finish building the packet.
                if(on_complete_pkt_build(&data_pkt_ptrs,
                  device->msg_id, single_msg.raw_pid) != ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return TRUE; // no outstanding transaction                            
                }

                // packet was built successfully.  Set the transaction,
                // state, etc.
                single_txn.priority = single_msg.priority;
                single_txn.data_len =
                  get_encoded_packet_len(single_msg.raw_pid, TRUE);
                *txn = &single_txn;
                (*txn)->retry = 0;
                (*txn)->response_timeout = one_net_response_time_out;
                (*txn)->device = device;
                on_state = ON_SEND_SINGLE_DATA_PKT;
                // set the timer to send immediately
                ont_set_timer((*txn)->next_txn_timer, 0);
                single_msg_ptr = &single_msg;
                
                (*txn)->send = TRUE;
                
                return FALSE; // transaction is not complete
            }

            break;
        } // case ON_LISTEN_FOR_DATA //
        
        case ON_SEND_PKT:
        #ifdef _ONE_NET_MASTER
        case ON_SEND_INVITE_PKT:
        #endif
        case ON_SEND_SINGLE_DATA_PKT:
        case ON_SEND_SINGLE_DATA_RESP:
        {
            if(ont_inactive_or_expired((*txn)->next_txn_timer)
              && check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, get_encoded_packet_len(
                   encoded_to_decoded_byte(
                   (*txn)->pkt[ONE_NET_ENCODED_PID_IDX], FALSE), TRUE));
                on_state++;
            } // if the channel is clear //
            
            break;
        } // case ON_SEND_SINGLE_DATA_PKT //
        
        case ON_SEND_PKT_WRITE_WAIT:
        #ifdef _ONE_NET_MASTER
        case ON_SEND_INVITE_PKT_WRITE_WAIT:
        #endif
        case ON_SEND_SINGLE_DATA_WRITE_WAIT:
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                #ifdef _ONE_NET_MULTI_HOP
                UInt32 new_timeout_ms = (*txn)->max_hops * ONE_NET_MH_LATENCY
                  + (1 + (*txn)->max_hops) * (*txn)->response_timeout;
                
                #ifdef _ONE_NET_MASTER
                if(on_state == ON_SEND_INVITE_PKT_WRITE_WAIT)
                {
                    // invite packets don't lengthen timeout with
                    // multi-hop
                    new_timeout_ms = invite_txn.response_timeout;
                }
                #endif
                
                #else
                UInt32 new_timeout_ms = (*txn)->response_timeout;
                #endif
                
                if(on_state == ON_SEND_PKT_WRITE_WAIT || on_state ==
                  ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT)
                {
                    // no timers are needed, no response is needed, we're just
                    // doing a quick write and that's it.  There is no
                    // follow-up, so we are done.  Reset the transaction and
                    // return the state to ON_LISTEN_FOR_DATA.
                    (*txn)->priority = ONE_NET_NO_PRIORITY;
                    *txn = NULL;
                    ont_stop_timer((*txn)->next_txn_timer);
                    on_state = ON_LISTEN_FOR_DATA;
                    return TRUE;
                }
                
                #ifdef _ONE_NET_MASTER  
                if(on_state == ON_SEND_INVITE_PKT_WRITE_WAIT)
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    ont_set_timer(invite_txn.next_txn_timer, MS_TO_TICK(
                      new_timeout_ms));
                    *txn = 0;
                }
                else
                {
                    ont_set_timer(ONT_RESPONSE_TIMER, MS_TO_TICK(new_timeout_ms));
                    on_state++;
                }
                #else
                on_state++;
                #endif
            } // if write is complete //
            break;
        } // send single data write wait case //
        
        case ON_WAIT_FOR_SINGLE_DATA_RESP:
        {
            on_message_status_t msg_status;
            BOOL terminate_txn = FALSE;
            BOOL response_msg_or_timeout = FALSE; // will be set to true if
                 // the response timer timed out or there was a response
            
            #ifndef _ONE_NET_SIMPLE_DEVICE
            // send right away unless overruled later.
            UInt32 next_send_pause_time = 0;
            #endif
            
            if(ont_inactive_or_expired(ONT_RESPONSE_TIMER))
            {
                #ifndef _ONE_NET_SIMPLE_DEVICE
                // we timed out without a response.  Notify application code
                // it's unlikely they'll decide to change the response time-
                // out, but they might.

                ack_nack_payload.nack_time_ms = (*txn)->response_timeout;
                ack_nack.handle = ON_NACK_TIMEOUT_MS;
                ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE;
                #endif
                
                response_msg_or_timeout = TRUE;
                (*txn)->retry++;
                
                #ifndef _ONE_NET_SIMPLE_DEVICE
                msg_status = (*pkt_hdlr.single_ack_nack_hdlr)(&single_txn,
                  &data_pkt_ptrs, single_msg_ptr->payload,
                  &(single_msg_ptr->msg_type), &ack_nack);
                  
                // we may have been given a pause by the application code,
                // so check if it has changed the nack handle to
                // ON_NACK_PAUSE_TIME_MS.  If so, we'll adjust the
                // next_send_pause_time variable.
                if(ack_nack.handle == ON_NACK_PAUSE_TIME_MS)
                {
                    next_send_pause_time = ack_nack_payload.nack_time_ms;
                }
                  
                // things may or may not have been changed.  One thing
                // that would change would be if the message was terminated
                // or failed in one way or another, so check msg_status
                // first.  The ACK-NACK handler will set this return value
                // if we have timed out too many times.
                switch(msg_status)
                {
                    case ON_MSG_DEFAULT_BHVR: case ON_MSG_CONTINUE:
                    {
                        // we continue.  The response timeout may or may not
                        // have been changed.  We don't care here.
                        msg_status = ON_MSG_TIMEOUT;
                        terminate_txn = ((*txn)->retry >= ON_MAX_RETRY);
                        break;
                    }
                    default:
                    {
                        // transaction has been terminated either by ONE-NET
                        // or by the application code.
                        terminate_txn = TRUE;
                    }
                }
                #else
                msg_status = ON_MSG_TIMEOUT;
                terminate_txn = ((*txn)->retry >= ON_MAX_RETRY);
                #endif
            }
            else
            {
                this_txn = &response_txn;
                this_pkt_ptrs = &response_pkt_ptrs;
                status = on_rx_packet((const on_encoded_did_t * const)
                  &(single_txn.pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                  &single_txn, &this_txn, &this_pkt_ptrs,
                  raw_payload_bytes);
            
                if(status == ONS_PKT_RCVD)
                {
                    response_msg_or_timeout = TRUE;
                    msg_status = rx_single_resp_pkt(txn, &this_txn,
                      this_pkt_ptrs, raw_payload_bytes, &ack_nack);
                      
                    switch(msg_status)
                    {
                        case ON_MSG_IGNORE:
                            response_msg_or_timeout = FALSE;
                            break;
                        default:
                            terminate_txn = ((*txn)->retry >= ON_MAX_RETRY ||
                              this_txn == 0);
                            #ifndef _ONE_NET_SIMPLE_DEVICE
                            at_least_one_response = TRUE;
                            #endif
                    }
                }
            }
            
            if(nack_reason_is_fatal(ack_nack.nack_reason))
            {
                // if nothing else set this terminate flag, something
                // somewhere set a fatal NACK, which means this transaction
                // will never work, so we'll terminate.  Why bother going
                // through a bunch of retries that will never work?
                terminate_txn = TRUE;
            }

            if(!terminate_txn && response_msg_or_timeout)
            {
                // rebuild the packet                
                if(setup_pkt_ptr(single_msg.raw_pid, single_txn.pkt,
                  &data_pkt_ptrs) && on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn,
                  (*txn)->device) == ONS_SUCCESS && on_complete_pkt_build(
                  &data_pkt_ptrs, (*txn)->device->msg_id, single_msg.raw_pid)
                  == ONS_SUCCESS)
                {
                    // do nothing. everything worked.
                }
                else
                {
                    // an error of some sort occurred.  Abort.
                    terminate_txn = TRUE;
                    msg_status = ON_MSG_INTERNAL_ERR;
                }
                
                if(!terminate_txn)
                {
                    #ifndef _ONE_NET_SIMPLE_DEVICE
                    ont_set_timer((*txn)->next_txn_timer,
                      MS_TO_TICK(next_send_pause_time));
                    #else
                    // send right away.
                    ont_set_timer((*txn)->next_txn_timer, 0);
                    #endif

                    on_state -= 2;
                }
            }
            
            if(terminate_txn)
            {
                // transaction has been terminated either by ONE_NET
                // or by the application code.
                
                if((msg_status == ON_MSG_DEFAULT_BHVR || msg_status ==
                  ON_MSG_CONTINUE) && ack_nack.nack_reason !=
                    ON_NACK_RSN_NO_ERROR)
                {
                      msg_status = ON_MSG_FAIL;
                }

                #ifndef _ONE_NET_SIMPLE_DEVICE
                // if we get no reponse this last try and we NEVER got any
                // reponse, we'll make the nack reason
                // ON_NACK_RSN_NO_RESPONSE_TXN.
                if(ack_nack.nack_reason == ON_NACK_RSN_NO_RESPONSE &&
                  !at_least_one_response)
                {
                    // if we got no response ever from the other device.
                    ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE_TXN;
                }

                (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                  single_msg_ptr->payload,
                  &(single_msg_ptr->msg_type), msg_status, &ack_nack);
                #else
                
                (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                  single_msg_ptr->payload, &(single_msg_ptr->msg_type),
                  msg_status, &ack_nack);                    
                #endif
                  
                // clear the transaction.
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                *txn = 0;
                single_msg_ptr = 0;
                on_state = ON_LISTEN_FOR_DATA;
                return TRUE;
            }
            
            break;
        } // case ON_WAIT_FOR_SINGLE_DATA_RESP
    }
    
    return FALSE;
} // one_net //


#ifdef _ONE_NET_MULTI_HOP
SInt8 one_net_set_hops(const on_raw_did_t* const raw_did, UInt8 hops)
{
    on_encoded_did_t enc_did;
    on_sending_device_t* device;

    
    if(!raw_did || on_encode(enc_did, *raw_did, ON_ENCODED_DID_LEN) !=
      ONS_SUCCESS)
    {
        return -1;
    }
    
    device = (*get_sender_info)(&enc_did);
    if(device == NULL)
    {
        return -1;
    }
    
    if(!features_known(device->features))
    {
        return -1;
    }
    
    if(hops > features_max_hops(device->features))
    {
        hops = features_max_hops(device->features);
    }
    
    if(hops > device->max_hops)
    {
        device->max_hops = hops;
    }
    
    device->hops = hops;
    return (SInt8) hops;
} // one_net_set_hops //


SInt8 one_net_set_max_hops(const on_raw_did_t* const raw_did, UInt8 max_hops)
{
    on_encoded_did_t enc_did;
    on_sending_device_t* device;

    
    if(!raw_did || on_encode(enc_did, *raw_did, ON_ENCODED_DID_LEN) !=
      ONS_SUCCESS)
    {
        return -1;
    }
    
    device = (*get_sender_info)(&enc_did);
    if(device == NULL)
    {
        return -1;
    }
    
    if(!features_known(device->features))
    {
        return -1;
    }
    
    if(max_hops > features_max_hops(device->features))
    {
        max_hops = features_max_hops(device->features);
    }
    
    if(device->hops > max_hops)
    {
        device->hops = max_hops;
    }
    
    device->max_hops = max_hops;
    return (SInt8) max_hops;
} // one_net_set_max_hops //
#endif


// TODO -- document
static on_message_status_t rx_single_resp_pkt(on_txn_t** const txn,
  on_txn_t** const this_txn, on_pkt_t* const pkt,
  UInt8* const raw_payload_bytes, on_ack_nack_t* const ack_nack)
{
    on_message_status_t msg_status;
    BOOL ack_rcvd = packet_is_ack(pkt->raw_pid);
    UInt8 txn_nonce = get_payload_txn_nonce(raw_payload_bytes);
    UInt8 resp_nonce = get_payload_resp_nonce(raw_payload_bytes);
    BOOL verify_needed;
    BOOL message_ignore = TRUE;
    const tick_t VERIFY_TIMEOUT = MS_TO_TICK(2000); // 2 seconds
    tick_t time_now = get_tick_count();
    
    if(on_parse_response_pkt(pkt->raw_pid, raw_payload_bytes, ack_nack) !=
      ONS_SUCCESS)
    {
        return ON_MSG_IGNORE; // undecipherable
    }
    
    // Replay attacks are FAR more worriesome on the receiving side.
    // We are the sender, but both sides should always check

    // first check the nonces and the message ID
    // against the sending device info.  We may or may not need to
    // verify anything.
    verify_needed = TRUE;

    if((*txn)->device->verify_time != 0 && time_now >=
      (*txn)->device->verify_time && time_now - (*txn)->device->verify_time
      < VERIFY_TIMEOUT)
    {
        verify_needed = FALSE;
    }
    else
    {
        // regardless of whether we may have verified in the past, we'll
        // want to re-sync / re-verify after getting this problem message.
        (*txn)->device->verify_time = 0;
    }

    #ifdef _ONE_NET_CLIENT
    features_override = FALSE; // if this flag's been set, clear it.  If we
                              // need to reset it, we'll do that below.
    
    // first check to make sure we have the other device's features and they
    // have ours
    if(ack_nack->nack_reason == ON_NACK_RSN_NEED_FEATURES)
    {
        // they need our features and have given us theirs.
        (*txn)->device->features = ack_nack->payload->features;
        features_override = TRUE; // we have theirs, but they need ours.
        return ON_MSG_CONTINUE;
    }
    else if(!features_known((*txn)->device->features))
    {
        // we need their features.  They may or may not have given them
        // to us.
        if(ack_nack->nack_reason == ON_NACK_RSN_FEATURES)
        {
            // they gave it to us.
            (*txn)->device->features = ack_nack->payload->features;
            #ifdef _ONE_NET_MULTI_HOP
            (*txn)->device->max_hops =
              features_max_hops((*txn)->device->features);
            #endif
            // OK, we both have each otehrs' features.  Next time we'll
            // send the real message.
            return ON_MSG_CONTINUE;
        }
        else
        {
            // can't do anything without the features and they didn't give
            // it to us, so ignore.  We need their features, so set the
            // features_override flag again.
            features_override = TRUE;
            return ON_MSG_CONTINUE;
        }
    }
    #endif


    // Now check the message IDs.  We should be getting a response to the one
    // we sent.  If we don't get a response to the one we sent, if we
    // have gotten back a NACK saying that the receiver did not like OUR
    // message ID and giving us one to use, we'll use it.  Otherwise, we'll
    // just call it a bad packet and otherwise ignore it.
    
    // TODO -- The protocol for avoiding replay attacks with message ids was
    // originally to always INCREASE the message id.  If we were given a lower
    // message id than we expected, we should not accept it.  This needs to be
    // revisited.  
    
    // The message_ignore flag is set to true.  If the message ID matches
    // what we gave the other device, we may set it false.  If they didn't
    // like our message ID and gave us one to use back, we'll use it if it's
    // not "less than" the one we gave them.  IF our current message ID is
    // within 2 of the maximum, we'll accept a 0 or 1 as well.
    if(pkt->msg_id == (*txn)->device->msg_id)
    {
        if(ack_nack->nack_reason == ON_NACK_RSN_INVALID_MSG_ID)
        {
            if(ack_nack->payload->nack_value >= (*txn)->device->msg_id ||
              ((*txn)->device->msg_id + 2 >= ON_MAX_MSG_ID &&
              ack_nack->payload->nack_value < 2))
            {
                // they gave us back a valid message id to use instead of the
                // one we were using.
                (*txn)->device->msg_id = ack_nack->payload->nack_value;
                pkt->msg_id = ack_nack->payload->nack_value;
                (*txn)->device->verify_time = 0;
                return ON_MSG_CONTINUE;
            }
        }
        else
        {
            message_ignore = FALSE;
        }
    }

    if(message_ignore)
    {
        return ON_MSG_IGNORE; // replay attack?  Out of sync?  Who knows?
                              // Ignore it.
    }
    
    // regardless of whether the other side liked our nonces or we like
    // theirs, if we made it this far we'll take the one they gave us to
    // give back to them as OK
    (*txn)->device->send_nonce = resp_nonce;
    
    if(verify_needed)
    {
        // we'll verify if the transaction nonce matches either the
        // expected nonce or the last nonce.
        if(txn_nonce == (*txn)->device->expected_nonce ||
          txn_nonce == (*txn)->device->last_nonce)
        {
            // If they liked OUR nonces and the message 
            (*txn)->device->verify_time = time_now;
        }
        else
        {
            // they gave us a bad nonce.  We accepted theirs.  We won't
            // change ours unless it's invalid and we'll hope that we'll
            // sync up the next time we send.  Set the NACK reason to a
            // bad nonce.
            ack_nack->nack_reason = ON_NACK_RSN_NONCE_ERR;
        }
    }
    

	// now we'll give the application code a chance to do whatever it wants to do
	// with the ACK/NACK, including handle it itself.  If it wants to handle everything
	// itself and cancel this transaction, it should return false.  It may decide
    // it wants to abort, in which case it should return ON_MSG_ABORT.  If
    // there was a problem with the message ID or the nonces, we're simply
    // calling the application code to notify it.  It should NOT change
    // anything, but it may want to be aware of failures for its own needs.
    msg_status = (*pkt_hdlr.single_ack_nack_hdlr)(&single_txn,
      &data_pkt_ptrs, single_msg_ptr->payload,
      &(single_msg_ptr->msg_type), ack_nack);
    
    if(msg_status == ON_MSG_ABORT)
    {
        return msg_status;
    }
    
    
    // if we have a nack reason of anything but a bad nonce or a bad message
    // id AND a verification was needed, we'll change nonces.  We will also
    // pick a new nonce if the expected nonce is illegal.  Otherwise the other
    // device can never match it.
    if((*txn)->device->expected_nonce > ON_MAX_NONCE || (verify_needed &&
      (ack_nack->nack_reason != ON_NACK_RSN_NONCE_ERR &&
      ack_nack->nack_reason != ON_NACK_RSN_INVALID_MSG_ID)))
    {
        // pick a new nonce
        (*txn)->device->last_nonce = (*txn)->device->expected_nonce;
        (*txn)->device->expected_nonce = one_net_prand(time_now,
          ON_MAX_NONCE);
    }
    
    #if defined(_ONE_NET_CLIENT) && defined(_DEVICE_SLEEPS)
    if(device_is_master && packet_is_stay_awake(*(pkt->pid)))
    {
        // we received a stay-awake, so set the stay-awake timer
        // for 3 seconds
        ont_set_timer(STAY_AWAKE_TIMER, MS_TO_TICK(3000));
    }
    #endif

    // the application code may or may not have changed this from an ACK to a
    // NACK or vice versa, changed the number of retries, added a delay,
    // changed the response timeout, or whateer else.  We don't particularly
    // care here.
    if(ack_nack->nack_reason != ON_NACK_RSN_NO_ERROR)
    {
        ((*txn)->retry)++;
    }
    else
    {
        *this_txn = 0;
        return ON_MSG_SUCCESS;
    }

    return ON_MSG_CONTINUE;
}


/*!
    \brief Finishes reception of a single data pkt

    \param[in/out] txn The single transaction
    \param[in] sing_pkt_ptr A pointer to elemens of the parsed packet
    \param[in] raw_payload The decoded, decrypted payload
    \param[out] ack_nack The ACK or NACK reason, handle, and payload

    \return ON_MSG_CONTINUE If the packet should be processed further
            ON_MSG_RESPONSE If the packet does not need further processing
                            and should be responded to.
            ON_MSG_IGNORE If the packet should be not be ignored
            See on_message_status_t & single_data_hdlr for more options.
*/
on_message_status_t rx_single_data(on_txn_t** txn, on_pkt_t* sing_pkt_ptr,
  UInt8* raw_payload, on_ack_nack_t* ack_nack)
{
    BOOL ack_msg = FALSE;
    UInt8 msg_type, resp_pid;
    UInt8 txn_nonce = get_payload_txn_nonce(raw_payload_bytes);
    UInt8 resp_nonce = get_payload_resp_nonce(raw_payload_bytes);
    BOOL src_features_known;
    BOOL verify_needed = FALSE;
    BOOL message_id_match = FALSE;
    BOOL message_ignore = FALSE;
    tick_t time_now = get_tick_count();
    on_message_status_t msg_status = ON_MSG_CONTINUE;
    
    if(!txn || !(*txn) || !raw_payload || !ack_nack)
    {
        return ONS_BAD_PARAM;
    }

    ack_nack->payload = (ack_nack_payload_t*) &raw_payload[ON_PLD_DATA_IDX];
    
    
    // we'll do a little prep work here before actually sending the message
    // to the single data handler.  Anything we can handle here where the
    // application code does not need to be alerted will be handled here.
    // We'll set some flags and nack reasons that will signal follow-up
    // functions that we've already processed the message.
    
    // Possibilities include...
    //
    // 1) Making sure that both ends have each others' features.
    // 2) Verifying message ids and nonces.
    // 3) Figuring out what messages can definitely be aborted.
    
    
    // Message ids and nonces serve three purposes...
    // 1) Protecting against replay attacks.
    // 2) Making sure devices are in sync.
    // 3) Making sure that single messages which have been completed are
    //    not acted upon again.  We sent and earlier ACK that must have been
    //    lost.  We will ACK again.
    //
    //    Note : The protocol is not complete on this.  Simply ACKing won't
    //           do the job because very often the message will require the
    //           current status and only the application code can provide
    //           that.  Some messages should never be acted upon twice
    //           (i.e. changing a key fragment) and some don't matter.  For
    //           now if we've ACK'd before, we will skip the verification.
    //           Acting upon a message twice, however, opens up the
    //           possibility of replay attacks, so this needs to be worked
    //           out much more thoroughly in the protocol.  Ideally we would
    //           like to ACKNOWLEDGE a valid message that has already been
    //           ACK'd, but not ACT UPON it, particularly if it is a command
    //           to turn a relay on or off or a command to set some other
    //           setting to a certain setting.
    //
    //           For now we will risk the replay attack in order to reduce
    //           lost messages, but we will require the nonce to be correct.
    //           Upon verification, we will CHANGE the nonce ONCE, then not
    //           again.  Verification will only occur if the message matches
    //           the EXPECTED nonce, not the expected or the last nonce, again
    //           to help prevent replay attacks.
    //                   
    
    

    (*txn)->device = (*get_sender_info)
      ((on_encoded_did_t*) &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]));

    if((*txn)->device == NULL)
    {
        // how the heck did this happen?
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    {
        (*txn)->device->hops = sing_pkt_ptr->hops;
        (*txn)->hops = (*txn)->device->hops;
        
        // assume it will take as many hops to get back as it took to get
        // there.  Application code will change if it likes.
        (*txn)->max_hops = (*txn)->device->hops;
    }
    #endif    
    

    src_features_known = features_known((*txn)->device->features);
    msg_type = get_payload_msg_type(raw_payload);
    
    // If the source features are not known and it is NOT an admin message
    // telling us what those features are, we'll NACK the message.  We'll
    // also NACK the message if the message type is ON_FEATURE_MSG.  This
    // generally means that the device wants to send something to us, but
    // doesn't have features.  It is giving us its features and wants ours
    // in return.  It also wants to be NACK'd, not ACK'd for easier handling
    // the other end.  If it wanted an ACK, it would have sent a features
    // request admin message.
    
    
    // first we'll check the features and set the handle and payload as
    // such.  If this level passes, we'll reset them to something else.
    ack_nack->handle = ON_NACK_FEATURES;
    

    if(msg_type == ON_FEATURE_MSG)
    {
        one_net_memmove(&((*txn)->device->features),
          &raw_payload[ON_PLD_DATA_IDX], sizeof(on_features_t));
        ack_nack->payload->features = THIS_DEVICE_FEATURES;
        // we'll NACK it and give the nack reason as ON_NACK_RSN_FEATURES,
        // but the NACK handle will be ON_NACK_FEATURES.  We are making the
        // nack reason ON_NACK_RSN_FEATURES rather than
        // ON_NACK_RSN_NEED_FEATURES because if we made the nack reason that,
        // the sending device would think that the problem was that WE didn't
        // have ITS features when in fact the opposite is true.
        ack_nack->nack_reason = ON_NACK_RSN_FEATURES;
        (*txn)->device->verify_time = 0;
        return ON_MSG_CONTINUE;
    }
    
    if(!src_features_known)
    {
        // this may be an admin message where this device is giving us its
        // features, so check and copy them if it is.  NACK regardless just in
        // case that device needs OUT features.
        // TODO -- does this make sense to NACK?  Seems a bit strange and
        // possibly redundant given that we have a message type called
        // ON_FEATURE_MSG.
        if(msg_type == ON_ADMIN_MSG && raw_payload[ON_PLD_ADMIN_TYPE_IDX] ==
          ON_PLD_ADMIN_TYPE_IDX)
        {
            one_net_memmove(&((*txn)->device->features),
              &raw_payload[ON_PLD_ADMIN_DATA_IDX], sizeof(on_features_t));
        }
        
        // this time WE need the other device's features, so we'll set
        // the nack reason to ON_NACK_FEATURES.
        ack_nack->nack_reason = ON_NACK_RSN_NEED_FEATURES;
        ack_nack->payload->features = THIS_DEVICE_FEATURES;
        (*txn)->device->verify_time = 0;
        return ON_MSG_CONTINUE;
    }
    
    
    // we've gotten this far, so we have each other's features.  Now we
    // need to check the message id.  If THAT test passes, we'll process
    // the nonces
    
    // If the message ID is the same as the one we have on file, we'll
    // assume this is a repeat packet.  If we have not ACK'd it yet,
    // everything is good.  If we HAVE ACK'd it, we'll have a verify time.
    // If that verify time is close to the current time, we'll assume that
    // the other side simply missed our ACK and count it as good.  If the
    // verify time is NOT close to the current time, we'll assume this is
    // possibly a replay attack or that something is out of sync.  We will
    // NACK it and send back the NACK along with a message id that we WILL
    // accept.
    
    // If the message is LESS THAN the message we have on file, there is
    // possibly a rollover.  We only have six bits after all.  If the one we
    // have on file is the maximum message id and the one we got is 0, we'll
    // accept that as a new message.  Otherwise, we'll NACK it and tell the
    // sender to use a message id of one greater than what we have on file.
    
    // If the message ID is GREATER than the one we have on file, we will
    // assume that this is a new message and accept it.
    {
        tick_t time_now = get_tick_count();
        const tick_t VERIFY_TIMEOUT = MS_TO_TICK(2000); // 2 seconds         
        UInt8 one_greater = ((*txn)->device->msg_id >= ON_MAX_MSG_ID ?
          0 : 1 + (*txn)->device->msg_id);
          
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_MSG_ID;


        if(sing_pkt_ptr->msg_id > (*txn)->device->msg_id ||
          sing_pkt_ptr->msg_id == one_greater)
        {
            // valid new message id for a new message we haven't seen before.
            (*txn)->device->msg_id = sing_pkt_ptr->msg_id;
            (*txn)->device->verify_time = 0;
            ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
        }
        else if(sing_pkt_ptr->msg_id == (*txn)->device->msg_id)
        {
            if((*txn)->device->verify_time == 0)
            {
                ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
            }
            else if(time_now >= (*txn)->device->verify_time &&
              time_now - (*txn)->device->verify_time < VERIFY_TIMEOUT)
            {
                // verified and ACK'd recently.  The ACK may have been missed.
                ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
            }
        }

        if(ack_nack->nack_reason == ON_NACK_RSN_INVALID_MSG_ID)
        {
            ack_nack->handle = ON_NACK_VALUE;
            // this is a message id we will accept. 
            ack_nack->payload->nack_value = one_greater;
            (*txn)->device->verify_time = 0; 
            return ON_MSG_CONTINUE;        
        }
    }
    
    
    ack_nack->handle = ON_ACK;
    
    // The message ID check passed.  Now let's check the nonces.  First, we're
    // far enough along in the process that we will accept the nonce that was
    // given to us regardles of whether the other device gave US the right
    // nonce.
    (*txn)->device->send_nonce = resp_nonce;
    
    // now let's see if the other device gave US the correct nonce.
    if((*txn)->device->expected_nonce != txn_nonce)
    {
        // Make sure that the expected nonce is a legal nonce.  If not, change
        // it to a legal one or we'll never get a matching nonce.
        if((*txn)->device->expected_nonce > ON_MAX_NONCE)
        {
            (*txn)->device->expected_nonce = one_net_prand(get_tick_count(),
              ON_MAX_NONCE);
        }
        
        // they gave us the wrong nonce.
        ack_nack->nack_reason = ON_NACK_RSN_NONCE_ERR;
    }

    return ON_MSG_CONTINUE;
} // rx_single_data //


#ifdef _BLOCK_MESSAGES_ENABLED
one_net_status_t rx_block_data(on_txn_t** txn, UInt8* raw_payload,
  UInt8 txn_nonce, UInt8 resp_nonce)
{
    return ONS_SUCCESS;
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t rx_stream_data(on_txn_t** txn, UInt8* raw_payload,
  UInt8 txn_nonce, UInt8 resp_nonce)
{
    return ONS_SUCCESS;
}
#endif


/*!
    \brief Receives packets.

    This function receives all packets.

    \param[in] EXPECTED_SRC_DID The encoded DID of the device that this device
      expects to receive a packet from.  If this is set to the broadcast
      address, this device does not expect a packet from anyone in particular.
    \param[in] The current transaction being carried out.
    \param[out] this_txn The packet received
    \param[out] this_pkt_ptrs Filled in pointers to the packet received
    \param[out] raw_payload_bytes The decoded / decrypted bytes of this packet

    \return ONS_PKT_RCVD if a valid packet was received
            For more return values one_net_status_codes.h
*/
one_net_status_t on_rx_packet(const on_encoded_did_t * const EXPECTED_SRC_DID,
  const on_txn_t* const txn, on_txn_t** this_txn, on_pkt_t** this_pkt_ptrs,
  UInt8* raw_payload_bytes)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    one_net_xtea_key_t* key = NULL;
    UInt8 raw_pid, enc_pid; // TODO -- make this one variable?  This can be
                            // easily done, but it's a little confusing to
                            // read the code, so I'll keep them as two
                            // separate variables for now.
    BOOL dst_is_broadcast, dst_is_me, src_match;
    #ifdef _ONE_NET_MULTI_HOP
    BOOL packet_is_mh;
    UInt8 raw_hops_field;
    #endif
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    BOOL repeat_this_packet = FALSE;
    #endif
    on_data_t type;
    UInt8* pkt_bytes;
    #ifdef _ONE_NET_MASTER
    UInt8 original_payload[33];
    #endif


    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!EXPECTED_SRC_DID || !this_txn || !this_pkt_ptrs)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    
    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) != ONS_SUCCESS)
    {
        return ONS_READ_ERR;
    }
    
    pkt_bytes = (*this_txn)->pkt;
    
    if(one_net_read(&pkt_bytes[ONE_NET_PREAMBLE_HEADER_LEN],
      ON_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN) !=
      ON_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN)
    {
        return ONS_READ_ERR;
    }
    
    // check some addresses
    #ifndef _ONE_NET_CLIENT
    if(!is_my_nid((on_encoded_nid_t*) (&pkt_bytes[ON_ENCODED_NID_IDX])))
    {
        return ONS_NID_FAILED; // not our network
    }
    #else
    if(*this_txn != &invite_txn && !is_my_nid((on_encoded_nid_t*)
      (&pkt_bytes[ON_ENCODED_NID_IDX])))
    {
        return ONS_NID_FAILED; // not our network
    }
    #endif    
    
    #ifdef _ONE_NET_MULTI_HOP
    // first check the source.  If it was us originally, then we probably
    // got back our own repeated packet.
    if(is_my_did((on_encoded_did_t*) &pkt_bytes[ON_ENCODED_SRC_DID_IDX]))
    {
        return ONS_DID_FAILED; // We SENT this packet, so no sense RECEIVING
                               // it.
    }
    #endif

    #ifdef _RANGE_TESTING
    if(!device_in_range((on_encoded_did_t*)
      &(pkt_bytes[ONE_NET_ENCODED_RPTR_DID_IDX])))
    {
        // we'll pretend that this device was out of range and we couldn't
        // read it.
        return ONS_READ_ERR;
    }
    #endif
    
    enc_pid = pkt_bytes[ONE_NET_ENCODED_PID_IDX];
    raw_pid = encoded_to_decoded_byte(enc_pid, FALSE);
    if(raw_pid >= 0x40)
    {
        return ONS_BAD_PKT_TYPE;
    }

    dst_is_broadcast = is_broadcast_did((on_encoded_did_t*)
      (&pkt_bytes[ONE_NET_ENCODED_DST_DID_IDX]));
    dst_is_me = is_my_did((on_encoded_did_t*)
      (&pkt_bytes[ONE_NET_ENCODED_DST_DID_IDX]));
    src_match = is_broadcast_did(EXPECTED_SRC_DID) ||
      on_encoded_did_equal(EXPECTED_SRC_DID,
      (on_encoded_did_t*) &pkt_bytes[ON_ENCODED_SRC_DID_IDX]);
    
    #ifdef _ONE_NET_MULTI_HOP
    packet_is_mh = packet_is_multihop(raw_pid);
    #endif
    
    #if !defined(_ONE_NET_MH_CLIENT_REPEATER) || !defined(_ONE_NET_CLIENT)
    if(!src_match || (!dst_is_me && !dst_is_broadcast))
    {
        return ONS_BAD_ADDR;
    }
    #else
    if(!src_match || (!dst_is_me && !dst_is_broadcast) || (enc_pid ==
      ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT && client_joined_network))
    {
        // not to us, but maybe we'll repeat it if we're not the master,
        // not in the middle of our own transaction, it's a multi-hop
        // packet, and it wasn't to us.
        if(dst_is_me || device_is_master || txn || !packet_is_mh)
        {
            return ONS_BAD_PARAM;
        }
        
        // we'll repeat it if there are any hops left.
        repeat_this_packet = TRUE;
    }
    #endif
    
    if(packet_is_data(raw_pid))
    {
        if(packet_is_single(raw_pid))
        {
            type = ON_SINGLE;
        }
        #ifdef _BLOCK_MESSAGES_ENABLED
        else if(packet_is_block(raw_pid))
        {
            type = ON_BLOCK;
        }
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        else if(packet_is_stream(raw_pid))
        {
            type = ON_STREAM;
        }
        #endif
    }
    else if(packet_is_ack(raw_pid) || packet_is_nack(raw_pid))
    {
        type = ON_RESPONSE;
    }
    #ifdef _ONE_NET_CLIENT
    else if(packet_is_invite(raw_pid))
    {
        type = ON_INVITE;
    }
    #endif
    else
    {
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if(!repeat_this_packet)
        #endif
        {
            return ONS_BAD_PKT_TYPE;
        }
    }

    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    if(repeat_this_packet)
    {
        one_net_memmove(&(mh_txn.pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
          &((*this_txn)->pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
          ON_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN);
        *this_txn = &mh_txn;
    }
    else
    #endif
    {
        // not repeating.  Let's verify the packets we are receiving
        // are valid for our current state.
        switch(type)
        {
            case ON_INVITE:
              #ifndef _ONE_NET_CLIENT
              return ONS_UNHANDLED_PKT;;
              #else
              if(*this_txn != &invite_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              #endif
              break;
            case ON_SINGLE:
              if(*this_txn != &single_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              break;
            #ifdef _BLOCK_MESSAGES_ENABLED
            case ON_BLOCK:
              if(*this_txn != &block_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              break;
            #endif
            #ifdef _STREAM_MESSAGES_ENABLED
            case ON_STREAM:
              if(*this_txn != &stream_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              break;
            #endif
            case ON_RESPONSE:
              if(*this_txn != &response_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
        }
    }

    if(!setup_pkt_ptr(raw_pid, (*this_txn)->pkt, *this_pkt_ptrs))
    {
        return ONS_INTERNAL_ERR;
    }

    if(one_net_read(&((*this_txn)->pkt[ON_PLD_IDX]), (*this_pkt_ptrs)->payload_len)
      != (*this_pkt_ptrs)->payload_len)
    {
        return ONS_READ_ERR;
    }
    
    #ifndef _ONE_NET_MH_CLIENT_REPEATER
    if(!verify_msg_crc(*this_pkt_ptrs))
    #else
    // don't bother verifying if we are just going to repeat.
    if(!repeat_this_packet && !verify_msg_crc(*this_pkt_ptrs))
    #endif
    {
        return ONS_CRC_FAIL;
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    // overwritten below if multi-hop
    (*this_pkt_ptrs)->hops = 0;
    (*this_pkt_ptrs)->max_hops = 0;
    if(packet_is_mh)
    {
        UInt8 raw_hops_field;

        if(one_net_read((*this_pkt_ptrs)->enc_hops_field, ON_ENCODED_HOPS_SIZE)
          != ON_ENCODED_HOPS_SIZE)
        {
            return ONS_READ_ERR;
        }

        if(on_decode(&raw_hops_field, (*this_pkt_ptrs)->enc_hops_field,
          ON_ENCODED_HOPS_SIZE) != ONS_SUCCESS)
        {
            return ONS_BAD_ENCODING;
        }
        
        // TODO -- use defined masks and shifts
        raw_hops_field >>= 2;
        (*this_pkt_ptrs)->max_hops = raw_hops_field & 0x07;
        (*this_pkt_ptrs)->hops = (raw_hops_field >> 3);
        
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if(repeat_this_packet)
        {
            if((*this_pkt_ptrs)->hops >= (*this_pkt_ptrs)->max_hops)
            {
                // too many hops.  Don't repeat.
                return ONS_UNHANDLED_PKT;
            }
            
            // we have everything.  Increment the hops, add ourself as the
            // repeater, pause a very short time, and send the message.
            ((*this_pkt_ptrs)->hops)++;
            
            raw_hops_field = ((((*this_pkt_ptrs)->hops) << 3) +
              (*this_pkt_ptrs)->max_hops) << 2;
            on_encode((*this_pkt_ptrs)->enc_hops_field, &raw_hops_field,
              ON_ENCODED_HOPS_SIZE);
            one_net_memmove((*this_pkt_ptrs)->enc_repeater_did,
              &(on_base_param->sid[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
            ont_set_timer(mh_txn.next_txn_timer, MS_TO_TICK(
              ONE_NET_MH_LATENCY));
              
            return ONS_PKT_RCVD;
        }
        #endif
    }
    #endif
    
    key = &(on_base_param->current_key);
    
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master && *this_txn == &invite_txn)
    {
        key = (one_net_xtea_key_t*) one_net_client_get_invite_key();
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        #ifdef _STREAM_MESSAGES_ENABLED
        key = master_get_encryption_key(type, (*this_pkt_ptrs)->enc_src_did);
        #else
        key = master_get_encryption_key((*this_pkt_ptrs)->enc_src_did);
        #endif
    }
    #endif
    
    if(key == NULL)
    {
        return ONS_DID_FAILED;
    }

    if((status = on_decode(raw_payload_bytes, (*this_pkt_ptrs)->payload,
      (*this_pkt_ptrs)->payload_len)) != ONS_SUCCESS)
    {
        return status;
    }
    
    #ifdef _ONE_NET_MASTER
    // copy in case this is a client in the middle of a key change
    one_net_memmove(original_payload, raw_payload_bytes,
      get_raw_payload_len(raw_pid));
      
master_decrypt_packet:
    #endif

    if((status = on_decrypt(type, raw_payload_bytes, key,
      get_raw_payload_len(raw_pid))) != ONS_SUCCESS)
    {   
        return status;
    }

    // check the payload CRC.
    if(!verify_payload_crc(raw_pid, raw_payload_bytes))
    {
        #ifdef _ONE_NET_MASTER
        if(device_is_master && (type == ON_SINGLE || type == ON_RESPONSE) &&
          master_try_alternate_key_change_key((*this_pkt_ptrs)->enc_src_did))
        {
            // the device may be in the middle of a key change so it may be
            // worthwhile to check both the new and the old key.
            if(key != &(on_base_param->current_key))
            {
                key = &(on_base_param->current_key);
                one_net_memmove(raw_payload_bytes, original_payload,
                  get_raw_payload_len(raw_pid));
                goto master_decrypt_packet; // try again with another key
            }
        }
        #endif
        return ONS_CRC_FAIL;
    }
    
    // decode the message id and fill it in.
    if(on_decode(&((*this_pkt_ptrs)->msg_id), (*this_pkt_ptrs)->enc_msg_id,
      ONE_NET_ENCODED_MSG_ID_LEN) != ONS_SUCCESS)
    {
        return ONS_BAD_ENCODING;
    }

    (*this_pkt_ptrs)->msg_id >>= 2;

    // set the key
    (*this_txn)->key = key;

    return ONS_PKT_RCVD;
} // on_rx_packet //


#ifdef _RANGE_TESTING
/*!
    \brief Turns range testing either on or off.

    \param on If false, range testing should be turned off. If true, range
              testing should be turned on.
    \return void
*/
void enable_range_testing(BOOL on)
{
    range_testing_on = on;
}


/*!
    \brief Returns an array of the encoded dids which are within range

    \param[out] enc_dids Array of encoded dids to store the in-range devices
    \param[in/out] num_in_range in --> size of array passed.  out --> number
                   of devices within range.
    
    \return TRUE if no errors and devices could be retrieved.
            FALSE is bad paraemter, device is not in a network, or not enough
                  room exists in the passed array.
*/
BOOL devices_within_range(on_encoded_did_t* enc_dids, UInt8* num_in_range)
{
    UInt8 i = 0;
    UInt8 arr_size;

    
    if(!enc_dids || !num_in_range)
    {
        return FALSE; // bad parameter.
    }

    arr_size = *num_in_range;
    *num_in_range = 0;
    
    while(i < RANGE_TESTING_ARRAY_SIZE &&
      one_net_memcmp(range_test_did_array[i], ON_ENCODED_BROADCAST_DID,
      ON_ENCODED_DID_LEN) != 0)
    {
        i++;
        if(i >= arr_size)
        {
            return FALSE; // no room.
        }
    }
    
    *num_in_range = i;
    one_net_memmove(*enc_dids, range_test_did_array, ((*num_in_range) *
      sizeof(on_encoded_did_t)));
    return TRUE;
}


/*!
    \brief Places a device either in range or out of range.

    \param did encoded did of the device
    \param add if true, make this device in range.  If false, then the device
               is out of range.
    \return TRUE If the list was changed
            FALSE If the list was not changed.
*/
BOOL adjust_range_test_did_array(on_encoded_did_t* const did, BOOL add)
{
    SInt8 i;
    SInt8 index = -1;
    SInt8 empty_index = -1;
    
    for(i = RANGE_TESTING_ARRAY_SIZE - 1; i >= 0 ; i--)
    {
        if(one_net_memcmp(range_test_did_array[i], ON_ENCODED_BROADCAST_DID,
          ON_ENCODED_DID_LEN) == 0)
        {
            empty_index = i;
        }
        else if(one_net_memcmp(range_test_did_array[i], *did,
          ON_ENCODED_DID_LEN) == 0)
        {
            index = i;
        }
    }
    
    if(!add && index == -1)
    {
        return TRUE; // nothing to do
    }
    else if(add && index != -1)
    {
        return TRUE; // nothing to do.
    }
    else if(add && empty_index == -1)
    {
        return FALSE; // no room.
    }
    else if(add)
    {
        one_net_memmove(range_test_did_array[empty_index], *did,
          ON_ENCODED_DID_LEN);
        return TRUE; // added
    }
    else
    {
        one_net_memmove(range_test_did_array[index],
          ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
        return TRUE; // added        
    }
}


/*!
    \brief Resets the range testing array to make everything out of range

    \return void
*/
void reset_range_test_did_array(void)
{
    one_net_memset_block(&range_test_did_array[0], ON_ENCODED_DID_LEN,
      RANGE_TESTING_ARRAY_SIZE, ON_ENCODED_BROADCAST_DID);
}


/*!
    \brief Checks to see whether a device is within range of this device

    \param did encoded did of the device

    \return TRUE If the device is in range
            FALSE If the device not in range
*/
BOOL device_in_range(on_encoded_did_t* did)
{
    SInt8 i;
    
    if(!range_testing_on)
    {
        return TRUE;
    }
    
    for(i = RANGE_TESTING_ARRAY_SIZE - 1; i >= 0 ; i--)
    {
        if(one_net_memcmp(range_test_did_array[i], *did,
          ON_ENCODED_DID_LEN) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}
#endif



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
    if(ont_inactive_or_expired(ONT_CLR_CHANNEL_TIMER))
    {
        ont_set_timer(ONT_CLR_CHANNEL_TIMER,
          MS_TO_TICK(ONE_NET_CLR_CHANNEL_TIME));
        return one_net_channel_is_clear();
    } // if it is time to check the channel //

    return FALSE;
} // check_for_clr_channel //



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET
