//! \addtogroup TAL_ADI Processor Abstraction Layer for ADI ADF7025.
//! \ingroup ADI
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
    \file tal_adi.c
    \brief Processor abstraction layer for the ADI ADF7025.
	
    This file implements the functionality needed by the ADI
    transceiver (such as interrupts for the various communication).  This file
    is for Atxmega256a3b devices that have the ADI set up to use Timercc1 has the transmit
    bit interrupt, and INT1 as the data clock interrupt for receive mode.

	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/

#include "config_options.h"

#include "tal_adi.h"
#include "hal_adi.h"
#include "one_net_data_rate.h"
#include "io_port_mapping.h"
#include "tal.h"


#include <avr/io.h>
#include <avr/interrupt.h>

//=============================================================================
//                                  CONSTANTS
//! \defgroup TAL_ADI_const
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup TAL_ADI_typedefs
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TAL_ADI_pri_var
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TAL_ADI_pri_func
//! \ingroup TAL_ADI
//! @{



//! @} TAL_ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TAL_ADI_pub_func
//! \ingroup TAL_ADI
//! @{



/*!
    \brief Initialize interrupts needed for ADI 7025 operation.

    Timer TCC1 is used as the bit timer for transmitting data.  The INT0 interrupt of IO port PORTD pin 1
    is triggered by the DATACLK signal from the ADI when receiving data.

    \param void

    \return TRUE if successful, FALSE otherwise
*/
one_net_status_t init_rf_interrupts(UInt8 DATA_RATE)
{

    // disable RF data transmit interrupt
    DISABLE_TX_BIT_INTERRUPTS();

    // Use timer TCC1
    // initialize TCC1 to to timer mode normal for rf output
    //    Prescaler and timer register initialization
    //    system clock Frequency =  11.0592 MHz

    // prescaler 2 leads to => 11.0529 MHz / 2 => 5.5296 * 10^6
	// t = (1 / 5.5296) * 10^-6 seconds = 0.180844907 * 10^-6 seconds  (this is 1 tick time)

    // For Higher Frequencies...

    //    5529.6 KHz / 38.400 KHz = 144
    //    5529.6 KHz / 76.800 KHz = 72
    //    5529.6 KHz / 115.200 KHz = 48
    //    5529.6 KHz / 153.600 KHz = 36
    //    5529.6 KHz / 192.000 KHz = 28.8 (round to 29)
    //    5529.6 KHz / 230.400 KHz = 24

    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 144.02 => this will give us a timer tick every X = 26 micro seconds
    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 72 => this will give us a timer tick every X = 13 micro seconds
    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 48 => this will give us a timer tick every X = 8.68 micro seconds
    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 36 => this will give us a timer tick every X = 6.51 micro seconds
    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 29 => this will give us a timer tick every X = 5.2445 micro seconds
    // PERIOD = X * 10^-6 / 0.180844907 * 10^-6 = 24 => this will give us a timer tick every X = 4.34 micro seconds


    const UInt8 register_values[ONE_NET_DATA_RATE_LIMIT] =
    {
	    143,  // after fine tunning
	    72,
	    48,
	    36,
	    29,
	    24
    };


   TCC1.CTRLB = 0x00;         // normal mode

   // Set period/TOP value.
   // after fine tunning
   TCC1.PER = register_values[DATA_RATE];         // use period  count of  after fine tuning

   // Select clock source.
   TCC1.CTRLA = (TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_DIV2_gc;

    // initialize the INT0 interrupt of IO port PORTF pin 1 to handle RX_BIT_CLK for receive mode
    // rising edge instead of falling edge on PORTF pin 1

	// Build pin control register value.
	uint8_t temp = (uint8_t) PORT_OPC_TOTEM_gc | PORT_ISC_RISING_gc;

	// Configure the pins in one atomic operation.

	// Save status register.
	uint8_t sreg = SREG;

	cli();
	PORTCFG.MPCMASK = 0x02;
	PORTF.PIN1CTRL = temp;

	// Restore status register.
	SREG = sreg;

    // Set PORTF pin 1 as input
	PORTF.INT0MASK = 0X02;

	// Enable medium level interrupts in the PMIC.
    PMIC.CTRL |= PMIC_MEDLVLEN_bm;

    return ONS_SUCCESS;
} // init_rf_interrupts //



//! @} TAL_ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ADI
