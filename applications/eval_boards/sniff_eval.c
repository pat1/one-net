//! \defgroup ONE-NET_sniff_eval ONE-NET Packet Sniffer
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file sniff_eval.c
    \brief The packet sniffer part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#ifdef _SNIFFER_MODE



#include "oncli.h"
#include "uart.h"
#include "one_net_xtea.h"
#include "one_net_encode.h"
#include "oncli_str.h"
#include "one_net_timer.h"
#include "tick.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "one_net_eval.h"
#include "hal.h"
#include "tal.h"
#include "oncli.h"
#include "cb.h"
#include "one_net_crc.h"



//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_sniff_eval_const
//! \ingroup ONE-NET_sniff_eval
//! @{


extern const char HEX_DIGIT[];
extern const UInt8 HEADER[];


//! @} ONE-NET_sniff_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_sniff_eval_typedefs
//! \ingroup ONE-NET_sniff_eval
//! @{

//! @} ONE-NET_sniff_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_sniff_eval_pri_var
//! \ingroup ONE-NET_sniff_eval
//! @{


//! The channel currently being sniffed
static UInt8 sniff_channel;

//! Buffer to hold the string representation of the channel being sniffed
static char channel_format_buffer[MAX_CHANNEL_STRING_FORMAT_LENGTH];

#if _DEBUG_VERBOSE_LEVEL > 1
enum
{
    //! number of known invite keys to try for decryption.
    NUM_SNIFF_INVITE_KEYS = 2,
    
    //! number of known encryption keys to try for decryption.
    NUM_SNIFF_ENCRYPT_KEYS = 1,

    #ifdef _STREAM_MESSAGES_ENABLED
    //! number of known encryption keys to try for decryption.
    NUM_SNIFF_STREAM_ENCRYPT_KEYS = 1
    #endif
};
#endif



//! @} ONE-NET_sniff_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

#if _DEBUG_VERBOSE_LEVEL > 1
//! Place any known invite keys in the array below
static const one_net_xtea_key_t sniff_invite_keys[NUM_SNIFF_INVITE_KEYS] =
{
    {'2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2'},
    {'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'}
};

//! Place any known encryption keys in the array below
static const one_net_xtea_key_t sniff_enc_keys[NUM_SNIFF_ENCRYPT_KEYS] =
{
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
     0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}
};

#ifdef _STREAM_MESSAGES_ENABLED
//! Place any known stream encryption keys in the array below
static const one_net_xtea_key_t
  sniff_stream_enc_keys[NUM_SNIFF_STREAM_ENCRYPT_KEYS] =
{
    {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
     0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}
};
#endif
#endif

#if _DEBUG_VERBOSE_LEVEL > 0
static void debug_display_did(const char* const description,
  on_encoded_did_t* enc_did);
static void debug_display_nid(const char* const description,
  on_encoded_nid_t* enc_nid);
#endif


//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_sniff_eval_pub_func
//! \ingroup ONE-NET_sniff_eval
//! @{



oncli_status_t oncli_reset_sniff(const UInt8 CHANNEL)
{
    if(oncli_format_channel(CHANNEL, channel_format_buffer,
      MAX_CHANNEL_STRING_FORMAT_LENGTH) != channel_format_buffer)
    {
        // error
        return ONCLI_INTERNAL_ERR;
    }
      
    in_sniffer_mode = TRUE;
    sniff_channel = CHANNEL;
    node_loop_func = &sniff_eval;
    on_base_param->channel = sniff_channel;
    one_net_set_channel(on_base_param->channel);
    
    return ONCLI_SUCCESS;
} // oncli_reset_sniff //


#if _DEBUG_VERBOSE_LEVEL > 0
/*!
    \brief Displays a DID in verbose fashion.

    \param[in] description The description to prepend in front of the DID
    \param[in] enc_did The encioded DID to display.
    
    \return void
*/
void debug_display_did(const char* const description,
  const on_encoded_did_t* const enc_did)
{
    #if _DEBUG_VERBOSE_LEVEL > 1
    on_raw_did_t raw_did;
    BOOL valid = (on_decode(raw_did, *enc_did, ON_ENCODED_DID_LEN) ==
      ONS_SUCCESS);
    #endif
    
    oncli_send_msg("%s : 0x%02X%02X", description, (*enc_did)[0], (*enc_did)[1]);
    #if _DEBUG_VERBOSE_LEVEL > 1
    if(valid)
    {
        oncli_send_msg(" -- Decoded : 0x%03X", did_to_u16(&raw_did));
    }
    else
    {
        oncli_send_msg(" -- Invalid");
    }
    #endif
    oncli_send_msg("\n");
}


