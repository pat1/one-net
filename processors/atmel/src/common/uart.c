//! \addtogroup uart
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
    \file uart.c
	
    \brief Contains functions for accessing the Atxmega256a3b serial port
      and using the cb functions for passing data to and from the ISRs.

    Contains functions for initializing the Atxmega256a3b serial port for interrupt
    driven i/o and interrupt service routines (ISRs) for handling serial data.

	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/


#include "config_options.h"

#ifdef UART

#include "uart.h"
#include "cb.h"
#include "one_net_port_const.h"

#include <avr/interrupt.h>
#include <avr/io.h>

extern BOOL binary_mode;
extern BOOL newline_rcvd;
extern BOOL command_processed;

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
static UInt8 uart_rx_buf[UART_RX_BUF_SIZE];

//! uart transmit buffer
static UInt8 uart_tx_buf[UART_TX_BUF_SIZE];


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
#define TX_BUFFER_EMPTY() (BOOL) ((USARTC0.STATUS & USART_DREIF_bm) == USART_DREIF_bm)


/*!
    \brief Returns TRUE if the uart transmit register is empty

    This is the register from which the data is sent over the line.

    \param void

    \return TRUE if the uart transmit register is empty
            FALSE if the uart transmit register is not empty
*/
#define TX_REG_EMPTY() (BOOL) ((USARTC0.STATUS & USART_DREIF_bm) == USART_DREIF_bm)

//! @} uart_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup uart_pub_func
//! \ingroup uart
//! @{

/*!
    \brief Initialize the serial port.

    Port direction initialization is handled by uart_init_ports. This function
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
       || (STOP_BITS != STOP_BITS_1
      && STOP_BITS != STOP_BITS_2) || (PARITY != PARITY_NONE
      && PARITY != PARITY_EVEN && PARITY != PARITY_ODD))
    {

        // arie todo


        EXIT();
    } // if any of the parameters are invalid //


	// This PORT setting is only valid to USARTC0 if other USARTs is used a
	//  different PORT and/or pins are used.
	// PC3 (TXD0) as output.
	PORTC.DIRSET   = PIN3_bm;
	// PC2 (RXD0) as input.
	PORTC.DIRCLR   = PIN2_bm;

	// Use USARTC0
	// assuming that transmit on falling edge - receive on rising edge
                   	// 8 bits              no parity                 1 stop bit
	USARTC0.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc | 0;

    ENABLE_RX_INTR();

    switch(BAUD_RATE)
	{
	   default:
       case BAUD_115200:
	   {		
		   // bsel = (fPER / 16 * fBR) - 1
		   
		int bsel = 5;                // 115200 baud 8;   could be 5 or 6
		uint8_t bscale = 0;


		USARTC0.BAUDCTRLA = (uint8_t) bsel;
		USARTC0.BAUDCTRLB = (bscale << USART_BSCALE0_bp) | (bsel >> 8);
		break;
	   }
       case BAUD_38400:	   
	   {
		   
		   // the values for 38400
		int bsel = 17;
		uint8_t bscale = 0;

		   USARTC0.BAUDCTRLA = (uint8_t) bsel;
		   USARTC0.BAUDCTRLB = (bscale << USART_BSCALE0_bp) | (bsel >> 8);
		   break;
	   }
	}

    // Enable receiver
    USARTC0.CTRLB |= USART_RXEN_bm;
    
    // Enable transmitter
    USARTC0.CTRLB |= USART_TXEN_bm;

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
UInt16 uart_write(const UInt8 * const DATA, const UInt16 LEN)
{
    UInt16 bytes_written = 0;
    UInt16 i;
    UInt8 byte;
    
    #ifdef BLOCKING_UART
    // add 15 for a little bit of a buffer
    while(cb_bytes_free(&uart_tx_cb) < (LEN + 15))
    {
    }
    #endif
    
    for (i = 0; i < LEN; i++)
    {
        #ifdef UART_CARRIAGE_RETURN_CONVERT
        if (DATA[i]== '\r')
        {
            continue;
        }
        else if (DATA[i]== '\n')
        {
            // silently send a newline
            char* end_of_line = "\r\n";
            if(cb_enqueue(&uart_tx_cb, end_of_line, 2) != 2)
            {
                break;
            }
            bytes_written += 2;
        }
        else
        #endif
        {
            if(cb_enqueue(&uart_tx_cb, &DATA[i], 1) != 1)
            {
                break;
            }
            ++bytes_written;
        }
    }
    
    if(TX_BUFFER_EMPTY())
    {
        if(cb_dequeue(&uart_tx_cb, &byte, 1) == 1)
        {
            USARTC0.DATA = byte;
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


/*!
    \brief Write an array of bytes in hex format out of the serial port

    \param[in] DATA The byte to be written in hex
    \param[in] separate If TRUE, add a space between each byte
    \param[in] len The number of bytes to write

    \return void
*/
void uart_write_int8_hex_array(const UInt8* DATA, BOOL separate, UInt16 len)
{
    UInt8 i;
    for(i = 0; i < len; i++)
    {
        if(separate && i > 0)
        {
            cb_putqueue(&uart_tx_cb, ' ');
        }
        uart_write_int8_hex(DATA[i]);
    }
} // uart_write_int8_hex_array //


