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
    application wants to use this module, it should set its first timer to
    NUM_ONE_NET_TIMERS and increment from that point.  The application must
    specify the number of timers used even if it does not use the timer
    functionality by defining the ONT_NUM_TIMERS variable to the total number of
    timers used.
    
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


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_TIMER_const
//! \ingroup ONE-NET_TIMER
//! @{


#ifdef _DEBUGGING_TOOLS
// Note : this will increase!
#ifdef _ONE_NET_MASTER
#define NUM_DEBUG_INTERVALS 5
#else
#define NUM_DEBUG_INTERVALS 3
#endif
#endif



enum
{
    //! General purpose timer
    ONT_GENERAL_TIMER = ONT_NUM_APP_TIMERS,
    
    //! Timer to check for a clear channel
    ONT_CLR_CHANNEL_TIMER,
    
    ONT_SINGLE_TIMER,
    
    ONT_RESPONSE_TIMER,
    
    ONT_INVITE_TIMER,

    #ifdef _ONE_NET_MASTER
    ONT_INVITE_SEND_TIMER,
    #endif

    #ifdef _BLOCK_MESSAGES_ENABLED
    ONT_BLOCK_TIMER,
    #endif
    
    #ifdef _STREAM_MESSAGES_ENABLED
    ONT_STREAM_TIMER,
    #endif

    #ifdef _ONE_NET_MH_CLIENT_REPEATER
    //! The number of multi-hop packets that need a timer.
    ONT_MH_TIMER,
    #endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

    #ifdef _ONE_NET_MASTER
    //! Timer used for key changes
    ONT_UPDATE_TIMER,
    #endif
    
    #ifdef _ONE_NET_CLIENT
    //! Timer for CLIENT to know when to send a Keep Alive
    ONT_KEEP_ALIVE_TIMER,

    #ifdef _DEVICE_SLEEPS
    //! Timer for the period the CLIENT should stay awake after receiving
    //! a Single Data ACK Stay Awake.
    ONT_STAY_AWAKE_TIMER,
    #endif // ifdef _DEVICE_SLEEPS //
    #endif // ifdef _ONE_NET_CLIENT //
    
    #ifdef _DEBUGGING_TOOLS
    WRITE_PAUSE_TIMER,
    #endif
    
    #ifdef _DATA_RATE
    ONT_DATA_RATE_TIMER,
    #endif

    //! The total number of timers in use
    ONT_NUM_TIMERS
};

//! @} ONE-NET_TIMER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_TIMER_typedefs
//! \ingroup ONE-NET_TIMER
//! @{


/*!
    \brief Represents a timer
*/
typedef struct
{
    BOOL active;                    //!< Flag to indicate if active(TRUE).
    tick_t tick;                    //!< number of ticks remaining
} ont_timer_t;

    
//! @} ONE-NET_TIMER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_TIMER_pub_var
//! \ingroup ONE-NET_TIMER
//! @{
    
    
//! Array to keep track of the timers.
extern ont_timer_t timer[];


// debugging tools for timers -- revert to #defines for chips NOT using these
// debugging tools.  If the debugging tool are used, these are adjustable
// variables that can be changed during run-time from a user interface.
#ifdef _DEBUGGING_TOOLS
extern UInt32* debug_intervals[];
extern UInt32 one_net_response_time_out; // 0
extern UInt32 write_pause; // 1
extern UInt32 one_net_master_invite_send_time; // 2
extern UInt32 one_net_master_channel_scan_time; // 3
extern UInt32 invite_transaction_timeout; // 4



// global variables used to pause and step-through code, adjustable via
// CLI
extern BOOL pause;
extern BOOL proceed;
extern BOOL ratchet;
extern BOOL pausing;

#else

// not using the debugging tools, but we've replaced the "constants" in the
// code so we could use them for the debugging tools.  Define those variables
// as constants with #define statements so things work like they are supposed
// to.
#define one_net_response_time_out ONE_NET_RESPONSE_TIME_OUT
#define invite_transaction_timeout INVITE_TRANSACTION_TIMEOUT
#ifdef _ONE_NET_MASTER
#define one_net_master_invite_send_time ONE_NET_MASTER_INVITE_SEND_TIME
#define one_net_master_channel_scan_time ONE_NET_MASTER_INVITE_SEND_TIME
#endif

#endif


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


void pause_timer(UInt8 TIMER);
void unpause_timer(UInt8 TIMER);





#ifdef _DEBUGGING_TOOLS
void print_timers(void);
void print_intervals(void);
void synchronize_last_tick(void);
#endif


//! @} ONE-NET_TIMER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_TIMER

#endif // _ONE_NET_TIMER_H //