/*!
    \brief Displays an NID in verbose fashion.

    \param[in] description The description to prepend in front of the NID
    \param[in] enc_nid The encioded NID to display.
    
    \return void
*/
void debug_display_nid(const char* const description,
  const on_encoded_nid_t* const enc_nid)
{
    #if _DEBUG_VERBOSE_LEVEL > 1
    on_raw_nid_t raw_nid;
    BOOL valid = (on_decode(raw_nid, *enc_nid, ON_ENCODED_NID_LEN) ==
      ONS_SUCCESS);
    #endif
    
    oncli_send_msg("%s : 0x", description);
    uart_write_int8_hex_array(*enc_nid, FALSE, ON_ENCODED_NID_LEN);
    #if _DEBUG_VERBOSE_LEVEL > 1
    if(valid)
    {
        oncli_send_msg(" -- Decoded : 0x");
        uart_write_int8_hex_array(raw_nid, FALSE, ON_RAW_NID_LEN-1);
        oncli_send_msg("%c", HEX_DIGIT[(raw_nid[ON_RAW_NID_LEN-1] >> 4)
          & 0x0F]);
    }
    else
    {
        oncli_send_msg(" -- Invalid");
    }
    #endif
    oncli_send_msg("\n");
}
#endif



/*!
    \brief Parses and displays a packet

    \param[in] packet_bytes The bytes that make up the packet.
    \param[in] num_bytes The number of bytes in the packet.
    \param[in] enc_keys the block / single keys to check.  If not relevant, set to
                 NULL and set num_keys to 0.
    \param[in] num_enc_keys the number of block / single keys to check.
    \param[in] invite_keys the invite keys to check.  If not relevant, set to
                 NULL and set num_invite_keys to 0.
    \param[in] num_invite_keys the number of invite keys to check.
    \param[in] stream_keys the stream keys to check.  If not relevant, set to
                 NULL and set num_stream_keys to 0.
    \param[in] num_stream_keys the number of stream keys to check.
    
    \return void
*/
#if _DEBUG_VERBOSE_LEVEL < 2
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes)
#else
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes,
  const one_net_xtea_key_t* const enc_keys, UInt8 num_enc_keys,
  const one_net_xtea_key_t* const invite_keys, UInt8 num_invite_keys,
  const one_net_xtea_key_t* const stream_keys, UInt8 num_stream_keys)
