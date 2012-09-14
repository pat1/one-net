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
#ifndef SINGLE_QUEUE_LEVEL
    #define NO_SINGLE_QUEUE_LEVEL 0
    #define MIN_SINGLE_QUEUE_LEVEL NO_SINGLE_QUEUE_LEVEL+1
	#define MED_SINGLE_QUEUE_LEVEL MIN_SINGLE_QUEUE_LEVEL+1
	#define MAX_SINGLE_QUEUE_LEVEL MED_SINGLE_QUEUE_LEVEL+1

	#define SINGLE_QUEUE_LEVEL MED_SINGLE_QUEUE_LEVEL
#endif


// Multi-Hop, Block, Extended Single, and Stream all require the master to
// update them when the master adds a new client.  They also require
// increased flexibility for timing options.  Therefore they should have the
// ability to stagger messages.  Therefore their "queue level" must be greater
// than MIN_SINGLE_QUEUE_LEVEL.
#if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
    // Extended Single -- enable if this device can handle "extended"(i.e. large payload) single messages
    #ifndef EXTENDED_SINGLE
        #define EXTENDED_SINGLE
    #endif

    // Multi-Hop
    #ifndef ONE_NET_MULTI_HOP
	    #define ONE_NET_MULTI_HOP
    #endif

    // sleeping devices cannot be repeaters.
    #if defined(ONE_NET_CLIENT) && defined(ONE_NET_MULTI_HOP) && !defined(DEVICE_SLEEPS)
	    #ifndef ONE_NET_MH_CLIENT_REPEATER
		    #define ONE_NET_MH_CLIENT_REPEATER
	    #endif
    #endif

    #ifdef EXTENDED_SINGLE
        // define if this device handles routing
        #define ROUTE
    #endif

    // Block Messages
    #ifdef ROUTE
        #ifndef BLOCK_MESSAGES_ENABLED
        	#define BLOCK_MESSAGES_ENABLED
        #endif
    #endif

    #ifdef BLOCK_MESSAGES_ENABLED
        // Stream Messages -- available only if block messages are enabled.
        #ifndef STREAM_MESSAGES_ENABLED
        	#define STREAM_MESSAGES_ENABLED
        #endif
    #endif

    #if defined(BLOCK_MESSAGES_ENABLED) && defined(DEVICE_SLEEPS)
        // Relevant only for clients initiating block / stream.  Enable if the
        // client has the ability to request permission from the master for
        // long block and stream transfers.  Do not enable if the device cannot
        // request permission from the client.
        #ifndef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
       	    #define BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        #endif
    #endif
#endif


#ifndef RANGE_TESTING
    #define RANGE_TESTING
#endif

#ifndef PID_BLOCK
    #define PID_BLOCK
#endif


// Idle Option - Should be defined if the device can ever be idle
#ifndef IDLE
    #define IDLE
#endif


// Enhanced Invite Option - Should be defined if you need the option of specifying a
// timeout time or specifying a specific channel range for invitations.  Only valid
// if IDLE is defined.
#if defined(IDLE) && defined(DEVICE_SLEEPS)
    #ifndef ENHANCED_INVITE
	    #define ENHANCED_INVITE
	#endif
#endif


// simple clients cannot be masters, queue messages for future sending, have extended single,
// block, stream, or multi-hop capability.  Some of this is mutually exclusive, so it's not
// needed to test.
#if SINGLE_QUEUE_LEVEL <= MIN_SINGLE_QUEUE_LEVEL && !defined(EXTENDED_SINGLE) && !defined(ONE_NET_MULTI_HOP)
    #ifndef ONE_NET_SIMPLE_CLIENT
        // comment in or out as needed.  Note.  Eval boards cannot be simple clients.
        //#define ONE_NET_SIMPLE_CLIENT
    #endif
#endif


// Locale for channels (Europe or U.S.A.).  At least one locale must be defined.  You can
// define more than one.
#ifndef US_CHANNELS
	#define US_CHANNELS
#endif

#ifndef EUROPE_CHANNELS
	#define EUROPE_CHANNELS
#endif


// Evaluation Board Options

#ifndef ONE_NET_EVAL
	#define ONE_NET_EVAL
#endif

