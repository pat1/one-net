//! \defgroup undefine_all_defines Undefine any configuration definitions here
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
    \file undefine_all_defines.h
    \brief Undefine any configuration options that may already be defined in this file.

    Undefine any configuration options that may already be defined in this file.

*/


#ifdef ONE_NET_MASTER
	#undef ONE_NET_MASTER
#endif

#ifdef ONE_NET_CLIENT
	#undef ONE_NET_CLIENT
#endif

#ifdef PEER
	#undef PEER
#endif

#ifdef BLOCK_MESSAGES_ENABLED
	#undef BLOCK_MESSAGES_ENABLED
#endif

#ifdef STREAM_MESSAGES_ENABLED
    #undef STREAM_MESSAGES_ENABLED
#endif

#ifdef ONE_NET_MULTI_HOP
	#undef ONE_NET_MULTI_HOP
#endif

#ifdef ONE_NET_MH_CLIENT_REPEATER
	#undef ONE_NET_MH_CLIENT_REPEATER
#endif

#ifdef _RANGE_TESTING
    #undef _RANGE_TESTING
#endif

#ifdef _ONE_NET_SIMPLE_CLIENT
    #define _ONE_NET_SIMPLE_CLIENT
#endif

#ifdef _IDLE
    #undef _IDLE
#endif

#ifdef _ENHANCED_INVITE
    #undef _ENHANCED_INVITE
#endif

#ifdef _US_CHANNELS
	#undef _US_CHANNELS
#endif

#ifdef _EUROPE_CHANNELS
	#undef _EUROPE_CHANNELS
#endif

#ifdef _ONE_NET_EVAL
	#undef _ONE_NET_EVAL
#endif

#ifdef _DEBUG_VERBOSE_LEVEL
    #undef _DEBUG_VERBOSE_LEVEL
#endif

#ifdef _AUTO_MODE
	#undef _AUTO_MODE
#endif

#ifdef _SNIFFER_MODE
    #undef _SNIFFER_MODE
#endif

#ifdef _CHIP_ENABLE
	#undef _CHIP_ENABLE
#endif

#ifdef _R8C_TINY
	#undef _R8C_TINY
#endif

#ifdef NON_VOLATILE_MEMORY
    #undef NON_VOLATILE_MEMORY
#endif

#ifdef _DATA_RATE_CHANNEL
    #undef _DATA_RATE_CHANNEL
#endif

#ifdef _ENABLE_CLI
	#undef _ENABLE_CLI
#endif

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
	#undef _AT_LEAST_ONE_COMMAND_ENABLED
#endif

#ifdef ENABLE_SINGLE_COMMAND
	#undef ENABLE_SINGLE_COMMAND
#endif

#ifdef ENABLE_BLOCK_COMMAND
    #undef ENABLE_BLOCK_COMMAND
#endif

#ifdef ENABLE_BLOCK_TEXT_COMMAND
    #undef ENABLE_BLOCK_TEXT_COMMAND
#endif
    
#ifdef ENABLE_ERASE_COMMAND
    #undef ENABLE_ERASE_COMMAND
#endif

#ifdef ENABLE_SAVE_COMMAND
    #undef ENABLE_SAVE_COMMAND
#endif

#ifdef ENABLE_DUMP_COMMAND
	#undef ENABLE_DUMP_COMMAND
#endif

#ifdef ENABLE_RSINGLE_COMMAND
	#undef ENABLE_RSINGLE_COMMAND
#endif

#ifdef ENABLE_RSSI_COMMAND
	#undef ENABLE_RSSI_COMMAND
#endif

#ifdef ENABLE_LIST_COMMAND
	#undef ENABLE_LIST_COMMAND
#endif

#ifdef ENABLE_CHANNEL_COMMAND
    #undef ENABLE_CHANNEL_COMMAND
#endif

#ifdef ENABLE_INVITE_COMMAND
    #undef ENABLE_INVITE_COMMAND
#endif

#ifdef ENABLE_CANCEL_INVITE_COMMAND
    #undef ENABLE_CANCEL_INVITE_COMMAND
#endif

#ifdef ENABLE_ASSIGN_PEER_COMMAND
    #undef ENABLE_ASSIGN_PEER_COMMAND
#endif

#ifdef ENABLE_UNASSIGN_PEER_COMMAND
    #undef ENABLE_UNASSIGN_PEER_COMMAND
#endif
		
#ifdef _ENABLE_CHANGE_KEY_COMMAND
    #undef _ENABLE_CHANGE_KEY_COMMAND
#endif

#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
    #undef _ENABLE_REMOVE_DEVICE_COMMAND
#endif

#ifdef _ENABLE_SET_FLAGS_COMMAND
    #undef _ENABLE_SET_FLAGS_COMMAND
#endif

#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
    #undef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
#endif

#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
    #undef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
#endif

#ifdef ENABLE_GET_CHANNEL_COMMAND
	#undef ENABLE_GET_CHANNEL_COMMAND
#endif

#ifdef ENABLE_USER_PIN_COMMAND
	#undef ENABLE_USER_PIN_COMMAND
#endif

#ifdef ENABLE_JOIN_COMMAND
    #undef ENABLE_JOIN_COMMAND
#endif

#ifdef ENABLE_SETNI_COMMAND
	#undef ENABLE_SETNI_COMMAND
#endif

#ifdef ENABLE_SNIFF_COMMAND
	#undef ENABLE_SNIFF_COMMAND
#endif

#ifdef ENABLE_MODE_COMMAND
	#undef ENABLE_MODE_COMMAND
#endif

#ifdef ENABLE_ECHO_COMMAND
	#undef ENABLE_ECHO_COMMAND
#endif
    
#ifdef ENABLE_SET_DR_CHANNEL_COMMAND
    #undef ENABLE_SET_DR_CHANNEL_COMMAND
#endif

#ifdef _EVAL_0005_NO_REVISION
	#undef _EVAL_0005_NO_REVISION
#endif

#ifdef DEBUGGING_TOOLS
    #undef DEBUGGING_TOOLS
#endif

#ifdef SINGLE_QUEUE_LEVEL
	#undef SINGLE_QUEUE_LEVEL
#endif

#ifdef EXTENDED_SINGLE
    #undef EXTENDED_SINGLE
#endif

#ifdef MINIMIZE_STRING_LENGTHS
   #undef MINIMIZE_STRING_LENGTHS
#endif

#ifdef HAS_LEDS
    #undef HAS_LEDS
#endif

#ifdef _BLOCKING_UART
    #undef _BLOCKING_UART
#endif

#ifdef CHANNEL_OVERRIDE
    #undef CHANNEL_OVERRIDE
#endif

#ifdef CHANNEL_OVERRIDE_CHANNEL
    #undef CHANNEL_OVERRIDE_CHANNEL
#endif



//==============================================================================
//                                  CONSTANTS
//! \defgroup undefine_all_defines_const
//! \ingroup undefine_all_defines_options
//! @{


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

