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
#include "one_net_application.h"
#include "one_net_timer.h"
#include "io_port_mapping.h"
#include "tick.h"
#include "nv_hal.h"
#include "hal.h"
#include "one_net_eval.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_master_port_specific.h"
#include "dfi.h"
#include "oncli_port.h"
#include "oncli.h"
#include "oncli_str.h"


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



//! The state of handling the user pins the MASTER is in;
static UInt8 master_user_pin_state;

//! The source unit of the user pin that has changed
static UInt8 user_pin_src_unit;



//! @} ONE-NET_master_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================


//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================




//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



static void init_base_param(on_base_param_t *base_param);
#ifdef _AUTO_MODE
static void send_auto_msg(void);
#endif
static void initialize_default_master_pins(void);
static void init_master_user_pin(const UInt8 *user_pin_type,
  UInt8 user_pin_count);
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
    
    
#ifdef _NON_VOLATILE_MEMORY
one_net_status_t one_net_master_save_settings(void)
{
    return ONS_SUCCESS;
} // one_net_master_save_settings //


one_net_status_t one_net_master_load_settings(void)
{
    return ONS_FAIL;
} // one_net_master_load_settings //


one_net_status_t one_net_master_erase_settings(void)
{
    return (eval_erase_data_flash() ? ONS_SUCCESS : ONS_FAIL);
} // one_net_master_erase_settings //
#endif // ifdef _NON_VOLATILE_MEMORY //    
    


void one_net_master_device_is_awake(const on_raw_did_t *DID)
{
} // one_net_master_device_is_awake //


