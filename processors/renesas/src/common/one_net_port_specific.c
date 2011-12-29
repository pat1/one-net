//! \defgroup ONE-NET_impl_specific Implementaton of ONE-NET Port Sepecific
//!   Functionality
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
    \file one_net_port_specific.c
    \brief Common implementation for ONE-NET port specific functionality.

    This file contains the common implementation for some of the port specific
    ONE-NET functionality declared in one_net_port_specific.h
*/

#include "one_net_port_specific.h"
#include "pal.h"
#include "tick.h"


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



/*
 * dje: The next function (short version of memmove) was used in
 * versions before 1.6.0.
 * It has the problem that it doesn't handle cases where the source
 * is below the destination and the buffers overlap.  It really,
 * really (really) shouldn't have been named one_net_memmove. 
 * (It should have been named one_net_memcpy or some such thing.)
 * 
 * Anyhow...
 *
 * Versions of one-net before 1.6.0 apparently used it without ever
 * calling it with fatal overlaps.  This is a Bad Thing, and, in fact
 * programmers working on 1.6 didn't know it was unsafe and wasted
 * a lot of time running down the bug.  (In my opinion, the unfortunate
 * choice of the name was just not a matter of bad style, but it was
 * a bug waiting to manifest itself.  Really.)
 *
 * So...
 *
 * I'll leave it here 'just in case,' but I'll disable it in favor of
 * the "real" memmove (the safe one) that follows.
 *
 * Dave Evans
 * Threshold Corporation
 * November 5, 2010
 *
 */

#ifdef _USE_UNSAFE_MEMMOVE
/*
 * This is the Bad Boy
*/
void * one_net_memmove(void * dst, const void * SRC, size_t len)
{
    const UInt8 * S = SRC;

    UInt16 bytes_copied;
    UInt8 * d = dst;
    
    if(!d || !S)
    {
        EXIT();
    } // if the parameters are invalid //
    
    for(bytes_copied = 0; bytes_copied < len; bytes_copied++)
    {
        *d++ = *S++;
    } // loop through to copy the bytes //
    
    return dst;
} // one_net_memmove //
#else


/* 
 * Safe copy src to dst is consistent with standard C library
 * function memmove, and handles all cases whether the
 * buffers overlap or not.
 *
 * Note that this takes 23 more bytes than the above function.
 *
 * If you really, really can't spare the 23 bytes, then you can try
 * the simpler one, but if you do, it is up to you to make
 * absoutely sure that it is guarandamnteed that you don't
 * call it with a bad overlap.  In fact, you should rename it
 * to one_net_memcpy and change all calls in your source code.
 *
 * dje: The previous version of this one had a (really) fatal bug:
 * It copied the destination buffer to source!
 *
 * Fixed and tested Novermber 5, 2010.
 * Dave Evans
 * Threshold Corporation
 *
 */
void *one_net_memmove(void *dst, const void *src, size_t n)
{
    UInt8 *d = dst;
    const UInt8 *s = src;

    if (!d || !s) { /* no recovery but at least don't crash */
        return NULL;
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
#endif //_USE_SAFE_MEMMOVE

SInt8 one_net_memcmp(const void *vp1, const void *vp2, size_t n)
{
    UInt8 * up1;
    UInt8 * up2;
    // If invalid, just return zero since there is no error recovery
    if (!vp1 || !vp2) {
        return 0;
    }
    for (up1 = vp1, up2 = vp2; n > 0; ++up1, ++up2, --n) {
        if (*up1 != *up2) {
            return ((*up1 < *up2) ? -1 : 1);
        }
    }
    return 0;
}


UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM)
{
    UInt16 val;
    
    if(!BYTE_STREAM)
    {
        EXIT();
    } // if parameters are invalid //
    
    val = (((UInt16)BYTE_STREAM[0]) << 8) & 0xFF00;
    val |= ((UInt16)BYTE_STREAM[1]) & 0x00FF;
    
    return val;
} // one_net_byte_stream_to_int16 //


void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream)
{
    if(!byte_stream)
    {
        EXIT();
    } // if parameters are invalid //
    
    byte_stream[0] = (UInt8)(VAL >> 8);
    byte_stream[1] = (UInt8)VAL;
} // one_net_int16_to_byte_stream //


UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM)
{
    UInt32 val;

    if(!BYTE_STREAM)
    {
        EXIT();
    } // if the parameter is invalid //
    
    val = (((UInt32)BYTE_STREAM[0]) << 24) & 0xFF000000;
    val |= (((UInt32)BYTE_STREAM[1]) << 16) & 0x00FF0000;
    val |= (((UInt32)BYTE_STREAM[2]) << 8) & 0x0000FF00;
    val |= ((UInt32)BYTE_STREAM[3]) & 0x000000FF;

    return val;
} // one_net_byte_stream_to_int32 //


void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream)
{
    if(!byte_stream)
    {
        EXIT();
    } // if parameters are invalid //

    byte_stream[0] = (UInt8)(VAL >> 24);
    byte_stream[1] = (UInt8)(VAL >> 16);
    byte_stream[2] = (UInt8)(VAL >> 8);
    byte_stream[3] = (UInt8)VAL;
} // one_net_int32_to_byte_stream //


tick_t one_net_tick(void)
{
    return get_tick_count();
} // one_net_tick //


tick_t one_net_ms_to_tick(const UInt32 MS)
{
    return MS_TO_TICK(MS);
} // one_net_ms_to_tick //


UInt32 one_net_tick_to_ms(const tick_t TICK)
{
    return TICK_TO_MS(TICK);
} // one_net_tick_to_ms //

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



