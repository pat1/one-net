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
#include "tick.h"
#include "nv_hal.h"
#include "hal.h"
#include "one_net_eval.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"


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


#if 1
// temporarily changing it so we send a 26 bit "turn switch on" message
// to 002 unit 3 in the 1.X strain format as a test to see if it can be
// sniffed by the 1.X sniffer.  This is an invalid 2.0 Beta packet, but
// a valid 2.0 Alpha and 1.X packet.  The packet was sniffed from a
// 2.0 Alpha packet.  The {0x55, 0x55, 0x55, 0x33} header has been added
// to the front.
#include "tal.h"
UInt8 send_sing_pkt[26] =
{
    0x55, 0x55, 0x55, 0x33, 0xB4, 0xB3,
    0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xBC,
    0xB4, 0xBC, 0xB9, 0x32, 0x93, 0x3C,
    0xD5, 0xD9, 0x33, 0x53, 0x32, 0x34,
    0xAA, 0x69
};


// see one_net.c -- temporarily making one_net.c static function global
BOOL check_for_clr_channel(void);
#endif
void master_eval(void)
{
    #if 1
    // temporarily changing it so we send a 26 bit "turn switch on" message
    // to 002 unit 3 in the 1.X strain format as a test to see if it can be
    // sniffed.
    while(1)
    {
        delay_ms(1000);
        while(!check_for_clr_channel())
        {
        }

        tal_write_packet(send_sing_pkt, 26);
        
        while(!tal_write_packet_done())
        {
        }
    }
    #endif
    

    
    
    
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