#ifndef UART
    // Enable this if there is UART
    #define UART

    // define the base baud rate.  Define DEFAULT_BAUD_RATE as 38400 or 115200.
    // If DEFAULT_BAUD_RATE is not defined or id defined to an invalid option,
    // 38400 baud will be used.  The baud rate can also be changed with the "baud"
    // command-line option.  "baud:38400" or
    #ifndef DEFAULT_BAUD_RATE
        #define DEFAULT_BAUD_RATE 115200
    #endif
#endif

// Enter 0 for no printouts, 1 for minimal printouts, 2 for semi-detailed printouts,
// 3 for more detailed printouts, etc.  The higher the number, the
// more detailed the display will be.  This value must be positive
// if using the sniffer, using the debugging tools, and can also be
// set if adding any of your own debugging statements.
#ifdef UART
    // You can change the value below.
    #define DEBUG_VERBOSE_LEVEL 6
#else
    // DO NOT change the value below.
    #define DEBUG_VERBOSE_LEVEL 0
#endif



#ifdef ONE_NET_EVAL
	// AUTO_MODE should be defined if you want the Auto Mode option available
	#ifndef AUTO_MODE
//		#define AUTO_MODE
	#endif

	// SNIFFER_MODE should be defined if you want the Sniffer Mode option available
    #if DEBUG_VERBOSE_LEVEL > 0
	    #ifndef SNIFFER_MODE
		    #define SNIFFER_MODE
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
#ifndef NON_VOLATILE_MEMORY
    #define NON_VOLATILE_MEMORY
#endif


// Enable this if data rates can be changed to anything besides the 38,400 base
// or the channel can be changed back and forth at run-time for anything but the
// invite process.
#ifndef DATA_RATE_CHANNEL
    #define DATA_RATE_CHANNEL
#endif


#ifdef UART
    // "Blocking" versus "Non-blocking" uart.
    #ifndef BLOCKING_UART
        #define BLOCKING_UART
    #endif

    // Command line interface
    #ifndef ENABLE_CLI
        #define ENABLE_CLI
    #endif
#endif



// #defines below are only relevant if ENABLE_CLI is defined.  Each CLI option should have its
// own #define for maximum ease of enabling and disabling features.  CLI options that don't make
// sense without other CLI options should be nested.



// Note : Dec. 19, 2010 - Right now it appears to be unfeasible to not have a CLI at all as far as adapting code.
// I can, however, see many cases where someone might want to take some Eval Board code and modify it where
// there is no CLI.  However, at the present time there is a lot of functions with "oncli" return types that
// perhaps should not have "oncli" return types.  I think these should probably be changed for more versatility,
// but right now I am going to leave them intact.  Thus for Eval boards, even if you never use a CLI, you should
// define the ENABLE_CLI option to get mit to compile.  Instead, I have created a new variable called
// AT_LEAST_ONE_COMMAND_ENABLED, which can be defined or not defined.
#ifdef ENABLE_CLI
	#ifndef AT_LEAST_ONE_COMMAND_ENABLED
		#define AT_LEAST_ONE_COMMAND_ENABLED
	#endif
#endif

