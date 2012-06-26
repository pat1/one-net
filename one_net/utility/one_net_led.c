//! \addtogroup one_net_led
//! @{

#include "config_options.h"
#ifdef HAS_LEDS


/*
    Copyright (c) 2011, Threshold Corporation
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
    \file one_net_led.c
    \brief Basis for led implementation.

    Generic functions involving LEDs.  A port- and hardware-specific file
    called led_hal.h should be written.  Contained within this file there should
    be a function or a macro called INIT_PORTS_LEDS(), which initializes the
    LED ports defines "TOGGLE_LED(LED)", "LED_TURN_ON(LED)" and "LED_TURN_OFF(LED)"
    macros, and defines "TX_LED", and "RX_LED" pins.  Upon startup, the APPLICATION
    code should call the function initialize_leds() initialize_leds() will call
    INIT_PORTS_LEDS().
*/


#include "one_net_led.h"
#include "led_hal.h"
#include "tick.h"
#include "one_net_status_codes.h"
#include "config_options.h"

// TODO -- this is a bit messy.  Find a better #define test.
#if defined(_R8C_TINY) && !defined(QUAD_OUTPUT)
    #pragma section program program_high_rom
#endif // if _R8C_TINY and not a 16K chip //


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_led_const
//! \ingroup one_net_led
//! @{

//! @} one_net_led_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_led_typedefs
//! \ingroup one_net_led
//! @{



//! @} one_net_led_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup one_net_led_pri_var
//! \ingroup one_net_led
//! @{

//! @} one_net_led_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup one_net_led_pri_func
//! \ingroup one_net_led
//! @{

//! @} one_net_led_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup one_net_led_pub_func
//! \ingroup one_net_led
//! @{


void rx_led_blink(UInt16 duration, UInt8 num_times)
{
    // num_times = 0 means infinite
    UInt8 i;

    for(i = num_times; num_times == 0 || i > 0; i--)
    {
        LED_TURN_ON(RX_LED);
        delay_ms(duration);
        LED_TURN_OFF(RX_LED);
        if(num_times == 0 || i > 1)
        {
            delay_ms(duration);
        }
    }
}


void tx_led_blink(UInt16 duration, UInt8 num_times)
{
    // num_times = 0 means infinite
    UInt8 i;

    for(i = num_times; num_times == 0 || i > 0; i--)
    {
        LED_TURN_ON(TX_LED);
        delay_ms(duration);
        LED_TURN_OFF(TX_LED);
        if(num_times == 0 || i > 1)
        {
            delay_ms(duration);
        }
    }
}


void tx_and_rx_led_blink(UInt16 duration, UInt8 num_times)
{
    // num_times = 0 means infinite
    UInt8 i;

    for(i = num_times; num_times == 0 || i > 0; i--)
    {
        LED_TURN_ON(TX_LED);
        LED_TURN_ON(RX_LED);
        delay_ms(duration);
        LED_TURN_OFF(TX_LED);
        LED_TURN_OFF(RX_LED);
        
        if(num_times == 0 || i > 1)
        {
            delay_ms(duration);
        }
    }
}


void startup_status_led_blink(one_net_startup_status_t startup_status)
{
    switch(startup_status)
    {
        case ON_STARTUP_IN_PROGRESS:
        {
            // three quick blinks of both LEDS
            tx_and_rx_led_blink(50, 3);
            break;
        }

        case ON_STARTUP_FAIL:
        {
            // infinite blinking of both LEDS once a second
            // three quick blinks of both LEDS
            tx_and_rx_led_blink(1000, 0);
            break;
        }

        case ON_STARTUP_SUCCESS:
        {
            // three quick blinks of the TX_LED, then a short pause, then
            // three quick blinks of the RX_LED
            tx_led_blink(50, 3);
            delay_ms(50);
            rx_led_blink(50, 3);
        }
    }
}


void initialize_leds(void)
{
    INIT_PORTS_LEDS();
    startup_status_led_blink(ON_STARTUP_IN_PROGRESS);
}


void set_tx_led(BOOL on)
{
    if(on)
    {
        LED_TURN_ON(TX_LED);
    }
    else
    {
        LED_TURN_OFF(TX_LED);
    }
}


void toggle_tx_led(void)
{
    LED_TOGGLE(TX_LED);
}


void set_rx_led(BOOL on)
{
    if(on)
    {
        LED_TURN_ON(RX_LED);
    }
    else
    {
        LED_TURN_OFF(RX_LED);
    }
}


void toggle_rx_led(void)
{
    LED_TOGGLE(RX_LED);
}



//! @} one_net_led_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup one_net_led_pri_func
//! \ingroup one_net_led
//! @{

//! @} one_net_led_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} one_net_led



#endif // ifdef HAS_LEDS //
