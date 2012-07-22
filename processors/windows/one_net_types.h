#ifndef _ONE_NET_TYPES_H
#define _ONE_NET_TYPES_H


#include <windef.h>
#include <stdint.h>


//! \defgroup ONE-NET_Types Type declarations for ONE-NET.
//! \ingroup ONE-NET
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
    \file one_net_types.h
    \brief Types used by ONE-NET

    This file defines known types and sizes.  It is processor dependent, so it
    will need to be changed when the processor is changed.
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_Types_const
//! \ingroup ONE-NET_Types
//! @{

//! The Renesas HEW tool chain does not seem to like the normal "static inline", so we
//! use only "inline" to make it happy, but on a non-embedded system we'll use "static inline" for
//! proper linking.
#define ONE_NET_INLINE  static inline

//! @} ONE-NET_Types_cons
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_Types_type_def
//! \ingroup ONE-NET_Types
//! @{

typedef uint8_t    UInt8;
typedef int8_t     SInt8;
typedef uint16_t   UInt16;
typedef int16_t    SInt16;
typedef uint32_t   UInt32;
typedef int32_t    SInt32;
typedef float      Float32;
typedef UInt32	   tick_t;


// TODO -- BOOL, FALSE, and TRUE need top be defined / enumerated / typedefed somewhere.  If we do it here, we seem
// to end defining it twice, so I have taken it out since it seems to be defined in windef.h.  I'm using Code Blocks with
// mingw 3.4.5 on Windows XP.  If someone is using something else and BOOL< TRUE, and FALSE are not defined, then define them here.


//! @} ONE-NET_Types_type_def
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_Types_pub_var
//! \ingroup ONE-NET_Types
//! @{

//! @} ONE-NET_Types_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_Types_pub_func
//! \ingroup ONE-NET_Types
//! @{

//! @} ONE-NET_Types_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_Types

#endif // _ONE_NET_TYPES_H //
