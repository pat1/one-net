//! \addtogroup oncli
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
    \file oncli.c
    \brief Contains the implementation for the ONE-NET Command Line interface
*/


#include "config_options.h"


#include "oncli.h"
#include "oncli_str.h"
#include "one_net_types.h"
#include "oncli_port_const.h"
#include "oncli_port.h"
#include "nprintf.h"
#include "uart.h"
#include "oncli_hdlr.h"
#include "ctype.h"



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
//							PUBLIC VARIABLES
//! \defgroup oncli_pub_var
//! \ingroup oncli
//! @{


//! flag to indicate if echoing or not
BOOL echo_on = TRUE;


//! @} oncli_pub_var
//							PUBLIC VARIABLES END
//==============================================================================




//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup oncli_pri_var
//! \ingroup oncli
//! @{
    

//! The state the ONCLI is in
static UInt8 state;

//! The verbosity mode of the device
static oncli_verbose_t verbosity = ONCLI_QUIET;

// The number of bytes in the command string so far.  This variable is so
// the strlen doesn't have to be computed every time.
static UInt16 input_len = 0;

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


static void echo(const char CH);
static void read_onc(void);
static void print_cmd_result(const char * const CMD,
  const oncli_status_t CMD_RESULT);


//! @} oncli_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup oncli_pub_func
//! \ingroup oncli
//! @{


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
    uart_write(output, output_len);
} // oncli_send_msg //


/*!
    \brief Main function for process handling ONE-NET Command Line Interface.
    
    This function should be called regularly from the programs main loop.
    
    \param void
    
    \return void
*/
void oncli(void)
{
    read_onc();
} // oncli //


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



//! @} oncli_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup oncli_pri_func
//! \ingroup oncli
//! @{

static void echo(const char CH)
{
    if(!echo_on)
    {
        return;
    } // if echoing is not enabled //

    oncli_send_msg("%c", CH);
    
} // echo //


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

                        if((status = oncli_parse_cmd(input, &CMD_STR, &next_state,
                          &cmd_hdlr)) != ONCLI_SUCCESS || (!cmd_hdlr))
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


/*!
    \brief Prints the result of the command
    
    \param[in] CMD The command string that was executed.
    \param[in] CMD_RESULT The result of CMD.
    
    \return void
*/
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



//! @} oncli_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


//! @} oncli
