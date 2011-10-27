#ifndef _TICK_H
#define _TICK_H


//! \defgroup TICK Time keeping functionality.
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
    \file tick.h
    \brief Declarations for timing.

    Data types and constants associated with timing are declared in this file.
    A tick.c implementation file should be written for each processor type.
*/

#include "one_net_types.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup TICK_const
//! \ingroup TICK
//! @{


#define INIT_TICK() init_tick()
#define MS_TO_TICK(MS) ms_to_tick(MS)
#define TICK_TO_MS(NUM_TICKS) tick_to_ms(NUM_TICKS)
#define ENABLE_TICK_TIMER() enable_tick_timer()
#define DISABLE_TICK_TIMER() disable_tick_timer()
#define POLLED_TICK_UPDATE() polled_tick_update()
#define TICK_DIFF(THEN, NOW) tick_diff(THEN, NOW)
#define SET_TICK_COUNT(NEW_TICK_COUNT) set_tick_count(NEW_TICK_COUNT)
#define INCREMENT_TICK_COUNT(INCREMENT) increment_tick_count(INCREMENT)


//! @} TICK_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TICK_type_defs
//! \ingroup TICK
//! @{

//! @} TICK_type_def
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup TICK_pub_var
//! \ingroup TICK
//! @{

//! @} TICK_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION DECLARATIONS
//! \defgroup TICK_pub_func
//! \ingroup TICK
//! @{


/*!
    \brief Initialize the tick timer.

    \param void

    \return void
*/
void init_tick(void);


/*!
    \brief Converts milliseconds to ticks
    
    Integer math is used (as opposed to float).
    
    \param[in] ms The number of milliseconds to convert
    
    \return The number of ticks for the given ms value
*/
tick_t ms_to_tick(UInt32 ms);


/*!
    \brief Converts ticks to milliseconds
    
    Integer math is used (as opposed to float).
    
    \param[in] num_ticks The number of ticks to convert
    
    \return The number of milliseconds for the given num_ticks value
*/
UInt32 tick_to_ms(tick_t num_ticks);


/*!
    \brief Returns the current tick count.

    \param void

    \return The current tick count.
*/
tick_t get_tick_count(void);


/*!
    \brief Computes the time difference of 2 tick values.

    \param[in] then The base time.
    \param[in] now The current time.

    \return The difference between then & then.
*/
tick_t get_tick_diff(tick_t then, tick_t now);


/*!
    \brief Allows external adjustment of the tick count when the tick count
           needs to be reset, either due to the tick interrupt being turned
           off due to processor sleep or for some either reason (i.e. timer
           pausing, application code override, etc.

    \param new_tick_count The new tick count

    \return void
*/
void set_tick_count(tick_t new_tick_count);


/*!
    \brief Allows external increment of the tick count when the tick count
           needs to be reset, either due to the tick interrupt being turned
           off due to processor sleep or for some either reason (i.e. timer
           pausing, application code override, etc.

    \param increment The number of ticks to add to the current tick count.

    \return void
*/
void increment_tick_count(tick_t increment);



/*!
    \brief Delay for a specified number of milliseconds.


    \param[in] count The number of ms to delay for.

    \return void
*/
void delay_ms(UInt16 count);


/*!
    \brief Delay for a specified number of 100s of micro seconds.

    \param[in] count The number of 100s of us to delay for.

    \return void
*/
void delay_100s_us(UInt16 count);


/*!
    \brief Polls the tick ir line to see if the tick count needs to be updated.

    This should only be called when the tick interrupt (or global interrupts)
    have been disabled.

    \param void

    \return void
*/
void polled_tick_update(void);


/*!
    \brief Disables the tick interrupt

    \param void

    \return void
*/
void disable_tick_timer(void);


/*!
    \brief Enables the tick interrupt

    \param void

    \return void
*/
void enable_tick_timer(void);








//! @} TICK_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

#endif // _TICK_H //
