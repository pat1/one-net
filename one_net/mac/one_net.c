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



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Preamble
static const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
static const UInt8 SOF[] = {0x33};

//! Header(Preamble and SOF)
static const UInt8 HEADER[] = {0x55, 0x55, 0x55, 0x33};


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


//! location to store the encoded data for an ack/nack packet
UInt8 response_pkt[ON_ACK_NACK_ENCODED_PKT_SIZE];

//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, sizeof(response_pkt), response_pkt,
  NULL, NULL};

//! location to store the encoded data for the single transaction
UInt8 single_pkt[ON_SINGLE_ENCODED_PKT_SIZE];

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  sizeof(single_pkt), single_pkt, NULL, NULL};

#ifdef _BLOCK_MESSAGES_ENABLED
    //! location to store the encoded data for a block transaction.
    UInt8 block_pkt[ON_BLOCK_ENCODED_PKT_SIZE];
    
    //! The current block transaction
    on_txn_t block_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BLOCK_TIMER, 0, sizeof(block_pkt), block_pkt, NULL, NULL};

    #ifdef _STREAM_MESSAGES_ENABLED
    //! location to store the encoded data for a stream transaction.
    UInt8 stream_pkt[ON_STREAM_ENCODED_PKT_SIZE];
    
    //! The current stream transaction
    on_txn_t stream_txn = {ON_STREAM, ONE_NET_NO_PRIORITY, 0,
      ONT_STREAM_TIMER, 0, sizeof(stream_pkt), stream_pkt, NULL, NULL};    
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
      ONT_MH_TIMER, 0, sizeof(mh_pkt), mh_pkt};
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

    \param[out] hops The hops field to be sent with the pkt.
    \param[in] MAX_HOPS The maximum number of hops the packet can take.
    \param[in] HOPS_LEFT The number of hops remaining that the pkt can take.

    \return ONS_SUCCESS If building the hops field was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
one_net_status_t on_build_hops(UInt8 * hops, UInt8 MAX_HOPS,
  UInt8 HOPS_LEFT)
{
    UInt8 raw_hops;

    if(!hops || MAX_HOPS > ON_MAX_HOPS_LIMIT || HOPS_LEFT > MAX_HOPS)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    raw_hops = ((MAX_HOPS << ON_MAX_HOPS_SHIFT) & ON_MAX_HOPS_BUILD_MASK)
      | ((HOPS_LEFT << ON_HOPS_LEFT_SHIFT) & ON_HOPS_LEFT_BUILD_MASK);

    on_encode(hops, &raw_hops, ON_ENCODED_HOPS_SIZE);

    return ONS_SUCCESS;
} // on_build_hops //
#endif // ifdef _ONE_NET_MULTI_HOP //
    

one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  const on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device)
{
    UInt8 status;
    UInt8 decoded_pld[4 * ONE_NET_XTEA_BLOCK_SIZE + 1]; // TODO -- make global
    SInt8 raw_pld_len = get_raw_payload_len(*(pkt_ptrs->pid));
    SInt8 num_words = get_encoded_payload_len(*(pkt_ptrs->pid));
    
    if(num_words <= 0)
    {
        return ONS_INTERNAL_ERR; // not a data PID
    }

    
    #ifdef _ONE_NET_MULTI_HOP
    if(packet_is_multihop(pkt_ptrs->pid))
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
    decoded_pld[ON_PLD_TXN_NONCE_IDX] = (device->expected_nonce <<
      ON_TXN_NONCE_SHIFT) & ON_TXN_NONCE_BUILD_MASK;
    decoded_pld[ON_PLD_RESP_NONCE_HIGH_IDX] |= (device->send_nonce
      >> ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK;
    decoded_pld[ON_PLD_RESP_NONCE_LOW_IDX] = (device->send_nonce <<
      ON_RESP_NONCE_LOW_SHIFT) & ON_RESP_NONCE_BUILD_LOW_MASK;
    decoded_pld[ON_PLD_MSG_TYPE_IDX] |= msg_type;
    one_net_memmove(&decoded_pld[ON_PLD_DATA_IDX], raw_pld,
      (raw_pld_len - 1) - ON_PLD_DATA_IDX);
      
    // compute the crc
    decoded_pld[0] = (UInt8)one_net_compute_crc(
      &decoded_pld[ON_PLD_CRC_SIZE], (raw_pld_len - 1) - ON_PLD_CRC_SIZE,
      ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
      
    if((status = on_encrypt(txn->txn_type, decoded_pld, txn->key,
      raw_pld_len)) == ONS_SUCCESS)
    {
        status = on_encode(pkt_ptrs->payload, decoded_pld,
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
          pkt_ptrs->max_hops) != ONS_SUCCESS)
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
            // we are listinging for data.  Make sure we have nothing
            // pending
            if(*txn == NULL && single_txn.priority == ONE_NET_NO_PRIORITY
              && single_msg_ptr == NULL)
            {
                on_sending_device_t* device;
                
                tick_t next_pop_time; // we don't sleep, so we won't use
                                       // this.  But the function requires it
                                       // as a parameter.
                

                // nothing is pending.  See if we have anything ready to go
                int index = single_data_queue_ready_to_send(&next_pop_time);
                if(index >= 0)
                {
                    if(pop_queue_element(&single_msg, single_data_raw_pld, index))
                    {
                        // we have a message ready to send and we've popped it.
                        // Now let's get things ready to send.
                        
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
                        
                        #ifdef _ONE_NET_MULTI_HOP
                        // TODO -- What about hops?
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
                            #ifdef _ONE_NET_MULTI_HOP
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
                        *txn = &single_txn;
                        single_msg_ptr = &single_msg;
                        on_state = ON_SEND_SINGLE_DATA_PKT;
                        
                        return FALSE; // transaction is not complete
                    }
                }
            }
        } // case ON_LISTEN_FOR_DATA //
    }
    
    return TRUE;
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

