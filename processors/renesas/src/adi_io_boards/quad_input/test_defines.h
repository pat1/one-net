//! \defgroup one_net_test_defines Test configuration options here
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
    \file test_defines.h
    \brief Test configuration definitions here for compatibility.

    Test configuration definitions here for compatibility.

*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_test_defines_const
//! \ingroup one_net_test_defines
//! @{


//! @} one_net_test_defines_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_test_defines_typedefs
//! \ingroup one_net_test_defines


#if defined(BLOCK_MESSAGES_ENABLED) && !defined(EXTENDED_SINGLE)
    #error "EXTENDED_SINGLE must be defined if BLOCK_MESSAGES_ENABLED is defined."
#endif

#if !defined(BLOCK_MESSAGES_ENABLED) && defined(STREAM_MESSAGES_ENABLED)
    #error "EXTENDED_SINGLE must be defined if BLOCK_MESSAGES_ENABLED is defined."
#endif

#if !defined(ONE_NET_CLIENT) && !defined(ONE_NET_MASTER)
    #error "ONE_NET_CLIENT and ONE_NET_MASTER cannot both be undefined."
#endif

#ifdef ONE_NET_MASTER
    #if SINGLE_QUEUE_LEVEL < MED_SINGLE_QUEUE_LEVEL
        #error "Masters must have queue levels of at least MED_SINGLE_QUEUE_LEVEL."
    #endif
#endif

#ifdef ONE_NET_MH_CLIENT_REPEATER
    #ifndef ONE_NET_MULTI_HOP
        #error "ONE_NET_MULTI_HOP must be defined if ONE_NET_MH_CLIENT_REPEATER is defined."
    #endif
#endif

#if !defined(US_CHANNELS) && !defined(EUROPE_CHANNELS)
    #error "US_CHANELS and EUROPE_CHANNELS cannot both be undefined."
#endif

#ifdef BLOCKING_UART
    #ifndef UART
        #error "UART must be defined if BLOCKING_UART is defined."
    #endif
#endif

// Simple clients cannot be masters, queue messages for future sending, have extended single,
// block, stream, or multi-hop capability.  Some of this is mutually exclusive, so it's not
// needed to test.  However, for easier readability, we'll test even some of the redundant tests.
#ifdef ONE_NET_SIMPLE_CLIENT
    #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
        #error "Simple clients cannot have SINGLE_QUEUE_LEVEL greater than MIN_SINGLE_QUEUE_LEVEL."
    #endif
    #ifdef ONE_NET_MASTER
        #error "Simple clients cannot have ONE_NET_MASTER defined."
    #endif
    #ifdef EXTENDED_SINGLE
        #error "Simple clients cannot have EXTENDED_SINGLE defined."
    #endif
    #ifdef ONE_NET_MULTI_HOP
        #error "Simple clients cannot have ONE_NET_MULTI_HOP defined."
    #endif
    #ifdef BLOCK_MESSAGES_ENABLED
        #error "Simple clients cannot have BLOCK_MESSAGES_ENABLED defined."
    #endif
    #ifdef STREAM_MESSAGES_ENABLED
        #error "Simple clients cannot have STREAM_MESSAGES_ENABLED defined."
    #endif
    #ifdef DATA_RATE
        #error "Simple clients cannot have DATA_RATE defined."
    #endif
    #ifndef ONE_NET_CLIENT
        #error "Simple clients must have ONE_NET_CLIENT defined."
    #endif
#endif

#ifdef ONE_NET_CLIENT
    #include "one_net_client_port_const.h"
    #ifndef ONE_NET_CLIENT_INVITE_DURATION
        #error "ONE_NET_CLIENT_INVITE_DURATION is not defined"
    #endif
#endif

#if defined(ONE_NET_MH_CLIENT_REPEATER) && defined(DEVICE_SLEEPS)
    #error "ONE_NET_MH_CLIENT_REPEATER and DEVICE_SLEEPS cannot both be defined."
#endif


//! @{

//! @} one_net_test_defines_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup one_net_test_defines_pub_var
//! \ingroup one_net_test_defines
//! @{

//! @} one_net_test_defines_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_test_defines_pub_func
//! \ingroup one_net_test_defines
//! @{


//! @} one_net_test_defines_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_test_defines

