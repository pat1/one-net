#ifndef _UART_HAL_H
#define _UART_HAL_H

//! \defgroup uart_hal UART hardware dependent functionality
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
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
    \file uart_hal.h

    \brief Contains declarations for hardware dependent uart functionality.
    
    This file needs to declare the baud rate and initial uart0 control value.
    The uart settings need to be of the form BAUD_9600, BAUD_19200, etc.
    The initial uart0 control value needs to be defined INIT_UART0_CONTROL.
*/


//==============================================================================
//								CONSTANTS 
//! \defgroup uart_hal_const
//! \ingroup uart_hal
//! @{

#include <one_net/port_specific/config_options.h>
#include <msp430.h>

enum
{
    UART_RX_BUF_SIZE = 16,          //!< Size of the uart receive buffer
// VladK
    UART_TX_BUF_SIZE = 216          //!< Size of the uart transmit buffer
//    UART_TX_BUF_SIZE = 80          //!< Size of the uart transmit buffer
};

//! @} uart_hal_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup uart_hal_typedefs
//! \ingroup uart_hal
//! @{

//! The baud rate numbers are assuming a 20Mhz system clock divided by 1
//! Note that the -1 is done when the uart brg register is set
typedef enum
{
    BAUD_9600 = 0,
    BAUD_19200 = 1,
    BAUD_38400 = 2,                 //!< Baud rate of 38400
    BAUD_56000 = 3,
    BAUD_115200 = 4,               //!< Baud rate of 115200
    BAUD_128000 = 5,
    BAUD_256000 = 6
} baud_rate_t;

//! @} uart_hal_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup uart_hal_pub_var
//! \ingroup uart_hal
//! @{

//! @} uart_hal_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup uart_hal_pub_func
//! \ingroup uart_hal
//! @{

//! @} uart_hal_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} uart_hal

#endif // #ifdef _UART_HAL_H //
