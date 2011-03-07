//! addtogroup TICK
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
    \file tick.c
    \brief Renesas specific timing module.

    This module contains functionality associated with timing.
*/

#include "sfr_r81B.h"
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

//! @} TICK_type_defs
//                                  TYPEDEFS_END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TICK_pri_var
//! \ingroup TICK
//! @{

//! The number of ticks since the application started.  This is not static since
//! it will be derived by pal_tick
tick_t tick_count = 0;

// The number of ticks since the application started.  "Derived" from tick.c
extern tick_t tick_count;


//! @} TICK_pri_var
//                              PRIVATE VARIABLES
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{

void update_tick_count(const tick_t UPDATE);

//! @} TICK_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TICK_pub_func
//! \ingroup TICK
//! @{

/*!
    \brief Initialize the tick timer.

    1 tick = 1ms.

    \param void

    \return void
*/
void init_tick(void)
{
    txck0 = 0;                      // Timer X count source = f1 
    txck1 = 0;

    // Setting main cycle timer
    // 16MHz * 1/1 * 256 * 250 = 4ms
    // 8MHz * 1/1 * 100 * 80 = 1ms
    prex = 250 - 1;             // Setting Prescaler X register 
    tx   = 78 - 1;              // Setting timer X register 
    txmr &= 0x04;                   // Timer X : timer mode 
    txic = 5;                       // Interrupt priority level = 5 

    ir_txic = 0;                    // Interrupt request flag clear 
    ENABLE_TICK_TIMER();            // Timer X count start flag = start
} // init_tick //


/*!
    \brief Returns the current tick count.

    \param void

    \return The current tick count.
*/
tick_t get_tick_count(void)
{
    return tick_count;
} // get_tick_count //


/*!
    \brief Polls the tick ir line to see if the tick count needs to be updated.

    This should only be called when the tick interrupt (or global interrupts)
    have been disabled.

    \param void

    \return void
*/
void polled_tick_update(void)
{
    #define TICK_INTERRUPT_FLAG ir_txic

    if(TICK_INTERRUPT_FLAG)
    {
        tick_count++;
        TICK_INTERRUPT_FLAG = 0;
    } // if the tick interrupt had occurred //
} // polled_tick_update //


/*!
    \brief Delay for a specified number of milliseconds.

    Uses machine instructions to simulate delaying (doing nothing) for a
    specified number of milliseconds.

    \param[in] count The number of ms to delay for.

    \return void
*/
void delay_ms(UInt16 count)
{
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


/*!
    \brief Delay for a specified number of 100s of micro seconds.

    Uses machine instructions to simulate delaying (doing nothing) for a
    specified number of 100s of micro seconds.

    \param[in] count The number of 100s of us to delay for.

    \return void
*/
void delay_100s_us(UInt16 count)
{
    UInt16 i;

    while(count-- != 0)
    {
        i = NOP_COUNT_100S_US;
        while(i--);
    } // delay loop //
} // delay_100us //

//! @} TICK_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{

/*!
    \brief Updates the tick count
    
    This function is considered private, but it is used in processor_sleep in
    pal.c, but it is not to be used anywhere else.

    \param[in] UPDATE The number of ticks to update the tickcount by

    \return void
*/
void update_tick_count(const tick_t UPDATE)
{
    tick_count += UPDATE;
} // update_tick_count //


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
