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
#ifdef _NON_VOLATILE_MEMORY
#include "nv_hal.h"
#endif
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

#include "one_net_timer.h"

#include "oncli_port_const.h"

#include "dfi.h"
#include "eeprom_driver.h"


#include <interrupt.h>
#include <string.h>
#include <stdio.h>


// FOR MY TESTING
#define DEBUG_TRANCEIVER
#define DEBUG_TIMER
//#define DEBUG_UART

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

//#if 0
//! the pins on the eval board.
user_pin_t user_pin[NUM_USER_PINS];
//#endif

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

#ifdef _BLOCK_MESSAGES_ENABLED
//! Buffer to hold block (and possibly stream) values.
static char bs_buffer[DEFAULT_BS_CHUNK_SIZE * ON_BS_DATA_PLD_SIZE];
#endif


//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{




#if 0
static void eval_set_modes_from_switch_positions(void);
#endif

static void eval_set_master_or_client(void);

#ifdef _UART
static const char* get_prompt_string(void);
static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const on_raw_did_t *SRC_ADDR);
#endif



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
    UInt8 read_value = 0;

    if(user_pin[0].pin_type == ON_INPUT_PIN)
    {
         read_value = USER_PIN0_INPUT_PORT_REG & (1 << USER_PIN0_BIT);
         if(read_value != 0)
         {
             read_value = 1;
         }

         if(read_value != user_pin[0].old_state)
         {
            user_pin_state = SEND_USER_PIN_INPUT;
            user_pin_src_unit = 0;
            user_pin[0].old_state = read_value;
         }
    } // if the user0 pin has been toggled //
    else if(user_pin[1].pin_type == ON_INPUT_PIN)
    {
         read_value = USER_PIN1_INPUT_PORT_REG & (1 << USER_PIN1_BIT);
         if(read_value != 0)
         {
             read_value = 1;
         }

         if(read_value != user_pin[1].old_state)
         {
            user_pin_state = SEND_USER_PIN_INPUT;
            user_pin_src_unit = 1;
            user_pin[1].old_state = read_value;
         } // if the user1 pin has been toggled //
    }
    else if(user_pin[2].pin_type == ON_INPUT_PIN)
    {
         read_value = USER_PIN2_INPUT_PORT_REG & (1 << USER_PIN2_BIT);
         if(read_value != 0)
         {
             read_value = 1;
         }

         if(read_value != user_pin[2].old_state)
         {
            user_pin_state = SEND_USER_PIN_INPUT;
            user_pin_src_unit = 2;
            user_pin[2].old_state = read_value;
         }
    } // if the user2 pin has been toggled //
    else if(user_pin[3].pin_type == ON_INPUT_PIN)
    {
         read_value = USER_PIN3_INPUT_PORT_REG & (1 << USER_PIN3_BIT);
         if(read_value != 0)
         {
             read_value = 1;
         }

         if(read_value != user_pin[3].old_state)
         {
            user_pin_state = SEND_USER_PIN_INPUT;
            user_pin_src_unit = 3;
            user_pin[3].old_state = read_value;
         }
    } // if the user3 pin has been toggled //

} // check_user_pins //


#ifdef _UART
void oncli_print_prompt(void)
{
    oncli_send_msg("ocm%s> ", get_prompt_string());
} // oncli_print_prompt //
#endif


int main(void)
{
	init_tick_1();

	ENABLE_GLOBAL_INTERRUPTS();

    TAL_INIT_TRANSCEIVER();

    INIT_PROCESSOR(TRUE);

    #ifdef _HAS_LEDS
        initialize_leds();
    #endif

    cli();
    INIT_TICK();

    disable_user_pins();


	ENABLE_GLOBAL_INTERRUPTS();

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

    // startup greeting
    oncli_send_msg("\n\n");
    oncli_send_msg(ONCLI_STARTUP_FMT, ONE_NET_VERSION_MAJOR,
      ONE_NET_VERSION_MINOR);
    oncli_send_msg(ONCLI_STARTUP_REV_FMT, ONE_NET_VERSION_REVISION,
      ONE_NET_VERSION_BUILD);
    oncli_send_msg("\n\n");
    #endif

    // read from EEPROM the operation mode MASTER or CLIENT
    eval_set_master_or_client();

    //  // do some initialization here.  It may get written over later, but
    // that's OK.
    #if defined(_ONE_NET_MASTER)
    on_encode(on_base_param->sid, DEFAULT_RAW_NID, ON_ENCODED_NID_LEN);
    #endif
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

    #ifdef _ONE_NET_MASTER
    if(device_is_master)
    {
        #ifndef _NON_VOLATILE_MEMORY
        init_serial_master(-1);
        #else
        init_serial_master(TRUE, -1);
        #endif
    }
    #endif
    #ifdef _ONE_NET_CLIENT
    if(!device_is_master)
    {
        init_serial_client();
    }
    #endif

    #ifdef _UART
	oncli_send_msg("%s\n", ONCLI_SERIAL_MODE_STR);
    #endif

    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
 	while(1)
 	{
        #ifdef _UART
        if(ont_expired(PROMPT_TIMER))
        {
            oncli_print_prompt();
        }
        #endif

        (*node_loop_func)();
        #ifdef _UART
        oncli();
        #endif
    }

    cli();

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
    UInt8 src_unit, dst_unit, msg_type;
    ona_msg_class_t msg_class;
    UInt32 msg_data;


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
    if(verbose_level > 3)
    {
        oncli_send_msg("eval_hdl_sng: ");
        print_app_payload(raw_pld, 5);
        oncli_send_msg("\n");
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    }
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
            const UInt8* ptr = &raw_pld[ONA_TEXT_DATA_IDX];
            text_len = 0;
            while(*ptr != 0)
            {
                ptr++;
                text_len++;
            }
        }
        #endif

        #ifdef _UART
        print_text_packet((UInt8 *)ONCLI_SINGLE_TXN_STR, &(raw_pld[ONA_TEXT_DATA_IDX]),
          text_len, src_did);

