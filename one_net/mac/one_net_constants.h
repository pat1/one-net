#ifndef _ONE_NET_CONSTANTS_H
#define _ONE_NET_CONSTANTS_H




//! \defgroup ONE-NET Constants
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
    \file one_net_contants.h
    \brief ONE-NET common ONE-NET constants and typedefs

    Application independent constants.
*/

#include "one_net_types.h"


//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET Constants_typedefs
//! \ingroup ONE-NET Constants
//! @{


//! Address related constants
enum
{
    //! Raw Network ID length (in bytes needed to store the value)
    ON_RAW_NID_LEN = 5,

    //! Raw Device ID length (in bytes needed to store the value)
    ON_RAW_DID_LEN = 2,

    //! Raw System ID length (in bytes)
    ON_RAW_SID_LEN = 6,

    //! Encoded Network ID length (in bytes)
    ON_ENCODED_NID_LEN = 6,

    //! Encoded Device ID length (in bytes)
    ON_ENCODED_DID_LEN = 2,

    //! Encoded SID length (in bytes)
    ON_ENCODED_SID_LEN = 8,
    
    //! Number of bits to shift when converting between raw and encoded DIDs
    RAW_DID_SHIFT = 4
};


//! type of a raw Network ID
typedef UInt8 on_raw_nid_t[ON_RAW_NID_LEN];

//! type of a raw Device ID
typedef UInt8 on_raw_did_t[ON_RAW_DID_LEN];

//! type of a raw System ID
typedef UInt8 on_raw_sid_t[ON_RAW_SID_LEN];

//! type of an encoded Network ID
typedef UInt8 on_encoded_nid_t[ON_ENCODED_NID_LEN];

//! type of an encoded Device ID
typedef UInt8 on_encoded_did_t[ON_ENCODED_DID_LEN];

//! type of an encoded System ID
typedef UInt8 on_encoded_sid_t[ON_ENCODED_SID_LEN];


//! @} ONE-NET Constants_typedefs
//                                  TYPEDEFS END
//==============================================================================



#endif // _ONE_NET_CONSTANTS_H //

