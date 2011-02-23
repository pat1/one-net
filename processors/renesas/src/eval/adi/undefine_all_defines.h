//! \defgroup undefine_all_defines Undefine any configuration definitions here
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
    \file undefine_all_defines.h
    \brief Undefine any configuration options that may already be defined in this file.

    Undefine any configuration options that may already be defined in this file.

*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup undefine_all_defines_const
//! \ingroup undefine_all_defines_options
//! @{



// Version Information

#ifdef _ONE_NET_VERSION_1_X
	#undef _ONE_NET_VERSION_1_X
#endif

#ifdef _ONE_NET_VERSION_2_X
	#undef _ONE_NET_VERSION_2_X
#endif


// Master/Client

#ifdef _ONE_NET_MASTER
	#undef _ONE_NET_MASTER
#endif

#ifdef _ONE_NET_CLIENT
	#undef _ONE_NET_CLIENT
#endif

#ifdef _ONE_NET_SIMPLE_DEVICE
	#undef _ONE_NET_SIMPLE_DEVICE
#endif

#ifdef _ONE_NET_SIMPLE_CLIENT
	#undef _ONE_NET_SIMPLE_CLIENT
#endif

#ifdef _ONE_NET_SIMPLE_MASTER
	#undef _ONE_NET_SIMPLE_CLIENT
#endif


// Block Messages
#ifdef _BLOCK_MESSAGES_ENABLED
	#undef _BLOCK_MESSAGES_ENABLED
#endif

// Stream Messages
#ifdef _STREAM_MESSAGES_ENABLED
	#undef _STREAM_MESSAGES_ENABLED
#endif

// Multi-Hop
#ifdef _ONE_NET_MULTI_HOP
	#undef _ONE_NET_MULTI_HOP
#endif

#ifdef _ONE_NET_MH_CLIENT_REPEATER
	#undef _ONE_NET_MH_CLIENT_REPEATER
#endif


// Idle option
#ifdef _IDLE
	#undef _IDLE
#endif

// Enhanced Invite
#ifdef _ENHANCED_INVITE
	#undef _ENHANCED_INVITE
#endif


// Encryption, Encoding, Random Padding of unused packet portions for
// increased security, and CRCs

#ifdef _ONE_NET_USE_ENCRYPTION
	#undef _ONE_NET_USE_ENCRYPTION
#endif

#ifdef _ONE_NET_USE_ENCODING
	#undef _ONE_NET_USE_ENCODING
#endif

#ifdef _ONE_NET_USE_RANDOM_PADDING
	#undef _ONE_NET_USE_RANDOM_PADDING
#endif

#ifdef _ONE_NET_USE_CRC
	#undef _ONE_NET_USE_CRC
#endif


// Peer assignments and polling
#ifdef _PEER
	#undef _PEER
#endif

#ifdef _POLL
	#undef _POLL
#endif


// Locale for channels (Europe or U.S.A.)
#ifdef _US_CHANNELS
	#undef _US_CHANNELS
#endif

#ifdef _EUROPE_CHANNELS
	#undef _EUROPE_CHANNELS
#endif



// Evaluation Board Options

#ifdef _ONE_NET_EVAL
	#undef _ONE_NET_EVAL
#endif

#ifdef _AUTO_MODE
	#undef _AUTO_MODE
#endif

#ifdef _SNIFFER_MODE
	#undef _SNIFFER_MODE
#endif

#ifdef _SERIAL_ASSIGN_DEMO_PINS
	#undef _SERIAL_ASSIGN_DEMO_PINS
#endif

#ifdef _ENABLE_CLIENT_PING_RESPONSE
	#undef _ENABLE_CLIENT_PING_RESPONSE
#endif




// I/O Board Options

#ifdef _QUAD_OUTPUT
	#undef _QUAD_OUTPUT
#endif

#ifdef _DUAL_OUTPUT
	#undef _DUAL_OUTPUT
#endif

#ifdef _QUAD_INPUT
	#undef _QUAD_INPUT
#endif



// message type defines

#ifdef _NEED_SWITCH_MESSAGE
    #undef _NEED_SWITCH_MESSAGE
#endif

#ifdef _NEED_PERCENT_MESSAGE
    #undef _NEED_PERCENT_MESSAGE
#endif

#ifdef _NEED_TEMPERATURE_MESSAGE
    #undef _NEED_TEMPERATURE_MESSAGE
#endif

#ifdef _NEED_HUMIDITY_MESSAGE
    #undef _NEED_HUMIDITY_MESSAGE
#endif

#ifdef _NEED_PRESSURE_MESSAGE
    #undef _NEED_PRESSURE_MESSAGE
#endif

#ifdef _NEED_RAINFALL_MESSAGE
    #undef _NEED_RAINFALL_MESSAGE
#endif

#ifdef _NEED_SPEED_MESSAGE
    #undef _NEED_SPEED_MESSAGE
#endif

#ifdef _NEED_DIRECTION_MESSAGE
    #undef _NEED_DIRECTION_MESSAGE
