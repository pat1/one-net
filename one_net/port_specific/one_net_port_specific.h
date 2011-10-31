#ifndef _ONE_NET_PORT_SPECIFIC_H
#define _ONE_NET_PORT_SPECIFIC_H


//! \defgroup ONE-NET_port_specific Application Specific ONE-NET functionality
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
    \file one_net_port_specific.h
    \brief Application specific ONE-NET declarations.

    The implementer of an application must implement the interfaces in this 
    file for the ONE-NET project to compile (and work).  This file contains 
    interfaces for those functions used by both the MASTER and the CLIENT in
    the ONE-NET project.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


#include <stdlib.h>
#include "one_net_types.h"
#include "one_net_constants.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_specific_const
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_port_specific_typedefs
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_port_specific_pub_var
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_port_specific_pub_func
//! \ingroup ONE-NET_port_specific
//! @{

/*!
    \brief Copies LEN bytes from SRC to dst

    This function needs to be able to handle copying data when dst & SRC
    overlap.

    \param[out] dst The mem location to receive the bytes
    \param[in] SRC The bytes to copy.
    \param[in] LEN The number of bytes to copy.
    \return void
*/
void * one_net_memmove(void * dst, const void * SRC, size_t len);

/*!
    \brief Fills memory with a certain value.

    This function is just like memset from the string.h library except that
    I've changed the "value" parameter" from an int to a UInt8 to make it
    more clear that the function expects an unsigned character / byte and
    to avoid having to typecast.  I'm not sure why the real memset from
    string.h takes an int instead of an unsigned char, but since this is
    "one_net_memset", not "memset", I'll change it to a UInt8 exxplicitly.
    It won't hurt my feelings if people decide to change it back.

    \param[out] dst The mem location to receive the bytes
    \param[in] value The value of each byte in the memory
    \param[in] len The number of bytes to set.
    \return dst
*/
void * one_net_memset (void* dst, UInt8 value, size_t len);

/*!
    \brief Fills memory with one or more blocks of a value

    Sort of a hybrid between fread, memmove, and memset.  Allows you to set
    the memory of an array with a structure annd to do it more than once.
    The best way to explain is with an example...
    
    Suppose you had an array of 6 encoded Device IDs and you want to set that
    array so that all elements contained the address {0xB4, 0xBC};
    The following code would accomplish that...
    
    on_encoded_did_t c[6];
    on_encoded_did_t b = {0xB4, 0xBC};
    one_net_memset_block(c, sizeof(b), 6, b);
    
    
    After this code, c will contain
      {{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC}}
    
    
    \param[out] dst The mem location to receive the bytes
    \param[in] size Size of each element
    \param[in] count The number of elements to set
    \param[in] src The element to copy.
    \return dst
*/
void* one_net_memset_block(void* const dst, size_t size, size_t count,
  const void* const src);


/*!
    \brief Compares sequences of chars

    Compares sequences as unsigned chars

    \param[in] vp1  Pointer to the first  sequence
    \param[in] vp2  Pointer to the second sequence
    \param[in] n    The number of bytes to compare.
    \return -1 if the first  is smaller
            +1 if the second is smaller
             0 if they are equal
*/
SInt8 one_net_memcmp(const void *vp1, const void *vp2, size_t n);



/*!
    \brief Convert a byte stream to a 16-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 2 bytes to convert to a 16-bit value, accounting
      for the endianess of the processor.

    \return The 16 bit value contained in the 2 bytes of BYTE_STREAM
*/
UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM);


/*!
    \brief Convert a 16 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream);


/*!
    \brief Convert a byte stream to a 32-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 4 bytes to convert to a 32-bit value, accounting
      for the endianess of the processor.

    \return The 32 bit value contained in the 4 bytes of BYTE_STREAM
*/
UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM);


/*!
    \brief Convert a 32 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream);


/*!
    \brief Converts a raw did to a U16 value.
    
    \param[in] DID The device id to convert
    
    \return The UInt16 value corresponding to the DID.
*/
UInt16 did_to_u16(const on_raw_did_t *DID);


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
long int one_net_strtol(const char * str, char ** endptr, int base);



//! @} ONE-NET_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_port_specific

#endif // _ONE_NET_PORT_SPECIFIC_H //