#ifdef AT_LEAST_ONE_COMMAND_ENABLED

	// ENABLE_SINGLE_COMMAND should be defined if you are implementing the "single" and "single text" command options
	#ifndef ENABLE_SINGLE_COMMAND
		#define ENABLE_SINGLE_COMMAND
	#endif

    #ifdef BLOCK_MESSAGES_ENABLED
	    // ENABLE_BLOCK_COMMAND should be defined if you are implementing the "block" command option
	    #ifndef ENABLE_BLOCK_COMMAND
		    #define ENABLE_BLOCK_COMMAND
	    #endif

	    // ENABLE_BLOCK_TEXT_COMMAND should be defined if you are implementing the "block text" command option
	    #ifndef ENABLE_BLOCK_TEXT_COMMAND
		    #define ENABLE_BLOCK_TEXT_COMMAND
	    #endif
	#endif

    #ifdef NON_VOLATILE_MEMORY
        // ENABLE_ERASE_COMMAND should be defined if you are implementing the "erase" command option
        #ifndef ENABLE_ERASE_COMMAND
            #define ENABLE_ERASE_COMMAND
        #endif

        // ENABLE_SAVE_COMMAND should be defined if you are implementing the "save" command option
        #ifndef ENABLE_SAVE_COMMAND
	        #define ENABLE_SAVE_COMMAND
        #endif

        // ENABLE_SETNI_COMMAND should be defined if you are implementing the "setni" command option
        #ifndef ENABLE_SETNI_COMMAND
            #define ENABLE_SETNI_COMMAND
        #endif

        // AUTO_SAVE should be defined if the parameters should be saved every
        // time they change
        #ifndef AUTO_SAVE
           // #define AUTO_SAVE
        #endif
    #endif

	// ENABLE_DUMP_COMMAND should be defined if you are implementing the "dump" command option
	#ifndef ENABLE_DUMP_COMMAND
	//	#define ENABLE_DUMP_COMMAND
	#endif

	// ENABLE_RSINGLE_COMMAND should be defined if you are implementing the "rsingle" command option
	#ifdef ENABLE_SINGLE_COMMAND
		#ifndef ENABLE_RSINGLE_COMMAND
	//		#define ENABLE_RSINGLE_COMMAND
		#endif
	#endif

	// ENABLE_RSSI_COMMAND should be defined if you are implementing the "rssi" command option
	/*#ifndef ENABLE_RSSI_COMMAND
		#define ENABLE_RSSI_COMMAND
	#endif*/

	// ENABLE_LIST_COMMAND should be defined if you are implementing the "list" command option
	#ifndef ENABLE_LIST_COMMAND
		#define ENABLE_LIST_COMMAND
	#endif

    // Master Only Commands
    #ifdef ONE_NET_MASTER

        // ENABLE_CHANNEL_COMMAND should be defined if you are implementing the "channel" command option
        #ifndef ENABLE_CHANNEL_COMMAND
            #define ENABLE_CHANNEL_COMMAND
        #endif

	    // ENABLE_INVITE_COMMAND should be defined if you are implementing the "invite" command option
	    #ifndef ENABLE_INVITE_COMMAND
		    #define ENABLE_INVITE_COMMAND
	    #endif

	    // ENABLE_CANCEL_INVITE_COMMAND should be defined if you are implementing the "cancel invite" command option
	    #ifndef ENABLE_CANCEL_INVITE_COMMAND
		    #define ENABLE_CANCEL_INVITE_COMMAND
	    #endif

    	#ifdef PEER
	    	// ENABLE_ASSIGN_PEER_COMMAND should be defined if you are implementing the "assign peer" command option
		    #ifndef ENABLE_ASSIGN_PEER_COMMAND
			    #define ENABLE_ASSIGN_PEER_COMMAND
		    #endif

		    // ENABLE_UNASSIGN_PEER_COMMAND should be defined if you are implementing the "unassign peer" command option
		    #ifndef ENABLE_UNASSIGN_PEER_COMMAND
			    #define ENABLE_UNASSIGN_PEER_COMMAND
		    #endif
	    #endif

	    // ENABLE_CHANGE_KEY_COMMAND should be defined if you are implementing the "change key" command option
	    #ifndef ENABLE_CHANGE_KEY_COMMAND
		    #define ENABLE_CHANGE_KEY_COMMAND
	    #endif

	    // ENABLE_REMOVE_DEVICE_COMMAND should be defined if you are implementing the "remove device" command option
	    #ifndef ENABLE_REMOVE_DEVICE_COMMAND
		    #define ENABLE_REMOVE_DEVICE_COMMAND
	    #endif

	    // ENABLE_SET_FLAGS_COMMAND should be defined if you are implementing the "set flags" command option
	    #ifndef ENABLE_SET_FLAGS_COMMAND
		    #define ENABLE_SET_FLAGS_COMMAND
	    #endif

	    // ENABLE_CHANGE_KEEP_ALIVE_COMMAND should be defined if you are implementing the "change keep-alive" command option
	    #ifndef ENABLE_CHANGE_KEEP_ALIVE_COMMAND
		    #define ENABLE_CHANGE_KEEP_ALIVE_COMMAND
	    #endif

        #ifdef BLOCK_MESSAGES_ENABLED
    	    // ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND should be defined if you are implementing the "change fragment delay" command option
    	    #ifndef ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    		    #define ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    	    #endif
        #endif
	#endif

	// ENABLE_GET_CHANNEL_COMMAND should be defined if you are implementing the "get channel" command option
	#ifndef ENABLE_GET_CHANNEL_COMMAND
	//	#define ENABLE_GET_CHANNEL_COMMAND
	#endif

	// ENABLE_USER_PIN_COMMAND should be defined if you are implementing the "user pin" command option
	#ifndef ENABLE_USER_PIN_COMMAND
		#define ENABLE_USER_PIN_COMMAND
	#endif

	// ENABLE_JOIN_COMMAND should be defined if you are implementing the "join" command option
    #ifdef DEVICE_SLEEPS
        #ifndef ENABLE_JOIN_COMMAND
		    #define ENABLE_JOIN_COMMAND
	    #endif
    #endif

	// ENABLE_SNIFF_COMMAND should be defined if you are implementing the "sniff" command option
	#ifdef SNIFFER_MODE
		#ifndef ENABLE_SNIFF_COMMAND
			#define ENABLE_SNIFF_COMMAND
		#endif
	#endif

	// ENABLE_MODE_COMMAND should be defined if you are implementing the "mode" command option
	#ifndef ENABLE_MODE_COMMAND
		#define ENABLE_MODE_COMMAND
	#endif

	// ENABLE_ECHO_COMMAND should be defined if you are implementing the "echo" command option
	#ifndef ENABLE_ECHO_COMMAND
		#define ENABLE_ECHO_COMMAND
	#endif

    // ENABLE_SET_DR_CHANNEL_COMMAND should be defined if you are implementing the "set dr_channel" command option
    #ifdef DATA_RATE_CHANNEL
        #ifndef ENABLE_SET_DR_CHANNEL_COMMAND
        //    #define ENABLE_SET_DR_CHANNEL_COMMAND
        #endif
    #endif

    // ENABLE_ROUTE_COMMAND should be defined if you are implementing the "route" command option
    #ifdef ROUTE
        #ifndef ENABLE_ROUTE_COMMAND
            #define ENABLE_ROUTE_COMMAND
        #endif
    #endif

    #ifdef UART
        // ENABLE_BAUD_COMMAND should be enabled if you are implementing the "baud" command
        #ifndef ENABLE_BAUD_COMMAND
            #define ENABLE_BAUD_COMMAND
        #endif

        // ENABLE_VERBOSE_LEVEL_COMMAND should be enabled if you are implementing
        // the "verbose level" command
        #ifndef ENABLE_VERBOSE_LEVEL_COMMAND
            #define ENABLE_VERBOSE_LEVEL_COMMAND
        #endif
    #endif
