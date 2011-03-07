//! \addtogroup oncli
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
    \file oncli.c
    \brief Contains the implementation for the ONE-NET Command Line interface
*/

//#ifdef _R8C_TINY // 2/15/09: moving this module to the low memory area
//    #pragma section program program_high_rom
//#endif // ifdef _R8C_TINY //

#include "config_options.h"

#ifdef _ENABLE_CLI


#include "oncli.h"

#include <string.h>
#include <ctype.h>

#include "nprintf.h"
#include "oncli_hdlr.h"
#include "oncli_str.h"
#include "str.h"
#include "one_net_port_specific.h"
#include "pal.h"
#include "uart.h"
#if defined(_ONE_NET_LOAD) || defined(_ONE_NET_DUMP)
    #include "one_net_crc.h"
	extern const char HEX_DIGIT[];
#endif

#if defined(_ONE_NET_CLIENT)
    #include "one_net_client_port_const.h" // for ONE_NET_MAX_PEER_DEV
#endif

// 1-15-2010 - TO-DO - What about masters with peer assignments which aren't eval boards?
#if defined(_ONE_NET_MASTER) && defined(_ONE_NET_EVAL)
    #include "one_net_eval_hal.h" // for NUM_MASTER_PEER
#endif



//==============================================================================
//								CONSTANTS
//! \defgroup oncli_const
//! \ingroup oncli
//! @{

//! @} oncli_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup oncli_typedefs
//! \ingroup oncli
//! @{

//! @} oncli_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup oncli_pri_var
//! \ingroup oncli
//! @{

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
//! The state the ONCLI is in
static UInt8 state;

//! flag to indicate if echoing or not
static BOOL echo_on = TRUE;
#endif

//! The verbosity mode of the device
static oncli_verbose_t verbosity = ONCLI_QUIET;

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
// The number of bytes in the command string so far.  This variable is so
// the strlen doesn't have to be computed every time.
static UInt16 input_len = 0;
#endif

//! The string being output
static char output[ONCLI_MAX_OUTPUT_STR_LEN];

//! @} oncli_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup oncli_pri_func
//! \ingroup oncli
//! @{

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
static void read_onc(void);

static void print_cmd_result(const char * const CMD,
  const oncli_status_t CMD_RESULT);
  
static void send_invalid_cmd_for_dev_msg(const char * const CMD,
  const char * const DEVICE);

static void echo(const char CH);
#endif

//! @} oncli_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup oncli_pub_func
//! \ingroup oncli
//! @{
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
oncli_status_t oncli_set_verbosity(const UInt8 VERBOSITY)
{
    if(VERBOSITY != ONCLI_QUIET && VERBOSITY != ONCLI_VERBOSE)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameters are not valid //

    if(!oncli_is_master() && !oncli_is_client())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER and not a CLIENT //
    
    verbosity = VERBOSITY;
    return ONCLI_SUCCESS;
} // oncli_set_verbosity //


/*!
    \brief Returns TRUE if there is user input.
    
    \param void
    
    \return TRUE if there is user input
            FALSE if there is no user input
*/
BOOL oncli_user_input(void)
{
    return (BOOL)input_len;
} // oncli_user_input //


/*!
    \brief Turns echoing on or off
    
    \param[in] echo_on TRUE to turn echoing on.  FALSE to turn echoing off
    
    \return void
*/
void oncli_set_echo(const BOOL ECHO)
{
    echo_on = ECHO;
} // oncli_set_echo //
#endif

/*!
    \brief Returns a pointer to the string representation of the one_net_status
      value.

    \param[in] STATUS The status to return the string for.
    
    \return The string representation of the status.  If the status is invalid,
      the internal error string is returned.
*/
const char * oncli_status_str(one_net_status_t STATUS)
{
    if(STATUS > ONS_WRITE_ERR)
    {
        return ONCLI_ONS_STR[ONS_INTERNAL_ERR];
    } // if the status is invalid //
    
    return ONCLI_ONS_STR[STATUS];
} // oncli_status_str //


