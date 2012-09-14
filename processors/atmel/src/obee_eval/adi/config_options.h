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

#ifndef ONE_NET_MASTER
	#define ONE_NET_MASTER
#endif

#ifndef ONE_NET_CLIENT
	#define ONE_NET_CLIENT
#endif



// Peer Assignments.  Some applications need to implement peer assignments.  Some do not.
// Define PEER if your application implements peer assignments.  Default is PEER assigned
#ifndef PEER
	#define PEER
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

	#define _SINGLE_QUEUE_LEVEL MED_SINGLE_QUEUE_LEVEL
#endif


// Multi-Hop, Block, Extended Single, and Stream all require the master to
// update them when the master adds a new client.  They also require
// increased flexibility for timing options.  Therefore they should have the
// ability to stagger messages.  Therefore their "queue level" must be greater
// than MIN_SINGLE_QUEUE_LEVEL.
#if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
    // Extended Single -- enable if this device can handle "extended"(i.e. large payload) single messages
    #ifndef _EXTENDED_SINGLE
        #define _EXTENDED_SINGLE
    #endif

    // Multi-Hop
    #ifndef _ONE_NET_MULTI_HOP
	    #define _ONE_NET_MULTI_HOP
    #endif

    // sleeping devices cannot be repeaters.
    #if defined(ONE_NET_CLIENT) && defined(_ONE_NET_MULTI_HOP) && !defined(_DEVICE_SLEEPS)
	    #ifndef _ONE_NET_MH_CLIENT_REPEATER
		    #define _ONE_NET_MH_CLIENT_REPEATER
	    #endif
    #endif

    #ifdef _EXTENDED_SINGLE
        // define if this device handles routing
        #define _ROUTE
    #endif

    // Block Messages
    #ifdef _ROUTE
        #ifndef _BLOCK_MESSAGES_ENABLED
        	#define _BLOCK_MESSAGES_ENABLED
        #endif
    #endif

    #ifdef _BLOCK_MESSAGES_ENABLED
        // Stream Messages -- available only if block messages are enabled.
        #ifndef _STREAM_MESSAGES_ENABLED
        	#define _STREAM_MESSAGES_ENABLED
        #endif
    #endif

    #if defined(_BLOCK_MESSAGES_ENABLED) && defined(_ONE_NET_CLIENT)
        // Relevant only for clients initiating block / stream.  Enable if the
        // client has the ability to request permission from the master for
        // long block and stream transfers.  Do not enable if the device cannot
        // request permission from the client.
        #ifndef _BLOCK_STREAM_REQUEST_MASTER_PERMISSION
       	    #define _BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        #endif
    #endif
#endif


#ifndef _RANGE_TESTING
    #define _RANGE_TESTING
#endif

#ifndef _PID_BLOCK
    #define _PID_BLOCK
#endif


// Idle Option - Should be defined if the device can ever be idle
#ifndef _IDLE
    #define _IDLE
#endif


// Enhanced Invite Option - Should be defined if you need the option of specifying a
// timeout time or specifying a specific channel range for invitations.  Only valid
// if _IDLE is defined.
#if defined(_IDLE) && defined(_ONE_NET_CLIENT)
    #ifndef _ENHANCED_INVITE
	    #define _ENHANCED_INVITE
	#endif
#endif


// simple clients cannot be masters, queue messages for future sending, have extended single,
// block, stream, or multi-hop capability.  Some of this is mutually exclusive, so it's not
// needed to test.
#if _SINGLE_QUEUE_LEVEL <= MIN_SINGLE_QUEUE_LEVEL && !defined(_EXTENDED_SINGLE) && !defined(_ONE_NET_MULTI_HOP)
    #ifndef _ONE_NET_SIMPLE_CLIENT
        // comment in or out as needed.  Note.  Eval boards cannot be simple clients.
        //#define _ONE_NET_SIMPLE_CLIENT
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

#ifndef _UART
    // Enable this if there is UART
    #define _UART

    // define the base baud rate.  Define _DEFAULT_BAUD_RATE as 38400 or 115200.
    // If _DEFAULT_BAUD_RATE is not defined or id defined to an invalid option,
    // 38400 baud will be used.  The baud rate can also be changed with the "baud"
    // command-line option.  "baud:38400" or
    #ifndef _DEFAULT_BAUD_RATE
        #define _DEFAULT_BAUD_RATE 115200
    #endif
