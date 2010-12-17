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


// add any new configuration options you need.  Comment out any you do not need

#ifndef _ONE_NET_MULTI_HOP
	#define _ONE_NET_MULTI_HOP
#endif

#ifndef _ONE_NET_EVAL
	#define _ONE_NET_EVAL
#endif

#ifndef _CHIP_ENABLE
	#define _CHIP_ENABLE
#endif

#ifndef _R8C_TINY
	#define _R8C_TINY
#endif

#ifndef _ENABLE_DUMP_COMMAND
	#define _ENABLE_DUMP_COMMAND
#endif

#ifndef _SERIAL_ASSIGN_DEMO_PINS
	#define _SERIAL_ASSIGN_DEMO_PINS
#endif

#ifndef _ENABLE_LIST_COMMAND
	#define _ENABLE_LIST_COMMAND
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



// options not needed for Eval Board ADI project are below and are therefore commented out


//#ifndef _ONE_NET_MH_CLIENT_REPEATER
//	#define _ONE_NET_MH_CLIENT_REPEATER
//#endif

//#ifndef _NEED_XDUMP
//	#define _NEED_XDUMP
//#endif

//#ifndef _ENABLE_RSINGLE_COMMAND
//	#define _ENABLE_RSINGLE_COMMAND
//#endif

//#ifndef _EVAL_0005_NO_REVISION
//	#define _EVAL_0005_NO_REVISION
//#endif

//#ifndef _ONE_NET_TEST_NACK_WITH_REASON_FIELD
//	#define _ONE_NET_TEST_NACK_WITH_REASON_FIELD
//#endif

//#ifndef _ONE_NET_DEBUG
//	#define _ONE_NET_DEBUG
//#endif

//#ifndef _ENABLE_RSSI_COMMAND
//	#define _ENABLE_RSSI_COMMAND
//#endif

//#ifndef _ONE_NET_DEBUG_STACK
//	#define _ONE_NET_DEBUG_STACK
//#endif



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

#endif // _ONE_NET_CONFIG_OPTIONS_H //
