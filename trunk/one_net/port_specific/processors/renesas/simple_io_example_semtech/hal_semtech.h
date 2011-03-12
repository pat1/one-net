#ifndef _HAL_SEMTECH_H
#define _HAL_SEMTECH_H

//! \defgroup HAL_Semtech Processor Abstraction Layer for Semtech XE1205.
//! \ingroup SEMTECH
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
    \file hal_semtech.h
    \brief Processor abstraction layer for the SEMTECH XE1205.

    This file declares the processor & board specific functionality needed by
    the Semtech transceiver.
*/

#include "pal.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup HAL_SEMTECH_const
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_SEMTECH_typedefs
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup HAL_SEMTECH_pub_var
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup HAL_SEMTECH_pub_func
//! \ingroup HAL_SEMTECH
//! @{

/*!
    \brief Enables the interrupt used to receive bits from the rf interface.

    This is the interrupt attached to the DCLK_IRQ1 pin from the XE1205.

    \param void

    return void
*/
#define ENABLE_RX_BIT_INTERRUPTS()   bit_mask = 0x80; int0en = 1;


/*!
    \brief Disables the interrupt used to receive bits over the rf interface.

    This is the interrupt attached to the DATACLK pin from the Semtech.

    \param void

    return void
*/
#define DISABLE_RX_BIT_INTERRUPTS() int0en = 0;


/*!
    \brief Enables the interrupt used to transmit bits over the rf interface.

    This is a timer interrupt.

    \param void

    return void
*/
#define ENABLE_TX_BIT_INTERRUPTS()   bit_mask = 0x80; tzs = 1


/*!
    \brief Disables the interrupt used to transmit bits over the rf interface.

    This is a timer interrupt.

    \param void

    return void
*/
#define DISABLE_TX_BIT_INTERRUPTS()   tzs = 0

//! @} HAL_SEMTECH_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} SEMTECH

#endif // ifndef _HAL_SEMTECH_H //