#endif
{
    UInt8 i;
    for(i = 0; i < num_bytes; i++)
    {
        oncli_send_msg("%02X ", packet_bytes[i]);
    
        if((i % 16) == 15)
        {
            oncli_send_msg("\n");
        } // if need to output a new line //
    } // loop to output the bytes that were read //
    oncli_send_msg("\n\n");   

    #if _DEBUG_VERBOSE_LEVEL > 0
    {
        UInt8 pid = packet_bytes[ONE_NET_ENCODED_PID_IDX];
        on_pkt_t debug_pkt_ptrs;
        oncli_send_msg("pid=%02X", pid);

        if(!setup_pkt_ptr(pid, packet_bytes, &debug_pkt_ptrs))
        {
            oncli_send_msg(" is invalid\n");
        }
        else
        {
            if(get_encoded_packet_len(pid, TRUE) != num_bytes)
            {
                oncli_send_msg(" -- Invalid number of bytes for pid %02X, "
                  "Expected=%d, Actual=%d\n\n", pid,
                  get_encoded_packet_len(pid, TRUE), num_bytes);
                return;
            }
            
            oncli_send_msg("\n");
            oncli_send_msg("Enc. Msg ID : 0x%02X", *(debug_pkt_ptrs.enc_msg_id));
            #if _DEBUG_VERBOSE_LEVEL > 1
            {
                oncli_send_msg(" -- Decoded : ");
                if(on_decode(&debug_pkt_ptrs.msg_id, debug_pkt_ptrs.enc_msg_id,
                  ONE_NET_ENCODED_MSG_ID_LEN) != ONS_SUCCESS)
                {
                    oncli_send_msg("Not decodable");
                }
                else
                {
                    oncli_send_msg("0x%02X", debug_pkt_ptrs.msg_id >> 2);
                }
            }
            #else
            oncli_send_msg("\n");
            #endif
            
            oncli_send_msg("\n");
            oncli_send_msg("Enc. Msg CRC : 0x%02X", *(debug_pkt_ptrs.enc_msg_crc));
            #if _DEBUG_VERBOSE_LEVEL > 1
            {
                UInt8 calculated_msg_crc;

                oncli_send_msg(" -- Decoded Msg. CRC : ");
                if(on_decode(&debug_pkt_ptrs.msg_crc, debug_pkt_ptrs.enc_msg_crc,
                  ONE_NET_ENCODED_MSG_ID_LEN) != ONS_SUCCESS)
                {
                    oncli_send_msg("Not decodable\n");
                }
                else
                {
                    oncli_send_msg("0x%02X\n", debug_pkt_ptrs.msg_crc);
                }
                
                calculated_msg_crc = calculate_msg_crc(&debug_pkt_ptrs);
                oncli_send_msg("Calculated Raw Msg CRC : 0x%02X\n", calculated_msg_crc);
                
                if(calculated_msg_crc != debug_pkt_ptrs.msg_crc)
                {
                    oncli_send_msg("Calc. msg. CRC does not match packet "
                      "CRC!\n");
                } 
            }
            #else
            oncli_send_msg("\n");
            #endif
            
            debug_display_did("Repeater DID",
              debug_pkt_ptrs.enc_repeater_did);
            debug_display_did("Dest. DID",
              debug_pkt_ptrs.enc_dst_did);
            debug_display_nid("NID",
              debug_pkt_ptrs.enc_nid);
            debug_display_did("Source DID",
              debug_pkt_ptrs.enc_src_did);
              
            oncli_send_msg("Encoded Payload Length : %d\n",
              debug_pkt_ptrs.payload_len);
            for(i = 0; i < debug_pkt_ptrs.payload_len; i++)
            {
                oncli_send_msg("%02X ", debug_pkt_ptrs.payload[i]);
    
                if((i % 16) == 15)
                {
                    oncli_send_msg("\n");
                } // if need to output a new line //
            } // loop to output the bytes that were read //
            
            #if _DEBUG_VERBOSE_LEVEL > 1
            {
                UInt8 raw_pld_len = get_raw_payload_len(pid);
                oncli_send_msg("\nDecoded Payload (# of Bytes = %d)\n",
                  raw_pld_len);
                if(on_decode(raw_payload_bytes, debug_pkt_ptrs.payload,
                  debug_pkt_ptrs.payload_len) != ONS_SUCCESS)
                {
                    oncli_send_msg("Not Decodable\n\n");
                    return;
                }
                
                for(i = 0; i < raw_pld_len; i++)
                {
                    oncli_send_msg("%02X ", raw_payload_bytes[i]);
    
                    if((i % 16) == 15)
                    {
                        oncli_send_msg("\n");
                    } // if need to output a new line //
                } // loop to output the raw payload //
                oncli_send_msg("\n");
                
                {
                    UInt8 decrypted[ON_MAX_RAW_PLD_LEN];
                    UInt8 j;
                    UInt8* encrypted = (UInt8*) raw_payload_bytes;
                    on_data_t data_type;
                    
                    UInt8 num_keys;
                    const one_net_xtea_key_t* keys;
                    
                    if(packet_is_invite(pid) && num_invite_keys > 0)
                    {
                        num_keys = num_invite_keys;
                        keys = (const one_net_xtea_key_t*) &invite_keys[0];
                        data_type = ON_INVITE;
                    }
                    #ifdef _STREAM_MESSAGES_ENABLED
                    else if(packet_is_stream(pid) && num_stream_keys > 0)
                    {
                        num_keys = num_stream_keys;
                        keys = (const one_net_xtea_key_t*)
                          &stream_keys[0];
                        data_type = ON_STREAM;
                    }
                    #endif
                    else if(num_enc_keys > 0)
                    {
                        num_keys = num_enc_keys;
                        keys = (const one_net_xtea_key_t*)
                          &enc_keys[0];
                        data_type = ON_SINGLE;                        
                    }
                    else
                    {
                        num_keys = 0;
                    }
                    
                    for(j = 0; j < num_keys; j++)
                    {
                        UInt8 calc_payload_crc;
                        BOOL crc_match;
                        oncli_send_msg("Decrypted using key ");
                        oncli_print_xtea_key(&keys[j]);
                        oncli_send_msg("\n");
                        one_net_memmove(decrypted, encrypted, raw_pld_len);
                        
                        if(on_decrypt(data_type, decrypted,
                          (one_net_xtea_key_t*)keys[j], raw_pld_len) !=
                          ONS_SUCCESS)
                        {
                            oncli_send_msg("Not decryptable\n");
                            break;
                        }
                        
                        for(i = 0; i < raw_pld_len -  1; i++)
                        {
                            oncli_send_msg("%02X ", decrypted[i]);
    
                            if((i % 16) == 15)
                            {
                                oncli_send_msg("\n");
                            } // if need to output a new line //
                        } // loop to output the raw payload //
                        oncli_send_msg("\n");

                        calc_payload_crc = (UInt8)one_net_compute_crc(
                          &decrypted[ON_PLD_CRC_SIZE], raw_pld_len - 1 -
                          ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);

                        crc_match = (calc_payload_crc == decrypted[0]);
                        oncli_send_msg("Payload CRC = 0x%02X, Calculated "
                          "Payload CRC = 0x%02X, CRCs%s match.\n",
                          decrypted[0], calc_payload_crc, crc_match ? "" : " do not");
                        #ifdef _ONE_NET_MULTI_HOP
                        {
                            if(packet_is_multihop(pid))
                            {
                                oncli_send_msg("Encoded Hops Field : %02X  ",
                                  *(debug_pkt_ptrs.enc_hops_field));
                                if(on_parse_hops(
                                  *(debug_pkt_ptrs.enc_hops_field),
                                  &(debug_pkt_ptrs.hops),
                                  &(debug_pkt_ptrs.max_hops)) != ONS_SUCCESS)
                                {
                                    oncli_send_msg("Not Decodable\n");
                                }
                                else
                                {
                                    oncli_send_msg("Hops : %d Max Hops : %d\n",
                                      debug_pkt_ptrs.hops,
                                      debug_pkt_ptrs.max_hops);
                                }
                            }
                        }
                        #endif
                    }
                }
            }
            #endif
        }
    }
    #endif
}


