#ifndef _ONE_NET_PORT_CONST_H
#define _ONE_NET_PORT_CONST_H

#include "config_options.h"

#ifdef _ONE_NET_CLIENT
#include "one_net_client_port_const.h" // for the _DEVICE_SLEEPS constant, if
                                       // defined.
#endif


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
    
  
//! The amount of time a device that sleeps is expected to stay awake when there
//! is "more to do" (in ms).  This needs to be defined for masters and for any
//! client that sleeps.
#define DEVICE_SLEEP_STAY_AWAKE_TIME 3000


enum
{
    //! Time in ms to spend checking for reception of a packet
    //! (the PREAMBLE & SOF). 10ms
    ONE_NET_WAIT_FOR_SOF_TIME = 10
};


enum
{
    //! Maximum number of recipient for any one message.
    ONE_NET_MAX_RECIPIENTS = 6
};


#ifdef _PEER
enum
{
    //! Size of the peer table
    ONE_NET_MAX_PEER_UNIT = 8,
    
    #ifdef _ONE_NET_CLIENT
    // subtract one for the actual recipient and another for the master
    // in case we need to send to it and it isn't already on the list
    ONE_NET_MAX_PEER_PER_TXN = ONE_NET_MAX_RECIPIENTS - 2
    #else
    // subtract one for the actual recipient
    ONE_NET_MAX_PEER_PER_TXN = ONE_NET_MAX_RECIPIENTS - 1
    #endif
};
#endif


#ifdef _ONE_NET_MULTI_HOP
enum
{
    //! The maximum number of hops
    ON_MAX_HOPS_LIMIT = 7,
};
#endif


#ifdef _RANGE_TESTING
enum
{
    //! When Multi-Hop range testing (i.e. declaring devices in and out of
    //! range when they actually are physically in range, the maximum
    //! number of in-range devices.
    RANGE_TESTING_ARRAY_SIZE = 5
};
#endif


#ifdef _PID_BLOCK
enum
{
    //! For debugging purposes.  The maximum number of PIDs that can be blocked
    PID_BLOCK_ARRAY_SIZE = 5
};
#endif


// Note : When experimenting with multi-hop, try changing ONE_NET_MH_LATENCY.
// If this value is too small, there may be collisions on the repeat.  If
// you increase this value, be aware that you may also need to increase
// ONE_NET_RESPONSE_TIME_OUT.

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
//! been passed between the master and the new client.  In ms.
#define INVITE_TRANSACTION_TIMEOUT 10000


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


#ifdef _ONE_NET_MEMORY
// see one_net_memory.h
enum
{
    // Size of the "heap".  one_net_memory provices a poor-man's "heap" for
    // embedded systems when you want to use a heap but can't or don't want
    // to use malloc, calloc, realloc, free from stdlib.h.
    ONE_NET_HEAP_SIZE = 100,
    
    ONE_NET_HEAP_NUM_ENTRIES = 5
};
#endif


// data rates -- uncomment any data rates that this device handles.
// 38,400 must be enabled / uncommented
#ifndef DATA_RATE_38_4_CAPABLE
    #define DATA_RATE_38_4_CAPABLE
#endif

// TODO -- Derek_S 3/19/2012 -- 76,800 seems to be partially capable,
// but drops far too many messages, particularly long messages of 52
// bytes or more, where the drop rate is above 50% in rapid messages,
// which is not nearly good enough.  For shorter messages of 30 bytes, the
// failure rate is lower, but still quite high (i.e. more messages require
// retries than do not, compared to the 38,400 rate, where the vast majority
// of the time, even long messages are not garbled.  Hence I am disabling the
// 76,800 data rate for now.
#ifndef DATA_RATE_76_8_CAPABLE
//    #define DATA_RATE_76_8_CAPABLE
#endif
// 115,200 is supposed to be possible, but there is a bug somewhere.  It seems
// to work 0% of the time.
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
// 230,400 is supposed to be possible, but there is a bug somewhere.  This bug
// actually causes the program to crash.
// TODO - fix
#ifndef DATA_RATE_230_4_CAPABLE
//    #define DATA_RATE_230_4_CAPABLE
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
//! Default chunk size for block / stream transfers.  Must be between 1 and
//! 40, inclusive.
#define DEFAULT_BS_CHUNK_SIZE 40

//! Default chunk delay for block / stream.
#define DEFAULT_BS_CHUNK_DELAY 100
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
