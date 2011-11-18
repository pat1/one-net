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
#ifdef _PEER
#include "one_net_peer.h"
#endif



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Preamble
const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
const UInt8 SOF[] = {0x33};

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


#ifdef _ONE_NET_MULTI_HOP
//! flag that indicates if any Multi-Hop Repeaters have joined the network
BOOL mh_repeater_available = FALSE;
#endif


//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
UInt8 nv_param[NV_PARAM_SIZE_BYTES];

//! The base parameters for the device
on_base_param_t* const on_base_param = (on_base_param_t* const) nv_param;


//! The set of packet handlers
on_pkt_hdlr_set_t pkt_hdlr;

//! a function to retrieve the sender information
one_net_get_sender_info_func_t get_sender_info;


//! location to store the encoded data for an ack/nack packet
UInt8 response_pkt[ON_ACK_NACK_ENCODED_PKT_SIZE];

//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, 0, response_pkt,
  NULL, NULL};

//! location to store the encoded data for the single transaction
UInt8 single_pkt[ON_SINGLE_ENCODED_PKT_SIZE];

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  0, single_pkt, NULL, NULL};

#ifdef _BLOCK_MESSAGES_ENABLED
    //! location to store the encoded data for a block transaction.
    UInt8 block_pkt[ON_BLOCK_ENCODED_PKT_SIZE];
    
    //! The current block transaction
    on_txn_t block_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BLOCK_TIMER, 0, 0, block_pkt, NULL, NULL};

    #ifdef _STREAM_MESSAGES_ENABLED
    //! location to store the encoded data for a stream transaction.
    UInt8 stream_pkt[ON_STREAM_ENCODED_PKT_SIZE];
    
    //! The current stream transaction
    on_txn_t stream_txn = {ON_STREAM, ONE_NET_NO_PRIORITY, 0,
      ONT_STREAM_TIMER, 0, 0, stream_pkt, NULL, NULL};    
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

//! location to store the encoded data for an invite transaction.
UInt8 invite_pkt[ON_INVITE_ENCODED_PKT_SIZE];

//! Unique key of the device being invited into the network
one_net_xtea_key_t invite_key;



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
    // fill in the preamble in the Multi-Hop packet to be sent.  The rest will
    // be filled in when the received Multi-Hop packet is read in over the rf
    // interface.
    static UInt8 mh_pkt[ON_MAX_ENCODED_PKT_SIZE] = {0x55, 0x55, 0x55, 0x33};

    // Transaction for forwarding on MH packets.
    static on_txn_t mh_txn = {ON_NO_TXN, ONE_NET_LOW_PRIORITY, 0,
      ONT_MH_TIMER, 0, 0, mh_pkt};
#endif

//! A place to store a message header for a data packet
static on_msg_hdr_t data_msg_hdr;

//! A place to store a message header for a reponse packet
static on_msg_hdr_t resp_msg_hdr;



#ifdef _RANGE_TESTING
//! Stores the DIDs of the devices which are in range.
static on_encoded_did_t range_test_did_array[RANGE_TESTING_ARRAY_SIZE];

//! If true, trange test.  If false, do not.
static BOOL range_testing_on = FALSE;
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
    

