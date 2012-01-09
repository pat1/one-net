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
#include "oncli_str.h"
#include "one_net.h"
#include "one_net_eval.h"
#include "one_net_prand.h"
#ifdef _HAS_LEDS
    #include "one_net_led.h"
#endif


#include "one_net_crc.h" // for displaying packets
#ifdef _ONE_NET_MASTER
#include "one_net_master.h" // for keys
#endif


#ifdef _ONE_NET_CLIENT
    #include "one_net_client_port_specific.h"
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
const tick_t DEFAULT_EVAL_KEEP_ALIVE_MS = 1800000; // TODO -- replace
                           // with ONE_NET_MASTER_DEFAULT_KEEP_ALIVE?
#endif

//! The key used in the evaluation network
const one_net_xtea_key_t EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

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


//! The state of handling the user pins the device is in
UInt8 user_pin_state;

//! The source unit of the user pin that has changed
UInt8 user_pin_src_unit;


	
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


static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const on_raw_did_t *SRC_ADDR);



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



/*!
    \brief Checks to see if the state of any of the user pins changed
    
    \param void
    
    \return void
*/
void check_user_pins(void)
{
    if(user_pin[0].pin_type == ON_INPUT_PIN
      && USER_PIN0 != user_pin[0].old_state)
    {
        user_pin_state = SEND_USER_PIN_INPUT;
        user_pin_src_unit = 0;
        user_pin[0].old_state = USER_PIN0;
    } // if the user0 pin has been toggled //
    else if(user_pin[1].pin_type == ON_INPUT_PIN
      && USER_PIN1 != user_pin[1].old_state)
    {
        user_pin_state = SEND_USER_PIN_INPUT;
        user_pin_src_unit = 1;
        user_pin[1].old_state = USER_PIN1;
    } // if the user1 pin has been toggled //
    else if(user_pin[2].pin_type == ON_INPUT_PIN
      && USER_PIN2 != user_pin[2].old_state)
    {
        user_pin_state = SEND_USER_PIN_INPUT;
        user_pin_src_unit = 2;
        user_pin[2].old_state = USER_PIN2;
    } // if the user2 pin has been toggled //
    else if(user_pin[3].pin_type == ON_INPUT_PIN
      && USER_PIN3 != user_pin[3].old_state)
    {
        user_pin_state = SEND_USER_PIN_INPUT;
        user_pin_src_unit = 3;
        user_pin[3].old_state = USER_PIN3;
    } // if the user3 pin has been toggled //
} // check_user_pins //


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
    #ifdef _NON_VOLATILE_MEMORY
    FLASH_ERASE_CHECK();
    #endif

    #ifdef _UART
    uart_init(BAUD_38400/*BAUD_115200*/, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
    #endif
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

    on_base_param->data_rate = ONE_NET_DATA_RATE_38_4;
    on_base_param->features = THIS_DEVICE_FEATURES;
    
    #ifdef _AUTO_MODE
    on_base_param->channel = DEFAULT_EVAL_CHANNEL;
    #endif
    
    #ifdef _BLOCK_MASSGES_ENABLED
    on_base_param->fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
    on_base_param->fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
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

    #if _DEBUG_VERBOSE_LEVEL > 3
    oncli_send_msg("eval_hdl_sng: ");
    print_app_payload(raw_pld, 5);
    oncli_send_msg("\n");
    #endif

    if(dst_unit >= ONE_NET_NUM_UNITS && dst_unit != ONE_NET_DEV_UNIT)
    {
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_UNIT_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }
    
    
    #ifdef _EXTENDED_SINGLE
    if(msg_type == ONA_SIMPLE_TEXT || msg_type == ONA_TEXT)
    #else
    if(msg_type == ONA_SIMPLE_TEXT)
    #endif
    {
        UInt8 text_len = 2;
        
        #ifdef _EXTENDED_SINGLE
        if(msg_type == ONA_TEXT)
        {
            // this is a NULL-terminated string, so find out how long it is;
            const UInt8* ptr = &raw_pld[ONA_MSG_DATA_IDX];
            text_len = 0;
            while(*ptr != 0)
            {
                ptr++;
                text_len++;
            }
        }
        #endif
        
        print_text_packet(ONCLI_SINGLE_TXN_STR, &(raw_pld[ONA_MSG_DATA_IDX]),
          text_len, src_did);
        return ON_MSG_CONTINUE;
    }
    
    #ifndef _ONE_NET_MULTI_HOP
    if(msg_hdr->raw_pid != ONE_NET_RAW_SINGLE_DATA)
    #else
    if(msg_hdr->raw_pid != ONE_NET_RAW_SINGLE_DATA &&
       msg_hdr->raw_pid != ONE_NET_RAW_MH_SINGLE_DATA)
    #endif
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }

    if(ONA_IS_STATUS_MESSAGE(msg_class))
    {
        oncli_send_msg(ONCLI_DEVICE_STATE_FMT, src_unit, did_to_u16(src_did),
          msg_data);
        if(msg_class == ONA_STATUS_CHANGE && msg_type == ONA_SWITCH &&
          (msg_data == ONA_ON || msg_data == ONA_OFF))
        {
            // interpret ONA_STATUS_CHANGE as ONA_COMMAND
            msg_class = ONA_COMMAND;
        }
        else
        {
            return ON_MSG_CONTINUE;
        }
    }

    if(msg_type != ONA_SWITCH || (msg_data != ONA_ON && msg_data != ONA_OFF
      && msg_data != ONA_TOGGLE) || (msg_data == ONA_TOGGLE && msg_class !=
      ONA_COMMAND))
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }

    if(dst_unit == ONE_NET_DEV_UNIT)
    {
        ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_class == ONA_COMMAND)
    {
        if(user_pin[dst_unit].pin_type != ON_OUTPUT_PIN)
        {
            // we'll use a user-defined fatal error for the reason            
            ack_nack->nack_reason = ON_NACK_RSN_MIN_USR_FATAL;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
        }
        
        switch(dst_unit)
        {
            case 0: USER_PIN0 = (msg_data == ONA_TOGGLE) ? !USER_PIN0 : 
                    msg_data; break;
            case 1: USER_PIN1 = (msg_data == ONA_TOGGLE) ? !USER_PIN1 : 
                    msg_data; break;
            case 2: USER_PIN2 = (msg_data == ONA_TOGGLE) ? !USER_PIN2 : 
                    msg_data; break;
            case 3: USER_PIN3 = (msg_data == ONA_TOGGLE) ? !USER_PIN3 : 
                    msg_data; break;
        }
    }
   
    switch(dst_unit)
	{
		case 0: msg_data = USER_PIN0; break;
		case 1: msg_data = USER_PIN1; break;
		case 2: msg_data = USER_PIN2; break;
		case 3: msg_data = USER_PIN3; break;

        default:
            oncli_send_msg("Invalid dest. unit.\n");
            ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
	}
        
    switch(msg_class)
    {
        // note : Status message have already been handled.
        case ONA_COMMAND:
            msg_class = ONA_STATUS_COMMAND_RESP;
            oncli_send_msg(ONCLI_CHANGE_PIN_STATE_FMT, dst_unit, msg_data);
            break;
        case ONA_QUERY:
            msg_class = ONA_STATUS_QUERY_RESP;
            break;
        case ONA_FAST_QUERY:
            msg_class = ONA_STATUS_FAST_QUERY_RESP;
            break;
        default:
            ack_nack->nack_reason = ON_NACK_RSN_MIN_USR_FATAL;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
    }

    ack_nack->handle = ON_ACK_STATUS;

    // source and destination are reversed in the response.
    put_src_unit(dst_unit, ack_nack->payload->status_resp);
    put_dst_unit(src_unit, ack_nack->payload->status_resp);
    put_msg_class(msg_class, ack_nack->payload->status_resp);

    // we don't need to fill in the type.  It's already there since this is
    // the same memory as the raw payload!

    put_msg_data(msg_data, ack_nack->payload->status_resp);
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
    
    #if _DEBUG_VERBOSE_LEVEL > 3
    oncli_send_msg("ehanr : ");
    print_ack_nack(resp_ack_nack, 5);
    delay_ms(10);
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


