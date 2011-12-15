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




// Master/Client

#ifndef _ONE_NET_MASTER
	#define _ONE_NET_MASTER
#endif

#ifndef _ONE_NET_CLIENT
	#define _ONE_NET_CLIENT
#endif



// Peer Assignments.  Some applications need to implement peer assignments.  Some do not.
// Define _PEER if your application implements peer assignments.  Default is _PEER assigned
#ifndef _PEER
	#define _PEER
#endif

// Block Messages
#ifndef _BLOCK_MESSAGES_ENABLED
//	#define _BLOCK_MESSAGES_ENABLED
#endif

// Stream Messages -- available only if block messages are enabled.
#ifdef _BLOCK_MESSAGES_ENABLED
    #ifndef _STREAM_MESSAGES_ENABLED
//	    #define _STREAM_MESSAGES_ENABLED
    #endif
#endif

// Multi-Hop
#ifndef _ONE_NET_MULTI_HOP
	#define _ONE_NET_MULTI_HOP
#endif


#ifdef _ONE_NET_MULTI_HOP
	#ifndef _ONE_NET_MH_CLIENT_REPEATER
//		#define _ONE_NET_MH_CLIENT_REPEATER
	#endif
#endif


#ifdef _ONE_NET_MULTI_HOP
    #ifndef _RANGE_TESTING
        #define _RANGE_TESTING
    #endif
#endif



// ONE_NET_SIMPLE_DEVICE, _ONE_NET_SIMPLE_MASTER, and _ONE_NET_SIMPLE_CLIENT
// are now defined explicitly.
#if !defined(_BLOCK_MESSAGES_ENABLED) && !defined(_ONE_NET_MULTI_HOP)
    #ifndef _ONE_NET_SIMPLE_DEVICE
        // comment in or out as needed
//        #define _ONE_NET_SIMPLE_DEVICE
    #endif
#endif

#ifdef _ONE_NET_SIMPLE_DEVICE
    #ifdef _ONE_NET_MASTER
        #ifndef _ONE_NET_SIMPLE_MASTER
            // comment in or out as needed
            #define _ONE_NET_SIMPLE_MASTER
        #endif
    #endif
    #ifdef _ONE_NET_CLIENT
        #ifndef _ONE_NET_SIMPLE_CLIENT
            // comment in or out as needed
            #define _ONE_NET_SIMPLE_CLIENT
        #endif
    #endif
#endif




// Idle Option - Should be defined if the device can ever be idle
#ifndef _IDLE
    #define _IDLE
#endif


// Enhanced Invite Option - Should be defined if you need the option of specifying a
// timeout time or specifying a specific channel range for invitations.  Only valid
// if _IDLE is defined.
#ifdef _IDLE
    #ifndef _ENHANCED_INVITE
	    #define _ENHANCED_INVITE
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


// Evaluation Board Options

#ifndef _ONE_NET_EVAL
	#define _ONE_NET_EVAL
#endif



// Enter 0 for no printouts, 1 for minimal printouts, 2 for semi-detailed printouts,
// 3 for more detailed printouts, etc.  The higher the number, the
// more detailed the display will be.  This value must be set
// if using the sniffer, using the debugging tools, and can also be
// set if adding any of your own debugging statements.
#define _DEBUG_VERBOSE_LEVEL 6


#ifdef _ONE_NET_EVAL
	#ifndef _SERIAL_ASSIGN_DEMO_PINS
		#define _SERIAL_ASSIGN_DEMO_PINS
	#endif
	
	// _AUTO_MODE should be defined if you want the Auto Mode option available
	#ifndef _AUTO_MODE
		#define _AUTO_MODE
	#endif

	// _SNIFFER_MODE should be defined if you want the Sniffer Mode option available
    #if _DEBUG_VERBOSE_LEVEL > 0
	    #ifndef _SNIFFER_MODE
		    #define _SNIFFER_MODE
	    #endif
    #endif
#endif



// Other Options

