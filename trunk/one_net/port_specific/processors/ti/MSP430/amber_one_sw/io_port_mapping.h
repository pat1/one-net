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


#include "x22x4.h"

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
#define SCLK            p3_3
#define MOSI            p3_1
#define MISO            pi3_2
#define SSNOT           p3_0


#define SCLK_DIR        pd3_3
#define MOSI_DIR        pd3_1
#define MISO_DIR        pd3_2
#define SSNOT_DIR       pd3_0

#define SCLK_SEL        ps3_3
#define MOSI_SEL        ps3_1
#define MISO_SEL        ps3_2

// General Data Out lines
#define GDO2            pi2_7
#define GDO0            pi2_6

#define GDO2_DIR        pd2_7
#define GDO0_DIR        pd2_6

#define GDO2_SEL        ps2_7
#define GDO0_SEL        ps2_6

// UART
#define RXD0            pi3_5
#define TXDO            p3_4

#define RXD0_DIR        pd3_5
#define TXDO_DIR        pd3_4

#define RXD0_SEL        ps3_5
#define TXDO_SEL        ps3_4

/* */
// switch
#define SW              pi1_2
#define SW_OUT          p1_2
#define SW_DIR          pd1_2
#define SW_REN          pre1_2
#define SW_IE           pie1_2
#define SW_ES           pis1_2
#define SW_IFG          pif1_2


// led
#define LED2            p1_1
#define LED2_DIR        pd1_1

// tx led
#define LED3            p1_0
#define LED3_DIR        pd1_0

#define RX_LED          LED2
#define TX_LED          LED3

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
