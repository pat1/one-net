//! \addtogroup one_net_crc
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
    \file one_net_crc.c
    \brief Basis for crc implementation.

    This file is application independent.  The functionality implemented here
    is also independent of the device being a MASTER or CLIENT.  Some of the
    functions do make calls that are dependent on the device being a MASTER
    or CLIENT, or some other hardware dependent calls, but the implementation
    for those calls are elsewhere.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_port_specific.h"
#include "one_net_xtea.h"


// TODO -- this is a bit messy.  Find a better #define test.
#if defined(_R8C_TINY) && !defined(_QUAD_OUTPUT)
    #pragma section program program_high_rom
#endif // if _R8C_TINY and not a 16K chip //


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_crc_const
//! \ingroup one_net_crc
//! @{

//! @} one_net_crc_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_crc_typedefs
//! \ingroup one_net_crc
//! @{

//! @} one_net_crc_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup one_net_crc_pri_var
//! \ingroup one_net_crc
//! @{

//! @} one_net_crc_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup one_net_crc_pri_func
//! \ingroup one_net_crc
//! @{

//! @} one_net_crc_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup one_net_crc_pub_func
//! \ingroup one_net_crc
//! @{

/*
    \brief Computes CRCs for 16th order or less polynomials.

    This is a very slow algorithm as it goes through bit by bit.  This was
    done so many different crcs can be implemented using the same code.

    Currently, only an 8th order polynomial is implemented.

    \param[in] DATA, byte stream of data to get CRC for
    \param[in] LEN, length of given byte stream
    \param[in] STARTING_CRC, the inital crc to use
    \param[in] ORDER The order of the polynomial to compute
   
    \return an updated CRC (in a UInt16)

*/
UInt16 one_net_compute_crc(const UInt8 * const DATA, const UInt8 LEN, 
  const UInt16 STARTING_CRC, const UInt8 ORDER)
{
    // bit by bit algorithm without augmented zero bytes.
    // does not use lookup table, suited for polynom orders between 1...16.

    UInt16 polynom;

    UInt16 i, j, c, bit, mask;
    UInt16 crc = STARTING_CRC;
    UInt16 crc_high_bit = (UInt16)1 << (ORDER - 1);
    const UInt8 * p = DATA;

    if(!DATA)
    {
        return 0;
    } // parameter was invalid //

    switch(ORDER)
    {
        case 8:
        {
            polynom = 0x00A6;
            mask = 0x00FF;
            break;
        } // 8th order crc //

        default:
        {
            return 0;
            break;
        } // default //
    } // switch polynomial order //

    for(i = 0; i < LEN; i++)
    {
        c = (UInt16)*p++;
        for(j = 0x80; j; j >>= 1)
        {
            bit = crc & crc_high_bit;
            crc <<= 1;

            if(c & j)
            {
                bit ^= crc_high_bit;
            } // if c & j //

            if(bit)
            {
                crc ^= polynom;
            } // if bit //
        } // for j //
    } // for LEN //

    return crc & mask;
} // one_net_compute_crc //

//! @} one_net_crc_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup one_net_crc_pri_func
//! \ingroup one_net_crc
//! @{

//! @} one_net_crc_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} one_net_crc

