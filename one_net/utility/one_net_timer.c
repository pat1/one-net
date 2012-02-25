//! \addtogroup ONE-NET_TIMER
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
    \file one_net_timer.c
    \brief Timer Implementation used by ONE-NET.
    
    The module keeps track of the last time that it checked the tick count and
    decrements the difference between then and now from ALL the timers whenever
    any timer is checked.  Timers are countdown timers.  This module handles
    overflow of the tick timer.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


// Nov. 28, 2011 -- temporarily reverting JMR's timer changes.  Will
// re-implement them again when I get more of a chance to look at them
// and everything else is working.


#include "one_net_timer.h"
#include "one_net_timer_port_const.h"
#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_TIMER_const
//! \ingroup ONE-NET_TIMER
//! @{

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
    
    
//! Array to keep track of the timers.
ont_timer_t timer[ONT_NUM_TIMERS] = {0};



#ifdef _DEBUGGING_TOOLS
#include "one_net_port_const.h"
#include "one_net_master_port_const.h"


UInt32 one_net_response_time_out = ONE_NET_RESPONSE_TIME_OUT;
UInt32 write_pause = 0;
UInt32 one_net_master_invite_send_time = ONE_NET_MASTER_INVITE_SEND_TIME;
tick_t one_net_master_channel_scan_time = ONE_NET_MASTER_CHANNEL_SCAN_TIME;
UInt32 invite_transaction_timeout = INVITE_TRANSACTION_TIMEOUT;



UInt32* debug_intervals[NUM_DEBUG_INTERVALS] =
{
    &one_net_response_time_out, // 0
    &write_pause, // 1
    #ifdef _ONE_NET_MASTER
    &one_net_master_invite_send_time, // 2
    &one_net_master_channel_scan_time, // 3
    &invite_transaction_timeout, // 4
    #endif
};


const char* const debug_interval_strs[NUM_DEBUG_INTERVALS] =
{
    "Resp. Timeout",
    "Write Pause",
    #ifdef _ONE_NET_MASTER
    "Invite Send Time",
    "Channel Scan Time",
    #endif
    "Invite Trans. Timeout",    
};





BOOL pause = FALSE;
BOOL proceed = FALSE;
BOOL ratchet = FALSE;
BOOL pausing = FALSE;
#endif



//! @} ONE-NET_TIMER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================




//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_TIMER_pri_var
//! \ingroup ONE-NET_TIMER
//! @{


//! The last time the tick count was read and the timers were updated.
static tick_t last_tick = 0;


//! @} ONE-NET_TIMER_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_TIMER_pri_func
//! \ingroup ONE-NET_TIMER
//! @{

static void update_timers(void);

//! @} ONE-NET_TIMER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_TIMER_pub_func
//! \ingroup ONE-NET_TIMER
//! @{

/*!
    \brief Sets the timer to start counting.
    
    \param[in] TIMER The timer to set
    \param[in] DURATION The number of ticks to set the timer for.
    
    \return TRUE if the timer was set.
            FALSE if the timer was not set (ie doesn't exist)
*/
BOOL ont_set_timer(const UInt8 TIMER, const tick_t DURATION)
{
    if(TIMER >= ONT_NUM_TIMERS)
    {
        return FALSE;
    } // if the timer is invalid //

    update_timers();
    
    timer[TIMER].active = TRUE;
    timer[TIMER].tick = DURATION;
    
    return TRUE;
} // ont_set_timer //


/*!
    \brief Returns the timer remaining before the timer expires.
    
    \param[in] TIMER The timer to get the time remaining for.
    
    \return The time remaining.  0 willl be returned if it is not a valid
      timer or the timer is inactive.
*/
tick_t ont_get_timer(const UInt8 TIMER)
{
    if(TIMER >= ONT_NUM_TIMERS || !timer[TIMER].active)
    {
        return 0;
    } // if the timer is invalid //
    
    update_timers();

    return timer[TIMER].tick;
} // ont_get_timer //


/*!
    \brief Stops a timer.
    
    \param[in] TIMER The timer to stop
    
    \return TRUE if the timer was stopped.
            FALSE if the timer was not stopped (invalid timer).
*/
BOOL ont_stop_timer(const UInt8 TIMER)
{
    if(TIMER >= ONT_NUM_TIMERS)
    {
        return FALSE;
    } // if the timer is invalid //
    
    timer[TIMER].active = FALSE;
    timer[TIMER].tick = 0;
    
    return TRUE;
} // ont_stop_timer //