one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  const on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device)
{
    UInt8 status;
    SInt8 raw_pld_len = get_raw_payload_len(*(pkt_ptrs->pid));
    SInt8 num_words = get_encoded_payload_len(*(pkt_ptrs->pid));
    
    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
    }

    
    #ifdef _ONE_NET_MULTI_HOP
    if(packet_is_multihop(*(pkt_ptrs->pid)))
    {
        // build hops
        if((status = on_build_hops(pkt_ptrs->enc_hops_field, pkt_ptrs->hops,
          pkt_ptrs->max_hops)) != ONS_SUCCESS)
        {
            return status;
        }
    }
    #endif
    
    // check nonces.  If they are invalid, pick some random ones
    if(device->expected_nonce > ON_MAX_NONCE)
    {
        device->expected_nonce = one_net_prand(get_tick_count(),
          ON_MAX_NONCE);
    }
    if(device->send_nonce > ON_MAX_NONCE)
    {
        device->send_nonce = one_net_prand(get_tick_count(),
          ON_MAX_NONCE);
    }

    // build the packet
    raw_payload_bytes[ON_PLD_TXN_NONCE_IDX] = (device->expected_nonce <<
      ON_TXN_NONCE_SHIFT) & ON_TXN_NONCE_BUILD_MASK;
    raw_payload_bytes[ON_PLD_RESP_NONCE_HIGH_IDX] |= (device->send_nonce
      >> ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK;
    raw_payload_bytes[ON_PLD_RESP_NONCE_LOW_IDX] = (device->send_nonce <<
      ON_RESP_NONCE_LOW_SHIFT) & ON_RESP_NONCE_BUILD_LOW_MASK;
    raw_payload_bytes[ON_PLD_MSG_TYPE_IDX] |= msg_type;
    one_net_memmove(&raw_payload_bytes[ON_PLD_DATA_IDX], raw_pld,
      (raw_pld_len - 1) - ON_PLD_DATA_IDX);
      
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

    \param[in] pid The pid of the packet
    \param[in] decrypted The decrypted bytes

    \return TRUE if the message CRC is valid, FALSE otherwise
*/
BOOL verify_payload_crc(UInt8 pid, const UInt8* decrypted)
{
    const UInt8* crc_calc_start;
    UInt8 crc_calc, crc_calc_len;
    SInt8 num_encoded_blocks = get_num_payload_blocks(pid);
    
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

    \param[in] pid pid of the packet
    \param[in] pkt_bytes The array holding the packet bytes
    \param[out] pkt The on_pkt_t structure to fill

    \return TRUE if the on_pkt structure was set up successfully.
            FALSE upon error.
*/
BOOL setup_pkt_ptr(UInt8 pid, UInt8* pkt_bytes, on_pkt_t* pkt)
{
    SInt8 len = get_encoded_payload_len(pid);
    if(len < 0)
    {
        return FALSE; // bad pid
    }
    
    if(!pkt_bytes || !pkt)
    {
        return FALSE;
    }
    
    pkt->packet_header    = &pkt_bytes[0];
    pkt->pid              = &pkt_bytes[ONE_NET_ENCODED_PID_IDX];
    *(pkt->pid)           = pid;
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
    switch(on_state)
    {
        case ON_LISTEN_FOR_DATA:
        {
            #ifdef _ONE_NET_MULTI_HOP
            on_raw_did_t raw_did;
            #endif
            
            // we are listinging for data.  Make sure we have nothing
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
                if(!load_next_peer(&single_msg))
                {
                    // we are not in the middle of a message.  We might have
                    // something ready to pop though.
                    
                    #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                    int index = single_data_queue_ready_to_send(
                        &next_pop_time);
                    #elif _SINGLE_QUEUE_LEVEL == MIN_SINGLE_QUEUE_LEVEL
                    int index = single_data_queue_ready_to_send();
                    #endif
                    if(index >=  0)
                    {
                        if(pop_queue_element(&single_msg, single_data_raw_pld,
                          index))
                        {
                            // we have just popped a message.  Set things up
                            // with the peer list, then next time the
                            // load_next_peer will take it from there.
                            if(setup_send_list(&single_msg, NULL, NULL))
                            {
                                // we have a message.  We'll take it further
                                // the next trip through.
                                return FALSE; // we have no transaction, but
                                              // we have something to send.
                                              // Return false.
                            }
                        }
                    }
                    
                    // we are either done or we never had anything to send in
                    // the first place.  Clear it and return TRUE.
                    single_msg_ptr = NULL;
                    return TRUE;
                }

                // we have a message.  Let's create the packet.
                
                // first get the sending device info.
                device = (*get_sender_info)((on_encoded_did_t*)
                  single_msg.dst_did);
                if(device == NULL)
                {
                    // an error occurred somewhere.  Abort.
                    return TRUE; // no pending transaction since we're
                                 // aborting
                }
  
                // fill with 0xEE for test purposes
                one_net_memset(single_txn.pkt, 0xEE, ON_SINGLE_ENCODED_PKT_SIZE);
                
                if(!setup_pkt_ptr(single_msg.pid, single_pkt,
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
                
                // change the pid if needed
                if(single_txn.max_hops)
                {
                    switch(single_msg.pid)
                    {
                        case ONE_NET_ENCODED_SINGLE_DATA:
                          *(data_pkt_ptrs.pid) = ONE_NET_ENCODED_MH_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
                          *(data_pkt_ptrs.pid) =
                            ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA;
                          break;
                        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA:
                          *(data_pkt_ptrs.pid) =
                            ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA;
                          break;
                    }
                }
                
                data_pkt_ptrs.hops = single_txn.hops;
                data_pkt_ptrs.max_hops = single_txn.max_hops;
                #endif

                // pick a message id if we don't already have one.
                if(device->msg_id < ON_MAX_MSG_ID)
                {
                    // message id is valid and is not ready for a
                    // "roll-over".  Increment it.
                    (device->msg_id)++;
                }
                else
                {
                    // pick a random one.
                    device->msg_id = /*one_net_prand(get_tick_count(),
                      ON_MAX_MSG_ID)*/0x35;
                }
                        
                // Check the nonces too.  See if they are valid
                if(device->send_nonce > ON_MAX_NONCE)
                {
                    device->send_nonce = /*one_net_prand(
                      get_tick_count(), ON_MAX_NONCE)*/28;
                }
                // Check the nonces too.  See if they are valid
                if(device->expected_nonce > ON_MAX_NONCE)
                {
                    device->expected_nonce = /*one_net_prand(
                      get_tick_count(), ON_MAX_NONCE)*/0x12;
                }
                
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
                if(on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn,
                  device)!= ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return TRUE; // no outstanding transaction
                }

                // now finish building the packet.
                if(on_complete_pkt_build(&data_pkt_ptrs,
                  device->msg_id, single_msg.pid) != ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return TRUE; // no outstanding transaction                            
                }

                // packet was built successfully.  Set the transaction,
                // state, etc.
                single_txn.priority = single_msg.priority;
                single_txn.data_len =
                  get_encoded_packet_len(single_msg.pid, TRUE);
                *txn = &single_txn;
                (*txn)->retry = 0;
                (*txn)->response_timeout = ONE_NET_RESPONSE_TIME_OUT;
                (*txn)->device = device;
                on_state = ON_SEND_SINGLE_DATA_PKT;
                // set the timer to send immediately
                ont_set_timer((*txn)->next_txn_timer, 0);
                single_msg_ptr = &single_msg;
                return FALSE; // transaction is not complete
            }

            break;
        } // case ON_LISTEN_FOR_DATA //
        
        case ON_SEND_PKT:
        case ON_SEND_SINGLE_DATA_PKT:
        case ON_SEND_SINGLE_DATA_RESP:
        {
            if(ont_inactive_or_expired((*txn)->next_txn_timer)
              && check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, get_encoded_packet_len(
                  (*txn)->pkt[ONE_NET_ENCODED_PID_IDX], TRUE));
                on_state++;
            } // if the channel is clear //
            
            break;
        } // case ON_SEND_SINGLE_DATA_PKT //
        
        case ON_SEND_PKT_WRITE_WAIT:
        case ON_SEND_SINGLE_DATA_WRITE_WAIT:
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                #ifdef _ONE_NET_MULTI_HOP
                UInt32 new_timeout_ms = (*txn)->max_hops * ONE_NET_MH_LATENCY
                  + (1 + (*txn)->max_hops) * (*txn)->response_timeout;
                #else
                UInt32 new_timeout_ms = (*txn)->response_timeout;
                #endif

                ont_set_timer((*txn)->next_txn_timer,
                  MS_TO_TICK(new_timeout_ms));
                  
                if(on_state == ON_SEND_PKT_WRITE_WAIT || on_state ==
                  ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT)
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    *txn = 0;
                    return TRUE;
                }
                else
                {
                    on_state++;
                }
            } // if write is complete //
            break;
        } // send single data write wait case //
        
        case ON_WAIT_FOR_SINGLE_DATA_RESP:
        {
            if(ont_inactive_or_expired(ONT_RESPONSE_TIMER))
            {
                BOOL terminate_txn = FALSE;
                
                #ifndef _ONE_NET_SIMPLE_DEVICE
                // we timed out without a response.  Notify application code
                // it's unlikely they'll decide to change the response time-
                // out, but they might.
                
                on_message_status_t msg_status;
                ack_nack_payload_t nack_payload;
                on_ack_nack_t ack_nack;
                ack_nack.payload = &nack_payload;
                nack_payload.nack_time_ms = ONE_NET_RESPONSE_TIME_OUT;
                ack_nack.handle = ON_NACK_TIME_MS;
                ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE;
                #endif
                
                (*txn)->retry++;
                
                #ifndef _ONE_NET_SIMPLE_DEVICE
                msg_status = (*pkt_hdlr.single_ack_nack_hdlr)(txn,
                  &data_pkt_ptrs, single_msg_ptr->payload,
                  &(single_msg_ptr->msg_type), &ack_nack);
                  
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
                        break;
                    }
                    default:
                    {
                        // transaction has been terminated either by ONE_NET
                        // or by the application code.
                        terminate_txn = TRUE;
                    }
                }
                #else
                terminate_txn = ((*txn)->retry >= ON_MAX_RETRY);
                #endif

                if(terminate_txn)
                {
                    #ifndef _ONE_NET_SIMPLE_DEVICE
                    // transaction has been terminated either by ONE_NET
                    // or by the application code.
                    (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                      single_msg_ptr->payload,
                      &(single_msg_ptr->msg_type), msg_status, &ack_nack);
                    #else
                    ack_nack_t ack_nack;
                    ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE_TXN;
                    (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                      single_msg_ptr->payload,
                      &(single_msg_ptr->msg_type), ON_MSG_TIMEOUT,
                      &ack_nack);                    
                    #endif
                      
                    // clear the transaction.
                    (*txn)->priority = ONE_NET_NO_PRIORITY;
                    *txn = 0;
                    single_msg_ptr = 0;
                    on_state = ON_LISTEN_FOR_DATA;
                    return TRUE;
                }
                else
                {
                    #ifdef _ONE_NET_MULTI_HOP
                    UInt32 new_timeout_ms = (*txn)->max_hops * ONE_NET_MH_LATENCY +
                      (1 + (*txn)->max_hops) * (*txn)->response_timeout;
                    #else
                    UInt32 new_timeout_ms = (*txn)->response_timeout;
                    #endif

                    ont_set_timer((*txn)->next_txn_timer,
                      MS_TO_TICK(new_timeout_ms));
                      on_state -= 2;  
                }
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
one_net_status_t rx_single_data(on_txn_t ** txn)
{
    return ONS_SUCCESS;
} // rx_single_data //


