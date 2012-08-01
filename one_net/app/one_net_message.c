//! \addtogroup ONE-NET_MESSAGE
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
    \file one_net_acknowledge.c
    \brief Implementation of message functions, including the single essage queue

    Implementation of message functions, including the single essage queue.
*/


#include "config_options.h"
#include "one_net_types.h"
#include "one_net_port_const.h"
#include "one_net_message.h"
#include "one_net_acknowledge.h"
#include "one_net_port_specific.h"
#include "tick.h"
#include "one_net_prand.h"
#include "one_net_crc.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MESSAGE_const
//! \ingroup ONE-NET_MESSAGE
//! @{

//! Header(Preamble and SOF)
const UInt8 HEADER[] = {0x55, 0x55, 0x55, 0x33};

const on_encoded_did_t NO_DESTINATION = {0xFF, 0xFF};


//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{


enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};


//! @} ONE-NET_MESSAGE_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{

#if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static void delete_expired_queue_elements(void);
#endif

//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_MESSAGE_pri_var
//! \ingroup ONE-NET_MESSAGE
//! @{


#if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
static UInt8 payload_buffer[SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE];
static on_single_data_queue_t single_data_queue[SINGLE_DATA_QUEUE_SIZE];
static UInt16 pld_buffer_tail_idx = 0;
#endif

UInt8 single_data_queue_size = 0;

#ifdef ONE_NET_CLIENT
extern BOOL device_is_master;
extern BOOL client_joined_network;
extern on_master_t * const master;
#endif


//! The list of recipients to send to for THIS message
on_recipient_list_t recipient_send_list;

//! Pointer to the list of recipients to send to for THIS message.  Generally
//! will point either to NULL or recipient_send_list.  However, the user is
//! allowed to provide their own recipient lists to override this list
on_recipient_list_t* recipient_send_list_ptr = NULL;


//! @} ONE-NET_MESSAGE_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MESSAGE_pub_var
//! \ingroup ONE-NET_MESSAGE
//! @{

#ifdef ONE_NET_CLIENT
//! If true and sending a single response, this flag signifies that we
// should instead send our features.
BOOL features_override = FALSE;
#endif

//! @} ONE-NET_MESSAGE_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================




//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MESSAGE_pub_func
//! \ingroup ONE-NET_MESSAGE
//! @{
    
    
    
#ifdef ONE_NET_MULTI_HOP
/*!
    \brief Builds the encoded hops field for the packet.

    \param[out] pkt The packet to be built.
    \param[in] hops The number of hops taken so far.
    \param[in] max_hops maximum number of hops the packet can take.

    \return ONS_SUCCESS If building the hops field was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_build_hops(on_pkt_t* pkt, UInt8 hops, UInt8 max_hops)
{
    UInt8 raw_hops;

    if(!pkt || max_hops > ON_MAX_HOPS_LIMIT || hops > max_hops)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    raw_hops = ((max_hops << ON_MAX_HOPS_BUILD_SHIFT) &
      ON_MAX_HOPS_BUILD_MASK) | ((hops << ON_HOPS_BUILD_SHIFT)
      & ON_HOPS_BUILD_MASK);

    on_encode(&(pkt->packet_bytes[ON_ENCODED_PLD_IDX]) + pkt->payload_len,
      &raw_hops, ON_ENCODED_HOPS_SIZE);

    return ONS_SUCCESS;
} // on_build_hops //


/*!
    \brief Parses the encoded hops field for the packet.

    \param[in] pkt The packet to be parsed
    \param[out] hops The number of hops taken so far.
    \param[out] max_hops maximum number of hops the packet can take.

    \return ONS_SUCCESS If parsing the hops field was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_parse_hops(const on_pkt_t* pkt, UInt8* hops,
  UInt8* max_hops)
{
    UInt8 raw_hops_field;
    one_net_status_t status;

    if(!hops || !max_hops)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if((status = on_decode(&raw_hops_field,
      &(pkt->packet_bytes[ON_ENCODED_PLD_IDX]) + pkt->payload_len,
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
#endif // ifdef ONE_NET_MULTI_HOP //


/*!
    \brief Parses the raw bytes of a response into an on_ack_nack_t structure.

    \param[in] raw_pid The PID of the response
    \param[in] raw_bytes The raw bytes containing the response
    \param[out] ack_nack maximum number of hops the packet can take.

    \return ONS_BAD_PKT_TYPE If parsing was unsuccessful due to a bad PID
            ONS_SUCCESS If the parsing was successful
*/
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
        switch(ack_nack->handle)
        {
	        case ON_ACK_VALUE:
	        case ON_ACK_TIME_MS:
            case ON_ACK_TIMEOUT_MS:
            case ON_ACK_SLOW_DOWN_TIME_MS:
            case ON_ACK_SPEED_UP_TIME_MS:
            case ON_ACK_RESPONSE_TIME_MS:
            case ON_ACK_PAUSE_TIME_MS:
                // reverse the bytes if necessary
                // assign it to nack_value.  Doesn't matter.  They all point
                // to the same place
                ack_nack->payload->nack_value = one_net_byte_stream_to_int32(
                  raw_bytes);
                break;
            default:
                // Payload is treated as an array.  Do nothing.
                break;
        }
    } 
    
    return ONS_SUCCESS;
}


