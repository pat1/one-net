//! \addtogroup uart
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
    \file uart.c
 
    \brief Contains functions for accessing the Renesas serial port
      and using the cb functions for passing data to and from the ISRs.

    Contains functions for initializing the Renesas serial port for interrupt
    driven i/o and interrupt service routines (ISRs) for handling serial data.
*/

#include <one_net/port_specific/uart.h>

#include <one_net/common/cb.h>


//==============================================================================
//								CONSTANTS 
//! \defgroup uart_const
//! \ingroup uart
//! @{

const char HEX_DIGIT[] = "0123456789ABCDEF";

//! @} uart_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup uart_typedefs
//! \ingroup uart
//! @{

//! @} uart_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup uart_pri_var
//! \ingroup uart
//! @{

//! uart receive buffer
#ifndef _SNIFFER_FRONT_END
static UInt8 uart_rx_buf[UART_RX_BUF_SIZE];
#else
static UInt8 uart_rx_buf[10];
#endif

//! uart transmit buffer
#ifndef _SNIFFER_FRONT_END
static UInt8 uart_tx_buf[UART_TX_BUF_SIZE];
#else
static UInt8 uart_tx_buf[1];
#endif

//! uart receive circular buffer
cb_rec_t uart_rx_cb = {0, 0, sizeof(uart_rx_buf) - 1, 0x00, uart_rx_buf};

//! uart receive circular buffer
cb_rec_t uart_tx_cb = {0, 0, sizeof(uart_tx_buf) - 1, 0x00, uart_tx_buf};

//! @} uart_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup uart_pri_func
//! \ingroup uart
//! @{


/*!
    \brief Returns TRUE if uart transmit buffer is empty
    
    This is the buffer waiting to be moved to the transmit register.
    
    \param void
    
    \return TRUE if uart transmit buffer is empty
            FALSE if uart transmit buffer is not empty
*/
#define TX_BUFFER_EMPTY() (IFG2 & UCA0TXIFG)


/*!
    \brief Returns TRUE if the uart transmit register is empty
    
    This is the register from which the data is sent over the line.
    
    \param void
    
    \return TRUE if the uart transmit register is empty
            FALSE if the uart transmit register is not empty
*/
//#define TX_REG_EMPTY() (BOOL)txept_u0c0


//! @} uart_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup uart_pub_func
//! \ingroup uart
//! @{
void init_ports_uart(void)
{
    RXD0_SEL = SPECIAL;
    TXDO_SEL = SPECIAL;
      
    RXD0_DIR = INPUT;
    TXDO_DIR = OUTPUT;
}

/*!
    \brief Initialize the Renesas serial port.

    Port direction initialization is handled by init_ports_uart. This function
    sets the baud rate, stop bits, data bits, and parity.

    \param[in] BAUD_RATE The baud rate to set the port to.  This should be on of
      the enumerations from baud_rate_t.
    \param[in] DATA_BITS The number of data bits for each character in the
      serial transfer.  This should be one of the values from data_bits_t.
    \param[in] STOP_BITS The number of stop bits to use.  This should be one of
      the values from stop_bits_t.
    \param[in] PARITY The parity to use when transfering data.  This should be
      one of the values from parity_t.

    \return void
*/
void uart_init(const UInt8 BAUD_RATE, const UInt8 DATA_BITS,
  const UInt8 STOP_BITS, const UInt8 PARITY)
{
    if((DATA_BITS != DATA_BITS_7 && DATA_BITS != DATA_BITS_8)
      || (STOP_BITS != STOP_BITS_1 && STOP_BITS != STOP_BITS_2) 
      || (PARITY != PARITY_NONE && PARITY != PARITY_EVEN && PARITY != PARITY_ODD))
    {
        EXIT();
    } // if any of the parameters are invalid //
    
    UCA0CTL1 |= UCSSEL_2;                                                       // SMCLK
    UCA0CTL1 |= UCSWRST;
    switch (BAUD_RATE)
    {
        case BAUD_9600:
          UCA0BR0 = 0x41;                                                       // 8MHz 9600
          UCA0BR1 = 0x03;                                                       // 8MHz 9600
          UCA0MCTL = UCBRS_2;                                                   // Modulation UCBRSx = 5
          break;
        case BAUD_19200:
          UCA0BR0 = 0xa0;                                                       // 8MHz 19200
          UCA0BR1 = 0x01;                                                       // 8MHz 19200
          UCA0MCTL = UCBRS_6;                                                   // Modulation UCBRSx = 6
          break;
        case BAUD_38400:
          UCA0BR0 = 0xd0;                                                       // 8MHz 38400
          UCA0BR1 = 0x00;                                                       // 8MHz 38400
          UCA0MCTL = UCBRS_3;                                                   // Modulation UCBRSx = 3
          break;
        case BAUD_56000:
          UCA0BR0 = 0x8e;                                                       // 8MHz 56000
          UCA0BR1 = 0x00;                                                       // 8MHz 56000
          UCA0MCTL = UCBRS_7;                                                   // Modulation UCBRSx = 7
          break;
        case BAUD_115200:   
          UCA0BR0 = 0x45;                                                       // 8MHz 115200
          UCA0BR1 = 0x00;                                                       // 8MHz 115200
          UCA0MCTL = UCBRS_4;                                                   // Modulation UCBRSx = 4
          break;
        case BAUD_128000:
          UCA0BR0 = 0x3e;                                                       // 8MHz 128000
          UCA0BR1 = 0x00;                                                       // 8MHz 128000
          UCA0MCTL = UCBRS_4;                                                   // Modulation UCBRSx = 4
          break;
        case BAUD_256000:
          UCA0BR0 = 0x1f;                                                       // 8MHz 256000
          UCA0BR1 = 0x00;                                                       // 8MHz 256000
          UCA0MCTL = UCBRS_2;                                                   // Modulation UCBRSx = 2
          break;
        default:
          EXIT();
    }
    UCA0CTL0 = PARITY | STOP_BITS | DATA_BITS;
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    ENABLE_RX_INTR();
} // uart_init //


