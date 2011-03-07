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


#include "sfr_r81B.h"

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

// These are here since they are input only ports.  Don't want to take out
// setting these pins to inputs since if the function that uses them may change
// pins, and we'd have to modify the cod based on this (if we remember).
#define    pd4_2         pd4_addr.bit.b2       /* Port P42 direction bit */
#define    pd4_7         pd4_addr.bit.b7       /* Port P47 direction bit */

// Serial I/O pins for ADI register access
#define SCLK            p3_5
#define SDATA           p3_7
#define SLE             p3_4
#define SREAD           p4_7

#define SCLK_DIR        pd3_5
#define SDATA_DIR       pd3_7
#define SLE_DIR         pd3_4
#define SREAD_DIR       pd4_7


// data, clock, and pattern detect signals
#define RF_DATA         p3_3
#define RX_BIT_CLK      p1_7
#define SYNCDET         p4_2

#define RF_DATA_DIR     pd3_3
#define RX_BIT_CLK_DIR  pd1_7
#define SYNCDET_DIR     pd4_2


// io
#ifdef _QUAD_INPUT
    #define INPUT1     p1_6
    #define INPUT2     p1_1
    #define INPUT3     p1_0
    #define INPUT4     p4_5
    
    #define INPUT1_DIR pd1_6
    #define INPUT2_DIR pd1_1
    #define INPUT3_DIR pd1_0
    #define INPUT4_DIR pd4_5
#elif defined(_DUAL_OUTPUT) // ifdef _QUAD_INPUT //
    #define OUTPUT1     p1_6
    #define OUTPUT2     p1_1
    
    #define OUTPUT1_DIR pd1_6
    #define OUTPUT2_DIR pd1_1
#elif defined(_QUAD_OUTPUT) // ifdef _DUAL_OUTPUT_INPUT //
    #define OUTPUT1     p1_6
    #define OUTPUT2     p1_1
    #define OUTPUT3     p1_0
    #define OUTPUT4     p4_5
    
    #define OUTPUT1_DIR pd1_6
    #define OUTPUT2_DIR pd1_1
    #define OUTPUT3_DIR pd1_0
    #define OUTPUT4_DIR pd4_5
#else // else if _QUAD_OUTPUT is defined //
    #error Unknown board type (see io_port_mapping.h)
#endif // else _QUAD_INPUT, _QUAD_OUPUT, and _DUAL_OUTPUT are defined //


// led
#define LED2            p1_3
#define LED3            p1_2

#define LED2_DIR        pd1_3
#define LED3_DIR        pd1_2

#define RX_LED          LED2
#define TX_LED          LED3


// pins used to check if the flash should be erased.
#define FLASH_CHECK_RX_PIN      p1_5
#define FLASH_CHECK_TX_PIN      p1_4

#define FLASH_CHECK_RX_PIN_DIR  pd1_5
#define FLASH_CHECK_TX_PIN_DIR  pd1_4


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