/*!
    \brief Sets the pointers of an on_pkt_t structure.

    \param[in] raw_pid the raw pid of the packet
    \param[in] pkt_bytes The array holding the packet bytes
    \param[in] msg_id The msg id to use, if any.  If 0, this is ignored.
    \param[out] pkt The on_pkt_t structure to fill

    \return TRUE if the on_pkt structure was set up successfully.
            FALSE upon error.
*/
BOOL setup_pkt_ptr(UInt16 raw_pid, UInt8* pkt_bytes, UInt16 msg_id, on_pkt_t* pkt)
{
    SInt8 len;
    if(raw_pid > 0xFFF)
    {
        return FALSE; // bad pid
    }
    
    if(!pkt_bytes || !pkt)
    {
        return FALSE;
    }
    
    if(!packet_length_is_valid(raw_pid))
    {
        SInt16 default_num_blocks = get_default_num_blocks(raw_pid);
        if(default_num_blocks < 0)
        {
            return FALSE;
        }
        
        raw_pid &= 0xFF;
        raw_pid |= (default_num_blocks << 8);
    }
    
    len = get_encoded_payload_len(raw_pid);
    if(len <= 0)
    {
        return FALSE;
    }
    
    
    pkt->packet_bytes     = &pkt_bytes[0];
    pkt->raw_pid          = raw_pid;
    if(msg_id)
    {
        pkt->msg_id = msg_id;
    }
    
    put_raw_pid(&(pkt->packet_bytes[ON_ENCODED_PID_IDX]), raw_pid);
    pkt->payload_len = (UInt8) len;
    
    return TRUE;
}


#ifdef ONE_NET_MULTI_HOP
SInt8 one_net_set_hops(const on_raw_did_t* const raw_did, UInt8 hops)
{
    on_encoded_did_t enc_did;
    on_sending_device_t* device;

    
    if(!raw_did || on_encode(enc_did, *raw_did, ON_ENCODED_DID_LEN) !=
      ONS_SUCCESS)
    {
        return -1;
    }
    
    device = (*get_sender_info)((const on_encoded_did_t* const) &enc_did);
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
    
    device = (*get_sender_info)((const on_encoded_did_t* const) &enc_did);
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

    put_raw_pid(&(pkt_ptrs->packet_bytes[ON_ENCODED_PID_IDX]),
      pkt_ptrs->raw_pid);
    

    // for all we know, ack_nack->payload is located at the same address
    // as the raw_payload_bytes[] array.  We don't want to overwrite without
    // knowing, so we'll copy the payload to a buffer that is going to be
    // overwritten anyway to make sure.
    one_net_memmove(&(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]), ack_nack->payload,
      ack_nack_pld_len);
    
    // now move the pointer.
    ack_nack->payload = (ack_nack_payload_t*)
      &(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]);

    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
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
        switch(ack_nack->handle)
        {
	        case ON_ACK_VALUE:
	        case ON_ACK_TIME_MS:
            case ON_ACK_TIMEOUT_MS:
            case ON_ACK_SLOW_DOWN_TIME_MS:
            case ON_ACK_SPEED_UP_TIME_MS:
            case ON_ACK_RESPONSE_TIME_MS:
            case ON_ACK_PAUSE_TIME_MS:
                // These are the value cases.
                // Adjust to MSB first and stick into the ACK / NACK payload
                one_net_int32_to_byte_stream(ack_nack->payload->ack_value,
                  ack_nack_pld_ptr);
                break;
            default:
                // Treat as an array. Hence no endianness issue.  Simply copy
                // the bytes.
                one_net_memmove(ack_nack_pld_ptr, ack_nack->payload,
                  ack_nack_pld_len);
                break;
        }
    }

    #ifdef ONE_NET_MULTI_HOP
    // change between multi-hop and non-multi-hop depending on whether 
    // txn->max_hops is positive.
    set_multihop_pid(&(pkt_ptrs->raw_pid), txn->max_hops > 0);
    
    put_raw_pid(&(pkt_ptrs->packet_bytes[ON_ENCODED_PID_IDX]),
      pkt_ptrs->raw_pid);
    
    if(txn->max_hops > 0)
    {
        // build hops
        if((status = on_build_hops(pkt_ptrs, 0, txn->max_hops)) != ONS_SUCCESS)
        {
            return status;
        }
        
        // put it back into the packet pointers.
        on_parse_hops(pkt_ptrs, &(pkt_ptrs->hops), &(pkt_ptrs->max_hops));        
    }
    #endif


    // build the packet
    // Use the value stored in the on_pkt_t structure rather than the device
    put_payload_msg_id(pkt_ptrs->msg_id, raw_payload_bytes);
    
    // fill in the ack/nack handle (The 4 LSB of raw data byte 2)
	put_ack_nack_handle(ack_nack->handle, raw_payload_bytes);

    // compute the crc
    raw_payload_bytes[0] = (UInt8)one_net_compute_crc(
      &raw_payload_bytes[ON_PLD_CRC_SIZE], (raw_pld_len - 1) - ON_PLD_CRC_SIZE,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
      
    #ifdef STREAM_MESSAGES_ENABLED  
    if((status = on_encrypt(FALSE, raw_payload_bytes,
      (const one_net_xtea_key_t * const) txn->key, raw_pld_len)) == ONS_SUCCESS)
    #else
    if((status = on_encrypt(raw_payload_bytes,
      (const one_net_xtea_key_t * const) txn->key, raw_pld_len)) == ONS_SUCCESS)
    #endif
    {
        status = on_encode(&(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]),
          raw_payload_bytes, num_words);
    } // if encrypting was successful //

    return status;
}


#ifndef BLOCK_MESSAGES_ENABLED
one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device)
#else
one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device,
  block_stream_msg_t* bs_msg)
