#ifndef _IO_PORT_MAPPING_H
#define _IO_PORT_MAPPING_H

//! \defgroup IO_PORT_MAPPING_SEMTECH_EVAL I/O Port Mapping for the
//!   Evaluation Project using a Semtech XE1205 transceiver.
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
      using an Semtech transceiver.
      
    The specific mapping of i/o ports to other components may vary from board to
    board.  This file contains symbolic names for port functions that are mapped
    to the i/o ports on the ONE-NET Example Boards using an Semtech tranceiver
    and Renesas R8C23 processor.
*/


#include "sfr_r823.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup IO_PORT_MAPPING_const
//! \ingroup IO_PORT_MAPPING_SEMTECH_EVAL
//! @{

//! @} IO_PORT_MAPPING_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup IO_PORT_MAPPING_typedefs
//! \ingroup IO_PORT_MAPPING_SEMTECH_EVAL
//! @{

//! @} IO_PORT_MAPPING_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup IO_PORT_MAPPING_pub_var
//! \ingroup IO_PORT_MAPPING_SEMTECH_EVAL
//! @{

// Serial I/O pins for Semtech register access
#define SCLK            p2_0
#define MOSI            p2_1
#define MISO            p2_2
#define NSS_CFG         p2_6
#define NSS_DATA        p2_5

#define SCLK_DIR        pd2_0
#define MOSI_DIR        pd2_1
#define MISO_DIR        pd2_2
#define NSS_CFG_DIR     pd2_6
#define NSS_DATA_DIR    pd2_5


// data, clock, and pattern detect signals
#define RF_DATA         p2_4
#define DCLK_IRQ1       p4_5
#define IRQ0            p6_6

#define RF_DATA_DIR     pd2_4
#define DCLK_IRQ1_DIR   pd4_5
#define IRQ0_DIR        pd6_6


// switch
#define SW4             p3_7
#define SW5             p0_0
#define SW6             p0_1

#define SW4_DIR         pd3_7
#define SW5_DIR         pd0_0
#define SW6_DIR         pd0_1

#ifdef _AUTO_MODE
	#define SW_MODE_SELECT  SW4
#endif

#define SW_ADDR_SELECT1 SW5
#define SW_ADDR_SELECT2 SW6


// led
#define LED2            p0_4
#define LED3            p0_5

#define LED2_DIR        pd0_4
#define LED3_DIR        pd0_5

#define TX_LED          LED3
#define RX_LED          LED2


// user pins
#define USER_PIN0       p6_7
#define USER_PIN1       p6_2
#define USER_PIN2       p6_1
#define USER_PIN3       p0_3

#define USER_PIN0_DIR   pd6_7
#define USER_PIN1_DIR   pd6_2
#define USER_PIN2_DIR   pd6_1
#define USER_PIN3_DIR   pd0_3


// uart pins
#define FLASH_CHECK_RX_PIN     p1_5
#define FLASH_CHECK_TX_PIN     p1_4

#define FLASH_CHECK_RX_PIN_DIR pd1_5
#define FLASH_CHECK_TX_PIN_DIR pd1_4

//! @} IO_PORT_MAPPING_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup IO_PORT_MAPPING_pub_func
//! \ingroup IO_PORT_MAPPING_SEMTECH_EVAL
//! @{

//! @} IO_PORT_MAPPING_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} IO_PORT_MAPPING_SEMTECH_EVAL

#endif
