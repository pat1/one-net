#ifndef _ONCLI_H 
#define _ONCLI_H 

#include "config_options.h"
#ifdef UART




#include "one_net_types.h"
#include "oncli_port.h"
#include "one_net_xtea.h"
#include "one_net_constants.h"
#include "one_net_features.h"
#include "one_net_status_codes.h"
#if _DEBUG_VERBOSE_LEVEL > 3
#include "one_net_acknowledge.h"
#endif



//! \defgroup oncli ONE-NET Command Line Interface
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
    \file oncli.h

    \brief Contains declarations for ONE-NET Command Line Interface
      functionality

    The ONE-NET Command Line Interface is an ASCII protocol designed for the
    ONE-NET evaluation boards so a user can easily test and evaluate the
    ONE-NET protocol (MAC layer).
*/


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


//! States for reading input
enum
{
    //! State while command is being read in
    ONCLI_RX_CMD_STATE,
    
    //! State while reading in parameter list, looking for new line
    ONCLI_RX_PARAM_NEW_LINE_STATE,
    
    //! State while reading in parameter list, looking for new line or quote
    ONCLI_RX_PARAM_NEW_LINE_OR_QUOTE_STATE,

    //! State while reading in parameter list, looking for closing quote
    ONCLI_RX_PARAM_QUOTE_STATE
};


typedef oncli_status_t (*oncli_cmd_hdlr_t)(const char * const ASCII_PARAM_LIST);


//! @} oncli_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup oncli_pub_var
//! \ingroup oncli
//! @{


#ifdef ALLOW_INPUT_ECHOING
extern BOOL echo_on;
#endif
extern BOOL newline_rcvd;
extern BOOL command_processed;
#ifdef _DEBUG_VERBOSE_LEVEL
    extern UInt8 verbose_level;
#else
    #error "_DEBUG_VERBOSE_LEVEL is not defined.  Please define it in config_options.h"
#endif

//! If binary_mode is true, uart input and outout is considered to be in binary
//! format.  Hence, bytes will not be interpreted as characters that were typed in
//! and are to be displayed and characters such as '\b', '\r', '\n', and DEL will
//! not be interpreted as back-spaces, carriage rerurns, newlines, delete keys,
//! etc., so checking and handling those cases will not occur.
extern BOOL binary_mode;



//! @} oncli_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup oncli_pub_func
//! \ingroup oncli
//! @{



#ifdef _DEBUGGING_TOOLS
void xdump(const UInt8* const ptr, UInt16 len);
#endif


BOOL oncli_user_input(void);
void oncli_send_msg(const char * const FMT, ...);
void oncli(void);
UInt16 oncli_read(UInt8 * buf, const UInt16 SIZE);
char* oncli_format_channel(UInt8 channel, char* buffer, UInt8 buffer_len);
#ifdef ONE_NET_CLIENT
oncli_status_t oncli_print_invite(void);
#endif
void oncli_print_xtea_key(const one_net_xtea_key_t* KEY);
oncli_status_t oncli_print_did(const on_encoded_did_t* const enc_did);
oncli_status_t oncli_print_sid(const on_encoded_sid_t* const enc_sid);
oncli_status_t oncli_print_data_rates(on_features_t features);
#if defined(ENABLE_LIST_COMMAND) && defined(PEER)
oncli_status_t oncli_print_peer_list(void);
#endif
oncli_status_t oncli_print_features(on_features_t features);
oncli_status_t oncli_print_channel(UInt8 channel);
#ifdef BLOCK_MESSAGES_ENABLED
void oncli_print_fragment_delays(void);
#endif

BOOL oncli_is_valid_unique_key_ch(const char CH);

const char * oncli_msg_status_str(on_message_status_t status);

#if _DEBUG_VERBOSE_LEVEL > 3
void print_msg_hdr(const on_msg_hdr_t* const msg_hdr);
void print_ack_nack(const on_ack_nack_t* ack_nack, UInt8 pld_len);
void print_app_payload(const UInt8* const payload, UInt8 pld_len);
#ifdef BLOCK_MESSAGES_ENABLED
#ifndef STREAM_MESSAGES_ENABLED
void print_bs_pkt(const block_stream_pkt_t* bs_pkt, BOOL print_msg_id);
#else
void print_bs_pkt(const block_stream_pkt_t* bs_pkt, BOOL print_msg_id,
  BOOL packet_is_stream);
#endif
#endif
#if _DEBUG_VERBOSE_LEVEL > 4
void print_single(UInt16 pid, const UInt8* raw_payload);
void print_response(UInt16 pid, const UInt8* raw_payload);
#endif
void print_admin_payload(const UInt8* const pld);
#ifdef ROUTE
void print_route(const UInt8* const route);
#endif
void print_recipient_list(const on_recipient_list_t* const recip_list);
void print_send_list(void);
#ifdef ONE_NET_CLIENT
void print_client_send_list(void);
#endif
#ifdef ONE_NET_MASTER
void print_master_send_list(void);
#endif
void print_sending_device_t(const on_sending_device_t* const device);
#endif

void print_raw_pid(UInt16 raw_pid);




//! @} oncli_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli


#endif // #ifdef UART //

#endif // #ifdef _ONCLI_H //