#endif

// Enter 0 for no printouts, 1 for minimal printouts, 2 for semi-detailed printouts,
// 3 for more detailed printouts, etc.  The higher the number, the
// more detailed the display will be.  This value must be positive
// if using the sniffer, using the debugging tools, and can also be
// set if adding any of your own debugging statements.
#ifdef _UART
    // You can change the value below.
    #define _DEBUG_VERBOSE_LEVEL 6
#else
    // DO NOT change the value below.
    #define _DEBUG_VERBOSE_LEVEL 0
#endif



#ifdef _ONE_NET_EVAL
	// _AUTO_MODE should be defined if you want the Auto Mode option available
	#ifndef _AUTO_MODE
//		#define _AUTO_MODE
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

//#ifndef _R8C_TINY
//	#define _R8C_TINY
//#endif

#ifndef _ATXMEGA256A3B
  #define _ATXMEGA256A3B
#endif

//#ifndef _8_MHZ_CLOCK
//  #define _8_MHZ_CLOCK
//#endif

#ifndef _110592_MHZ_CLOCK             // adi eval board
//  #define  _110592_MHZ_CLOCK
#endif

#ifndef _221184_MHZ_CLOCK             // one net eval board
  #define  _221184_MHZ_CLOCK
#endif


#ifdef _ATXMEGA256A3B
   // define eval board or OBE board (when the following _ATXMEGA256A3B_EVAL is not defined)
    #ifndef _ATXMEGA256A3B_EVAL
//     #define _ATXMEGA256A3B_EVAL
	 // when using the emulator uncomment the following, if not using the emulator comment out the following
//	 #define _ATXMEGA256A3B_EVAL_USER_PIN
   #endif
#endif

// Enable this if the device has the ability to save to / load from
// non-volatile memory (i.e. Flash memory)
#ifndef _NON_VOLATILE_MEMORY
    #define _NON_VOLATILE_MEMORY
#endif


// Enable this if data rates can be changed to anything besides the 38,400 base
// or the channel can be changed back and forth at run-time for anything but the
// invite process.
#ifndef _DATA_RATE_CHANNEL
    #define _DATA_RATE_CHANNEL
#endif


#ifdef _UART
    // "Blocking" versus "Non-blocking" uart.
    #ifndef _BLOCKING_UART
        #define _BLOCKING_UART
    #endif

    // Command line interface
    #ifndef _ENABLE_CLI
        #define _ENABLE_CLI
    #endif
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

        // _ENABLE_SETNI_COMMAND should be defined if you are implementing the "setni" command option
        #ifndef _ENABLE_SETNI_COMMAND
            #define _ENABLE_SETNI_COMMAND
        #endif

        // _AUTO_SAVE should be defined if the parameters should be saved every
        // time they change
        #ifndef _AUTO_SAVE
           // #define _AUTO_SAVE
        #endif
    #endif

	// _ENABLE_DUMP_COMMAND should be defined if you are implementing the "dump" command option
	#ifndef _ENABLE_DUMP_COMMAND
	//	#define _ENABLE_DUMP_COMMAND
	#endif

	// _ENABLE_RSINGLE_COMMAND should be defined if you are implementing the "rsingle" command option
	#ifdef _ENABLE_SINGLE_COMMAND
		#ifndef _ENABLE_RSINGLE_COMMAND
	//		#define _ENABLE_RSINGLE_COMMAND
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
    #ifdef ONE_NET_MASTER

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

    	#ifdef PEER
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

	    // _ENABLE_REMOVE_DEVICE_COMMAND should be defined if you are implementing the "remove device" command option
	    #ifndef _ENABLE_REMOVE_DEVICE_COMMAND
		    #define _ENABLE_REMOVE_DEVICE_COMMAND
	    #endif

	    // _ENABLE_SET_FLAGS_COMMAND should be defined if you are implementing the "set flags" command option
	    #ifndef _ENABLE_SET_FLAGS_COMMAND
		    #define _ENABLE_SET_FLAGS_COMMAND
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
	//	#define _ENABLE_GET_CHANNEL_COMMAND
	#endif

	// _ENABLE_USER_PIN_COMMAND should be defined if you are implementing the "user pin" command option
	#ifndef _ENABLE_USER_PIN_COMMAND
		#define _ENABLE_USER_PIN_COMMAND
	#endif

	// _ENABLE_JOIN_COMMAND should be defined if you are implementing the "join" command option
    #ifdef _ONE_NET_CLIENT
        #ifndef _ENABLE_JOIN_COMMAND
		    #define _ENABLE_JOIN_COMMAND
	    #endif
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

    // _ENABLE_SET_DR_CHANNEL_COMMAND should be defined if you are implementing the "set dr_channel" command option
    #ifdef _DATA_RATE_CHANNEL
        #ifndef _ENABLE_SET_DR_CHANNEL_COMMAND
        //    #define _ENABLE_SET_DR_CHANNEL_COMMAND
        #endif
    #endif

    // _ENABLE_ROUTE_COMMAND should be defined if you are implementing the "route" command option
    #ifdef _ROUTE
        #ifndef _ENABLE_ROUTE_COMMAND
            #define _ENABLE_ROUTE_COMMAND
        #endif
    #endif

    #ifdef _UART
        // _ENABLE_BAUD_COMMAND should be enabled if you are implementing the "baud" command
        #ifndef _ENABLE_BAUD_COMMAND
            #define _ENABLE_BAUD_COMMAND
        #endif

        // _ENABLE_VERBOSE_LEVEL_COMMAND should be enabled if you are implementing
        // the "verbose level" command
        #ifndef _ENABLE_VERBOSE_LEVEL_COMMAND
            #define _ENABLE_VERBOSE_LEVEL_COMMAND
        #endif
    #endif
