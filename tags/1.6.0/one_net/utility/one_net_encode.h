#ifndef _ONE_NET_ENCODE_H
#define _ONE_NET_ENCODE_H

//! \defgroup ONE-NET_encode ONE-NET Encoding and Decoding
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
    \file one_net_encode.h
    \brief ONE-NET encoding/decoding declarations.

    \note These functions SHOULD NOT be called by any application code!
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_status_codes.h"
#include "one_net_types.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_encode_const
//! \ingroup ONE-NET_encode
//! @{

enum
{
    //! the number of bits in a raw work
    ON_RAW_WORD_SIZE = 6,

    //! the number of bits in an encoded word
    ON_ENCODED_WORD_SIZE = 8
};
    
//! @} ONE-NET_encode_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_encode_typedefs
//! \ingroup ONE-NET_encode
//! @{

//! @} ONE-NET_encode_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_encode_pub_var
//! \ingroup ONE-NET_encode
//! @{

//! @} ONE-NET_encode_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_encode_pub_func
//! \ingroup ONE-NET_encode
//! @{

one_net_status_t on_encode(UInt8 * encoded, const UInt8 * RAW, 
  const UInt16 ENCODED_SIZE);
one_net_status_t on_decode(UInt8 * raw, const UInt8 * ENCODED, 
  const UInt16 ENCODED_SIZE);

//! @} ONE-NET_encode_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_encode

#endif // _ONE_NET_ENCODE_H //