void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t* KEY, const on_raw_did_t *CLIENT_DID)
{
    if(!KEY)
    {
        oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
          &one_net_master_invite_result);
        return;
    }

    switch(STATUS)
    {
        case ONS_SUCCESS:
        {
            if(!CLIENT_DID)
            {
                oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
                  &one_net_master_invite_result);
            } // if the parameters are invalid //
            else
            {
                oncli_send_msg(ONCLI_DEVICE_ADD_FMT, &((*KEY)[0]),
                  &((*KEY)[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
                  did_to_u16(CLIENT_DID));
            } // else the parameter is valid //
            break;
        } // success case //
        
        case ONS_TIME_OUT:
        {
            oncli_send_msg(ONCLI_DEVICE_NOT_ADDED_FMT,
              &((*KEY)[0]), &((*KEY)[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              "timed out.");
            break;
        } // timeout case // 
               
        case ONS_CANCELED:
        {
            oncli_send_msg(ONCLI_DEVICE_NOT_ADDED_FMT,
              &((*KEY)[0]), &((*KEY)[ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
              "cancelled.");
            break;
        } // cancelled case //

        default:
        {
            oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
              &one_net_master_invite_result);
            break;
        } // default case //
    } // switch(STATUS) //
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
    UInt8 i;
    master_param->client_count = NUM_AUTO_CLIENTS;
    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID +
      NUM_AUTO_CLIENTS * ON_CLIENT_DID_INCREMENT;
    for(i = 0; i < NUM_AUTO_CLIENTS; i++)
    {
        on_encode(client_list[i].device_send_info.did,
          RAW_AUTO_CLIENT_DID[i], ON_ENCODED_DID_LEN);
        client_list[i].device_send_info.features = THIS_DEVICE_FEATURES;
        client_list[i].device_send_info.data_rate = ONE_NET_DATA_RATE_38_4;
        #ifdef _ONE_NET_MULTI_HOP
        client_list[i].device_send_info.hops = 0;
        client_list[i].device_send_info.max_hops =
          features_max_hops(THIS_DEVICE_FEATURES);
        #endif
        client_list[i].flags = ON_JOINED;
        client_list[i].use_current_key = TRUE;
        #ifdef _STREAM_MESSAGES_ENABLED
        client_list[i].use_current_stream_key = TRUE;
        #endif
        
        client_list[i].device_send_info.expected_nonce = ON_INVALID_NONCE;
        client_list[i].device_send_info.last_nonce = ON_INVALID_NONCE;
        client_list[i].device_send_info.send_nonce = ON_INVALID_NONCE;
        client_list[i].device_send_info.msg_id = ON_MAX_MSG_ID + 1; // invalid
    }
    
    one_net_master_init(NULL, 0);
    
    // initialize timer for auto mode to send right away
    ont_set_timer(AUTO_MODE_TIMER, 0);
} // init_auto_master //
#endif


/*!
    \brief Initializes the device as a MASTER in serial mode

    \return void
*/
void init_serial_master(void)
{
    const UInt8 * MASTER_PARAM;
    UInt16 master_len;

    if(eval_load(DFI_ST_ONE_NET_MASTER_SETTINGS, &master_len, &MASTER_PARAM))
    {
    } // if previous settings were stored //
    else
    {
        one_net_master_reset_master(); // start a brand new network
    } // else start a new network //    
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
    #ifdef _AUTO_MODE
    if(in_auto_mode)
    {
        send_auto_msg();
    }
    #endif
    master_user_pin();
    one_net_master();
} // master_eval //


#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_master_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack)
#else
on_message_status_t one_net_master_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops)
#endif
{
#ifndef _ONE_NET_MULTI_HOP
    return eval_handle_single(raw_pld, msg_hdr, src_did, repeater_did,
      ack_nack);
#else
    return eval_handle_single(raw_pld, msg_hdr, src_did, repeater_did,
      ack_nack, hops, max_hops);
#endif
}


#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_master_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries)
#else
on_message_status_t one_net_master_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries,
  UInt8 hops, UInt8* const max_hops)
#endif
{
    #ifndef _ONE_NET_MULTI_HOP
    return eval_handle_ack_nack_response(raw_pld, msg_hdr, resp_msg_hdr,
      resp_ack_nack, src_did, repeater_did, retries);
    #else
    return eval_handle_ack_nack_response(raw_pld, msg_hdr, resp_msg_hdr,
      resp_ack_nack, src_did, repeater_did, retries, hops, max_hops);
    #endif    
}


one_net_status_t one_net_master_reset_master(void)
{
    one_net_status_t status;
    one_net_xtea_key_t key, stream_key;
    initialize_default_master_pins();
    
    #ifdef _STREAM_MESSAGES_ENABLED
    if(one_net_master_create_network(&DEFAULT_RAW_SID, &EVAL_KEY,
      ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32, &EVAL_STREAM_KEY,
      ONE_NET_STREAM_ENCRYPT_XTEA8) == ONS_SUCCESS)
    #else
    if(one_net_master_create_network(&DEFAULT_RAW_SID, &EVAL_KEY,
      ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32) == ONS_SUCCESS)
    #endif
    {    
        return ONCLI_SUCCESS;
    } // if creating the network was successful //

    return ONS_SUCCESS;
}


#ifndef _ONE_NET_MULTI_HOP
void one_net_master_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack)
#else
void one_net_master_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack, SInt8 hops)
#endif
{
    #ifndef _ONE_NET_MULTI_HOP
    eval_single_txn_status(status, retry_count, msg_hdr, data,
      dst, ack_nack);
    #else
    eval_single_txn_status(status, retry_count, msg_hdr, data,
      dst, ack_nack, hops);
    #endif    
}


