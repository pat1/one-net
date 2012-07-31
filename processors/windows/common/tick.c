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
#include "one_net_types.h"
#include <windows.h>


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



//! The maximum value of a tick
#define TICK_MAX   0xFFFFFFFF
//! 1 tick per millisecond
#define TICK_1MS 1
//! There are 10,000 100-nanosecond intervals in 1 millisecond
#define HUNDRED_NANO_PER_MILLI 10000


//! @} TICK_type_defs
//                                  TYPEDEFS_END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TICK_pri_var
//! \ingroup TICK
//! @{


static uint64_t time0;
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


static uint64_t calculate_start_time_from_tick_count(tick_t tick_count);
static void update_tick_count(void);
static uint64_t FILETIME_to_uint64_t(FILETIME ft);
// Commenting out since this function is not called from anywhere and we can't compile
// with warnings treated as errors if we leave it in.  Might be useful, so simply commenting
// it out rather than deleting it.
#if 0
static FILETIME uint64_t_to_FILETIME(uint64_t hundred_nano_since_1601);
#endif



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
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    time0 = FILETIME_to_uint64_t(ft);
} // init_tick //


tick_t ms_to_tick(UInt32 ms)
{
    return ms * TICK_1MS;
} // ms_to_tick //


UInt32 tick_to_ms(tick_t num_ticks)
{
    return num_ticks / TICK_1MS;
} // tick_to_ms //


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


static uint64_t FILETIME_to_uint64_t(FILETIME ft)
{
    uint64_t hundred_nano_since_1601 = ft.dwHighDateTime;
    hundred_nano_since_1601 <<= 32;
    hundred_nano_since_1601 += ft.dwLowDateTime;
    return hundred_nano_since_1601;
}


// Commenting out since this function is not called from anywhere and we can't compile
// with warnings treated as errors if we leave it in.  Might be useful, so simply commenting
// it out rather than deleting it.
#if 0
static FILETIME uint64_t_to_FILETIME(uint64_t hundred_nano_since_1601)
{
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)(hundred_nano_since_1601 & 0xFFFFFFFF);
    ft.dwHighDateTime = (DWORD)(hundred_nano_since_1601 >> 32);
    return ft;
}
#endif


static uint64_t calculate_start_time_from_tick_count(tick_t tick_count)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t current_time = FILETIME_to_uint64_t(ft);
    uint64_t tick_count_100_nano = HUNDRED_NANO_PER_MILLI * ((uint64_t) tick_to_ms(tick_count));
    return current_time - tick_count_100_nano;
}


static void update_tick_count(void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t hundred_nano_since_1601 = FILETIME_to_uint64_t(ft);
    uint64_t elapsed_100_nano = hundred_nano_since_1601 - time0;
    UInt32 elapsed_ms = (UInt32)(elapsed_100_nano / HUNDRED_NANO_PER_MILLI);
    tick_count = ms_to_tick(elapsed_ms);
}


//! @} TICK_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} TICK
