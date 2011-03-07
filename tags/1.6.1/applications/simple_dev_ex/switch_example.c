//! \defgroup SW_EXAMPLE
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
    \file switch_example.c
    \brief Application example that demonstrates a ONE-NET switch.
    
    This file is the main application for demonstrating a ONE-NET switch.  It
    is abstracted to not depend on the hardware layout.
    
    \version 0.6
*/

#include "client_util.h"
#include "one_net_client.h"
#include "one_net_port_specific.h"
#include "pal.h"
#include "voltage_simple.h"
#include "switch.h"
#include "tal.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup SW_EX_const
//! \ingroup SW_EXAMPLE
//! @{

enum
{
    LED_DURATION = 50               //!< The duration the led stays on in ticks
};

//! The unique key for this device.  This key is used when this device is added
//! to the network.  Note that this is a very bad key, and is used only in this
//! example.
const one_net_xtea_key_t DEV_KEY = {'S', 'I', 'M', 'P', '7', 'E', 'S',
  'W', 'I', 'T', 'C', 'H', 'E', 'X', '2', '2'};

const unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES]
  = {{SIMPLE_SWITCH, ONE_NET_NUM_UNITS}};

//! @} SW_EX_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup SW_EX_typedefs
//! \ingroup SW_EXAMPLE
//! @{

//! @} SW_EX_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup SW_EX_pri_var
//! \ingroup SW_EXAMPLE
//! @{

static tick_t led_off_time = 0;

//! the battery status reading
static UInt8 battery_status = ONA_VOLTAGE_GOOD;

//! @} SW_EX_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup SW_EX_pri_func
//! \ingroup SW_EXAMPLE
//! @{

//! @} SW_EX_pri_func
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup SW_EX_pub_func
//! \ingroup SW_EXAMPLE
//! @{

BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    ona_msg_type_t msg_type;
    ona_msg_class_t msg_class;
    UInt8 src_unit, dst_unit;
    BOOL rv = FALSE;

    msg_type = one_net_byte_stream_to_int16(RX_PLD);
    msg_class = msg_type & ONA_MSG_CLASS_MASK;
    msg_type &= ~ONA_MSG_CLASS_MASK;

    switch(msg_type)
    {
        case ONA_SWITCH:
        {
            switch_status_t status;

            if(parse_switch(&(RX_PLD[sizeof(msg_type)]),
              RX_PLD_LEN - sizeof(msg_class), &src_unit, &dst_unit, 
              &status) != ONS_SUCCESS)
            {
                break;
            } // if parsing the switch payload was not successful //

            switch(msg_class)
            {
                case ONA_QUERY:
                {
                    if(send_switch_status(0, src_unit, SW, SRC_ADDR)
                      == ONS_SUCCESS)
                    {
                        rv = TRUE;
                    } // if sending the status was successful //
                    break;
                } // QUERY case //

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

            if(send_unit_type_count_status(SRC_ADDR) == ONS_SUCCESS)
            {
                rv = TRUE;
            } // if sending was successful //
            break;
        } // UNIT_TYPE_COUNT case //
        
        case ONA_UNIT_TYPE:
        {
            if(msg_class != ONA_QUERY
              || parse_unit_type_query(&(RX_PLD[sizeof(msg_type)]),
              RX_PLD_LEN - sizeof(msg_class), &dst_unit) != ONS_SUCCESS
              || send_unit_type_status(dst_unit, SRC_ADDR) != ONS_SUCCESS)
            {
                break;
            } // if an incorrect message class was received || parsing failed //

            rv = TRUE;
            break;
        } // UNIT_TYPE case //

        case ONA_VOLTAGE:
        {
            battery_status = READ_BATTERY_STATUS() >= LOW_BATTERY_THRESHOLD
              ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
            send_voltage_simple_status(battery_status, SRC_ADDR);
            break;
        } // VOLTAGE case //

        default:
        {
            break;
        } // default case //
    } // switch(msg_type) //

    TURN_ON(LED);
    led_off_time = one_net_tick() + LED_DURATION;
    
    return rv;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 * const DATA, const one_net_raw_did_t * const DST)
{
} // one_net_client_single_txn_status //

//! @} SW_EX_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup SW_EX_pri_func
//! \ingroup SW_EXAMPLE
//! @{

/*!
    \brief The entry point into the application

    \param void

    \return void
*/
void main(void)
{
    const UInt8 * PARAM_PTR;

    tick_t sleep_time = 0;    
    UInt16 param_len;
    UInt8 sw_state;                 // switch state

    INIT_PORTS();

    // set up the transceiver first since the CLKOUT from the transceiver will
    // be used as the main clock for the processor
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR();
    INIT_TICK();
    ENABLE_GLOBAL_INTERRUPTS();

    if((PARAM_PTR = read_param(&param_len)))
    {
        one_net_client_init(PARAM_PTR, param_len);
    } // if the settings were successfully returned //
    else
    {
        one_net_client_look_for_invite(&DEV_KEY,
          ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32);
    } // else join a network //
    
    sw_state = !SW;                 // Force sending the switch status.
    while(1)
    {
        if(sw_state != SW)
        {
            sw_state = SW;
            send_switch_command(0, 0, sw_state, 0);

            TURN_ON(LED);
            led_off_time = one_net_tick() + LED_DURATION;
        } // if the switch state has changed //
        else if(led_off_time && led_off_time < one_net_tick())
        {
            TURN_OFF(LED);
            led_off_time = 0;
        } // if the LED needs to be turned off //

        sleep_time = one_net_client();

        if(sleep_time)
        {
            // The battery reading is not converted from it's raw form because
            // adding float operations will take too much memory.
            UInt16 battery_level = READ_BATTERY_STATUS();
            UInt8 old_battery_status = battery_status;

            battery_status = battery_level >= LOW_BATTERY_THRESHOLD
              ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
            
            if(battery_status != old_battery_status)
            {
                send_voltage_simple_status(battery_status, 0);
            } // if the battery status has changed //
            else if(!led_off_time)
            {
                // Does not sleep if the led off timer is active.  This is
                // demo functionality only.
                SLEEP(sleep_time);
            } // else the battery status has not changed //
        } // if the CLIENT can sleep //
    } // main loop //
} // main //

//! @} SW_EX_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} SW_EXAMPLE