#endif
{
    UInt8 status;
    SInt8 raw_pld_len = get_raw_payload_len(pkt_ptrs->raw_pid);
    SInt8 num_words = get_encoded_payload_len(pkt_ptrs->raw_pid);
    #ifdef STREAM_MESSAGES_ENABLED
    BOOL is_stream_txn = (bs_msg && get_bs_transfer_type(bs_msg->flags) ==
      ON_STREAM_TRANSFER);
    #endif
    
    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
    }

    
    #ifdef ONE_NET_MULTI_HOP
    // change between multi-hop and non-multi-hop depending on whether 
    // max_hops is positive.
    #ifdef BLOCK_MESSAGES_ENABLED
    if((pkt_ptrs->raw_pid & 0x3F) >= ONE_NET_RAW_BLOCK_DATA &&
      (pkt_ptrs->raw_pid & 0x3F) < ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT)
    {
        pkt_ptrs->hops= 0;
        pkt_ptrs->max_hops = get_bs_hops(bs_msg->flags);
    }
    #endif
    
    set_multihop_pid(&(pkt_ptrs->raw_pid), pkt_ptrs->max_hops > 0);
    put_raw_pid(&(pkt_ptrs->packet_bytes[ON_ENCODED_PID_IDX]),
      pkt_ptrs->raw_pid);
    
    if(pkt_ptrs->max_hops > 0)
    {
        // build hops
        if((status = on_build_hops(pkt_ptrs, pkt_ptrs->hops,
          pkt_ptrs->max_hops)) != ONS_SUCCESS)
        {
            return status;
        }
    }
    #endif

    // build the packet.  Use the on_pkt_t object message id, not the device
    put_payload_msg_id(pkt_ptrs->msg_id, raw_payload_bytes);
    #ifdef ONE_NET_CLIENT
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
        #ifndef BLOCK_MESSAGES_ENABLED
        put_payload_msg_type(msg_type, raw_payload_bytes);
        one_net_memmove(&raw_payload_bytes[ON_PLD_DATA_IDX], raw_pld,
          (raw_pld_len - 1) - ON_PLD_DATA_IDX);
        #else
        UInt8 pld_idx = bs_msg ? ON_BS_DATA_PLD_IDX : ON_PLD_DATA_IDX;
        if(!bs_msg)
        {
            put_payload_msg_type(msg_type, raw_payload_bytes);
        }
        else
        {
            #ifdef STREAM_MESSAGES_ENABLED
            if(is_stream_txn)
            {
                put_stream_response_needed(bs_msg->response_needed,
                  raw_payload_bytes);
                put_stream_elapsed_time(bs_msg->bs.stream.elapsed_time,
                  raw_payload_bytes);
            }
            else
            #endif
            {
                put_bs_chunk_idx(bs_msg->bs.block.chunk_idx, raw_payload_bytes);
                put_bs_chunk_size(bs_msg->bs.block.chunk_size,
                  raw_payload_bytes);
                put_block_pkt_idx(bs_msg->bs.block.byte_idx, raw_payload_bytes);
            }
        }
        
        one_net_memmove(&raw_payload_bytes[pld_idx], raw_pld,
          (raw_pld_len - 1) - pld_idx);
        #endif
    }
      
    // compute the crc
    raw_payload_bytes[0] = (UInt8)one_net_compute_crc(
      &raw_payload_bytes[ON_PLD_CRC_SIZE], (raw_pld_len - 1) - ON_PLD_CRC_SIZE,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    
    #ifdef STREAM_MESSAGES_ENABLED  
    if((status = on_encrypt(is_stream_txn, raw_payload_bytes,
      (const one_net_xtea_key_t * const) txn->key, raw_pld_len)) == ONS_SUCCESS)
    #else
    if((status = on_encrypt(raw_payload_bytes,
      (const one_net_xtea_key_t * const) txn->key, raw_pld_len)) == ONS_SUCCESS)
    #endif
    {
        status = on_encode(&(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]),
          raw_payload_bytes, num_words);
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
    
    one_net_memmove(&(pkt_ptrs->packet_bytes[ON_ENCODED_NID_IDX]), *nid,
      ON_ENCODED_NID_LEN);
    one_net_memmove(&(pkt_ptrs->packet_bytes[ON_ENCODED_RPTR_DID_IDX]),
      *repeater_did, ON_ENCODED_DID_LEN);
    one_net_memmove(&(pkt_ptrs->packet_bytes[ON_ENCODED_DST_DID_IDX]),
      *dst_did, ON_ENCODED_DID_LEN);
    one_net_memmove(&(pkt_ptrs->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
      *src_did, ON_ENCODED_DID_LEN);
      
    return ONS_SUCCESS;
}


one_net_status_t on_build_my_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did)
{
    const on_encoded_nid_t* nid = (const on_encoded_nid_t*) on_base_param->sid;
    const on_encoded_did_t* repeater_did = (const on_encoded_did_t*)
      (&on_base_param->sid[ON_ENCODED_NID_LEN]);
    if(src_did == NULL)
    {
        // source must be the same as repeater (i.e. we are originating)
        src_did = repeater_did;
    }
    
    return on_build_pkt_addresses(pkt_ptrs, nid, repeater_did, dst_did,
      src_did);
}


one_net_status_t on_complete_pkt_build(on_pkt_t* pkt_ptrs, UInt8 pid)
{
    UInt8 msg_crc, msg_crc_calc_len;
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
    msg_crc_start = &(pkt_ptrs->packet_bytes[ON_ENCODED_MSG_CRC_IDX]) +
      ONE_NET_ENCODED_MSG_CRC_LEN;
    msg_crc_calc_len = (&(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]) +
      pkt_ptrs->payload_len) - msg_crc_start;
    
    #ifdef ONE_NET_MULTI_HOP
    // fill in hops if needed
    if(packet_is_multihop(pid))
    {
        one_net_status_t status;
        if((status = on_build_hops(pkt_ptrs, pkt_ptrs->hops, pkt_ptrs->max_hops)
          != ONS_SUCCESS))
        {
            return status;
        }
    }
    #endif
    
    // preamble and start of frame
    one_net_memmove(pkt_ptrs->packet_bytes, HEADER, sizeof(HEADER));

    // we have everything filled in but the the msg_crc, so we can calculate
    // it now.
    msg_crc = (UInt8) one_net_compute_crc(msg_crc_start,
      msg_crc_calc_len, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    // we are only interested in the 6 most significant bits, so mask them
    msg_crc &= 0xFC; // 11111100 -- six most significant bits.
    // now encode for the message
    on_encode(&(pkt_ptrs->packet_bytes[ON_ENCODED_MSG_CRC_IDX]),
      &msg_crc, ONE_NET_ENCODED_MSG_CRC_LEN);
      
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
    UInt8* msg_crc_start =
      &(pkt_ptrs->packet_bytes[ON_ENCODED_MSG_CRC_IDX]) +
      ONE_NET_ENCODED_MSG_CRC_LEN;
    UInt8 msg_crc_calc_len = (&(pkt_ptrs->packet_bytes[ON_ENCODED_PLD_IDX]) +
      pkt_ptrs->payload_len) - msg_crc_start;
      
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
    
    return (pkt_ptrs->packet_bytes[ON_ENCODED_MSG_CRC_IDX] == enc_crc);
}


