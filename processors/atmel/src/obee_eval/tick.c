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
    \brief Atxmega256a3b specific timing module.

    This module contains functionality associated with timing.
	
    2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/



#include <avr/io.h>
#include <avr/interrupt.h>


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

#define TICK_1MS 1                     //!< Number of ticks in a millisecond
#define TICK_1S  1000                  //!< Number of ticks in a second


#define NOP_COUNT_MS  330
#define NOP_COUNT_100S_US 38


//! @} TICK_type_defs
//                                  TYPEDEFS_END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TICK_pri_var
//! \ingroup TICK
//! @{

//! The number of ticks since the application started.
volatile static tick_t tick_count = 0;
BOOL tick_flag = FALSE;

#ifdef DEBUGGING_TOOLS
	volatile static tick_t processor_tick_count = 0;
	UInt8 csdf = 1;
#else
	#ifdef CLOCK_SLOW_DOWN_FACTOR
		#if CLOCK_SLOW_DOWN_FACTOR > 1
			volatile static tick_t processor_tick_count = 0;
		#endif
	#endif
#endif


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


void init_tick_1(void)
{
   // Timer is a 1 milli second general timer
   // The system clock frequency  is 2 MHz
   // timer frequency = (2 * 10^6) / prescaler

   // timer time = (presaclar / 2) * 10^-6

   // counter value = (1 msec) /timer time
   // counter value = (1000 * 10^-6) / [(presaclar / 2) * 10^-6]

   // counter value = 1000 / (presaclar / 2)
   // counter value = (2 * 10^3) / prescalar

   // counter value = (2 / presaclar) * 10^3

   // prescalar = 1
   //counter value = 2 * 10^3 = 2000 = 0x7D0

   // prescalar = 2
   //counter value = (2 / 2) * 10^3 = 1000 = 0x3E8

   // prescalar = 4
   //counter value = (2 / 4) * 10^3 = 500 = 0x1F4

   // prescalar = 8
   //counter value = (2 / 8) * 10^3 = 250 = 0xFA

   // prescalar = 64
   //counter value = (2 / 64) * 10^3 = 31.25 = 0x1F

   // prescalar = 256
   //counter value = (2 / 256) * 10^3 = 15.625 = 0x0F

   // prescalar = 1024
   //counter value = (2 / 1024) * 10^3 = 1.953 =  0x01


   // initialize timer variables
   tick_flag = FALSE;
   tick_count = 0;

   TCC0.CTRLB = 0x00;         // normal mode

   // around 250 us tick time
   TCC0.PER = 63;     // use period
   TCC0.CCA = 63;     // use compare A register as top

   // Select clock source.
   TCC0.CTRLA = (TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV8_gc;

   //  Enable interrupt on overflow level LOW
   TCC0.INTCTRLA = (TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;

   // Set the Programmable Interrupt Controller multi level interrupts to LOW
   PMIC.CTRL |= PMIC_LOLVLEN_bm;

} // init_tick_1 //



//  This function sets up a general timer for a tick time of 1 miili second.
void init_tick(void)
{

   // Timer  is a 1 milli second general timer
   // The system clock frequency  is 11.0592 MHz
   // timer frequency = (11.0592 * 10^6) / prescaler
   // timer time = (presaclar / 11.0592) * 10^-6
   // counter value = (1 msec) /timer time
   // counter value = (1000 * 10^-6) / [(presaclar / 11.0592) * 10^-6]
   // counter value = 1000 / (presaclar/ 11.0592)
   // counter value = (11.0592 * 10^3) / (prescalar)

   // counter value = 10^3 * (11.0592 / (presaclar))

   // prescalar = 1
   //counter value = (11.0592) * 10^3 = 11059.2     = 0x2B33

   // prescalar = 2
   //counter value = (11.0592 / 2) * 10^3 = 5529.6 =  0x1599

   // prescalar = 4
   //counter value = (11.0592 / 4) * 10^3 = 2764.8  = 0x0ACC

   // prescalar = 8
   //counter value = (11.0592 / 8) * 10^3 = 1382.4  = 0x0566

   // prescalar = 64
   //counter value = (11.0592 / 64) * 10^3 = 172.8  = 0x00AC

   // prescalar = 256
   //counter value = (11.0592 / 256) * 10^3 = 43.2 =  0x002B

   // prescalar = 1024
   //counter value = (11.0592 / 1024) * 10^3 = 10.8 = 0x000A


   // initialize timer variables
   tick_flag = FALSE;
   tick_count = 0;

   //  disable interrupt on overflow level LOW
   TCC0.INTCTRLA &= ~TC0_OVFINTLVL_gm;

   TCC0.CTRLB = 0x00;         // normal mode

   // Set period/TOP value.
   // after fine tuning
   TCC0.PER = 0x002A;     // use period

   // Select clock source.
   TCC0.CTRLA = (TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV256_gc;

   //  Enable timer tick interrupt
   ENABLE_TICK_TIMER();

} // init_tick //


tick_t ms_to_tick(UInt32 ms)
{
    return ms * TICK_1MS;
} // tick_to_ms //


UInt32 tick_to_ms(tick_t num_ticks)
{
    return num_ticks / TICK_1MS;
} // ms_to_tick //


tick_t get_tick_count(void)
{
    tick_t local_tick_count = tick_count;
    return (local_tick_count);

//      return tick_count;
} // get_tick_count //


tick_t get_tick_diff(tick_t now, tick_t then)
{
    return (now < then ? ((tick_t)TICK_MAX - then) + now : now - then);
} // get_tick_diff //



void set_tick_count(tick_t new_tick_count)
{

    tick_count = new_tick_count;
	
    #ifdef DEBUGGING_TOOLS
	    processor_tick_count = tick_count * csdf;
    #else
		#ifdef CLOCK_SLOW_DOWN_FACTOR
			#if CLOCK_SLOW_DOWN_FACTOR > 1
				processor_tick_count = tick_count * CLOCK_SLOW_DOWN_FACTOR;
			#endif
		#endif
    #endif
	
}


void increment_tick_count(tick_t increment)
{
    tick_count += increment;

    #ifdef DEBUGGING_TOOLS
	    processor_tick_count += increment;
		tick_count = processor_tick_count / csdf;
    #else
		#ifdef CLOCK_SLOW_DOWN_FACTOR
			#if CLOCK_SLOW_DOWN_FACTOR > 1
				processor_tick_count += increment;
				tick_count = processor_tick_count / CLOCK_SLOW_DOWN_FACTOR;
		    #else
				tick_count += increment;
			#endif
		#else
			tick_count += increment;
		#endif
    #endif
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

    asm("NOP");

} // delay_ms //


// The following delay function creates a delay of 101.8 micro seconds
void delay_100s_us(UInt16 count)
{
    // Uses machine instructions to simulate delaying (doing nothing) for a
    // specified number of microseconds.
    UInt16 i;

    while(count-- != 0)
    {
        i = NOP_COUNT_100S_US;
        while(i--);
    } // delay loop //

    asm("NOP");

} // delay_100us //



void enable_tick_timer(void)
{
   //  Enable interrupt on overflow level LOW
   TCC0.INTCTRLA = (TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm) | TC_OVFINTLVL_LO_gc;

   // Set the Programmable Interrupt Controller multi level interrupts to LOW
   PMIC.CTRL |= PMIC_LOLVLEN_bm;
}


void disable_tick_timer(void)
{
   //  disable interrupt on overflow level LOW
   TCC0.INTCTRLA = (TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm);
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
// This timer interrupt is the general timer that its tick time is 1 mili second
ISR(TCC0_OVF_vect)
{
//    tick_count++;
//    tick_flag = TRUE;

    #ifdef DEBUGGING_TOOLS
	    processor_tick_count++;
		tick_count = processor_tick_count / csdf;
    #else
		#ifdef CLOCK_SLOW_DOWN_FACTOR
			#if CLOCK_SLOW_DOWN_FACTOR > 1
				processor_tick_count++;
				tick_count = processor_tick_count / CLOCK_SLOW_DOWN_FACTOR;
			#else
				tick_count++;
			#endif
		#else
			tick_count++;
			#endif
    #endif

    tick_flag = TRUE;

} // tick_timer //


//! @} TICK_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} TICK
