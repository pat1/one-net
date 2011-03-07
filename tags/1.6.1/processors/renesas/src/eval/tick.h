#ifndef _TICK_H
#define _TICK_H

//! \defgroup TICK Time keeping functionality.
//! \ingroup RENESAS_R8C
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
    \file tick.h
    \brief Declarations for timing.

    Data types and constants associated with timing are declared in this file.

    \note Threshold Corporation
*/

#include "one_net_types.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup TICK_const
//! \ingroup TICK
//! @{

//! The maximum value of a tick
#define TICK_MAX 4294967295

enum
{
    TICK_1MS = 1,                   //!< Number of ticks in a millisecond
    TICK_1S = 1000                  //!< Number of ticks in a second
};

enum
{
    //! Number of times to loop through nop loop during the ms delay function.
    NOP_COUNT_MS = 325,

    //! Number of times to loop through nop loop during the 100s of us delay
    //! function
    NOP_COUNT_100S_US = 45
};

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

void init_tick(void);

/*!
    \brief Initialize the tick timer.

    \param void

    \return void
*/
#define INIT_TICK() init_tick()


/*!
    \brief Converts milliseconds to ticks
    
    Integer math is used (as opposed to float).
    
    \param[in] MS The number of milliseconds to convert
    
    \return The number of ticks for the given ms value
*/
#define MS_TO_TICK(MS) ((MS) / TICK_1MS)


/*!
    \brief Converts ticks to milliseconds

    Integer math is used (as opposed to float).

    \param[in] TICK The number of ticks to convert
    
    \return The number of milliseconds for the given tick value
*/
#define TICK_TO_MS(TICK) ((TICK) * TICK_1MS)


/*!
    \brief Enables the tick interrupt

    \param void

    \return void
*/
#define ENABLE_TICK_TIMER() tstart_trbcr = 1


/*!
    \brief Disables the tick interrupt

    \param void

    \return void
*/
#define DISABLE_TICK_TIMER() tstart_trbcr = 0


/*!
    \brief Polls the tick ir line to see if the tick count needs to be updated.

    This should only be called when the tick interrupt (or global interrupts)
    have been disabled.

    \param void

    \return void
*/
#define POLLED_TICK_UPDATE() polled_tick_update()


/*!
    \brief Computes the time difference of 2 tick values.

    \param[in] THEN The base time.
    \param[in] NOW The current time.

    \return The difference between THEN & NOW.
*/
#define TICK_DIFF(THEN, NOW) (NOW < THEN ? (TICK_MAX - THEN) + NOW : NOW - THEN)


tick_t get_tick_count(void);

void delay_ms(UInt16 count);
void delay_100s_us(UInt16 count);

//! @} TICK_pub_func
//                          PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

#endif // _TICK_H //
