//! \addtogroup str
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
    \file str.c
    \brief Contains the implementation for string manipulation
*/


#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //


#include "str.h"

#include <ctype.h>


//==============================================================================
//								CONSTANTS
//! \defgroup str_const
//! \ingroup str
//! @{

//! @} str_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup str_typedefs
//! \ingroup str
//! @{

//! @} str_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup str_pri_var
//! \ingroup str
//! @{

//! @} str_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup str_pri_func
//! \ingroup str
//! @{

//! @} str_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup str_pub_func
//! \ingroup str
//! @{

/*!
    \brief Converts an ASCII character to a hex value.

    \param[in] ASCII The ASCII byte to convert.
    
    \return The corresponding hex value (0x00 - 0x0F), or 0xFF if it is not a
      valid ASCII hex value.
*/
UInt8 ascii_hex_to_nibble(const char ASCII)
{
    if(!isxdigit(ASCII))
    {
        return 0xFF;
    } // if one of the parameters is not valid //
    
    if(ASCII <= '9')
    {
        return ASCII - '0';
    } // if it is numeric //

    return (ASCII | 0x20) - 'a' + 0x0A;
} // ascii_hex_to_byte //

//! @} str_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup str_pri_func
//! \ingroup str
//! @{

//! @} str_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} str