/*!
    \brief Sends the user pin state to the assigned peers
    
    \param void
    
    \return void
*/
void send_user_pin_input(void)
{
    oncli_send_msg(ONCLI_CHANGE_PIN_STATE_FMT, user_pin_src_unit,
      user_pin[user_pin_src_unit].old_state);
      
    send_switch_status_change_msg(user_pin_src_unit, 
      user_pin[user_pin_src_unit].old_state, ONE_NET_DEV_UNIT, NULL);

    user_pin_state = CHECK_USER_PIN;
} // send_user_pin_input //


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
    
    #if _DEBUG_VERBOSE_LEVEL > 3
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

    print_ack_nack(ack_nack, get_raw_payload_len(msg_hdr.raw_pid) -  1 -
      ON_PLD_DATA_IDX);
    #endif

    oncli_send_msg(ONCLI_SINGLE_RESULT_FMT, did_to_u16(dst),
      oncli_msg_status_str(status));

    #if _DEBUG_VERBOSE_LEVEL > 3
    oncli_send_msg("ests end\n");
    #endif
    
    if(ack_nack->handle == ON_ACK_STATUS)
    {
        UInt8 src_unit = get_src_unit(ack_nack->payload->status_resp);
        UInt16 msg_data = get_msg_data(ack_nack->payload->status_resp);
        oncli_send_msg(ONCLI_DEVICE_STATE_FMT, src_unit, did_to_u16(dst),
          msg_data);
    }
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
    #if _DEBUG_VERBOSE_LEVEL > 2
    on_raw_nid_t raw_nid;
    BOOL valid = (on_decode(raw_nid, *enc_nid, ON_ENCODED_NID_LEN) ==
      ONS_SUCCESS);
    #endif
    
    oncli_send_msg("%s : 0x", description);
    uart_write_int8_hex_array(*enc_nid, FALSE, ON_ENCODED_NID_LEN);
    #if _DEBUG_VERBOSE_LEVEL > 2
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
    
    \return void
