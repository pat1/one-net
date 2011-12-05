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
#include "nv_hal.h"
#include "uart.h"
#include "io_port_mapping.h"
#include "oncli.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "io_port_mapping.h"
#include "oncli_str.h"
#include "one_net.h"
#include "one_net_eval.h"
#include "one_net_prand.h"
#ifdef _HAS_LEDS
    #include "one_net_led.h"
#endif



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



#ifdef _AUTO_MODE
//! The raw CLIENT DIDs for auto mode
const on_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] =
{
    {0x00, 0x20}, {0x00, 0x30}, {0x00, 0x40}
};

//! The encoded CLIENT DIDs for auto mode
const on_encoded_did_t ENC_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] =
{
    {0xB4, 0xB3}, {0xB4, 0xBA}, {0xB4, 0xB5}
};
#endif


#ifdef _AUTO_MODE
//! The default keep alive for Eval Boards
const tick_t DEFAULT_EVAL_KEEP_ALIVE_MS = 1800000;
#endif

//! The key used in the evaluation network
const one_net_xtea_key_t EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

#ifdef _STREAM_MESSAGES_ENABLED
//! The key to use for stream transactions in the eval network
const one_net_xtea_key_t EVAL_STREAM_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
#endif

//! Default invite key to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_INVITE_KEY[] = { '2', '2', '2', '2',   '2', '2', '2', '2',
                                     '2', '2', '2', '2',   '2', '2', '2', '2'};
                                     
#if defined(_AUTO_MODE) || defined(_ONE_NET_MASTER)
//! Default NID to use if no NID is found in the manufacturing data segment
//! of data flash.
const UInt8 DEFAULT_RAW_NID[] =        {0x00, 0x00, 0x00, 0x00, 0x10};

//! Default SID to use if no NID is found in the manufacturing data segment
//! of data flash.
const on_raw_sid_t DEFAULT_RAW_SID = {0x00, 0x00, 0x00, 0x00, 0x10, 0x01};
#endif


//! Master prompt
static const char* const master_prompt = "-m";

//! Client prompt
static const char* const client_prompt = "-c";

#ifdef _AUTO_MODE
//! Auto Client prompts
static const char* const auto_client_prompts[] = {"-c1", "-c2", "-c3"};
#endif

