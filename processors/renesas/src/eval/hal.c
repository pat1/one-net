//! \defgroup HAL Hardware abstraction layer.
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
    \file hal.c
    \brief Contains common hardware declarations.  Implementation will be
        processor-specific and possibly hardware-specific.  Not all ports will
        implement all functions here.
*/


#include "hal.h"
#include "sfr_r823.h"
#include "io_port_mapping.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup HAL_const
//! \ingroup HAL
//! @{

//! @} HAL_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_typedefs
//! \ingroup HAL
//! @{


//! @} HAL_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup HAL_pri_var
//! \ingroup HAL
//! @{

//! @} HAL_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup HAL_pri_func
//! \ingroup HAL
//! @{

//! @} HAL_pri_func
//!                     PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup HAL_pub_func
//! \ingroup HAL
//! @{


/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void init_ports_leds(void)
{
    prc2 = 1;
    LED2_DIR = OUTPUT;
    LED3_DIR = OUTPUT;
    prc2 = 0;

    TURN_OFF(LED2);
    TURN_OFF(LED3);
} // init_ports_leds //


//! @} HAL_pub_func
//!                     PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup HAL_pri_func
//! \ingroup HAL
//! @{

//! @} HAL_pri_func
//!                     PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


//! @} HAL