/*!
    \brief Outputs the received admin message.
    
    \param[in] MSG_TYPE The type of message being sent.  Valid values are
      ON_ADMIN_MSG and ON_EXTENDED_ADMIN_MSG.
    \param[in] TXN_TYPE The type of transaction that was received.  Valid values 
      are ON_SINGLE and ON_BLOCK.
    \param[in] ADMIN_MSG_TYPE The admin message that was received.
    \param[in] ADMIN_MSG_DATA The admin message data that was received.
    \param[in] LEN The number of bytes in the admin message.
    
    \return void
*/
void oncli_print_admin_msg(const UInt8 MSG_TYPE, const UInt8 TXN_TYPE,
  const UInt8 ADMIN_MSG_TYPE, const UInt8 * ADMIN_MSG_DATA, const UInt16 LEN)
{
    const char * MSG_TYPE_STR = 0;
    const char * TXN_TYPE_STR = 0;
    const char * ADMIN_TYPE_STR = 0;

    // ASSIGN Multi-Hop peer is currently the last admin packet (spec ver 1.2).
    if(verbosity != ONCLI_VERBOSE || !ADMIN_MSG_DATA)
    {
        return;
    } // if not verbose mode //
    
    // get the message type string
    if(MSG_TYPE == ON_ADMIN_MSG)
    {
        MSG_TYPE_STR = ONCLI_ADMIN_MSG_STR;
    } // if an admin message //
    else if(MSG_TYPE == ON_EXTENDED_ADMIN_MSG)
    {
        MSG_TYPE_STR = ONCLI_EXTENDED_ADMIN_MSG_STR;
    } // else if an extended admin message //
    else
    {
        return;
    } // else invalid message type //
    
    // get the transaction type string and the admin type string
    if(TXN_TYPE == ON_SINGLE)
    {
        if(ADMIN_MSG_TYPE > ONCLI_ADMIN_STR_COUNT)
        {
            return;
        } // if admin message type is out of bounds //

        TXN_TYPE_STR = ONCLI_SINGLE_STR;
        
        // get the string for the admin message that was sent.  A single
        // extended admin message uses the same values as an admin message.
        
        ADMIN_TYPE_STR = ONCLI_ADMIN_STR[ADMIN_MSG_TYPE];
    } // if a single transaction //
    else if(TXN_TYPE == ON_BLOCK)
    {
        if(ADMIN_MSG_TYPE > ONCLI_EXTENDED_ADMIN_STR_COUNT)
        {
            return;
        } // if extended admin message type is out of bounds //

        TXN_TYPE_STR = ONCLI_BLOCK_STR;
        
        // get the string for the extended admin message that was sent
        ADMIN_TYPE_STR = ONCLI_EXTENDED_ADMIN_STR[ADMIN_MSG_TYPE];
    } // else if a block transaction //
    else
    {
        return;
    } // invalid transaction type //

    oncli_send_msg(ONCLI_RX_ADMIN_FMT, TXN_TYPE_STR, MSG_TYPE_STR,
      ADMIN_TYPE_STR);
    
    // don't display the block admin message since the only one is the change
    // stream key message, and we don't want to display the key.
    if(TXN_TYPE == ON_SINGLE && ADMIN_MSG_TYPE != ON_NEW_KEY_FRAGMENT)
    {
        UInt8 i;

        oncli_send_msg("\t");   		
		// not sure what exactly this is printing, but since we now have a print function
		// for xtea keys, use it.
		oncli_print_xtea_key(&((one_net_xtea_key_t) ADMIN_MSG_DATA));
		oncli_send_msg("\n");
    } // if not one of the change key messages //
    
    oncli_print_prompt();
} // oncli_print_admin_msg //


/*!
    \brief Reads up to SIZE bytes from the ONE-NET command line interface
    
    \param[out] buf Pointer to location to receive the data from the ONE-NET
      command line interface.
    \param[in] SIZE The maximum number of bytes to read.
    
    \return The number of bytes read
*/
UInt16 oncli_read(UInt8 * buf, const UInt16 SIZE)
{
    return uart_read(buf, SIZE);
} // oncli_read //


/*!
    \brief Sends a message out of the serial port.
    
    The std library provided by Renesas does not provide output functions that
    

    \param[in] FMT Format of the string to send.
    \param[in] ... Optional parameters to put use in the final string based on
      FMT.

    \return void
*/
void oncli_send_msg(const char * const FMT, ...)
{
#ifdef _ONE_NET_DEBUG_STACK 
    UInt8 tmp;
#endif

    va_list ap;
    int output_len;

    if(!FMT)
    {
        return;
    } // if the parameter is invalid //

    va_start(ap, FMT);    
    if((output_len = vsnprintf(output, sizeof(output), FMT, ap))
      > sizeof(output))
    {
        if((output_len = snprintf(output, sizeof(output),
          ONCLI_OUTPUT_STR_TOO_SHORT_FMT, (UInt16)sizeof(output), output_len))
          > sizeof(output))
        {
            uart_write(ONCLI_FATAL_ERR_1_STR,ONCLI_FATAL_ERR_1_LEN);
            while(uart_tx_bytes_free() < uart_tx_buffer_size());
            return;
            // EXIT();
        } // if the output string is still to short //
    } // if the output string is too short //

    va_end(ap);
    #if 0   // for debugging the garbled output on receiving a message bug TODO: RWM: remove after debugging
        if ((FMT == ONCLI_RX_DATA_FMT) || (FMT == ONCLI_RX_TXT_FMT))
        {
            UInt8 i, high_nibble, low_nibble;
#ifdef _ONE_NET_DEBUG_STACK 
            uart_write("\nIn oncli_send, stack is ", 25);
            uart_write_int8_hex( (((UInt16)(&tmp))>>8) & 0xff );
            uart_write_int8_hex( ((UInt16)(&tmp)) & 0xff );
            uart_write("\n", 1);
#endif

            uart_write("\nRMDBG: ", 8);
            for (i=0; i<output_len; i++)
            {
                uart_write_int8_hex(output[i]);
                uart_write("-", 1);
            }
            uart_write("\n", 1);
        }
        else
        {
            uart_write(output, output_len);
        }
    #else
        #ifdef _ONE_NET_DEBUG_STACK 
            if ((FMT == ONCLI_RX_DATA_FMT) || (FMT == ONCLI_RX_TXT_FMT))
            {
                uart_write("\nIn oncli_send, stack is ", 25);
                uart_write_int8_hex( (((UInt16)(&tmp))>>8) & 0xff );
                uart_write_int8_hex( ((UInt16)(&tmp)) & 0xff );
                uart_write("\n", 1);
            }
        #endif
        uart_write(output, output_len);
    #endif
} // oncli_send_msg //


