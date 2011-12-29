//! \defgroup PAL_SW_EX Processor abstraction layer for ONE-NET switch example.
//! \ingroup PAL
//! @{

/*
    Copyright (c) 2010, Threshold Corporation
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
    \file pal.c (for switch example)
    \brief Contains implementation of R8C1B specific functions.

    Hardware specific functions for the R8C1B.  Despite being specific for the
    processor, these functions are also specific based on the hardware.  For
    example, some boards may use an external clock as the main clock while
    others may use one of the internal oscillators (for the R8C series).
*/

#include "pal.h"
#include "spi.h"
#include "tal.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_SW_EX_const
//! \ingroup PAL_SW_EX
//! @{

//! The rate the low speed oscillator runs at (in Hz)
#define LOW_SPEED_OSCILLATOR 125000

//! Low battery voltage threshold.  This is used when reading the battery status
const UInt16 BATTERY_THRESHOLD = 0x01;

//! Low voltage threshold.
const UInt16 VOLTAGE_THRESHOLD = 0x01;

//! @} PAL_SW_EX_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_SW_EX_typedefs
//! \ingroup PAL_SW_EX
//! @{

//! @} PAL_SW_EX_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup PAL_SW_EX_pri_var
//! \ingroup PAL_SW_EX
//! @{

//! @} PAL_SW_EX_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup PAL_SW_EX_pri_func
//! \ingroup PAL_SW_EX
//! @{

// In tick.c
extern void update_tick_count(const tick_t UPDATE);

//! @} PAL_SW_EX_pri_func
//!                     PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup PAL_SW_EX_pub_func
//! \ingroup PAL_SW_EX
//! @{

/*!
    \brief Initializes the ports that MUST be initialized before anything else.

    \param void

    \return void
*/
void init_ports(void)
{
    INIT_PORTS_LEDS();
    INIT_SPI(TRUE, 0, 0, TRUE);

    // Initialize voltage detection circuit
    prc3 = 1;
    vca27 = 1;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    vw2c1 = 1;
    vw2c6 = 0;
    vw2c2 = 0;
    prc3 = 0;
} // init_ports //


/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void init_ports_leds(void)
{
    LED_DIR = OUTPUT;

    TURN_OFF(LED);
} // init_ports_leds //


/*!
    \brief Sets the processor to use the main clock as the sytem clock.

    The Renesas R8C comes up using a very low speed clock.  This sets the main
    clock (external input to p4_6) as the system clock.  It is assumed that
    interrupts are disabled when this function is called.

    \param void

    \return void
*/
void high_speed_mode(void)
{
    prc0 = 1;                       // protect off
    cm13 = 1;                       // p4_6, 4_7 to XIN-XOUT
    cm05 = 0;                       // main clock oscillates

    // delay 1 ms to make sure the oscillator settles.  Since we are not
    // running at the speed used to calculate the constants for the delay, this
    // will actually take longer than 1ms running on the low speed internal
    // oscillator, but we don't want to make code size bigger, and this is only
    // done when the device starts up.
    delay_ms(1);
    asm("nop");                     // let it settle
    asm("nop");
    asm("nop");
    asm("nop");

    cm06 = 0;                       // enable cm16 & cm17 divide bits

    // divide system clock by 1
    cm16 = 0;
    cm17 = 0;

    ocd2 = 0;                       // select main clock as system clock
    prc0 = 0;                       // protect on
} // high_speed_mode //


/*!
    \brief Sets the processor to use the low speed internal oscillator

    \param void

    \return void
*/
void low_speed_mode(void)
{
    prc0 = 1;                       // protect off
    ocd2 = 1;                       // select internal oscillator
    cm05 = 1;                       // main clock stops
    prc0 = 0;                       // protect on
} // low_speed_mode //


/*!
    \brief Returns the battery status reported by the ADI

    \param void

    \return The battery status.
*/
UInt16 read_battery(void)
{
    return vca13 ? 0x01 : 0x00;
} // read_battery //