#endif




// Other Options


//#ifndef EVAL_0005_NO_REVISION
//	#define EVAL_0005_NO_REVISION
//#endif


#ifndef DEBUGGING_TOOLS
    #define DEBUGGING_TOOLS
#endif


// enable this when you want more ROMDATA or other memory.
// Shortens strings to save memory, but the strings won't make
// much sense (i.e. "zo" instead of "STATUS QUERY".  Generally used for
// developers.  You need to look at the arrays in "oncli.c" and elsewhere
// to make sense of the shortens strings.  However, shortening the strings
// can allow you to use the debugger and use other debugging tools.

// TODO  --  shorten some strings in oncli_str.c to save memory / code space.
#ifndef MINIMIZE_STRING_LENGTHS
 //   #define MINIMIZE_STRING_LENGTHS
#endif


// Enable this if the device has transmit and receive LEDS
#ifndef HAS_LEDS
//    #define HAS_LEDS
#endif

// Enable ONE_NET_MEMORY is you are implementing the ONE-NET versions of
// malloc and free.  If ONE_NET_MEMORY is enabled, you must define
// ONE_NET_HEAP_SIZE and ONE_NET_HEAP_NUM_ENTRIES in
// one_net_port_const.h.
#ifndef ONE_NET_MEMORY
//    #define ONE_NET_MEMORY
#endif



// Use this feature to override any random channel searching and select a
// particular channel.  See one_net_channel.h.  Selecting this option will
// override channel setting in the transceiver.  Comment out the
// "#define _CHANNEL_OVERIDE" line for normal behavior.
// behavior.
#ifndef CHANNEL_OVERRIDE
//    #define _CHANNEL_OVERIDE
    #ifdef CHANNEL_OVERRIDE
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
