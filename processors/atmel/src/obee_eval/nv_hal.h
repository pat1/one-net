#ifndef _NV_EVAL_HAL_H
#define _NV_HAL_H

#include "config_options.h"


#ifdef NON_VOLATILE_MEMORY


#include "one_net_types.h"


//! \defgroup nv_hal The ONE-NET Evaluation Project Hardware Abstraction Layer for non-volatile memory.
//! @{

/*!
    \file nv_hal.h
    \brief The ONE-NET Evaluation Project hardware abstraction layer for non-volatile memory.
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
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



BOOL eval_erase_data_eeprom(void);
BOOL eval_save(void);



//! @} nv_hal_pub_func

//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} nv_hal


#endif // #ifdef NON_VOLATILE_MEMORY //

#endif // _NV_HAL_H //
