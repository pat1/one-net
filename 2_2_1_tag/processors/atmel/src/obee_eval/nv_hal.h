#ifndef _NV_EVAL_HAL_H
#define _NV_HAL_H

#include "config_options.h"


#ifdef _NON_VOLATILE_MEMORY


#include "one_net_types.h"


//! \defgroup nv_hal The ONE-NET Evaluation Project Hardware Abstraction Layer for non-volatile memory.
//! @{

/*!
    \file nv_hal.h
    \brief The ONE-NET Evaluation Project hardware abstraction layer for non-volatile memory.
*/


//=============================================================================
//                                  CONSTANTS
//! \defgroup nv_hal_const
//! \ingroup nv_hal
//! @{



//! @} nv_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup nv_hal_typedefs
//! \ingroup nv_hal
//! @{


/*!
    \brief Checks to see if the data flash should be erased (and erases it if
      it should -- i.e. erase if a "shorting device" is attached to the uart port).

    Checks if the uart rx & tx pins are connected to indicate that the flash
    should be erased (i.e. a person has physically attached a shorting plus
    that does this)

    \param void

    \return void
*/
#define FLASH_ERASE_CHECK() flash_erase_check()


//! @} nv_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup nv_hal_pub_var
//! \ingroup nv_hal
//! @{



//! @} nv_hal_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS

//! \defgroup nv_hal_pub_func
//! \ingroup nv_hal
//! @{



void flash_erase_check(void);
BOOL eval_erase_data_flash(void);
#ifndef _ATXMEGA256A3B
BOOL eval_load(const UInt8 NV_DATA_TYPE, UInt16 * const len,
  const UInt8 ** const DATA);
#else
BOOL eval_load(const UInt8 NV_DATA_TYPE, UInt16 * const len,
  const UInt16 ** const DATA);
#endif
BOOL eval_save(void);



//! @} nv_hal_pub_func

//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} nv_hal


#endif // #ifdef _NON_VOLATILE_MEMORY //

#endif // _NV_HAL_H //
