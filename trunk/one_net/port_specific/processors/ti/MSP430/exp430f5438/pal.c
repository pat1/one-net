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

#include "pal.h"
#include "flash.h"


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

char pal_sw_1;
char pal_sw_2;
char pal_sw_up;
char pal_sw_down;
char pal_sw_left;
char pal_sw_right;
char pal_sw_select;

//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup PAL_SW_EX_pri_func
//! \ingroup PAL_SW_EX
//! @{

static void halBoardGetSystemClockSettings(unsigned char systemClockSpeed, unsigned char *setDcoRange, unsigned char *setVCore, unsigned int  *setMultiplier);
void halBoardStartXT1(void);
void halBoardSetSystemClock(unsigned char systemClockSpeed);
void halBoardOutputSystemClock(void);
void halBoardStopOutputSystemClock(void);

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

/*-------------------------------------------------------------------------*/

/**********************************************************************//**
 * @brief  Get function for the DCORSEL, VCORE, and DCO multiplier settings 
 *         that map to a given clock speed. 
 * 
 * @param  systemClockSpeed Target DCO frequency - SYSCLK_xxMHZ.
 * 
 * @param  setDcoRange      Pointer to the DCO range select bits.
 * 
 * @param  setVCore         Pointer to the VCore level bits. 
 * 
 * @param  setMultiplier    Pointer to the DCO multiplier bits. 
 *
 * @return none
 ************************************************************************/
static void halBoardGetSystemClockSettings(unsigned char systemClockSpeed, 
                                    unsigned char *setDcoRange,
                                    unsigned char *setVCore,
                                    unsigned int  *setMultiplier)
{
  switch (systemClockSpeed)
  {
  case SYSCLK_1MHZ: 
    *setDcoRange = DCORSEL_1MHZ;
    *setVCore = VCORE_1MHZ;
    *setMultiplier = DCO_MULT_1MHZ;
    break;
  case SYSCLK_4MHZ: 
    *setDcoRange = DCORSEL_4MHZ;
    *setVCore = VCORE_4MHZ;
    *setMultiplier = DCO_MULT_4MHZ;
    break;
  case SYSCLK_8MHZ: 
    *setDcoRange = DCORSEL_8MHZ;
    *setVCore = VCORE_8MHZ;
    *setMultiplier = DCO_MULT_8MHZ;
    break;
  case SYSCLK_12MHZ: 
    *setDcoRange = DCORSEL_12MHZ;
    *setVCore = VCORE_12MHZ;
    *setMultiplier = DCO_MULT_12MHZ;
    break;
  case SYSCLK_16MHZ: 
    *setDcoRange = DCORSEL_16MHZ;
    *setVCore = VCORE_16MHZ;
    *setMultiplier = DCO_MULT_16MHZ;
    break;
/*------------------------------------- 
 * Commented out because fmax = 18 MHz 
 * ------------------------------------
  case SYSCLK_20MHZ: 
    *setDcoRange = DCORSEL_20MHZ;
    *setVCore = VCORE_20MHZ;
    *setMultiplier = DCO_MULT_20MHZ;
    break;
  case SYSCLK_25MHZ: 
    *setDcoRange = DCORSEL_25MHZ;
    *setVCore = VCORE_25MHZ;
    *setMultiplier = DCO_MULT_25MHZ;
    break;
 *-------------------------------------*/	     
  }	
}

/**********************************************************************//**
 * @brief  Initialization routine for XT1. 
 * 
 * Sets the necessary internal capacitor values and loops until all 
 * ocillator fault flags remain cleared. 
 * 
 * @param  none
 * 
 * @return none
 *************************************************************************/
