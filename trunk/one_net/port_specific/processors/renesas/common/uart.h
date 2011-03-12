#ifndef _UART_H
#define _UART_H


//! \defgroup uart UART functionality
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
    \file uart.h

    \brief Contains declarations for functions associated with asynchronous
      serial i/o.
*/

#include <one_net/port_specific/config_options.h>
#include "uart_hal.h"

#include "one_net_types.h"
#include "pal.h"

//==============================================================================
//								CONSTANTS 
//! \defgroup uart_const 
//! \ingroup uart
//! @{

//! @} uart_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup uart_typedefs
//! \ingroup uart
//! @{

typedef enum
{
    STOP_BITS_1 = 0x00,             //!< 1 stop bit
    STOP_BITS_2 = 0x10              //!< 2 stop bit
} stop_bits_t;


typedef enum
{
    DATA_BITS_7 = 0x04,             //!< 7 data bits
    DATA_BITS_8 = 0x05,             //!< 8 data bits
    DATA_BITS_9 = 0x06,             //!< 9 data bits
} data_bits_t;


typedef enum
{
    PARITY_NONE = 0x00,             //!< No parity
    PARITY_EVEN = 0x60,             //!< Even parity
    PARITY_ODD = 0x40               //!< Odd parity
} parity_t;

//! @} uart_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup uart_pub_var
//! \ingroup uart
//! @{

//! @} uart_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup uart_pub_func
//! \ingroup uart
//! @{

/*!
    \brief Disables UART transmit interrupts
    
    \param void
    
    \return void
*/
#define DISABLE_TX_INTR() te_u0c1 = 0


/*!
    \brief Enables UART transmit interrupts
    
    \param void
    
    \return void
*/
#define ENABLE_TX_INTR() te_u0c1 = 1


/*!
    \brief Disables the uart receive interrupt
    
    \param void
    
    \return void
*/
#define DISABLE_RX_INTR() re_u0c1 = 0


/*!
    \brief Enables the uart receive interrupt
    
    \param void
    
    \return void
*/
#define ENABLE_RX_INTR() re_u0c1 = 1

void init_ports_uart(void);
void uart_init(const UInt8 BAUD_RATE, const UInt8 DATA_BITS,
  const UInt8 STOP_BITS, const UInt8 PARITY);

UInt16 uart_tx_bytes_free(void);
UInt16 uart_rx_bytes_available(void);

UInt16 uart_rx_buffer_size(void);
UInt16 uart_tx_buffer_size(void);

UInt16 uart_read(UInt8 * const data, const UInt16 LEN);
UInt16 uart_write(const UInt8 * const DATA, const UInt16 LEN);
void uart_write_int8_hex(const UInt8 DATA);

//! @} uart_pub_func
//                  PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} uart

#endif // #ifdef _UART_H //

