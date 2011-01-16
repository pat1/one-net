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


// _OVERRIDE_CONFIG_OPTIONS_FILE is not defined.  Use the code below to define things.



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
	//#define _ONE_NET_MASTER
#endif

#ifndef _ONE_NET_CLIENT
	#define _ONE_NET_CLIENT
#endif



// Peer Assignments.  Some applications need to implement peer assignments.  Some do not.
// Define _PEER if your application implements peer assignments.  Default is _PEER assigned.
// Devices with no inputs (i.e. I/O devices with only relays) cannot have peer assignments.
#ifndef _PEER
	#define _PEER
#endif


// Idle Option - Should be defined if the device can ever be idle
#ifndef _IDLE
//    #define _IDLE
#endif


// Main #define options for I/O boards

// A device is a "simple client" if and only if it is never used as a master, it cannot
// implement multi-hop, it cannot implement block messages, and it cannot implement
// stream messages.
#ifndef _ONE_NET_SIMPLE_CLIENT
	#define _ONE_NET_SIMPLE_CLIENT
#endif

#ifndef _QUAD_OUTPUT
//	#define _QUAD_OUTPUT
#endif

#ifndef _DUAL_OUTPUT
//	#define _DUAL_OUTPUT
#endif

#ifndef _QUAD_INPUT
	#define _QUAD_INPUT
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


// CRC.  All implementations of ONE-NET must use CRC's, but for debugging and
// learning purposes, it may be useful to turn CRC's on and off.  Comment the three
// lines below out if not using CRC's.  If CRC's are not defined, CRC's will be assigned
// a value of 0.
#ifndef _ONE_NET_USE_CRC
	#define _ONE_NET_USE_CRC
#endif


// Random Padding.  If defined, unused portions of encrypted packets will be randomly
// generated. If not turned on, unused portions of encrypted packets will be either 0
// or "undefined" ("undefined" means that they may be zeroed out, they may be left as-is
// i.e. whatever is in memory is what is used, or may be randomly generated.  The behavior
// should not be assumed and is left to the developer).  Note that this option should have
// no effect on the parsing of packets.  It only affects the creation of packets.
#ifdef _ONE_NET_VERSION_2_X
    #ifndef _ONE_NET_USE_RANDOM_PADDING
	    #define _ONE_NET_USE_RANDOM_PADDING
    #endif
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

#ifndef _NEED_SIMPLE_TEXT_MESSAGE
    #define _NEED_SIMPLE_TEXT_MESSAGE
#endif

#ifndef _NEED_DATE_MESSAGE
    #define _NEED_DATE_MESSAGE
#endif

#ifndef _NEED_TIME_MESSAGE
    #define _NEED_TIME_MESSAGE
#endif
*/
#ifndef _NEED_VOLTAGE_MESSAGE
    #define _NEED_VOLTAGE_MESSAGE
#endif

#ifndef _NEED_VOLTAGE_SIMPLE_MESSAGE
    #define _NEED_VOLTAGE_SIMPLE_MESSAGE
#endif
/*
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

#ifndef _NEED_BLOCK_TEXT_MESSAGE
    #define _NEED_BLOCK_TEXT_MESSAGE
#endif*/



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
*/
#ifndef _NEED_OUTLET_TYPE
    #define _NEED_OUTLET_TYPE
#endif
/*
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