/*!
    \brief Returns whether or not the timer is active.
    
    \param[in] TIMER The timer whose status is being checked.
    
    \return TRUE if the timer is active.
            FALSE if the timer is not valid or inactive
*/
BOOL ont_active(const UInt8 TIMER)
{
    if(TIMER >= ONT_NUM_TIMERS)
    {
        return FALSE;
    } // if the timer is invalid //
    
    return timer[TIMER].active;
} // ont_active //


/*!
    \brief Returns whether or not the timer has expired.
    
    The timer is deactivated by this function if the timer has expired and TRUE
    is returned.
    
    \param[in] TIMER The timer who is being checked.
    
    \return TRUE if the timer has expired
            FALSE if the timer is not valid, is not active, or has not expired.
*/
BOOL ont_expired(const UInt8 TIMER)
{
    if(TIMER >= ONT_NUM_TIMERS || !timer[TIMER].active)
    {
        return FALSE;
    } // if the timer is invalid //
    
    update_timers();
    
    if(timer[TIMER].tick == 0)
    {
        timer[TIMER].active = FALSE;
    } // if the timer has expired //
    
    return timer[TIMER].tick == 0;
} // ont_expired //


/*!
    \brief Returns TRUE if the timer is inactive or has expired.
    
    \param[in] TIMER The timer who is being checked.
    
    \return TRUE if the timer is inactive or has expired
            FALSE if the timer is not valid, or is not active and has not
            expired.
*/
BOOL ont_inactive_or_expired(const UInt8 TIMER)
{
    if(TIMER >= ONT_NUM_TIMERS)
    {
        return FALSE;
    } // if the timer is invalid //
    
    update_timers();
    
    if(timer[TIMER].tick == 0)
    {
        timer[TIMER].active = FALSE;
    } // if the timer has expired //

    // If the timer expired, it was deactivated so the timer's active flag
    // will be FALSE if it was inactive or the timer expired
    return !timer[TIMER].active;
} // ont_inactive_or_expired //


#ifdef _DEBUGGING_TOOLS
#include "oncli.h"
void print_intervals(void)
{
    UInt8 i;
    for(i = 0; i < NUM_DEBUG_INTERVALS; i++)
    {
        oncli_send_msg("Interval %d(%s):\t%ld\n", i,
        debug_interval_strs[i], *(debug_intervals[i]));
        delay_ms(10);
    }
}


void print_timers(void)
{
    UInt8 i;
    const char* const active_state_str = "active\t";
    const char* const inactive_state_str = "inactive";
    for(i = 0; i < ONT_NUM_TIMERS; i++)
    {
        const char* const state_str = timer[i].active ? active_state_str :
            inactive_state_str;
        oncli_send_msg("Timer %d:\t%s\t%ld\n", i, state_str, timer[i].tick);
        delay_ms(10);
    }
}


void synchronize_last_tick(void)
{
    last_tick = get_tick_count();
}
#endif


//! @} ONE-NET_TIMER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_TIMER_pri_func
//! \ingroup ONE-NET_TIMER
//! @{

/*!
    \brief Updates the timers
    
    \param void
    
    \return void
*/
static void update_timers(void)
{
    tick_t tick_diff, tick_now;

    UInt8 i;
    
    tick_now = get_tick_count();
    
    if(tick_now < last_tick)
    {
        // Subtract from 0 since going from all 1s to 0 takes a tick
        tick_diff = ((tick_t)0 - last_tick) + tick_now;
    } // if rollover has occurred //
    else
    {
        tick_diff = tick_now - last_tick;
    } // else rollover has not occured //
    
    if(!tick_diff)
    {
        return;
    } // if the time hasn't changed //
    
    last_tick = tick_now;
    
    for(i = 0; i < ONT_NUM_TIMERS; i++)
    {
        #ifdef _DEBUGGING_TOOLS
        if(pausing && i != WRITE_PAUSE_TIMER && i >= ONT_NUM_APP_TIMERS)
        {
            // pausing everything but the APP timers
            continue;
        }
        #endif
        
        if(timer[i].active)
        {
            if(timer[i].tick > tick_diff)
            {
                timer[i].tick -= tick_diff;
            } // if more time remains //
            else
            {
                timer[i].tick = 0;
            } // else the timer has expired //
        } // if the timer is active //
    } // loop to adjust the timers //
} // update_timers //


//! @} ONE-NET_TIMER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_TIMER