void halBoardStartXT1(void)
{
  // Set up XT1 Pins to analog function, and to lowest drive	
  P7SEL |= 0x03;                            
  UCSCTL6 |= XCAP_3 ;                       // Set internal cap values
  
  while(SFRIFG1 & OFIFG) {                  // Check OFIFG fault flag
    while ( (SFRIFG1 & OFIFG))              // Check OFIFG fault flag
    {    
      // Clear OSC fault flags 
      UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT1HFOFFG + XT2OFFG);
      SFRIFG1 &= ~OFIFG;                    // Clear OFIFG fault flag
    }
    UCSCTL6_bit.XT1DRIVE0 = 0;
    UCSCTL6_bit.XT1DRIVE1 = 0;
  }
}

/**********************************************************************//**
 * @brief  Set function for MCLK frequency.
 * 
 * @param  systemClockSpeed Intended frequency of operation - SYSCLK_xxMHZ.
 * 
 * @return none
 *************************************************************************/
void halBoardSetSystemClock(unsigned char systemClockSpeed)
{
  unsigned char setDcoRange, setVCore;
  unsigned int  setMultiplier;

  halBoardGetSystemClockSettings( systemClockSpeed, &setDcoRange,  \
                                  &setVCore, &setMultiplier);
  	
  UCSCTL0 = 0x00;                           // Set lowest possible DCOx, MODx
  UCSCTL1 = setDcoRange;                    // Select suitable range
  
  UCSCTL2 = setMultiplier + FLLD_1;         // Set DCO Multiplier
  UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV  |  SELM__DCOCLKDIV ;
  // Worst-case settling time for the DCO when the DCO range bits have been 
  // changed is n x 32 x 32 x f_FLL_reference. See UCS chapter in 5xx UG 
  // for optimization.
  // 32 x 32 x 1 / f_FLL_reference (32,768 Hz) = .03125 = t_DCO_settle
  // t_DCO_settle / (1 / 18 MHz) = 562500 = counts_DCO_settle
  __delay_cycles(562500);  
}

/**********************************************************************//**
 * @brief  Initializes ACLK, MCLK, SMCLK outputs on P11.0, P11.1, 
 *         and P11.2, respectively.
 * 
 * @param  none
 * 
 * @return none
 *************************************************************************/
void halBoardOutputSystemClock(void)
{
  P11DIR |= 0x07;
  P11SEL |= 0x07;                           
}

/**********************************************************************//**
 * @brief  Stops the output of ACLK, MCLK, SMCLK on P11.0, P11.1, and P11.2.
 * 
 * @param  none
 * 
 * @return none
 *************************************************************************/
void halBoardStopOutputSystemClock(void)
{  
  P11OUT &= ~0x07;
  P11DIR |= 0x07;	
  P11SEL &= ~0x07;                 
}

