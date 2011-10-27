#ifndef _HAL_H
#define _HAL_H


//! \defgroup HAL Processor abstraction layer.
//! \ingroup HAL
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
        implement all functions here.
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup HAL_const
//! \ingroup HAL
//! @{


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


//! @} HAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_typedefs
//! \ingroup HAL
//! @{

enum
{
    INPUT = 0,                      //!< Value when setting a pin as an input
    OUTPUT = 1                      //!< Value when setting a pin as an output
};

//! @} HAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup HAL_pub_var
//! \ingroup HAL
//! @{

//! @} HAL_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup HAL_pub_func
//! \ingroup HAL
//! @{

 
#define INIT_PORTS_UART() hal_init_ports_uart()
#define INIT_PORTS_LEDS() hal_init_ports_leds()


/*!
    \brief Initialize the ports used by the uart.

    \param void

    \return void
*/
void hal_init_ports_uart(void);


/*!
    \brief Initialize the ports used by the LEDs.

    \param void

    \return void
*/
void hal_init_ports_leds(void);


//! @} HAL_pub_func
//!                     PUBLIC FUNCTION DECLARATIONS END
//==============================================================================


//! @} HAL

#endif // _HAL_H //
