#ifndef _ONE_NET_PORT_CONST_H
#define _ONE_NET_PORT_CONST_H

#include "config_options.h"
#include "one_net_data_rate.h"


//! \defgroup ONE-NET_port_const Application Specific ONE-NET constants.
//! \ingroup ONE-NET
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
    \file one_net_port_const.h
    \brief Application specific ONE-NET constants.

    These are constants that are specific to each ONE-NET device
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_const_const
//! \ingroup ONE-NET_port_const
//! @{


enum
{
    //! Time in ms to spend checking for reception of a packet
    //! (the PREAMBLE & SOF). 10ms
    ONE_NET_WAIT_FOR_SOF_TIME = 10
};


#ifdef _PEER
enum
{
    //! Size of the peer table
    ONE_NET_MAX_PEER_UNIT = 8,
    
    //! Number of peers than any one message / transaction can send to
    //! (not including the original recipient).
    ONE_NET_MAX_PEER_PER_TXN = 4
};
#endif


#ifdef _ONE_NET_MULTI_HOP
enum
{
    //! The maximum number of hops
    ON_MAX_HOPS_LIMIT = 7,
};
#endif


//! Timer related constants
enum
{
    //! Time in ms a device must wait in between checking if a channel is
    //! clear (5ms)
    ONE_NET_CLR_CHANNEL_TIME = 5,
    
    //! Time in ms a device waits for a response (50ms)
    ONE_NET_RESPONSE_TIME_OUT = 50,
    
    #ifdef _BLOCK_MESSAGES_ENABLED
    //! Base Fragment delay in ms for low priority transactions (125ms)
    ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY = 125,

    //! Base Fragment delay in ms for high priority transactions (25ms)
    ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY = 25,
    #endif
    
    #ifdef _ONE_NET_MULTI_HOP
    //! Multi-hop retpeater latency -- i.e. estimated time it takes for a
    //! repeater to forward a message in milliseconds
    ONE_NET_MH_LATENCY = 5
    #endif
};

//! The time before timeout of the invite process from beginning of the time
//! that the invite starts to be accepted until the time all information has
//! been passed between the master and the new client.
#define INVITE_TRANSACTION_TIMEOUT 5000


enum
{
    //! Number of pins on this device.  The Eval Board contains 4 switches
    //! so this value is 4.
    NUM_USER_PINS = 4,
    
    //! The number of different unit types this device supports.
    //! The Eval Board contains only switches, so this value is 1
    ONE_NET_NUM_UNIT_TYPES = 1,
    
    //! Number of units on this device.  The Eval Board contains 4 switches
    //! so this value is 4.
    ONE_NET_NUM_UNITS = NUM_USER_PINS
};


// uart buffer size
enum
{
    UART_RX_BUF_SIZE = 100,   //!< Size of the uart receive buffer
    UART_TX_BUF_SIZE = 100    //!< Size of the uart transmit buffer
};


enum
{
    SINGLE_DATA_QUEUE_SIZE = /*12*/3,
    SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE = /*100*/50
};


// data rates -- uncomment any data rates that this device handles.
// 38,400 must be enabled / uncommented
#ifndef DATA_RATE_38_4_CAPABLE
    #define DATA_RATE_38_4_CAPABLE
#endif
#ifndef DATA_RATE_76_8_CAPABLE
    #define DATA_RATE_76_8_CAPABLE
#endif
// 115,200 is supposed to be possible, but there is a bug somewhere.
// TODO - fix
#ifndef DATA_RATE_115_2_CAPABLE
//    #define DATA_RATE_115_2_CAPABLE
#endif
#ifndef DATA_RATE_153_6_CAPABLE
//    #define DATA_RATE_153_6_CAPABLE
#endif
#ifndef DATA_RATE_192_0_CAPABLE
//    #define DATA_RATE_192_0_CAPABLE
#endif
// 230,400 is supposed to be possible, but there is a bug somewhere.
// TODO - fix
#ifndef DATA_RATE_230_4_CAPABLE
//    #define DATA_RATE_230_4_CAPABLE
#endif



//! @} ONE-NET_port_const_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_port_const_typedefs
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_port_const_pub_var
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_port_const_pub_func
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_port_specific

#endif // _ONE_NET_PORT_CONST_H //
