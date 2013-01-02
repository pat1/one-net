#ifndef _IO_PORT_MAPPING_H
#define _IO_PORT_MAPPING_H

#include "config_options.h"

#include <avr/io.h>


//! \defgroup IO_PORT_MAPPING_ADI_EVAL I/O Port Mapping for the Evaluation
//!   Project using an ADI ADF7025 transceiver.
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
    \file io_port_mapping.h
    \brief Contains the port definitions for the ONE-NET switch example project
      using an ADI transceiver.

    The specific mapping of i/o ports to other components may vary from board to
    board.  This file contains symbolic names for port functions that are mapped
    to the i/o ports on the ONE-NET Example Boards using an ADI tranceiver and
	Atxmega256a3b processor.
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup IO_PORT_MAPPING_const
//! \ingroup IO_PORT_MAPPING_ADI_EVAL
//! @{

//! @} IO_PORT_MAPPING_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup IO_PORT_MAPPING_typedefs
//! \ingroup IO_PORT_MAPPING_ADI_EVAL
//! @{

//! @} IO_PORT_MAPPING_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup IO_PORT_MAPPING_pub_var
//! \ingroup IO_PORT_MAPPING_ADI_EVAL
//! @{


#ifdef ATXMEGA256A3B_EVAL

// define ADI setup registers read/write signals
//-----------------------------------------------

// define signal as port regiter the bit belongs to
#define SLE_PORT_REG               PORTA.OUT
#define SCLK_PORT_REG              PORTA.OUT
#define SDATA_PORT_REG             PORTA.OUT
#define SREAD_PORT_REG             PORTA.IN

// define signal as port bit
#define SCLK_BIT                   PIN0_bp       // output
#define SREAD_BIT                  PIN1_bp       // input
#define SDATA_BIT                  PIN2_bp       // output
#define SLE_BIT                    PIN3_bp       // output

// define signal as port register direction the bit belongs to
#define SLE_DIR_REG                PORTA.DIR
#define SCLK_DIR_REG               PORTA.DIR
#define SDATA_DIR_REG              PORTA.DIR
#define SREAD_DIR_REG              PORTA.DIR

// define signal as port register direction bit
#define SCLK_DIR_BIT               PIN0_bp       // output
#define SREAD_DIR_BIT              PIN1_bp       // input
#define SDATA_DIR_BIT              PIN2_bp       // output
#define SLE_DIR_BIT                PIN3_bp       // output

// define RF signals
//------------------
// define signal as port register the bit belongs to

//#define RX_BIT_CLK_PORT_REG      PORTE.IN
#define RF_DATA_OUTPUT_PORT_REG    PORTE.OUT
#define RF_DATA_INPUT_PORT_REG     PORTE.IN
#define CHIP_ENABLE_PORT_REG       PORTE.OUT
#define SYNCDET_PORT_REG           PORTE.IN

// define signal as port bit
//#define RX_BIT_CLK_BIT           PIN1_bp     // input
#define RF_DATA_OUTPUT_BIT         PIN2_bp     // output
#define RF_DATA_INPUT_BIT          PIN2_bp     // input
#define CHIP_ENABLE_BIT            PIN4_bp     // output
#define SYNCDET_BIT                PIN5_bp     // input

// define signal as port register direction the bit bilongs to

//#define RX_BIT_CLK_DIR_REG       PORTE.DIR
#define RF_DATA_DIR_REG            PORTE.DIR
#define CHIP_ENABLE_DIR_REG        PORTE.DIR
#define SYNCDET_DIR_REG            PORTE.DIR

// define signal as port register direction
//#define RX_BIT_CLK_DIR_BIT       PIN1_bp
#define RF_DATA_DIR_BIT            PIN2_bp
#define CHIP_ENABLE_DIR_BIT        PIN4_bp
#define SYNCDET_DIR_BIT            PIN5_bp

#else // OBE board

// define ADI setup registers read/write signals
//-----------------------------------------------

// define signal as port regiter the bit belongs to
#define SLE_PORT_REG               PORTA.OUT
#define SCLK_PORT_REG              PORTA.OUT
#define SDATA_PORT_REG             PORTA.OUT
#define SREAD_PORT_REG             PORTA.IN

// define signal as port bit
#define SCLK_BIT                   PIN0_bp       // output
#define SREAD_BIT                  PIN1_bp       // input
#define SDATA_BIT                  PIN2_bp       // output
#define SLE_BIT                    PIN3_bp       // output

