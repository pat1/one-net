//! \defgroup SIMPLE_RELAY
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    \file output_example.c
    \brief Quad relay output board.
    
    This file is the main application for a simple ONE-NET quad relay output
    device.

    \version 0.4
*/

#include "client_util.h"
#include "one_net_client.h"
#include "one_net_port_specific.h"
#include "pal.h"
#include "ona_voltage_simple.h"
#include "ona_switch.h"
#include "tal.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup simple_relay_const
//! \ingroup SIMPLE_RELAY
//! @{

enum
{
    //! The time in ticks to leave the LEDs on
    EVAL_LED_ON_TIME = 50
};

//! The unique key for this device.  This key is used when this device is added
//! to the network.
// Changed to be able to use new cli invite command in the master.
// The first two groups are duplicated in the upper two
//const one_net_xtea_key_t DEV_KEY = {'4', '8', 'u', 't', 'p', 'u', 't',
  //'r', 'e', '7', 'a', 'y', '2', '2', '2', '2'};
const one_net_xtea_key_t DEV_KEY = {
    '4', '8', 'u', 't', 
    'p', 'u', 't', '7',
    '4', '8', 'u', 't', 
    'p', 'u', 't', '7'
};

const ona_unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES]
  = {{ONA_OUTLET, ONE_NET_NUM_UNITS}};

//! @} simple_relay_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup simple_relay_typedefs
//! \ingroup SIMPLE_RELAY
//! @{

//! @} simple_relay_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup simple_relay_pri_var
//! \ingroup SIMPLE_RELAY
//! @{

//! Timer to turn off the transmit LED
static tick_t tx_led_timer = 0;

//! Timer to turn off the receive LED
static tick_t rx_led_timer = 0;

//! the voltage status reading
static UInt8 voltage_status = ONA_VOLTAGE_GOOD;

//! @} simple_relay_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup simple_relay_pri_func
//! \ingroup SIMPLE_RELAY
//! @{

static BOOL set_output(UInt8 unit, BOOL ON);
static BOOL get_output(UInt8 unit, UInt16 *status);
static BOOL toggle_output(UInt8 unit);

//! @} simple_relay_pri_func
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup simple_relay_pub_func
//! \ingroup SIMPLE_RELAY
//! @{

void one_net_client_joined_network(const one_net_raw_did_t *RAW_DID,
  const one_net_raw_did_t *MASTER_DID)
{
} // one_net_client_joined_network //


BOOL one_net_client_handle_status_msg(const UInt8* const raw_pld, const ona_msg_class_t msg_class,
       const ona_msg_type_t msg_type, const UInt8 src_unit,
       const UInt8 dst_unit, const UInt16 msg_data,
	   const one_net_raw_did_t* const src_addr,
	   on_nack_rsn_t* const nack_reason, on_ack_nack_handle_t* const ack_nack_handle,
	   ack_nack_payload_t* const ack_nack_payload)
{
    return TRUE;
}


BOOL one_net_client_handle_ack_nack_response(const ona_msg_type_t msg_type,
    UInt8* const payload, const UInt8 payload_len, BOOL* const need_txn_payload,
    UInt8* const retries,
	const one_net_raw_did_t* const src_did, on_nack_rsn_t* const nack_reason,
	on_ack_nack_handle_t* const ack_nack_handle,
	ack_nack_payload_t* const ack_nack_payload)
{
    return TRUE;
}