/*!
    \brief Returns the number of bytes available in the uart send buffer.

    \param void
    
    \return The number of bytes available in the uart send buffer.
*/
UInt16 uart_tx_bytes_free(void)
{
    return cb_bytes_free(&uart_tx_cb);
} // uart_tx_bytes_free //


/*!
    \brief Returns the number of bytes that have been received and not yet read.
    
    \param void
    
    \return The number of bytes received, waiting to be read.
*/
UInt16 uart_rx_bytes_available(void)
{
    return cb_bytes_queued(&uart_rx_cb);
} // rx_bytes_available //


/*!
    \brief Returns the size of the receive buffer.
    
    \param void
    
    \return The size of the receive buffer
*/
UInt16 uart_rx_buffer_size(void)
{
    return cb_size(&uart_rx_cb);
} // uart_rx_buffer_size //


/*!
    \brief Returns the size of the transmit buffer.
    
    \param void
    
    \return The size of the transmit buffer
*/
UInt16 uart_tx_buffer_size(void)
{
    return cb_size(&uart_tx_cb);
} // uart_tx_buffer_size //


/*!
    \brief Reads data that has been received over the serial port

    \param[in] data Buffer to return data in.
    \param[in] LEN The number of bytes to read (data must be at least this big)

    \return The number of bytes read
*/
UInt16 uart_read(UInt8 * const data, const UInt16 LEN)
{
    UInt16 i;
    for (i = 0; i < LEN; i++) {
        if (!cb_getqueue(&uart_rx_cb, data+i)) {
            break;
        }
    }
    return i;
} // uart_read //


/*!
    \brief Write data out of the serial port

    \param[in] DATA The data to be written.
    \param[in] LEN The number of bytes to be written.

    \return The number of bytes written
*/
//UInt16 uart_write(const char * const DATA, const UInt16 LEN)
UInt16 uart_write(const UInt8 * const DATA, const UInt16 LEN)
{
    UInt16 bytes_written = 0;
    UInt16 i;
    UInt8 temp;
    for (i = 0; i < LEN; i++) {
        temp = DATA[i];
        if (DATA[i]== '\n') { // silently send a carriage return
            temp = cb_putqueue(&uart_tx_cb, '\r');
            if (!temp) { // can't send it; we are done
                break;
            }
        }
        else if (DATA[i]== '\r') { // silently send a newline
            temp = cb_putqueue(&uart_tx_cb, '\n');
            if (!temp) { // can't send it; we are done
                break;
            }
        }
        cb_putqueue(&uart_tx_cb, DATA[i]);
        if (!temp) {
            break;
        }
        ++bytes_written;
    }
    
    if(TX_BUFFER_EMPTY())
    {
        UInt8 byte;

        if(cb_getqueue(&uart_tx_cb, &byte))
        {
            UCA0TXBUF = byte;
            ENABLE_TX_INTR();
        } // if dequeueing the byte was successful //
    } // if the transmit buffer is empty //

    return bytes_written;
} // uart_write //


/*!
    \brief Write a byte in hex format out of the serial port
    
    \param[in] DATA The byte to be written in hex
    
    \return void
*/
void uart_write_int8_hex(const UInt8 DATA)
{
    cb_putqueue(&uart_tx_cb, (HEX_DIGIT[(DATA >> 4) & 0x0F]));
    cb_putqueue(&uart_tx_cb, (HEX_DIGIT[DATA & 0x0F]));
} // uart_write_int8_hex //

//! @} uart_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================


//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup uart_pri_func
//! \ingroup uart
//! @{

/*!
    \brief ISR for transmitting data over the serial port

    This function is called when the TXREG is emptied if TMIE is set.

    \param void

    \return void
*/
#pragma vector=USCIAB0TX_VECTOR
__interrupt void uart_tx_isr( void )
{
#ifndef _SNIFFER_FRONT_END
    UInt8 byte;

    // if there was a TX interrupt and the cb is not empty, get a byte
    // from the cb and put it in the TXREG
    if(cb_bytes_queued(&uart_tx_cb) && cb_getqueue(&uart_tx_cb, &byte) == 1)
    {
        // circular buffer not empty, send the next byte and clear  
        // the "ran dry" flag
        UCA0TXBUF = byte;
        uart_tx_cb.flags &= ~((UInt8)CB_FLAG_RAN_DRY);
    } // if more data to send //
    else
    {
        // circular buffer is empty, set "ran dry" flag
        uart_tx_cb.flags |= CB_FLAG_RAN_DRY;
        
         // make sure uart transmit register is empty
        DISABLE_TX_INTR();
//    	IFG2_bit.UCA0TXIFG = 1;
        while(!TX_BUFFER_EMPTY());
    } // else no more data to send

    // clear interrupt flag
    //IFG2_bit.UCA0TXIFG = 0;
#endif
} // uart_transmit_isr //


/*!
    \brief ISR for receiving data over the serial port

    \param void

    \return void
*/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void uart_rx_isr(void)
{
	UInt8 byte;

	byte = UCA0RXBUF;

    // put this byte in circular buffer
    if(cb_putqueue(&uart_rx_cb, byte) != 1)
    {
        uart_rx_cb.flags |= CB_FLAG_OVERFLOW;
    } // if the receive buffer overflowed //

    // clear interrupt flag
	IFG2_bit.UCA0RXIFG = 0;
} // uart_receive_isr //

//! @} uart_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//============================================================================== 
//! @} uart