#ifdef _SNIFFER_MODE
//! Sniffer prompt
static const char* const sniffer_prompt = "-s";
#endif



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{


#ifdef _AUTO_MODE
//! True if in Auto Mode
BOOL in_auto_mode = FALSE;

//! If in auto mode and a client, the index number of the client
UInt8 auto_client_index;
#endif

//! the pins on the eval board.
user_pin_t user_pin[NUM_USER_PINS];

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



static const char* get_prompt_string(void);
static void eval_set_modes_from_switch_positions(void);



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{


/*!
    \brief Initializes the parameters used with the user pins.
    
    \param[in] USER_PIN_TYPE List containing the state of the user pins.  If
      this is 0, then the default configuration will be used.
    \param[in] USER_PIN_COUNT The number of pins to configure.  This should be
      equal to the number of user pins, or else the default configuration will
      be used.
    
    \return void
*/
void init_user_pin(const UInt8 *user_pin_type,
  UInt8 user_pin_count)
{
} // init_user_pin //


void oncli_print_prompt(void)
{   
    oncli_send_msg("ocm%s> ", get_prompt_string());
} // oncli_print_prompt //


int main(void)
{
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR(TRUE);

    #ifdef _HAS_LEDS
        initialize_leds();
    #endif
    
    INIT_TICK();
    FLASH_ERASE_CHECK();

    uart_init(/*BAUD_38400*/BAUD_115200, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
    disable_user_pins();
    ENABLE_GLOBAL_INTERRUPTS();
    
    // startup greeting
    oncli_send_msg("\n\n");
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);   
    oncli_send_msg("\n\n");

    eval_set_modes_from_switch_positions();
    
    // do some initialization here.  It may get written over later, but
    // that's OK.
    on_encode(on_base_param->sid, DEFAULT_RAW_NID, ON_ENCODED_NID_LEN);
    one_net_memmove(on_base_param->current_key, EVAL_KEY,
      ONE_NET_XTEA_KEY_LEN);
    #ifdef _ONE_NET_MASTER  
    one_net_memmove(&on_base_param->sid[ON_ENCODED_NID_LEN],
      MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);
    #endif
    
    on_base_param->single_block_encrypt = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;
    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    on_base_param->features = THIS_DEVICE_FEATURES;
    
    #ifdef _AUTO_MODE
    on_base_param->channel = DEFAULT_EVAL_CHANNEL;
    #endif
    
    #ifdef _BLOCK_MASSGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
    #endif

    #ifdef _STREAM_MESSAGES_ENABLED
    one_net_memmove(on_base_param->stream_key, EVAL_STREAM_KEY,
      ONE_NET_XTEA_KEY_LEN);
    on_base_param->stream_encrypt = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32;
    #endif
    
    
#ifdef _AUTO_MODE
	if(in_auto_mode)
	{
        #ifdef _ONE_NET_MASTER
        if(device_is_master)
        {
            init_auto_master();
        }
        #endif
        #ifdef _ONE_NET_CLIENT
        if(!device_is_master)
        {
            init_auto_client(auto_client_index);
        }
        #endif
        
		oncli_send_msg("%s\n", ONCLI_AUTO_MODE_STR);
	} // if auto mode //
	else
	{
        #ifdef _ONE_NET_MASTER
        if(device_is_master)
        {
            init_serial_master();
        }
        #endif
        #ifdef _ONE_NET_CLIENT
        if(!device_is_master)
        {
            init_serial_client();
        }
        #endif
   		oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
	} // else serial //
#else
    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        init_serial_master();
    }
    #endif
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        init_serial_client();
    }
    #endif
	oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
#endif
 
    oncli_print_prompt();    
    while(1)
    {
        (*node_loop_func)();
        oncli();
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
    UInt8 src_unit, dst_unit;
    ona_msg_class_t msg_class;
    UInt16 msg_type, msg_data;

    #ifndef _ONE_NET_MULTI_HOP
    if(!raw_pld || !msg_hdr || !src_did || !repeater_did || !ack_nack ||
      !ack_nack->payload || ack_nack->payload !=
      (ack_nack_payload_t*) raw_pld)
    #else
    if(!raw_pld || !msg_hdr || !src_did || !repeater_did || !ack_nack ||
      !ack_nack->payload || ack_nack->payload !=
      (ack_nack_payload_t*) raw_pld || !max_hops)
    #endif
    {
        return ON_MSG_INTERNAL_ERR;
    }
    
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;
    
    if(msg_hdr->msg_type != ON_APP_MSG)
    {
        return ON_MSG_IGNORE;
    }
    
    on_parse_app_pld(raw_pld, &src_unit, &dst_unit, &msg_class, &msg_type,
      &msg_data);

    oncli_send_msg("eval_hdl_sng: ");
    print_app_payload(raw_pld, 5);
    oncli_send_msg("\n");

    if(dst_unit >= ONE_NET_NUM_UNITS && dst_unit != ONE_NET_DEV_UNIT)
    {
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_UNIT_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_type != ONA_SWITCH)
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_data != ONA_ON && msg_data != ONA_OFF && msg_data != ONA_TOGGLE)
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_class == ONA_COMMAND)
    {
        if(dst_unit == ONE_NET_DEV_UNIT)
        {
            ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
        }
        
        if (user_pin[dst_unit].pin_type != ON_OUTPUT_PIN)
        {
            // we'll use a user-defined fatal error for the reason            
            ack_nack->nack_reason = ON_NACK_RSN_MIN_USR_FATAL;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
        }
        
		switch(dst_unit)
		{
			case 0: USER_PIN0 = (msg_data == ONA_TOGGLE) ? !USER_PIN0 : ((msg_data == ONA_ON) ? 1 : 0); msg_data = USER_PIN0; break;
			case 1: USER_PIN1 = (msg_data == ONA_TOGGLE) ? !USER_PIN1 : ((msg_data == ONA_ON) ? 1 : 0); msg_data = USER_PIN1; break;
			case 2: USER_PIN2 = (msg_data == ONA_TOGGLE) ? !USER_PIN2 : ((msg_data == ONA_ON) ? 1 : 0); msg_data = USER_PIN2; break;
			case 3: USER_PIN3 = (msg_data == ONA_TOGGLE) ? !USER_PIN3 : ((msg_data == ONA_ON) ? 1 : 0); msg_data = USER_PIN3; break;
            default:
                oncli_send_msg("Invalid dest. unit for COMMAND\n");
                ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
                ack_nack->handle = ON_NACK;
                return ON_MSG_CONTINUE;
		}
        
        ack_nack->handle = ON_ACK_STATUS;
        
        // source and destination are reversed in the response.
        put_src_unit(dst_unit, ack_nack->payload->status_resp);
        put_dst_unit(src_unit, ack_nack->payload->status_resp);
        put_msg_class(ONA_STATUS_COMMAND_RESP, ack_nack->payload->status_resp);
        
        // we don't need to fill in the type.  It's already there since this is the same
        // memory as the raw payload!
        
        put_msg_data(msg_data, ack_nack->payload->status_resp);
        return ON_MSG_CONTINUE;
    }
    
    // fill in poll and query and status later.  For now, NACK it.
    oncli_send_msg("Currently only handling commands.\n", dst_unit);
    // we'll use a user-defined fatal error for the reason            
    ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
    ack_nack->handle = ON_NACK;
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
    #if 0
    // just to prove we can pause and change the response timeout.
    if(resp_ack_nack->nack_reason == ON_NACK_RSN_NO_RESPONSE)
    {
        if(one_net_prand(get_tick_count(), 2))
        {
            resp_ack_nack->handle = ON_NACK_PAUSE_TIME_MS;
            resp_ack_nack->payload->nack_time_ms =
              one_net_prand(get_tick_count(), 250);
        }
        else
        {
            resp_ack_nack->handle = ON_NACK_SLOW_DOWN_TIME_MS;
            resp_ack_nack->payload->nack_time_ms =
              one_net_prand(get_tick_count(), 250);
        }
    }
    #endif
    return ON_MSG_CONTINUE;
}