/*!
    \brief The packet sniffer evaluation application

    This is the main function for the packet sniffer.

    \param void

    \return void
*/
void sniff_eval(void)
{
    UInt8 pkt[ON_MAX_ENCODED_PKT_SIZE];
    UInt8* pkt_wo_header = &pkt[ONE_NET_ENCODED_RPTR_DID_IDX];
    UInt16 bytes_read = sizeof(pkt);
    
    one_net_memmove(pkt, HEADER, ONE_NET_PREAMBLE_HEADER_LEN);

    if(oncli_user_input())
    {
        ont_set_timer(USER_INPUT_TIMER, MS_TO_TICK(USER_INPUT_PAUSE_TIME));
        return;
    } // if there has been user input //

    if(ont_active(USER_INPUT_TIMER))
    {
        if(ont_expired(USER_INPUT_TIMER))
        {
            ont_stop_timer(USER_INPUT_TIMER);
        } // if the user input timer has expired //
        else
        {
            return;
        } // else the user input timer has not expired //
    } // if there had been user input //


    if(one_net_look_for_pkt(MS_TO_TICK(ONE_NET_WAIT_FOR_SOF_TIME))
      != ONS_SUCCESS)
    {
        if(ont_active(PROMPT_TIMER) && ont_expired(PROMPT_TIMER))
        {
            ont_stop_timer(PROMPT_TIMER);
            oncli_print_prompt();
        } // if the prompt needs to be displayed //

        return;
    } // if SOF was not received //

    bytes_read = one_net_read(pkt_wo_header, bytes_read);
    
    #ifdef _RANGE_TESTING
    if(!device_in_range((on_encoded_did_t*)
      &(pkt[ONE_NET_ENCODED_RPTR_DID_IDX])))
    {
        // we are filtering the packets.  Ignore this one.
        return;
    }
    #endif
    
    
    oncli_send_msg("%lu received %u bytes:\n", get_tick_count(), bytes_read +
      ONE_NET_PREAMBLE_HEADER_LEN);
    
    #if _DEBUG_VERBOSE_LEVEL > 1
    display_pkt(pkt, bytes_read + ONE_NET_PREAMBLE_HEADER_LEN
      , sniff_enc_keys, NUM_SNIFF_ENCRYPT_KEYS
      , sniff_invite_keys, NUM_SNIFF_INVITE_KEYS
      #ifdef _STREAM_MESSAGES_ENABLED
      , sniff_stream_keys, NUM_SNIFF_STREM_ENCRYPT_KEYS);
      #else
      , NULL, 0);
      #endif
    #else
    display_pkt(pkt, bytes_read + ONE_NET_PREAMBLE_HEADER_LEN);
    #endif

    oncli_send_msg("\n\n\n");

    // update the time to display the prompt
    ont_set_timer(PROMPT_TIMER, PROMPT_PERIOD);
} // sniff_eval //



//! @} ONE-NET_sniff_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_sniff_eval
//! @{



//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

#endif

//! @} ONE_NET_sniff_eval
