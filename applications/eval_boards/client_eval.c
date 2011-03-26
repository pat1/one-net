//! \defgroup ONE-NET_client_eval ONE-NET CLIENT Evaluation
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file client_eval.c
    \brief The CLIENT part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#include "one_net_client_net.h"
#include "one_net_client_port_specific.h"
#include "one_net_crc.h"
#include "io_port_mapping.h"

#ifdef _ENABLE_CLI
	#include "oncli.h"
	#include "oncli_port.h"
	#include "oncli_str.h"
#endif

#include "one_net_eval.h"
#include "one_net_eval_hal.h"
#include "pal.h"

#ifdef _ENABLE_CLIENT_PING_RESPONSE
    #include "uart.h"
#endif

//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_client_eval_const
//! \ingroup ONE-NET_client_eval
//! @{

extern const one_net_xtea_key_t EVAL_KEY;

const ona_unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES]
  = {{ONA_SIMPLE_SWITCH, ONE_NET_NUM_UNITS}};

#ifdef _ENABLE_CLIENT_PING_RESPONSE
    extern UInt8 client_send_ping_response;
    extern const UInt8 CLIENT_PING_REQUEST[];
    extern const UInt8 CLIENT_PING_REPONSE[];

#endif


//! @} ONE-NET_client_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_client_eval_typedefs
//! \ingroup ONE-NET_client_eval
//! @{

//! @} ONE-NET_client_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_client_eval_pri_var
//! \ingroup ONE-NET_client_eval
//! @{

// from one_net_eval
extern user_pin_t user_pin[NUM_USER_PINS];

//! @} ONE-NET_client_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_client_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

// forward declarations
// "derived" from one_net_eval
void disable_user_pins(void);

static void initialize_client_pins_for_demo(void);

extern BOOL client_joined_network;
one_net_raw_did_t client_did;

// "protected" functions
#ifdef _AUTO_MODE
	void init_auto_client(node_select_t CLIENT);
#endif
oncli_status_t set_device_type(UInt8 device_type);

static void client_check_user_pins(void);

static void initialize_client_pins_for_demo(void)
{
    oncli_set_user_pin_type(0, ONCLI_OUTPUT_PIN);
    oncli_set_user_pin_type(1, ONCLI_OUTPUT_PIN);
    oncli_set_user_pin_type(2, ONCLI_INPUT_PIN);
    oncli_set_user_pin_type(3, ONCLI_INPUT_PIN);
}


//! @} ONE-NET_client_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_client_eval_pub_func
//! \ingroup ONE-NET_client_eval
//! @{
oncli_status_t oncli_reset_client(void)
{
    oncli_status_t status;
    one_net_xtea_key_t * invite_key;

    if((status = set_device_type(CLIENT_NODE)) != ONCLI_SUCCESS)
    {
        return status;
    } // if could not set the device to a CLIENT //

#ifndef _SERIAL_ASSIGN_DEMO_PINS
    disable_user_pins();
#else
    initialize_client_pins_for_demo(); // 1,2 are outputs, 3,4 are inputs
#endif

    // get the unique invite code for this device
    invite_key = (one_net_xtea_key_t *) get_invite_key();

#ifdef _STREAM_MESSAGES_ENABLED
  #ifdef _ENHANCED_INVITE
      one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE),
        eval_encryption(ON_STREAM), 0, ONE_NET_MAX_CHANNEL, 0);
  #else
      one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE),
        eval_encryption(ON_STREAM));
  #endif
#else
  #ifdef _ENHANCED_INVITE
      one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE),
        0, ONE_NET_MAX_CHANNEL, 0);
  #else
      one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE));
  #endif
#endif

    return ONCLI_SUCCESS;
} // oncli_reset_client //

void one_net_client_joined_network(const one_net_raw_did_t * const RAW_DID,
  const one_net_raw_did_t * const MASTER_DID)
{
    //
    // save data for use by the CLI
    //
    one_net_memmove(&client_did[0], RAW_DID, sizeof(one_net_raw_did_t));

    //
    // print the joined message
    //
    oncli_send_msg(ONCLI_JOINED_FMT, did_to_u16(RAW_DID));
    oncli_print_prompt();
} // one_net_client_joined_network //

#ifdef _ENHANCED_INVITE
void one_net_client_invite_cancelled(cancel_invite_reason_t reason)
{
    oncli_send_msg("Join command timed out.  Device not added.\n");
} // one_net_client_invite_cancelled //
#endif