#endif




// Other Options


//#ifndef _EVAL_0005_NO_REVISION
//	#define _EVAL_0005_NO_REVISION
//#endif


#ifndef _DEBUGGING_TOOLS
    #define _DEBUGGING_TOOLS
#endif


// enable this when you want more ROMDATA or other meemory.
// Shortens strings to save memory, but the strings won't make
// much sense (i.e. "zo" instead of "STATUS QUERY".  Generally used for
// developers.  You need to look at the arrays in "oncli.c" and elsewhere
// to make sense of the shortens strings.  However, shortening the strings
// can allow you to use the ebugger and use other debugging tools.

// TODO  --  shorten some strings in oncli_str.c to save memory / code space.
#ifndef _MINIMIZE_STRING_LENGTHS
 //   #define _MINIMIZE_STRING_LENGTHS
#endif


// Enable this if the device has transmit and receive LEDS
#ifndef _HAS_LEDS
//    #define _HAS_LEDS
#endif

// Enable _ONE_NET_MEMORY is you are implementing the ONE-NET versions of
// malloc and free.  If _ONE_NET_MEMORY is enabled, you must define
// ONE_NET_HEAP_SIZE and ONE_NET_HEAP_NUM_ENTRIES in
// one_net_port_const.h.
#ifndef _ONE_NET_MEMORY
//    #define _ONE_NET_MEMORY
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

// Now test #defines for compatibility
#include "test_defines.h"




// these aren't configuration options but rather aliases to avoid multiple
// identical functions.
#define one_net_client_handle_single_pkt eval_handle_single
#define one_net_master_handle_single_pkt eval_handle_single
#define one_net_client_handle_block_pkt eval_handle_block
#define one_net_master_handle_block_pkt eval_handle_block
#define one_net_client_handle_stream_pkt eval_handle_stream
#define one_net_master_handle_stream_pkt eval_handle_stream
#define one_net_client_block_chunk_received eval_block_chunk_received
#define one_net_master_block_chunk_received eval_block_chunk_received
#define one_net_client_handle_ack_nack_response eval_handle_ack_nack_response
#define one_net_master_handle_ack_nack_response eval_handle_ack_nack_response
#define one_net_client_single_txn_status eval_single_txn_status
#define one_net_master_single_txn_status eval_single_txn_status
#define one_net_client_block_txn_status eval_bs_txn_status
#define one_net_master_block_txn_status eval_bs_txn_status
#define one_net_client_stream_txn_status eval_bs_txn_status
#define one_net_master_stream_txn_status eval_bs_txn_status
#define one_net_client_handle_bs_ack_nack_response eval_handle_bs_ack_nack_response
#define one_net_master_handle_bs_ack_nack_response eval_handle_bs_ack_nack_response



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
