#ifndef _ONE_NET_PORT_CONST_H
#define _ONE_NET_PORT_CONST_H

#include "config_options.h"

// Test channels.  At least one locale must be defined.
#if !defined(_US_CHANNELS) && !defined(_EUROPE_CHANNELS)
	#error "ERROR : At least one locale must be defined.  Both _US_CHANNELS and _EUROPE_CHANNELS are currently undefined.  Please adjust the #define values in the config_options.h file."
#endif


//! \defgroup ONE-NET_port_const Application Specific ONE-NET constants.
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
    \file one_net_port_const.h
    \brief Application specific ONE-NET constants.

    These are constants that are specific to each ONE-NET device.  This file
    should be copied to a project specific location and renamed to
    one_net_port_const.h.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net.h"
#include "tick.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_const_const
//! \ingroup ONE-NET_port_const
//! @{

enum
{
    //! Time in ticks to spend polling for reception of a packet (the PREAMBLE
    //! & SOF). 10ms
    ONE_NET_WAIT_FOR_SOF_TIME = MS_TO_TICK(10),

    //! The maximum data rate this device can operate at.
    ONE_NET_MAX_DATA_RATE = ONE_NET_DATA_RATE_38_4
};

//! Timer related constants
enum
{
    //! Time in ticks a device must wait in between checking if a channel is
    //! clear (5ms)
    ONE_NET_CLR_CHANNEL_TIME = MS_TO_TICK(5),

    //! Time in ticks a device waits for a response (50ms)
    ONE_NET_RESPONSE_TIME_OUT = MS_TO_TICK(50),

    //! The base time in ticks for the retransmit delay for low priority
    //! transactions (10ms)
    ONE_NET_RETRANSMIT_LOW_PRIORITY_TIME = MS_TO_TICK(10),

    //! The base time in ticks for the retransmit delay for high prioritty
    //! transactions (2ms)
    ONE_NET_RETRANSMIT_HIGH_PRIORITY_TIME = MS_TO_TICK(2),

    //! Base Fragment delay in ticks for low priority transactions (125ms)
    ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY = MS_TO_TICK(125),

    //! Base Fragment delay in ticks for high priority transactions (25ms)
    ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY = MS_TO_TICK(25),

    //! Time in ticks to stay awake after receiving a Single Data ACK Stay
    //! Awake Packet (2s)
    ONE_NET_STAY_AWAKE_TIME = MS_TO_TICK(2000)
};

//! The frequencies supported by ONE-NET (USA & European)  These need to be 0
//! based without any gaps.  Whole groups (US & European) need to be included.
typedef enum
{
#ifdef _US_CHANNELS
    // US frequencies
    ONE_NET_MIN_US_CHANNEL,                             //!< Min US frequency
    ONE_NET_US_CHANNEL_1 = ONE_NET_MIN_US_CHANNEL,      //!< 903.0Mhz
    ONE_NET_US_CHANNEL_2,                               //!< 904.0Mhz
    ONE_NET_US_CHANNEL_3,                               //!< 905.0Mhz
    ONE_NET_US_CHANNEL_4,                               //!< 906.0Mhz
    ONE_NET_US_CHANNEL_5,                               //!< 907.0Mhz
    ONE_NET_US_CHANNEL_6,                               //!< 908.0Mhz
    ONE_NET_US_CHANNEL_7,                               //!< 909.0Mhz
    ONE_NET_US_CHANNEL_8,                               //!< 910.0Mhz
    ONE_NET_US_CHANNEL_9,                               //!< 911.0Mhz
    ONE_NET_US_CHANNEL_10,                              //!< 912.0Mhz
    ONE_NET_US_CHANNEL_11,                              //!< 913.0Mhz
    ONE_NET_US_CHANNEL_12,                              //!< 914.0Mhz
    ONE_NET_US_CHANNEL_13,                              //!< 915.0Mhz
    ONE_NET_US_CHANNEL_14,                              //!< 916.0Mhz
    ONE_NET_US_CHANNEL_15,                              //!< 917.0Mhz
    ONE_NET_US_CHANNEL_16,                              //!< 918.0Mhz
    ONE_NET_US_CHANNEL_17,                              //!< 919.0Mhz
    ONE_NET_US_CHANNEL_18,                              //!< 920.0Mhz
    ONE_NET_US_CHANNEL_19,                              //!< 921.0Mhz
    ONE_NET_US_CHANNEL_20,                              //!< 922.0Mhz
    ONE_NET_US_CHANNEL_21,                              //!< 923.0Mhz
    ONE_NET_US_CHANNEL_22,                              //!< 924.0Mhz
    ONE_NET_US_CHANNEL_23,                              //!< 925.0Mhz
    ONE_NET_US_CHANNEL_24,                              //!< 926.0Mhz
    ONE_NET_US_CHANNEL_25,                              //!< 927.0Mhz
    ONE_NET_MAX_US_CHANNEL = ONE_NET_US_CHANNEL_25,     //!< Max US frequency
#endif
#ifdef _EUROPE_CHANNELS    
    // European frequencies
    ONE_NET_MIN_EUR_CHANNEL,                            //!< Min European freq.
    ONE_NET_EUR_CHANNEL_1 = ONE_NET_MIN_EUR_CHANNEL,    //!< 865.8Mhz
    ONE_NET_EUR_CHANNEL_2,                              //!< 866.5Mhz
    ONE_NET_EUR_CHANNEL_3,                              //!< 867.2Mhz
    ONE_NET_MAX_EUR_CHANNEL = ONE_NET_EUR_CHANNEL_3,    //!< Max European freq.
#endif

    ONE_NET_NUM_CHANNELS,                               //!< Number of channels
    ONE_NET_MAX_CHANNEL = ONE_NET_NUM_CHANNELS - 1      //!< Max ONE-NET channel
} one_net_channel_t;

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