#ifdef _BLOCK_MESSAGES_ENABLED
one_net_status_t rx_block_data(on_txn_t** txn)
{
    return ONS_SUCCESS;
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t rx_stream_data(on_txn_t** txn)
{
    return ONS_SUCCESS;
}
#endif


/*!
    \brief Receives data packets only.

    This function is to only receive Single, Block, and Stream packets.
    All other packets are discarded.

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
    on_message_status_t hdlr_status;
    on_encoded_did_t src_did;
    one_net_xtea_key_t* key = NULL;
    UInt8 pid;
    BOOL dst_is_broadcast, dst_is_me, src_match;
    #ifdef _ONE_NET_MULTI_HOP
    BOOL packet_is_mh;
    UInt8 raw_hops_field;
    #endif
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    BOOL repeat_this_packet = FALSE;
    #endif
    UInt8 pkt_hdr[ON_PLD_IDX];
    on_txn_t* this_txn = NULL;
    on_data_t type;


    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!EXPECTED_SRC_DID || !txn)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) != ONS_SUCCESS)
    {
        return ONS_READ_ERR;
    }
    
    if(one_net_read(&pkt_hdr[ONE_NET_PREAMBLE_HEADER_LEN],
      ON_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN) !=
      ON_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN)
    {
        return ONS_READ_ERR;
    }
    
    // check some addresses
    if(!is_my_nid((on_encoded_nid_t*) (&pkt_hdr[ON_ENCODED_NID_IDX])))
    {
        return ONS_NID_FAILED; // not our network
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    // first check the source.  If it was us originally, then we probably
    // got back our own repeated packet.
    if(is_my_did((on_encoded_did_t*) &pkt_hdr[ON_ENCODED_SRC_DID_IDX]))
    {
        return ONS_DID_FAILED; // We SENT this packet, so no sense RECEIVING
                               // it.
    }
    #endif    

    #ifdef _RANGE_TESTING
    // check the repeater did
    #endif
    
    pid = pkt_hdr[ONE_NET_ENCODED_PID_IDX];
    dst_is_broadcast = is_broadcast_did((on_encoded_did_t*)
      (&pkt_hdr[ONE_NET_ENCODED_DST_DID_IDX]));
    dst_is_me = is_my_did((on_encoded_did_t*)
      (&pkt_hdr[ONE_NET_ENCODED_DST_DID_IDX]));
    src_match = is_broadcast_did(EXPECTED_SRC_DID) ||
      on_encoded_did_equal(EXPECTED_SRC_DID,
      (on_encoded_did_t*) &pkt_hdr[ON_ENCODED_SRC_DID_IDX]);
    
    #ifdef _ONE_NET_MULTI_HOP
    packet_is_mh = packet_is_multihop(pid);
    #endif
    
    #ifndef _ONE_NET_MH_CLIENT_REPEATER
    if(!src_match || (!dst_is_me && !dst_is_broadcast))
    {
        return ONS_BAD_ADDR;
    }
    #else
    if(!src_match || (!dst_is_me && !dst_is_broadcast))
    {
        // not to us, but maybe we'll repeat it if we're not the master,
        // not in the middle of our own transaction, it's a multi-hop
        // packet, and it wasn't to us.
        if(dst_is_me || device_is_master || *txn || !packet_is_multihop)
        {
            return ONS_BAD_PARAM;
        }
        
        // we'll repeat it if there are any hops left.
        repeat_this_packet = TRUE;
    }
    #endif


    switch(get_pkt_family(pid))
    {
        case SINGLE_PKT_GRP:
        {
            type = ON_SINGLE;
            this_txn = &single_txn;
            break;
        }
        #ifdef _BLOCK_MESSAGES_ENABLED
        case BLOCK_PKT_GRP:
        {
            type = ON_BLOCK;
            if(*txn != &block_txn)
            {
                return ONS_UNHANDLED_PKT; // why are we getting a block packet
                  // if we are not in the middle of a block transaction?
            }
            this_txn = &block_txn;
            break;
        }
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case STREAM_PKT_GRP:
        {
            type = ON_STREAM;
            if(*txn != &stream_txn)
            {
                return ONS_UNHANDLED_PKT; // why are we getting a block packet
                  // if we are not in the middle of a block transaction?
            }
            this_txn = &stream_txn;
            break;
        }
        #endif
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
        case INVITE_PKT_GRP: case ACK_NACK_PKT_GRP:
        {
            // this definitely isn't for us, but maybe we'll repeat it.
            if(device_is_master || *txn || !packet_is_multihop)
            {
                return ONS_UNHANDLED_PKT;
            }
            repeat_this_packet = TRUE;
            break;
        }
        #endif
        default:
        {
            return ONS_BAD_PKT_TYPE;
        }
    }
    
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    if(repeat_this_packet)
    {
        this_txn = &mh_txn;
    }
    #endif
    
    one_net_memmove(this_txn->pkt, pkt_hdr, ON_PLD_IDX);

    if(!setup_pkt_ptr(pid, this_txn->pkt, &data_pkt_ptrs))
    {
        return ONS_INTERNAL_ERR;
    }
    
    if(!one_net_read(&(this_txn->pkt[ON_PLD_IDX]), data_pkt_ptrs.payload_len)
      != data_pkt_ptrs.payload_len)
    {
        return ONS_READ_ERR;
    }
    
    #ifndef _ONE_NET_MH_CLIENT_REPEATER
    if(!verify_msg_crc(&data_pkt_ptrs))
    #else
    // don't bother verifying if we are just going to repeat.
    if(!repeat_this_packet && !verify_msg_crc(&data_pkt_ptrs))
    #endif
    {
        return ONS_CRC_FAIL;
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    if(packet_is_mh)
    {
        UInt8 raw_hops_field;

        if(one_net_read(data_pkt_ptrs.enc_hops_field, ON_ENCODED_HOPS_SIZE)
          != ON_ENCODED_HOPS_SIZE)
        {
            return ONS_READ_ERR;
        }

        if(on_decode(&raw_hops_field, data_pkt_ptrs.enc_hops_field,
          ON_ENCODED_HOPS_SIZE) != ONS_SUCCESS)
        {
            return ONS_BAD_ENCODING;
        }
        
        // TODO -- use defined masks and shifts
        raw_hops_field >>= 2;
        data_pkt_ptrs.max_hops = raw_hops_field & 0x07;
        data_pkt_ptrs.hops = (raw_hops_field >> 3);
        
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if(repeat_this_packet)
        {
            if(data_pkt_ptrs.hops >= data_pkt_ptrs.max_hops)
            {
                // too many hops.  Don't repeat.
                return ONS_UNHANDLED_PKT;
            }
            
            // we have everything.  Increment the hops, add ourself as the
            // repeater, and send the message.
            (data_pkt_ptrs.hops)++;
            
            // TODO -- fix the building and parsing of hops.  Some start with
            // 0 and add each time, some start with the max and SUBTRACT each
            // time.  Doesn't matter which we do, but be consistent
            // everywhere.
            raw_hops_field = (((data_pkt_ptrs.hops) << 3) +
              data_pkt_ptrs.max_hops) << 2;
            on_encode(data_pkt_ptrs.enc_hops_field, &raw_hops_field,
              ON_ENCODED_HOPS_SIZE);
            one_net_memmove(data_pkt_ptrs.enc_repeater_did,
              &(on_base_param->sid[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
            // packet is all filled in.  Send it back.
            *txn = &mh_txn;
            on_state = ON_SEND_PKT;
            return ONS_SUCCESS;
        }
        #endif
    }
    #endif
    
    key = &(on_base_param->current_key);
    #ifdef _STREAM_MESSAGES_ENABLED
    if(type == ON_STREAM)
    {
        key = &(on_base_param->stream_key);
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        #ifdef _STREAM_MESSAGES_ENABLED
        key = master_get_encryption_key(type, data_pkt_ptrs.enc_src_did);
        #else
        key = master_get_encryption_key(data_pkt_ptrs.enc_src_did);
        #endif
    }
    #endif
    
    if((status = on_decrypt(type, raw_payload_bytes, key,
      data_pkt_ptrs.payload_len)) != ONS_SUCCESS)
    {
        return status;
    }
    
    // check the payload CRC.
    if(!verify_payload_crc(pid, raw_payload_bytes))
    {
        return ONS_CRC_FAIL;
    }
    
    // decode the message id and fill it in.
    on_decode(&(data_pkt_ptrs.msg_id), data_pkt_ptrs.enc_msg_id,
      ONE_NET_ENCODED_MSG_ID_LEN);

    // so far, so good.
    *txn = this_txn; // pass it to the individual handlers
    switch(type)
    {
        // note -- we'll pass NULL as the message type and let the
        // handlers handle it further.  TODO -- we may decide to parse
        // some of that here.
        case ON_SINGLE: hdlr_status = (*pkt_hdlr.single_data_hdlr)(txn,
          &data_pkt_ptrs, raw_payload_bytes, NULL); break;
        #ifdef _BLOCK_MESSAGES_ENABLED  
        case ON_BLOCK: hdlr_status = (*pkt_hdlr.block_data_hdlr)(txn,
          &data_pkt_ptrs, raw_payload_bytes, NULL); break;
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED  
        case ON_STREAM: hdlr_status = (*pkt_hdlr.stream_data_hdlr)(txn,
          &data_pkt_ptrs, raw_payload_bytes, NULL); break;
        #endif
    }
    
    // TODO -- harness the return value?  What handles aborting of the
    // transaction, etc., when the transaction needs to be aborted,
    // paused, re-sent, etc.?  We'll probably NOT handle much of it in the
    // main ONE-NET state machine, but instead handle a lot of it here.
    // The handling of block, stream, and single-SPECIFIC issues should
    // be handled there, not here, but anything in common, should be
    // handled here.  For now, if we got this far, just return ONS_SUCCESS.
    
    return ONS_SUCCESS;
} // on_rx_data_pkt //


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

