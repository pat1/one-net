#ifndef _ONE_NET_EVAL_H
#define _ONE_NET_EVAL_H



//! \defgroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.h
    \brief The ONE-NET evaluation project.

    Declarations for the ONE-NET evaluation project.
*/


#include "one_net_application.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{


/*!
    \brief Holds the functionality and state for the user pins.
*/
typedef struct
{
    on_pin_state_t pin_type;        //!< Functionality type
    UInt8 old_state;                //!< The last state of the pin
} user_pin_t;


//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{

extern user_pin_t user_pin[NUM_USER_PINS];

//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{


void disable_user_pins(void);
void init_user_pin(const UInt8 *user_pin_type, UInt8 user_pin_count);


//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE_NET_eval

#endif // _ONE_NET_EVAL_H //
