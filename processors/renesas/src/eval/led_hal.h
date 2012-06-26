#ifndef _LED_HAL_H
#define _LED_HAL_H


#include "config_options.h"


#ifdef HAS_LEDS

#include "io_port_mapping.h"
#include "hal.h"



//! \defgroup led_hal ONE-NET Evaluation Hardware Abstraction Layer for LEDs
//! @{

/*!
    \file one_net_eval_hal.h
    \brief ONE-NET Evaluation Hardware Abstraction Layer for LEDs.
*/


//=============================================================================
//                                  CONSTANTS
//! \defgroup led_hal_const
//! \ingroup led_hal
//! @{



//! @} one_net_eval_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup led_hal_typedefs
//! \ingroup led_hal
//! @{


#define INIT_PORTS_LEDS() hal_init_ports_leds()


/*!
    \brief Turns on a LED

    \param[in] LED The LED pin to turn on

    \return void
*/
#define LED_TURN_ON(LED) TURN_ON(LED)


/*!
    \brief Turns off a LED

    \param[in] LED The LED pin to turn off

    \return
*/
#define LED_TURN_OFF(LED) TURN_OFF(LED)


/*!
    \brief Toggles an LED

    \param[in] LED The LED pin to turn off

    \return
*/
#define LED_TOGGLE(LED) TOGGLE(LED)


//! @} led_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup led_hal_pub_var
//! \ingroup led_hal
//! @{

//! @} led_hal_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup led_hal_pub_func
//! \ingroup led_hal
//! @{


/*!
    \brief Initializes the ports used by LEDs

    \param void

    \return void
*/
void hal_init_ports_leds(void);


//! @} led_hal_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} led_hal


#endif // ifdef HAS_LEDS //


#endif // _LED_HAL_H //
