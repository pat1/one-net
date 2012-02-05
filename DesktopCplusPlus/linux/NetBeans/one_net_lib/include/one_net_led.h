#ifndef _ONE_NET_LED_H
#define _ONE_NET_LED_H

#include "config_options.h"


#ifdef _HAS_LEDS


#include "one_net_types.h"
#include "one_net_status_codes.h"



//! \defgroup one_net_led Functionality for flashing LEDs.
//! @{

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
    \file one_net_led.h
    \brief led declarations.

    Declarations for functionality involving LEDS
*/



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
//                              PUBLIC VARIABLES
//! \defgroup one_net_led_pub_var
//! \ingroup one_net_led
//! @{

//! @} one_net_led_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_led_pub_func
//! \ingroup one_net_led
//! @{





/*!
    \brief Blinks the receive LED

    \param[in] duration time in milliseconds for each blink
    \param[in] num_times number of times to blink

    \return void
*/
void rx_led_blink(UInt16 duration, UInt8 num_times);


/*!
    \brief Blinks the transmit LED

    \param[in] duration time in milliseconds for each blink
    \param[in] num_times number of times to blink

    \return void
*/
void tx_led_blink(UInt16 duration, UInt8 num_times);


/*!
    \brief Blinks both the transmit and receive LEDS

    \param[in] duration time in milliseconds for each blink
    \param[in] num_times number of times to blink

    \return void
*/
void tx_and_rx_led_blink(UInt16 duration, UInt8 num_times);


/*!
    \brief Blinks the LEDS to show what stage of initialization the device
        is currently in.

    \param[in] status Current stage of initialization (success, failure, or in process)

    \return void
*/
void startup_status_led_blink(one_net_startup_status_t startup_status);


/*!
    \brief Initializes the LED ports and blinks the LEDS in a way that shows
        that the device is in the process of initializing.

    \return void
*/
void initialize_leds(void);


/*!
    \brief Sets the TX LED

    \param[in] on True if the LED should be turned on, False if the LED should
        be turned off.

    \return void
*/
void set_tx_led(BOOL on);


/*!
    \brief Toggles the TX LED

    \return void
*/
void toggle_tx_led(void);


/*!
    \brief Sets the RX LED

    \param[in] on True if the LED should be turned on, False if the LED should
        be turned off.

    \return void
*/
void set_rx_led(BOOL on);


/*!
    \brief Toggles the RX LED

    \return void
*/
void toggle_rx_led(void);





//! @} one_net_led_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_led_led

#endif // ifdef _HAS_LEDS

#endif // _ONE_NET_LED_H //