#ifndef _CHIP_ENABLE
	#define _CHIP_ENABLE
#endif

#ifndef _R8C_TINY
	#define _R8C_TINY
#endif


// Enable this if the device has the ability to save to / load from
// non-volatile memory (i.e. Flash memory)
#ifndef _NON_VOLATILE_MEMORY
    #define _NON_VOLATILE_MEMORY
#endif


// Enable this if data rates can be changed to anything besides the 38,400 base.
#ifndef _DATA_RATE
    #define _DATA_RATE
#endif


// Command line interface
#ifndef _ENABLE_CLI
	#define _ENABLE_CLI
#endif

// #defines below are only relevant if _ENABLE_CLI is defined.  Each CLI option should have its
// own #define for maximum ease of enabling and disabling features.  CLI options that don't make
// sense without other CLI options should be nested.



// Note : Dec. 19, 2010 - Right now it appears to be unfeasible to not have a CLI at all as far as adapting code.
// I can, however, see many cases where someone might want to take some Eval Board code and modify it where
// there is no CLI.  However, at the present time there is a lot of functions with "oncli" return types that
// perhaps should not have "oncli" return types.  I think these should probably be changed for more versatility,
// but right now I am going to leave them intact.  Thus for Eval boards, even if you never use a CLI, you should
// define the _ENABLE_CLI option to get mit to compile.  Instead, I have created a new variable called
// _AT_LEAST_ONE_COMMAND_ENABLED, which can be defined or not defined.
#ifdef _ENABLE_CLI
	#ifndef _AT_LEAST_ONE_COMMAND_ENABLED
		#define _AT_LEAST_ONE_COMMAND_ENABLED
	#endif
