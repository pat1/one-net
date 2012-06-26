//! \defgroup ONE-NET_sniff_eval ONE-NET Packet Sniffer
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file sniff_eval.c
    \brief The packet sniffer part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#ifdef SNIFFER_MODE

#if DEBUG_VERBOSE_LEVEL == 0
    #error "DEBUG_VERBOSE_LEVEL must be greater than 0 if SNIFFER_MODE is enabled"
#endif



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

#if DEBUG_VERBOSE_LEVEL > 2
enum
{
    //! number of known invite keys to try for decryption.
    NUM_SNIFF_INVITE_KEYS = 2,
    
    //! number of known encryption keys to try for decryption.
    NUM_SNIFF_ENCRYPT_KEYS = 2,
};
#endif


static tick_t sniff_start_time = 0;
static tick_t sniff_duration_ms = 0;



//! @} ONE-NET_sniff_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

#if DEBUG_VERBOSE_LEVEL > 2
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
     0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F},
     
    // use with "change key:44-44-44-44" command
    {0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
     0x0C,0x0D,0x0E,0x0F,0x44,0x44,0x44,0x44}
};
#endif



//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_sniff_eval_pub_func
//! \ingroup ONE-NET_sniff_eval
//! @{
    


// If sniff_time_ms is non-zero, timestamps will start at 0 and packets will
// only be siffed for a certain time period.  For example, if sniff_time_ms
// is equal to 3500, the first packet sniffed will be given a timestamp of 0.
// Any packets received more than 3.5 seconds after the first packet will not
// be displayed.
oncli_status_t oncli_reset_sniff(const UInt8 CHANNEL, tick_t sniff_time_ms)
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
    
    sniff_duration_ms = sniff_time_ms;
    sniff_start_time = 0;
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
    tick_t packet_time_ms;
    UInt8 pkt[ON_MAX_ENCODED_PKT_SIZE];
    UInt8* pkt_wo_header = &pkt[ON_ENCODED_RPTR_DID_IDX];
    UInt16 bytes_read = sizeof(pkt);
    
    one_net_memmove(pkt, HEADER, ONE_NET_PREAMBLE_HEADER_LEN);

    if(oncli_user_input())
    {
        ont_set_timer(USER_INPUT_TIMER, MS_TO_TICK(
          SNIFF_USER_INPUT_PAUSE_TIME));
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
    
    #ifdef RANGE_TESTING
    if(!device_in_range((on_encoded_did_t*)
      &(pkt[ON_ENCODED_RPTR_DID_IDX])))
    {
        // we are filtering the packets.  Ignore this one.
        return;
    }
    #endif
    
    packet_time_ms = TICK_TO_MS(get_tick_count());
    if(sniff_duration_ms)
    {
        if(sniff_start_time == 0)
        {
            sniff_start_time = packet_time_ms;
        }

        packet_time_ms -= sniff_start_time;
        if(packet_time_ms > sniff_duration_ms)
        {
            return;
        }
    }      
    
    oncli_send_msg("\n\n%lu received %u bytes:\n", packet_time_ms, bytes_read +
      ONE_NET_PREAMBLE_HEADER_LEN);
    
    #if DEBUG_VERBOSE_LEVEL > 2
    display_pkt(pkt, bytes_read + ONE_NET_PREAMBLE_HEADER_LEN
      , sniff_enc_keys, NUM_SNIFF_ENCRYPT_KEYS
      , sniff_invite_keys, NUM_SNIFF_INVITE_KEYS);
    #else
    display_pkt(pkt, bytes_read + ONE_NET_PREAMBLE_HEADER_LEN);
    #endif

    oncli_send_msg("\n\n");

    // update the time to display the prompt
    ont_set_timer(PROMPT_TIMER, SNIFF_PROMPT_PERIOD);
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