//        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
        oncli_print_prompt();

        #endif
        return ON_MSG_CONTINUE;
    }


    if((msg_hdr->raw_pid & 0x3F) != ONE_NET_RAW_SINGLE_DATA)
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        ack_nack->handle = ON_NACK;
        return ON_MSG_CONTINUE;
    }

    if(ONA_IS_STATUS_MESSAGE(msg_class))
    {
        #ifdef _UART
        if(verbose_level)
        {
            oncli_send_msg(ONCLI_DEVICE_STATE_FMT, src_unit,
              did_to_u16(src_did), msg_data);
            ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
        }
        #endif
        if(msg_class == ONA_STATUS_CHANGE && msg_type == ONA_SWITCH &&
          (msg_data == ONA_ON || msg_data == ONA_OFF) && dst_unit <
          ONE_NET_NUM_UNITS)
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
            ack_nack->nack_reason = ON_NACK_RSN_UNIT_IS_INPUT;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
        }

        switch(dst_unit)
        {
            case 0:
			  if(msg_data == ONA_TOGGLE)
			  {
				  USER_PIN0_OUTPUT_PORT_REG |= (1 << USER_PIN0_BIT);
			  }
			  else if(msg_data == ONA_ON)
			  {
  				  USER_PIN0_OUTPUT_SET_PORT_REG |= (1 << USER_PIN0_BIT);
			  }
			  else if(msg_data == ONA_OFF)
			  {
			      USER_PIN0_OUTPUT_PORT_REG &= ~(1 << USER_PIN0_BIT);
			  }
			  break;

            case 1:
			  if(msg_data == ONA_TOGGLE)
			  {
				  USER_PIN1_OUTPUT_PORT_REG |= (1 << USER_PIN1_BIT);
			  }
			  else if(msg_data == ONA_ON)
			  {
				  USER_PIN1_OUTPUT_SET_PORT_REG |= (1 << USER_PIN1_BIT);
			  }
			  else if(msg_data == ONA_OFF)
			  {
                  USER_PIN1_OUTPUT_PORT_REG &= ~(1 << USER_PIN1_BIT);
			  }
			  break;

            case 2:
            if(msg_data == ONA_TOGGLE)
            {
	            USER_PIN2_OUTPUT_PORT_REG |= (1 << USER_PIN2_BIT);
            }
            else if(msg_data == ONA_ON)
            {
	            USER_PIN2_OUTPUT_SET_PORT_REG |= (1 << USER_PIN2_BIT);
            }
            else if(msg_data == ONA_OFF)
            {
                USER_PIN2_OUTPUT_PORT_REG &= ~(1 << USER_PIN2_BIT);
            }
            break;

            case 3:
            if(msg_data == ONA_TOGGLE)
            {
	            USER_PIN3_OUTPUT_PORT_REG |= (1 << USER_PIN3_BIT);
            }
            else if(msg_data == ONA_ON)
            {
	            USER_PIN3_OUTPUT_SET_PORT_REG |= (1 << USER_PIN3_BIT);
            }
            else if(msg_data == ONA_OFF)
            {
                USER_PIN3_OUTPUT_PORT_REG &= ~(1 << USER_PIN3_BIT);
            }
            break;
            default:  break;
        }
    }

    switch(dst_unit)
	{
//		case 0: msg_data = USER_PIN0; break;
		case 0:
		 msg_data = (UInt32)(USER_PIN0_INPUT_PORT_REG & (1 << USER_PIN0_BIT));
		 if(msg_data != 0)
		 {
			 msg_data = 1;
		 }
		 break;

//		case 1: msg_data = USER_PIN1; break;
        case 1:
		 msg_data = (UInt32)(USER_PIN1_INPUT_PORT_REG & (1 << USER_PIN1_BIT));
		 if(msg_data != 0)
		 {
			 msg_data = 1;
		 }
		 break;

//		case 2: msg_data = USER_PIN2; break;
        case 2:
        msg_data = (UInt32)(USER_PIN2_INPUT_PORT_REG & (1 << USER_PIN2_BIT));
		 if(msg_data != 0)
		 {
			 msg_data = 1;
		 }
        break;

//		case 3: msg_data = USER_PIN3; break;
       case 3:
        msg_data = (UInt32)(USER_PIN3_INPUT_PORT_REG & (1 << USER_PIN3_BIT));
		 if(msg_data != 0)
		 {
			 msg_data = 1;
		 }
        break;

        default:
            #ifdef _UART
            if(verbose_level)
            {
                oncli_send_msg("Invalid dest. unit.\n");
                ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
            }
            #endif
            ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
            ack_nack->handle = ON_NACK;
            return ON_MSG_CONTINUE;
	}

    switch(msg_class)
    {
        // note : Status message have already been handled.
        case ONA_COMMAND:
            msg_class = ONA_STATUS_COMMAND_RESP;
            #ifdef _UART
            if(verbose_level)
            {
                oncli_send_msg(ONCLI_CHANGE_PIN_STATE_FMT, dst_unit, msg_data);
//                ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
                oncli_print_prompt();

            }
            #endif
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
    //#endif

    #if _DEBUG_VERBOSE_LEVEL > 3
    if(verbose_level > 3)
    {
        oncli_send_msg("ehanr : ");
        print_ack_nack(resp_ack_nack, 5);
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
        delay_ms(10);
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
    } // loop to clear user pins ////
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
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);

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
//            user_pin[0].old_state = USER_PIN0_INPUT_PORT_REG & (1 << USER_PIN0_BIT);
//            USER_PIN0_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
//              : pin_type;

			// read the input
            user_pin[0].old_state = USER_PIN0_INPUT_PORT_REG & (1 << USER_PIN0_BIT);

            if(user_pin[0].old_state != 0)
            {
                user_pin[0].old_state = 1;
            }

            if((pin_type == ON_DISABLE_PIN) || (pin_type == ON_INPUT_PIN))
			{
				// configure the pin as an INPUT
				USER_PIN0_DIR_REG &= ~(1 << USER_PIN0_DIR_BIT);
			}
			else
			{
				// configure the pin as an OUTPUT
				USER_PIN0_DIR_REG |= (1 << USER_PIN0_DIR_BIT);
			}
            break;
        } // pin 0 case //

        case 1:
        {
//            user_pin[1].old_state = USER_PIN1_INPUT_PORT_REG & (1 << USER_PIN1_BIT);;
//            USER_PIN1_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
//              : pin_type;

			// read the input
            user_pin[1].old_state = USER_PIN1_INPUT_PORT_REG & (1 << USER_PIN1_BIT);

            if(user_pin[1].old_state != 0)
            {
                user_pin[1].old_state = 1;
            }

            if((pin_type == ON_DISABLE_PIN) || (pin_type == ON_INPUT_PIN))
            {
				// configure the pin as an INPUT
	            USER_PIN1_DIR_REG &= ~(1 << USER_PIN1_DIR_BIT);
            }
            else
            {
				// configure the pin as an OUTPUT
	            USER_PIN1_DIR_REG |= (1 << USER_PIN1_DIR_BIT);
            }
            break;
        } // pin 1 case //

        case 2:
        {
//            user_pin[2].old_state = USER_PIN2;
//            USER_PIN2_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
//              : pin_type;

    		// read the input
            user_pin[2].old_state = USER_PIN2_INPUT_PORT_REG & (1 << USER_PIN2_BIT);

            if(user_pin[2].old_state != 0)
            {
                user_pin[2].old_state = 1;
            }

            if((pin_type == ON_DISABLE_PIN) || (pin_type == ON_INPUT_PIN))
            {
				// configure the pin as an INPUT
	            USER_PIN2_DIR_REG &= ~(1 << USER_PIN2_DIR_BIT);
            }
            else
            {
				// configure the pin as an OUTPUT
	            USER_PIN2_DIR_REG |= (1 << USER_PIN2_DIR_BIT);
            }
            break;
        } // pin 2 case //

        case 3:
        {
//            user_pin[3].old_state = USER_PIN3;
//            USER_PIN3_DIR = pin_type == ON_DISABLE_PIN ? ON_INPUT_PIN
//              : pin_type;

            // read the input
            user_pin[3].old_state = USER_PIN3_INPUT_PORT_REG & (1 << USER_PIN3_BIT);

            if(user_pin[3].old_state != 0)
            {
                user_pin[3].old_state = 1;
            }

            if((pin_type == ON_DISABLE_PIN) || (pin_type == ON_INPUT_PIN))
            {
				// configure the pin as an INPUT
	            USER_PIN3_DIR_REG &= ~(1 << USER_PIN3_DIR_BIT);
            }
            else
            {
				// configure the pin as an OUTPUT
	            USER_PIN3_DIR_REG |= (1 << USER_PIN3_DIR_BIT);
            }
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
//                state = USER_PIN0;
                state = USER_PIN0_INPUT_PORT_REG & (1 << USER_PIN0_BIT);
				if(state != 0)
				{
		 			state = 1;
				}
                break;
            }

            case 1:
            {
//                state = USER_PIN1;
                state = USER_PIN1_INPUT_PORT_REG & (1 << USER_PIN1_BIT);
				if(state != 0)
				{
					state = 1;
				}
                break;
            }

            case 2:
            {
//                state = USER_PIN2;
                state = USER_PIN2_INPUT_PORT_REG & (1 << USER_PIN2_BIT);
				if(state != 0)
				{
					state = 1;
				}
                break;
            }

            case 3:
            {
//                state = USER_PIN3;
                state = USER_PIN3_INPUT_PORT_REG & (1 << USER_PIN3_BIT);
				if(state != 0)
				{
					state = 1;
				}
                break;
            }

            default:
            {
                state = 3;
            }
        }
        type_string = (const UInt8 *)ONCLI_DISABLE_STR;
        if (user_pin[i].pin_type == ON_INPUT_PIN)
        {
            type_string = (const UInt8 *)ONCLI_INPUT_STR;
            oncli_send_msg("  %d %s state: %d\n", i, type_string, state);
        }
        else if (user_pin[i].pin_type == ON_OUTPUT_PIN)
        {
            type_string = (const UInt8 *)ONCLI_OUTPUT_STR;
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
    #ifdef _ROUTE
    tick_t route_time = get_tick_count() - route_start_time;
    #endif

    #ifdef _UART
    if(!dst)
    {
        return;
    } // if the parameters are invalid //

    #if _DEBUG_VERBOSE_LEVEL > 3
    if(verbose_level > 3)
    {
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
    }
    #endif

    if(verbose_level)
    {
        oncli_send_msg(ONCLI_SINGLE_RESULT_FMT, did_to_u16(dst),
        oncli_msg_status_str(status));
    }

    #if _DEBUG_VERBOSE_LEVEL > 3
    if(verbose_level > 3)
    {
        #ifdef _ROUTE
        if(ack_nack->handle == ON_ACK_ROUTE)
        {
            on_raw_did_t raw_did;
            on_decode(raw_did, &on_base_param->sid[ON_ENCODED_NID_LEN],
               ON_ENCODED_DID_LEN);
            append_raw_did_to_route(ack_nack->payload->ack_payload,
               (const on_raw_did_t* const) raw_did);
            oncli_send_msg("Route Time:%ld ms:", route_time);
            print_route(ack_nack->payload->ack_payload);
            oncli_send_msg("\n");
        }
        #endif
        oncli_send_msg("ests end\n");
   }
   #endif

   if(ack_nack->handle == ON_ACK_STATUS)
   {
       UInt8 src_unit = get_src_unit(ack_nack->payload->status_resp);
       UInt32 msg_data = get_msg_data(ack_nack->payload->status_resp);
       oncli_send_msg(ONCLI_DEVICE_STATE_FMT, src_unit, did_to_u16(dst),
          msg_data);
   }

    if(verbose_level)
    {
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    }
   #endif
}