void one_net_master_update_result(one_net_mac_update_t update,
  const on_raw_did_t* did, const on_ack_nack_t* ack_nack)
{
    const char * result_status;
    const char * result_type;
    const char * result_fmt = ONCLI_UPDATE_RESULT_FMT;
    
    if(ack_nack->nack_reason == ON_NACK_RSN_NO_ERROR)
    {
        result_status = ONCLI_SUCCEEDED_STR;
    } // if update was successful //
    else
    {
        result_status = ONCLI_FAILED_STR;
    } // else update failed //
    
    if(!did)
    {
        #ifdef _STREAM_MESSAGES_ENABLED
        if(update == ONE_NET_UPDATE_NETWORK_KEY || update ==
          ONE_NET_UPDATE_STREAM_KEY)
        #else
        if(update == ONE_NET_UPDATE_NETWORK_KEY)
        #endif
        {
            result_fmt = ONCLI_UPDATE_RESULT_WITH_OUT_DID_FMT;
        } // if the update network key //
        else
        {
            return;
        } // else the parameters are invalid //
    } // if the DID is not valid //

    switch(update)
    {
        case ONE_NET_UPDATE_DATA_RATE:
            result_type = ONCLI_M_UPDATE_RESULT_DATA_RATE_STR;
            break;
        case ONE_NET_UPDATE_NETWORK_KEY:
            result_type = ONCLI_M_UPDATE_RESULT_KEY_STR;
            break;
        #ifdef _PEER
        case ONE_NET_UPDATE_ASSIGN_PEER:
            result_type = ONCLI_M_UPDATE_RESULT_ASSIGN_PEER_STR;
            break;
        case ONE_NET_UPDATE_UNASSIGN_PEER:
            result_type = ONCLI_M_UPDATE_RESULT_UNASSIGN_PEER_STR;
            break;
        #endif
        case ONE_NET_UPDATE_REPORT_TO_MASTER:
            result_type = ONCLI_M_UPDATE_RESULT_REPORT_TO_MASTER_STR;
            break;
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_UPDATE_FRAGMENT_DELAY:
            result_type = ONCLI_M_UPDATE_RESULT_FRAG_STR;
            break;
        #endif
        case ONE_NET_UPDATE_KEEP_ALIVE:
            result_type = ONCLI_M_UPDATE_RESULT_KEEP_ALIVE_STR;
            break;
        case ONE_NET_UPDATE_SETTINGS:
            result_type = ONCLI_M_UPDATE_RESULT_SETTINGS_STR;
            break;
        case ONE_NET_UPDATE_REMOVE_DEVICE:
            result_type = ONCLI_M_UPDATE_RESULT_RM_DEV_STR;
            break;
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_UPDATE_STREAM_KEY:
            result_type = ONCLI_M_UPDATE_RESULT_STREAM_KEY_STR;
            break;
        #endif
        default:
            return; // bad parameter
    }

    if(did)
    {
        oncli_send_msg(result_fmt, result_type, did_to_u16(did),
          result_status);
    }
    else
    {
        oncli_send_msg(result_fmt, result_type, result_status);
    }
} // one_net_master_update_result //


BOOL one_net_master_client_missed_check_in(on_client_t* client)
{
    on_raw_did_t raw_did;
    on_decode(raw_did, client->device_send_info.did, ON_ENCODED_DID_LEN);
    oncli_send_msg(ONCLI_CLIENT_MISS_CHECK_IN_FMT, did_to_u16(&raw_did));
    return FALSE;
}


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
    
    This functions sets pins 0 and 1 as inputs and pins 2 and 3 as output
    Clients have a default of 0 and 1 as outputs and 2 and 3 as inputs.
    These can all be changed, but allows for quick commands without having
    to set pin directions on the Eval Board.

    \return void
*/
static void initialize_default_master_pins(void)
{
    oncli_set_user_pin_type(0, ON_INPUT_PIN);
    oncli_set_user_pin_type(1, ON_INPUT_PIN);
    oncli_set_user_pin_type(2, ON_OUTPUT_PIN);
    oncli_set_user_pin_type(3, ON_OUTPUT_PIN);
    master_user_pin_state = M_CHECK_USER_PIN;
}