*/
#if _DEBUG_VERBOSE_LEVEL < 3
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes)
#else
void display_pkt(const UInt8* packet_bytes, UInt8 num_bytes,
  const one_net_xtea_key_t* const enc_keys, UInt8 num_enc_keys,
  const one_net_xtea_key_t* const invite_keys, UInt8 num_invite_keys)
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

    #if _DEBUG_VERBOSE_LEVEL > 1
    {
        UInt8 raw_pid = encoded_to_decoded_byte(
            packet_bytes[ONE_NET_ENCODED_PID_IDX], FALSE);
        on_pkt_t debug_pkt_ptrs;
        oncli_send_msg("Raw PID=%02X", raw_pid);

        if(!setup_pkt_ptr(raw_pid, packet_bytes, &debug_pkt_ptrs))
        {
            oncli_send_msg(" is invalid\n");
        }
        else
        {
            if(get_encoded_packet_len(raw_pid, TRUE) != num_bytes)
            {
                oncli_send_msg(" -- Invalid number of bytes for raw pid %02X, "
                  "Expected=%d, Actual=%d\n\n", raw_pid,
                  get_encoded_packet_len(raw_pid, TRUE), num_bytes);
                return;
            }
            
            oncli_send_msg("\n");
            oncli_send_msg("Enc. Msg ID : 0x%02X", *(debug_pkt_ptrs.enc_msg_id));
            #if _DEBUG_VERBOSE_LEVEL > 2
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
            #if _DEBUG_VERBOSE_LEVEL > 2
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
            
            #if _DEBUG_VERBOSE_LEVEL > 2
            {
                UInt8 raw_pld_len = get_raw_payload_len(raw_pid);
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
                    
                    #ifndef _ONE_NET_MASTER
                    one_net_xtea_key_t alternate_keys[1];
                    #else
                    one_net_xtea_key_t alternate_keys[2];
                    #endif

                    UInt8 num_keys;
                    const one_net_xtea_key_t* keys =
                      (const one_net_xtea_key_t*) &alternate_keys[0];

                    if(packet_is_invite(raw_pid))
                    {
                        if(num_invite_keys > 0)
                        {
                            num_keys = num_invite_keys;
                            keys = (const one_net_xtea_key_t*) &invite_keys[0];
                        }
                        else
                        {
                            #ifdef _ONE_NET_CLIENT
                            if(!device_is_master)
                            {
                                one_net_memmove(alternate_keys[0],
                                  one_net_client_get_invite_key(),
                                  sizeof(one_net_xtea_key_t));
                                num_keys = 1;
                            }
                            else
                            {
                                num_keys = 0;
                            }
                            #else
                            num_keys = 0;
                            #endif
                        }

                        data_type = ON_INVITE;
                    }
                    else
                    {
                        if(num_enc_keys > 0)
                        {
                            num_keys = num_enc_keys;
                            keys = (const one_net_xtea_key_t*)
                              &enc_keys[0];
                        }
                        else
                        {
                            one_net_memmove(alternate_keys[0],
                              on_base_param->current_key,
                              sizeof(one_net_xtea_key_t));
                            num_keys = 1;
                            #ifdef _ONE_NET_MASTER
                            if(device_is_master)
                            {
                                one_net_memmove(alternate_keys[1],
                                  on_base_param->old_key,
                                  sizeof(one_net_xtea_key_t));
                                num_keys = 2;
                            }
                            #endif
                        }
                        
                        data_type = ON_SINGLE;
                        #ifdef _BLOCK_MESSAGES_ENABLED
                        if(packet_is_block(raw_pid))
                        {
                            data_type = ON_BLOCK;
                        }
                        #endif
                        #ifdef _STREAM_MESSAGES_ENABLED
                        else if(packet_is_stream(raw_pid))
                        {
                            data_type = ON_STREAM;
                        }
                        #endif
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
                            if(packet_is_multihop(raw_pid))
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
#endif


#ifndef _ONE_NET_SIMPLE_DEVICE
void one_net_adjust_recipient_list(const on_single_data_queue_t* const msg,
  on_recipient_list_t** recipient_send_list)
{
}
#endif


/*!
    \brief Initializes the pins of the master to the default directions and values
    
    Masters have even pins as outputs and odd pins and inputs, clients have
    the reverse, which allows for quick commands between master and clients
    without having to change pin directions on the Eval Board manually.
    
    \param[in] is_master If true, the device is a master.  If false, the
               device is a client.

    \return void
*/
void initialize_default_pin_directions(BOOL is_master)
{
    UInt8 i;
    for(i = 0; i < NUM_USER_PINS; i++)
    {
        oncli_set_user_pin_type(i, (is_master == (i % 2)) ? ON_INPUT_PIN :
          ON_OUTPUT_PIN);
    }
    user_pin_state = CHECK_USER_PIN;
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
            ONS_RSRC_FULL otherwise
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

    if(one_net_send_single(ONE_NET_RAW_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, src_did, enc_dst
      #ifdef _PEER
          , TRUE, src_unit
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , 0
      #endif
      ))
    {
        return ONS_SUCCESS;
    }
    else
    {
        return ONS_RSRC_FULL;
    }
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
            ONS_RSRC_FULL otherwise.
*/
one_net_status_t send_simple_text_command(const char* text, UInt8 src_unit, 
  UInt8 dst_unit, const on_encoded_did_t* const enc_dst)
{
    UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];

    put_src_unit(src_unit, raw_pld);
    put_dst_unit(dst_unit, raw_pld);
    put_msg_hdr(ONA_COMMAND | ONA_SIMPLE_TEXT, raw_pld);    
    // we won't use the put_msg_data function here due to
    // endianness.  Instead use one_net_memmove
    one_net_memmove(&raw_pld[ONA_MSG_DATA_IDX], text, ONA_MSG_DATA_LEN);
      
    if(one_net_send_single(ONE_NET_RAW_SINGLE_DATA,
      ON_APP_MSG, raw_pld, ONA_SINGLE_PACKET_PAYLOAD_LEN,
      ONE_NET_HIGH_PRIORITY, NULL, enc_dst
      #ifdef _PEER
          , FALSE, ONE_NET_DEV_UNIT
      #endif
      #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
          , 0
      #endif
      ))
    {
        return ONS_SUCCESS;
    }
    else
    {
        return ONS_RSRC_FULL;
    }
} // send_send_simple_text_command //
#endif


/*!
    \brief Prints the contents of the received text packet.
    
    \param[in] TXN_STR String representing the transaction type that was rcv'd
    \param[in] TXT The text that was received.
    \param[in] TXT_LEN The number of characters received
    \param[in] SRC_ADDR The sender of the data packet

    \return void
*/
static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const on_raw_did_t *SRC_ADDR)
{
    oncli_send_msg(ONCLI_RX_TXT_FMT, did_to_u16(SRC_ADDR), TXT_LEN, TXT);
} // print_text_packet //



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
