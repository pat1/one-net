//! \addtogroup one_net_xtea
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
    \file one_net_xtea.c
    \brief XTEA implementation.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include <one_net/one_net_xtea.h>

#include <one_net/port_specific/one_net_port_specific.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_xtea_const
//! \ingroup one_net_xtea
//! @{

//! Delta value applied during encryption/decryption
static const UInt32 DELTA = 0x9E3779B9;

//! @} one_net_xtea_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_xtea_typedefs
//! \ingroup one_net_xtea
//! @{

//! @} one_net_xtea_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup one_net_xtea_pri_var
//! \ingroup one_net_xtea
//! @{

//! @} one_net_xtea_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup one_net_xtea_pri_func
//! \ingroup one_net_xtea
//! @{

//! @} one_net_xtea_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup one_net_xtea_pub_func
//! \ingroup one_net_xtea
//! @{

/*!
    \brief Enciphers a 64-bit block using XTEA.

    \param[in] ROUNDS The number of rounds to perform.
    \param[in/out] data Input: The plain text.
                        Output: The cipher text.
    \param[in] KEY The key used to encipher the data

    \return void
*/
void one_net_xtea_encipher(const UInt8 ROUNDS, UInt8 * data,
  const one_net_xtea_key_t * const KEY)
{
    UInt32 v[2] = {0x00};
    UInt32 k[4] = {0x00};
    UInt32 sum = 0;
    UInt8 i;

    if(!data || !KEY)
    {
        return;
    } // if parameters are invalid //
    
    // get v
    v[0] = one_net_byte_stream_to_int32(data);
    v[1] = one_net_byte_stream_to_int32(data + sizeof(UInt32));

    // get k
    for(i = 0; i < sizeof(UInt32); i++)
    {
        k[i] = one_net_byte_stream_to_int32((const UInt8 * const)KEY
          + i * sizeof(UInt32));
    } // loop to get k //

    for(i = 0; i < ROUNDS; i++) 
    {
        v[0] += ((v[1] << 4 ^ v[1] >> 5) + v[1]) ^ (sum + k[sum & 3]);
        sum += DELTA;
        v[1] += ((v[0] << 4 ^ v[0] >> 5) + v[0]) ^ (sum + k[sum >> 11 & 3]);
    } // encipher loop //

    // convert output to byte stream
    one_net_int32_to_byte_stream(v[0], data);
    one_net_int32_to_byte_stream(v[1], data + sizeof(UInt32));
} // one_net_xtea_encipher //


/*!
    \brief Deciphers a 64-bit block using XTEA.

    \param[in] ROUNDS The number of rounds to perform.
    \param[in/out] data Input: The cipher text.
                        Output: The plain text.
    \param[in] KEY The key used to decipher the data

    \return void
*/
void one_net_xtea_decipher(const UInt8 ROUNDS, UInt8 * data,
  const one_net_xtea_key_t * const KEY)
{
    UInt32 v[2] = {0x00}; 
    UInt32 k[4] = {0x00};
    UInt32 sum = DELTA * ROUNDS;
    UInt8 i;

    // get v
    v[0] = one_net_byte_stream_to_int32(data);
    v[1] = one_net_byte_stream_to_int32(data + sizeof(v[0]));

    // get k
    for(i = 0; i < sizeof(UInt32); i++)
    {
        k[i] = one_net_byte_stream_to_int32((const UInt8 * const)KEY + i
          * sizeof(UInt32));
    } // loop to get k //
    
    for(i = 0; i < ROUNDS; i++) 
    {
        v[1] -= ((v[0] << 4 ^ v[0] >> 5) + v[0]) ^ (sum + k[sum>>11 & 3]);
        sum -= DELTA;
        v[0] -= ((v[1] << 4 ^ v[1] >> 5) + v[1]) ^ (sum + k[sum & 3]);
    } // loop to decipher //

    // convert output to byte_stream
    one_net_int32_to_byte_stream(v[0], data);
    one_net_int32_to_byte_stream(v[1], data + sizeof(v[0]));
} // one_net_xtea_decipher //

//! @} one_net_xtea_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup one_net_xtea_pri_func
//! \ingroup one_net_xtea
//! @{

//! @} one_net_xtea_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} one_net_xtea

