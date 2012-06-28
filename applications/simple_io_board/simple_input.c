//! \defgroup SIMPLE_INPUT
//! @{

/*
    Copyright (c) 2012, Threshold Corporation
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
    \file simple_input.c
    \brief Opto Input board.
    
    This file is the main application for a simple ONE-NET opto input device.

    \version 0.3
*/


#include "client_util.h"
#include "one_net_client.h"
#include "one_net_port_specific.h"
#include "io_port_mapping.h"
#include "hal_adi.h"
#include "pal.h"
#include "tal.h"
#ifdef HAS_LEDS
    #include "one_net_led.h"
#endif
#include "tick.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup simple_input_const
//! \ingroup SIMPLE_INPUT
//! @{


//! The unique key for this device.  This key is used when this device is added
//! to the network.
//Changed to make it work with new cli invite command.
//First two groups are duplicated into upper
//
//const one_net_xtea_key_t DEV_KEY = {'4', '5', 'p', 't', '6', 'i', 'n',
  //'p', 'u', 't', '2', '2', '2', '2', '2', '2'};
const one_net_xtea_key_t DEV_KEY = {
    '4', 'i', 'n', 'p', 
    'u', 't', '2', '7',
    '4', 'i', 'n', 'p', 
    'u', 't', '2', '7'
};

const ona_unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES]
  = {{ONA_SIMPLE_SWITCH, ONE_NET_NUM_UNITS}};

//! @} simple_input_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup simple_input_typedefs
//! \ingroup SIMPLE_INPUT
//! @{

//! @} simple_input_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup simple_input_pri_var
//! \ingroup SIMPLE_INPUT
//! @{

#if 0
//! Timer to turn off the transmit LED
static tick_t tx_led_timer = 0;

//! Timer to turn off the receive LED
static tick_t rx_led_timer = 0;

//! the voltage status reading
static UInt8 voltage_status = ONA_VOLTAGE_GOOD;
#endif

//! The state of the inputs the last time they were checked
static UInt8 input_pin[ONE_NET_NUM_UNITS];

//! @} simple_input_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup simple_input_pri_func
//! \ingroup SIMPLE_INPUT
//! @{

/* Inputs are defined in io_port_mapping.h so that hi = off and lo = on */
#define INVAL1 !INPUT1
#define INVAL2 !INPUT2
#define INVAL3 !INPUT3
#define INVAL4 !INPUT4


static void init_input(void);
static BOOL get_pin_state(UInt8 unit, SInt32 *status);
static one_net_status_t send_switch_status_change_msg(UInt8 src_unit, 
  UInt8 status, UInt8 dst_unit, const on_encoded_did_t* const enc_dst);
static void check_input_pin(void);


//! @} simple_input_pri_func
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup simple_input_pub_func
//! \ingroup SIMPLE_INPUT
//! @{


void one_net_client_invite_result(const on_raw_did_t * const RAW_DID,
  one_net_status_t status)
{
} // one_net_client_invite_result //


on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack)
{
	SInt32 msg_data;
    UInt8 src_unit, dst_unit, msg_type;
    ona_msg_class_t msg_class;

    on_parse_app_pld(raw_pld, &src_unit, &dst_unit, &msg_class, &msg_type,
      &msg_data);
	
	ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
	if(ONA_IS_STATUS_MESSAGE(msg_class))
	{
        // accept message, but don't do anything
	}
    else if(msg_class != ONA_FAST_QUERY || msg_type != ONA_SWITCH)
	{
		ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
	}
	else if(dst_unit >= NUM_IO_UNITS)
	{
		ack_nack->nack_reason = ON_NACK_RSN_UNIT_FUNCTION_ERR;
	}
    else
	{
		// Legitimate Fast Query.  We will respond.
		get_pin_state(dst_unit, &msg_data);
		
	    // source and destination are reversed
	    put_src_unit(dst_unit, ack_nack->payload->status_resp);
	    put_dst_unit(src_unit, ack_nack->payload->status_resp);
	    put_msg_data(msg_data, ack_nack->payload->status_resp);
	    put_msg_type(ONA_SWITCH, ack_nack->payload->status_resp);
        ack_nack->handle = ON_ACK_STATUS;
        put_msg_class(ONA_STATUS_FAST_QUERY_RESP,
		  ack_nack->payload->status_resp);
	}

	return ON_MSG_CONTINUE;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack)
{
} // one_net_client_single_txn_status //


void one_net_client_client_remove_device(void)
{
    clr_flash();
    one_net_client_look_for_invite(&DEV_KEY);
} // one_net_client_client_remove_device //


one_net_status_t one_net_client_reset_client(one_net_xtea_key_t* invite_key)
{
    return one_net_client_look_for_invite(invite_key);
}


/*!
    \brief Returns a pointer to the invite key to use in for joining a network.
    
    \return A pointer to the invite key to use.
*/
one_net_xtea_key_t* one_net_client_get_invite_key(void)
{
    return &DEV_KEY;
}


void one_net_client_client_removed(const on_raw_did_t * const raw_did,
    BOOL this_device_removed)
{
}


void one_net_client_client_added(const on_raw_did_t * const raw_did)
{
}


on_message_status_t one_net_client_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries)
{
    return ON_MSG_CONTINUE;
}



