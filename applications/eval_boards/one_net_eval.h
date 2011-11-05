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
#include "one_net_port_const.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{



#ifdef _AUTO_MODE
extern const on_raw_did_t RAW_AUTO_CLIENT_DID[];
#endif

#if defined(_AUTO_MODE) || defined(_ONE_NET_MASTER)
extern const UInt8 DEFAULT_RAW_NID[];
#endif



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{
    
    
enum
{
    #ifdef _AUTO_MODE
    //! The number of clients in AUTO mode.
    NUM_AUTO_CLIENTS = 3
    #endif
};    


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
#ifdef _SNIFFER_MODE
extern BOOL in_sniffer_mode;
#endif

//! Pointer to the device dependent (MASTER, CLIENT, SNIFF) function that
//! should be called in the main loop
extern void(*node_loop_func)(void);

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
#ifdef _ONE_NET_MASTER
void master_eval(void); // in master_eval.c
#endif
#ifdef _ONE_NET_CLIENT
void client_eval(void); // in client_eval.c
#endif
#ifdef _SNIFFER_MODE
void sniff_eval(void); // in sniff_eval.c
#endif


//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE_NET_eval

#endif // _ONE_NET_EVAL_H //
