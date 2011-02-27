#ifndef _ONE_NET_CONFIG_OPTIONS_H
#define _ONE_NET_CONFIG_OPTIONS_H

//! \defgroup one_net_config_options Place configuration options here
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

// Version Information

// Either _ONE_NET_VERSION_1_X or _ONE_NET_VERSION_2_X should be defined, but not both.  If
// you are using a version of ONE-NET lower than 2.0, _ONE_NET_VERSION_1_X should be defined
// and _ONE_NET_VERSION_2_X should not be defined.  If you are using version 2.0 or higher,
// _ONE_NET_VERSION_2_X should be defined and _ONE_NET_VERSION_1_X should not be defined.

#ifndef _ONE_NET_VERSION_1_X
	#define _ONE_NET_VERSION_1_X
#endif

#ifndef _ONE_NET_VERSION_2_X
//	#define _ONE_NET_VERSION_2_X
#endif


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
	#define _BLOCK_MESSAGES_ENABLED
#endif

// Stream Messages
#ifndef _STREAM_MESSAGES_ENABLED
	#define _STREAM_MESSAGES_ENABLED
#endif


// Multi-Hop - only available if _PEER defined
#ifdef _PEER
    #ifndef _ONE_NET_MULTI_HOP
	    #define _ONE_NET_MULTI_HOP
    #endif
#endif

#ifdef _ONE_NET_MULTI_HOP
	#ifndef _ONE_NET_MH_CLIENT_REPEATER
//		#define _ONE_NET_MH_CLIENT_REPEATER
	#endif
#endif



// ONE_NET_SIMPLE_DEVICE - a device is a simple device if it is does not
// implement any of the following: Multi-Hop, Stream, Block
#if defined(_STREAM_MESSAGES_ENABLED) || defined(_BLOCK_MESSAGES_ENABLED) || defined(_ONE_NET_MULTI_HOP)
    #ifdef _ONE_NET_SIMPLE_DEVICE
        #undef _ONE_NET_SIMPLE_DEVICE
	#endif
#else
    #ifndef _ONE_NET_SIMPLE_DEVICE
        #define _ONE_NET_SIMPLE_DEVICE
	#endif
#endif

#ifdef _ONE_NET_SIMPLE_DEVICE
	#ifdef _ONE_NET_CLIENT
		#ifndef _ONE_NET_SIMPLE_CLIENT
			#define _ONE_NET_SIMPLE_CLIENT
		#endif	
	#endif
	#ifdef _ONE_NET_MASTER
		#ifndef _ONE_NET_SIMPLE_MASTER
			#define _ONE_NET_SIMPLE_MASTER
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
//	    #define _ENHANCED_INVITE
	#endif
#endif

// load/dump options - commented out for now
#ifdef _IDLE
    #ifndef _ONE_NET_LOAD
        #define _ONE_NET_LOAD
	#endif
	
    #ifndef _ONE_NET_DUMP
        #define _ONE_NET_DUMP
	#endif
#endif


// Encryption, Encoding, and Random Padding of unused packet portions for
// increased security

// Encryption.  All implementations of ONE-NET must use encryption, but for debugging and
// learning purposes, it may be useful to turn encryption on and off.  Comment the three
// lines below out if not using encryption.  we already have a variable called
// ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE, which makes it so encryption is not used, but
// that may be checked somewhere and cause an error, so I've created a new variable
// below.  Again, encryption and encoding should only be turned off for debugging/development
// purposes.  Actual products implementing ONE-NET must be encoded and encrypted.
#ifndef _ONE_NET_USE_ENCRYPTION
	#define _ONE_NET_USE_ENCRYPTION
#endif


// Encoding.  All implementations of ONE-NET must use encoding, but for debugging and
// learning purposes, it may be useful to turn encoding on and off.  Comment the three
// lines below out if not using encoding.  Note : Not using encoding WILL NOT affect packet
// sizes.  There will still be a 6 bit to 8 bit encoding transformation and an 8 bit to
// 6 bit decoding transformation.  However, when not using encoding , 0 will map to 0,
// 1 will map to 1, 2 will map to 2, etc.  If using encoding, 0 will map to 0xB4, 1 will
// map to 0xBC, 2 will to 0xB3, etc.
#ifndef _ONE_NET_USE_ENCODING
	#define _ONE_NET_USE_ENCODING