// define signal as port register direction the bit belongs to
#define SLE_DIR_REG                PORTA.DIR
#define SCLK_DIR_REG               PORTA.DIR
#define SDATA_DIR_REG              PORTA.DIR
#define SREAD_DIR_REG              PORTA.DIR

// define signal as port register direction bit
#define SCLK_DIR_BIT               PIN0_bp       // output
#define SREAD_DIR_BIT              PIN1_bp       // input
#define SDATA_DIR_BIT              PIN2_bp       // output
#define SLE_DIR_BIT                PIN3_bp       // output

// define RF signals
//------------------
// define signal as port register the bit belongs to

//#define RX_BIT_CLK_PORT_REG      PORTE.IN
#define RF_DATA_OUTPUT_PORT_REG    PORTF.OUT
#define RF_DATA_INPUT_PORT_REG     PORTF.IN
#define CHIP_ENABLE_PORT_REG       PORTE.OUT
#define SYNCDET_PORT_REG           PORTE.IN

// define signal as port bit
//#define RX_BIT_CLK_BIT           PIN1_bp     // input
#define RF_DATA_OUTPUT_BIT         PIN2_bp     // output
#define RF_DATA_INPUT_BIT          PIN2_bp     // input
#define CHIP_ENABLE_BIT            PIN4_bp     // output
#define SYNCDET_BIT                PIN5_bp     // input

// define signal as port register direction the bit bilongs to

//#define RX_BIT_CLK_DIR_REG       PORTE.DIR
#define RF_DATA_DIR_REG            PORTF.DIR
#define CHIP_ENABLE_DIR_REG        PORTE.DIR
#define SYNCDET_DIR_REG            PORTE.DIR

// define signal as port register direction
//#define RX_BIT_CLK_DIR_BIT       PIN1_bp
#define RF_DATA_DIR_BIT            PIN2_bp
#define RF_DATA_ALT_DIR_BIT        PIN3_bp
#define CHIP_ENABLE_DIR_BIT        PIN4_bp
#define SYNCDET_DIR_BIT            PIN5_bp

#endif


#ifndef ATXMEGA256A3B_EVAL
// user pins defines as port belong to (OUTPUT or INPUT)
#define USER_PIN0_OUTPUT_PORT_REG          PORTB.OUT       // DIO0
#define USER_PIN0_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN0_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN0_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN0_INPUT_PORT_REG           PORTB.IN        // DIO0

#define USER_PIN1_OUTPUT_PORT_REG          PORTB.OUT       // DIO1
#define USER_PIN1_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN1_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN1_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN1_INPUT_PORT_REG           PORTB.IN        // DIO1

#define USER_PIN2_OUTPUT_PORT_REG          PORTB.OUT       // DIO2
#define USER_PIN2_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN2_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN2_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN2_INPUT_PORT_REG           PORTB.IN        // DIO2

#define USER_PIN3_OUTPUT_PORT_REG          PORTB.OUT       // DIO3
#define USER_PIN3_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN3_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN3_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN3_INPUT_PORT_REG           PORTB.IN        // DIO3


/*
#define USER_PIN4_OUTPUT_PORT_REG          PORTB.OUT       // DIO4
#define USER_PIN4_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN4_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN4_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN4_INPUT_PORT_REG           PORTB.IN        // DIO4

#define USER_PIN5_OUTPUT_PORT_REG          PORTB.OUT       // DIO5
#define USER_PIN5_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN5_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN5_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN5_INPUT_PORT_REG           PORTB.IN        // DIO5

#define USER_PIN6_OUTPUT_PORT_REG          PORTB.OUT       // DIO6
#define USER_PIN6_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN6_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN6_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN6_INPUT_PORT_REG           PORTB.IN        // DIO6

#define USER_PIN7_OUTPUT_PORT_REG          PORTD.OUT       // DIO7
#define USER_PIN7_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN7_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN7_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN7_INPUT_PORT_REG           PORTD.IN        // DIO7

#define USER_PIN8_OUTPUT_PORT_REG          PORTD.OUT       // DIO8
#define USER_PIN8_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN8_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN8_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN8_INPUT_PORT_REG           PORTD.IN        // DIO8

#define USER_PIN9_OUTPUT_PORT_REG          PORTD.OUT       // DIO9
#define USER_PIN9_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN9_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN9_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN9_INPUT_PORT_REG           PORTD.IN        // DIO9

#define USER_PIN10_OUTPUT_PORT_REG         PORTC.OUT       // DIO10
#define USER_PIN10_OUTPUT_TOGGLE_PORT_REG  PORTC.OUTTGL    // DIO0
#define USER_PIN10_OUTPUT_CLEAR_PORT_REG   PORTC.OUTCLR    // DIO0
#define USER_PIN10_OUTPUT_SET_PORT_REG     PORTC.OUTSET    // DIO0
#define USER_PIN10_INPUT_PORT_REG          PORTC.IN        // DIO10

#define USER_PIN11_OUTPUT_PORT_REG         PORTC.OUT       // DIO11
#define USER_PIN11_OUTPUT_TOGGLE_PORT_REG  PORTC.OUTTGL    // DIO0
#define USER_PIN11_OUTPUT_CLEAR_PORT_REG   PORTC.OUTCLR    // DIO0
#define USER_PIN11_OUTPUT_SET_PORT_REG     PORTC.OUTSET    // DIO0
#define USER_PIN11_INPUT_PORT_REG          PORTC.IN        // DIO11

#define USER_PIN12_OUTPUT_PORT_REG         PORTD.OUT       // DIO12
#define USER_PIN12_OUTPUT_TOGGLE_PORT_REG  PORTD.OUTTGL    // DIO0
#define USER_PIN12_OUTPUT_CLEAR_PORT_REG   PORTD.OUTCLR    // DIO0
#define USER_PIN12_OUTPUT_SET_PORT_REG     PORTD.OUTSET    // DIO0
#define USER_PIN12_INPUT_PORT_REG          PORTD.IN        // DIO12
*/