/*#ifndef _ONE_NET_VERSION_2_X*/
BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    eval_handle_single(RX_PLD, RX_PLD_LEN, SRC_ADDR);
    return TRUE;
} // client handle_single_pkt //
/*#else
BOOL one_net_client_handle_single_pkt(ona_msg_class_t msg_class, ona_msg_type_t msg_type, 
         UInt8 src_unit, UInt8 dst_unit, UInt16* msg_data,
         const one_net_raw_did_t* const SRC_ADDR, BOOL* useDefaultHandling,
		 on_nack_rsn_t* nack_reason)
{
	return eval_handle_single(msg_class, msg_type, src_unit, dst_unit, msg_data,
         SRC_ADDR, useDefaultHandling, nack_reason);
} // client handle_single_pkt //
#endif*/



void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 RETRY_COUNT, const UInt8 * const DATA,
  const one_net_raw_did_t * const DST)
{
    eval_single_txn_status(STATUS, DATA, DST);
} // one_net_client_single_txn_status //


BOOL one_net_client_txn_requested(const UInt8 TYPE, const BOOL SEND,
  const UInt16 DATA_TYPE, const UInt16 DATA_LEN,
  const one_net_raw_did_t * const DID)
{
    return eval_txn_requested(TYPE, SEND, DATA_TYPE, DATA_LEN, DID);
} // one_net_client_txn_requested //


BOOL one_net_client_handle_block_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    print_packet(ONCLI_BLOCK_TXN_STR, PLD, LEN, SRC_ADDR);
    return TRUE;
} // one_net_client_handle_block_pkt //


void one_net_client_block_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
    eval_block_txn_status(STATUS, DID);
} // one_net_client_block_txn_status //


void one_net_client_stream_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
} // one_net_client_stream_txn_status //


BOOL one_net_client_handle_stream_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    return FALSE;
} // one_net_client_handle_stream_pkt //


const UInt8 * one_net_client_next_payload(const UInt8 TYPE, UInt16 * len,
  const one_net_raw_did_t * const DST)
{
    return eval_next_payload(TYPE, len, DST);
}  // one_net_client_next_payload //
      
      
void one_net_client_client_remove_device(void)
{
} // one_net_client_client_remove_device //

//! @} ONE-NET_client_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_client_eval_pri_func
//! \ingroup ONE-NET_client_eval
//! @{