#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Callback function called when a block transaction is complete.

    Several things can cause this function to be called.

    1. We are the source and everything transferred successfully and we are
       informing the application code that this is the case, and we will also
       immediately inform the destination device, any repeaters, and possibly
       the master.
    2. We are the source and we need to terminate prematurely on our end.  We
       need to inform our application code as well as the other device(s).
    3. We are the destination and everything transferred successfully and we are
       informing the application code that this is the case.  No other messages
       are needed.
    4. We are the destination and we need to terminate prematurely on our end.
       Any ACKs or NACKs to the sending device have been handled elsewhere.
    5. We are the master and the destinatoion or another device terminated the
       transaction prematurely.  We are informing the application code that this
       is the case, and we will also immediately inform the destination device,
       any repeaters, and possibly the master.

    If we are the source, then the ack_nack message will be non-NULL and
    will be pre-loaded with what ONE-NET intends to send the other device(s) in
    the termination message.  This function can either leave the ack_nack
    alone, it can change it, or it tells ONE-NET NOT TO send this termination
    message.  It does that by returning anything but ON_MSG_RESPOND.

    If we are not the source, we might either abort immediately or "hang around"
    as a service to any other device(s) and give them an ACK or a NACK or
    stay as a repeater or whatever. In that case, this function should return
    ON_MSG_RESPOND and we will, if we are the destination and we are the ones
    terminating the device, at least tell the sending device that we are
    terminating and why.  On the other hand, we can also simply "drop out" of
    the message and the other device(s) will eventually time out.

    The termination message should be changed if it is something that would
    not make sense to the other devices.

    Note that the status can also be changed if desired by this application code.


    \param[in] msg The block / stream message that is being terminated.
    \param[in] terminating_device The device that terminated this transaction.  If NULL, then this device is the one that terminated
    \param[in/out] status The status of the message that was just completed.
    \param[in/out] ack_nack Any ACK or NACK associated with this termination.

    \return ON_MSG_RESPOND if this device should inform the other devices
              of the termination.
            All other return types abort immediately with no further messages.
