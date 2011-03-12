#ifndef _HAL_TI_H
#define _HAL_TI_H

//! \defgroup HAL_TI Processor Abstraction Layer for TI CC1100
//! \ingroup TI
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
    \file hal_ti.h
    \brief Processor abstraction layer for the TI CC1100.

    This file declares the processor & board specific functionality needed by
    the TI CC1100 transceiver.
*/

#include "pal.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup HAL_TI_const
//! \ingroup HAL_TI
//! @{

//! @} HAL_TI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_TI_typedefs
//! \ingroup HAL_TI
//! @{

//! @} HAL_TI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup HAL_TI_pub_var
//! \ingroup HAL_TI
//! @{

//! @} HAL_TI_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup HAL_TI_pub_func
//! \ingroup HAL_TI
//! @{

extern void turn_on_tx_led(void);
extern void turn_on_rx_led(void);


/*!
    \brief Enables the interrupt used to receive bits from the rf interface.

    \param void

    \return void
*/
#define ENABLE_RX_BIT_INTERRUPTS()


/*!
    \brief Disables the interrupt used to receive bits over the rf interface.

    \param void

    \return void
*/
#define DISABLE_RX_BIT_INTERRUPTS()


/*!
    \brief Enables the interrupt used to transmit bits over the rf interface.

    \param void

    \return void
*/
#define ENABLE_TX_BIT_INTERRUPTS()


/*!
    \brief Disables the interrupt used to transmit bits over the rf interface.

    \param void

    \return void
*/
#define DISABLE_TX_BIT_INTERRUPTS()


/*!
    \brief turns on the transmit LED
    
    \param void
    
    \return void
*/
#define TURN_ON_TX_LED() turn_on_tx_led()
//#define TURN_ON_TX_LED()


/*!
    \brief turns on the receive LED
    
    \param void
    
    \return void
*/
#define TURN_ON_RX_LED() turn_on_rx_led()
//#define TURN_ON_RX_LED()

//! @} HAL_TI_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} TI

#endif // ifndef _HAL_TI_H //

