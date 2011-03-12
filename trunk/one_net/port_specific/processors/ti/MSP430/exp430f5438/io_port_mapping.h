#ifndef _IO_PORT_MAPPING_H
#define _IO_PORT_MAPPING_H

//! \defgroup IO_PORT_MAPPING_ADI_SW I/O Port Mapping for the ADI Switch Example
//!   Project.
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
    \file io_port_mapping.h
    \brief Contains the port definitions for the ONE-NET switch example project
      using an ADI transceiver.
      
    The specific mapping of i/o ports to other components may vary from board to
    board.  This file contains symbolic names for port functions that are mapped
    to the i/o ports on the ONE-NET Example Boards using an ADI tranceiver and
    Renesas R8C1B processor.
*/

//==============================================================================
//                                  CONSTANTS
//! \defgroup IO_PORT_MAPPING_const
//! \ingroup IO_PORT_MAPPING_ADI_SW
//! @{

//! @} IO_PORT_MAPPING_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup IO_PORT_MAPPING_typedefs
//! \ingroup IO_PORT_MAPPING_ADI_SW
//! @{

//! @} IO_PORT_MAPPING_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup IO_PORT_MAPPING_pub_var
//! \ingroup IO_PORT_MAPPING_ADI_SW
//! @{
// Serial I/O pins for TI register access
#define SCLK              p3_3
#define MOSI              p3_1
#define MISO              pi3_2
#define SSNOT             p3_0


#define SCLK_DIR          pd3_3
#define MOSI_DIR          pd3_1
#define MISO_DIR          pd3_2
#define SSNOT_DIR         pd3_0

#define SCLK_SEL          ps3_3
#define MOSI_SEL          ps3_1
#define MISO_SEL          ps3_2

#define MISO_REN          pre3_2
#define MISO_OUT          p3_2


// General Data Out lines
#define GDO2              pi1_3
#define GDO0              pi1_7

#define GDO2_DIR          pd1_3
#define GDO0_DIR          pd1_7

// UART
#define USB_PORT_OUT      P5OUT
#define USB_PORT_SEL      P5SEL
#define USB_PORT_DIR      P5DIR
#define USB_PORT_REN      P5REN
#define USB_PIN_TXD       BIT6
#define USB_PIN_RXD       BIT7

/* */
// switch
#define BUTTON_PORT_DIR   P2DIR
#define BUTTON_PORT_SEL   P2SEL
#define BUTTON_PORT_OUT   P2OUT
#define BUTTON_PORT_REN   P2REN
#define BUTTON_PORT_IE    P2IE
#define BUTTON_PORT_IES   P2IES
#define BUTTON_PORT_IFG   P2IFG
#define BUTTON_PORT_IN    P2IN

#define BUTTON_SELECT     BIT3
#define BUTTON_DOWN       BIT5
#define BUTTON_UP         BIT4
#define BUTTON_RIGHT      BIT2
#define BUTTON_LEFT       BIT1 
#define BUTTON_S1         BIT6 
#define BUTTON_S2         BIT7 
#define BUTTON_ALL        0xFE

#define SW1               (!(pi2_6))
#define SW2               (!(pi2_7))
#define SW_SELECT         (!(pi2_3))
#define SW_UP             (!(pi2_4))
#define SW_DOWN           (!(pi2_5))
#define SW_RIGHT          (!(pi2_2))
#define SW_LEFT           (!(pi2_1))

//#define SW_OUT          p1_7
//#define SW_DIR          pd1_7
//#define SW_REN          pre1_7
#define SW1_IE           pie2_6
#define SW1_ES           pis2_6
//#define SW_IFG          pif1_7

#define SW_MODE_SELECT  (!(P5IN & BIT0))
#define SW_ADDR_SELECT1 (!(P6IN & BIT7))
#define SW_ADDR_SELECT2 (!(P7IN & BIT5))

// rx led
#define LED2            p1_0
#define LED2_DIR        pd1_0

// tx led
#define LED3            p1_1
#define LED3_DIR        pd1_1

#define RX_LED          LED2
#define TX_LED          LED3

// user pins
#define USER_PIN0_OUT   p4_0
#define USER_PIN1_OUT   p4_2
#define USER_PIN2_OUT   p4_5
#define USER_PIN3_OUT   p4_7

#define USER_PIN0_IN    pi4_0
#define USER_PIN1_IN    pi4_2
#define USER_PIN2_IN    pi4_5
#define USER_PIN3_IN    pi4_7

#define USER_PIN0_DIR   pd4_0
#define USER_PIN1_DIR   pd4_2
#define USER_PIN2_DIR   pd4_4
#define USER_PIN3_DIR   pd4_7

#define USER_PIN0_REN   pre4_0
#define USER_PIN1_REN   pre4_2
#define USER_PIN2_REN   pre4_4
#define USER_PIN3_REN   pre4_7
/* */
//! @} IO_PORT_MAPPING_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup IO_PORT_MAPPING_pub_func
//! \ingroup IO_PORT_MAPPING_ADI_SW
//! @{

//! @} IO_PORT_MAPPING_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} IO_PORT_MAPPING_EX_ADI

#endif