#endif //  if UART is enabled //



// Jan. 12, 2012//==============================================================================
// We won't define out the interrupts just because it's easier to keep them
// so we don't get an error in sect30.inc if you only change config_options.h
// and don't want to mess with vector tables.  However, if you really aren't
// using uart, you should comment the two interrupts out and remove uart_tx_isr
// and uart_rx_isr from the interrupt table in sect30.inc.



/*!
    \brief ISR for transmitting data over the serial port

    This function is called when the TXREG is emptied if TMIE is set.

    \param void

    \return void
*/
// uart_tx_isr( void )
ISR(USARTC0_DRE_vect)
{
    #ifdef UART
    UInt8 byte;

    // if there was a TX interrupt and the cb is not empty, get a byte
    // from the cb and put it in the TXREG
    if(cb_bytes_queued(&uart_tx_cb) && cb_getqueue(&uart_tx_cb, &byte) == 1)
    {
        // circular buffer not empty, send the next byte and clear
        // the "ran dry" flag
        USARTC0.DATA = byte;
        uart_tx_cb.flags &= ~((UInt8)CB_FLAG_RAN_DRY);
    } // if more data to send //
    else
    {
        // circular buffer is empty, set "ran dry" flag
        uart_tx_cb.flags |= CB_FLAG_RAN_DRY;

         // make sure uart transmit register is empty
        while(!TX_REG_EMPTY());
        DISABLE_TX_INTR();
    } // else no more data to send
    #endif
} // uart_transmit_isr //


/*!
    \brief ISR for receiving data over the serial port

    \param void

    \return void
*/
// uart_rx_isr(void)
ISR(USARTC0_RXC_vect)
{
    #ifdef UART
	UInt8 byte;
    BOOL place_byte_in_buffer = TRUE;
    
	byte = USARTC0.DATA;
    
    if(command_processed && newline_rcvd)
    {
        cb_clear(&uart_rx_cb);
        // reset
        newline_rcvd = FALSE;
        command_processed = FALSE;
    }
    
    if(!binary_mode)
    {
        if(byte == '\r')
        {
            byte = '\n'; // Minicom seems to send out '\r', not '\n'
        }
        
        if(newline_rcvd)
        {
            // can only handle one newline at a time and we already have one
            // throw away all carriage returns when not in binary mode
            place_byte_in_buffer = FALSE;
        }
        else if(byte == '\n')
        {
            newline_rcvd = TRUE;
        }
        #ifndef HANDLE_BACKSPACE
        else if(byte == '\b' || byte == 127) // delete is 127,  TODO - use named
        {                                    // constant for 127
            place_byte_in_buffer = FALSE;
        }
        #endif 
    }
    

    // put this byte in circular buffer (sizeof(char) is always 1)
    if(place_byte_in_buffer && cb_putqueue(&uart_rx_cb, byte) != 1)
    {
        uart_rx_cb.flags |= CB_FLAG_OVERFLOW;
    } // if the receive buffer overflowed //

    #endif
} // uart_receive_isr //

//! @} uart_pub_func
// PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================


//! @} uart