#endif

#ifdef _NEED_OPENING_MESSAGE
    #undef _NEED_OPENING_MESSAGE
#endif

#ifdef _NEED_SEAL_MESSAGE
    #undef _NEED_SEAL_MESSAGE
#endif

#ifdef _NEED_COLOR_MESSAGE
    #undef _NEED_COLOR_MESSAGE
#endif

#ifdef _NEED_SIMPLE_TEXT_MESSAGE
    #undef _NEED_SIMPLE_TEXT_MESSAGE
#endif

#ifdef _NEED_DATE_MESSAGE
    #undef _NEED_DATE_MESSAGE
#endif

#ifdef _NEED_TIME_MESSAGE
    #undef _NEED_TIME_MESSAGE
#endif

#ifdef _NEED_VOLTAGE_MESSAGE
    #undef _NEED_VOLTAGE_MESSAGE
#endif

#ifdef _NEED_VOLTAGE_SIMPLE_MESSAGE
    #undef _NEED_VOLTAGE_SIMPLE_MESSAGE
#endif

#ifdef _NEED_ENERGY_MESSAGE
    #undef _NEED_ENERGY_MESSAGE
#endif

#ifdef _NEED_ACCUM_ENERGY_MESSAGE
    #undef _NEED_ACCUM_ENERGY_MESSAGE
#endif

#ifdef _NEED_PEAK_ENERGY_MESSAGE
    #undef _NEED_PEAK_ENERGY_MESSAGE
#endif

#ifdef _NEED_GAS_MESSAGE
    #undef _NEED_GAS_MESSAGE
#endif

#ifdef _NEED_ACCUM_GAS_MESSAGE
    #undef _NEED_ACCUM_GAS_MESSAGE
#endif

#ifdef _NEED_AVERAGE_GAS_MESSAGE
    #undef _NEED_AVERAGE_GAS_MESSAGE
#endif

#ifdef _NEED_PEAK_GAS_MESSAGE
    #undef _NEED_PEAK_GAS_MESSAGE
#endif

#ifdef _NEED_INSTEON_MESSAGE
    #undef _NEED_INSTEON_MESSAGE
#endif

#ifdef _NEED_X10_MESSAGE
    #undef _NEED_X10_MESSAGE
#endif

#ifdef _NEED_BLOCK_TEXT_MESSAGE
    #undef _NEED_BLOCK_TEXT_MESSAGE
#endif



// unit type defines

#ifdef _NEED_SIMPLE_SWITCH_TYPE
    #undef _NEED_SIMPLE_SWITCH_TYPE
#endif

#ifdef _NEED_DIMMER_SWITCH_TYPE
    #undef _NEED_DIMMER_SWITCH_TYPE
#endif

#ifdef _NEED_DISPLAY_SWITCH_TYPE
    #undef _NEED_DISPLAY_SWITCH_TYPE
#endif

#ifdef _NEED_DISPLAY_DIMMER_TYPE
    #undef _NEED_DISPLAY_DIMMER_TYPE
#endif

#ifdef _NEED_SIMPLE_LIGHT_TYPE
    #undef _NEED_SIMPLE_LIGHT_TYPE
#endif

#ifdef _NEED_DIMMING_LIGHT_TYPE
    #undef _NEED_DIMMING_LIGHT_TYPE
#endif

#ifdef _NEED_OUTLET_TYPE
    #undef _NEED_OUTLET_TYPE
#endif

#ifdef _NEED_SPEAKER_TYPE
    #undef _NEED_SPEAKER_TYPE
#endif

#ifdef _NEED_TEMPERATURE_SENSOR_TYPE
    #undef _NEED_TEMPERATURE_SENSOR_TYPE
#endif

#ifdef _NEED_HUMIDITY_SENSOR_TYPE
    #undef _NEED_HUMIDITY_SENSOR_TYPE
#endif

#ifdef _NEED_DOOR_WINDOW_SENSOR_TYPE
    #undef _NEED_DOOR_WINDOW_SENSOR_TYPE
#endif

#ifdef _NEED_MOTION_SENSOR_TYPE
    #undef _NEED_MOTION_SENSOR_TYPE
#endif

#ifdef _NEED_X10_BRIDGE_TYPE
    #undef _NEED_HUMIDITY_SENSOR_TYPE
#endif

#ifdef _NEED_INSTEON_BRIDGE_TYPE
    #undef _NEED_INSTEON_BRIDGE_SENSOR_TYPE
#endif




// Other #define options

#ifdef _ONE_NET_EVAL
	#undef _ONE_NET_EVAL
#endif

#ifdef _CHIP_ENABLE
	#undef _CHIP_ENABLE
#endif

#ifdef _R8C_TINY
	#undef _R8C_TINY
#endif

#ifdef _NEED_XDUMP
	#undef _NEED_XDUMP
#endif

#ifdef _EVAL_0005_NO_REVISION
	#undef _EVAL_0005_NO_REVISION
#endif

