//! \addtogroup ONE-NET_encode
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
    \file one_net_encoded.c
    \brief ONE-NET encoding/decoding definitions.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_encode.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_encode_const
//! \ingroup ONE-NET_encode
//! @{

/*!
    \brief Table to convert packets from raw values to encoded values.

    The raw value is an index into this table which returns the coresponding
    encoded value.
*/
static const UInt8 RAW_TO_ENCODED[] =
{
    0xB4, 0xBC, 0xB3, 0xBA, 0xB5, 0xB9, 0xB6, 0xB2,
    0xC4, 0xCC, 0xC3, 0xCA, 0xC5, 0xC9, 0xC6, 0xC2,
    0x34, 0x3C, 0x33, 0x3A, 0x35, 0x39, 0x36, 0x32,
    0xA4, 0xAC, 0xA3, 0xAA, 0xA5, 0xA9, 0xA6, 0xA2,
    0x54, 0x5C, 0x53, 0x5A, 0x55, 0x59, 0x56, 0x52,
    0x94, 0x9C, 0x93, 0x9A, 0x95, 0x99, 0x96, 0x92,
    0x64, 0x6C, 0x63, 0x6A, 0x65, 0x69, 0x66, 0x62,
    0xD4, 0xDC, 0xD3, 0xDA, 0xD5, 0xD9, 0xD6, 0xD2
};


/*!
    \brief The first of 2 tables to convert encoded values to raw values.

    This table contains the upper nibble conversion.  The upper nibble of
    the encoded value is used as an index into this table.  The lower nibble
    will index the lower nibble table.  The two results will be added.  Invalid
    items are marked with 0x40.  If upper &/or lower table have an invalid 
    index, the result of the addition will be more than the max raw value size 
    (invalid numbers were chosen to avoid overflow).
*/
static const UInt8 ENCODED_TO_RAW_H_NIB[] =
{
    0x40, 0x40, 0x40, 0x10, 0x40, 0x20, 0x30, 0x40,
    0x40, 0x28, 0x18, 0x00, 0x08, 0x38, 0x40, 0x40
};


