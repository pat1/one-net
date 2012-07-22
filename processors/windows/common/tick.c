//! addtogroup TICK
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
    \file tick.c
    \brief Windows specific timing module.

    This module contains functionality associated with timing.
*/


#include "tick.h"
#include <Winsock2.h> // for struct timeval


//==============================================================================
//                                  CONSTANTS
//! \defgroup TICK_const
//! \ingroup TICK
//! @{



#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif



//! @} TICK_const
//                                  CONSTANTS
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TICK_type_defs
//! \ingroup TICK
//! @{


#define TICK_MAX   0xFFFFFFFF         //! The maximum value of a tick
#define TICK_1MS   1000               //!< Number of ticks in a millisecond
#define TICK_100MS 10000              //!< Number of ticks in 100 microseconds

//! @} TICK_type_defs
//                                  TYPEDEFS_END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TICK_pri_var
//! \ingroup TICK
//! @{


static struct timeval time0;
static tick_t tick_count = 0;
static BOOL tick_enabled = FALSE;


//! @} TICK_pri_var
//                              PRIVATE VARIABLES
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{


static BOOL timeval_greater(struct timeval time1, struct timeval time2);
static tick_t elapsed_ticks(struct timeval start_time, struct timeval end_time);
static struct timeval calculate_start_time_from_tick_count(tick_t tick_count);
static void update_tick_count(void);



//! @} TICK_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TICK_pub_func
//! \ingroup TICK
//! @{
    

void init_tick(void)
{
    tick_enabled = TRUE;
    tick_count = 0;
    gettimeofday(&time0, NULL);
} // init_tick //
    

tick_t ms_to_tick(UInt32 ms)
{
    return ms * TICK_1MS;
} // tick_to_ms //
    

UInt32 tick_to_ms(tick_t num_ticks)
{
    return num_ticks / TICK_1MS;
} // ms_to_tick //


tick_t get_tick_count(void)
{
    update_tick_count();
    return tick_count;
} // get_tick_count //


tick_t get_tick_diff(tick_t now, tick_t then)
{
    if(now < then)
    {
        return get_tick_diff(then,now);
    }
    return now - then;
} // get_tick_diff //


void set_tick_count(tick_t new_tick_count)
{
    tick_count = new_tick_count;
    time0 = calculate_start_time_from_tick_count(tick_count);
}


void increment_tick_count(tick_t increment)
{
    tick_count += increment;
    time0 = calculate_start_time_from_tick_count(tick_count);
}


void delay_ms(UInt16 count)
{
    tick_t current_tick_count = get_tick_count();
    tick_t ending_tick_count = current_tick_count + MS_TO_TICK(count);
    while(get_tick_count() < ending_tick_count)
    {
    }
} // delay_ms //


void delay_100s_us(UInt16 count)
{
    tick_t current_tick_count = get_tick_count();
    tick_t ending_tick_count = current_tick_count + count * (1000000 / 10000);
    while(get_tick_count() < ending_tick_count)
    {
    }
} // delay_100us //


void enable_tick_timer(void)
{
    if(tick_enabled)
    {
        update_tick_count();
        return;
    }
    calculate_start_time_from_tick_count(tick_count);
    tick_enabled = TRUE;
}


void disable_tick_timer(void)
{
    if(!tick_enabled)
    {
        return;
    }
    update_tick_count();
    tick_enabled = FALSE;
}


//! @} TICK_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup TICK_pri_func
//! \ingroup TICK
//! @{


static BOOL timeval_greater(struct timeval time1, struct timeval time2)
{
    if(time1.tv_sec > time2.tv_sec)
    {
        return TRUE;
    }
    else if(time1.tv_sec < time2.tv_sec)
    {
        return FALSE;
    }
    else
    {
        return (time1.tv_usec > time2.tv_usec);
    }
}


static tick_t elapsed_ticks(struct timeval start_time, struct timeval end_time)
{
    struct timeval time_diff = {0, 0};

    if(timeval_greater(start_time, end_time))
    {
        return 0; // no negative times
    }

    time_diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
    if(end_time.tv_usec < start_time.tv_usec)
    {
        time_diff.tv_sec--;
        end_time.tv_usec += 1000000;
    }
    time_diff.tv_usec = end_time.tv_usec - start_time.tv_usec;
    return (tick_t)(time_diff.tv_sec * 1000000 + time_diff.tv_usec);
}


static struct timeval calculate_start_time_from_tick_count(tick_t tick_count)
{
    // TODO -- not a problem here since a tick is defined as exactly 1
    // microsecond, intentionally in order to make the math easy with
    // struct timeval, but the formula below will not work if a tick
    // was defined as anything else. Consider using the TICK_1MS and / or
    // the TICK_1S values and creating a more generic conversion formula
    // rather than hardcoding 1 million.

    struct timeval current_time, start_time;
    gettimeofday(&current_time, NULL);
    current_time.tv_sec--;
    current_time.tv_usec += 1000000;
    start_time.tv_sec = current_time.tv_sec - (tick_count / 1000000);
    start_time.tv_usec = current_time.tv_usec - (tick_count % 1000000);
    if(start_time.tv_usec >= 1000000)
    {
        start_time.tv_sec++;
        start_time.tv_usec -= 1000000;
    }
    return start_time;
}


static void update_tick_count(void)
{
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    tick_count = elapsed_ticks(time0, time_now);
}


//! @} TICK_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} TICK
