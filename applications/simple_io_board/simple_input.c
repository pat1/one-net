//! \defgroup SIMPLE_INPUT
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
    \file simple_input.c
    \brief Opto Input board.
    
    This file is the main application for a simple ONE-NET opto input device.

    \version 0.3
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
//! \defgroup simple_input_const
//! \ingroup SIMPLE_INPUT
//! @{

enum
{
    //! The time in ticks to leave the LEDs on
    EVAL_LED_ON_TIME = 50
};

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

//! Timer to turn off the transmit LED
static tick_t tx_led_timer = 0;

//! Timer to turn off the receive LED
static tick_t rx_led_timer = 0;

//! the voltage status reading
static UInt8 voltage_status = ONA_VOLTAGE_GOOD;

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

static BOOL get_pin_state(UInt8 unit, UInt16 *status);
static void check_input_pin(void);

//! @} simple_input_pri_func
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup simple_input_pub_func
//! \ingroup SIMPLE_INPUT
//! @{

void one_net_client_joined_network(const one_net_raw_did_t * const RAW_DID,
  const one_net_raw_did_t * const MASTER_DID)
{
} // one_net_client_joined_network //

/* Changed so that input_pin[0] is unit 1 */

BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    ona_msg_type_t msg_type;
    ona_msg_class_t msg_class;
    UInt8 src_unit, dst_unit;
    BOOL rv = FALSE;

    msg_type = get_msg_hdr(RX_PLD);
    msg_class = msg_type & ONA_MSG_CLASS_MASK;
    msg_type &= ~ONA_MSG_CLASS_MASK;

    switch(msg_type)
    {
        case ONA_SWITCH:
        {
            UInt16 status;
            UInt8 stat8;

            // get src unit and dst unit from payload
            // dst_unit has value 1, 2, 3, 4
            //
            if(ona_parse_switch(RX_PLD, &src_unit, &dst_unit, &status) !=
               ONS_SUCCESS || (dst_unit > ONE_NET_NUM_UNITS) || (dst_unit < 1))
            {
                break;
            } // if parsing the switch payload was not successful //

            switch(msg_class)
            {
                case ONA_QUERY:
                {
                    // units 1, 2, 3, 4
                    if(((rv = get_pin_state(dst_unit-1, &status)) != FALSE) && 
                      ona_send_switch_status(
                          dst_unit, src_unit, status, SRC_ADDR) == ONS_SUCCESS)
                    {
                        // Update the state of the pin that was checked since
                        // the message was successfully queued
                        input_pin[dst_unit-1] = status;
                        rv = TRUE;
                    } // if sending the status was successful //
                    break;
                } // ONA_QUERY case //

                default:
                {
                    break;
                } // default case //
            } // switch on msg class //
            break;
        } // SWITCH case //

        case ONA_UNIT_TYPE_COUNT:
        {
            if(msg_class != ONA_QUERY)
            {
                break;
            } // if an incorrect message class was received //

            if(ona_send_unit_type_count_status(SRC_ADDR) == ONS_SUCCESS)
            {
                rv = TRUE;
            } // if sending was successful //
            break;
        } // UNIT_TYPE_COUNT case //
        
        case ONA_UNIT_TYPE:
        {
            if(msg_class != ONA_QUERY
              || ona_parse_unit_type_query(RX_PLD, &dst_unit) != ONS_SUCCESS
              || ona_send_unit_type_status(dst_unit, SRC_ADDR) != ONS_SUCCESS)
            {
                break;
            } // if an incorrect message class was received || parsing failed //

            rv = TRUE;
            break;
        } // UNIT_TYPE case //

        case ONA_VOLTAGE:
        {
            voltage_status = READ_BATTERY_STATUS() >= LOW_BATTERY_THRESHOLD
              ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
            ona_send_voltage_simple_status(voltage_status, SRC_ADDR);
            rv = TRUE;
            break;
        } // VOLTAGE case //

        default:
        {
            break;
        } // default case //
    } // switch(msg_type) //

    return rv;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 * const DATA, const one_net_raw_did_t * const DST)
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
void init_input(void)
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
static BOOL get_pin_state(UInt8 unit, UInt16 *status)
{
    UInt8 stat8;

    if(!status || (unit >= _NUM_IO_UNITS))
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
    \brief Checks the state of the user pins and sends a switch message if a
      pin changed.

    \param void
    
    \return void
*/
/* dje: Changed so that input_pin[0] corresponds to unit 1
 *                      input_pin[1] is unit 2
 *                      etc.
 */
// Unit numbers 1, 2, 3, 4

/* rwm: Changed so that input_pin[0] corresponds to unit 0
 *                      input_pin[1] is unit 1
 *                      etc.
 */
// rwm: Unit numbers 0, 1, 2, 3
static void check_input_pin(void)
{
    if(input_pin[0] != INVAL1)
    {
        if(ona_send_switch_command(0, 0, INVAL1 ? ONA_ON : ONA_OFF, 0))
        {
            input_pin[0] = INVAL1;
        } // if scheduling the transaction was successful //
    } // if input1 changed //
    else if(input_pin[1] != INVAL2)
    {
        if(ona_send_switch_command(1, 0, INVAL2 ? ONA_ON : ONA_OFF, 0))
        {
            input_pin[1] = INVAL2;
        } // if scheduling the transaction was successful //
    } // if input2 changed //
    else if(input_pin[2] != INVAL3)
    {
        if(ona_send_switch_command(2, 0, INVAL3 ? ONA_ON : ONA_OFF, 0))
        {
            input_pin[2] = INVAL3;
        } // if scheduling the transaction was successful //
    } // if input3 changed //
    else if(input_pin[3] != INVAL4)
    {
        if(ona_send_switch_command(3, 0, INVAL4 ? ONA_ON : ONA_OFF, 0))
        {
            input_pin[3] = INVAL4;
        } // if scheduling the transaction was successful //
    } // if input4 changed //
} // check_input_pin //


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
    init_input();
    
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
        one_net_copy_to_nv_param(PARAM_PTR, param_len);

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

        check_input_pin();
        one_net_client();
    } // main loop //
} // main //

//! @} simple_input_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} SIMPLE_INPUT