/*!
    \brief The second of 2 tables to convert encoded values to raw values.

    This table contains the lower nibble conversion.  The lower nibble of the
    encoded value is used as an index into this table.  The upper nibble will
    index the upper nibble table, and the two results will be added together.
    Invalid items are marked with 0x40.  If upper &/or lower table have an
    invalid index, the result of the addition will be more than the max raw
    value size (invalid numbers were chosen to avoid overflow).
*/
static const UInt8 ENCODED_TO_RAW_L_NIB[] =
{
     0x40, 0x40, 0x07, 0x02, 0x00, 0x04, 0x06, 0x40,
     0x40, 0x05, 0x03, 0x40, 0x01, 0x40, 0x40, 0x40,
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
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_encode_pri_var
//! \ingroup ONE-NET_encode
//! @{

//! @} ONE-NET_encode_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_encode_pri_func
//! \ingroup ONE-NET_encode
//! @{

//! @} ONE-NET_encode_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_encode_pub_func
//! \ingroup ONE-NET_encode
//! @{

/*!
    \brief Encodes 6-bit data to 8-bit data.

    Takes in a bit stream and converts every 6 bits into an encoded 8-bit value.

    \param[out] encoded The encoded version of the raw data.
    \param[in] RAW The raw bit stream to encoded.
    \param[in] ENCODED_SIZE The size of encoded in bytes.  This is also the
     number of 6 bit blocks to encoded.

    \return The status of the operation.
*/
one_net_status_t on_encode(UInt8 * encoded, const UInt8 * RAW, 
  const UInt16 ENCODED_SIZE)
{
    UInt16 val, encoded_idx, raw_idx, step;

    if(!encoded || !RAW || !ENCODED_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    val = 0;
    raw_idx = 0;
    step = 0;

    // there are 4 stages when looping through the bit stream to collect the 6
    // bit values as they cross byte boundries.  The diagram below shows the
    // pattern of how a stream of 24 bits grouped as 6-bit values repeats.
    // The 1, 2, 3 accross the top represent the byte that the current 6-bit
    // group in the pattern is in.  The bits in the bytes show which byte(s)
    // the current 6-bit value falls into.
    //      1          2          3
    // +----------+----------+----------+
    // + 11111122 + 22223333 + 33444444 +
    // +----------+----------+----------+
    // Only step through the number of
    // steps necessary to convert all the data
    for(encoded_idx = 0; encoded_idx < ENCODED_SIZE; encoded_idx++)
    {
        switch(step)
        {
            case 0:
            {
                val = (RAW[raw_idx] >> 2) & 0x3F;
                step++;
                break;
            } // first step //

            case 1:
            {
                val = (RAW[raw_idx++] << 4) & 0x30;
                val |= (RAW[raw_idx] >> 4) & 0x0F;
                step++;
                break;
            } // second step //

            case 2:
            {
                val = (RAW[raw_idx++] << 2) & 0x3C;
                val |= (RAW[raw_idx] >> 6) & 0x03;
                step++;
                break;
            } // third step //

            case 3:
            {
                val = RAW[raw_idx++] & 0x3F;
                step = 0;
                break;
            } // fourth step //

            default:
            {
                // should never get here
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch (step)

#ifdef _ONE_NET_USE_ENCODING
        encoded[encoded_idx] = RAW_TO_ENCODED[val];
#else
        encoded[encoded_idx] = val;
#endif
    } // loop to encoded raw data //

    return ONS_SUCCESS;
} // on_encode //


/*!
    \brief Decodes 8-bit data to 6-bit data.

    Takes in a bit stream and converts every 8 bits into an decoded 6-bit value.
    These decoded 6-bit values are returned as a bit stream.

    \param[out] raw The decoded version of the encoded data.
    \param[in] ENCODED The encoded bit stream to decode.
    \param[in] ENCODED_LEN The number of encoded blocks to decode.  raw must be
      big enough to handle the data.
    \return The status of the operation.
*/
one_net_status_t on_decode(UInt8 * raw, const UInt8 * ENCODED, 
  const UInt16 ENCODED_SIZE)
{
    UInt16 val, encoded_idx, raw_idx, step;

    if(!ENCODED || !raw || !ENCODED_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if parameters are not valid //

    val = 0;
    raw_idx = 0;
    step = 0;

    // there are 4 stages when looping through the bit stream to collect the 6
    // bit values as they cross byte boundries. Only step through the number of
    // steps necessary to convert all the data
    for(encoded_idx = 0; encoded_idx < ENCODED_SIZE; encoded_idx++)
    {
#ifdef _ONE_NET_USE_ENCODING
        val = ENCODED_TO_RAW_H_NIB[(ENCODED[encoded_idx] >> 4) & 0x0F]
         + ENCODED_TO_RAW_L_NIB[ENCODED[encoded_idx] & 0x0F];
#else
        val = ENCODED[encoded_idx];
#endif

        // value >= 0x40 means the it was not a valid encoded value
        if(val >= 0x40)
        {
            return ONS_BAD_ENCODING;
        } // if not valid encoded value //

        switch(step)
        {
            case 0:
            {
                raw[raw_idx] = (val << 2) & 0xFC;
                step++;
                break;
            } // step 1 //

            case 1:
            {
                raw[raw_idx++] |= (val >> 4) & 0x03;
                raw[raw_idx] = (val << 4) & 0xF0;
                step++;
                break;
            } // step 2 //

            case 2:
            {
                raw[raw_idx++] |= (val >> 2) & 0x0F;
                raw[raw_idx] = (val << 6) & 0xC0;
                step++;
                break;
            } // step 3 //

            case 3:
            {
                raw[raw_idx++] |= val & 0x3F;
                step = 0;
                break;
            } // step 4 //

            default:
            {
                // should never get here
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch (step)
    } // loop to decode data //

    return ONS_SUCCESS;
} // on_decode //

//! @} ONE-NET_encode_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_encode_pri_func
//! \ingroup ONE-NET_encode
//! @{

//! @} ONE-NET_encode_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_encode