*/
on_message_status_t eval_bs_txn_status(const block_stream_msg_t* msg,
  const on_encoded_did_t* terminating_device, on_message_status_t* status,
  on_ack_nack_t* ack_nack)
{
    // if we are a repeater, don't do anything.
    if(!bs_msg.src && !bs_msg.dst)
    {
        return ON_MSG_DEFAULT_BHVR; // return type is irrelevant here
    }
    #if _DEBUG_VERBOSE_LEVEL > 0
    if(verbose_level > 0)
    {
        #ifndef _STREAM_MESSAGES_ENABLED
        const char* transfer_type = "Block";
        #else
        const char* transfer_type = ((get_bs_transfer_type(msg->flags) ==
          ON_BLK_TRANSFER) ? "Block" : "Stream");
        #endif
        const char* result_str = ((*status == ON_MSG_SUCCESS) ?
          "terminated successfully" : (*status == ON_MSG_TIMEOUT) ?
          "timed out" : "terminated prematurely");

        on_encoded_did_t* other_device = (bs_msg.src ? &bs_msg.src->did :
          &bs_msg.dst->did);
        on_raw_did_t raw_did;
        on_decode(raw_did, *other_device, ON_ENCODED_DID_LEN);
        oncli_send_msg("\n%s message with %03X %s.\n", transfer_type,
          did_to_u16((const on_raw_did_t *)&raw_did), result_str);
    }
    #endif

    return ON_MSG_RESPOND; // irrelevant if we are not the source.
}


