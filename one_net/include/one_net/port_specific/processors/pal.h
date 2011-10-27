#ifndef _PAL_H
#define _PAL_H


//! \defgroup PAL Processor abstraction layer.
//! \ingroup PAL
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
    \file pal.h
    \brief Contains common hardware declarations.  Implementation will be
        processor-specific and possibly hardware-specific.  Not all ports will
        implment all functions here.
*/


#include "one_net_types.h"



//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_const
//! \ingroup PAL
//! @{

//! @} PAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_typedefs
//! \ingroup PAL
//! @{

//! @} PAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup PAL_pub_var
//! \ingroup PAL
//! @{

//! @} PAL_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup PAL_pub_func
//! \ingroup PAL
//! @{

#define INIT_PORTS() init_ports()
#define INIT_PROCESSOR(SPEED) init_processor(SPEED)
#define ENABLE_GLOBAL_INTERRUPTS() enable_global_interrupts()
#define DISABLE_GLOBAL_INTERRUPTS() disable_global_interrupts()
#define SLEEP(MS) processor_sleep(MS)
#define EXIT() one_net_exit()


/*!
    \brief Initialize all the ports on the processor

    \param void

    \return void
*/
void init_ports(void);


/*!
    \brief Initialize the processor speed.

    \param high_speed True if initializing to high speed, false if low speed

    \return void
*/
void init_processor(BOOL high_speed);


/*!
    \brief Set processor speed to high speed.

    \return void
*/
void high_speed_mode(void);


/*!
    \brief Set processor speed to low speed.

    \return void
*/
void low_speed_mode(void);


/*!
    \brief Enable global interrupts

    \param void

    \return void
*/
void enable_global_interrupts(void);


/*!
    \brief Disables global interrupts

    \param void

    \return void
*/
void disable_global_interrupts(void);


/*!
    \brief Puts the microcontroller to sleep.

    The micro controller will sleep until an interrupt occurs or until ms
    milliseconds elapses.  Pass 0 to sleep only until an interrupt occurs.

    \param[in] ms The number of milliseconds to sleep for

    \return void
*/
void processor_sleep(UInt32 ms);


/*!
    \brief Exits the program.

    Disables global interrupts & stops program execution.

    \param void

    \return void
*/
void one_net_exit(void);


//! @} PAL_pub_func
//!                     PUBLIC FUNCTION DECLARATIONS END
//==============================================================================


//! @} PAL

#endif // _PAL_H //