/*!
    \brief Prints an xtea key.
    
    
    \param[in] KEY Pointer to xtea key to print
    
    \return void
*/
void oncli_print_xtea_key(const one_net_xtea_key_t* KEY)
{
    UInt8 i;

    for(i = 0; i < ONE_NET_XTEA_KEY_LEN / 4; i++)
    {
		if(i != 0)
		{
			oncli_send_msg(" - ");
		}
		oncli_send_msg("(%02x-%02x-%02x-%02x)", 
		    (*KEY)[i*4], (*KEY)[i*4+1], (*KEY)[i*4+2], (*KEY)[i*4+3]);
    }
} // oncli_print_xtea_key //


#if defined(_NEED_XDUMP) || defined(_ENABLE_DUMP_COMMAND)
void xdump(UInt8 *pt, UInt16 len)
{
    int i;
    delay_ms(100);
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            oncli_send_msg("\n%p: ", pt+i);
        }
        oncli_send_msg("%02x ", pt[i]);
        delay_ms(100);
    }
    oncli_send_msg("\n");
    delay_ms(100);
}
#endif


/*!
    \brief Function to dump volatile memory to UART
        
    \param ptr pointer to start of memory to be dumped
	\param length number of bytes to dump
    
    \return true if successful, false otherwise
*/
#ifdef _ONE_NET_DUMP
    BOOL dump_volatile_memory(UInt8* ptr, const UInt16 length)
	{
		// note : newline signifies a break/"your turn to send"
		// note : Reading from uart seems to change '\n' to '\r' and/or
		// vice-versa.  I think a newline is considered 1 character, not 2,
		// even with Windows.  But we'll call either '\r' or '\n' a newline
		UInt16 i, j, chunkSize;
		UInt8 response, crc, high_nibble, low_nibble;
		UInt8* temp_ptr;
		UInt16 numChunks;
		UInt8 bytesRead;
		BOOL success, abort;
		const UInt8 ACK = '0';  // All is OK
		const UInt8 NACK_RESEND = '1'; // Problem.  Try again.
		const UInt8 NACK_ABORT = '2'; // Problem.  Unrecoverable.  Abort
		const UInt8 MAX_CHUNK_SIZE = 20;
		UInt8 buffer[8];

		// we want un-interrupted communication, so set idle and prevent anyone
		// from changing that.
        if(!set_on_state(ON_IDLE))
		{
			return FALSE;
		}
		
		set_allow_set_state(FALSE);

		numChunks = length / MAX_CHUNK_SIZE;
		if(length % MAX_CHUNK_SIZE != 0)
		{
			numChunks++;
		}
		
		// first send the start of transmission/number of bytes/number of chunks
		success = FALSE;
		abort = FALSE;
		while(!success && !abort)
		{
			delay_ms(50);
			buffer[0] = (UInt8)(length >> 8);
			buffer[1] = (UInt8)(length & 0x00FF);
			buffer[2] = (UInt8)(numChunks >> 8);
			buffer[3] = (UInt8)(numChunks & 0x00FF);
			buffer[4] = one_net_compute_crc(buffer, 4, ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
			oncli_send_msg("TRANS_START:");
			for(i = 0; i < 5; i++)
			{
				if(i == 4)
				{
					oncli_send_msg(":");
				}
                high_nibble = buffer[i] >> 4;
                low_nibble  = buffer[i] & 0x0F;
				oncli_send_msg("%c%c", HEX_DIGIT[high_nibble], HEX_DIGIT[low_nibble]);
			}
			oncli_send_msg("\n");

			bytesRead = 0;
			do
			{
			    bytesRead += oncli_read(&buffer[bytesRead], 1);
			}
			while(bytesRead < 2);
			
			if(buffer[1] == '\r' || buffer[1] == '\n')
			{
				// we got the break.
				if(buffer[0] == ACK)
				{
					success = TRUE;
				}
				else if(buffer[0] == NACK_ABORT)
				{
					// other end has given up.
					abort = TRUE;
				}
			}
		}
		
		for(i = 0; !abort && i < numChunks; i++)
		{
			delay_ms(50);
			temp_ptr = ptr + (i * MAX_CHUNK_SIZE);
			
			chunkSize = MAX_CHUNK_SIZE;
			if(i == numChunks - 1)
			{
				chunkSize = length - (i * MAX_CHUNK_SIZE);
			}
			
            crc = one_net_compute_crc(temp_ptr, chunkSize, ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
			
			buffer[0] = HEX_DIGIT[(i & 0xF000) >> 12];
			buffer[1] = HEX_DIGIT[(i & 0x0F00) >> 8];
			buffer[2] = HEX_DIGIT[(i & 0x00F0) >> 4];
			buffer[3] = HEX_DIGIT[(i & 0x000F)];
			buffer[4] = HEX_DIGIT[(chunkSize & 0xF000) >> 12];
			buffer[5] = HEX_DIGIT[(chunkSize & 0x0F00) >> 8];
			buffer[6] = HEX_DIGIT[(chunkSize & 0x00F0) >> 4];
			buffer[7] = HEX_DIGIT[(chunkSize & 0x000F)];
			oncli_send_msg("CHUNK_START:");
			for(j = 0; j < 8; j++)
			{
			    oncli_send_msg("%c", buffer[j]);
			}
			oncli_send_msg(":");
			for(j = 0; j < chunkSize; j++)
			{
				high_nibble = (*(temp_ptr + j)) >> 4;
				low_nibble  = (*(temp_ptr + j)) & 0x0F;
				oncli_send_msg("%c%c", HEX_DIGIT[high_nibble], HEX_DIGIT[low_nibble]);				
			}
			
			// send the crc and the break
			high_nibble = crc >> 4;
			low_nibble  = crc & 0x0F;
			oncli_send_msg(":%c%c\n", HEX_DIGIT[high_nibble], HEX_DIGIT[low_nibble]);
			
			// now listen for the ACK or NACK
			bytesRead = 0;
			do
			{
			    bytesRead += oncli_read(&buffer[bytesRead], 1);
			}
			while(bytesRead < 2);

		    if(buffer[1] == '\r' || buffer[1] == '\n')
			{
				// we got the break.
				if(buffer[0] == ACK)
				{
					success = TRUE; 
				}
				else if(buffer[0] == NACK_ABORT)
				{
					// other end has given up.
					abort = TRUE;
				}
				else
				{
					// NACK - try again
					i--;
				}
			}
			else
			{
				// no newline break.  Try again.
				i--;
			}			
		}
		
		// we're done.  Get out of idle mode and reset the flag
		set_allow_set_state(TRUE);
		set_on_state(ON_LISTEN_FOR_DATA);
		
        return !abort;
	}
#endif


/*!
    \brief Function to load data from UART into volatile memory
        
    \param ptr pointer to start of memory to be loaded
    
    \return true if successful, false otherwise
*/
#ifdef _ONE_NET_LOAD
    BOOL load_volatile_memory(UInt8* ptr)
	{
        // note : this function is basically the reverse of the dump_volatile_memory function
		// note : newline signifies a break/"your turn to send"
		// note : Reading from uart seems to change '\n' to '\r' and/or
		// vice-versa.  I think a newline is considered 1 character, not 2,
		// even with Windows.  But we'll call either '\r' or '\n' a newline
		
		// This function and the last can be consolidated in many ways, plus we can shrink things
		// down.  We aren't taking full advantage of some helper functions.  Is there a getline function?
		int i, j;
		UInt8 response, crc, chunkSize, expectedChunkSize, thisChunkSize, high_nibble, low_nibble, crcPos;
		UInt8* temp_ptr;
		UInt16 length, numChunks, chunkNumber, thisChunkNumber;
		UInt8 bytesRead;
		BOOL success, abort, error;
		const UInt8 ACK = '0';  // All is OK
		const UInt8 NACK_RESEND = '1'; // Problem.  Try again.
		const UInt8 NACK_ABORT = '2'; // Problem.  Unrecoverable.  Abort
		const UInt8 MIN_CHUNK_SIZE = 1;
		const UInt8 MAX_CHUNK_SIZE = 20;
		UInt8 ascii_buffer[2 * MAX_CHUNK_SIZE + 30];  // 30 is just a number more than big enough to 
	                                            // contain all the "extra" characters.
		UInt8 buffer[MAX_CHUNK_SIZE + 30] ;// 30 is just a number more than big enough to 
	                                            // contain all the "extra" bytes.
		

		// we want un-interrupted communication, so set idle and prevent anyone
		// from changing that.
        if(!set_on_state(ON_IDLE))
		{
			return FALSE;
		}
		
		set_allow_set_state(FALSE);
		
		// First line should be "TRANS_START:", then 8 hex digits, then ":", then
		// 2 hex digits, then a '\r' or '\n', 24 characters total.
		success = FALSE;
		abort = FALSE;
		numChunks = 0;
		length = 0;
		
		while(!success && !abort)
		{
			delay_ms(40);
		    bytesRead = 0;
		    ascii_buffer[bytesRead] = 0;
		    abort = FALSE;
		    success = FALSE;
		    error = FALSE;

	    	while(bytesRead == 0 || (ascii_buffer[bytesRead-1] != '\r' && ascii_buffer[bytesRead-1] != '\n'))
		    {
			    bytesRead += oncli_read(&ascii_buffer[bytesRead], 1);
				
		    	if(bytesRead > 24)
		    	{
			    	// too long
					error = TRUE;
					bytesRead = 1; // read things in and throw them away.  We're in error condition.
			    }
		    }
			
			if(error || bytesRead != 24 || strnicmp("TRANS_START:", ascii_buffer, 12) ||
			    ascii_buffer[20] != ':')
			{
				error = TRUE;
			}
			if(error || (ascii_hex_to_byte_stream(&ascii_buffer[12], buffer, 8) != 8))
			{
				error = TRUE;
			}
			if(error || (ascii_hex_to_byte_stream(&ascii_buffer[21], &buffer[4], 2) != 2))
			{
				error = TRUE;
			}
			if(!error)
			{
				// calculate crc
				crc = one_net_compute_crc(buffer, 4, ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
				if(crc != buffer[4])
				{
					// crc's don't match
					error = TRUE;
				}
			}
			
			if(error)
			{
				oncli_send_msg("%c\n", NACK_RESEND);
			}
			else
			{
			    length = one_net_byte_stream_to_int16(&buffer[0]);
			    numChunks = one_net_byte_stream_to_int16(&buffer[2]);
				if(numChunks <= 0)
				{
					abort = TRUE;
					oncli_send_msg("%c\n", NACK_ABORT);
				}
				else
				{
					success = TRUE;
					oncli_send_msg("%c\n", ACK);
				}
			}
		}


        // we want to read in a chunk at a time, test it, and send the ACK, NACK, or ABORT.
		// A "chunk" representing 15 bytes would look like this:
		// CHUNK_START:0002000F:0A0B0C0D0E0F407D00000019000000:B0
		// It starts with "CHUNK_START:", then 8 hex digits.  The first 4 represent the chunk
		// number in hex (in this case, the chunk number is 0x0002, or 2.  The next four bits is
		// the chunk size in bytes (in this case, the chunk number is 0x0002, or 15).
		// Then there are 30 hexadecimal digits, representing 15 bytes (matching the chunk size of
		// 15).  These 15 bytes will then have a CRC calculated over them.
		// Then there is a colon, then there are 2 hexadecimal digits, which represent the CRC.  This
		// should match the calculated CRC calculated over the 15 bytes.
		//
		// The total size of the line, including the newline/carriage return, should be the 25 +
		// the chunk size times 2, so for a chunk size of 15, that would be (25 + (15 * 2)) or 55.
		chunkNumber = 0;
		chunkSize = 0;
		thisChunkSize = 0;
		success = FALSE;
		while(!success && !abort)
		{
		    bytesRead = 0;
		    ascii_buffer[bytesRead] = 0;
		    error = FALSE;

	    	while(bytesRead == 0 || (ascii_buffer[bytesRead-1] != '\r' && ascii_buffer[bytesRead-1] != '\n'))
		    {
			    bytesRead += oncli_read(&ascii_buffer[bytesRead], 1);			
		    	if(bytesRead > (25 + MAX_CHUNK_SIZE * 2))
		    	{
			    	// too long
					error = TRUE;
					bytesRead = 1; // read things in and throw them away.  We're in error condition.
			    }
		    }
			
			if(error)
			{
				goto memload_send_chunk_reply;
			}
			
			// set error to true.  If any tests fail, skip the rest with a goto command.  If all tests
			// pass, we'll set error to false at the end.
			error = TRUE;
			
			if(bytesRead < (25 + MIN_CHUNK_SIZE * 2) || strnicmp("CHUNK_START:", ascii_buffer, 12) ||
			    ascii_buffer[20] != ':')
			{
				goto memload_send_chunk_reply;
			}
		
			if(ascii_hex_to_byte_stream(&ascii_buffer[12], buffer, 8) != 8)
			{
				goto memload_send_chunk_reply;
			}

			thisChunkNumber = one_net_byte_stream_to_int16(&buffer[0]);
			thisChunkSize = one_net_byte_stream_to_int16(&buffer[2]);
		    if(thisChunkNumber != chunkNumber)
			{
				// expecting a different chunk number
				goto memload_send_chunk_reply;
			}
				
			if(chunkNumber == 0)
			{
				chunkSize = thisChunkSize;
				if(chunkSize > MAX_CHUNK_SIZE)
				{
					abort = TRUE;
					goto memload_send_chunk_reply;
				}
			}
				
			expectedChunkSize = chunkSize;
			if(chunkNumber == numChunks - 1)
			{
				expectedChunkSize = length - (chunkNumber * chunkSize);
				if(expectedChunkSize > chunkSize)
				{
					// abort.  Can't have last chunk size bigger than the other chunk sizes.
					abort = TRUE;
					goto memload_send_chunk_reply;
				}
			}
				
			if(thisChunkSize != expectedChunkSize)
			{
				// expecting a different chunk size
				goto memload_send_chunk_reply;
			}
				
			if(bytesRead != (25 + thisChunkSize * 2))
			{
				// length is wrong.
				goto memload_send_chunk_reply;
			}
				
			crcPos = 22 + 2 * thisChunkSize;
			if(ascii_buffer[crcPos - 1] != ':' ||
			    (ascii_buffer[crcPos + 2] != '\r' && ascii_buffer[crcPos + 2] != '\n'))
			{
				goto memload_send_chunk_reply;
			}
				
			if((ascii_hex_to_byte_stream(&ascii_buffer[21], buffer, 2 * thisChunkSize) != 2 * thisChunkSize)
			    || (ascii_hex_to_byte_stream(&ascii_buffer[crcPos], &buffer[thisChunkSize], 2) != 2))
			{
				goto memload_send_chunk_reply;
			}
				
			// calculate crc
		    crc = one_net_compute_crc(buffer, thisChunkSize, ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
			if(crc != buffer[thisChunkSize])
			{
			    // crc's don't match
			    goto memload_send_chunk_reply;
		    }
		 
            // things are good.  Set error to false.
			error = FALSE;
			
memload_send_chunk_reply:		
			delay_ms(50);
			if(abort)
			{
				oncli_send_msg("%c\n", NACK_ABORT);
			}
			else if(error)
			{
				oncli_send_msg("%c\n", NACK_RESEND);
			}
			else
			{
                // everything is good for this chunk.  Copy it to memory
				temp_ptr = ptr + (chunkNumber * chunkSize);
				one_net_memmove(temp_ptr, buffer, thisChunkSize);
				chunkNumber++;
				if(chunkNumber >=  numChunks)
				{
					success = TRUE;
				}
				oncli_send_msg("%c\n", ACK);
			}
		}
		
		// we're done.  Get out of idle mode and reset the flag
		set_allow_set_state(TRUE);
		set_on_state(ON_LISTEN_FOR_DATA);
		
        return !abort;
	}
#endif


/*!
    \brief Main function for process handling ONE-NET Command Line Interface.
    
    This function should be called regularly from the programs main loop.
    
    \param void
    
    \return void
*/
void oncli()
{
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
    read_onc();
#endif
} // oncli //


/*!
    \brief Converts a string of ASCCI hex digits to a byte stream.
    
    \param[in] STR The ASCII string of hex digits.
    \param[out] byte_stream The byte stream that results from STR
    \param[in] NUM_ASCII_CHAR The number of ascii characters to convert.  This
      is really twice the number of bytes that were converted.
    
    \return The number of ASCII characters that were converted.
*/
UInt16 ascii_hex_to_byte_stream(const char * STR, UInt8 * byte_stream,
  const UInt16 NUM_ASCII_CHAR)
{
    UInt16 num_converted;
    
    UInt8 hex;

    if(!STR || !byte_stream || !NUM_ASCII_CHAR)
    {
        return 0;
    } // if any of the parameters are invalid //

    for(num_converted = 0; num_converted < NUM_ASCII_CHAR; num_converted++)
    {
        hex = ascii_hex_to_nibble(STR[num_converted]);
        if(hex > 0x0F)
        {
            break;
        } // if the conversion failed //

        if(num_converted & 0x01)
        {
            byte_stream[num_converted >> 1] |= hex;
        } // if the second nibble in the byte //
        else
        {
            byte_stream[num_converted >> 1] = hex << 4;
        } // else the first nibble in the byte //
    } // loop to convert payload from ascii //
    
    return num_converted;
} // ascii_hex_to_byte_stream //


/*!
    \brief Checks if a given character is a valid ONE-NET unique key character.
    
    Valid unique key for adding devices characters are '2' - '9', and 'A' - 'Z'
    except for 'O' & 'L'.  The key is case sensitive.
*/
BOOL oncli_is_valid_unique_key_ch(const char CH)
{
    return (BOOL)(isalnum(CH) && CH >= '2'
      && ((CH | 0x20) != 'o' && (CH | 0x20) != 'l'));
} // oncli_is_valid_unique_key_ch //


//! @} oncli_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup oncli_pri_func
//! \ingroup oncli
//! @{

/*!
    \brief Reads in input from the UART.

    Reads in a ONE-NET command (or parameters for the command).  Input is read
    in one character at a time to make searching for a terminating character
    (which depends on the state).

    Note that '\r' input chars are always translated to '\n', so there will
    never, ever be a '\r' in any input buffer from this function

    \param void

    \return void
*/
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
static void read_onc(void)
{
    // Pointer to the function that handles the command whose parameters are
    // being received.
    static oncli_cmd_hdlr_t cmd_hdlr = 0;

    // The command that was parsed
    static const char * CMD_STR = 0;
    
    // The number of quotes that have been received, if quotes are being
    // accepted (ie one of the two quote states).
    static UInt16 quote_count = 0;

    // The command being read in
    static char input[ONCLI_MAX_INPUT_STR_LEN];

    while(input_len < sizeof(input) - 1 && oncli_read(&(input[input_len]), 1))
    {
        if (input[input_len] == '\b') { // special case for backspace
            if (input_len > 0) { // Ignore backspace at beginning of line
                oncli_send_msg("\b \b"); // write over previous char
                --input_len;
            }
        }
        else { // Everything else for non-backspace
            if (input[input_len] == '\r') { // Convert carriage ret to newline
                input[input_len] = '\n';
            }
            echo(input[input_len]);

            // check to see if it was a terminating character
            switch(state)
            {
                case ONCLI_RX_CMD_STATE:
                {
                    if(input[input_len] == ':' || input[input_len] == '\n')
                    {
                        oncli_status_t status;
                        UInt8 next_state = ONCLI_RX_CMD_STATE;

                        input_len++;

                        // make sure input is NULL terminated
                        input[input_len] = '\0';

#ifdef _ENABLE_GET_CHANNEL_COMMAND
                        if((status = oncli_parse_cmd(input, &CMD_STR, &next_state,
                          &cmd_hdlr)) != ONCLI_SUCCESS || (!cmd_hdlr
                          && CMD_STR != ONCLI_GET_CHANNEL_CMD_STR))
#else
                        if((status = oncli_parse_cmd(input, &CMD_STR, &next_state,
                          &cmd_hdlr)) != ONCLI_SUCCESS || (!cmd_hdlr))
#endif
                        {
                            // remove the command terminator ('\n', or ':')
                            // since if the command was invalid, we don't want
                            // this printed as part of the command.
                            input_len--;
                            input[input_len] = '\0';
                            print_cmd_result(CMD_STR, status);
                            CMD_STR = 0;
                            oncli_print_prompt();
                        } // if the command failed //
                        else if(next_state == ONCLI_RX_PARAM_NEW_LINE_STATE
                          || next_state == ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE)
                        {
                            state = next_state;
                        } // if the state changed //

                        // clear the command to read in the parameters or the
                        // next command
                        quote_count = 0;
                        input_len = 0;
                        input[0] = '\0';
                        return;
                    } // if a command terminator was read in //
                    break;
                } // ONCI_RX_CMD_STATE case //
            
                case ONCLI_RX_PARAM_NEW_LINE_STATE:
                case ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE:
                {
                    if(input[input_len] == '\n')
                    {
                        input_len++;

                        // make sure input is NULL terminated
                        input[input_len] = '\0';

                        print_cmd_result(CMD_STR, (*cmd_hdlr)(input));
                        oncli_print_prompt();

                        quote_count = 0;
                        input_len = 0;
                        state = ONCLI_RX_CMD_STATE;
                        return;
                    } // if the parameter terminating character was read //
                    else if(state == ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE
                      && input[input_len] == '"')
                    {
                        quote_count++;
                        state = ONCLI_RX_PARAM_QUOTE_STATE;
                    } // if new line or quote state and a quote was received //
                    break;
                } // ONCLI_RX_PARAM_NEW_LINE_STATE case //
                
                case ONCLI_RX_PARAM_QUOTE_STATE:
                {
                    if(input[input_len] == '"')
                    {
                        quote_count++;
                        state = ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE;
                    } // if the ending quote was received //
                    break;
                } // ONCLI_RX_PARAM_QUOTE_STATE case //

                default:
                {
                    oncli_send_msg(ONCLI_INTERNAL_ERR_FMT, &read_onc);
                    state = ONCLI_RX_CMD_STATE;
                    return;
                    break;
                } // default case //
            } // switch(state) //

            // if it was the backspace, remove the previous character
            if(input[input_len] == '\b' || input[input_len] == 0x7F)
            {
                if(input_len)
                {
                    input_len--;

                    // The quote count is only active if quotes are accepted.
                    // If there has been a quote, see if the deleted character
                    // is a quote, thus needing a state change.
                    if(quote_count && input[input_len] == '"')
                    {
                        quote_count--;
                        
                        // change the state
                        if(quote_count & 0x0001)
                        {
                            // Need to look for the trailing quote
                            state = ONCLI_RX_PARAM_QUOTE_STATE;
                        } // if odd number of quotes //
                        else
                        {
                            // look for new line or leading quote
                            state = ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE;
                        } // else even number of quotes //
                    } // if a quote was removed //
                } // if there is a character to remove //
            } // if the backspace or delete character was received //
            else if(!isprint(input[input_len]) && !isspace(input[input_len]))
            {
                oncli_send_msg(ONCLI_RX_INVALID_CH_FMT, input[input_len]);
                oncli_send_msg(ONCLI_CLR_INPUT_STR);
                oncli_print_prompt();
                quote_count = 0;
                input_len = 0;
                state = ONCLI_RX_CMD_STATE;
            } // else if the ch is not in the valid character set //
            else if(++input_len >= sizeof(input) - 1)
            {
                oncli_send_msg(ONCLI_INVALID_CMD_LEN_STR);
                oncli_send_msg(ONCLI_CLR_INPUT_STR);
                oncli_print_prompt();
                quote_count = 0;
                input_len = 0;
                state = ONCLI_RX_CMD_STATE;
            } // else if the command entered is too long //
        } // end of else for non-back-space chars
    } // loop to read command string bytes //
} // read_onc //
#endif


/*!
    \brief Prints the result of the command
    
    \param[in] CMD The command string that was executed.
    \param[in] CMD_RESULT The result of CMD.
    
    \return void
*/
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
static void print_cmd_result(const char * const CMD,
  const oncli_status_t CMD_RESULT)
{
    if(!CMD)
    {
        return;
    } // if the parameter is invalid //
    
    switch(CMD_RESULT)
    {
        case ONCLI_SUCCESS:
        {
            oncli_send_msg(ONCLI_CMD_SUCCESS_STR);
            break;
        } // SUCCESS case //
        
        case ONCLI_ALREADY_IN_PROGRESS:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_IN_PROGRESS_STR);
            break;
        } // ONCLI_ALREADY_IN_PROGRESS case //
        
        case ONCLI_RSRC_UNAVAILABLE:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_RSRC_UNAVAILABLE_STR);
            break;
        } // resource unavailable case //
        
        case ONCLI_UNSUPPORTED:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_UNSUPPORTED_STR);
            break;
        } // if the request is not supported //

        case ONCLI_INTERNAL_ERR:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_INTERNAL_ERR_STR);
            break;
        } // internal error case //

        case ONCLI_INVALID_CMD:
        {    
            oncli_send_msg(ONCLI_INVALID_CMD_FMT, CMD);
            break;
        } // invalid command case //
        
        case ONCLI_CMD_FAIL:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_FAILED_STR);
            break;
        } // if an invalid command //
        
        case ONCLI_INVALID_DST:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_INVALID_DST_STR);
            break;
        } // invalid destination case //
        
        case ONCLI_NOT_JOINED:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_NEED_TO_JOIN_STR);
            break;
        } // device needs to join a network first //

