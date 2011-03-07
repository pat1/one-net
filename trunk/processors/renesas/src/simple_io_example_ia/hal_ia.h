#ifndef _HAL_IA_H
#define _HAL_IA_H

//! \defgroup HAL_IA Processor Abstraction Layer for Integration Associates
//!   IA4421.
//! \ingroup IA
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
    \file hal_ia.h
    \brief Processor abstraction layer for the Integration Associates IA4421.

    This file declares the processor & board specific functionality needed by
    the Integration Associates transceiver.
*/

#include "pal.h"

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
//                              PUBLIC VARIABLES
//! \defgroup HAL_IA_pub_var
//! \ingroup HAL_IA
//! @{

//! @} HAL_IA_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup HAL_IA_pub_func
//! \ingroup HAL_IA
//! @{

/*!
    \brief Enables the interrupt used to receive bits from the rf interface.

    This is the interrupt attached to the IRQ pin from the IA4421.

    \param void

    return void
*/
#define ENABLE_RX_BIT_INTERRUPTS()   bit_mask = 0x80; int1ic |= 0x07;


/*!
    \brief Disables the interrupt used to receive bits over the rf interface.

    This is the interrupt attached to the IRQ pin from the IA.

    \param void

    return void
*/
#define DISABLE_RX_BIT_INTERRUPTS() int1ic &= 0xF8;


/*!
    \brief Enables the interrupt used to transmit bits over the rf interface.

    \param void

    return void
*/
#define ENABLE_TX_BIT_INTERRUPTS()


/*!
    \brief Disables the interrupt used to transmit bits over the rf interface.

    \param void

    return void
*/
#define DISABLE_TX_BIT_INTERRUPTS()

//! @} HAL_IA_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} IA

#endif // ifndef _HAL_IA_H //