#endif

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED

	// _ENABLE_SINGLE_COMMAND should be defined if you are implementing the "single" and "single text" command options
	#ifndef _ENABLE_SINGLE_COMMAND
		#define _ENABLE_SINGLE_COMMAND
	#endif

	// _ENABLE_SET_VALUE_COMMAND should be defined if you are implementing the "set value" command option
	#ifndef _ENABLE_SET_VALUE_COMMAND
		#define _ENABLE_SET_VALUE_COMMAND
	#endif

	// _ENABLE_SET_PIN_COMMAND should be defined if you are implementing the "set pin" command option
	#ifndef _ENABLE_SET_PIN_COMMAND
		#define _ENABLE_SET_PIN_COMMAND
	#endif

    #ifdef _BLOCK_MESSAGES_ENABLED
	    // _ENABLE_BLOCK_COMMAND should be defined if you are implementing the "block" command option
	    #ifndef _ENABLE_BLOCK_COMMAND
		    #define _ENABLE_BLOCK_COMMAND
	    #endif

	    // _ENABLE_BLOCK_TEXT_COMMAND should be defined if you are implementing the "block text" command option
	    #ifndef _ENABLE_BLOCK_TEXT_COMMAND
		    #define _ENABLE_BLOCK_TEXT_COMMAND
	    #endif
	#endif
    
    #ifdef _NON_VOLATILE_MEMORY
        // _ENABLE_ERASE_COMMAND should be defined if you are implementing the "erase" command option
        #ifndef _ENABLE_ERASE_COMMAND
            #define _ENABLE_ERASE_COMMAND
        #endif

        // _ENABLE_SAVE_COMMAND should be defined if you are implementing the "save" command option
        #ifndef _ENABLE_SAVE_COMMAND
	        #define _ENABLE_SAVE_COMMAND
        #endif
    #endif

	// _ENABLE_DUMP_COMMAND should be defined if you are implementing the "dump" command option
	#ifndef _ENABLE_DUMP_COMMAND
		#define _ENABLE_DUMP_COMMAND
	#endif

	// _ENABLE_RSINGLE_COMMAND should be defined if you are implementing the "rsingle" command option
	#ifdef _ENABLE_SINGLE_COMMAND
		#ifndef _ENABLE_RSINGLE_COMMAND
			#define _ENABLE_RSINGLE_COMMAND
		#endif
	#endif

	// _ENABLE_RSSI_COMMAND should be defined if you are implementing the "rssi" command option
	/*#ifndef _ENABLE_RSSI_COMMAND
		#define _ENABLE_RSSSI_COMMAND
	#endif*/

	// _ENABLE_LIST_COMMAND should be defined if you are implementing the "list" command option
	#ifndef _ENABLE_LIST_COMMAND
		#define _ENABLE_LIST_COMMAND
	#endif

    // Master Only Commands
    #ifdef _ONE_NET_MASTER
    
        // _ENABLE_CHANNEL_COMMAND should be defined if you are implementing the "channel" command option
        #ifndef _ENABLE_CHANNEL_COMMAND
            #define _ENABLE_CHANNEL_COMMAND
        #endif
        
	    // _ENABLE_INVITE_COMMAND should be defined if you are implementing the "invite" command option
	    #ifndef _ENABLE_INVITE_COMMAND
		    #define _ENABLE_INVITE_COMMAND
	    #endif

	    // _ENABLE_CANCEL_INVITE_COMMAND should be defined if you are implementing the "cancel invite" command option
	    #ifndef _ENABLE_CANCEL_INVITE_COMMAND
		    #define _ENABLE_CANCEL_INVITE_COMMAND
	    #endif

    	#ifdef _PEER
	    	// _ENABLE_ASSIGN_PEER_COMMAND should be defined if you are implementing the "assign peer" command option
		    #ifndef _ENABLE_ASSIGN_PEER_COMMAND
			    #define _ENABLE_ASSIGN_PEER_COMMAND
		    #endif

		    // _ENABLE_UNASSIGN_PEER_COMMAND should be defined if you are implementing the "unassign peer" command option
		    #ifndef _ENABLE_UNASSIGN_PEER_COMMAND
			    #define _ENABLE_UNASSIGN_PEER_COMMAND
		    #endif
	    #endif
		
	    // _ENABLE_CHANGE_KEY_COMMAND should be defined if you are implementing the "change key" command option
	    #ifndef _ENABLE_CHANGE_KEY_COMMAND
		    #define _ENABLE_CHANGE_KEY_COMMAND
	    #endif
		
        #ifdef _STREAM_MESSAGES_ENABLED
    	    // _ENABLE_CHANGE_KEY_COMMAND should be defined if you are implementing the "change stream key" command option
    	    #ifndef _ENABLE_CHANGE_STREAM_KEY_COMMAND
    		    #define _ENABLE_CHANGE_STREAM_KEY_COMMAND
    	    #endif
        #endif

	    // _ENABLE_REMOVE_DEVICE_COMMAND should be defined if you are implementing the "remove device" command option
	    #ifndef _ENABLE_REMOVE_DEVICE_COMMAND
		    #define _ENABLE_REMOVE_DEVICE_COMMAND
	    #endif

	    // _ENABLE_UPDATE_MASTER_COMMAND should be defined if you are implementing the "set update master flag" command option
	    #ifndef _ENABLE_UPDATE_MASTER_COMMAND
		    #define _ENABLE_UPDATE_MASTER_COMMAND
	    #endif

	    // _ENABLE_CHANGE_KEEP_ALIVE_COMMAND should be defined if you are implementing the "change keep-alive" command option
	    #ifndef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
		    #define _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
	    #endif

        #ifdef _BLOCK_MESSAGES_ENABLED
    	    // _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND should be defined if you are implementing the "change fragment delay" command option
    	    #ifndef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    		    #define _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    	    #endif
        #endif
	#endif

	// _ENABLE_GET_CHANNEL_COMMAND should be defined if you are implementing the "get channel" command option
	#ifndef _ENABLE_GET_CHANNEL_COMMAND
		#define _ENABLE_GET_CHANNEL_COMMAND
	#endif

	// _ENABLE_USER_PIN_COMMAND should be defined if you are implementing the "user pin" command option
	#ifndef _ENABLE_USER_PIN_COMMAND
		#define _ENABLE_USER_PIN_COMMAND
	#endif

	// _ENABLE_JOIN_COMMAND should be defined if you are implementing the "join" command option
	#ifndef _ENABLE_JOIN_COMMAND
		#define _ENABLE_JOIN_COMMAND
	#endif

	// _ENABLE_SETNI_COMMAND should be defined if you are implementing the "setni" command option
	#ifndef _ENABLE_SETNI_COMMAND
		#define _ENABLE_SETNI_COMMAND
	#endif

	// _ENABLE_SNIFF_COMMAND should be defined if you are implementing the "sniff" command option
	#ifdef _SNIFFER_MODE
		#ifndef _ENABLE_SNIFF_COMMAND
			#define _ENABLE_SNIFF_COMMAND
		#endif
	#endif

	// _ENABLE_MODE_COMMAND should be defined if you are implementing the "mode" command option
	#ifndef _ENABLE_MODE_COMMAND
		#define _ENABLE_MODE_COMMAND
	#endif

	// _ENABLE_ECHO_COMMAND should be defined if you are implementing the "echo" command option
	#ifndef _ENABLE_ECHO_COMMAND
		#define _ENABLE_ECHO_COMMAND
	#endif
    
    // _ENABLE_SET_DATA_RATE_COMMAND should be defined if you are implementing the "set data rate" command option
    #ifdef _DATA_RATE
        #ifndef _ENABLE_SET_DATA_RATE_COMMAND
            #define _ENABLE_SET_DATA_RATE_COMMAND
        #endif
    #endif