#endif

// Random Padding.  If defined, unused portions of encrypted packets will be randomly
// generated. If not turned on, unused portions of encrypted packets will be either 0
// or "undefined" ("undefined" means that they may be zeroed out, they may be left as-is
// i.e. whatever is in memory is what is used, or may be randomly generated.  The behavior
// should not be assumed and is left to the developer).  Note that this option should have
// no effect on the parsing of packets.  It only affects the creation of packets.
/*#ifndef _ONE_NET_USE_RANDOM_PADDING
	#defined _ONE_NET_USE_RANDOM_PADDING
#endif*/

// CRC.  All implementations of ONE-NET must use CRC's, but for debugging and
// learning purposes, it may be useful to turn CRC's on and off.  Comment the three
// lines below out if not using CRC's.  If CRC's are not defined, CRC's will be assigned
// a value of 0.
#ifndef _ONE_NET_USE_CRC
	#define _ONE_NET_USE_CRC
#endif


// Polling - only available for version 2.0 and higher.  Define _POLL if you are using
// polling.  Default for Version 2.0 is _POLL defined.
#ifdef _ONE_NET_VERSION_2_X
	#ifndef _POLL
		#define _POLL
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




// message type defines

#ifndef _NEED_SWITCH_MESSAGE
    #define _NEED_SWITCH_MESSAGE
#endif
/*
#ifndef _NEED_PERCENT_MESSAGE
    #define _NEED_PERCENT_MESSAGE
#endif

#ifndef _NEED_TEMPERATURE_MESSAGE
    #define _NEED_TEMPERATURE_MESSAGE
#endif

#ifndef _NEED_HUMIDITY_MESSAGE
    #define _NEED_HUMIDITY_MESSAGE
#endif

#ifndef _NEED_PRESSURE_MESSAGE
    #define _NEED_PRESSURE_MESSAGE
#endif

#ifndef _NEED_RAINFALL_MESSAGE
    #define _NEED_RAINFALL_MESSAGE
#endif

#ifndef _NEED_SPEED_MESSAGE
    #define _NEED_SPEED_MESSAGE
#endif

#ifndef _NEED_DIRECTION_MESSAGE
    #define _NEED_DIRECTION_MESSAGE
#endif

#ifndef _NEED_OPENING_MESSAGE
    #define _NEED_OPENING_MESSAGE
#endif

#ifndef _NEED_SEAL_MESSAGE
    #define _NEED_SEAL_MESSAGE
#endif

#ifndef _NEED_COLOR_MESSAGE
    #define _NEED_COLOR_MESSAGE
#endif
*/
#ifndef _NEED_SIMPLE_TEXT_MESSAGE
    #define _NEED_SIMPLE_TEXT_MESSAGE
#endif
/*
#ifndef _NEED_DATE_MESSAGE
    #define _NEED_DATE_MESSAGE
#endif

#ifndef _NEED_TIME_MESSAGE
    #define _NEED_TIME_MESSAGE
#endif

#ifndef _NEED_VOLTAGE_MESSAGE
    #define _NEED_VOLTAGE_MESSAGE
#endif

#ifndef _NEED_VOLTAGE_SIMPLE_MESSAGE
    #define _NEED_VOLTAGE_SIMPLE_MESSAGE
#endif

#ifndef _NEED_ENERGY_MESSAGE
    #define _NEED_ENERGY_MESSAGE
#endif

#ifndef _NEED_ACCUM_ENERGY_MESSAGE
    #define _NEED_ACCUM_ENERGY_MESSAGE
#endif

#ifndef _NEED_PEAK_ENERGY_MESSAGE
    #define _NEED_PEAK_ENERGY_MESSAGE
#endif

#ifndef _NEED_GAS_MESSAGE
    #define _NEED_GAS_MESSAGE
#endif

#ifndef _NEED_ACCUM_GAS_MESSAGE
    #define _NEED_ACCUM_GAS_MESSAGE
#endif

#ifndef _NEED_AVERAGE_GAS_MESSAGE
    #define _NEED_AVERAGE_GAS_MESSAGE
#endif

#ifndef _NEED_PEAK_GAS_MESSAGE
    #define _NEED_PEAK_GAS_MESSAGE
#endif

#ifndef _NEED_INSTEON_MESSAGE
    #define _NEED_INSTEON_MESSAGE
#endif

#ifndef _NEED_X10_MESSAGE
    #define _NEED_X10_MESSAGE
#endif
*/
#ifndef _NEED_BLOCK_TEXT_MESSAGE
    #define _NEED_BLOCK_TEXT_MESSAGE