/*!
    \brief Puts the micro controller to sleep.

    The micro controller will sleep until an interrupt occurs or until ms
    milliseconds elapses.  Pass 0 to sleep until an interrupt occurs.

    This function will also need to update the tick count.

    \param[in] MS The number of milliseconds to sleep for

    \return void
*/
void processor_sleep(const UInt32 MS)
{
    enum
    {
        // value (+1) to set tx reg to for very short times
        TX_SHORT = 125,

        // value (+1) to set tx to for longer durations
        TX_LONG = 250
    };

    tick_t delay, ticks_slept, tick;

    // used to include scaling to ticks when calculating how long the processor
    // really slept for
    UInt16 scale = 0;

    UInt8 last_sw_pos = SW;

    // timer settings
    UInt8 timer_src_multiplier;
    UInt8 n, m;                     // from data sheet to represent prex & tx

    DISABLE_GLOBAL_INTERRUPTS();
    low_speed_mode();
    DISABLE_TRANSCEIVER();

    delay = MS ? MS : TICK_1S;

    while(SW == last_sw_pos && (delay || !MS))
    {
        if(delay < 256)
        {
            txck0 = 0;              // Timer X count source = f1
            txck1 = 0;

            // don't need to scale to ticks when computing ticks_slept
            scale = 0;

            timer_src_multiplier = 1;
            n = delay;              // Setting Prescaler X register
            m = TX_SHORT;           // Setting timer X register
        } // if delay < 256 //
        else if(delay <= 1000)
        {
            UInt32 temp;

            txck0 = 1;              // Timer X count source = f2
            txck1 = 1;

            // need to scale to ticks when computing ticks_slept
            scale = 1;

            timer_src_multiplier = 2;

            // 2 is from f2 (see txck)
            temp = (UInt32)LOW_SPEED_OSCILLATOR / (UInt32)TICK_1S
              * (UInt32)delay / (UInt32)timer_src_multiplier / (UInt32)TX_LONG;
            n = (UInt8)temp;
            m = TX_LONG;            // Setting timer X register
        } // else if delay <= 1000 //
        else
        {
            txck0 = 1;              // Timer X count source = f2
            txck1 = 1;

            // need to scale to ticks when computing ticks_slept
            scale = 1;

            timer_src_multiplier = 2;
            n = TX_LONG;           // Setting Prescaler X register
            m = TX_LONG;           // Setting timer X register
        } // else //

        tick = get_tick_count();
        prex = n - 1;
        tx = m - 1;
        ENABLE_TICK_TIMER();
        SET_SW_INTERRUPT_EDGE(!SW);
        ENABLE_SW_INTERRUPT();
        ENABLE_GLOBAL_INTERRUPTS();

        if(SW == last_sw_pos)
        {
            // make sure one last time that the switch has not been moved
            asm("WAIT");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
        } // if the switch has not moved //

        DISABLE_GLOBAL_INTERRUPTS();
        DISABLE_SW_INTERRUPT();
        DISABLE_TICK_TIMER();

        tick = get_tick_count() - tick;
        if(!tick)
        {
            n -= prex;
            m -= tx;
        } // if a timer interrupt did not happen //

        // update tickcount & delay with the length of time the processor
        // was not running.
        ticks_slept = (tick_t)m * (tick_t)n * (tick_t)TICK_1S
          * (tick_t)timer_src_multiplier / (tick_t)LOW_SPEED_OSCILLATOR;

        if(MS)
        {
            delay -= (ticks_slept < delay ? ticks_slept + 1 : delay);
        } // if doing a timed sleep //

        // subtract the number of times the tick has changed since we started
        // the sleep process
        if(ticks_slept > tick)
        {
            update_tick_count(ticks_slept - tick);
        } // if the device slept //
    } // if the switch has not moved && there is more time to sleep //

    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR();
    INIT_TICK();
    ENABLE_GLOBAL_INTERRUPTS();
} // processor_sleep //

//! @} PAL_SW_EX_pub_func
//!                     PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup PAL_SW_EX_pri_func
//! \ingroup PAL_SW_EX
//! @{

//! @} PAL_SW_EX_pri_func
//!                     PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} PAL_SW_EX
