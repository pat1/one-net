#ifndef _PAL_H
#define _PAL_H

//! \defgroup PAL_EVAL Processor abstraction layer for r8c1 family of processors.
//! \ingroup PAL
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    \file pal.h (for r8c23)
    \brief Contains Renesas R8C hadrware specific declarations.

    The interface declarations in this file are written in a way that attempts
    to be independent of certain details of the the underlying Renesas hardware.
    The goal is that any higher level code using the interface declarations in
    this file should not have to change when porting to a differnet hardware
    platform if the two platforms similar in  nature.
*/

#include "sfr_r823.h"

#include <stdlib.h>

#include "io_port_mapping.h"
#include "one_net_types.h"
#include "tick.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup PAL_EVAL_const
//! \ingroup PAL_EVAL
//! @{

//! Using const (ROM) causes variables to be placed in program memory
#ifndef ROM
    #define ROM const
#endif

enum
{
    INPUT = 0,                      //!< Value when setting a pin as an input
    OUTPUT = 1                      //!< Value when setting a pin as an output
};


//! The threshold for low batter indication.
#define LOW_BATTERY_THRESHOLD BATTERY_THRESHOLD

//! The threshold for low voltage indication
#define LOW_VOLTAGE_THRESHOLD VOLTAGE_THRESHOLD

extern const UInt16 BATTERY_THRESHOLD;
extern const UInt16 VOLTAGE_THRESHOLD;

//! @} PAL_EVAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_EVAL_typedefs
//! \ingroup PAL_EVAL
//! @{

//! @} PAL_EVAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup PAL_EVAL_pub_var
//! \ingroup PAL_EVAL
//! @{

#ifdef _DEBUG
    extern UInt8 global_interrupts_on;
#endif // ifdef _DEBUG //

//! @} PAL_EVAL_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup PAL_EVAL_pub_func
//! \ingroup PAL_EVAL
//! @{

void init_ports(void);
void init_ports_leds(void);
void high_speed_mode(void);
void low_speed_mode(void);

void processor_sleep(const UInt32 MS);


/*!
    \brief Converts a string to a long.
    
    This macro is here because of the R8C library declared some of the pointers
    as _far pointers.  Other processors don't know about different pointer
    sizes, so this keeps the processor independance of the files where strtol
    is used.  This project is not using  far pointers, so use the near/near
    strtol that is provided in the Renesas library.
*/
#define strtol(s, endptr, base) _n_n_strtol((s), (endptr), (base))


/*!
    \brief Initialize all the ports on the processor

    \param void

    \return void
*/
#define INIT_PORTS()                init_ports()


/*!
    \brief Initialize the ports used by the uart.

    \param void

    \return void
*/
#define INIT_PORTS_UART()           init_ports_uart()


/*!
    \brief Initialize the ports used by the LEDs.

    \param void

    \return void
*/
#define INIT_PORTS_LEDS()           init_ports_leds()


/*!
    \brief Initialize the processor speed.

    \param void

    \return void
*/
#define INIT_PROCESSOR()            high_speed_mode()


/*!
    \brief Enable global interrupts

    \param void

    \return void
*/
#ifndef _DEBUG
    #define ENABLE_GLOBAL_INTERRUPTS()  asm("FSET I")
#else
    #define ENABLE_GLOBAL_INTERRUPTS()  \
    {                                   \
        asm("FSET I");                  \
        global_interrupts_on = 1;       \
    } // ENABLE_GLOBAL_INTERRUPTS //
#endif // if not defined _DEBUG //


/*!
    \brief Disables global interrupts

    \param void

    \return void
*/
#ifndef _DEBUG
    #define DISABLE_GLOBAL_INTERRUPTS() asm("FCLR I")
#else
    #define DISABLE_GLOBAL_INTERRUPTS() \
    {                                   \
        asm("FCLR I");                  \
        global_interrupts_on = 0;       \
    } // DISABLE_GLOBAL_INTERRUPTS //
#endif // if not defined _DEBUG //


/*!
    \brief Reads the battery status

    \param void

    \return void
*/
#define READ_BATTERY_STATUS() read_battery()


/*!
    \brief Reads the voltage status.

    \param void

    \return void
*/
#define READ_LINE_VOLTAGE_STATUS() read_adc()


/*!
    \brief Turns on a LED

    \param[out] led The LED pin to set

    \return void
*/
#define TURN_ON(led)    led = 1


/*!
    \brief Turns off a LED

    \param[out] led The LED pin to clear

    \return
*/
#define TURN_OFF(led)   led = 0


/*!
    \brief Toggles a pin

    \param[out] pin The pin to toggle

    \return void
*/
#define TOGGLE(pin)     pin = !pin


/*!
    \brief Puts the micro controller to sleep.

    The micro controller will sleep until an interrupt occurs or until ms
    milliseconds elapses.  Pass 0 to sleep only until an interrupt occurs.

    \param[in] MS The number of milliseconds to sleep for

    \return void
*/
#define SLEEP(MS) processor_sleep(MS);


/*!
    \brief Exits the program.

    Disables global interrupts & calls the WAIT assembly instruction to stop
    program execution.

    \param void

    \return void
*/
#define EXIT()  asm("FCLR I"); asm("WAIT")

//! @} PAL_EVAL_pub_func
//!                     PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} PAL_EVAL

#endif // _PAL_H //

