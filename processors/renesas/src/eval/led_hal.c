//! \addtogroup led_hal ONE-NET Evaluation Hardware Abstraction Layer
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
    \file led_hal.c
    \brief The ONE-NET evaluation project hardware abstraction layer for LEDs.
*/



#include "led_hal.h"
#include "tick.h"
#include "one_net_application.h" // for "INPUT" and "OUTPUT"



//=============================================================================
//                                  CONSTANTS
//! \defgroup led_hal_const
//! \ingroup led_hal
//! @{

//! @} led_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup led_hal_typedefs
//! \ingroup led_hal
//! @{



//! @} led_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup led_hal_pri_var
//! \ingroup led_hal
//! @{



//! @} led_hal_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup led_hal_pri_func
//! \ingroup led_hal
//! @{


//! @} led_hal_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC VARAIBLES DECLARATIONS
//! \defgroup led_hal_pub_data
//! \ingroup led_hal
//! @{



//! @} led_hal_pub_data
//                      PUBLIC VARAIBLES DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup led_hal_pub_func
//! \ingroup led_hal
//! @{



/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void hal_init_ports_leds(void)
{
    prc2 = 1; // protect off
    LED2_DIR = OUTPUT;
    prc2 = 0; // protect on    
    prc2 = 1; // protect off
    LED3_DIR = OUTPUT;
    prc2 = 0; // protect on    
    
    LED_TURN_OFF(LED2);
    LED_TURN_OFF(LED3);
} // init_ports_leds //



//! @} led_hal_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup led_hal_pri_func
//! \ingroup led_hal
//! @{

//! @} led_hal_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} led_eval_hal