/*!
    \brief Initializes the ports that MUST be initialized before anything else.
    
    \param void
    
    \return void
*/
void init_ports(void)
{
    volatile int i = flash_mem[0];
  //Tie unused ports
    P1OUT  = 0;
    P1DIR  = 0xFF;
    P1SEL  = 0;

    P2OUT  = 0;
    P2DIR  = 0xFF;
    P2SEL  = 0;

    P3OUT  = 0;
    P3DIR  = 0xFF;
    P3SEL  = 0;

    P4OUT  = 0;
    P4DIR  = 0xFF;
    P4SEL  = 0;

    P5OUT  = 0;
    P5DIR  = 0xFF;
    P5SEL  = 0;

    P6OUT  = 0;
    P6DIR  = 0xFF;
    P6SEL  = 0;

    P7OUT  = 0;
    P7DIR  = 0xFF;
    P7SEL  = 0;

    P8OUT  = 0;
    P8DIR  = 0xFF;
    P8SEL  = 0;

    P9OUT  = 0;
    P9DIR  = 0xFF;
    P9SEL  = 0;

    P10OUT = 0;
    P10DIR = 0xFE;
    P10SEL = 0;

    P11OUT = 0;
    P11DIR = 0xFF;
    PJOUT  = 0;    
    PJDIR  = 0xFF;
    P11SEL = 0;
    
    halBoardOutputSystemClock();
    
    INIT_PORTS_LEDS();
    INIT_PORTS_UART();
    INIT_SPI(TRUE, 0, 0, TRUE);
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
    
    BUTTON_PORT_OUT |= BUTTON_ALL;
    BUTTON_PORT_DIR &= ~BUTTON_ALL;
    BUTTON_PORT_REN |= BUTTON_ALL; 
    BUTTON_PORT_SEL &= ~BUTTON_ALL;   
    BUTTON_PORT_IFG &= ~BUTTON_ALL; 
    BUTTON_PORT_IE |= BUTTON_ALL; 

    P4DIR |= (BIT1 | BIT3 | BIT6);
    P4OUT &= ~(BIT1 | BIT3 | BIT6);
    P5DIR |= (BIT4);
    P5OUT &= ~(BIT4);

    P5REN |= (BIT0);
    P5OUT |= (BIT0);
    P5DIR |= (BIT1);
    P5OUT &= ~(BIT1);

    P6REN |= (BIT7);
    P6OUT |= (BIT7);
    P7DIR |= (BIT4);
    P7OUT &= ~(BIT4);

    P7REN |= (BIT5);
    P7OUT |= (BIT5);
    P7DIR |= (BIT6);
    P7OUT &= ~(BIT6);

    pal_sw_1 = pal_sw_2 = pal_sw_up = pal_sw_down =  pal_sw_left = pal_sw_right = pal_sw_select = 0;
    
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
    halBoardStartXT1();	
    halBoardSetSystemClock(SYSCLK_8MHZ);
    halBoardOutputSystemClock();
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
    UInt8   last_sw_pos = SW1;
    
    DISABLE_TICK_TIMER();
    DISABLE_GLOBAL_INTERRUPTS();
    DISABLE_TRANSCEIVER();

    delay = MS ? MS : TICK_1S;

    while(SW1 == last_sw_pos && (delay || !MS))
    {
        UInt32 temp;

        temp = (UInt32)LOW_SPEED_OSCILLATOR / (UInt32)TICK_1S * (UInt32)delay;
        if (temp > 60000)
            temp = 60000;
        TA1CTL |= TACLR;
        TA1CCTL0 = CCIE;                                                        // CCR0 interrupt enabled
        TA1CCTL0 &= (~CCIFG);
        TA1CCR0 = (UInt16)temp - 1;
        TA1CTL = TASSEL_1;                                                      // ACLK
        tick = get_tick_count();
        ENABLE_TICK_TIMER();
        SET_SW_INTERRUPT_EDGE(SW1);
        ENABLE_SW_INTERRUPT();
        ENABLE_GLOBAL_INTERRUPTS();
        if(SW1 == last_sw_pos)
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
   TAL_INIT_TRANSCEIVER();
   INIT_TICK();
   ENABLE_GLOBAL_INTERRUPTS();
} // processor_sleep //

UInt16 read_battery(void)
{
   UInt16 v;
   volatile long temp;

   ADC12CTL0 = ADC12SHT0_15 + ADC12REFON + ADC12ON + ADC12REF2_5V;
                                                                                // Internal ref = 2.5V
   ADC12CTL1 = ADC12SHP;                                                        // enable sample timer
   ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_11;                                     // ADC i/p ch A11 = Vcc/2
   ADC12IE = 0x001;                                                             // ADC_IFG upon conv result-ADCMEMO
   __delay_cycles(1260);
   ADC12CTL0_bit.ADC12ENC = 1;

   ADC12CTL0_bit.ADC12SC = 1;                                                   // Sampling and conversion start

   __bis_SR_register(CPUOFF + GIE);                                             // LPM0 with interrupts enabled
   __no_operation();
   temp = ADC12MEM0;                                                            // Move results, IFG is cleared
  
   ADC12CTL0_bit.ADC12ENC = 0;
   ADC12CTL0 &= ~(ADC12REFON + ADC12ON);                                            // turn off A/D to save power

   v = (((temp * 500) / 4096) + 5) / 10;
   return v;
}

#define CELSIUS_MUL			7040
#define CELSIUS_OFFSET		2620
#define FAHRENHEIT_MUL		12672
#define FAHRENHEIT_OFFSET	3780 

UInt16 read_temperature(void)
{
   UInt16 v;
   volatile long temp;

   ADC12CTL0 = ADC12SHT0_15 + ADC12REFON + ADC12ON + ADC12REF2_5V;
                                                                                // Internal ref = 2.5V
   ADC12CTL1 = ADC12SHP;                                                        // enable sample timer
   ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;                                     // ADC i/p ch A10 = Temp
   ADC12IE = 0x001;                                                             // ADC_IFG upon conv result-ADCMEMO
   __delay_cycles(1260);
   ADC12CTL0_bit.ADC12ENC = 1;

   ADC12CTL0_bit.ADC12SC = 1;                                                   // Sampling and conversion start

   __bis_SR_register(CPUOFF + GIE);                                             // LPM0 with interrupts enabled
   __no_operation();
   temp = ADC12MEM0;                                                            // Move results, IFG is cleared
  
   ADC12CTL0_bit.ADC12ENC = 0;
   ADC12CTL0 &= ~(ADC12REFON + ADC12ON);                                        // turn off A/D to save power

   v = temp * CELSIUS_MUL / 4096 - CELSIUS_OFFSET;   	
   return v;
}

char IsSW1Pressed(void)
{
    return pal_sw_1;
}

void ClearSW1(void)
{
    pal_sw_1 = 0;
}

char IsSW2Pressed(void)
{
    return pal_sw_2;
}

void ClearSW2(void)
{
    pal_sw_2 = 0;
}

char IsUpPressed(void)
{
    return pal_sw_up;
}

void ClearUp(void)
{
    pal_sw_up = 0;
}

char IsDownPressed(void)
{
    return pal_sw_down;
}

void ClearDown(void)
{
    pal_sw_down = 0;
}

char IsLeftPressed(void)
{
    return pal_sw_left;
}

void ClearLeft(void)
{
    pal_sw_left = 0;
}

char IsRightPressed(void)
{
    return pal_sw_right;
}

void ClearRight(void)
{
    pal_sw_right = 0;
}

char IsSelectPressed(void)
{
    return pal_sw_select;
}

void ClearSelect(void)
{
    pal_sw_select = 0;
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

// Port 2 interrupt service routine

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
  switch(__even_in_range(P2IV,16))
  {
    case  0: break;                          // No interrupt
    case  2:                                 // BIT0
        P2IFG &= ~(BIT0);
        break;
    case  4:                                 // BIT1
        pal_sw_left = 1;
        P2IFG &= ~(BIT1);
        break;
    case  6:                                 // BIT2
        pal_sw_right = 1;
        P2IFG &= ~(BIT2);
        break;
    case  8:                                 // BIT3
        pal_sw_select = 1;
        P2IFG &= ~(BIT3);
        break;
    case 10:                                 // BIT4
        pal_sw_up = 1;
        P2IFG &= ~(BIT4);
        break;
    case 12:                                 // BIT5
        pal_sw_down = 1;
        P2IFG &= ~(BIT5);
        break;
    case 14:                                 // BIT6
        pal_sw_1 = 1;
        P2IFG &= ~(BIT6);
        break;
    case 16:                                 // BIT7
        pal_sw_2 = 1;
        P2IFG &= ~(BIT7);
        break;
    default: 
        break; 
  }
  __bic_SR_register_on_exit(LPM3_bits);        // Clear LPM3 bit from 0(SR)
}


/*------------------------------------------------------------------------------
* ADC12 interrupt service routine
------------------------------------------------------------------------------*/
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  ADC12IFG = 0;                           // Clear the interrupt flags
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}