#endif



// unit type defines

#ifndef _NEED_SIMPLE_SWITCH_TYPE
    #define _NEED_SIMPLE_SWITCH_TYPE
#endif
/*
#ifndef _NEED_DIMMER_SWITCH_TYPE
    #define _NEED_DIMMER_SWITCH_TYPE
#endif

#ifndef _NEED_DISPLAY_SWITCH_TYPE
    #define _NEED_DISPLAY_SWITCH_TYPE
#endif

#ifndef _NEED_DISPLAY_DIMMER_TYPE
    #define _NEED_DISPLAY_DIMMER_TYPE
#endif

#ifndef _NEED_SIMPLE_LIGHT_TYPE
    #define _NEED_SIMPLE_LIGHT_TYPE
#endif

#ifndef _NEED_DIMMING_LIGHT_TYPE
    #define _NEED_DIMMING_LIGHT_TYPE
#endif

#ifndef _NEED_OUTLET_TYPE
    #define _NEED_OUTLET_TYPE
#endif

#ifndef _NEED_SPEAKER_TYPE
    #define _NEED_SPEAKER_TYPE
#endif

#ifndef _NEED_TEMPERATURE_SENSOR_TYPE
    #define _NEED_TEMPERATURE_SENSOR_TYPE
#endif

#ifndef _NEED_HUMIDITY_SENSOR_TYPE
    #define _NEED_HUMIDITY_SENSOR_TYPE
#endif

#ifndef _NEED_DOOR_WINDOW_SENSOR_TYPE
    #define _NEED_DOOR_WINDOW_SENSOR_TYPE
#endif

#ifndef _NEED_MOTION_SENSOR_TYPE
    #define _NEED_MOTION_SENSOR_TYPE
#endif

#ifndef _NEED_X10_BRIDGE_TYPE
    #define _NEED_X10_BRIDGE_TYPE
#endif

#ifndef _NEED_INSTEON_BRIDGE_TYPE
    #define _NEED_INSTEON_BRIDGE_SENSOR_TYPE
#endif
*/





// Evaluation Board Options

#ifndef _ONE_NET_EVAL
	#define _ONE_NET_EVAL
#endif

#ifdef _ONE_NET_EVAL
	#ifndef _SERIAL_ASSIGN_DEMO_PINS
		#define _SERIAL_ASSIGN_DEMO_PINS
	#endif
	
	#ifndef _ENABLE_CLIENT_PING_RESPONSE
		#define _ENABLE_CLIENT_PING_RESPONSE
	#endif
	
	// _AUTO_MODE should be defined if you want the Auto Mode option available
	#ifndef _AUTO_MODE
		#define _AUTO_MODE
	#endif

	// _SNIFFER_MODE should be defined if you want the Sniffer Mode option available
	#ifndef _SNIFFER_MODE
		#define _SNIFFER_MODE
	#endif
#endif



// I/O Board Options

/*#ifndef _QUAD_OUTPUT
	#define _QUAD_OUTPUT
#endif

#ifndef _DUAL_OUTPUT
	#define _DUAL_OUTPUT
#endif

#ifndef _QUAD_INPUT
	#define _QUAD_INPUT
#endif*/