/*!
    \brief Initializes the device as a CLIENT in auto mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param CLIENT The CLIENT_NODE this device is supposed to operate as.

    \return void
*/
#ifdef _AUTO_MODE
	void init_auto_client(node_select_t CLIENT)
	{
	    on_base_param_t * base_param;
	    on_master_t * master;
		
#ifdef _PEER
	    on_peer_t * peer;
#endif

	    // The parameters used to initialize an existing network as if they were
	    // stored in a continguous block of memory
		
#ifdef _PEER
	    UInt8 param[sizeof(on_base_param_t) + sizeof(on_master_t)
	      + sizeof(on_peer_t)];
#else
	    UInt8 param[sizeof(on_base_param_t) + sizeof(on_master_t)];
#endif
	    UInt8 i;

	    // check the parameters
	    if(CLIENT < AUTO_CLIENT1_NODE || CLIENT > AUTO_CLIENT3_NODE)
	    {
	        EXIT();
	    } // if any of the parameters are invalid //
    
	    // set the pointers
	    base_param = (on_base_param_t *)param;
	    master = (on_master_t *)(((UInt8 *)base_param) + sizeof(on_base_param_t));
		
#ifdef _PEER
	    peer = (on_peer_t *)(((UInt8 *)master) + sizeof(on_master_t));
#endif

	    // set up the base parameters
	    base_param->version = EVAL_PARAM_VERSION;
	    get_eval_encoded_nid((on_encoded_nid_t *)&(base_param->sid));
	    get_eval_encoded_did(CLIENT,
	      (on_encoded_did_t *)&(base_param->sid[ON_ENCODED_NID_LEN]));
	    base_param->channel = eval_channel();
	    base_param->data_rate = eval_data_rate(CLIENT);
	    get_eval_key(&(base_param->current_key));
	    base_param->single_block_encrypt = eval_encryption(ON_SINGLE);
#ifdef _STREAM_MESSAGES_ENABLED
	    get_eval_stream_key(&(base_param->stream_key));
	    base_param->stream_encrypt = eval_encryption(ON_STREAM);
#endif
	    base_param->fragment_delay_low = eval_fragment_delay(ONE_NET_LOW_PRIORITY);
	    base_param->fragment_delay_high
	      = eval_fragment_delay(ONE_NET_HIGH_PRIORITY);

	    // set up the on_master_t parameters
	    get_eval_encoded_did(MASTER_NODE, &(master->device.did));
	    master->device.expected_nonce = 0;
	    master->device.last_nonce = 0;
	    master->device.send_nonce = 0;
	    #ifdef _ONE_NET_MULTI_HOP
	    master->device.max_hops = 0;
	    #endif
	    master->keep_alive_interval = eval_keep_alive();
	    master->settings.master_data_rate = ONE_NET_DATA_RATE_38_4;
	    master->settings.flags = eval_client_flag(CLIENT);

#ifdef _PEER    
	    // set up the peer parameters
	    for(i = 0; i < ONE_NET_MAX_PEER_DEV; i++)
	    {
	        one_net_memmove(peer->dev[i].did, ON_ENCODED_BROADCAST_DID,
	          sizeof(peer->dev[i].did));
	    } // loop to initialize peer devices //

	    for(i = 0; i < ONE_NET_MAX_PEER_UNIT; i++)
	    {
            one_net_memmove(peer->unit[i].peer_did, ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
	        peer->unit[i].src_unit = ONE_NET_DEV_UNIT;
	        peer->unit[i].peer_unit = ONE_NET_DEV_UNIT;
	    }
#endif
    
	    base_param->crc = one_net_compute_crc((UInt8 *)base_param
	      + sizeof(base_param->crc), sizeof(param) - sizeof(base_param->crc),
	      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
	    // initialize the ONE-NET CLIENT
	    one_net_client_init(param, sizeof(param));
	} // init_auto_client //
#endif

/*!
    \brief Checks if any of the user pins have been enabled as inputs and sends
      a message if they have and the state of the pin has changed.

    \param void

    \return void

    dje: August 29, 2008
        Changed so that user_pin[0] (user pin 1) corresponds to src unit 1
                        user_pin[1] (user pin 2) corresponds to src unit 2
                        etc.
    rwm: December 14, 2008
        Changed so that user_pin[0] (user pin 0) corresponds to src unit 0
                        user_pin[1] (user pin 1) corresponds to src unit 1
                        etc.
 
*/
static void client_check_user_pins(void)
{
    if(user_pin[0].pin_type == ONCLI_INPUT_PIN
      && USER_PIN0 != user_pin[0].old_state)
    {
        if(send_switch_command(USER_PIN0 ? ONA_ON : ONA_OFF, 0,
          ONE_NET_DEV_UNIT, 0))
        {
            user_pin[0].old_state = USER_PIN0;
        } // if scheduling the transaction was successful //
    } // if the user1 pin has been toggled //
    else if(user_pin[1].pin_type == ONCLI_INPUT_PIN
      && USER_PIN1 != user_pin[1].old_state)
    {
        if(send_switch_command(USER_PIN1 ? ONA_ON : ONA_OFF, 1,
          ONE_NET_DEV_UNIT, 0))
        {
            user_pin[1].old_state = USER_PIN1;
        } // if scheduling the transaction was successful //
    } // if the user2 pin has been toggled //
    else if(user_pin[2].pin_type == ONCLI_INPUT_PIN
      && USER_PIN2 != user_pin[2].old_state)
    {
        if(send_switch_command(USER_PIN2 ? ONA_ON : ONA_OFF, 2,
          ONE_NET_DEV_UNIT, 0))
        {
            user_pin[2].old_state = USER_PIN2;
        } // if scheduling the transaction was successful //
    } // if the user3 pin has been toggled //
    else if(user_pin[3].pin_type == ONCLI_INPUT_PIN
      && USER_PIN3 != user_pin[3].old_state)
    {
        if(send_switch_command(USER_PIN3 ? ONA_ON : ONA_OFF, 3,
          ONE_NET_DEV_UNIT, 0))
        {
            user_pin[3].old_state = USER_PIN3;
        } // if scheduling the transaction was successful //
    } // if the user4 pin has been toggled //
} // client_check_user_pins //


/*!
    \brief The CLIENT evaluation application
    
    This is the main function for the CLIENT evaluation.  The CLIENT is
    initialized and ran from this function.
    
    \param void
    
    \return void
*/
void client_eval(void)
{
    client_check_user_pins();
    one_net_client();
#ifdef _ENABLE_CLIENT_PING_RESPONSE
    if (client_send_ping_response == TRUE)
    {
        one_net_status_t status;

        enum
        {
            SRC_UNIT = 0, 
            DST_UNIT = 0
        };

        UInt8 MASTER_DID[2] = {0x00, 0x10};

        //
        // send the client ping response (single data simple text message)
        //
        status = send_simple_text_command((UInt8 *)&(CLIENT_PING_REPONSE[0]), (UInt8) SRC_UNIT,
          (UInt8) DST_UNIT, (one_net_raw_did_t *) MASTER_DID);
        if (status != ONS_RSRC_FULL)
        {
            client_send_ping_response = FALSE;

        }
    }
#endif
} // client_eval //

//! @} ONE-NET_client_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_client_eval