// define signal as port bit
#define USER_PIN0_BIT                      PIN1_bp         // DIO0
#define USER_PIN1_BIT                      PIN2_bp         // DIO1
#define USER_PIN2_BIT                      PIN3_bp         // DIO2
#define USER_PIN3_BIT                      PIN4_bp         // DIO3

/*
#define USER_PIN4_BIT                      PIN7_bp         // DIO4
#define USER_PIN5_BIT                      PIN6_bp         // DIO5
#define USER_PIN6_BIT                      PIN5_bp         // DIO6
#define USER_PIN7_BIT                      PIN6_bp         // DIO7
#define USER_PIN8_BIT                      PIN4_bp         // DIO8
#define USER_PIN9_BIT                      PIN7_bp         // DIO9
#define USER_PIN10_BIT                     PIN4_bp         // DIO10
#define USER_PIN11_BIT                     PIN5_bp         // DIO11
#define USER_PIN12_BIT                     PIN3_bp         // DIO12
*/

// define signal as port register direction the bit belongs to
#define USER_PIN0_DIR_REG                  PORTB.DIR       // DIO0
#define USER_PIN1_DIR_REG                  PORTB.DIR       // DIO1
#define USER_PIN2_DIR_REG                  PORTB.DIR       // DIO2
#define USER_PIN3_DIR_REG                  PORTB.DIR       // DIO3

/*
#define USER_PIN4_DIR_REG                  PORTB.DIR       // DIO4
#define USER_PIN5_DIR_REG                  PORTB.DIR       // DIO5
#define USER_PIN6_DIR_REG                  PORTB.DIR       // DIO6
#define USER_PIN7_DIR_REG                  PORTD.DIR       // DIO7
#define USER_PIN8_DIR_REG                  PORTD.DIR       // DIO8
#define USER_PIN9_DIR_REG                  PORTD.DIR       // DIO9
#define USER_PIN10_DIR_REG                 PORTC.DIR       // DIO10
#define USER_PIN11_DIR_REG                 PORTC.DIR       // DIO11
#define USER_PIN12_DIR_REG                 PORTD.DIR       // DIO12
*/

// define signal as port register direction the bit bilongs to
#define USER_PIN0_DIR_BIT                  PIN1_bp         // DIO0
#define USER_PIN1_DIR_BIT                  PIN2_bp         // DIO1
#define USER_PIN2_DIR_BIT                  PIN3_bp         // DIO2
#define USER_PIN3_DIR_BIT                  PIN4_bp         // DIO3

/*
#define USER_PIN4_DIR_BIT                  PIN7_bp         // DIO4
#define USER_PIN5_DIR_BIT                  PIN6_bp         // DIO5
#define USER_PIN6_DIR_BIT                  PIN5_bp         // DIO6
#define USER_PIN7_DIR_BIT                  PIN6_bp         // DIO7
#define USER_PIN8_DIR_BIT                  PIN4_bp         // DIO8
#define USER_PIN9_DIR_BIT                  PIN7_bp         // DIO9
#define USER_PIN10_DIR_BIT                 PIN4_bp         // DIO10
#define USER_PIN11_DIR_BIT                 PIN5_bp         // DIO11
#define USER_PIN12_DIR_BIT                 PIN3_bp         // DIO12
*/

