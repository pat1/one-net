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
#include "oncli_str.h"
#include "one_net_timer.h"
#include "tick.h"
#include "one_net_port_specific.h"
#include "one_net_eval.h"
#include "hal.h"
#include "tal.h"
#include "oncli.h"



//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_sniff_eval_const
//! \ingroup ONE-NET_sniff_eval
//! @{

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


//! @} ONE-NET_sniff_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



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

    UInt16 i, bytes_read = sizeof(pkt);

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

    bytes_read = one_net_read(pkt, bytes_read);

    oncli_send_msg("%lu received %u bytes:\r\n", get_tick_count(), bytes_read);
    for(i = 0; i < bytes_read; i++)
    {
        oncli_send_msg("%02X ", pkt[i]);
    
        if((i % 16) == 15)
        {
            oncli_send_msg("\r\n");
        } // if need to output a new line //
    } // loop to output the bytes that were read //

    if((i % 16) != 15)
    {
        oncli_send_msg("\n");
    } // if a new line needs to be output //

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
