//! addtogroup TICK
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
    \file tick.c
    \brief Renesas specific timing module.

    This module contains functionality associated with timing.
*/


// Nov. 28, 2011 -- temporarily reverting JMR's timer changes.  Will
// re-implement them again when I get more of a chance to look at them
// and everything else is working.


#include "sfr_r823.h"
#include "tick.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup TICK_const
//! \ingroup TICK
//! @{

//! @} TICK_const
//                                  CONSTANTS
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TICK_type_defs
//! \ingroup TICK
//! @{


//! The maximum value of a tick
#define TICK_MAX 4294967295


#define TICK_1MS 1                  //!< Number of ticks in a millisecond
#define TICK_1S  1000               //!< Number of ticks in a second


//! Number of times to loop through nop loop during the ms delay function.
#define NOP_COUNT_MS 325

//! Number of times to loop through nop loop during the 100s of us delay
//! function
#define NOP_COUNT_100S_US 45



//! @} TICK_type_defs
//                                  TYPEDEFS_END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TICK_pri_var
//! \ingroup TICK
//! @{

//! The number of ticks since the application started.
static tick_t tick_count = 0;

//! @} TICK_pri_var
//                              PRIVATE VARIABLES
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{


//! @} TICK_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TICK_pub_func
//! \ingroup TICK
//! @{
    

void init_tick(void)
{
    tck0_trbmr = 0;                 // Timer B count source = f1 
    tck1_trbmr = 0;

    // Setting main cycle timer
    // 16MHz * 1/1 * 256 * 250 = 4ms
    // 8MHz * 1/1 * 100 * 80 = 1ms
    trbpre = 200 - 1;               // Setting Prescaler X register 
    trbpr   = 100 - 1;              // Setting timer X register 
    
    trbmr &= 0xFC;                  // Timer B : timer mode 
    trbic = 0x05;                   // Int priority level = 5, clr request

    enable_tick_timer();            // Timer B count start flag = start
} // init_tick //
    

tick_t ms_to_tick(UInt32 ms)
{
    return ms / TICK_1MS;
} // tick_to_ms //
    

UInt32 tick_to_ms(tick_t num_ticks)
{
    return num_ticks * TICK_1MS;
} // ms_to_tick //


tick_t get_tick_count(void)
{
    return tick_count;
} // get_tick_count //


tick_t get_tick_diff(tick_t now, tick_t then)
{
    return (now < then ? (TICK_MAX - then) + now : now - then);
} // get_tick_diff //


void set_tick_count(tick_t new_tick_count)
{
    tick_count = new_tick_count;
}


void increment_tick_count(tick_t increment)
{
    tick_count += increment;
}


void delay_ms(UInt16 count)
{
    // Uses machine instructions to simulate delaying (doing nothing) for a
    // specified number of milliseconds.
    UInt16 i; 

    while(count-- != 0)
    {
        for(i = 0; i < NOP_COUNT_MS; i++)
        {
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
        } // nop loop //
    } // delay loop //
} // delay_ms //


void delay_100s_us(UInt16 count)
{
    // Uses machine instructions to simulate delaying (doing nothing) for a
    // specified number of milliseconds.
    UInt16 i;

    while(count-- != 0)
    {
        i = NOP_COUNT_100S_US;
        while(i--);
    } // delay loop //
} // delay_100us //


void polled_tick_update(void)
{
    #define TICK_INTERRUPT_FLAG ir_trbic

    if(TICK_INTERRUPT_FLAG)
    {
        tick_count++;
        TICK_INTERRUPT_FLAG = 0;
    } // if the tick interrupt had occurred //
} // polled_tick_update //


void enable_tick_timer(void)
{
    tstart_trbcr = 1;
}


void disable_tick_timer(void)
{
    tstart_trbcr = 0;
}


//! @} TICK_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{


/*!
    \brief Tick Interrupt handler.

    \param void

    \return void
*/
#pragma interrupt tick_timer_isr
void tick_timer_isr(void)
{
    tick_count++;
} // tick_timer //


//! @} TICK_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} TICK
