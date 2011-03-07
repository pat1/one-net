#ifndef _ONE_NET_EVAL_HAL_H
#define _ONE_NET_EVAL_HAL_H

//! \defgroup ont_net_eval_hal ONE-NET Evaluation Hardware Abstraction Layer
//! @{

/*!
    \file one_net_eval_hal.h
    \brief The ONE-NET evaluation project hardware abstraction layer.
*/

#include "one_net_types.h"
#include "tick.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup one_net_eval_hal_const
//! \ingroup one_net_eval_hal
//! @{

enum
{
    //! The interval in ticks to automatically send data when in auto mode. 1s
    AUTO_INTERVAL = MS_TO_TICK(1000),

    //! The timeout after a user switches the mode switch before the device
    //! goes back to automatically sending commands (AUTO_INTERVAL). 20s
    AUTO_MANUAL_DELAY = MS_TO_TICK(20000),
};

// constants for the sniffer
enum
{
    //! Number of ticks of inactivity before the prompt is displayed. 5s
    PROMPT_PERIOD = MS_TO_TICK(5000),
    
    //! Number of ticks to wait after user input has been detected before
    //! sniffing the channel again.  5s
    USER_INPUT_PAUSE_TIME = MS_TO_TICK(5000)
};

enum
{
    //! The maximum number of user pins
    NUM_USER_PINS = 4,
    
    //! Number of peers the MASTER keeps track of
    NUM_MASTER_PEER = 4
};

//! @} one_net_eval_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_eval_hal_typedefs
//! \ingroup one_net_eval_hal
//! @{


//! @} one_net_eval_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup one_net_eval_hal_pub_var
//! \ingroup one_net_eval_hal
//! @{

//! @} one_net_eval_hal_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_eval_hal_pub_func
//! \ingroup one_net_eval_hal
//! @{

/*!
    \brief Checks to see if the data flash should be erased (and erases it if
      it does).
    
    \param void
    
    \return void
*/
#define FLASH_ERASE_CHECK() flash_erase_check()


// This needs to be defined somewhere else.
extern BOOL get_user_pin_type(UInt8 * user_pin_type, const UInt8 NUM_PINS);

void flash_erase_check(void);
BOOL eval_erase_data_flash(void);
BOOL eval_load(const UInt8 NV_DATA_TYPE, UInt16 * const len,
  const UInt8 ** const DATA);
BOOL eval_save(void);
UInt8 * oncli_get_param(void);

//! @} one_net_eval_hal_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} one_net_eval_hal

#endif // _ONE_NET_EVAL_HAL_H //