on_message_status_t eval_handle_bs_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, const on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return ON_MSG_ACCEPT_PACKET;
}
#endif





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
            oncli_send_msg(" -- Decoded : 0x%03X", did_to_u16((const on_raw_did_t *)&raw_did));
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
    if(verbose_level < 2)
    {
        return;
    }
    {
        UInt16 raw_pid;
        on_pkt_t debug_pkt_ptrs;

        if(!get_raw_pid((UInt8*)&packet_bytes[ON_ENCODED_PID_IDX], &raw_pid))
        {
            return;
        }

        print_raw_pid(raw_pid);

        if(!setup_pkt_ptr(raw_pid, (UInt8*)packet_bytes, 0, &debug_pkt_ptrs))
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

            oncli_send_msg("\nEnc. Msg CRC : 0x%02X",
              debug_pkt_ptrs.packet_bytes[ON_ENCODED_MSG_CRC_IDX]);
            #if _DEBUG_VERBOSE_LEVEL > 2
            if(verbose_level > 2)
            {
                UInt8 calculated_msg_crc, msg_crc;

                oncli_send_msg(" -- Decoded Msg. CRC : ");
                if(on_decode(&msg_crc,
                  &debug_pkt_ptrs.packet_bytes[ON_ENCODED_MSG_CRC_IDX],
                  ONE_NET_ENCODED_MSG_CRC_LEN) != ONS_SUCCESS)
                {
                    oncli_send_msg("Not decodable\n");
                }
                else
                {
                    oncli_send_msg("0x%02X\n", msg_crc);
                }

                calculated_msg_crc = calculate_msg_crc(&debug_pkt_ptrs);
                oncli_send_msg("Calculated Raw Msg CRC : 0x%02X\n", calculated_msg_crc);

                if(calculated_msg_crc != msg_crc)
                {
                    oncli_send_msg("Calc. msg. CRC does not match packet "
                      "CRC!\n");
                }
            }
            else
            #endif
            {
                oncli_send_msg("\n");
            }


            debug_display_did("Repeater DID", (const on_encoded_did_t* const)
              &(debug_pkt_ptrs.packet_bytes[ON_ENCODED_RPTR_DID_IDX]));
            debug_display_did("Dest. DID", (const on_encoded_did_t* const)
              &(debug_pkt_ptrs.packet_bytes[ON_ENCODED_DST_DID_IDX]));
            debug_display_nid("NID", (const on_encoded_nid_t* const)
              &(debug_pkt_ptrs.packet_bytes[ON_ENCODED_NID_IDX]));
            debug_display_did("Source DID", (const on_encoded_did_t* const)
              &(debug_pkt_ptrs.packet_bytes[ON_ENCODED_SRC_DID_IDX]));

            oncli_send_msg("Encoded Payload Length : %d\n",
              debug_pkt_ptrs.payload_len);
            for(i = 0; i < debug_pkt_ptrs.payload_len; i++)
            {
                oncli_send_msg("%02X ",
                  debug_pkt_ptrs.packet_bytes[ON_PLD_IDX + i]);

                if((i % 16) == 15)
                {
                    oncli_send_msg("\n");
                } // if need to output a new line //
            } // loop to output the bytes that were read //

            #if _DEBUG_VERBOSE_LEVEL > 2
            if(verbose_level > 2)
            {
                UInt8 raw_pld_len = get_raw_payload_len(raw_pid);
                oncli_send_msg("\nDecoded Payload (# of Bytes = %d)\n",
                  raw_pld_len);
                if(on_decode(raw_payload_bytes,
                  &(debug_pkt_ptrs.packet_bytes[ON_PLD_IDX]),
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

                        #ifdef _STREAM_MESSAGES_ENABLED
                        if(on_decrypt(data_type == ON_STREAM, decrypted,
                          (const one_net_xtea_key_t * const)keys[j], raw_pld_len) !=
                          ONS_SUCCESS)
                        #else
                        if(on_decrypt(decrypted, (const one_net_xtea_key_t * const)keys[j],
                          raw_pld_len) != ONS_SUCCESS)
                        #endif
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
                        oncli_send_msg("Msg. ID=%03X\n", get_payload_msg_id(decrypted));

                        #ifdef _ONE_NET_MULTI_HOP
                        {
                            if(packet_is_multihop(raw_pid))
                            {
                                oncli_send_msg("Encoded Hops Field : %02X  ",
                                  *(&(debug_pkt_ptrs.packet_bytes[ON_PLD_IDX])
                                  + debug_pkt_ptrs.payload_len));
                                if(on_parse_hops(&debug_pkt_ptrs,
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

                        #if _DEBUG_VERBOSE_LEVEL > 4
                        if(verbose_level <= 4 || !crc_match)
                        {
                            continue;
                        }
                        if(packet_is_data(raw_pid) && packet_is_single(raw_pid))
                        {
                            print_single(raw_pid, decrypted);
                        }
                        else if(packet_is_ack(raw_pid) ||
                          packet_is_nack(raw_pid))
                        {
                            print_response(raw_pid, decrypted);
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


#ifndef _ONE_NET_SIMPLE_CLIENT
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



#ifdef _DATA_RATE_CHANNEL
void one_net_data_rate_channel_changed(UInt8 new_channel, UInt8 new_data_rate)
{
    oncli_send_msg("Changed to data rate %s, channel ",
      DATA_RATE_STR[new_data_rate]);
    oncli_print_channel(new_channel);
}
#endif


#ifndef _ONE_NET_SIMPLE_CLIENT
/*!
    \brief Allows the application code to override whether a nack reason is fatal

           If desired, the application code can change the is_fatal parameter.


    \param[in] nack_reason
    \param[in/out] is_fatal Whether ONE-NET has determined a NACK Reason to be fatal.  To override
                   ONE-NET's decision, change the is_fatal parameter.  Otherwise, do nothing
*/
void one_net_adjust_fatal_nack(on_nack_rsn_t nack_reason, BOOL* is_fatal)
{
     // Empty function.  No adjustment.
}


void one_net_single_msg_loaded(on_txn_t** txn, on_single_data_queue_t* msg)
{
}
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
void one_net_block_stream_transfer_requested(const block_stream_msg_t* const
  bs_msg, on_ack_nack_t* ack_nack)
{
    #ifdef _STREAM_MESSAGES_ENABLED
    if(get_bs_transfer_type(bs_msg->flags) == ON_STREAM_TRANSFER)
    {
        return;
    }
    #endif
    if(!bs_msg->dst)
    {
        if(bs_msg->bs.block.chunk_size > DEFAULT_BS_CHUNK_SIZE)
        {
            // make sure we can handle the chunk size.
            ack_nack->nack_reason = ON_NACK_RSN_INVALID_CHUNK_SIZE;
            ack_nack->handle = ON_NACK_VALUE;
            ack_nack->payload->nack_value = DEFAULT_BS_CHUNK_SIZE;
            return;
        }

        // we are the recipient, so clear.
        one_net_memset(bs_buffer, 0, sizeof(bs_buffer));
    }
}
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
      ONE_NET_HIGH_PRIORITY, (const on_encoded_did_t* const)src_did, enc_dst
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
#if 0
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
#endif


static void eval_set_master_or_client(void)
{
    // read eeprom operation mode byte(master= 1 or client = 0, if value is 0xFF defaults to client)
    UInt16 address = DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET;
    UInt8 read_byte = eeprom_read_byte((UInt16)address);

    #ifdef _ONE_NET_CLIENT

    #ifdef _ONE_NET_MASTER
    // note : this includes devices which are both master and clients
    if((read_byte == 0) || (read_byte == 0xFF))
    {
        device_is_master = FALSE;
        node_loop_func = &client_eval;
    }
    else if (read_byte == 1)
    {
        device_is_master = TRUE;
        node_loop_func = &master_eval;
    }
    #endif

    // note : this includes devices which are clients only
    if((read_byte == 0) || (read_byte == 0xFF))
    {
        device_is_master = FALSE;
        node_loop_func = &client_eval;
    }
    else
    {
        // this condition could not happen (when the option _ONE_NET_CLIENT is disbaled in the file config_options.h and
        // the project is rebuild, then the EEPROM IS ERASED
    }
    #endif   // #ifdef _ONE_NET_CLIENT

	#ifdef _ONE_NET_MASTER
    // note : this includes devices which are masters only
    if(read_byte == 1)
    {
        device_is_master = TRUE;
        node_loop_func = &master_eval;
    }
    else
    {
        // this condition could not happen (when the option _ONE_NET_MASTER is disbaled in the file config_options.h and
        // the project is rebuild, then the EEPROM IS ERASED
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
    // simple text
    one_net_memmove(&raw_pld[ONA_TEXT_DATA_IDX], text,
	  	       ONA_SIMPLE_TEXT_DATA_LEN);

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


#ifdef _UART
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
     char output_string[ONCLI_MAX_OUTPUT_STR_LEN];
     UInt16 output_len = 0;

     #ifdef _EXTENDED_SINGLE
     UInt8 TextBuffer[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN-ONA_TEXT_DATA_IDX-1];
     #else
     UInt8 TextBuffer[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN-ONA_TEXT_DATA_IDX];
     #endif

     UInt16 i=0;
     for(i=0; i<sizeof(TextBuffer); i++)
     {
	     if(i<TXT_LEN)
	     {
		     TextBuffer[i] = *(TXT+i);
	     }
	     else
	     {
		     TextBuffer[i] = 0;
	     }
     }
     sprintf(output_string, "Received text from %03X:\n%s\n", did_to_u16(SRC_ADDR), TextBuffer);
     
     output_len = strlen(output_string);

     uart_write((const UInt8 * const)output_string, output_len);

} // print_text_packet //

/*
static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const on_raw_did_t *SRC_ADDR)
{
    oncli_send_msg(ONCLI_RX_TXT_FMT, did_to_u16(SRC_ADDR), TXT_LEN, TXT);
} // print_text_packet //
*/

#endif


#ifdef _DATA_RATE_CHANNEL
SInt8 one_net_get_alternate_channel(void)
{
    // TODO -- We need to find a much better way to get alternate channels.
    // For now, just pick one at random.  Don't even bother to check whether
    // it is clear.
    while(1)
    {
        SInt8 new_channel = (SInt8) one_net_prand(get_tick_count(),
          ONE_NET_MAX_CHANNEL);
        if((UInt8) new_channel != on_base_param->channel)
        {
            return new_channel;
        }
    }
}
#endif



#ifdef _BLOCK_MESSAGES_ENABLED
on_message_status_t one_net_block_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack)
{
    // just a quick load function for testing.  Loads with values from 'a'
    // to 'y' depending on the packet index.
    one_net_memset(buffer, 'a' + ((bs_msg->bs.block.byte_idx +
      bs_msg->bs.block.chunk_idx) % 25), ON_BS_DATA_PLD_SIZE);
    return ON_MSG_CONTINUE;
}


/*!
    \brief Called when a chunk of a block has been received.

    \param[in] bs_msg The block / stream message
    \param[in] byte_idx The byte index of the start of the chunk
    \param[in] chunk_size The size of the chunk
    \param[out] The ACK or NACK that should be returned in the response, if any

    \return ON_MSG_ACCEPT_CHUNK to mark the chunk as valid and move on.
            ON_MSG_REJECT_CHUNK to force the other side to send the entire chunk again.
            See on_message_status_t for other options.
*/
on_message_status_t eval_block_chunk_received(
  block_stream_msg_t* bs_msg, UInt32 byte_idx, UInt8 chunk_size,
  on_ack_nack_t* ack_nack)
{
    UInt16 i;
    for(i = 0; i < chunk_size; i++)
    {
        UInt32 remaining;
        if(i >= DEFAULT_BS_CHUNK_SIZE)
        {
            // should never get here.
            break;
        }

        remaining = block_get_bytes_remaining(bs_msg->bs.block.transfer_size,
          byte_idx, i);
        uart_write((const UInt8 * const)&bs_buffer[i * ON_BS_DATA_PLD_SIZE], remaining <
          ON_BS_DATA_PLD_SIZE ? remaining : ON_BS_DATA_PLD_SIZE);
    }
    one_net_memset(bs_buffer, 0, sizeof(bs_buffer));
    return ON_MSG_ACCEPT_CHUNK;
}


/*!
    \brief Handles the received block packet.

    \param[in] txn The block / stream transaction
    \param[in] bs_msg The block / stream message
    \param[in] block_pkt The packet received
    \param[out] The ACK or NACK that should be returned in the response, if any

    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_ACCEPT_PACKET if the packet is valid and a response should be sent
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/
on_message_status_t eval_handle_block(on_txn_t* txn,
  block_stream_msg_t* bs_msg, block_pkt_t* block_pkt, on_ack_nack_t* ack_nack)
{
    #if _DEBUG_VERBOSE_LEVEL > 3
    if(verbose_level > 3)
    {
        #ifndef _STREAM_MESSAGES_ENABLED
        print_bs_pkt((const block_stream_pkt_t*) block_pkt, TRUE);
        #else
        print_bs_pkt((const block_stream_pkt_t*) block_pkt, TRUE, FALSE);
        #endif
    }
    #endif

    // TODO -- what if chunk index is too high?
    if(block_pkt->chunk_idx < DEFAULT_BS_CHUNK_SIZE)
    {
        one_net_memmove(&bs_buffer[ON_BS_DATA_PLD_SIZE * block_pkt->chunk_idx],
          block_pkt->data, ON_BS_DATA_PLD_SIZE);
    }

    return ON_MSG_ACCEPT_PACKET;
}


on_message_status_t on_master_handle_block_ack_nack_response(
  on_txn_t* txn, block_stream_msg_t* bs_msg, on_pkt_t* pkt,
  const UInt8* raw_payload_bytes, on_ack_nack_t* ack_nack)
{
    return ON_MSG_DEFAULT_BHVR;
}


#ifdef _STREAM_MESSAGES_ENABLED
on_message_status_t one_net_stream_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack)
{
    // Just fill with the current number of milliseconds since we booted
    // for the first four bytes, then add one for each byte after that. Just
    // making an easy to read buffer for easy testing purposes.  Don't worry about
    // the last byte.
    UInt32 time_ms = TICK_TO_MS(get_tick_count());
    UInt8 i;

    for(i = 0; i < 7; i++)
    {
        one_net_int32_to_byte_stream(time_ms, buffer);
        time_ms++;
        buffer += sizeof(UInt32);
    }

    return ON_MSG_CONTINUE;
}


/*!
    \brief Handles the received stream packet.

    \param[in] txn The block / stream transaction
    \param[in] bs_msg The block / stream message
    \param[in] stream_pkt The packet received
    \param[out] The ACK or NACK that should be returned in the response, if any

    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
            See on_message_status_t for other options.
*/
on_message_status_t eval_handle_stream(on_txn_t* txn,
  block_stream_msg_t* bs_msg, stream_pkt_t* stream_pkt, on_ack_nack_t* ack_nack)
{
    UInt8 i;
    UInt8* buf = stream_pkt->data;
//    UInt32 time_ms = one_net_byte_stream_to_int32(buf);
    oncli_send_msg("ET:%ld:NR:%d", stream_pkt->elapsed_time,
      stream_pkt->response_needed);
    for(i = 0; i < 6; i++)
    {
        oncli_send_msg("%ld", one_net_byte_stream_to_int32(buf));
        buf += sizeof(UInt32);
        if(i < 5)
        {
            oncli_send_msg(",");
        }
    }
    oncli_send_msg("\n");
    return ON_MSG_RESPOND;
}
#endif
#endif



//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval