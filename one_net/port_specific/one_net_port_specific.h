#ifndef _ONE_NET_PORT_SPECIFIC_H
#define _ONE_NET_PORT_SPECIFIC_H

#include "config_options.h"


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

#include "one_net_port_const.h"

#include "one_net.h"
#include "one_net_status_codes.h"
#include "one_net_types.h"


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
    \brief Returns the number of ticks since bootup.
    
    \param void

    \return The numer of ticks since bootup
*/
tick_t one_net_tick(void);


/*!
    \brief Converts milliseconds to ticks
    
    \param[in] MS The number of milliseconds to convert
    
    \return The number of ticks for the given ms value
*/
tick_t one_net_ms_to_tick(const UInt32 MS);


/*!
    \brief Converts ticks to milliseconds
    
    \param[in] TICK The number of ticks to convert
    
    \return The number of milliseconds for the given tick value
*/
UInt32 one_net_tick_to_ms(const tick_t TICK);


/*!
    \brief Changes the channel the device is on

    \param[in] CHANNEL The channel to change to.  This is one of the values
      in on_channel_t.

    \return void
*/
void one_net_set_channel(const UInt8 CHANNEL);


/*!
    \brief Checks the channel to see if it is clear.

    This function performs the Carrier Sense.  It is called before a device
    transmits.

    \param void

    \return TRUE if the channel is clear
            FALSE if the channel is currently in use.
*/
BOOL one_net_channel_is_clear(void);


/*!
    \brief Changes the data rate the device is operating at.

    The ONE-NET code does not keep track if it is changing the data rate to a
    rate that is already set.  It is up to the implementer to check this.

    \param[in] DATA_RATE The data rate to set the transceiver to. See
      data_rate_t for values.

    \return void
*/
void one_net_set_data_rate(const UInt8 DATA_RATE);


/*!
    \brief Waits a specified number of ticks for receiption of a packet
    
    \param[in] DURATION Time in ticks to look for a packet

    \return SUCCESS if a packet has been received
*/
one_net_status_t one_net_look_for_pkt(const tick_t DURATION);


/*!
    \brief Reads bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[out] data Byte array to store the receive data in.
    \param[in] LEN The number of bytes to receive (data is at least this long).
    \return The number of bytes read
*/
UInt16 one_net_read(UInt8 * data, const UInt16 LEN);


/*!
    \brief Sends bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[in] DATA An array of bytes to be sent out of the rf interface
    \param[in] LEN The number of bytes to send

    \return The number of bytes sent.
*/
UInt16 one_net_write(const UInt8 * DATA, const UInt16 LEN);


/*!
    \brief Returns TRUE if writing the data out of the rf channel is complete.

    \param void

    \return TRUE If the device is done writing the data out of the rf channel.
            FALSE If the device is still writing the data out of the rf channel.
*/
BOOL one_net_write_done(void);

//! @} ONE-NET_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_port_specific

#endif // _ONE_NET_PORT_SPECIFIC_H //
