#ifndef _NPRINTF_H 
#define _NPRINTF_H 

#include <one_net/port_specific/config_options.h>


//! \defgroup nprintf
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
    \file nprintf.h

    \brief Contains the declaration for the Xnprintf functions (ie snprintf,
      vsnprintf, etc).

    \Note These functions are not thread safe.  snprintf and vsprintf cannot be
      called from different threads at the same time.
*/

#include <stdarg.h>
#include <stdio.h>


//==============================================================================
//								CONSTANTS
//! \defgroup nprintf_const 
//! \ingroup nprintf
//! @{

//! @} nprintf_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup nprintf_typedefs
//! \ingroup nprintf
//! @{

//! @} nprintf_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup nprintf_pub_var
//! \ingroup nprintf
//! @{

//! @} nprintf_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup nprintf_pub_func
//! \ingroup nprintf
//! @{

int snprintf(char * out_str, const size_t SIZE,
  const char * const FMT, ...);

int vsnprintf(char * out_str, const size_t SIZE,
  const char * FMT, va_list ap);

//! @} nprintf_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} nprintf

#endif // #ifdef _NPRINTF_H //
