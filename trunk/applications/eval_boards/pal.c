//! \defgroup PAL_EVAL Processor abstraction layer for ONE-NET switch example.
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
    \brief Contains implementation of R8C23 specific functions.

    Hardware specific functions for the R8C23.  Despite being specific for the
    processor, these functions are also specific based on the hardware.  For
    example, some boards may use an external clock as the main clock while
    others may use one of the internal oscillators (for the R8C series).
*/


#include <one_net/port_specific/config_options.h>


#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif 


#include <one_net/port_specific/pal.h>
#include <one_net/port_specific/tal.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_EVAL_const
//! \ingroup PAL_EVAL
//! @{
    
//! The rate the low speed oscillator runs at (in Hz)
#define LOW_SPEED_OSCILLATOR 125000

//! @} PAL_EVAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_EVAL_typedefs
//! \ingroup PAL_EVAL
//! @{

//! @} PAL_EVAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup PAL_EVAL_pri_var
//! \ingroup PAL_EVAL
//! @{

//! @} PAL_EVAL_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup PAL_EVAL_pri_func
//! \ingroup PAL_EVAL
//! @{

// In tick.c
extern void update_tick_count(const tick_t UPDATE);

//! @} PAL_EVAL_pri_func
//!                     PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup PAL_EVAL_pub_func
//! \ingroup PAL_EVAL
//! @{

/*!
    \brief Initializes the ports that MUST be initialized before anything else.
    
    \param void
    
    \return void
*/
void init_ports(void)
{
    //pu00 = 1;
    //pu14 = 1;
    //Pull-ups on all inputs
    pur0  = 0xff;
    pur1  = 0x33;
    INIT_PORTS_LEDS();
} // init_ports //


/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void init_ports_leds(void)
{
    prc2 = 1;
    LED2_DIR = OUTPUT;
    LED3_DIR = OUTPUT;
    prc2 = 0;

    TURN_OFF(LED2);
    TURN_OFF(LED3);
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
    fra00 = 1;                      // enable high speed oscillator
    
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

    cm06 = 0;                       // enable CM16, CM17    
    // no system clock division
    cm16 = 0;
    cm17 = 0;
    
    // divide the high speed oscillator by 2
    fra20 = 0;
    fra21 = 0;
    fra22 = 0;

    fra01 = 1;                      // system clock is internal high speed osc.
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
    \brief Puts the micro controller to sleep.

    The micro controller will sleep until an interrupt occurs or until ms
    milliseconds elapses.  Pass 0 to sleep until an interrupt occurs.

    This function will also need to update the tick count.

    \param[in] MS The number of milliseconds to sleep for

    \return void
*/
void processor_sleep(const UInt32 MS)
{
} // processor_sleep //

//! @} PAL_EVAL_pub_func
//!                     PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup PAL_EVAL_pri_func
//! \ingroup PAL_EVAL
//! @{

//! @} PAL_EVAL_pri_func
//!                     PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} PAL_EVAL