//! @} simple_input_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup simple_input_pri_func
//! \ingroup SIMPLE_INPUT
//! @{



/*!
    \brief Initialize the variable that keeps track of the input pin states.
    
    \param void
    
    \return void
*/
static void init_input(void)
{
    input_pin[0] = INVAL1;
    input_pin[1] = INVAL2;
    input_pin[2] = INVAL3;
    input_pin[3] = INVAL4;
} // init_input //



/*!
    \brief Returns the current status for one of the units
    
    \param[in] UNIT The unit to get the status for.
    \param[out] status The status of the unit.  Either ONA_ON or ONA_OFF.
    
    \return TRUE if returning the status was successful
            FALSE if returning the status was not successful (invalid params or
              and invalid unit).
*/
/* This gets pins according to unit 0, 1, 2, 3 */
static BOOL get_pin_state(UInt8 unit, SInt32 *status)
{
    UInt8 stat8;

    if(!status || (unit >= NUM_IO_UNITS))
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    switch(unit)
    {
        case 0:
        {
            stat8 = INVAL1;
            break;
        } // unit 0 case //

        case 1:
        {
            stat8 = INVAL2;
            break;
        } // unit 1 case //

        case 2:
        {
            stat8 = INVAL3;
            break;
        } // unit 2 case //

        case 3:
        {
            stat8 = INVAL4;
            break;
        } // unit 3 case //

        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    
    *status = (stat8 ? ONA_ON : ONA_OFF);
    return TRUE;
} // get_pin_state //


/*!
    \brief Sends a switch command message when a switch is flipped.
    
    \param[in] src_unit The source unit 
    \param[in] status The status of the pin
    \param[in] dst_unit The destination unit
    \param[in] enc_dst The device that is to receive this message.
    
    \return ONS_SUCCESS if the message was successfully queued.
            ONS_RSRC_FULL otherwise
*/
static one_net_status_t send_switch_status_change_msg(UInt8 src_unit, 
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
      #ifdef PEER
          , TRUE, src_unit
      #endif
      #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
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
    \brief Checks the state of the user pins and sends a switch message if a
      pin changed.

    \param void
    
    \return void
*/
static void check_input_pin(void)
{
	UInt8 changed_pin = 0xFF;
    if(input_pin[0] != INVAL1)
    {
        input_pin[0] = INVAL1;
        changed_pin = 0;
    }
    else if(input_pin[1] != INVAL2)
    {
        input_pin[1] = INVAL2;
        changed_pin = 1;
    }
    else if(input_pin[2] != INVAL3)
    {
        input_pin[2] = INVAL3;
        changed_pin = 2;
    }
    else if(input_pin[3] != INVAL4)
    {
        input_pin[3] = INVAL4;
        changed_pin = 3;
    }
    else
    {
        return;
    }
    
    send_switch_status_change_msg(changed_pin, input_pin[changed_pin],
      ONE_NET_DEV_UNIT, &MASTER_ENCODED_DID);
} // check_input_pin //


/*!
    \brief The entry point into the application

    \param void

    \return void
*/
int main(void)
{
    const UInt8 * PARAM_PTR = NULL;
    UInt16 param_len = 0;
    #ifdef PEER
    const UInt8* peer_param_ptr = NULL;
    UInt16 peer_param_len = 0;
    #endif
    one_net_status_t status = ONS_NOT_JOINED;

    INIT_PORTS();
    init_input();
    
    // set up the transceiver first since the CLKOUT from the transceiver will
    // be used as the main clock for the processor
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR(TRUE);
    
    #ifdef HAS_LEDS
        initialize_leds();
    #endif    
    
    INIT_TICK();
    ENABLE_GLOBAL_INTERRUPTS();
    FLASH_ERASE_CHECK();

    #ifdef PEER
    if(((PARAM_PTR = read_param(ONE_NET_CLIENT_FLASH_NV_DATA, &param_len))
      != NULL) && ((peer_param_ptr = read_param(ONE_NET_CLIENT_FLASH_PEER_DATA,
      &peer_param_len)) != NULL))
    #else
    if((PARAM_PTR = read_param(&param_len)) != NULL)
    #endif
    {
        #ifdef PEER
        status = one_net_client_init(PARAM_PTR, param_len, peer_param_ptr,
          peer_param_len);
        #else
        status = one_net_client_init(PARAM_PTR, param_len);
        #endif
    } // if the settings were successfully returned //
    
    if(status != ONS_SUCCESS)
    {
        one_net_client_look_for_invite(&DEV_KEY);
    } // else join a network //

    while(1)
    {
        // Dec. 3, 2011 -- commenting out
        #if 0
        if(tx_led_timer && tx_led_timer < one_net_tick())
        {
            TURN_OFF(TX_LED);
            tx_led_timer = 0;
        } // if time to turn off the tx led //
        
        if(rx_led_timer && rx_led_timer < one_net_tick())
        {
            TURN_OFF(RX_LED);
            rx_led_timer = 0;
        } // if time to turn off the rx led //
        #endif

        check_input_pin();
        one_net_client();
    } // main loop //
	
	return 0;
} // main //



//! @} simple_input_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} SIMPLE_INPUT
