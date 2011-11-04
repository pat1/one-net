#ifndef _TAL_H
#define _TAL_H


#include "config_options.h"
#include "one_net_types.h"
#include "one_net_status_codes.h"


//! \defgroup TAL Transceiver Abstraction Layer
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
    \file tal.h (for ADI)
    \brief Defines the transceiver specific layer for ONE-NET code.

    You must create a transceiver specific version of this file for each
    transceiver.  The tal.h file in each transceiver directory defines the
    interface that should be used to access all transceiver specific
    functionality. 
*/

//==============================================================================
//                                  CONSTANTS
//! \defgroup TAL_const
//! \ingroup TAL
//! @{



//! @} TAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TAL_type_defs
//! \ingroup TAL
//! @{


#define TAL_INIT_TRANSCEIVER() tal_init_transceiver()
#define one_net_channel_is_clear() tal_channel_is_clear()
#define one_net_write(X, Y) tal_write_packet(X, Y)
#define one_net_write_done() tal_write_packet_done()
#define one_net_read(X) tal_read_bytes(X)
#define one_net_look_for_pkt(X) tal_look_for_packet(X)
#define one_net_set_data_rate(X) tal_set_data_rate(X)
#define one_net_set_channel(X) tal_set_channel(X)



//! @} TAL_type_defs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup TAL_pub_var
//! \ingroup TAL
//! @{


//! The current ONE-NET channel
extern UInt8 current_channel; // current_channel should be declared in
                              // the transceiver .c file.


//! @} TAL_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup TAL_pub_func
//! \ingroup TAL
//! @{






/*!
    \brief Initialize the transceiver. 

    This function should perform any one of a kind transceiver initialization.
    There are other functions for turning on the transmitter and receiver.
    However, any initialization that does not need to be repeated when turning
    on the transmitter or recevier should be done in this function so that less
    time is needed to turn on the transmitter or receiver.

    \param void

    \return void
*/
void tal_init_transceiver(void);


/*!
    \brief Enables the transceiver.

    \param void

    \return void
*/
void tal_enable_transceiver(void);


/*!
    \brief Disables the transceiver

    \param void

    \return void
*/
void tal_disable_transceiver(void);


/*!
    \brief Checks the channel to see if it is clear.

    This function performs the Carrier Sense.  It is called before a device
    transmits.

    \param void

    \return TRUE if the channel is clear
            FALSE if the channel is currently in use.
*/
BOOL tal_channel_is_clear(void);


/*!
    \brief Sends bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[in] data An array of bytes to be sent out of the rf interface
    \param[in] len The number of bytes to send

    \return The number of bytes sent.
*/
UInt16 tal_write_packet(const UInt8 * data, const UInt16 len);


/*!
    \brief Returns TRUE if writing the data out of the rf channel is complete.

    \param void

    \return TRUE If the device is done writing the data out of the rf channel.
            FALSE If the device is still writing the data out of the rf channel.
*/
BOOL tal_write_packet_done(void);


/*!
    \brief Reads bytes out from the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[out] data Byte array to store the receive data in.
    \param[in] len The number of bytes to receive (data is at least this long).
    \return The number of bytes read
*/
UInt16 tal_read_bytes(UInt8 * data, const UInt16 len);


/*!
    \brief Waits a specified number of ticks for receiption of a packet
    
    \param[in] duration Time in ticks to look for a packet before giving up

    \return ONS_SUCCESS if a packet has been received.
*/
one_net_status_t tal_look_for_packet(tick_t duration);


/*!
    \brief Changes the data rate the device is operating at.

    The ONE-NET code does not keep track if it is changing the data rate to a
    rate that is already set.  It is up to the implementer to check this.

    \param[in] data_rate The data rate to set the transceiver to. See
      data_rate_t for values.

    \return ONS_SUCCESS if data rate was successfully changed, error otherwise
*/
one_net_status_t tal_set_data_rate(UInt8 data_rate);


/*!
    \brief Changes the channel the device is on

    \param[in] channel The channel to change to.  This is one of the values
      in one_net_channel_t.

    \return ONS_SUCCEESS if channel was set successfully, error message otherwise
*/
one_net_status_t tal_set_channel(const UInt8 channel);



//! @} TAL_pub_func
//!                         PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} TAL

#endif // _TAL_H //
