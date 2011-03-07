#ifndef _ONE_NET_TIMER_H
#define _ONE_NET_TIMER_H

#include "config_options.h"


//! \defgroup ONE-NET_TIMER Timer functionality used by ONE-NET
//! \ingroup ONE-NET_utility
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
    \file one_net_timer.h
    \brief Timer functionality used by ONE-NET

    The timer functionality requires the use of a tick timer whose rate does
    not vary.  The tick timer cannot be reset or modified either.  This module
    will handle rollover of the tick timer.
    
    This module keeps track of the various timers used by ONE-NET, and can be
    used by applications.  Timers are identified by an enumeration, so if an
    application wants to use this module, it should set it's first timer to
    NUM_ONE_NET_TIMERS and increment from that point.  The application must
    specify the number of timers used even if it does not use the timer
    functionality by defining the NUM_TIMERS variable to the total number of
    timers used (NUM_ONE_NET_TIMERS if the application is not useing this
    module).
    
    Unfortuantly, the implementation is tied to the one_net_master and
    one_net_client files since it needs to know how many timers to allocate
    (doing a static allocation to ensure there is enough memory since this is
    for really small devices).

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_timer_port_const.h"

#include "tick.h"

#ifdef _ONE_NET_EVAL
    #include "one_net_client.h"
    #include "one_net_master.h"
#elif defined(_ONE_NET_MASTER) // ifdef _ONE_NET_EVAL //
    #include "one_net_master.h"
#else // elif _ONE_NET_MASTER is defined //
    #include "one_net_client.h"
#endif // else _ONE_NET_EVAL and _ONE_NET_MASTER are not defined //


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_TIMER_const
//! \ingroup ONE-NET_TIMER
//! @{

enum
{
    //! the number of timers needed for transactions
    #ifdef _ONE_NET_EVAL
        // This is the only case where we are allowing a device to contain
        // both MASTER and CLIENT code.  The transaction timers will be shared
        // since the device can only run as a MASTER or a CLIENT at a time.
        // This is to make sure we have enough timers to be shared by both.
        ONT_TXN_TIMER_COUNT = ON_MASTER_TXN_COUNT > ON_CLIENT_TXN_COUNT
          ? ON_MASTER_TXN_COUNT : ON_CLIENT_TXN_COUNT
    #elif defined(_ONE_NET_MASTER) // ifdef _ONE_NET_EVAL //
        ONT_TXN_TIMER_COUNT = ON_MASTER_TXN_COUNT
    #else // elif defined(_ONE_NET_MASTER) //
        ONT_TXN_TIMER_COUNT = ON_CLIENT_TXN_COUNT
    #endif // else must be a CLIENT //
};


// If additional timers, add them before the #if defined(_ONE_NET_MASTER) ...
enum
{
    //! General purpose timer (shared to save resources)
    ONT_GENERAL_TIMER = ONT_NUM_APP_TIMERS,

    //! The number of multi-hop packets that need a timer.
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        ONT_MH_TIMER,
    #endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //
	
    #if defined(_ONE_NET_MASTER) || defined(_ONE_NET_EVAL) || defined(_ENHANCED_INVITE)
        //! Timer to know when to abort the invite process.  This is also used
        //! when the MASTER creates a network.
        ONT_INVITE_TIMER,
	#endif

    #if defined(_ONE_NET_MASTER) || defined(_ONE_NET_EVAL)
        //! Timer used to attempt to send a key change message to a different
        //! device.  To save space, this is also reused when the MASTER is
        //! creating the network since the two operations are mutually exclusive
        ONT_CHANGE_KEY_TIMER,
		
        //! Timer used to attempt to send a change stream key message to a
        //! different device.
        ONT_CHANGE_STREAM_KEY_TIMER,
        
        //! Timer used for the data rate test.
        ONT_DATA_RATE_TEST_TIMER,
        
        // Add any timers above this value.  This is used in the eval case when
        // both the MASTER and CLIENT are in the same device, so some of the
        // timers are shared
        #ifdef _ONE_NET_EVAL
            // The timer where the transaction timers would start for the MASTER
            ONT_M_TXN_START,
        #endif // ifdef _ONE_NET_EVAL //
    #endif // if a MASTER //
    
    #if !defined(_ONE_NET_MASTER) || defined(_ONE_NET_EVAL)
        // if it is an eval board that contains both the MASTER and the CLIENT,
        // reuse the same timers declared by the MASTER since the MASTER and
        // CLIENT can't run at the same time.
        #ifdef _ONE_NET_EVAL
            //! Timer for CLIENT to know when to send a Keep Alive
            ONT_KEEP_ALIVE_TIMER = ONT_INVITE_TIMER,
        #else // ifdef _ONE_NET_EVAL //
            ONT_KEEP_ALIVE_TIMER,
        #endif // else _ONE_NET_EVAL is not defined //

        //! Timer for the period the CLIENT should stay awake after receiving
        //! a Single Data ACK Stay Awake.
        ONT_STAY_AWAKE_TIMER,

        #if !defined(_ONE_NET_SIMPLE_CLIENT)
            //! Timer for CLIENT to query for the stream key
            ONT_STREAM_KEY_TIMER,
        #endif // if _ONE_NET_SIMPLE_CLIENT is not defined //
        
        // Add any timers above this value.  This is used in the eval case when
        // both the MASTER and CLIENT are in the same device, so some of the
        // timers are shared
        #ifdef _ONE_NET_EVAL
            // The timer where the transaction timers would start for the CLIENT
            ONT_C_TXN_START,
        #endif // ifdef _ONE_NET_EVAL //
    #endif // ifndef _ONE_NET_MASTER || defined(_ONE_NET_EVAL) //

#if defined(BLUE_SPOT_DEVICE)
        //! Timer used when powering on the Blue Spot PIC
        ONA_BS_PIC_POWER,

        //! Timer used when powering down the Blue Spot PIC
        ONA_SAVING_NV_DATA_TIMER,
#endif
   
    #ifdef _ONE_NET_EVAL
        ONT_FIRST_TXN_TIMER = ONT_M_TXN_START > ONT_C_TXN_START
          ? ONT_M_TXN_START : ONT_C_TXN_START,
    #else // ifdef _ONE_NET_EVAL //
        ONT_FIRST_TXN_TIMER,
    #endif // else _ONE_NET_EVAL is not defined //

    //! The total number of timers in use
    ONT_NUM_TIMERS = ONT_FIRST_TXN_TIMER + ONT_TXN_TIMER_COUNT
};

//! @} ONE-NET_TIMER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_TIMER_typedefs
//! \ingroup ONE-NET_TIMER
//! @{
    
//! @} ONE-NET_TIMER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_TIMER_pub_var
//! \ingroup ONE-NET_TIMER
//! @{

//! @} ONE-NET_TIMER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_TIMER_pub_func
//! \ingroup ONE-NET_TIMER
//! @{

BOOL ont_set_timer(const UInt8 TIMER, const tick_t DURATION);
tick_t ont_get_timer(const UInt8 TIMER);
BOOL ont_stop_timer(const UInt8 TIMER);

BOOL ont_active(const UInt8 TIMER);
BOOL ont_expired(const UInt8 TIMER);
BOOL ont_inactive_or_expired(const UInt8 TIMER);

//! @} ONE-NET_TIMER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_TIMER

#endif // _ONE_NET_TIMER_H //

