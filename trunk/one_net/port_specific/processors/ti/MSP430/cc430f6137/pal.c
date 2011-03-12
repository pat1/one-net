//! \defgroup PAL_SW_EX Processor abstraction layer for ONE-NET switch example.
//! \ingroup PAL
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
    \file pal.c (for switch example)
    \brief Contains implementation of R8C1B specific functions.

    Hardware specific functions for the R8C1B.  Despite being specific for the
    processor, these functions are also specific based on the hardware.  For
    example, some boards may use an external clock as the main clock while
    others may use one of the internal oscillators (for the R8C series).
*/

#include "cc430x613x.h"
#include "intrinsics.h"
#include "cc430x613x_PMM.h"
#include "pal.h"
//#include "spi.h"
//#include "uart.h"
//#include "tal.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_SW_EX_const
//! \ingroup PAL_SW_EX
//! @{
    
//! The rate the low speed oscillator runs at (in Hz)
#define LOW_SPEED_OSCILLATOR 32768

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
    P1OUT_byte = 0x00; 
    P2OUT_byte = 0x00; 
    P3OUT_byte = 0x00; 
    P4OUT_byte = 0x00; 
    P5OUT = 0x00;
    P1DIR_byte = 0xFF; 
    P2DIR_byte = 0xFF; 
    P3DIR_byte = 0xFF; 
    P4DIR_byte = 0xFF; 
    P5DIR = 0xFF;

    PJOUT = 0x00;
    PJDIR = 0xFF;
    
    INIT_PORTS_LEDS();
//    INIT_PORTS_UART();
} // init_ports //


/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void init_ports_leds(void)
{
    SW_DIR = INPUT;
    SW_REN = ENABLE;
    SW_OUT = HIGH;

    LED2_DIR = OUTPUT;
    LED3_DIR = OUTPUT;

    TURN_OFF(LED2);
    TURN_OFF(LED3);

    pd2_0 = INPUT;
    pd2_1 = OUTPUT;
    pre2_0 = ENABLE;
    p2_0 = 1;
    p2_1 = 0;

    pd2_2 = INPUT;
    pd2_3 = OUTPUT;
    pre2_2 = ENABLE;
    p2_2 = 1;
    p2_3 = 0;

    pd2_4 = INPUT;
    pd2_5 = OUTPUT;
    pre2_4 = ENABLE;
    p2_4 = 1;
    p2_5 = 0;

    pd4_1 = OUTPUT;
    pd4_3 = OUTPUT;
    pd4_5 = OUTPUT;
    pd4_7 = OUTPUT;

    p4_1 = 0;
    p4_3 = 0;
    p4_5 = 0;
    p4_7 = 0;
    
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
    SetVCore(2);                                                                // Note: change level by one step only
    UCSCTL3 |= SELREF__REFOCLK;                                                 // Set DCO FLL reference = REFO
    UCSCTL4 = SELM__DCOCLKDIV + SELS__DCOCLKDIV + SELA__REFOCLK; 

    __bis_SR_register(SCG0);                                                    // Disable the FLL control loop
    UCSCTL0 = 0x0000;                                                           // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                                                        // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_1 + 243;                                                     // Set DCO Multiplier for 8MHz
                                                                                // (N + 1) * FLLRef = Fdco
                                                                                // (243 + 1) * 32768 = 8MHz
                                                                                // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                                                    // Enable the FLL control loop

// Worst-case settling time for the DCO when the DCO range bits have been
// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
// UG for optimization.
// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
    __delay_cycles(250000);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
                                                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                                                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                                                     // Test oscillator fault flag
} // high_speed_mode //

/*!
    \brief Sets the processor to use the low speed internal oscillator

    \param void

    \return void
*/
void low_speed_mode(void)
{
    UCSCTL4 = SELM__VLOCLK + SELS__VLOCLK + SELA__VLOCLK; 
    __bis_SR_register(SCG0);                                                    // Disable the FLL control loop
    SetVCore(0);                                                                  // Note: change level by one step only
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
    tick_t  delay, ticks_slept, tick;
    UInt8   last_sw_pos = SW;
    
    DISABLE_TICK_TIMER();
    DISABLE_GLOBAL_INTERRUPTS();

    delay = MS ? MS : TICK_1S;

    while(SW == last_sw_pos && (delay || !MS))
    {
        UInt32 temp;

        temp = (UInt32)LOW_SPEED_OSCILLATOR / (UInt32)TICK_1S * (UInt32)delay;
        if (temp > 32768)
            temp = 32768;
        TA1CTL |= TACLR;
        TA1CCTL0 = CCIE;                                                        // CCR0 interrupt enabled
        TA1CCTL0 &= (~CCIFG);
        TA1CCR0 = (UInt16)temp - 1;
        TA1CTL = TASSEL__ACLK;                                                  // ACLK
        tick = get_tick_count();
        ENABLE_TICK_TIMER();
        SET_SW_INTERRUPT_EDGE(SW);
        ENABLE_SW_INTERRUPT();
        ENABLE_GLOBAL_INTERRUPTS();
        if(SW == last_sw_pos)
        {
            // make sure one last time that the switch has not been moved
            __bis_SR_register(LPM3_bits + GIE);                                 // LPM3 with interrupts enabled
            asm("NOP");
        } // if the switch has not moved //
        DISABLE_TICK_TIMER();
        DISABLE_GLOBAL_INTERRUPTS();
        DISABLE_SW_INTERRUPT();
        tick = get_tick_count() - tick;
        if(!tick)
        {
            temp -= TA1R;
        } // if a timer interrupt did not happen //
        ticks_slept = temp * (tick_t)TICK_1S / (tick_t)LOW_SPEED_OSCILLATOR;
        if(MS)
        {
            delay -= (ticks_slept < delay ? ticks_slept + 1 : delay);
        } // if doing a timed sleep //
        if(ticks_slept > tick)
        {
            update_tick_count(ticks_slept - tick);
        } // if the device slept //
   }
   INIT_TICK();
   ENABLE_GLOBAL_INTERRUPTS();
} // processor_sleep //

UInt16 read_battery(void)
{
/*  
   UInt16 v;
   volatile long temp;
   
   ADC10CTL1 = INCH_11;                                                         // AVcc/2
   ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE + REF2_5V;
   for(v = 300; v > 0; v--);                                                    // delay to allow reference to settle
   ADC10CTL0 |= ENC + ADC10SC;                                                  // Sampling and conversion start
   __bis_SR_register(CPUOFF + GIE);                                             // LPM0 with interrupts enabled
   v = ADC10MEM;
   ADC10CTL0 &= ~ENC;
   ADC10CTL0 &= ~(REFON + ADC10ON);                                             // turn off A/D to save power
   temp = v;
   v = (temp * 25) / 512;
   return v;
*/
   return 33;
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

// Port 1 interrupt service routine

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG_byte = 0;                              // P1 IFG cleared
  __bic_SR_register_on_exit(LPM3_bits);        // Clear LPM3 bit from 0(SR)
}


/*------------------------------------------------------------------------------
* ADC10 interrupt service routine
------------------------------------------------------------------------------*/
/*
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}
*/