#else

// a problem using PORTB : it is one of the JTAG signals
// so use PORTD
// user pins defines as port belong to (OUTPUT or INPUT)
#define USER_PIN0_OUTPUT_PORT_REG          PORTD.OUT       // DIO0
#define USER_PIN0_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN0_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN0_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN0_INPUT_PORT_REG           PORTD.IN        // DIO0

#define USER_PIN1_OUTPUT_PORT_REG          PORTD.OUT       // DIO1
#define USER_PIN1_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN1_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN1_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN1_INPUT_PORT_REG           PORTD.IN        // DIO1

#define USER_PIN2_OUTPUT_PORT_REG          PORTD.OUT       // DIO2
#define USER_PIN2_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN2_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN2_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN2_INPUT_PORT_REG           PORTD.IN        // DIO2

#define USER_PIN3_OUTPUT_PORT_REG          PORTD.OUT       // DIO3
#define USER_PIN3_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN3_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN3_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN3_INPUT_PORT_REG           PORTD.IN        // DIO3

#define USER_PIN3_OUTPUT_PORT_REG          PORTD.OUT       // DIO3
#define USER_PIN3_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN3_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN3_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN3_INPUT_PORT_REG           PORTD.IN        // DIO3

/*
#define USER_PIN4_OUTPUT_PORT_REG          PORTD.OUT       // DIO4
#define USER_PIN4_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN4_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN4_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN4_INPUT_PORT_REG           PORTD.IN        // DIO4

#define USER_PIN5_OUTPUT_PORT_REG          PORTD.OUT       // DIO5
#define USER_PIN5_OUTPUT_TOGGLE_PORT_REG   PORtD.OUTTGL    // DIO0
#define USER_PIN5_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN5_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN5_INPUT_PORT_REG           PORTD.IN        // DIO5

#define USER_PIN6_OUTPUT_PORT_REG          PORTD.OUT       // DIO6
#define USER_PIN6_OUTPUT_TOGGLE_PORT_REG   PORTD.OUTTGL    // DIO0
#define USER_PIN6_OUTPUT_CLEAR_PORT_REG    PORTD.OUTCLR    // DIO0
#define USER_PIN6_OUTPUT_SET_PORT_REG      PORTD.OUTSET    // DIO0
#define USER_PIN6_INPUT_PORT_REG           PORTD.IN        // DIO6


#define USER_PIN7_OUTPUT_PORT_REG          PORTD.OUT       // DIO7
#define USER_PIN7_OUTPUT_TOGGLE_PORT_REG   PORT.OUTTGL    // DIO0
#define USER_PIN7_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN7_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN7_INPUT_PORT_REG           PORTD.IN        // DIO7

#define USER_PIN8_OUTPUT_PORT_REG          PORTD.OUT       // DIO8
#define USER_PIN8_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN8_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN8_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN8_INPUT_PORT_REG           PORTD.IN        // DIO8

#define USER_PIN9_OUTPUT_PORT_REG          PORTD.OUT       // DIO9
#define USER_PIN9_OUTPUT_TOGGLE_PORT_REG   PORTB.OUTTGL    // DIO0
#define USER_PIN9_OUTPUT_CLEAR_PORT_REG    PORTB.OUTCLR    // DIO0
#define USER_PIN9_OUTPUT_SET_PORT_REG      PORTB.OUTSET    // DIO0
#define USER_PIN9_INPUT_PORT_REG           PORTD.IN        // DIO9

#define USER_PIN10_OUTPUT_PORT_REG         PORTC.OUT       // DIO10
#define USER_PIN10_OUTPUT_TOGGLE_PORT_REG  PORTc.OUTTGL    // DIO0
#define USER_PIN10_OUTPUT_CLEAR_PORT_REG   PORTC.OUTCLR    // DIO0
#define USER_PIN10_OUTPUT_SET_PORT_REG     PORTC.OUTSET    // DIO0
#define USER_PIN10_INPUT_PORT_REG          PORTC.IN        // DIO10

#define USER_PIN11_OUTPUT_PORT_REG         PORTC.OUT       // DIO11
#define USER_PIN11_OUTPUT_TOGGLE_PORT_REG  PORTC.OUTTGL    // DIO0
#define USER_PIN11_OUTPUT_CLEAR_PORT_REG   PORTC.OUTCLR    // DIO0
#define USER_PIN11_OUTPUT_SET_PORT_REG     PORTC.OUTSET    // DIO0
#define USER_PIN11_INPUT_PORT_REG          PORTC.IN        // DIO11

#define USER_PIN12_OUTPUT_PORT_REG         PORTD.OUT       // DIO12
#define USER_PIN12_OUTPUT_TOGGLE_PORT_REG  PORTD.OUTTGL    // DIO0
#define USER_PIN12_OUTPUT_CLEAR_PORT_REG   PORTD.OUTCLR    // DIO0
#define USER_PIN12_OUTPUT_SET_PORT_REG     PORTD.OUTSET    // DIO0
#define USER_PIN12_INPUT_PORT_REG          PORTD.IN        // DIO12
*/