// Other Options

#ifndef _CHIP_ENABLE
	#define _CHIP_ENABLE
#endif

#ifndef _R8C_TINY
	#define _R8C_TINY
#endif




// DEBUG options
//#ifdef _ONE_NET_DEBUG
//	#undef _ONE_NET_DEBUG
//#endif

//#ifdef _ONE_NET_DEBUG_STACK
//	#undef _ONE_NET_DEBUG_STACK
//#endif



// Sniffer Front End
//#ifdef _SNIFFER_FRONT_END
//	#undef _SNIFFER_FRONT_END
//#endif



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


	// _ENABLE_IDLE_COMMAND should be defined if you need to be able to switch the device in and out of "idle" mode
    #ifdef _IDLE
        #ifndef _ENABLE_IDLE_COMMAND
            #define _ENABLE_IDLE_COMMAND
		#endif
	#endif

	// _ENABLE_SINGLE_COMMAND should be defined if you are implementing the "single" command option
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

	// _ENABLE_SINGLE_TEXT_COMMAND should be defined if you are implementing the "single text" command option
	#ifndef _ENABLE_SINGLE_TEXT_COMMAND
		#define _ENABLE_SINGLE_TEXT_COMMAND
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

	// _ENABLE_ERASE_COMMAND should be defined if you are implementing the "erase" command option
	#ifndef _ENABLE_ERASE_COMMAND
		#define _ENABLE_ERASE_COMMAND
	#endif

	// _ENABLE_SAVE_COMMAND should be defined if you are implementing the "save" command option
	#ifndef _ENABLE_SAVE_COMMAND
		#define _ENABLE_SAVE_COMMAND
	#endif

	// _ENABLE_DUMP_COMMAND should be defined if you are implementing the "dump" command option
/*	#ifndef _ENABLE_DUMP_COMMAND
		#define _ENABLE_DUMP_COMMAND
	#endif*/
	
	// _ENABLE_MEMDUMP_COMMAND should be defined if you are implementing the "memdump" command option
	#ifdef _ONE_NET_DUMP
	    #ifndef _ENABLE_MEMDUMP_COMMAND
		    #define _ENABLE_MEMDUMP_COMMAND
	    #endif
	#endif
	
	// _ENABLE_MEMLOAD_COMMAND should be defined if you are implementing the "memload" command option
	#ifdef _ONE_NET_LOAD
	    #ifndef _ENABLE_MEMLOAD_COMMAND
		    #define _ENABLE_MEMLOAD_COMMAND
	    #endif
	#endif
	
	// _ENABLE_RSINGLE_COMMAND should be defined if you are implementing the "rsingle" command option
	/*#ifdef _ENABLE_SINGLE_COMMAND
		#ifndef _ENABLE_RSINGLE_COMMAND
			#define _ENABLE_RSINGLE_COMMAND
		#endif
	#endif*/

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

	    // _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND should be defined if you are implementing the "change fragment delay" command option
	    #ifndef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
		    #define _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
	    #endif

    	// _ENABLE_DATA_RATE_TEST_COMMAND should be defined if you are implementing the "data rate test" command option
	    #ifndef _ENABLE_DATA_RATE_TEST_COMMAND
		    #define _ENABLE_DATA_RATE_TEST_COMMAND
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

	// _ENABLE_CHANNEL_COMMAND should be defined if you are implementing the "channel" command option
	#ifndef _ENABLE_CHANNEL_COMMAND
		#define _ENABLE_CHANNEL_COMMAND
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
#endif
	



// Other Options

//#ifndef _NEED_XDUMP
//	#define _NEED_XDUMP
//#endif

//#ifndef _EVAL_0005_NO_REVISION
//	#define _EVAL_0005_NO_REVISION
//#endif

//#ifndef _ONE_NET_TEST_NACK_WITH_REASON_FIELD
//	#define _ONE_NET_TEST_NACK_WITH_REASON_FIELD
//#endif





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