/*!
    \brief Initializes the parameters used with the master user pins.
    
    \param[in] USER_PIN_TYPE List containing the state of the user pins.  If
      this is 0, then the default configuration will be used.
    \param[in] USER_PIN_COUNT The number of pins to configure.  This should be
      equal to the number of user pins, or else the default configuration will
      be used.
    
    \return void
*/
static void init_master_user_pin(const UInt8 *user_pin_type,
  UInt8 user_pin_count)
{
    master_user_pin_state = M_CHECK_USER_PIN;
    
    if(user_pin_type && user_pin_count == NUM_USER_PINS)
    {
        UInt8 i;

        for(i = 0; i < NUM_USER_PINS; i++)
        {
            oncli_set_user_pin_type(i, user_pin_type[i]);
        } // loop to set the user pin type //
    } // if the pins should be configured a certain way //
    else
    {
        disable_user_pins();
    } // else use the default pin configuration //
} // init_master_user_pin //






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
    if(oncli_user_input())
    {
        ont_set_timer(AUTO_MODE_TIMER, MS_TO_TICK(AUTO_MANUAL_DELAY));
        return;
    } // if there is user input //

    if(ont_expired(AUTO_MODE_TIMER))
    {
        UInt8 i;
        on_encoded_did_t* src_did = (on_encoded_did_t*)
          (&(on_base_param->sid[ON_ENCODED_NID_LEN]));
        
        UInt8 raw_pld[ONA_SINGLE_PACKET_PAYLOAD_LEN];
        
        // each client gets a different text message
        char* auto_messages[NUM_AUTO_CLIENTS] = {"11", "22", "33"};

        for(i = 0; i < /*NUM_AUTO_CLIENTS*/1; i++)
        {
            send_simple_text_command(auto_messages[i], ONE_NET_DEV_UNIT,
              ONE_NET_DEV_UNIT, &ENC_AUTO_CLIENT_DID[i]);
        }
        ont_set_timer(AUTO_MODE_TIMER, MS_TO_TICK(AUTO_INTERVAL));
    } // if the timer has expired //
} // send_auto_msg //
#endif


/*!
    \brief Checks to see if the state of any of the user pins changed
    
    \param void
    
    \return void
*/
static void master_check_user_pins(void)
{
    if(user_pin[0].pin_type == ON_INPUT_PIN
      && USER_PIN0 != user_pin[0].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 0;
        user_pin[0].old_state = USER_PIN0;
    } // if the user0 pin has been toggled //
    else if(user_pin[1].pin_type == ON_INPUT_PIN
      && USER_PIN1 != user_pin[1].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 1;
        user_pin[1].old_state = USER_PIN1;
    } // if the user1 pin has been toggled //
    else if(user_pin[2].pin_type == ON_INPUT_PIN
      && USER_PIN2 != user_pin[2].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 2;
        user_pin[2].old_state = USER_PIN2;
    } // if the user2 pin has been toggled //
    else if(user_pin[3].pin_type == ON_INPUT_PIN
      && USER_PIN3 != user_pin[3].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 3;
        user_pin[3].old_state = USER_PIN3;
    } // if the user3 pin has been toggled //
} // master_check_user_pins //


/*!
    \brief Sends the user pin state to the assigned peers
    
    \param void
    
    \return void
*/
static void master_send_user_pin_input(void)
{
    oncli_send_msg(ONCLI_CHANGE_PIN_STATE_FMT, user_pin_src_unit,
      user_pin[user_pin_src_unit].old_state);
      
    send_switch_status_change_msg(user_pin_src_unit, 
      user_pin[user_pin_src_unit].old_state, ONE_NET_DEV_UNIT, NULL);

    master_user_pin_state = M_CHECK_USER_PIN;
} // master_send_user_pin_input //


/*!
    \brief Checks the user pins and sends messages if the state has changed.
    
    \param void
    
    \return void
*/
static void master_user_pin(void)
{
    switch(master_user_pin_state)
    {
        case M_SEND_USER_PIN_INPUT:
        {
            master_send_user_pin_input();
            break;
        } // M_USER_PIN_SEND_INPUT state //

        default:
        {
            master_check_user_pins();
            break;
        } // default case //
    } // switch(master_user_pin_state) //
} // master_user_pin //



//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_master_eval


#endif // ifdef _ONE_NET_MASTER //