/*!
    \brief disables the user pins

    \param void

    \return void
*/
void disable_user_pins(void)
{
    UInt8 i;
    
    for(i = 0; i < NUM_USER_PINS; i++)
    {
        user_pin[i].pin_type = ON_DISABLE_PIN;
    } // loop to clear user pins //
} // disable_user_pins //


oncli_status_t oncli_set_user_pin_type(UInt8 pin, on_pin_state_t pin_type)
{
    if(pin_type > ON_DISABLE_PIN)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //

    switch(pin)
    {
        case 0:
        {
            user_pin[0].old_state = USER_PIN0;
            USER_PIN0_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
              : pin_type;
            break;
        } // pin 0 case //

        case 1:
        {
            user_pin[1].old_state = USER_PIN1;
            USER_PIN1_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
              : pin_type;
            break;
        } // pin 1 case //

        case 2:
        {
            user_pin[2].old_state = USER_PIN2;
            USER_PIN2_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
              : pin_type;
            break;
        } // pin 2 case //

        case 3:
        {
            user_pin[3].old_state = USER_PIN3;
            USER_PIN3_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
              : pin_type;
            break;
        } // pin 3 case //

        default:
        {
            return ONCLI_BAD_PARAM;
            break;
        } // default case //
    } // switch(PIN) //

    user_pin[pin].pin_type = pin_type;
    return ONCLI_SUCCESS;
} // oncli_set_user_pin_type //


void oncli_print_user_pin_cfg(void)
{
    UInt8 i;
    UInt8 state;
    const UInt8 * type_string;

    oncli_send_msg("User pins:\n");
    for(i = 0; i < NUM_USER_PINS; i++)
    {
        //
        // collect the state of the user pin, so we can print it
        //
        switch (i)
        {
            case 0:
            {
                state = USER_PIN0;
                break;
            }

            case 1:
            {
                state = USER_PIN1;
                break;
            }

            case 2:
            {
                state = USER_PIN2;
                break;
            }

            case 3:
            {
                state = USER_PIN3;
                break;
            }

            default:
            {
                state = 3;
            }
        }
        type_string = ONCLI_DISABLE_STR;
        if (user_pin[i].pin_type == ON_INPUT_PIN)
        {
            type_string = ONCLI_INPUT_STR;
            oncli_send_msg("  %d %s state: %d\n", i, type_string, state); 
        }
        else if (user_pin[i].pin_type == ON_OUTPUT_PIN)
        {
            type_string = ONCLI_OUTPUT_STR;
            oncli_send_msg("  %d %s state: %d\n", i, type_string, state); 
        }
        else
        {
            oncli_send_msg("  %d %s\n", i, type_string); 
        }
    }
} // oncli_print_user_pin_cfg //


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
    if(!dst)
    {
        return;
    } // if the parameters are invalid //
    
    #ifdef _VERBOSE
    oncli_send_msg("ests start:");
    oncli_send_msg("message:%02X%02X%02X%02X%02X\n",
      data[0], data[1], data[2], data[3], data[4]);
    oncli_send_msg("Hdr-->");
    print_msg_hdr(&msg_hdr);
    #ifndef _ONE_NET_MULTI_HOP
    oncli_send_msg("retry=%d,dst=%02X%02X\nack_nack-->", retry_count,
      (*dst)[0], (*dst)[1]);
    #else
    oncli_send_msg("retry=%d,hops=%d,dst=%02X%02X\nack_nack-->", retry_count,
      hops, (*dst)[0], (*dst)[1]);
    #endif

    print_ack_nack(ack_nack, get_raw_payload_len(msg_hdr.pid) -  1 -
      ON_PLD_DATA_IDX);
    #endif

    oncli_send_msg(ONCLI_SINGLE_RESULT_FMT, did_to_u16(dst),
      oncli_msg_status_str(status));

    #ifdef _VERBOSE
    oncli_send_msg("ests end\n");
    #endif
}



