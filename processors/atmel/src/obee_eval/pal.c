//! \defgroup PAL Processor abstraction layer.
//! \ingroup PAL
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
    \file pal.c
    \brief Contains common hardware declarations.  Implementation will be
        processor-specific and possibly hardware-specific.  Not all ports will
        implement all functions here.
		
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/


#include "pal.h"
#include "io_port_mapping.h"
#include "tick.h"
#include "one_net_application.h" // for "INPUT" and "OUTPUT"

#include <avr/interrupt.h>
#include <stdlib.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_const
//! \ingroup PAL
//! @{

//! The rate the low speed oscillator runs at (in Hz)
//#define LOW_SPEED_OSCILLATOR 125000

//! @} PAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_typedefs
//! \ingroup PAL
//! @{



//! @} PAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup PAL_pri_var
//! \ingroup PAL
//! @{

//! @} PAL_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup PAL_pri_func
//! \ingroup PAL
//! @{

//! @} PAL_pri_func
//!                     PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup PAL_pub_func
//! \ingroup PAL
//! @{

/*!
    \brief Initializes the ports that MUST be initialized before anything else.

    \param void

    \return void
*/
void pal_init_ports(void)
{

} // init_ports //


/*!
    \brief Sets the processor to use the main clock as the sytem clock.

    The ATxmega256a3b comes up using a low speed clock.  This sets the main
    clock (external input to p4_6) as the system clock.  It is assumed that
    interrupts are disabled when this function is called.

    \param void

    \return void
*/

#define AVR_ENTER_CRITICAL_REGION()  {uint8_t volatile saved_sreg = SREG; cli();
#define AVR_LEAVE_CRITICAL_REGION()  SREG = saved_sreg;}

void pal_high_speed_mode(void)
{
    // set PC7 as an output
//    PORTC.DIRSET = PIN7_bm;

    // set PC7 to be the clock output pin
//    PORTCFG.CLKEVOUT = (PORTCFG.CLKEVOUT & ~PORTCFG_CLKOUT_gm) | PORTCFG_CLKOUT_PC7_gc;
	
	/* Enable for external 9-12 MHz crystal with quick startup time
		* (256CLK). Check if it's stable and set the external
		* oscillator as the main clock source. Wait for user input
		* while the LEDs toggle.
	*/

	OSC.XOSCCTRL = (uint8_t) OSC_FRQRANGE_9TO12_gc | 0 | OSC_XOSCSEL_EXTCLK_gc;

    // enable external oscillator
    OSC.CTRL |= OSC_XOSCEN_bm;

	// Check the ready flag before using the clock.
    do {} while ( (OSC.STATUS & OSC_XOSCRDY_bm) == 0 );
    
    // set external oscillator as the main clock
	uint8_t clkCtrl = ( CLK.CTRL & ~CLK_SCLKSEL_gm ) | CLK_SCLKSEL_XOSC_gc;

   #ifdef __ICCAVR__

   // Store global interrupt setting in scratch register and disable interrupts.
   asm("in  R1, 0x3F \n"
   "cli"
   );

   // Move destination address pointer to Z pointer registers.
   asm("movw r30, r16");
   #ifdef RAMPZ
   asm("ldi  R16, 0 \n"
   "out  0x3B, R16"
   );

   #endif
   asm("ldi  r16,  0xD8 \n"
   "out  0x34, r16  \n"
   #if (__MEMORY_MODEL__ == 1)
   "st     Z,  r17  \n");
   #elif (__MEMORY_MODEL__ == 2)
   "st     Z,  r18  \n");
   #else /* (__MEMORY_MODEL__ == 3) || (__MEMORY_MODEL__ == 5) */
   "st     Z,  r19  \n");
   #endif /* __MEMORY_MODEL__ */

   // Restore global interrupt setting from scratch register.
   asm("out  0x3F, R1");

   #elif defined __GNUC__
   AVR_ENTER_CRITICAL_REGION( );
   //	volatile uint8_t * tmpAddr = address;
   volatile uint8_t * tmpAddr = (volatile uint8_t *)&CLK.CTRL;
   #ifdef RAMPZ
   RAMPZ = 0;
   #endif
   asm volatile(
   "movw r30,  %0"	      "\n\t"
   "ldi  r16,  %2"	      "\n\t"
   "out   %3, r16"	      "\n\t"
   "st     Z,  %1"       "\n\t"
   :
   //		: "r" (tmpAddr), "r" (value), "M" (CCP_IOREG_gc), "i" (&CCP)
   : "r" (tmpAddr), "r" (clkCtrl), "M" (CCP_IOREG_gc), "i" (&CCP)
   : "r16", "r30", "r31"
   );

   AVR_LEAVE_CRITICAL_REGION( );
   #endif

   clkCtrl = ( CLK.CTRL & CLK_SCLKSEL_XOSC_gc );
	
} // high_speed_mode //


/*!
    \brief Sets the processor to use the low speed internal oscillator

    \param void

    \return void
*/
void pal_low_speed_mode(void)
{
} // low_speed_mode //


/*!
    \brief Enable global interrupts

    \param void

    \return void
*/
void pal_enable_global_interrupts(void)
{
      sei();         // set the I bit in The AVR Status Register – SREG

}


/*!
    \brief Disables global interrupts

    \param void

    \return void
*/
void pal_disable_global_interrupts(void)
{
      cli();         // clear the I bit in The AVR Status Register – SREG
}


/*!
    \brief Puts the micro controller to sleep.

    The micro controller will sleep until an interrupt occurs or until ms
    milliseconds elapses.  Pass 0 to sleep until an interrupt occurs.

    This function will also need to update the tick count.

    \param[in] MS The number of milliseconds to sleep for

    \return void
*/
void pal_processor_sleep(UInt32 MS)
{
    // TODO - write this function, if necessary
} // processor_sleep //


/*!
    \brief Exits the program.

    Disables global interrupts & stops program execution.

    \param void

    \return void
*/
void pal_exit(void)
{
    pal_disable_global_interrupts();
    exit(-1);
}


/*!
    \brief Initializes the processor

    Initializes the processor.

    \param high_speed If TRUE, directs the processor to start in high_speed.
         Otherwise the proocessor should start in low speed.

    \return void
*/
void pal_init_processor(BOOL high_speed)
{
    if(high_speed)
    {
        pal_high_speed_mode();
    }
    else
    {
        pal_low_speed_mode();
    }
}


//! @} PAL_pub_func
//!                     PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup PAL_pri_func
//! \ingroup PAL
//! @{

//! @} PAL_pri_func
//!                     PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup PAL_pub_func
//! \ingroup PAL
//! @{





//! @} PAL_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================


//! @} PAL

