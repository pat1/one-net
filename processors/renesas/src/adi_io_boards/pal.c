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
#include "io_port_mapping.h"
#include "one_net_client_port_specific.h"
#include "tal.h"
#include "one_net_application.h" // for INPUT and OUTPUT
#include "hal_adi.h" // for TURN_OFF //
#include "tick.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_SW_EX_const
//! \ingroup PAL_SW_EX
//! @{
    
//! The rate the low speed oscillator runs at (in Hz)
#define LOW_SPEED_OSCILLATOR 125000

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
//                              PUBLIC VARIABLES
//! \defgroup PAL_SW_EX_pub_var
//! \ingroup PAL_SW_EX
//! @{


#ifdef _DEBUG
extern UInt8 global_interrupts_on;
#endif


//! @} PAL_SW_EX_pub_var
//                              PUBLIC VARIABLES END
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
void pal_init_ports(void)
{
    // dje: Enable all pull-ups
    pur0  = 0xcc;
    pur1  = 0x02;

    #ifdef QUAD_INPUT
        #if NUM_IO_UNITS > 0
            INPUT1_DIR = INPUT;
        #endif // #if _NUM_IO_UNITS > 0 //

        #if NUM_IO_UNITS > 1
            INPUT2_DIR = INPUT;
        #endif // #if _NUM_IO_UNITS > 1 //

        #if NUM_IO_UNITS > 2
            INPUT3_DIR = INPUT;
        #endif // #if _NUM_IO_UNITS > 2 //

        #if NUM_IO_UNITS > 3
            INPUT4_DIR = INPUT;
        #endif // #if _NUM_IO_UNITS > 3 //
    #elif defined(QUAD_OUTPUT) || defined(DUAL_OUTPUT)
        #if NUM_IO_UNITS > 0
            OUTPUT1_DIR = OUTPUT;
        #endif // #if _NUM_IO_UNITS > 0 //

        #if NUM_IO_UNITS > 1
            OUTPUT2_DIR = OUTPUT;
        #endif // #if _NUM_IO_UNITS > 1 //

        #if NUM_IO_UNITS > 2
            OUTPUT3_DIR = OUTPUT;
        #endif // #if _NUM_IO_UNITS > 2 //

        #if NUM_IO_UNITS > 3
            OUTPUT4_DIR = OUTPUT;
        #endif // #if _NUM_IO_UNITS > 3 //
    #else
        #error Unknown board type (see init_ports in pal.c)
    #endif
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

    The Renesas R8C comes up using a very low speed clock.  This sets the main
    clock (external input to p4_6) as the system clock.  It is assumed that
    interrupts are disabled when this function is called.

    \param void

    \return void
*/
void high_speed_mode(void)
{
    prc0 = 1;                       // protect off
    cm13 = 0;  //dje: Note that this must be zero to enable p4_7 as input
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
    prc0 = 0;                       // protoect on
} // low_speed_mode //


void pal_init_processor(BOOL high_speed)
{
    if(high_speed)
    {
        high_speed_mode();
    }
    else
    {
        low_speed_mode();
    }
}


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


/*!
    \brief Disables global interrupts

    \param void

    \return void
*/
void pal_disable_global_interrupts(void)
{
    asm("FCLR I");
    #ifdef _DEBUG
    global_interrupts_on = 0;
    #endif
}


/*!
    \brief Enables global interrupts

    \param void

    \return void
*/
void pal_enable_global_interrupts(void)
{
    asm("FSET I");
    #ifdef _DEBUG
    global_interrupts_on = 1;
    #endif
}


/*!
    \brief Exits the program.

    Disables global interrupts & stops program execution.

    \param void

    \return void
*/
void pal_exit(void)
{
    asm("FCLR I");
    asm("WAIT");
}



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
