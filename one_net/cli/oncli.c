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
#include "one_net.h"
#include "one_net_encode.h"
#include "oncli_str.h"
#include "one_net_types.h"
#include "oncli_port_const.h"
#include "oncli_port.h"
#include "nprintf.h"
#include "uart.h"
#include "oncli_hdlr.h"
#include "one_net_port_specific.h"
#include <ctype.h>
#include "one_net_channel.h"
#ifdef _ONE_NET_CLIENT
#include "one_net_client_port_specific.h"
#endif
#include "one_net_features.h"



//==============================================================================
//								CONSTANTS
//! \defgroup oncli_const
//! \ingroup oncli
//! @{


extern const char HEX_DIGIT[];


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
    \brief Formats a channel in a string format that shows the region.
    
    \param[in] channel : The channel (0 based) to display.
    \param[out] bufffer : The string representation of the buffer
    \param[in] buffer_len : The size of the buffer.
    
    \return A pointer to buffer.
*/
char* oncli_format_channel(UInt8 channel, char* buffer, UInt8 buffer_len)
{
    // As new channel domains are added, adjust this function as well as
    // the one_net_channel.h, oncli_str.h, and oncli_str.c files.
    // As of October 31, 2011, only U.S. and European domains are defined.
    if(!buffer)
    {
        // we are likely to crash here if we don't return something valid.
        // It is the reponsibility of the caller to make sure buffer is not
        // NULL.  We'll try to help it out by returning an "internal error"
        // string in case they try to immediately print a NULL string.  The
        // "internal error" string is a const char* const type, so it
        // shouldn't be changed.  However, we're in error recovery here, so
        // we'll just typecast to a char* and hope they do not change it.
        // It's a "should never get here" type of thing.
        return (char*) ONCLI_INTERNAL_ERR_STR;
    }

    if(buffer_len < MAX_CHANNEL_STRING_FORMAT_LENGTH)
    {
        // same thing here.  Just return a char* typedefed pointer to
        // ONCLI_INTERNAL_ERR_STR and hope they don't change it.  Should not
        // get here.
        return  (char*) ONCLI_INTERNAL_ERR_STR;
    }
    
    
#ifdef _US_CHANNELS
    if((SInt8)channel >= ONE_NET_MIN_US_CHANNEL && channel <= ONE_NET_MAX_US_CHANNEL)
    {
        // +1 since channels are stored 0 based, but output 1 based
        snprintf(buffer, MAX_CHANNEL_STRING_FORMAT_LENGTH,
          ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_US_STR, channel
          - ONE_NET_MIN_US_CHANNEL + 1);
    } // if a US channel //
#endif
#ifdef _EUROPE_CHANNELS
#ifdef _US_CHANNELS
    else if(channel >= ONE_NET_MIN_EUR_CHANNEL
      && channel <= ONE_NET_MAX_EUR_CHANNEL)
#else
    if((SInt8) channel >= ONE_NET_MIN_EUR_CHANNEL
      && channel <= ONE_NET_MAX_EUR_CHANNEL)
#endif
#endif
    {
        // +1 since channels are stored 0 based, but output 1 based
        snprintf(buffer, MAX_CHANNEL_STRING_FORMAT_LENGTH,
          ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_EUR_STR, channel
          - ONE_NET_MIN_EUR_CHANNEL + 1);
    } // else if a European channel //
    else
    {
        snprintf(buffer, MAX_CHANNEL_STRING_FORMAT_LENGTH,
          "%s", ONCLI_CHANNEL_INVALID_STR);
    } // else the channel is invalid //
    
    return buffer;
} // oncli_format_channel //


#ifdef _ONE_NET_CLIENT
/*!
    \brief Prints the invite code.
    
    \return ONCLI_SUCCESS If the message was successfully output.
*/
oncli_status_t oncli_print_invite(void)
{
    UInt8 * ptr_invite_key = one_net_client_get_invite_key();
    oncli_send_msg(ONCLI_DISPLAY_INVITE_STR, &ptr_invite_key[0], &ptr_invite_key[4]);
    return ONCLI_SUCCESS;
}
#endif


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


/*!
    \brief Prints the Raw DID
    
    \param[in] enc_did the encoded did
    
    \return ONCLI_SUCCESS if the DID was successfully output
*/
oncli_status_t oncli_print_did(const on_encoded_did_t* const enc_did)
{
    on_raw_did_t raw_did;
    on_decode(raw_did, *enc_did, ON_ENCODED_DID_LEN);
    oncli_send_msg("DID: 0x%03X", did_to_u16(&raw_did));
    return ONCLI_SUCCESS;
} // oncli_print_did //