// define signal as port bit
#define USER_PIN0_BIT                      PIN1_bp         // DIO0
#define USER_PIN1_BIT                      PIN2_bp         // DIO1
#define USER_PIN2_BIT                      PIN3_bp         // DIO2
#define USER_PIN3_BIT                      PIN4_bp         // DIO3

/*
#define USER_PIN4_BIT                      PIN7_bp         // DIO4
#define USER_PIN5_BIT                      PIN6_bp         // DIO5
#define USER_PIN6_BIT                      PIN5_bp         // DIO6
#define USER_PIN7_BIT                      PIN6_bp         // DIO7
#define USER_PIN8_BIT                      PIN4_bp         // DIO8
#define USER_PIN9_BIT                      PIN7_bp         // DIO9
#define USER_PIN10_BIT                     PIN4_bp         // DIO10
#define USER_PIN11_BIT                     PIN5_bp         // DIO11
#define USER_PIN12_BIT                     PIN3_bp         // DIO12
*/

// define signal as port register direction the bit belongs to
#define USER_PIN0_DIR_REG                  PORTD.DIR       // DIO0
#define USER_PIN1_DIR_REG                  PORTD.DIR       // DIO1
#define USER_PIN2_DIR_REG                  PORTD.DIR       // DIO2
#define USER_PIN3_DIR_REG                  PORTD.DIR       // DIO3

/*
#define USER_PIN4_DIR_REG                  PORTB.DIR       // DIO4
#define USER_PIN5_DIR_REG                  PORTB.DIR       // DIO5
#define USER_PIN6_DIR_REG                  PORTB.DIR       // DIO6
#define USER_PIN7_DIR_REG                  PORTD.DIR       // DIO7
#define USER_PIN8_DIR_REG                  PORTD.DIR       // DIO8
#define USER_PIN9_DIR_REG                  PORTD.DIR       // DIO9
#define USER_PIN10_DIR_REG                 PORTC.DIR       // DIO10
#define USER_PIN11_DIR_REG                 PORTC.DIR       // DIO11
#define USER_PIN12_DIR_REG                 PORTD.DIR       // DIO12
*/

// define signal as port register direction the bit bilongs to
#define USER_PIN0_DIR_BIT                  PIN1_bp         // DIO0
#define USER_PIN1_DIR_BIT                  PIN2_bp         // DIO1
#define USER_PIN2_DIR_BIT                  PIN3_bp         // DIO2
#define USER_PIN3_DIR_BIT                  PIN4_bp         // DIO3

/*
#define USER_PIN4_DIR_BIT                  PIN7_bp         // DIO4
#define USER_PIN5_DIR_BIT                  PIN6_bp         // DIO5
#define USER_PIN6_DIR_BIT                  PIN5_bp         // DIO6
#define USER_PIN7_DIR_BIT                  PIN6_bp         // DIO7
#define USER_PIN8_DIR_BIT                  PIN4_bp         // DIO8
#define USER_PIN9_DIR_BIT                  PIN7_bp         // DIO9
#define USER_PIN10_DIR_BIT                 PIN4_bp         // DIO10
#define USER_PIN11_DIR_BIT                 PIN5_bp         // DIO11
#define USER_PIN12_DIR_BIT                 PIN3_bp         // DIO12
*/

#endif


/*
// led
#define LED2
#define LED3

#define LED2_DIR
#define LED3_DIR

#define TX_LED
#define RX_LED
*/



//! @} IO_PORT_MAPPING_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup IO_PORT_MAPPING_pub_func
//! \ingroup IO_PORT_MAPPING_ADI_EVAL
//! @{

//! @} IO_PORT_MAPPING_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} IO_PORT_MAPPING_EX_ADI

#endif
