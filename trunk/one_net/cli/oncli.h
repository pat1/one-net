#ifndef _ONCLI_H 
#define _ONCLI_H 

//! \defgroup oncli ONE-NET Command Line Interface
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
    \file oncli.h

    \brief Contains declarations for ONE-NET Command Line Interface
      functionality

    The ONE-NET Command Line Interface is an ASCII protocol designed for the
    ONE-NET evaluation boards so a user can easily test and evaluate the
    ONE-NET protocol (MAC layer).
*/

#include "oncli_port.h"
#include "one_net_types.h"


//==============================================================================
//								CONSTANTS
//! \defgroup oncli_const 
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
//                              PUBLIC VARIABLES
//! \defgroup oncli_pub_var
//! \ingroup oncli
//! @{

//! @} oncli_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup oncli_pub_func
//! \ingroup oncli
//! @{

BOOL oncli_user_input(void);

void oncli_set_echo(const BOOL ECHO);

const char * oncli_status_str(one_net_status_t STATUS);

void oncli_print_admin_msg(const UInt8 MSG_TYPE, const UInt8 TXN_TYPE,
  const UInt8 ADMIN_MSG_TYPE, const UInt8 * ADMIN_MSG_DATA, const UInt16 LEN);

UInt16 oncli_read(UInt8 * buf, const UInt16 SIZE);
void oncli_send_msg(const char * const FMT, ...);
#if defined(_NEED_XDUMP) || defined(_ENABLE_DUMP_COMMAND)
void xdump(UInt8 *pt, UInt16 len);
#endif


void oncli(void);

//! @} oncli_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli

#endif // #ifdef _ONCLI_H //

