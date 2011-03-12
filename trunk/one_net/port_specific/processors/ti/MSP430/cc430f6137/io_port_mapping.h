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


#include "x613x.h"

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

// UART
#define RXD0            pi1_5
#define TXDO            p1_6

#define RXD0_DIR        pd1_5
#define TXDO_DIR        pd1_6

#define RXD0_SEL        ps1_5
#define TXDO_SEL        ps1_6

/* */
// switch
#define SW              pi1_7
#define SW_OUT          p1_7
#define SW_DIR          pd1_7
#define SW_REN          pre1_7
#define SW_IE           pie1_7
#define SW_ES           pis1_7
#define SW_IFG          pif1_7

#define SW_MODE_SELECT  (!(pi2_0))
#define SW_ADDR_SELECT1 (!(pi2_2))
#define SW_ADDR_SELECT2 (!(pi2_4))

// rx led
#define LED2            p1_0
#define LED2_DIR        pd1_0

// tx led
#define LED3            p3_6
#define LED3_DIR        pd3_6

#define RX_LED          LED2
#define TX_LED          LED3

// user pins
#define USER_PIN0_OUT   p4_0
#define USER_PIN1_OUT   p4_2
#define USER_PIN2_OUT   p4_4
#define USER_PIN3_OUT   p4_6

#define USER_PIN0_IN    pi4_0
#define USER_PIN1_IN    pi4_2
#define USER_PIN2_IN    pi4_4
#define USER_PIN3_IN    pi4_6

#define USER_PIN0_DIR   pd4_0
#define USER_PIN1_DIR   pd4_2
#define USER_PIN2_DIR   pd4_4
#define USER_PIN3_DIR   pd4_6

#define USER_PIN0_REN   pre4_0
#define USER_PIN1_REN   pre4_2
#define USER_PIN2_REN   pre4_4
#define USER_PIN3_REN   pre4_6
/* */
/* 
// switch
#define SW              1
// led
#define LED             p1_2
#define LED_DIR         pd1_2

// tx led
#define TXLED           p1_3
#define TXLED_DIR       pd1_3
 */
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
