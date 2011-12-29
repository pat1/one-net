//! \defgroup one_net_test_defines Test configuration options here
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
    \file test_defines.h
    \brief Test configuration definitions here for compatibility.

    Test configuration definitions here for compatibility.

*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_test_defines_const
//! \ingroup one_net_test_defines
//! @{



// Tests below check for any incompatible #define values.
// TO-DO : Add more tests.  Some of this should probably go in the port_specific
// directories.  You can always override any tests by placing a file called
// test_defines.h earlier in the include path, which has the effect of over-riding
// these tests.


// First test the version.
#ifdef _ONE_NET_VERSION_1_X
	#ifdef _ONE_NET_VERSION_2_X
		#error "ERROR : _ONE_NET_VERSION_1_X and _ONE_NET_VERSION_2_X are both defined. Exactly one should be defined.  Please adjust the #define values in the config_options.h file."
	#endif
#elif !defined(_ONE_NET_VERSION_2_X)
	#error "ERROR : Neither _ONE_NET_VERSION_1_X nor _ONE_NET_VERSION_2_X are defined. Exactly one should be defined.  Please adjust the #define values in the config_options.h file."
#endif


// Now make sure that either master or client is defined
#if !defined(_ONE_NET_MASTER) && !defined(_ONE_NET_CLIENT)
	#error "ERROR : Neither _ONE_NET_MASTER nor _ONE_NET_CLIENT are defined. At least one must be defined.  Please adjust the #define values in the config_options.h file."
#endif


// Test channels.  At least one locale must be defined.
#if !defined(_US_CHANNELS) && !defined(_EUROPE_CHANNELS)
	#error "ERROR : At least one locale must be defined.  Both _US_CHANNELS and _EUROPE_CHANNELS are currently undefined.  Please adjust the #define values in the config_options.h file."
#endif


// Now make sure that _ONE_NET_SIMPLE_CLIENT is properly defined
#if defined(_ONE_NET_SIMPLE_CLIENT) && !defined(_ONE_NET_CLIENT)
	#error "ERROR : _ONE_NET_CLIENT must be defined if _ONE_NET_SIMPLE_CLIENT is defined.  Please adjust the #define values in the config_options.h file."
#endif

#if defined(_ONE_NET_SIMPLE_CLIENT) && defined(_BLOCK_MESSAGES_ENABLED)
	#error "ERROR : _ONE_NET_SIMPLE_CLIENT and _BLOCK_MESSAGES_ENABLED cannot both be defined.  Please adjust the #define values in the config_options.h file."
#endif	

#if defined(_ONE_NET_SIMPLE_CLIENT) && defined(_STREAM_MESSAGES_ENABLED)
	#error "ERROR : _ONE_NET_SIMPLE_CLIENT and _STREAM_MESSAGES_ENABLED cannot both be defined.  Please adjust the #define values in the config_options.h file."
#endif	

#if defined(_ONE_NET_SIMPLE_CLIENT) && defined(_ONE_NET_MULTI_HOP)
	#error "ERROR : _ONE_NET_SIMPLE_CLIENT and _ONE_NET_MULTI_HOP cannot both be defined.  Please adjust the #define values in the config_options.h file."
#endif

#if !defined(_BLOCK_MESSAGES_ENABLED) && !defined(_STREAM_MESSAGES_ENABLED) && !defined(_ONE_NET_MULTI_HOP)
	#if defined(_ONE_NET_CLIENT) && !defined(_ONE_NET_SIMPLE_CLIENT)
		#error "ERROR : This device is a client device that does not implement block messages, stream messages, or multi-hop.  Therefore _ONE_NET_SIMPLE_CLIENT must be defined.   Please adjust the #define values in the config_options.h file."
	#endif
#endif


// Check polling option
#if defined(_POLL) && !defined(_ONE_NET_VERSION_2_X)
	#error "ERROR : _ONE_NET_VERSION_2_X is not defined and _POLL is defined.  Polling is only available in ONE-NET Version 2.0 and higher.  Please adjust the #define values in the config_options.h file."
#endif


// More multi-hop testing

#if defined(_ONE_NET_MH_CLIENT_REPEATER) && !defined(_ONE_NET_MULTI_HOP)
    #error "Need to define _ONE_NET_MULTI_HOP if _ONE_NET_MH_CLIENT_REPEATER is defined!"
#endif


// Block/Stream Tests

// Right now it appears that either both Stream and Block should be enabled or neither should be enabled
// I'm not positive this is true, but I'm going on that assumption for now.

#if defined(_ONE_NET_BLOCK_MESSAGES_ENABLED) && !defined(_ONE_NET_STREAM_MESSAGES_ENABLED)
    #error "ERROR : _ONE_NET_BLOCK_MESSAGES_ENABLED and _ONE_NET_STREAM_MESSAGES_ENABLED should either both be defined or both should be undefined.  Please adjust the #define values in the config_options.h file."
#endif

#if !defined(_ONE_NET_BLOCK_MESSAGES_ENABLED) && defined(_ONE_NET_STREAM_MESSAGES_ENABLED)
    #error "ERROR : _ONE_NET_BLOCK_MESSAGES_ENABLED and _ONE_NET_STREAM_MESSAGES_ENABLED should either both be defined or both should be undefined.  Please adjust the #define values in the config_options.h file."
#endif



// I/O Board
#ifdef _QUAD_OUTPUT
	#ifdef _ONE_NET_MASTER
		#error "ERROR : _QUAD_OUTPUT and _ONE_NET_MASTER should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifdef _DUAL_OUTPUT
		#error "ERROR : _QUAD_OUTPUT and _DUAL_OUTPUT should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifdef _QUAD_INPUT
		#error "ERROR : _QUAD_OUTPUT and _QUAD_INPUT should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifndef _ONE_NET_SIMPLE_CLIENT
		#error "ERROR : _ONE_NET_SIMPLE_CLIENT must be defined if _QUAD_OUTPUT is defined.  Please adjust the #define values in the config_options.h file."
	#endif
#endif

#ifdef _DUAL_OUTPUT
	#ifdef _ONE_NET_MASTER
		#error "ERROR : _DUAL_OUTPUT and _ONE_NET_MASTER should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifdef _QUAD_INPUT
		#error "ERROR : _DUAL_OUTPUT and _QUAD_INPUT should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifndef _ONE_NET_SIMPLE_CLIENT
		#error "ERROR : _ONE_NET_SIMPLE_CLIENT must be defined if _DUAL_OUTPUT is defined.  Please adjust the #define values in the config_options.h file."
	#endif
#endif	

#ifdef _QUAD_INPUT
	#ifdef _ONE_NET_MASTER
		#error "ERROR : _QUAD_INPUT and _ONE_NET_MASTER should not both be defined.  Please adjust the #define values in the config_options.h file."
	#endif
	#ifndef _ONE_NET_SIMPLE_CLIENT
		#error "ERROR : _ONE_NET_SIMPLE_CLIENT must be defined if _QUAD_INPUT is defined.  Please adjust the #define values in the config_options.h file."
	#endif
#endif


// test command compatibility
#ifdef _ENABLE_SET_PIN_COMMAND
	#ifndef _NEED_SWITCH_MESSAGE
		#error "ERROR : _NEED_SWITCH_MESSAGE must be defined if _ENABLE_SET_PIN_COMMAND is defined.  Please adjust the #define values in the config_options.h file."
    #endif
#endif



//! @} one_net_test_defines_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_test_defines_typedefs
//! \ingroup one_net_test_defines
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