//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



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
    
    #ifndef _AUTO_MODE
    return client_prompt;
    #else
        if(!in_auto_mode)
        {
            return client_prompt;
        }
        
        return auto_client_prompts[auto_client_index];
    #endif
}


/*!
    \brief Sends a switch command message when a switch is flipped.
    
    \param[in] src_unit The source unit 
    \param[in] status The status of the pin
    \param[in] dst_unit The destination unit
    \param[in] enc_dst The device that is to receive this message.
    
    \return ONS_SUCCESS if the message was successfully queued.
            ONS_INTERNAL_ERR If the send single function could not be retrieved.
            See one_net_client_send_single & one_net_master_send_single for
            more return values.
*/
one_net_status_t send_switch_status_change_msg(UInt8 src_unit, 
  UInt8 status, UInt8 dst_unit, const on_encoded_did_t* const enc_dst)
{
    // source is this device
    on_encoded_did_t* src_did = (on_encoded_did_t*)
      (&(on_base_param->sid[ON_ENCODED_NID_LEN]));    
    UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];

    put_src_unit(src_unit, raw_pld);
    put_msg_hdr(ONA_STATUS_CHANGE | ONA_SWITCH, raw_pld);
    put_msg_data(status, raw_pld);
    put_dst_unit(dst_unit, raw_pld);

    return (*one_net_send_single)(ONE_NET_ENCODED_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, src_did, enc_dst
      #ifdef _PEER
          , TRUE, src_unit
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , NULL
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , NULL
      #endif
      );    
} // send_switch_status_change_msg //


/*!
    \brief Checks the three switches to see whether the device boots in
        auto mode, whether the device is a master or a client, and,
        if in auto mode and a client, which client the device should
        be assigned.
        
    \return none
*/
static void eval_set_modes_from_switch_positions(void)
{
    #ifdef _ONE_NET_CLIENT
        // note : this includes devices which are both master and clients
        // If that is the case and we are in master mode, it will be set
        // below
        device_is_master = FALSE;
        node_loop_func = &client_eval;
    #else
        device_is_master = TRUE;
        node_loop_func = &master_eval;    
    #endif
    
    #ifdef _AUTO_MODE
	// check mode switch (Auto/Serial)
	if(SW_MODE_SELECT == 0)
	{
		in_auto_mode = TRUE;
    }
    #endif

    #ifdef _ONE_NET_MASTER
    if((SW_ADDR_SELECT1 == 0) && (SW_ADDR_SELECT2 == 0))  
    {
        device_is_master = TRUE;
        node_loop_func = &master_eval;
    }
    #endif
    
    #ifdef _AUTO_MODE
    if(!device_is_master && in_auto_mode)
    {
        if((SW_ADDR_SELECT1 == 1) && (SW_ADDR_SELECT2 == 0))
        {
            auto_client_index = 0;
        }
        else if((SW_ADDR_SELECT1 == 0) && (SW_ADDR_SELECT2 == 1))        
        {
            auto_client_index = 1;
        }
        else       
        {
            auto_client_index = 2;
        }
    }
    #endif
}


#ifdef _AUTO_MODE
/*!
    \brief Sends a simple text command message.
    
    \param[in] text Pointer to the two characters to send.
    \param[in] src_unit The unit in this device the message pertains to.
    \param[in] dst_unit The device that is to receive this message.
    \param[in] enc_dst The device that is to receive this message.
    
    \return ONS_SUCCESS if the message was successfully queued.
            ONS_INTERNAL_ERR If the send single function could not be retrieved.
            See one_net_client_send_single & one_net_master_send_single for
            more return values.
*/
one_net_status_t send_simple_text_command(const char* text, UInt8 src_unit, 
  UInt8 dst_unit, const on_encoded_did_t* const enc_dst)
{
    // source is this device
    on_encoded_did_t* src_did = (on_encoded_did_t*)
      (&(on_base_param->sid[ON_ENCODED_NID_LEN]));    
    UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];

    put_src_unit(src_unit, raw_pld);
    put_dst_unit(dst_unit, raw_pld);
    put_msg_hdr(ONA_COMMAND | ONA_SIMPLE_TEXT, raw_pld);    
    // we won't use the put_msg_data function here due to
    // endianness.  Instead use one_net_memmove
    one_net_memmove(&raw_pld[ONA_MSG_DATA_IDX], text, ONA_MSG_DATA_LEN);
      
    return (*one_net_send_single)(ONE_NET_ENCODED_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, src_did, enc_dst
      #ifdef _PEER
          , FALSE, ONE_NET_DEV_UNIT
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , NULL
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , NULL
      #endif
      );    
} // send_send_simple_text_command //
#endif



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