#endif
	



// Other Options

//#ifndef _NEED_XDUMP
//	#define _NEED_XDUMP
//#endif

//#ifndef _EVAL_0005_NO_REVISION
//	#define _EVAL_0005_NO_REVISION
//#endif


#ifndef _DEBUGGING_TOOLS
    #define _DEBUGGING_TOOLS
#endif



// SINGLE_QUEUE_LEVEL - different levels of options for a single queue
// NO_SINGLE_QUEUE_LEVEL means no single queue is used
// MIN_SINGLE_QUEUE_LEVEL means no "times".
// MED_SINGLE_QUEUE_LEVEL means send time, but no expire time
// MAX_SINGLE_QUEUE_LEVEL means both send and expire times
#ifndef _SINGLE_QUEUE_LEVEL
    #define NO_SINGLE_QUEUE_LEVEL 0
    #define MIN_SINGLE_QUEUE_LEVEL 1
	#define MED_SINGLE_QUEUE_LEVEL 2
	#define MAX_SINGLE_QUEUE_LEVEL 3
	
	#define _SINGLE_QUEUE_LEVEL MAX_SINGLE_QUEUE_LEVEL
#endif

#ifndef _EXTENDED_SINGLE
    #define _EXTENDED_SINGLE
#endif


// replace with named constants
#ifndef _ACK_NACK_LEVEL
    #define _ACK_NACK_LEVEL 3
#endif

// enable this when you want more ROMDATA or other meemory.
// Shortens strings to save memory, but the strings won't make
// much sense (i.e. "zo" instead of "STATUS QUERY".  Generally used for
// developers.  You need to look at the arrays in "oncli.c" and elsewhere
// to make sense of the shortens strings.  However, shortening the strings
// can allow you to use the ebugger and use other debugging tools.
#ifndef _MINIMIZE_STRING_LENGTHS
    #define _MINIMIZE_STRING_LENGTHS
#endif


// Enable this if the device has transmit and receive LEDS
#ifndef _HAS_LEDS
    #define _HAS_LEDS
#endif


// "Blocking" versus "Non-blocking" uart.
#ifndef _BLOCKING_UART
    #define _BLOCKING_UART
#endif


// Now test #defines for compatibility
#include "test_defines.h"



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
