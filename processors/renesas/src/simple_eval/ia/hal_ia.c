//! \addtogroup HAL_IA Processor Abstraction Layer for Integration Associates
//!   IA4421
//! \ingroup IA
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
    \file hal_ia.c
    \brief Processor abstraction layer for the Integration Associates IA4421.

    This file implements the processor specific functionality needed by the
    Semtech transceiver (such as interrupts for the various communication).
*/

#include "hal_ia.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "tal.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup HAL_IA_const
//! \ingroup HAL_IA
//! @{

//! @} HAL_IA_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_IA_typedefs
//! \ingroup HAL_IA
//! @{

//! @} HAL_IA_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup HAL_IA_pri_var
//! \ingroup HAL_IA
//! @{

// These are derived from ia.c
extern UInt16 rf_idx;
extern UInt16 rf_count;
extern UInt8 rf_data[];

extern UInt8 bit_mask;

//! @} HAL_IA_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup HAL_IA_pri_func
//! \ingroup HAL_IA
//! @{

//! @} HAL_IA_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup HAL_IA_pub_func
//! \ingroup HAL_IA
//! @{

/*!
    \brief Initialize ports used by the IA.

    \param void

    \return void
*/
void tal_init_ports(void)
{
    FSEL_NOT_DIR = OUTPUT;
    FSEL_NOT = 1;

    DATA_CLK_DIR = INPUT;
    IRQ_DIR = INPUT;
    VDI_DIR = INPUT;

    SSNOT_DIR = OUTPUT;
    SSNOT = 1;
} // tal_init_ports //

//! @} HAL_IA_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup HAL_IA_pri_func
//! \ingroup HAL_IA
//! @{

//! @} HAL_IA_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} IA

