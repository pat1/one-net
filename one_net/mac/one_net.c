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



//! A buffer containing all encoded bytes for transmitting and receiving
UInt8 encoded_pkt_bytes[SNIFF_PKT_BUFFER_SIZE][ON_MAX_ENCODED_PKT_SIZE];



//                              PUBLIC VARIABLES
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{



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

    \param[in] is_stream_pkt True if the packet is a stream packet, false otherwise
    \param[in/out] data The data to encrypt
    \param[in] KEY The XTEA key used to encrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t on_encrypt(BOOL is_stream_pkt , UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
#else
one_net_status_t on_encrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
#endif
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    #ifdef _STREAM_MESSAGES_ENABLED
    // get the number of XTEA rounds
    if(is_stream_pkt)
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

    \param[in] is_stream_pkt True if the packet is a stream packet, false otherwise
    \param[in/out] data The data to decrypt
    \param[in] key The XTEA key used to decrypt the data
    \param[in] payload_len Length to be encrypted, including one byte that is
               NOT to be encrypted and instead holds the encryption TECHNIQUE.
               Must be a multiple of 8, plus 1 (i.e. 9, 17, 25, 33, ...)

    \return The status of the operation
*/
#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t on_decrypt(BOOL is_stream_pkt , UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
#else
one_net_status_t on_decrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len)
#endif
{
    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY || (payload_len < 9) || ((payload_len % 8) != 1))
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    // get the number of XTEA rounds
	#ifdef _STREAM_MESSAGES_ENABLED
    if(is_stream_pkt)
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
void one_net(on_txn_t ** txn)
{
} // one_net //


// TODO -- document
static on_message_status_t rx_single_resp_pkt(on_txn_t** const txn,
  on_txn_t** const this_txn, on_pkt_t* const pkt,
  UInt8* const raw_payload_bytes, on_ack_nack_t* const ack_nack)
{
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

    \param[in] The current transaction being carried out.
    \param[out] this_txn The packet received
    \param[out] this_pkt_ptrs Filled in pointers to the packet received
    \param[out] raw_payload_bytes The decoded / decrypted bytes of this packet

    \return ONS_PKT_RCVD if a valid packet was received
            For more return values one_net_status_codes.h
*/
#if defined(_BLOCK_MESSAGES_ENABLED) || defined(_ONE_NET_MH_CLIENT_REPEATER)
one_net_status_t on_rx_packet(const on_txn_t* const txn, on_txn_t** this_txn,
  on_pkt_t** this_pkt_ptrs, UInt8* raw_payload_bytes)
#else
one_net_status_t on_rx_packet(on_txn_t** this_txn, on_pkt_t** this_pkt_ptrs,
  UInt8* raw_payload_bytes)
#endif
{
    return ONS_READ_ERR;
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
