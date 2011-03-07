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
    LED2_DIR = OUTPUT;
    LED3_DIR = OUTPUT;

    TURN_OFF(LED2);
    TURN_OFF(LED3);
} // init_ports_leds //


/*!
    \brief Sets the processor to use the main clock as the sytem clock.

    The Renesas R8C comes up using a very low speed clock.  This sets the
    internal high speed clock to be the system clock.  It is assumed that
    interrupts are disabled when this function is called.

    \param void

    \return void
*/
void high_speed_mode(void)
{
    prc0 = 1;                       // protect off
    cm06 = 0;                       // set CPU clock to sys clk div-by-one
    hra00 = 1;                      // enable high-speed internal clock
    asm("nop");                     // let it settle
    asm("nop");
    asm("nop");
    asm("nop");
    hra01 = 1;                      // set divide-by-4 mode
    ocd2 = 1;                       // select internal clock as system clock
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
    prc0 = 0;                       // protoect on
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
