//! \defgroup ONE-NET_master_eval ONE-NET MASTER Evaluation
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file master_eval.c
    \brief The MASTER part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#ifdef _ONE_NET_MASTER


#include "one_net_status_codes.h"
#include "one_net_constants.h"
#include "one_net.h"
#include "one_net_master.h"
#include "tick.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_master_eval_const
//! \ingroup ONE-NET_master_eval
//! @{

//! State machine for dealing with the user pins.
enum
{
    M_CHECK_USER_PIN,               //!< State to check user pins for changes
    M_SEND_USER_PIN_INPUT           //!< State to send user pin changes to peers
};



//! The key used in the evaluation network ("protected")
static const one_net_xtea_key_t DEFAULT_EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

#ifdef _STREAM_MESSAGES_ENABLED
//! The key to use for stream transactions
static const one_net_xtea_key_t EVAL_STREAM_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
#endif

//! Default SID to use if no NID is found in the manufacturing data segment
//! of data flash.
static const UInt8 DEFAULT_RAW_SID[] =        {0x00, 0x00, 0x00, 0x00, 0x10};



//! @} ONE-NET_master_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_master_eval_typedefs
//! \ingroup ONE-NET_master_eval
//! @{

//! @} ONE-NET_master_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================




//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_master_eval_pub_var
//! \ingroup ONE-NET_master_eval
//! @{



//! True if in Auto Mode
extern BOOL in_auto_mode; // declared in one_net_eval.c



//! @} ONE-NET_master_eval_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================




//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_master_eval_pri_var
//! \ingroup ONE-NET_master_eval
//! @{



//! @} ONE-NET_master_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================


//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



void disable_user_pins(void);
#ifdef _AUTO_MODE
void init_auto_master(void);
#endif
void init_serial_master(void);



//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================




//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



static void init_base_param(on_base_param_t *base_param);
static void init_master_user_pin(const UInt8 *user_pin_type, 
                                       UInt8 user_pin_count);
#ifdef _AUTO_MODE
static void send_auto_msg(void);
#endif
static void initialize_master_pins(void);
static void master_check_user_pins(void);
static void master_send_user_pin_input(void);
static void master_user_pin(void);



//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_master_eval_pub_func
//! \ingroup ONE-NET_master_eval
//! @{


void one_net_master_device_is_awake(const on_raw_did_t *DID)
{
} // one_net_master_device_is_awake //


void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t KEY, const on_raw_did_t *CLIENT_DID)
{
} // one_net_master_invite_result //


BOOL one_net_master_remove_device_result(const on_raw_did_t *DID,
  BOOL SUCCEEDED)
{
    return TRUE;
} // one_net_master_remove_device_result //


#ifdef _AUTO_MODE
/*!
    \brief Initializes the device as a MASTER in auto mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param BASE_PARAM The base parameters.

    \return void
*/
void init_auto_master(void)
{
} // init_auto_master //
#endif


/*!
    \brief Initializes the device as a MASTER in serial mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param EVAL_SID_IDX Index of the SID to use.

    \return void
*/
void init_serial_master(void)
{
} // init_serial_master //


/*!
    \brief The MASTER evaluation application
    
    This is the main function for the MASTER evaluation.  In auto mode, the
    MASTER will automatically send a text message to the CLIENTS every X
    seconds if there has been no input through the oncli.

    \param void

    \return void
*/
void master_eval(void)
{
    // empty function right now.  Just print something to prove we're here.
    static UInt8 delay_counter = 0;
    delay_ms(10);
    if(delay_counter == 0)
    {
        oncli_send_msg("Currently in master_eval() function\n");
    }
    delay_counter++;

    #ifdef _AUTO_MODE
    if(in_auto_mode)
    {
        send_auto_msg();
    }
    #endif
    master_user_pin();
    one_net_master();
} // master_eval //


//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_master_eval
//! @{




/*!
    \brief Initializes the pins of the master to the default directions and values
    
    \return void
*/
static void initialize_master_pins(void)
{
}


/*!
    \brief Initializes the base parameters for the evaluation network.
    
    \param[out] base_param The base parameters to initialize.  All base
      parameters except the SID are initialized.
    
    \return void
*/
static void init_base_param(on_base_param_t* base_param)
{
} // init_base_param //


#ifdef _AUTO_MODE
/*!
    \brief Automatically sends a message when in auto mode.
    
    This function also checks the time interval before sending the message.
    
    \param void
    
    \return void
*/
static void send_auto_msg(void)
{
} // send_auto_msg //
#endif


/*!
    \brief Checks to see if the state of any of the user pins changed
    
    \param void
    
    \return void
*/
static void master_check_user_pins(void)
{
} // master_check_user_pins //


/*!
    \brief Sends the user pin state to the assigned peers
    
    \param void
    
    \return void
*/
static void master_send_user_pin_input(void)
{
} // master_send_user_pin_input //


/*!
    \brief Checks the user pins and sends messages if the state has changed.
    
    \param void
    
    \return void
*/
static void master_user_pin(void)
{
} // master_user_pin //



//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_master_eval


#endif // ifdef _ONE_NET_MASTER //