/*!
    \brief Prints the SID(NID and DID)
    
    \param[in] enc_sid the encoded sid
        
    \return ONCLI_SUCCESS If the SID was successfully output.
*/
oncli_status_t oncli_print_sid(const on_encoded_sid_t* const enc_sid)
{
    on_raw_nid_t raw_nid;
    UInt8 i, nibble;
    
    on_decode(raw_nid, *enc_sid, ON_ENCODED_NID_LEN);

    oncli_send_msg("NID: 0x");
    for (i = 0; i < ON_RAW_NID_LEN; i++)
    {
        // 
        // print the high order nibble
        //
        nibble = (raw_nid[i] >> 4) & 0x0f;
        oncli_send_msg("%c", HEX_DIGIT[nibble]);

        //
        // if this is the not the last byte, print the low order nibble also
        //
        if (i < ON_RAW_NID_LEN-1)
        {
            nibble = raw_nid[i] & 0x0f;
            oncli_send_msg("%c", HEX_DIGIT[nibble]);
        }
    }
    oncli_send_msg("\n");
    oncli_print_did((on_encoded_did_t*)(&((*enc_sid)[ON_ENCODED_NID_LEN])));
    oncli_send_msg("\n");
    return ONCLI_SUCCESS;
} // oncli_print_sid //


/*!
    \brief Prints the data rate capabilities of a device
    
    \param[in] features the features the device supports.
        
    \return ONCLI_SUCCESS If the data rates were successfully output.
*/
oncli_status_t oncli_print_data_rates(on_features_t features)
{
    UInt8 i;
    for(i = 0; i < ONE_NET_DATA_RATE_LIMIT; i++)
    {
        BOOL dr_capable = features_data_rate_capable(features, i);
        oncli_send_msg("Data rate %s : %s\n", DATA_RATE_STR[i],
          dr_capable ? CAPABLE_STR : NOT_CAPABLE_STR);
    }
    
    return ONCLI_SUCCESS;
} // oncli_print_data_rates //


/*!
    \brief Prints the features / capabilities of a device
    
    \param[in] features the features the device supports.
        
    \return ONCLI_SUCCESS If the features were successfully output.
*/
oncli_status_t oncli_print_features(on_features_t features)
{
    oncli_send_msg("Max Hops : %d\n", features_max_hops(features));
    oncli_send_msg("Max Peers : %d\n", features_max_peers(features));
    oncli_send_msg("Multi-Hop : %s\n", features_mh_capable(features) ?
      CAPABLE_STR : NOT_CAPABLE_STR);
    oncli_send_msg("Multi-Hop Repeat : %s\n",
      features_mh_repeat_capable(features) ? CAPABLE_STR : NOT_CAPABLE_STR);
    oncli_send_msg("Block : %s\n",
      features_block_capable(features) ? CAPABLE_STR : NOT_CAPABLE_STR);
    oncli_send_msg("Stream : %s\n",
      features_stream_capable(features) ? CAPABLE_STR : NOT_CAPABLE_STR);
    oncli_send_msg("Device Sleeps : %s\n",
      features_device_sleeps(features) ? TRUE_STR : FALSE_STR);
    oncli_send_msg("ACK / NACK Level : %d\n",
      features_ack_nack_level(features));
    oncli_send_msg("\n\nData Rates...\n\n");
    oncli_print_data_rates(features);
    
    return ONCLI_SUCCESS;
} // oncli_print_features //


/*!
    \brief Prints the current channel
        
    \return ONCLI_SUCCESS If the channel was successfully output.
    \return ONCLI_CMD_FAIL If the cahnnel is invalid.
*/
oncli_status_t oncli_print_channel(void)
{
    #ifdef _US_CHANNELS
    if((SInt8)on_base_param->channel >= ONE_NET_MIN_US_CHANNEL &&
      on_base_param->channel <= ONE_NET_MAX_US_CHANNEL)
    {
        // +1 since channels are stored 0 based, but output 1 based
        oncli_send_msg(ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_US_STR,
          on_base_param->channel - ONE_NET_MIN_US_CHANNEL + 1);
    } // if a US channel //
    #endif
    #ifdef _EUROPE_CHANNELS
    #ifdef _US_CHANNELS
    else if(on_base_param->channel >= ONE_NET_MIN_EUR_CHANNEL
      && on_base_param->channel <= ONE_NET_MAX_EUR_CHANNEL)
    #else
    if((SInt8) on_base_param->channel >= ONE_NET_MIN_EUR_CHANNEL
      && on_base_param->channel <= ONE_NET_MAX_EUR_CHANNEL)
    #endif
    #endif
    {
        // +1 since channels are stored 0 based, but output 1 based
        oncli_send_msg(ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_EUR_STR,
          on_base_param->channel - ONE_NET_MIN_EUR_CHANNEL + 1);
    } // else if a European channel //
    else
    {
        return ONCLI_CMD_FAIL;
    } // else the channel is not selected //

    return ONCLI_SUCCESS;    
}


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

