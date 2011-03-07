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
#include "one_net_eval.h"
#include "one_net_eval_hal.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"


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

//! @} ONE-NET_sniff_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

// forward declarations
// "derived" from one_net_eval
oncli_status_t set_device_type(UInt8 device_type);

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
    oncli_status_t status;

    if(CHANNEL > ONE_NET_MAX_CHANNEL)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if((status = set_device_type(SNIFFER_NODE)) != ONCLI_SUCCESS)
    {
        return status;
    } // if could not set the device to a CLIENT //

    ont_stop_timer(PROMPT_TIMER);
    one_net_set_channel(CHANNEL);

    return ONCLI_SUCCESS;
} // oncli_reset_sniff //

//! @} ONE-NET_sniff_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_sniff_eval_pri_func
//! \ingroup ONE-NET_sniff_eval
//! @{

/*!
    \brief The packet sniffer evaluation application
    
    This is the main function for the packet sniffer.

    \param void

    \return void
*/
void sniff_eval(void)
{
    UInt8 pkt[ONE_NET_MAX_ENCODED_PKT_LEN];

    UInt16 i, bytes_read = sizeof(pkt);

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
    if(oncli_user_input())
    {
        ont_set_timer(USER_INPUT_TIMER, USER_INPUT_PAUSE_TIME);
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
#endif

    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) != ONS_SUCCESS)
    {
        if(ont_active(PROMPT_TIMER) && ont_expired(PROMPT_TIMER))
        {
            ont_stop_timer(PROMPT_TIMER);
            oncli_print_prompt();
        } // if the prompt needs to be displayed //

        return;
    } // if SOF was not received //

    bytes_read = one_net_read(pkt, bytes_read);

    oncli_send_msg("%lu received %u bytes:\r\n", one_net_tick(), bytes_read);
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
        oncli_send_msg("\r\n");
    } // if a new line needs to be output //

    // update the time to display the prompt
    ont_set_timer(PROMPT_TIMER, PROMPT_PERIOD);
} // sniff_eval //

//! @} ONE-NET_sniff_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

#endif

//! @} ONE_NET_sniff_eval
