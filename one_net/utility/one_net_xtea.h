#ifndef ONE_NET_XTEA_H
#define ONE_NET_XTEA_H

#include "config_options.h"
#include "one_net_types.h"


//! \defgroup one_net_xtea XTEA functionality
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
    \file one_net_xtea.h
    \brief Contains function prootypes for xtea functions.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_xtea_const
//! \ingroup one_net_xtea
//! @{

enum
{
    //! Length of an XTEA key (in bytes)
    ONE_NET_XTEA_KEY_LEN = 16,

    //! Size in bytes of a XTEA key fragment
    ONE_NET_XTEA_KEY_FRAGMENT_SIZE = 4,

    //! size of a block that gets enciphered/deciphered (in bytes)
    ONE_NET_XTEA_BLOCK_SIZE = 8
};

//! @} one_net_xtea_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_xtea_typedefs
//! \ingroup one_net_xtea
//! @{

//! Typedef the xtea key
typedef UInt8 one_net_xtea_key_t[ONE_NET_XTEA_KEY_LEN];

//! type of the XTEA key fragment
typedef UInt8 one_net_xtea_key_fragment_t[ONE_NET_XTEA_KEY_FRAGMENT_SIZE];


/*!
    \brief Encryption method used to encrypt single and block transactions

    The encryption method for a packets payload is only 2 bits.  These
    enumerations represent the 2 bit field indicating what method of
    encryption was used (already shifted to their position in the payload).
*/
typedef enum
{
    //! No encryption used. DEBUG ONLY
    ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE = 0x00,

    //! 32 round XTEA
    ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32 = 0x40,

    //! TBD
    ONE_NET_SINGLE_BLOCK_ENCRYPT_TBD0 = 0x80,

    //! TBD
    ONE_NET_SINGLE_BLOCK_ENCRYPT_TBD1 = 0xC0
} one_net_single_blk_encryption_t;


#ifdef STREAM_MESSAGES_ENABLED
/*!
    \brief Encryption method used to encrypt stream transactions

    The encryption method for a packets payload is only 2 bits.  These
    enumerations represent the 2 bit field indicating what method of
    encryption was used (already shifted to their position in the payload).
*/
typedef enum
{
    //! No encryption used. DEBUG ONLY
    ONE_NET_STREAM_ENCRYPT_NONE = 0x00,

    //! 32 round XTEA
    ONE_NET_STREAM_ENCRYPT_XTEA8 = 0x40,

    //! TBD
    ONE_NET_STREAM_ENCRYPT_TBD0 = 0x80,

    //! TBD
    ONE_NET_STREAM_ENCRYPT_TBD1 = 0xC0
} one_net_stream_encryption_t;
#endif


//! @} one_net_xtea_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup one_net_xtea_pub_var
//! \ingroup one_net_xtea
//! @{

//! @} one_net_xtea_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_xtea_pub_func
//! \ingroup one_net_xtea
//! @{

void one_net_xtea_encipher(const UInt8 ROUNDS, UInt8 * data,
  const one_net_xtea_key_t * const KEY);
void one_net_xtea_decipher(const UInt8 ROUNDS, UInt8 * data,
  const one_net_xtea_key_t * const KEY);

//! @} one_net_xtea_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_xtea

#endif // ifdef ONE_NET_XTEA_H //

