#ifndef _IO_PORT_MAPPING_H
#define _IO_PORT_MAPPING_H

//! \defgroup IO_PORT_MAPPING_IA_SW I/O Port Mapping for the Integration
//!   Associates Switch Example Project.
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
      using an IA transceiver.
      
    The specific mapping of i/o ports to other components may vary from board to
    board.  This file contains symbolic names for port functions that are mapped
    to the i/o ports on the ONE-NET Example Boards using a IA tranceiver
    and Renesas R8C1B processor.
*/


#include "sfr_r81B.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup IO_PORT_MAPPING_const
//! \ingroup IO_PORT_MAPPING_IA_SW
//! @{

//! @} IO_PORT_MAPPING_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup IO_PORT_MAPPING_typedefs
//! \ingroup IO_PORT_MAPPING_IA_SW
//! @{

//! @} IO_PORT_MAPPING_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup IO_PORT_MAPPING_pub_var
//! \ingroup IO_PORT_MAPPING_IA_SW
//! @{

// Serial I/O pins for IA register access
#define SCLK            p3_5
#define MOSI            p3_7
#define MISO            p1_6
#define SSNOT           p3_4


#define SCLK_DIR        pd3_5
#define MOSI_DIR        pd3_7
#define MISO_DIR        pd1_6
#define SSNOT_DIR       pd3_4


// data, irq lines
#define FSEL_NOT        p3_3
#define DATA_CLK        p1_7
#define IRQ             p4_5
#define VDI             p1_0

#define FSEL_NOT_DIR    pd3_3
#define DATA_CLK_DIR    pd1_7
#define IRQ_DIR         pd4_5
#define VDI_DIR         pd1_0


// switch
#define SW              p1_2

#define SW_DIR          pd1_2


// led
#define LED             p1_3

#define LED_DIR         pd1_3

//! @} IO_PORT_MAPPING_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup IO_PORT_MAPPING_pub_func
//! \ingroup IO_PORT_MAPPING_IA_SW
//! @{

//! @} IO_PORT_MAPPING_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} IO_PORT_MAPPING_EX_IA

#endif