#ifdef _ONE_NET_TEST_NACK_WITH_REASON_FIELD
	#undef _ONE_NET_TEST_NACK_WITH_REASON_FIELD
#endif



// DEBUG options
#ifdef _ONE_NET_DEBUG
	#undef _ONE_NET_DEBUG
#endif

#ifdef _ONE_NET_DEBUG_STACK
	#undef _ONE_NET_DEBUG_STACK
#endif



// Sniffer Front End
#ifdef _SNIFFER_FRONT_END
	#undef _SNIFFER_FRONT_END
#endif



// command line interface defines
#ifdef _ENABLE_CLI
	#undef _ENABLE_CLI
#endif

#ifdef _ENABLE_IDLE_COMMAND
    #undef _ENABLE_IDLE_COMMAND
#endif

#ifdef _ENABLE_SINGLE_COMMAND
	#undef _ENABLE_SINGLE_COMMAND
#endif

#ifdef _ENABLE_SET_VALUE_COMMAND
	#undef _ENABLE_SET_VALUE_COMMAND
#endif

#ifdef _ENABLE_SET_PIN_COMMAND
	#undef _ENABLE_SET_PIN_COMMAND
#endif

#ifdef _ENABLE_SINGLE_TEXT_COMMAND
	#undef _ENABLE_SINGLE_TEXT_COMMAND
#endif

#ifdef _ENABLE_BLOCK_COMMAND
	#undef _ENABLE_BLOCK_COMMAND
#endif

#ifdef _ENABLE_BLOCK_TEXT_COMMAND
	#undef _ENABLE_BLOCK_TEXT_COMMAND
#endif

#ifdef _ENABLE_ERASE_COMMAND
	#undef _ENABLE_ERASE_COMMAND
#endif

#ifdef _ENABLE_SAVE_COMMAND
	#undef _ENABLE_SAVE_COMMAND
#endif

#ifdef _ENABLE_DUMP_COMMAND
	#undef _ENABLE_DUMP_COMMAND
#endif

#ifdef _ENABLE_RSINGLE_COMMAND
	#undef _ENABLE_RSINGLE_COMMAND
#endif

#ifdef _ENABLE_RSSI_COMMAND
	#undef _ENABLE_RSSI_COMMAND
#endif

#ifdef _ENABLE_LIST_COMMAND
	#undef _ENABLE_LIST_COMMAND
#endif

#ifdef _ENABLE_INVITE_COMMAND
	#undef _ENABLE_INVITE_COMMAND
#endif

#ifdef _ENABLE_CANCEL_INVITE_COMMAND
	#undef _ENABLE_CANCEL_INVITE_COMMAND
#endif

#ifdef _ENABLE_ASSIGN_PEER_COMMAND
	#undef _ENABLE_ASSIGN_PEER_COMMAND
#endif

#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
	#undef _ENABLE_UNASSIGN_PEER_COMMAND
#endif

#ifdef _ENABLE_UPDATE_MASTER_COMMAND
	#undef _ENABLE_UPDATE_MASTER_COMMAND
#endif

#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
	#undef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
#endif

#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
	#undef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
#endif

#ifdef _ENABLE_CHANGE_KEY_COMMAND
	#undef _ENABLE_CHANGE_KEY_COMMAND
#endif

#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
	#undef _ENABLE_REMOVE_DEVICE_COMMAND
#endif

#ifdef _ENABLE_DATA_RATE_TEST_COMMAND
	#undef _ENABLE_DATA_RATE_TEST_COMMAND
#endif

#ifdef _ENABLE_GET_CHANNEL_COMMAND
	#undef _ENABLE_GET_CHANNEL_COMMAND
#endif

#ifdef _ENABLE_USER_PIN_COMMAND
	#undef _ENABLE_USER_PIN_COMMAND
#endif

#ifdef _ENABLE_JOIN_COMMAND
	#undef _ENABLE_JOIN_COMMAND
#endif

#ifdef _ENABLE_CHANNEL_COMMAND
	#undef _ENABLE_CHANNEL_COMMAND
#endif

#ifdef _ENABLE_SETNI_COMMAND
	#undef _ENABLE_SETNI_COMMAND
#endif

#ifdef _ENABLE_SNIFF_COMMAND
	#undef _ENABLE_SNIFF_COMMAND
#endif

#ifdef _ENABLE_MODE_COMMAND
	#undef _ENABLE_SNIFF_COMMAND
#endif

#ifdef _ENABLE_ECHO_COMMAND
	#undef _ENABLE_ECHO_COMMAND
#endif




//! @} one_net_undefine_all_defines_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_undefine_all_defines_typedefs
//! \ingroup one_net_undefine_all_defines
//! @{

//! @} one_net_undefine_all_defines_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup one_net_undefine_all_defines_pub_var
//! \ingroup one_net_undefine_all_defines
//! @{

//! @} one_net_undefine_all_defines_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_undefine_all_defines_pub_func
//! \ingroup one_net_undefine_all_defines
//! @{


//! @} one_net_undefine_all_defines
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_undefine_all_defines

