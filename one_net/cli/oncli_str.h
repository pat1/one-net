#ifndef _ONCLI_STR_H
#define _ONCLI_STR_H


//! \defgroup oncli_str ONE-NET Command Line Interface Strings
//! \ingroup oncli
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
    \file oncli_str.h

    \brief Contains string constants associated with the ONE-NET Command Line
      interface.

    These strings are to be used in a case insensitive manner.
*/


#include "config_options.h"


//==============================================================================
//								CONSTANTS
//! \defgroup oncli_str_const
//! \ingroup oncli_str
//! @{



// Device strings
extern const char * const ONCLI_MASTER_STR;
extern const char * const ONCLI_CLIENT_STR;



// Argument strings
extern const char * const ONCLI_ON_STR;
extern const char * const ONCLI_OFF_STR;



// Command Strings
#ifdef _ENABLE_ECHO_COMMAND
    extern const char * const ONCLI_ECHO_CMD_STR;
#endif



// Reponse Strings
extern const char * const ONCLI_CMD_SUCCESS_STR;
extern const char * const ONCLI_SUCCEEDED_STR;
extern const char * const ONCLI_FAILED_STR;
extern const char * const ONCLI_CLR_INPUT_STR;



// Error Formats
extern const char * const ONCLI_INVALID_CMD_FMT;
extern const char * const ONCLI_CMD_FAIL_FMT;
extern const char * const ONCLI_INVALID_CMD_FOR_DEVICE_FMT;
extern const char * const ONCLI_INTERNAL_ERR_FMT;
extern const char * const ONCLI_GENERAL_ERROR_FMT;
extern const char * const ONCLI_RX_INVALID_CH_FMT;
extern const char * const ONCLI_OUTPUT_STR_TOO_SHORT_FMT;


// Error Strings
extern const char * const ONCLI_IN_PROGRESS_STR;
extern const char * const ONCLI_RSRC_UNAVAILABLE_STR;
extern const char * const ONCLI_UNSUPPORTED_STR;
extern const char * const ONCLI_INVALID_FORMAT_STR;
extern const char * const ONCLI_SNGH_STR;
extern const char * const ONCLI_ONS_NOT_INIT_ERR_STR;
extern const char * const ONCLI_INTERNAL_ERR_STR;
extern const char * const ONCLI_INVALID_DST_STR;
extern const char * const ONCLI_NEED_TO_JOIN_STR;
extern const char * const ONCLI_INVALID_CMD_LEN_STR;
extern const char * const ONCLI_LOAD_FAIL_STR;
extern const char * const ONCLI_DISPLAY_INVITE_STR;
extern const char * const ONCLI_FATAL_ERR_1_STR;
enum
{
    ONCLI_FATAL_ERR_1_LEN = 15
};



//! @} oncli_str_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup oncli_str_typedefs
//! \ingroup oncli_str
//! @{

//! @} oncli_str_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup oncli_str_pub_var
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup oncli_str_pub_func
//! \ingroup oncli_str
//! @{

//! @} oncli_str_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} oncli_str


#endif // #ifdef _ONCLI_STR_H //