/*!
    \brief Verify that the payload CRC is valid

    \param[in] raw_pid The raw pid of the packet
    \param[in] decrypted The decrypted bytes

    \return TRUE if the message CRC is valid, FALSE otherwise
*/
BOOL verify_payload_crc(UInt16 raw_pid, const UInt8* decrypted)
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


#ifndef _R8C_TINY
// This is a helper function not used by ONE-NET. Just added it as a helper function for anyone
// who wants it.  #defining it out to spare about 60 bytes of compiled code.
// Note: Some compilers will automatically discard unused functions, but Renesas does not, so
// adding the #ifndef guard.
/*!
    \brief Calculate the payload CRC is valid

    \param[out] crc_calc The calculated payload 
    \param[in] raw_pid The raw pid of the packet
    \param[in] decrypted The decrypted bytes

    \return TRUE if the message CRC is valid, FALSE otherwise
*/
BOOL calculate_payload_crc(UInt8* crc_calc, UInt16 raw_pid, const UInt8* decrypted)
{
    const UInt8* crc_calc_start;
    UInt8 crc_calc_len;
    SInt8 num_encoded_blocks = get_num_payload_blocks(raw_pid);
    
    if(num_encoded_blocks == 0)
    {
        return TRUE; // no CRC to check.
    }
    else if(num_encoded_blocks < 0)
    {
        return FALSE; // invalid CRC
    }
    
    if(!decrypted || !crc_calc)
    {
        return FALSE; // bad parameter
    }
    
    crc_calc_len = (UInt8) (num_encoded_blocks * ONE_NET_XTEA_BLOCK_SIZE -
      ON_PLD_CRC_SIZE);
    crc_calc_start = &decrypted[ON_PLD_CRC_SIZE];
    *crc_calc = (UInt8) one_net_compute_crc(crc_calc_start, crc_calc_len,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
    
    return TRUE;
}
#endif


/*!
    \brief Encrypt the data passed in.

    data should be formatted such that the first byte is the location where
    the 8-bit crc is going to go, then the next N bytes are the data that is
    being encrypted, and there should be room for 1 extra byte on the end
    for the encryption type.  In short, data should be the format of the
    payload field for the appropriate data type.

    \param[in] is_stream_pkt True if the packet is a stream packet, false otherwise
    \param[in/out] data The data to encrypt
    \param[in] KEY The XTEA key used to encrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
#ifdef STREAM_MESSAGES_ENABLED
one_net_status_t on_encrypt(BOOL is_stream_pkt , UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len)
#else
one_net_status_t on_encrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len)
#endif
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    #ifdef STREAM_MESSAGES_ENABLED
    // get the number of XTEA rounds
    if(!is_stream_pkt)
    {
	#endif
        rounds = ON_XTEA_32_ROUNDS;
        data[payload_len - 1] = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;
	#ifdef STREAM_MESSAGES_ENABLED
    } // if not stream //
        else
        {
            rounds = ON_XTEA_8_ROUNDS;
            data[payload_len - 1] = ONE_NET_STREAM_ENCRYPT_XTEA8;
        } // else stream //
    #endif // if STREAM_MESSAGES_ENABLED is not defined //

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

    \param[in] is_stream_pkt True if the packet is a stream packet, false otherwise
    \param[in/out] data The data to decrypt
    \param[in] key The XTEA key used to decrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
#ifdef STREAM_MESSAGES_ENABLED
one_net_status_t on_decrypt(BOOL is_stream_pkt , UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len)
#else
one_net_status_t on_decrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len)
#endif
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    // get the number of XTEA rounds
	#ifdef STREAM_MESSAGES_ENABLED
    if(!is_stream_pkt)
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
	#ifdef STREAM_MESSAGES_ENABLED
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
    #endif // ifdef STREAM_MESSAGES_ENABLED //
   
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











/*
    \brief Remove all messages from the queue
    
    \return void
*/
void empty_queue(void)
{
    single_data_queue_size = 0;
    single_msg_ptr = NULL;
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    pld_buffer_tail_idx = 0;
    #endif
}


/*!
    \brief Add a single message to the queue.
    
    The message is either sent to the peer list or only to the specific device
    that is passed in.
    
    \param[in] raw_pid The raw pid of the message.
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
	\param[in] send_time_from_now Time to pause before sending.  0 is interpreted as "send immediately"
	\param[in] expire_time_from_now If after this time, don't bother sending.  0 is interpreted as "no expiration"
    
    \return pointer to the queue element if the queue add was successful
            NULL if error or no room in queue.
*/
on_single_data_queue_t* push_queue_element(UInt16 raw_pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t send_time_from_now
  #endif
  #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t expire_time_from_now
  #endif
  )
{
    on_single_data_queue_t* element = NULL;
    
    #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL    
    tick_t time_now = get_tick_count();
    #endif
    
    #ifdef EXTENDED_SINGLE
    // check the pid length to make sure it's long enough
    UInt8 pid_passed_blocks = ((raw_pid & ONE_NET_RAW_PID_SIZE_MASK) >>
      ONE_NET_RAW_PID_SIZE_SHIFT);
    UInt16 pid_data_len_blocks = (data_len + ONA_DATA_INDEX) /
      ONE_NET_XTEA_BLOCK_SIZE;
    if((data_len + ONA_DATA_INDEX) % ONE_NET_XTEA_BLOCK_SIZE)
    {
        pid_data_len_blocks++; // always round up if needed.
    }
    if(pid_passed_blocks < pid_data_len_blocks)
    {
        // set the number of blocks to be long enough
        raw_pid &= ~ONE_NET_RAW_PID_SIZE_MASK;
        raw_pid |= (pid_data_len_blocks << ONE_NET_RAW_PID_SIZE_SHIFT);
    }
    #endif
    
    #ifdef _BLOCK_MESSAGES
    if(bs_msg.transfer_in_progress)
    {
        // certain things can go through, certain things cannot depending
        // on the priorities and other things.  For now, we will use the
        // following criteria...
        // 1. All messages that come in as low priority are rejected
        // 2. If the block / stream transfer is high priority, no application
        //    messages are accepted.
        // 3. If the block / stream transfer is low priority, application
        //    messages are allowed, but are made low priority.
        //
        // TODO -- More control of what can and can't go through and when.
        //
        if(priority < ONE_NET_HIGH_PRIORITY)
        {
            return NULL;
        }
        if(msg_type < ON_ADMIN_MSG) // message is an APP message
        {
            priority = ONE_NET_LOW_PRIORITY;
            if(bs_msg.priority == ONE_NET_HIGH_PRIORITY)
            {
                return NULL;
            }
        }
    }
    #endif
    
    if(!raw_data)
    {
        return NULL; // invalid parameter
    }
    
    if(is_my_did(enc_dst))
    {
        return NULL; // sending to ourself.  Why bother?
    }
    
    #ifdef ONE_NET_MASTER
    if(enc_dst && device_is_master)
    {
        if(!(*get_sender_info)(enc_dst))
        {
            return NULL; // invalid destination
        }
    }
    #endif
    
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    if(single_data_queue_size >= SINGLE_DATA_QUEUE_SIZE)
    {
        return NULL; // no room in queue
    }
    if((data_len + pld_buffer_tail_idx) >
      SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE)
    {
        return NULL; // no room in queue
    }

    element = &single_data_queue[single_data_queue_size];
    #else
    if(single_msg_ptr || single_data_queue_size)
    {
        return NULL;  // no room
    }
    element= &single_msg;
    #endif
    
    element->raw_pid = raw_pid;
    element->priority = priority;
    element->msg_type = msg_type;
    element->payload_size = data_len;
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    element->payload = &payload_buffer[pld_buffer_tail_idx];
    pld_buffer_tail_idx += data_len;
    #else
    element->payload = single_data_raw_pld;
    #endif

    one_net_memmove(element->payload, raw_data, data_len);
    
    if(src_did != NULL)
    {
        one_net_memmove(element->src_did, *src_did, ON_ENCODED_DID_LEN);
    }
    else
    {
        one_net_memmove(element->src_did,
          &(on_base_param->sid[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
    }
    
    if(enc_dst)
    {
        one_net_memmove(element->dst_did, *enc_dst, ON_ENCODED_DID_LEN);
    }
    else
    {
        one_net_memmove(element->dst_did, NO_DESTINATION, ON_ENCODED_DID_LEN);
    }
    
    #ifdef PEER
	element->send_to_peer_list = send_to_peer_list;
    element->src_unit = src_unit;
    #endif
    
    #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
    element->send_time = 0;
    element->send_time = time_now + send_time_from_now;

    #endif
    #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
    element->expire_time = 0;
    if(expire_time_from_now)
    {
	    element->expire_time = time_now + expire_time_from_now;
    }
    #endif
    single_data_queue_size++;
    return element;
}


#if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
// return true if an element was popped, false otherwise.
BOOL pop_queue_element(on_single_data_queue_t* const element,
    UInt8* const buffer, UInt8 index)
#else
BOOL pop_queue_element(void)
#endif
{
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    UInt8 i;

    if(index >= single_data_queue_size)
    {
        // index out of range.
        return FALSE;
    }
    
    // we have a legitimate index.  Pop the element if the caller to the
    // function provided somewhere to store it.
    if(element != NULL && buffer != NULL)
    {
        one_net_memmove(element, &single_data_queue[index],
            sizeof(on_single_data_queue_t));
        element->payload = buffer;
        one_net_memmove(element->payload, single_data_queue[index].payload,
            single_data_queue[index].payload_size);
    }
    
    // now delete the element
    
    // first delete the payload memory it takes in the buffer.
    // TODO - more range checking.  Unnecessaray if the queue is doing what
    // it's supposed to
    if(index < single_data_queue_size - 1)
    {
        // we aren't popping the last element
        UInt16 this_msg_buffer_start_idx = single_data_queue[index].payload -
            &payload_buffer[0];
        UInt16 next_msg_buffer_start_idx = this_msg_buffer_start_idx +
            single_data_queue[index].payload_size;
        UInt16 bytes_to_move = pld_buffer_tail_idx - next_msg_buffer_start_idx;
        one_net_memmove(&payload_buffer[this_msg_buffer_start_idx],
            &payload_buffer[next_msg_buffer_start_idx], bytes_to_move);
            
        // payloads have been moved.  Now adjust the payload pointers
        for(i = index + 1; i < single_data_queue_size; i++)
        {
            single_data_queue[i].payload -= single_data_queue[index].payload_size;
        }
    }
    
    // adjust the tail index.
    pld_buffer_tail_idx -= single_data_queue[index].payload_size;
    
    // payload(s) have been adjusted.  Now move the queue elements themselves
    if(index < single_data_queue_size - 1)
    {
        one_net_memmove(&single_data_queue[index], &single_data_queue[index + 1],
            (single_data_queue_size - index - 1) * sizeof(on_single_data_queue_t));
    }
    
    // finally adjust the queue size
    single_data_queue_size--;
    return TRUE;
    
    #else
    {
        // there's nothing to copy since everything is already loaded if
        // it exists.
        BOOL ret_value = (single_data_queue_size > 0);
        single_data_queue_size = 0;
        return ret_value;
    }
    #endif
}


/*!
    \ brief Checks the the single message queue for messages ready to pop
	
	If there is a single data transaction in the queue and it's ready to
	be placed into a transaction, it will be placed into the single transaction spot
	
	Any unsent packets that have expired will be deleted.
	
	\param[out] next_sleep_time The earliest time the queue might have something to pop.
                If the value is 0 and the function returns -1, then the queue is empty and
                next_pop_time should be disregarded.  If the function returns a non-negative
                value, next_pop_time should be disregarded.
	
	\return index of an element ready to be popped.  If none, -1 is returned.
*/
#if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
int single_data_queue_ready_to_send(tick_t* const next_pop_time)
{
	int i, j;
    tick_t sleep_time;
	UInt8 priority = ONE_NET_HIGH_PRIORITY;
	tick_t cur_tick = get_tick_count();
	*next_pop_time = 0;
        
    #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
    delete_expired_queue_elements();
    #endif
    
    if(single_data_queue_size == 0)
    {
        return -1; // empty queue
    }
	
	// note that send_time equals 0 means send immediately

	
	// Go through the list and see if any packets are ready.  Try high priority, then low priority.
	// Note that priority started initialized as HIGH_PRIORITY.  Go through the list twice, once for
	// each priority.  We'll change priority at the bottom of the outer loop.  If anything is ready
    // to pop, pop it
	for(j = 0; j < 2; j++)
	{
	    for(i = 0; i < single_data_queue_size; i++)
	    {
            if(single_data_queue[i].priority != priority)
            {
                continue; // wrong priority
            }
            
		    if(single_data_queue[i].send_time <= cur_tick)
		    {
                // we're ready to pop this element.
                return i;
		    }
            
            sleep_time = single_data_queue[i].send_time - cur_tick;
            if(*next_pop_time == 0 || sleep_time < *next_pop_time)
            {
                *next_pop_time = sleep_time;
            }
		}
		 
		priority = ONE_NET_LOW_PRIORITY;
	}
	
	return -1; // nothing ready to pop.
}
#else
int single_data_queue_ready_to_send(void)
{
    if(single_data_queue_size == 0)
    {
        return -1;
    }
    
    #if SINGLE_QUEUE_LEVEL == MIN_SINGLE_QUEUE_LEVEL
    if(single_data_queue[0].priority != ONE_NET_HIGH_PRIORITY)
    {
        UInt8 i;
        
        // go throught the list.  If we have a HIGH priority message, return the index
        for(i = 1; i < single_data_queue_size; i++)
        {
            if(single_data_queue[i].priority == ONE_NET_HIGH_PRIORITY)
            {
                return i;
            }
        }
    }
    #endif
    
    return 0;
}
#endif


#ifdef ONE_NET_CLIENT
/*!
    \ brief Determines whether a message must have an added message to the
            master tacked on to the end.
    
    The message will need to have this extra message sent if and only if...
    
    1) Device is a client.
    2) Device has joined the network.
    3) Device has the _SEND_TO_MASTER flag set.
    4) The message is a single message (as opposed to a block or stream)
    5) The message type is an ON_APP_MSG (as opposed to an admin message or
           a user-defined message message type.
    6) The message class is a "status-type" message.
    7) The message is not already being sent to the master as a recipient,
       either because the master is the recipient or the master is on the
       peer list (if relevant).
       
    Note that for devices where PEER is enabled, the second part of
        condition #7 will be checked elsewhere rather than from this function.
	
	\param[in] element An on_single_data_queue_t object containing the message
                       to be sent.

	\return TRUE if an extra _SEND_TO_MASTER message needs to be sent.
*/
BOOL must_send_to_master(const on_single_data_queue_t* const element)
{
    return (!device_is_master && client_joined_network
      && (master->flags & ON_SEND_TO_MASTER)
      && element->msg_type == ON_APP_MSG && 
      ONA_IS_STATUS_MESSAGE(get_msg_class(element->payload)) &&
      !is_master_did(&(element->dst_did)));
}
#endif



/*!
    \ brief Clears the recipient list of all recipients
	
	\param[out] rec_list The recipient list to clear.
*/
void clear_recipient_list(on_recipient_list_t* rec_list)
{
    rec_list->num_recipients = 0;
    rec_list->recipient_index = -1;
}


/*!
    \ brief Compares two did / unit pairs and returns TRUE if they are both
      the same.
	
	\param[in] dev1 the first did / unit pair.
    \param[in] dev2 the second did / unit pair.
    
    \return TRUE if the dev1 and dev2 have the same dids and unit numbers
            FALSE otherwise
*/
BOOL did_and_unit_equal(const on_did_unit_t* const dev1,
  const on_did_unit_t* const dev2)
{
    return (on_encoded_did_equal((const on_encoded_did_t* const) dev1->did,
        (const on_encoded_did_t* const) dev2->did) && dev1->unit == dev2->unit);
}


/*!
    \ brief Removes a recipient from a recipient list
	
	\param[out] rec_list The recipient list to delete from.
    \param[in] recipient_to_remove The recipient to remove
    
    \return TRUE if the list was changed, false otherwise.
*/
BOOL remove_recipient_from_recipient_list(on_recipient_list_t* rec_list,
    const on_did_unit_t* const recipient_to_remove)
{
    UInt8 i;
    for(i = 0; i < rec_list->num_recipients; i++)
    {
        if(did_and_unit_equal(&(rec_list->recipient_list[i]),
          recipient_to_remove))
        {
            if(i < rec_list->num_recipients - 1)
            {
                one_net_memmove(&(rec_list->recipient_list[i]),
                  &(rec_list->recipient_list[i+1]),
                  rec_list->num_recipients - i - 1);
            }
            
            return TRUE;
        }
    }
    
    return FALSE;
}


/*!
    \ brief Adds a recipient to a recipient list
	
	\param[out] rec_list The recipient list to add to.
    \param[in] recipient_to_add The recipient to add
    
    \return TRUE if the recipient is on the list after the function is done
            FASLE otherwise
*/
BOOL add_recipient_to_recipient_list(on_recipient_list_t* rec_list,
    const on_did_unit_t* const recipient_to_add)
{
    UInt8 i;

    if(on_encoded_did_equal(&(recipient_to_add->did), &NO_DESTINATION))
    {
        return FALSE; // destination is "no destination", so don't add
    }

    if(is_my_did(&(recipient_to_add->did)))
    {
        return FALSE; // message is to us.  Don't bother sending.
    }

    for(i = 0; i < rec_list->num_recipients; i++)
    {
        if(did_and_unit_equal(&(rec_list->recipient_list[i]),
          recipient_to_add))
        {
            return TRUE; // already on the list.
        }
    }

    if(rec_list->num_recipients >= ONE_NET_MAX_RECIPIENTS)
    {
        return FALSE; // no room.
    }

    one_net_memmove(&(rec_list->recipient_list[rec_list->num_recipients]),
        recipient_to_add, sizeof(on_did_unit_t));

    (rec_list->num_recipients)++;
    return TRUE;
}


/*!
    \brief Copies the did and sometimes the destination unit number
           of the next recipient of a message into the message itself
           so that the message can be sent
    
    \param[in/out] msg the message being sent
    \param[in/out] recipient_list The list of devices and units to send to

    \return
        The message if there is a message to send.
        NULL if there is no message to send.
*/
on_single_data_queue_t* load_next_recipient(on_single_data_queue_t* msg,
  on_recipient_list_t* recipient_list)
{
    on_did_unit_t* next_recipient;
    
    if(!msg || !recipient_list)
    {
        return NULL;
    }
    
    (recipient_list->recipient_index)++;
    if(recipient_list->recipient_index >=
      recipient_send_list_ptr->num_recipients
      || recipient_send_list_ptr->recipient_index < 0)
    {
        recipient_list->recipient_index = -2; // end of list
        return NULL;
    }
    
    next_recipient =
      &(recipient_list->recipient_list[recipient_list->recipient_index]);
    one_net_memmove(msg->dst_did, next_recipient->did,
      ON_ENCODED_DID_LEN);
      
    if(msg->msg_type == ON_APP_MSG)
    {
        put_dst_unit(next_recipient->unit, msg->payload);
    }
    return msg;
}


/*!
    \brief Determines whether this sevice will need to communicate in the
           near future with a device.
           
    This function is called when deciding whether to send a regular "ACK" or
    "NACK" or a "Stay Awake" ACK or NACK to a device.  A device should stay
    awake if there is a message to it in the queue or it is on the recipient
    list of a pending message.
    
    TODO -- we should probably also check all the messages in the queue to see
            if it might be on the peer list.  Also possibly alert the
            application code, which may be waiting.
            
    Note : The master must also check whether it is in the middle of a key
           change.  This function does not do that.
    
    \param[in] The DID of the device.

    \return
        TRUE if we have something pending for this device
        FALSE otherwise.
*/
BOOL device_should_stay_awake(const on_encoded_did_t* const did)
{
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    UInt8 i;
    for(i = 0; i < single_data_queue_size; i++)
    {
        if(on_encoded_did_equal((const on_encoded_did_t* const)
          &(single_data_queue[i].dst_did), (const on_encoded_did_t* const) did))
        {
            return TRUE;
        }
    }
    #endif
    
    // TODO -- check everything else.  This function probably needs to be
    //         moved from one_net_message.c to one_net.c so more things
    //         can be checked.
    //
    //         Also, what needs to be checked for devices with no queue?
    return FALSE;
}


#ifdef BLOCK_MESSAGES_ENABLED
on_single_data_queue_t* send_bs_setup_msg(const block_stream_msg_t* bs_msg,
  const on_encoded_did_t* dst)
{
    UInt8 setup_msg[21];
    block_stream_msg_t_to_admin_msg(setup_msg, bs_msg);
    return push_queue_element(ONE_NET_RAW_SINGLE_DATA, ON_ADMIN_MSG, setup_msg,
      21, ONE_NET_HIGH_PRIORITY, NULL, dst
      #ifdef PEER
          , FALSE,
          ONE_NET_DEV_UNIT
      #endif
      #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
    	  , 0
      #endif
  );
}


void admin_msg_to_block_stream_msg_t(const UInt8* msg, block_stream_msg_t*
  bs_msg, const on_encoded_did_t* src_did)
{
    // msg buffer must be at least 21 bytes
    bs_msg->flags = msg[BLOCK_STREAM_SETUP_FLAGS_IDX];
    bs_msg->frag_dly = one_net_byte_stream_to_int16(
      &msg[BLOCK_STREAM_SETUP_FRAG_DLY_IDX]);

    // irrelevant for stream 
    bs_msg->bs.block.transfer_size = one_net_byte_stream_to_int32(
      &msg[BLOCK_STREAM_SETUP_TRANSFER_SIZE_IDX]);
    bs_msg->bs.block.chunk_size = msg[BLOCK_STREAM_SETUP_CHUNK_SIZE_IDX];
    bs_msg->bs.block.chunk_pause = one_net_byte_stream_to_int16(
      &msg[BLOCK_STREAM_SETUP_CHUNK_PAUSE_IDX]);
      
    bs_msg->channel = msg[BLOCK_STREAM_SETUP_CHANNEL_IDX];
    bs_msg->data_rate = msg[BLOCK_STREAM_SETUP_DATA_RATE_IDX];
    bs_msg->timeout = one_net_byte_stream_to_int16(
      &msg[BLOCK_STREAM_SETUP_TIMEOUT_IDX]);

    bs_msg->src = NULL;
    bs_msg->dst = NULL;
    if(!is_my_did((const on_encoded_did_t*) &msg[BLOCK_STREAM_SETUP_DST_IDX]))
    {
        bs_msg->dst = (*get_sender_info)((const on_encoded_did_t* const)
          &msg[BLOCK_STREAM_SETUP_DST_IDX]);
    }
    
    if(src_did)
    {
        bs_msg->src = (*get_sender_info)(src_did);
    }
      
    bs_msg->time = get_tick_count() +
      MS_TO_TICK(one_net_byte_stream_to_int32(
      &msg[BLOCK_STREAM_SETUP_ESTIMATED_TIME_IDX]));
}


void block_stream_msg_t_to_admin_msg(UInt8* msg, const block_stream_msg_t*
  bs_msg)
{
    // msg buffer must be at least 21 bytes
    msg[0] = ON_REQUEST_BLOCK_STREAM;
    msg[BLOCK_STREAM_SETUP_FLAGS_IDX] = bs_msg->flags;      
    msg[BLOCK_STREAM_SETUP_CHANNEL_IDX] = bs_msg->channel;
    msg[BLOCK_STREAM_SETUP_DATA_RATE_IDX] = bs_msg->data_rate;
    one_net_int16_to_byte_stream(bs_msg->timeout,
      &msg[BLOCK_STREAM_SETUP_TIMEOUT_IDX]);
    one_net_memmove(&msg[BLOCK_STREAM_SETUP_DST_IDX], bs_msg->dst->did,
      ON_ENCODED_DID_LEN);
    
    #ifdef STREAM_MESSAGES_ENABLED  
    if(get_bs_transfer_type(bs_msg->flags) == ON_STREAM_TRANSFER)
    {
        one_net_int32_to_byte_stream(bs_msg->time,
          &msg[BLOCK_STREAM_SETUP_ESTIMATED_TIME_IDX]);
    }
    else
    #endif
    {
        tick_t est_tick = 0;
        tick_t cur_tick = get_tick_count();
        
        one_net_int32_to_byte_stream(bs_msg->bs.block.transfer_size,
          &msg[BLOCK_STREAM_SETUP_TRANSFER_SIZE_IDX]);
        msg[BLOCK_STREAM_SETUP_CHUNK_SIZE_IDX] = bs_msg->bs.block.chunk_size;
        one_net_int16_to_byte_stream(bs_msg->frag_dly,
          &msg[BLOCK_STREAM_SETUP_FRAG_DLY_IDX]);
        one_net_int16_to_byte_stream(bs_msg->bs.block.chunk_pause,
          &msg[BLOCK_STREAM_SETUP_CHUNK_PAUSE_IDX]);
        
        if(cur_tick < bs_msg->time)
        {
            est_tick = bs_msg->time - cur_tick;
        }
        one_net_int32_to_byte_stream(TICK_TO_MS(est_tick),
          &msg[BLOCK_STREAM_SETUP_ESTIMATED_TIME_IDX]);
    }
}


UInt8 get_current_bs_chunk_size(const block_stream_msg_t* bs_msg)
{
    UInt32 num_packets_total = bs_msg->bs.block.transfer_size /
      ON_BS_DATA_PLD_SIZE;
    UInt32 num_packets_left;
    
    if(bs_msg->bs.block.byte_idx < 40)
    {
        return 1;
    }
    
    num_packets_left = num_packets_total - bs_msg->bs.block.byte_idx;
    
    if(num_packets_left <= 40)
    {
        return 1;
    }
    
    if(num_packets_left >= 40 + bs_msg->bs.block.chunk_size)
    {
        return bs_msg->bs.block.chunk_size;
    }
    
    return num_packets_left - 40;
}


BOOL block_get_index_sent(UInt8 index, const UInt8 array[5])
{
    UInt8 mask = (0x80 >> (index % 8));
    if(index >= MAX_CHUNK_SIZE)
    {
        return FALSE;
    }
    
    if(array[index / 8] & mask)
    {
        return TRUE;
    }
    return FALSE;
}


void block_set_index_sent(UInt8 index, BOOL rcvd, UInt8 array[5])
{
    UInt8 array_index;
    UInt8 mask = (0x80 >> (index % 8));
    if(index >= MAX_CHUNK_SIZE)
    {
        return;
    }
    
    array_index = index / 8;
    
    array[array_index] &= ~mask;
    if(!rcvd)
    {
        return;
    }
    array[array_index] |= mask;
}


// returns -1 if all are sent
SInt8 block_get_lowest_unsent_index(const UInt8 array[5], UInt8 chunk_size)
{
    UInt8 i;
    for(i = 0; i < chunk_size; i++)
    {
        if(!block_get_index_sent(i, array))
        {
            return i;
        }
    }
    return -1;
}


UInt32 block_get_bytes_remaining(UInt32 transfer_size, UInt32 byte_index,
  UInt8 chunk_index)
{
    UInt32 byte_number_start = (byte_index + chunk_index) * ON_BS_DATA_PLD_SIZE;
    if(byte_number_start > transfer_size)
    {
        return 0;
    }
    transfer_size -= byte_number_start;
    return (transfer_size < ON_BS_DATA_PLD_SIZE ? transfer_size :
      ON_BS_DATA_PLD_SIZE );
}
#endif





//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{
    
    
#if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static void delete_expired_queue_elements(void)
{
    int i;
    tick_t cur_tick = get_tick_count();
    for(i = single_data_queue_size - 1; i >= 0 ; i--)
    {
        if(single_data_queue[i].expire_time > 0 &&
          single_data_queue[i].expire_time < cur_tick)
        {
            pop_queue_element(NULL, NULL, i);
        }
    }
}
#endif    


//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_MESSAGE
