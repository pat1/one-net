//! \addtogroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.c
    \brief The ONE-NET evaluation project.

    This is the application that runs on the ONE-NET evaluation boards.
*/



#include "config_options.h"
#include "oncli_port.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "tick.h"
#include "pal.h"
#include "hal.h"
#include "tal.h"
#include "uart.h"
#include "io_port_mapping.h"
#include "oncli.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "oncli_str.h"
#include "one_net.h"
#include "one_net_eval.h"
#include "one_net_prand.h"
#ifdef _HAS_LEDS
    #include "one_net_led.h"
#endif


#include "one_net_crc.h" // for displaying packets



//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{



//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{

                                     

//! Master prompt
static const char* const master_prompt = "-m";

//! Client prompt
static const char* const client_prompt = "-c";


#ifdef _SNIFFER_MODE
//! Sniffer prompt
static const char* const sniffer_prompt = "-s";
#endif


#if _DEBUG_VERBOSE_LEVEL > 0
extern const char HEX_DIGIT[]; // for displaying packets
#endif


//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{


#ifdef _SNIFFER_MODE
//! True if in Sniffer Mode
BOOL in_sniffer_mode = FALSE;
#endif

//! Pointer to the device dependent (MASTER, CLIENT, SNIFF) function that
//! should be called in the main loop
void(*node_loop_func)(void) = 0;


	
//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================


//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_eval_pri_var
//! \ingroup ONE-NET_eval
//! @{
  


//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



#ifdef _UART
static const char* get_prompt_string(void);
#endif



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



#ifdef _UART
void oncli_print_prompt(void)
{   
    oncli_send_msg("ocm%s> ", get_prompt_string());
} // oncli_print_prompt //
#endif


int main(void)
{
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR(TRUE);

    #ifdef _HAS_LEDS
        initialize_leds();
    #endif
    
    INIT_TICK();
    #ifdef _NON_VOLATILE_MEMORY
    FLASH_ERASE_CHECK();
    #endif

    #ifdef _UART
        #ifdef _DEFAULT_BAUD_RATE
            #if _DEFAULT_BAUD_RATE == 115200
                uart_init(BAUD_115200, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
            #else
                uart_init(BAUD_38400, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
            #endif
        #else
            uart_init(BAUD_38400, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
        #endif
    #endif
    ENABLE_GLOBAL_INTERRUPTS();
    
    #ifdef _UART
    // startup greeting
    oncli_send_msg("\n\n");
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);   
    oncli_send_msg("\n\n");
    #endif

    oncli_reset_sniff(DEFAULT_EVAL_CHANNEL, 0);    

    #ifdef _UART 
    oncli_print_prompt();
    #endif   
    while(1)
    {
        (*node_loop_func)();
        #ifdef _UART
        oncli();
        #endif
    }

    EXIT();
	return 0;
} // main //


#ifndef _ONE_NET_MULTI_HOP
on_message_status_t eval_handle_single(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack)
#else
on_message_status_t eval_handle_single(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops)
#endif
{
    return ON_MSG_CONTINUE;
}


#ifndef _ONE_NET_MULTI_HOP
on_message_status_t eval_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries)
#else
on_message_status_t eval_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries,
  UInt8 hops, UInt8* const max_hops)
#endif
{
    return ON_MSG_CONTINUE;
}


#ifdef _ONE_NET_MULTI_HOP
on_message_status_t one_net_adjust_hops(const on_raw_did_t* const raw_dst,
  UInt8* const max_hops)
{
    return ON_MSG_DEFAULT_BHVR;
}
#endif


#ifndef _ONE_NET_MULTI_HOP
void eval_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack)
#else
void eval_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack, SInt8 hops)
#endif
{
}



// Packet Display Funcitonality
// TODO -- Should these functions be in oncli.c instead of here?
#if _DEBUG_VERBOSE_LEVEL > 0

#if _DEBUG_VERBOSE_LEVEL > 1
/*!
    \brief Displays a DID in verbose fashion.

    \param[in] description The description to prepend in front of the DID
    \param[in] enc_did The encioded DID to display.
    
    \return void
*/
void debug_display_did(const char* const description,
  const on_encoded_did_t* const enc_did)
{
    #if _DEBUG_VERBOSE_LEVEL > 2
    on_raw_did_t raw_did;
    BOOL valid = (on_decode(raw_did, *enc_did, ON_ENCODED_DID_LEN) ==
      ONS_SUCCESS);
    #endif
    
    oncli_send_msg("%s : 0x%02X%02X", description, (*enc_did)[0], (*enc_did)[1]);
    #if _DEBUG_VERBOSE_LEVEL > 2
    if(verbose_level > 2)
    {
        if(valid)
        {
            oncli_send_msg(" -- Decoded : 0x%03X", did_to_u16(&raw_did));
        }
        else
        {
            oncli_send_msg(" -- Invalid");
        }
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
    #if _DEBUG_VERBOSE_LEVEL > 2
    on_raw_nid_t raw_nid;
    BOOL valid = (on_decode(raw_nid, *enc_nid, ON_ENCODED_NID_LEN) ==
      ONS_SUCCESS);
    #endif
    
    oncli_send_msg("%s : 0x", description);
    uart_write_int8_hex_array(*enc_nid, FALSE, ON_ENCODED_NID_LEN);
    #if _DEBUG_VERBOSE_LEVEL > 2
    if(verbose_level > 2)
    {
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
    }
    #endif
    oncli_send_msg("\n");
}
#endif
#endif




//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



#ifdef _UART
/*!
    \brief returns the string to use as part of the Command-line-interface
           prompt
        
    \return string to use as part of the Command-line-interface prompt
*/
static const char* get_prompt_string(void)
{
    #ifdef _SNIFFER_MODE
    if(in_sniffer_mode)
    {
        return sniffer_prompt;
    }
    #endif
    
    #ifdef _ONE_NET_MASTER
        #ifdef _ONE_NET_CLIENT
        if(device_is_master)
        {
            return master_prompt;
        }
        #else
        return master_prompt;
        #endif
    #endif
    
    return client_prompt;
}
#endif


/*!
    \brief Sends a switch command message when a switch is flipped.
    
    \param[in] src_unit The source unit 
    \param[in] status The status of the pin
    \param[in] dst_unit The destination unit
    \param[in] enc_dst The device that is to receive this message.
    
    \return ONS_SUCCESS if the message was successfully queued.
            ONS_RSRC_FULL otherwise
*/
one_net_status_t send_switch_status_change_msg(UInt8 src_unit, 
  UInt8 status, UInt8 dst_unit, const on_encoded_did_t* const enc_dst)
{
    return ONS_SUCCESS;
} // send_switch_status_change_msg //



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