BOOL one_net_client_handle_single_pkt(const UInt8* const raw_pld, const ona_msg_class_t msg_class,
       ona_msg_type_t* const msg_type, const UInt8 src_unit,
       const UInt8 dst_unit, UInt16* const msg_data,
       const one_net_raw_did_t* const SRC_ADDR,
	   BOOL* const useDefaultHandling, on_nack_rsn_t* const nack_reason,
	   on_ack_nack_handle_t* const ack_nack_handle,
	   ack_nack_payload_t* const ack_nack_payload)
{
    BOOL rv = TRUE;
    *useDefaultHandling = TRUE;

    if(*nack_reason != ON_NACK_RSN_NO_ERROR)
    {
        return TRUE;
    }

    switch(*msg_type)
    {
        case ONA_SWITCH:
        {
            switch(msg_class)
            {
                case ONA_COMMAND:
                {
                    if(*msg_data == ONA_TOGGLE)
                    {
                        rv = toggle_output(dst_unit);
                    } // if a toggle was received //
                    else
                    {
                        rv = set_output(dst_unit, *msg_data == ONA_ON);
                    } // else an on or off message //
                } // COMMAND case - fall through to get status //

                case ONA_QUERY:
                #ifdef _POLL
                case ONA_POLL:
                #endif
                {
                    if(rv)
                    {
                        rv = get_output(dst_unit, msg_data);
                    }
                    
                    if(!rv)
                    {
                        // we don't know why it failed.  Nack it with
                        // a general reason
                        *ack_nack_handle = ON_NACK;
                        *nack_reason = ON_NACK_RSN_GENERAL_ERR;
                    }
                } // ONA_QUERY case, ON_POLL, ON_COMMAND //
            } // switch on msg class //
            break;
        } // SWITCH case //

        case ONA_UNIT_TYPE_COUNT:
        {
            #ifdef _POLL
            if(msg_class != ONA_QUERY && msg_class != ONA_POLL)
            #else
            if(msg_class != ONA_QUERY)
            #endif
            {
                *ack_nack_handle = ON_NACK;
                *nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            } // if an incorrect message class was received //
            else
            {
                *msg_data = (ONE_NET_NUM_UNITS << 8) + ONE_NET_NUM_UNIT_TYPES;
            }
            break;
        } // UNIT_TYPE_COUNT case //
        
        case ONA_UNIT_TYPE:
        {
            *useDefaultHandling = FALSE;
            // TODO - add poll?
            if(msg_class != ONA_QUERY)
            {
                *ack_nack_handle = ON_NACK;
                *nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            } // if an incorrect message class was received || parsing failed //

            else if(ona_send_unit_type_status(dst_unit, SRC_ADDR) != ONS_SUCCESS)
            {
                // not sure what happened, so we'll send them a general error
                *ack_nack_handle = ON_NACK;
                *nack_reason = ON_NACK_RSN_GENERAL_ERR;             
            }

            break;
        } // UNIT_TYPE case //

        case ONA_VOLTAGE:
        {
            *useDefaultHandling = FALSE;
            voltage_status = READ_BATTERY_STATUS() >= LOW_BATTERY_THRESHOLD
              ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
            ona_send_voltage_simple_status(voltage_status, SRC_ADDR);
            break;
        } // VOLTAGE case //

        default:
        {
            *ack_nack_handle = ON_NACK;
            *nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        } // default case //
    } // switch(msg_type) //

    return TRUE;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(one_net_status_t STATUS,
  const UInt8 *DATA, const one_net_raw_did_t *DST)
{
} // one_net_client_single_txn_status //


void one_net_client_client_remove_device(void)
{
    clr_flash();
    one_net_client_look_for_invite(&DEV_KEY,
      ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32);
} // one_net_client_client_remove_device //


/*!
    \brief Turns on the transmit LED
    
    \param void
    
    \return void
*/
void turn_on_tx_led(void)
{
    TURN_ON(TX_LED);
    tx_led_timer = one_net_tick() + EVAL_LED_ON_TIME;
} // turn_on_tx_led //


/*!
    \brief Turns on the receive LED
    
    \param void
    
    \return void
*/
void turn_on_rx_led(void)
{
    TURN_ON(RX_LED);
    rx_led_timer = one_net_tick() + EVAL_LED_ON_TIME;
} // turn_on_rx_led //

//! @} simple_relay_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup simple_relay_pri_func
//! \ingroup SIMPLE_RELAY
//! @{

/*!
    \brief Sets the output for the given unit.
    
    \param[in] unit The unit to set.
    \param[in] ON   TRUE if setting the output pin
                    FALSE if clearing the output pin

    \return TRUE if the operation was successful
            FALSE if the operation was not successful (such as an invalid unt)
*/
static BOOL set_output(UInt8 unit, BOOL ON)
{
    switch(unit)
    {
        #if _NUM_IO_UNITS > 0
            case 0:
            {
                OUTPUT1 = ON;
                break;
            } // unit 0 case //
        #endif // #if _NUM_IO_UNITS > 0 //

        #if _NUM_IO_UNITS > 1
            case 1:
            {
                OUTPUT2 = ON;
                break;
            } // unit 1 case //
        #endif // #if _NUM_IO_UNITS > 1 //

        #if _NUM_IO_UNITS > 2
            case 2:
            {
                OUTPUT3 = ON;
                break;
            } // unit 3 case //
        #endif // #if _NUM_IO_UNITS > 2 //

        #if _NUM_IO_UNITS > 3
            case 3:
            {
                OUTPUT4 = ON;
                break;
            } // unit 4 case //
        #endif // #if _NUM_IO_UNITS > 3 //

        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    
    return TRUE;
} // set_output //


/*!
    \brief Toggles the output for the given unit.
    
    \param[in] unit The unit to toggle.

    \return TRUE if the operation was successful
            FALSE if the operation was not successful (such as an invalid unt)
*/
static BOOL toggle_output(UInt8 unit)
{
    switch(unit)
    {
        #if _NUM_IO_UNITS > 0
            case 0:
            {
                TOGGLE(OUTPUT1);
                break;
            } // unit 0 case //
        #endif // #if _NUM_IO_UNITS > 0 //

        #if _NUM_IO_UNITS > 1
            case 1:
            {
                TOGGLE(OUTPUT2);
                break;
            } // unit 1 case //
        #endif // #if _NUM_IO_UNITS > 1 //

        #if _NUM_IO_UNITS > 2
            case 2:
            {
                TOGGLE(OUTPUT3);
                break;
            } // unit 3 case //
        #endif // #if _NUM_IO_UNITS > 2 //

        #if _NUM_IO_UNITS > 3
            case 3:
            {
                TOGGLE(OUTPUT4);
                break;
            } // unit 4 case //
        #endif // #if _NUM_IO_UNITS > 3 //

        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    
    return TRUE;
} // toggle_output //


/*!
    \brief Returns the current status for one of the units
    
    \param[in] unit The unit to get the status for.
    \param[out] status The status of the unit.  Either ONA_ON or ONA_OFF.
    
    \return TRUE if returning the status was successful
            FALSE if returning the status was not successful (invalid params or
              and invalid unit).
*/
static BOOL get_output(UInt8 unit, UInt16 *status)
{
    UInt8 stat8; /* local variable for get_output() */

    if(!status)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    switch(unit)
    {
        #if _NUM_IO_UNITS > 0
            case 0:
            {
                stat8 = OUTPUT1;
                break;
            } // unit 0 case //
        #endif // #if _NUM_IO_UNITS > 0 //

        #if _NUM_IO_UNITS > 1
            case 1:
            {
                stat8 = OUTPUT2;
                break;
            } // unit 1 case //
        #endif // #if _NUM_IO_UNITS > 1 //

        #if _NUM_IO_UNITS > 2
            case 2:
            {
                stat8 = OUTPUT3;
                break;
            } // unit 2 case //
        #endif // #if _NUM_IO_UNITS > 2 //

        #if _NUM_IO_UNITS > 3
            case 3:
            {
                stat8 = OUTPUT4;
                break;
            } // unit 3 case //
        #endif // #if _NUM_IO_UNITS > 3 //
        
        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    *status = stat8;
    return TRUE;
} // get_output //


/*!
    \brief The entry point into the application

    \param void

    \return void
*/
void main(void)
{
    const UInt8 * PARAM_PTR;
    
    UInt16 param_len;

    INIT_PORTS();
    
    // set up the transceiver first since the CLKOUT from the transceiver will
    // be used as the main clock for the processor
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR();
    INIT_TICK();
    ENABLE_GLOBAL_INTERRUPTS();
    FLASH_ERASE_CHECK();

    if((PARAM_PTR = read_param(&param_len)) != NULL)
    {
        one_net_client_init(PARAM_PTR, param_len);
    } // if the settings were successfully returned //
    else
    {
        one_net_client_look_for_invite(&DEV_KEY,
          ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32);
    } // else join a network //

    while(1)
    {
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

        one_net_client();
    } // main loop //
} // main //

//! @} simple_relay_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} SIMPLE_RELAY
