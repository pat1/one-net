//! \defgroup ONE-NET_impl_specific Implementaton of ONE-NET Port Sepecific
//!   Functionality
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
    \file one_net_port_specific.c
    \brief Common implementation for ONE-NET port specific functionality.

    This file contains the common implementation for some of the port specific
    ONE-NET functionality declared in one_net_port_specific.h
*/

#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_impl_specific_const
//! \ingroup ONE-NET_impl_specific
//! @{

//! @} ONE-NET_impl_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_impl_specific_typedef
//! \ingroup ONE-NET_impl_specific
//! @{

//! @} ONE-NET_impl_specific_typedef
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_impl_specific_pri_var
//! \ingroup ONE-NET_impl_specific
//! @{

//! @} ONE-NET_impl_specific_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_impl_specific_pri_fun
//! \ingroup ONE-NET_impl_specific
//! @{

//! @} ONE-NET_impl_specific_pri_var
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_impl_specific_pub_func
//! \ingroup ONE-NET_impl_specific
//! @{


void *one_net_memmove(void *dst, const void *src, size_t n)
{
    UInt8 *d = (UInt8*) dst;
    const UInt8 *s = (const UInt8*) src;

    if (!d || !s) { /* no recovery but at least don't crash */
        return NULL;
    }
    
    if(n == 0 || dst == src)
    {
        return dst; // addresses are the same or the number of bytes to move is
                    // 0.  No need to "move" any memory.  Just return.
    }
    
	
	// copy from beginning to end a byte at a time
    if ((d < s) && (s < (d + n))) {
        for (; n > 0; --n) {
            *(d++) = *(s++);
        }
    }
	// copy in reverse order from end to beginning a byte at a time
    else {
        for (d += n, s += n; n > 0; --n) {
            *(--d) = *(--s);
        }
    }
    return dst;
}


void * one_net_memset (void* dst, UInt8 value, size_t len)
{
    size_t i;
    UInt8* ptr;
    
    if (!dst) 
    { /* no recovery but at least don't crash */
        return NULL;
    }
    
    ptr = (UInt8*) dst;
    
    for(i = 0; i < len; i++)
    {
        *(ptr++) = value;
    }
    
    return dst;
}


void* one_net_memset_block(void* const dst, size_t size, size_t count,
  const void* const src)
{
    size_t i;
    UInt8* ptr;
    
    if (!dst) 
    { /* no recovery but at least don't crash */
        return NULL;
    }    
    
    ptr = (UInt8*) dst;
    
    for(i = 0; i < count; i++)
    {
        one_net_memmove(ptr, src, size);
        ptr += size;
    }

    return dst;
}


SInt8 one_net_memcmp(const void *vp1, const void *vp2, size_t n)
{
    const UInt8 * up1;
    const UInt8 * up2;
    // If invalid, just return zero since there is no error recovery
    if (!vp1 || !vp2)
    {
        return 0;
    }
    for (up1 = (const UInt8*) vp1, up2 = (const UInt8*) vp2;
      n > 0; ++up1, ++up2, --n)
    {
        if (*up1 != *up2)
        {
            return ((*up1 < *up2) ? -1 : 1);
        }
    }
    return 0;
}


UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM)
{
    UInt16 val;
    val = (((UInt16)BYTE_STREAM[0]) << 8) & 0xFF00;
    val |= ((UInt16)BYTE_STREAM[1]) & 0x00FF;
    
    return val;
} // one_net_byte_stream_to_int16 //


void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream)
{
    byte_stream[0] = (UInt8)(VAL >> 8);
    byte_stream[1] = (UInt8)VAL;
} // one_net_int16_to_byte_stream //


UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM)
{
    UInt32 val;
    val = (((UInt32)BYTE_STREAM[0]) << 24) & 0xFF000000;
    val |= (((UInt32)BYTE_STREAM[1]) << 16) & 0x00FF0000;
    val |= (((UInt32)BYTE_STREAM[2]) << 8) & 0x0000FF00;
    val |= ((UInt32)BYTE_STREAM[3]) & 0x000000FF;

    return val;
} // one_net_byte_stream_to_int32 //


void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream)
{
    byte_stream[0] = (UInt8)(VAL >> 24);
    byte_stream[1] = (UInt8)(VAL >> 16);
    byte_stream[2] = (UInt8)(VAL >> 8);
    byte_stream[3] = (UInt8)VAL;
} // one_net_int32_to_byte_stream //


/*!
    \brief Converts a raw did to a U16 value.
    
    \param[in] DID The device id to convert
    
    \return The UInt16 value corresponding to the DID.
*/
UInt16 did_to_u16(const on_raw_did_t *DID)
{
    return one_net_byte_stream_to_int16(*DID) >> RAW_DID_SHIFT;
} // did_to_u16 //


/*!
    \brief converts a U16 value to a raw DID
    
    \param[in] raw_did_int -- the UInt16 representation of the raw DID
      (0 - 4015 range)
    \param[out] raw_did -- the converted raw DID
    
    \return True if the conversion was successful, false otherwise
*/
BOOL u16_to_did(UInt16 raw_did_int, on_raw_did_t* raw_did)
{
    if(!raw_did || raw_did_int > 0xFFF)
    {
        return FALSE;
    }
    
    (*raw_did)[1] = ((raw_did_int & 0x0F) << RAW_DID_SHIFT);
    (*raw_did)[0] = (raw_did_int >> RAW_DID_SHIFT);
    return TRUE;
}


/*!
    \brief Converts string to long integer.
    
    \param[in] str String to convert
    \param[in] endptr Reference to an object of type char*, whose value is set
        by the function to the next character in str after the numerical
        value.  This parameter can also be a null pointer, in which case it is
        not used.
    \param[in] base String to convert
    
    \return The "base" of the string representation.
*/
long int one_net_strtol(const char * str, char ** endptr, int base)
{
    // use near pointers
    return _n_n_strtol(str, endptr, base);
}


//! @} ONE-NET_impl_specific_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_impl_specific_pri_func
//! \ingroup ONE-NET_impl_specific
//! @{

//! @} ONE-NET_impl_specific_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_impl_specific



