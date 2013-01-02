#ifndef _ONE_NET_TYPES_H
#define _ONE_NET_TYPES_H

#include "config_options.h"


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
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_Types_const
//! \ingroup ONE-NET_Types
//! @{

//! The Renesas HEW tool chain does not seem to like the normal "static inline", so we
//! use only "inline" to make it happy.
#define ONE_NET_INLINE  static inline

//! @} ONE-NET_Types_cons
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_Types_type_def
//! \ingroup ONE-NET_Types
//! @{

typedef unsigned char   UInt8;
typedef signed char     SInt8;
typedef unsigned int    UInt16;
typedef int             SInt16;
typedef unsigned long   UInt32;
typedef long            SInt32;
typedef float           Float32;

typedef UInt32			tick_t;

enum
{
    FALSE = 0,
    TRUE
};

typedef UInt8 BOOL;

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
