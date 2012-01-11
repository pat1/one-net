#ifndef _ONE_NET_CONFIG_OPTIONS_H
#define _ONE_NET_CONFIG_OPTIONS_H

//! \defgroup one_net_config_options Place configuration options here
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
    \file config_options.h
    \brief Place any configuration options you want in this file.

    Place any configuration options you want in this file.  Leave it
	empty if there are no configuration options.

*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_config_options_const
//! \ingroup one_net_config_options
//! @{



// If you do not want to use the config_options.h file to implment the #define
// values (i.e. you are using an IDE and would rather define things there),
// make sure you define a #define variable called _OVERRIDE_CONFIG_OPTIONS_FILE.



#ifndef _OVERRIDE_CONFIG_OPTIONS_FILE


// _OVERRIDE_CONFIG_OPTIONS_FILE is not defined.  Use the code belo to define things.



// First undefine everything to be extra careful

#include "undefine_all_defines.h"





// Now add any new configuration options you need.  Comment out any you do not need.  #ifdef
// guards aren't needed since we undefined everything above, but can't hurt so we'll leave them
// in.



#ifndef _QUAD_OUTPUT
    #define _QUAD_OUTPUT
#endif



// Master/Client
#ifndef _ONE_NET_MASTER
//	#define _ONE_NET_MASTER
#endif

#ifndef _ONE_NET_CLIENT
	#define _ONE_NET_CLIENT
#endif



// Peer Assignments.  Some applications need to implement peer assignments.  Some do not.
// Define _PEER if your application implements peer assignments.  Default is _PEER assigned
#ifndef _PEER
	#define _PEER
#endif

#ifndef _EXTENDED_SINGLE
//    #define _EXTENDED_SINGLE
#endif

// Extended Single must be enabled if Block messages is enabled.
// Block Messages
#ifdef _EXTENDED_SINGLE
    #ifndef _BLOCK_MESSAGES_ENABLED
    //	#define _BLOCK_MESSAGES_ENABLED
    #endif
#endif

// Stream Messages -- available only if block messages are enabled.
#ifdef _BLOCK_MESSAGES_ENABLED
    #ifndef _STREAM_MESSAGES_ENABLED
//	    #define _STREAM_MESSAGES_ENABLED
    #endif
#endif

// Multi-Hop
#ifndef _ONE_NET_MULTI_HOP
//	#define _ONE_NET_MULTI_HOP
#endif

#ifdef _ONE_NET_MULTI_HOP
	#ifndef _ONE_NET_MH_CLIENT_REPEATER
//		#define _ONE_NET_MH_CLIENT_REPEATER
	#endif
#endif

#ifndef _RANGE_TESTING
//    #define _RANGE_TESTING
#endif

// variable data rates are only available if block messages are enabled
#ifdef _BLOCK_MESSAGES_ENABLED
    // Enable this if data rates can be changed to anything besides the 38,400 base.
    #ifndef _DATA_RATE
    //    #define _DATA_RATE
    #endif
#endif


// SINGLE_QUEUE_LEVEL - different levels of options for a single queue
// NO_SINGLE_QUEUE_LEVEL means no single queue is used
// MIN_SINGLE_QUEUE_LEVEL means no "times".
// MED_SINGLE_QUEUE_LEVEL means send time, but no expire time
// MAX_SINGLE_QUEUE_LEVEL means both send and expire times
#ifndef _SINGLE_QUEUE_LEVEL
    #define NO_SINGLE_QUEUE_LEVEL 0
    #define MIN_SINGLE_QUEUE_LEVEL NO_SINGLE_QUEUE_LEVEL+1
	#define MED_SINGLE_QUEUE_LEVEL MIN_SINGLE_QUEUE_LEVEL+1
	#define MAX_SINGLE_QUEUE_LEVEL MED_SINGLE_QUEUE_LEVEL+1
	
	#define _SINGLE_QUEUE_LEVEL NO_SINGLE_QUEUE_LEVEL
#endif

// simple clients cannot be masters, queue messages for future sending, have extended single,
// block, stream, or multi-hop capability.  Some of this is mutually exclusive, so it's not
// needed to test.
#if _SINGLE_QUEUE_LEVEL <= MIN_SINGLE_QUEUE_LEVEL && !defined(_EXTENDED_SINGLE) && !defined(_ONE_NET_MULTI_HOP)
    #ifndef _ONE_NET_SIMPLE_CLIENT
        // comment in or out as needed
        #define _ONE_NET_SIMPLE_CLIENT
    #endif
#endif


// Idle Option - Should be defined if the device can ever be idle
#ifndef _IDLE
//  #define _IDLE
#endif


// Enhanced Invite Option - Should be defined if you need the option of specifying a
// timeout time or specifying a specific channel range for invitations.  Only valid
// if _IDLE is defined.
#if defined(_IDLE) && defined(_ONE_NET_CLIENT)
    #ifndef _ENHANCED_INVITE
//   #define _ENHANCED_INVITE
	#endif
#endif


// Locale for channels (Europe or U.S.A.).  At least one locale must be defined.  You can
// define more than one.
#ifndef _US_CHANNELS
	#define _US_CHANNELS
#endif

#ifndef _EUROPE_CHANNELS
	#define _EUROPE_CHANNELS
#endif



// Other Options
#ifndef _R8C_TINY
	#define _R8C_TINY
#endif


// Enable this if the device has the ability to save to / load from
// non-volatile memory (i.e. Flash memory)
#ifndef _NON_VOLATILE_MEMORY
    #define _NON_VOLATILE_MEMORY
#endif


// Enable this if the device has transmit and receive LEDS
#ifndef _HAS_LEDS
    #define _HAS_LEDS
#endif


// Use this feature to override any random channel searching and select a
// particular channel.  See one_net_channel.h.  Selecting this option will
// override channel setting in the transcevier.  Comment out the
// "#define _CHANNEL_OVERIDE" line for normal behavior.
// behavior.
#ifndef _CHANNEL_OVERRIDE
//    #define _CHANNEL_OVERIDE
    #ifdef _CHANNEL_OVERRIDE
        // overriding with US Channel 2.  See one_net_channel.h for options
        #define CHANNEL_OVERRIDE_CHANNEL ONE_NET_US_CHANNEL_2
    #endif
#endif


#define _ACK_NACK_LEVEL 3


// Now test #defines for compatibility
#include "test_defines.h"




// these aren't configuration options but rather aliases to avoid multiple
// identical functions.
#define one_net_client_handle_single_pkt eval_handle_single
#define one_net_master_handle_single_pkt eval_handle_single
#define one_net_client_handle_ack_nack_response eval_handle_ack_nack_response
#define one_net_master_handle_ack_nack_response eval_handle_ack_nack_response
#define one_net_client_single_txn_status eval_single_txn_status
#define one_net_master_single_txn_status eval_single_txn_status




//! @} one_net_config_options_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_config_options_typedefs
//! \ingroup one_net_config_options
//! @{

//! @} one_net_config_options_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup one_net_config_options_pub_var
//! \ingroup one_net_config_options
//! @{

//! @} one_net_config_options_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_config_options_pub_func
//! \ingroup one_net_config_options
//! @{


//! @} one_net_config_options_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_config_options


#endif // _OVERRIDE_CONFIG_OPTIONS_FILE //
#endif // _ONE_NET_CONFIG_OPTIONS_H //