#ifdef _AUTO_MODE
        case ONCLI_INVALID_CMD_FOR_MODE:
        {
            const char * MODE_TYPE_STR;
            
            if(!(MODE_TYPE_STR = oncli_mode_type_str()))
            {
                oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_INTERNAL_ERR_STR);
            } // if getting the node type string failed //
            else
            {
                send_invalid_cmd_for_dev_msg(CMD, MODE_TYPE_STR);
            } // else the mode type string was successfully retrieved //
            break;
        } // invalid command for mode case //
#endif
        case ONCLI_INVALID_CMD_FOR_NODE:

        {
            const char * NODE_TYPE_STR;

            if(!(NODE_TYPE_STR = oncli_node_type_str()))
            {
                oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_INTERNAL_ERR_STR);
            } // if getting the node type string failed //
            else
            {
                send_invalid_cmd_for_dev_msg(CMD, NODE_TYPE_STR);
            } // else the node type string was successfully retrieved //
            break;
        } // invalid command for node case //

        case ONCLI_BAD_PARAM:       // fall through
        case ONCLI_PARSE_ERR:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_INVALID_FORMAT_STR);
            break;
        } // parse error case //

        case ONCLI_SNGH_INTERNAL_ERR:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_SNGH_STR);
            break;
        } // parse error case //

        case ONCLI_ONS_NOT_INIT_ERR:
        {
            oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, ONCLI_ONS_NOT_INIT_ERR_STR);
            break;
        } // parse error case //

        default:
        {
            break;
        } // default case //
    } // switch(CMD_RESULT) //
} // print_cmd_result //
#endif


/*!
    \brief Sends the invalid command for device message.
    
    \param[in] CMD The command that is invalid.
    \param[in] DEVICE The type of device that does not handle CMD.
    
    \return void
*/
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
static void send_invalid_cmd_for_dev_msg(const char * const CMD,
  const char * const DEVICE)
{
    char msg[ONCLI_ERR_MSG_SIZE];

    if(!CMD || !DEVICE)
    {
        oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
          (UInt32)&send_invalid_cmd_for_dev_msg);
        return;
    } // if the parameters are invalid //
    
    snprintf(msg, sizeof(msg), ONCLI_INVALID_CMD_FOR_DEVICE_FMT, DEVICE);
    oncli_send_msg(ONCLI_CMD_FAIL_FMT, CMD, msg);
} // send_invalid_cmd_for_dev_msg //
#endif


/*!
    \brief Echoes received characters.
    
    \param[in] CH The character to echo.
    
    \return void
*/
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
static void echo(const char CH)
{
    if(!echo_on)
    {
        return;
    } // if echoing is not enabled //

    oncli_send_msg("%c", CH);
    
} // echo //
#endif

//! @} oncli_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


#endif // #ifdef _ENABLE_CLI
//! @} oncli

