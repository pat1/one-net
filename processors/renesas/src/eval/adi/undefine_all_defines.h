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


#ifdef _ONE_NET_MASTER
	#undef _ONE_NET_MASTER
#endif

#ifdef _ONE_NET_CLIENT
	#undef _ONE_NET_CLIENT
#endif

#ifdef _PEER
	#undef _PEER
#endif

#ifdef _BLOCK_MESSAGES_ENABLED
	#undef _BLOCK_MESSAGES_ENABLED
#endif

#ifdef _STREAM_MESSAGES_ENABLED
    #undef _STREAM_MESSAGES_ENABLED
#endif

#ifdef _ONE_NET_MULTI_HOP
	#undef _ONE_NET_MULTI_HOP
#endif

#ifdef _ONE_NET_MH_CLIENT_REPEATER
	#undef _ONE_NET_MH_CLIENT_REPEATER
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

#ifdef _NON_VOLATILE_MEMORY
    #undef _NON_VOLATILE_MEMORY
#endif

#ifdef _DATA_RATE
    #undef _DATA_RATE
#endif

#ifdef _ENABLE_CLI
	#undef _ENABLE_CLI
#endif

#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
	#undef _AT_LEAST_ONE_COMMAND_ENABLED
#endif

#ifdef _ENABLE_SINGLE_COMMAND
	#undef _ENABLE_SINGLE_COMMAND
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
	#undef _ENABLE_RSSSI_COMMAND
#endif

#ifdef _ENABLE_LIST_COMMAND
	#undef _ENABLE_LIST_COMMAND
#endif

#ifdef _ENABLE_CHANNEL_COMMAND
    #undef _ENABLE_CHANNEL_COMMAND
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
		
#ifdef _ENABLE_CHANGE_KEY_COMMAND
    #undef _ENABLE_CHANGE_KEY_COMMAND
#endif

#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
    #undef _ENABLE_REMOVE_DEVICE_COMMAND
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

#ifdef _ENABLE_GET_CHANNEL_COMMAND
	#undef _ENABLE_GET_CHANNEL_COMMAND
#endif

#ifdef _ENABLE_USER_PIN_COMMAND
	#undef _ENABLE_USER_PIN_COMMAND
#endif

#ifdef _ENABLE_JOIN_COMMAND
    #undef _ENABLE_JOIN_COMMAND
#endif

#ifdef _ENABLE_SETNI_COMMAND
	#undef _ENABLE_SETNI_COMMAND
#endif

#ifdef _ENABLE_SNIFF_COMMAND
	#undef _ENABLE_SNIFF_COMMAND
#endif

#ifdef _ENABLE_MODE_COMMAND
	#undef _ENABLE_MODE_COMMAND
#endif

#ifdef _ENABLE_ECHO_COMMAND
	#undef _ENABLE_ECHO_COMMAND
#endif
    
#ifdef _ENABLE_SET_DATA_RATE_COMMAND
    #undef _ENABLE_SET_DATA_RATE_COMMAND
#endif

#ifdef _NEED_XDUMP
	#undef _NEED_XDUMP
#endif

#ifdef _EVAL_0005_NO_REVISION
	#undef _EVAL_0005_NO_REVISION
#endif

#ifdef _DEBUGGING_TOOLS
    #undef _DEBUGGING_TOOLS
#endif

#ifdef _SINGLE_QUEUE_LEVEL
	#undef _SINGLE_QUEUE_LEVEL
#endif

#ifdef _EXTENDED_SINGLE
    #undef _EXTENDED_SINGLE
#endif

#ifdef _MINIMIZE_STRING_LENGTHS
   #undef _MINIMIZE_STRING_LENGTHS
#endif

#ifdef _HAS_LEDS
    #undef _HAS_LEDS
#endif

#ifdef _BLOCKING_UART
    #undef _BLOCKING_UART
#endif

#ifdef _CHANNEL_OVERRIDE
    #undef _CHANNEL_OVERIDE
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

