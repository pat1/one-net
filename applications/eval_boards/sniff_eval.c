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
#include "oncli_port.h"
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
