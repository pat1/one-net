//! \defgroup ONE-NET_master_eval ONE-NET MASTER Evaluation
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file master_eval.c
    \brief The MASTER part of the ONE-NET evaluation project.
*/

#include "config_options.h"

#include "one_net_crc.h"
#include "io_port_mapping.h"
#include "one_net_master_port_specific.h"

#ifdef _ENABLE_CLI
	#include "oncli.h"
	#include "oncli_port.h"
	#include "oncli_str.h"
#endif
#include "one_net_encode.h"
#include "one_net_eval.h"
#include "one_net_eval_hal.h"
#include "one_net_timer.h"
#include "pal.h"
#include "dfi.h"
#include "one_net_peer.h"


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


extern const eval_sid_t EVAL_SID[];

// used by oncli_print_nid, defined in uart.c
extern const char HEX_DIGIT[];

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
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_master_eval_pri_var
//! \ingroup ONE-NET_master_eval
//! @{

// from one_net_eval
extern user_pin_t user_pin[NUM_USER_PINS];


//! The unique key of the device currently going through the invite process
static one_net_xtea_key_t invite_key;

//! The state of handling the user pins the MASTER is in;
static UInt8 master_user_pin_state;

//! The source unit of the user pin that has changed
static UInt8 user_pin_src_unit;

//! The index into the peer list for the given user pin
static UInt8 user_pin_peer_idx;

//! @} ONE-NET_master_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

// forward declarations
// "derived" from one_net_eval
oncli_status_t set_device_type(UInt8 device_type);
void disable_user_pins(void);
BOOL get_raw_master_did(one_net_raw_did_t *did);

// These are not static since it will be called from eval, but we don't want it
// called from anywhere else
#ifdef _AUTO_MODE
	void init_auto_master(void);
#endif
void init_serial_master(void);

static void init_base_param(on_base_param_t *base_param);
static void init_master_user_pin(const UInt8 *user_pin_type, 
                                       UInt8 user_pin_count);
static BOOL is_valid_eval_sid(const one_net_raw_sid_t *sid, UInt8 *i);
#ifdef _AUTO_MODE
	static void send_auto_msg(void);
#endif

void initialize_master_pins_for_demo(void);

static void master_check_user_pins(void);
static void master_send_user_pin_input(void);
static void master_user_pin(void);

void master_eval(void);

//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_master_eval_pub_func
//! \ingroup ONE-NET_master_eval
//! @{

oncli_status_t oncli_reset_master(void)
{
    oncli_status_t status;
    one_net_xtea_key_t key, stream_key;

    if((status = set_device_type(MASTER_NODE)) != ONCLI_SUCCESS)
    {
        return status;
    } // if could not set the device to a MASTER //
    
#ifdef _SERIAL_ASSIGN_DEMO_PINS
    initialize_master_pins_for_demo(); // 1,2 inputs; 3,4 outputs
#else
    init_master_user_pin(0, 0);
#endif

    init_master_peer();
    
    get_eval_key(&key);
    get_eval_stream_key(&stream_key);
    if(one_net_master_create_network(get_raw_sid(), &key,
      eval_encryption(ON_SINGLE), &stream_key, eval_encryption(ON_STREAM))
      == ONS_SUCCESS)
    {    
        return ONCLI_SUCCESS;
    } // if creating the network was successful //
    
    return ONCLI_INTERNAL_ERR;
} // oncli_reset_master //


oncli_status_t oncli_reset_master_with_channel(
  const one_net_raw_sid_t *SID, UInt8 CHANNEL)
{
    on_base_param_t * base_param;
    on_master_param_t * master_param;
    
    oncli_status_t status;

    UInt8 param[sizeof(on_base_param_t) + sizeof(on_master_param_t)];    
    UInt8 i;

    if(!SID)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //)

    if((status = set_device_type(MASTER_NODE)) != ONCLI_SUCCESS)
    {
        return status;
    } // if could not set the device to a CLIENT //
    
#ifdef _SERIAL_ASSIGN_DEMO_PINS
    initialize_master_pins_for_demo(); // 1,2 inputs; 3,4 outputs
#else
    init_master_user_pin(0, 0);
#endif
    init_master_peer();

    base_param = (on_base_param_t *)param;
    master_param = (on_master_param_t *)(param + sizeof(on_base_param_t));

    // set up the base parameters
    init_base_param(base_param);
    base_param->channel = CHANNEL;
    on_encode(base_param->sid, *SID, sizeof(base_param->sid));

    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID;
    master_param->client_count = 0;

    base_param->crc = one_net_compute_crc((UInt8 *)base_param
      + sizeof(base_param->crc), sizeof(param) - sizeof(base_param->crc),
      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
    one_net_master_init(param, sizeof(param));

    return ONCLI_SUCCESS;
} // oncli_reset_master_with_channel //


#ifdef _ENABLE_INVITE_COMMAND
oncli_status_t oncli_invite(const one_net_xtea_key_t *KEY)
{
#ifndef _SNIFFER_FRONT_END
    if(!KEY)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    // parse the return value from the request
    switch(one_net_master_invite(KEY))
    {
        case ONS_SUCCESS:
        {
            one_net_memmove(invite_key, *KEY, sizeof(invite_key));
            return ONCLI_SUCCESS;
            break;
        } // success case //

        case ONS_BAD_PARAM:         // fall through
        case ONS_INTERNAL_ERR:
        {
            // This function should not have passed bad parameters
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case //
        
        case ONS_DEVICE_LIMIT:      // fall through
        case ONS_RSRC_FULL:
        {
            return ONCLI_RSRC_UNAVAILABLE;
            break;
        } // resource full case //
        
        case ONS_ALREADY_IN_PROGRESS:
        {
            return ONCLI_ALREADY_IN_PROGRESS;
            break;
        } // already in progress case //

        case ONS_NOT_INIT:
        {
            return ONCLI_ONS_NOT_INIT_ERR;
            break;
        } // ONE-NET has not been initialized //

        default:
        {
            break;
        } // default case //
    } // switch(request_status) //

    // all return types should have been handled
    return ONCLI_SNGH_INTERNAL_ERR;
#else
    return ONCLI_INTERNAL_ERR;
#endif
} // oncli_invite //
#endif


#ifdef _ENABLE_CANCEL_INVITE_COMMAND
oncli_status_t oncli_cancel_invite(void)
{
    UInt8 i;

    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    one_net_master_cancel_invite(&invite_key);
    for(i = 0; i < sizeof(invite_key); i++)
    {
        invite_key[i] = 0x00;
    } // loop to clear the key //
    
    return ONCLI_SUCCESS;
} // oncli_cancel_invite //
#endif


#ifdef _ENABLE_ASSIGN_PEER_COMMAND
oncli_status_t oncli_assign_peer(const one_net_raw_did_t* const SRC_DID,
  UInt8 SRC_UNIT, const one_net_raw_did_t* const PEER_DID,
  UInt8 PEER_UNIT)
{
    one_net_raw_did_t master_raw_did;
    on_encoded_did_t enc_peer_did;

    if(!PEER_DID || !SRC_DID)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //

    if(get_raw_master_did(&master_raw_did)
      && mem_equal(master_raw_did, *SRC_DID, ONE_NET_RAW_DID_LEN))
    {
		one_net_status_t ret;
        on_encode(enc_peer_did, *PEER_DID, ON_ENCODED_DID_LEN);
		
#ifdef _ONE_NET_MULTI_HOP
        ret = master_assigned_peer(SRC_UNIT, &enc_peer_did, PEER_UNIT);
#else
        ret = master_assigned_peer(SRC_UNIT, &enc_peer_did, PEER_UNIT);
#endif		
        switch(ret)
		{
            case ONS_SUCCESS:
			{
				return ONCLI_SUCCESS;
			}
			case ONS_INVALID_DATA:
			{
				return ONCLI_INVALID_DST;
			}
			case ONS_RSRC_FULL:
			{
				return ONCLI_RSRC_UNAVAILABLE;
			}
				
			// default case.
			return ONCLI_INTERNAL_ERR;
		}
    } // if the destination is the MASTER //
    
    switch(one_net_master_peer_assignment(TRUE, PEER_DID, PEER_UNIT, SRC_DID,
      SRC_UNIT))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
		/*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */
		

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(one_net_master_peer_assignment) //
    
    return ONCLI_INTERNAL_ERR;
} // oncli_assign_peer //
#endif


#ifdef _ENABLE_UNASSIGN_PEER_COMMAND
oncli_status_t oncli_unassign_peer(const one_net_raw_did_t *PEER_DID,
  UInt8 peer_unit, const one_net_raw_did_t *SRC_DID,
  UInt8 src_unit)
{
    one_net_raw_did_t master_raw_did;
    on_encoded_did_t encoded_peer_did;

    if(!PEER_DID || !SRC_DID)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    if(get_raw_master_did(&master_raw_did)
      && mem_equal(master_raw_did, *SRC_DID, ONE_NET_RAW_DID_LEN))
    {
        one_net_status_t ret;
        on_encode(encoded_peer_did, *PEER_DID, ON_ENCODED_DID_LEN);
        ret = master_unassigned_peer(src_unit, &encoded_peer_did, peer_unit, TRUE);
		switch(ret)
		{
			case ONS_SUCCESS:
			{
				return ONCLI_SUCCESS;
			}
		}
		
		return ONCLI_INTERNAL_ERR;
    } // if the destination is the MASTER //

    switch(one_net_master_peer_assignment(FALSE, PEER_DID, peer_unit, SRC_DID,
      src_unit))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        

		/* case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */


        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(master_peer_assignment) //
    
    return ONCLI_INTERNAL_ERR;
} // oncli_unassign_peer //
#endif


#ifdef _ENABLE_UPDATE_MASTER_COMMAND
oncli_status_t oncli_set_update_master_flag(BOOL SET,
  const one_net_raw_did_t *DST)
{
    if(!DST)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    switch(one_net_master_set_update_master_flag(SET, DST))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
		

        /*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */


        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(one_net_master_set_update_master_flag) //

    return ONCLI_INTERNAL_ERR;
} // oncli_set_update_master_flag //
#endif


#ifdef _ENABLE_CHANGE_KEEP_ALIVE_COMMAND
oncli_status_t oncli_change_keep_alive(UInt32 KEEP_ALIVE,
  const one_net_raw_did_t *DST)
{
    if(!DST)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    switch(one_net_master_change_client_keep_alive(DST, KEEP_ALIVE))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        /*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(one_net_master_change_client_keep_alive) //

    return ONCLI_INTERNAL_ERR;
} // oncli_change_keep_alive //
#endif


#ifdef _ENABLE_CHANGE_FRAGMENT_DELAY_COMMAND
oncli_status_t oncli_change_frag_dly(const one_net_raw_did_t *DID,
  UInt8 PRIORITY, UInt32 DLY)
{
    if(!DID)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    switch(one_net_master_change_frag_dly(DID, PRIORITY, DLY))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        /*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */

        case ONS_INVALID_DATA:
        {
            return ONCLI_PARSE_ERR;
            break;
        } // invalid data case //

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_BAD_PARAM;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(master_change_client_frag_dly) //

    return ONCLI_INTERNAL_ERR;
} // oncli_change_frag_dly //
#endif


#ifdef _ENABLE_CHANGE_KEY_COMMAND
oncli_status_t oncli_change_key(
  const one_net_xtea_key_fragment_t *FRAGMENT)
{
    if(!FRAGMENT)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    switch(one_net_master_change_key(*FRAGMENT))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        case ONS_ALREADY_IN_PROGRESS:
        {
            return ONCLI_ALREADY_IN_PROGRESS;
            break;
        } // already in progress case //

        default:
        {
            break;
        } // default case //
    } // switch(master_change_key) //

    return ONCLI_INTERNAL_ERR;
} // oncli_change_key //
#endif


#ifdef _ENABLE_REMOVE_DEVICE_COMMAND
oncli_status_t oncli_remove_device(const one_net_raw_did_t *DST)
{
    if(!DST)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
#ifdef _AUTO_MODE
    if(mode_type() == AUTO_MODE)
    {
        return ONCLI_INVALID_CMD_FOR_MODE;
    } // if auto mode //
#endif
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    switch(one_net_master_remove_device(DST))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        /*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(one_net_master_remove_device) //

    return ONCLI_INTERNAL_ERR;
} // oncli_remove_device //
#endif


void one_net_master_device_is_awake(const one_net_raw_did_t *DID)
{
} // one_net_master_device_is_awake //


BOOL one_net_master_handle_single_pkt(const UInt8 *RX_PLD,
  UInt16 RX_PLD_LEN, const one_net_raw_did_t *SRC_ADDR)
{
    eval_handle_single(RX_PLD, RX_PLD_LEN, SRC_ADDR);
    return TRUE;
} // one_net_master_handle_single_pkt //


void one_net_master_single_txn_status(one_net_status_t STATUS,
  UInt8 RETRY_COUNT, const UInt8 *DATA,
  const one_net_raw_did_t *DST)
{
    eval_single_txn_status(STATUS, DATA, DST);
} // one_net_master_single_txn_status //


BOOL one_net_master_handle_block_pkt(const UInt8 *PLD, UInt16 LEN,
  const one_net_raw_did_t *SRC_ADDR)
{
    print_packet(ONCLI_BLOCK_TXN_STR, PLD, LEN, SRC_ADDR);
    return TRUE;
} // one_net_master_handle_block_pkt //


void one_net_master_block_txn_status(one_net_status_t STATUS,
                                     const one_net_raw_did_t *DID)
{
    eval_block_txn_status(STATUS, DID);
} // one_net_master_block_txn_status //


BOOL one_net_master_handle_stream_pkt(const UInt8 *PLD, UInt16 LEN,
  const one_net_raw_did_t *SRC_ADDR)
{
    return FALSE;
} // one_net_master_handle_stream_pkt //



void one_net_master_stream_txn_status(one_net_status_t STATUS,
  const one_net_raw_did_t *DID)
{
} // one_net_master_stream_txn_status //


const UInt8 * one_net_master_next_payload(UInt8 TYPE, UInt16 *len,
  const one_net_raw_did_t *DST)
{
    return eval_next_payload(TYPE, len, DST);
} // one_net_master_next_payload //


void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t KEY, const one_net_raw_did_t *CLIENT_DID)
{
    UInt8 i;
    
    // check that the key is correct
    for(i = 0; i < sizeof(invite_key); i++)
    {
        if(KEY[i] != invite_key[i])
        {
            oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
              &one_net_master_invite_result);
        } // if the key does not match //
    } // loop to make sure the key matches //

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
                oncli_send_msg(ONCLI_DEVICE_ADD_FMT, KEY, &(KEY[4]), 
                  did_to_u16(CLIENT_DID));
            } // else the parameter is valid //
            break;
        } // success case //
        
        case ONS_TIME_OUT:
        {
            oncli_send_msg(ONCLI_ADD_DEVICE_FAILED_FMT, KEY, &(KEY[4]));
            break;
        } // time out case //

        default:
        {
            oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
              &one_net_master_invite_result);
            break;
        } // default case //
    } // switch(STATUS) //

    oncli_print_prompt();
} // one_net_master_invite_result //


BOOL one_net_master_txn_requested(UInt8 TYPE, BOOL SEND,
  UInt16 DATA_TYPE, UInt16 DATA_LEN,
  const one_net_raw_did_t *DID)
{
    return eval_txn_requested(TYPE, SEND, DATA_TYPE, DATA_LEN, DID);
} // one_net_master_txn_requested //


void one_net_master_data_rate_result(const one_net_raw_did_t *SENDER,
  const one_net_raw_did_t *RECEIVER, UInt8 DATA_RATE,
  UInt8 SUCCESS_COUNT, UInt8 ATTEMPTS)
{
    oncli_send_msg(ONCLI_DATA_RATE_TEST_RESULT_FMT, did_to_u16(RECEIVER),
      SUCCESS_COUNT, ATTEMPTS, did_to_u16(SENDER), DATA_RATE);
    oncli_print_prompt();
} // one_net_master_data_rate_result //


void one_net_master_update_result(one_net_mac_update_t UPDATE,
  const one_net_raw_did_t *DID, BOOL SUCCEEDED)
{
    const char * RESULT_STR;
    
    if(SUCCEEDED)
    {
        RESULT_STR = ONCLI_SUCCEEDED_STR;
    } // if update was successful //
    else
    {
        RESULT_STR = ONCLI_FAILED_STR;
    } // else update failed //

    if(!DID)
    {
        if(UPDATE == ONE_NET_UPDATE_NETWORK_KEY)
        {
            oncli_send_msg(ONCLI_UPDATE_RESULT_WITH_OUT_DID_FMT,
              ONCLI_M_UPDATE_STR[UPDATE], RESULT_STR);
        } // if the update network key //
        else
        {
            return;
        } // else the parameters are invalid //
    } // if the DID is not valid //
    else if(UPDATE < ONCLI_ADMIN_STR_COUNT)
    {
        oncli_send_msg(ONCLI_UPDATE_RESULT_FMT,
          ONCLI_M_UPDATE_STR[UPDATE], did_to_u16(DID), RESULT_STR);
    } // else if the update index is valid //
    else
    {
        oncli_send_msg(ONCLI_UNKNOWN_UPDATE_RESULT_FMT, UPDATE, did_to_u16(DID),
          RESULT_STR);
    } // else an internal error has occurred //

    oncli_print_prompt();
} // one_net_master_update_result //


BOOL one_net_master_remove_device_result(const one_net_raw_did_t *DID,
  BOOL SUCCEEDED)
{
    const char * RESULT_STR;

    if(!DID)
    {
        return FALSE;
    } // if any of the parameters are invalid //

    if(SUCCEEDED)
    {
        RESULT_STR = ONCLI_SUCCEEDED_STR;
    } // if update was successful //
    else
    {
        RESULT_STR = ONCLI_FAILED_STR;
    } // else update failed //

    oncli_send_msg(ONCLI_UPDATE_RESULT_FMT,
      ONCLI_M_UPDATE_STR[ONE_NET_UPDATE_REMOVE_DEVICE], did_to_u16(DID),
      RESULT_STR);

    if(!SUCCEEDED)
    {
        oncli_send_msg(ONCLI_M_RM_DEV_ANYWAY_STR);
    } // if the REMOVE_DEV message failed //

    oncli_print_prompt();
    return TRUE;
} // one_net_master_remove_device_result //


oncli_status_t oncli_start_data_rate_test(
  const one_net_raw_did_t *SENDER,
  const one_net_raw_did_t *RECEIVER, UInt8 DATA_RATE)
{
    if(!RECEIVER)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameters are invalid //
    
    switch(one_net_master_start_data_rate_test(SENDER, RECEIVER, DATA_RATE))
    {
        case ONS_SUCCESS:
        {
            return ONCLI_SUCCESS;
            break;
        } // success case //
        
        /*case ONS_BAD_PARAM:
        {
            return ONCLI_INTERNAL_ERR;
            break;
        } // bad parameter case // */
        
        case ONS_INVALID_DATA:
        {
            return ONCLI_UNSUPPORTED;
            break;
        } // invali data case //

        case ONS_INCORRECT_ADDR:
        {
            return ONCLI_INVALID_DST;
            break;
        } // incorrect address case //

        default:
        {
            break;
        } // default case //
    } // switch(one_net_master_start_data_rate_test) //
    
    return ONCLI_INTERNAL_ERR;
} // oncli_start_data_rate_test //


#ifdef _ENABLE_LIST_COMMAND
oncli_status_t oncli_print_master_peer(BOOL prompt_flag)
{
    one_net_raw_did_t raw_did;
    UInt8 i, count;
    enum {MASTER_DID = 1};

    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    oncli_send_msg(ONCLI_LIST_PEER_TABLE_HEADING);
    count = 0;

    //
    // loop through each unit/user pin (i)
    //
    for (i = 0; i < NUM_MASTER_PEER; i++)
    {
        if(master_peer[i].peer_unit != ONE_NET_DEV_UNIT)
        {
            //
            // found a peer, print it
            //
            on_decode(raw_did, master_peer[i].peer_did, ONE_NET_RAW_DID_LEN);
            oncli_send_msg(ONCLI_LIST_PEER_FMT, MASTER_DID, master_peer[i].src_unit,
              did_to_u16(&raw_did), master_peer[i].peer_unit);
            count++;
        }
    }
    if (count == 0)
    {
        oncli_send_msg(ONCLI_LIST_NO_PEERS);
    }
    return ONCLI_SUCCESS;
} // oncli_print_master_peer //
#endif


oncli_status_t oncli_print_channel(BOOL prompt_flag)
{
    UInt8 channel;

    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
    
    channel = one_net_master_get_channel();

    // dje: Cast to eliminate compiler warning UInt8 >= 0
    if((SInt8)channel >= ONE_NET_MIN_US_CHANNEL && channel <= ONE_NET_MAX_US_CHANNEL)
    {
        // +1 since channels are stored 0 based, but output 1 based
        oncli_send_msg(ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_US_STR, channel
          - ONE_NET_MIN_US_CHANNEL + 1);
    } // if a US channel //
    else if(channel >= ONE_NET_MIN_EUR_CHANNEL
      && channel <= ONE_NET_MAX_EUR_CHANNEL)
    {
        // +1 since channels are stored 0 based, but output 1 based
        oncli_send_msg(ONCLI_GET_CHANNEL_RESPONSE_FMT, ONCLI_EUR_STR, channel
          - ONE_NET_MIN_EUR_CHANNEL + 1);
    } // else if a European channel //
    else
    {
        oncli_send_msg(ONCLI_CHANNEL_NOT_SELECTED_STR);
    } // else the channel is not selected //

    if (prompt_flag == TRUE)
    {
        oncli_print_prompt();
    }
    return ONCLI_SUCCESS;
} // oncli_print_channel //

oncli_status_t oncli_print_nid(BOOL prompt_flag)
{
    UInt8 * ptr_nid;
    UInt8 i, nibble;

    ptr_nid = (UInt8 *) get_raw_sid();

    oncli_send_msg("NID: 0x");
    for (i=0; i<ONE_NET_RAW_NID_LEN; i++)
    {
        // 
        // print the high order nibble
        //
        nibble = (ptr_nid[i] >> 4) & 0x0f;
        oncli_send_msg("%c", HEX_DIGIT[nibble]);

        //
        // if this is the not the last byte, print the low order nibble also
        //
        if (i < ONE_NET_RAW_NID_LEN-1)
        {
            nibble = ptr_nid[i] & 0x0f;
            oncli_send_msg("%c", HEX_DIGIT[nibble]);
        }
    }
    oncli_send_msg("\n");
    if (prompt_flag == TRUE)
    {
        oncli_print_prompt();
    }
    return ONCLI_SUCCESS;
} // oncli_print_nid //


oncli_status_t oncli_print_invite(BOOL prompt_flag)
{
    UInt8 * ptr_invite_key;

#if 0 // TODO: RWM: I don't think we need to limit this to the master
    if(!oncli_is_master())
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // if not a MASTER device //
#endif

    ptr_invite_key = get_invite_key();

    oncli_send_msg(ONCLI_DISPLAY_INVITE_STR, &ptr_invite_key[0], &ptr_invite_key[4]);

    if (prompt_flag == TRUE)
    {
        oncli_print_prompt();
    }
    return ONCLI_SUCCESS;
} // oncli_print_sid_and_invite //

//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_master_eval
//! @{

/*!
    \brief Initializes the device as a MASTER in auto mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param BASE_PARAM The base parameters.

    \return void
*/
#ifdef _AUTO_MODE
void init_auto_master(void)
{
    on_base_param_t * base_param;
    on_master_param_t * master_param;
    on_client_t * client;
    
    one_net_raw_did_t raw_did;

    // The parameters used to initialize an existing network as if they were
    // stored in a continguous block of memory
    UInt8 param[sizeof(on_base_param_t) + sizeof(on_master_param_t)
      + sizeof(on_client_t) * NUM_AUTO_CLIENTS];

    UInt8 client_count;
	UInt8 i;

    init_master_user_pin(0, 0);
    init_master_peer();
    
    // initialize timer for auto mode to send right away
    ont_set_timer(AUTO_MODE_TIMER, 0);

    // set the pointers
    base_param = (on_base_param_t *)param;
    master_param = (on_master_param_t *)(((UInt8 *)base_param)
      + sizeof(on_base_param_t));
    client = (on_client_t *)(((UInt8 *)master_param) + sizeof(on_master_param_t));
    
    // fill in the base parameters
    init_base_param(base_param);
    get_eval_encoded_nid((on_encoded_nid_t *)&(base_param->sid));
    get_eval_encoded_did(MASTER_NODE,
      (on_encoded_did_t *)&(base_param->sid[ON_ENCODED_NID_LEN]));
    
	
	// Derek_S 11/1/2010 - let update_client_count do below.
    /*// fill in the master_param data*/
    master_param->next_client_did = ONE_NET_INITIAL_CLIENT_DID
      + (NUM_AUTO_CLIENTS << RAW_DID_SHIFT);
    master_param->client_count = NUM_AUTO_CLIENTS;
    
    // fill in the CLIENT table data
    for(client_count = 0; client_count < NUM_AUTO_CLIENTS; client_count++)
    {
        one_net_int16_to_byte_stream(ONE_NET_INITIAL_CLIENT_DID
          + (client_count << RAW_DID_SHIFT), raw_did);
        on_encode(client[client_count].did, raw_did, ON_ENCODED_DID_LEN);
        client[client_count].expected_nonce = 0;
        client[client_count].last_nonce = 0;
        client[client_count].send_nonce = 0;
        client[client_count].data_rate
          = eval_data_rate(AUTO_CLIENT1_NODE + client_count);
        client[client_count].max_data_rate
          = eval_data_rate(AUTO_CLIENT1_NODE + client_count);
        client[client_count].flags = eval_client_flag(AUTO_CLIENT1_NODE
          + client_count);
        client[client_count].features = eval_client_features(AUTO_CLIENT1_NODE
          + client_count);
        client[client_count].use_current_key = TRUE;
        client[client_count].use_current_stream_key = TRUE;
        client[client_count].max_hops = 0;
    } // loop to fill in the CLIENT list //

    base_param->crc = one_net_compute_crc((UInt8 *)base_param
      + sizeof(base_param->crc), sizeof(param) - sizeof(base_param->crc),
      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);
    // initialize the ONE-NET MASTER
    one_net_master_init(param, sizeof(param));
} // init_auto_master //
#endif

/*!
    \brief Initializes the device as a MASTER in serial mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param EVAL_SID_IDX Index of the SID to use.

    \return void
*/
void initialize_master_pins_for_demo(void)
{
    oncli_set_user_pin_type(0, ONCLI_INPUT_PIN);
    oncli_set_user_pin_type(1, ONCLI_INPUT_PIN);
    oncli_set_user_pin_type(2, ONCLI_OUTPUT_PIN);
    oncli_set_user_pin_type(3, ONCLI_OUTPUT_PIN);
}


void init_serial_master(void)
{
    const UInt8 * MASTER_PARAM;
    UInt16 master_len;

    if(eval_load(DFI_ST_ONE_NET_MASTER_SETTINGS, &master_len, &MASTER_PARAM))
    {
        const UInt8 * MASTER_PEERS, * USER_PIN_TYPE;
        UInt16 master_peer_len, user_pin_type_len;

        if(eval_load(DFI_ST_APP_DATA_2, &master_peer_len, &MASTER_PEERS)
          && master_peer_len == sizeof(master_peer)
          && eval_load(DFI_ST_APP_DATA_1, &user_pin_type_len,
          &USER_PIN_TYPE) && user_pin_type_len == NUM_USER_PINS)
        {
            one_net_master_init(MASTER_PARAM, master_len);
            one_net_memmove(master_peer, MASTER_PEERS, sizeof(master_peer));
            init_master_user_pin(USER_PIN_TYPE, user_pin_type_len);
        } // if loading the rest of the parameters successful //
        else
        {
            oncli_send_msg(ONCLI_LOAD_FAIL_STR);
            oncli_reset_master();
            init_master_peer();
        } // else loading all the parameters failed //
    } // if prervious settings were stored //
    else
    {
        oncli_reset_master();
    } // else start a new network //
} // init_serial_master //


/*!
    \brief Initializes the base parameters for the evaluation network.
    
    \param[out] base_param The base parameters to initialize.  All base
      parameters except the SID are initialized.
    
    \return void
*/
static void init_base_param(on_base_param_t *base_param)
{
    if(!base_param)
    {
        return;
    } // if the parameter is invalid //

    base_param->version = EVAL_PARAM_VERSION;
    base_param->channel = eval_channel();
    base_param->data_rate = eval_data_rate(MASTER_NODE);
    get_eval_key(&(base_param->current_key));
    base_param->single_block_encrypt = eval_encryption(ON_SINGLE);
    get_eval_stream_key(&(base_param->stream_key));
    base_param->stream_encrypt = eval_encryption(ON_STREAM);
    base_param->fragment_delay_low = eval_fragment_delay(ONE_NET_LOW_PRIORITY);
    base_param->fragment_delay_high
      = eval_fragment_delay(ONE_NET_HIGH_PRIORITY);
} // init_base_param //


/*!
    \brief Initializes the parameters used with the user pins.
    
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
    \brief Automatically sends a message when in auto mode.
    
    This function also checks the time interval before sending the message.
    
    \param void
    
    \return void
*/
#ifdef _AUTO_MODE
static void send_auto_msg(void)
{
    if(mode_type() == AUTO_MODE)
    {
#ifdef _AT_LEAST_ONE_COMMAND_ENABLED
        if(oncli_user_input())
        {
            ont_set_timer(AUTO_MODE_TIMER, AUTO_MANUAL_DELAY);
            return;
        } // if there is user input //
#endif

        if(ont_expired(AUTO_MODE_TIMER))
        {
            one_net_raw_did_t dst;

            UInt8 pld[ONE_NET_RAW_SINGLE_DATA_LEN];
            UInt8 client_count;
            put_msg_hdr(ONA_COMMAND | ONA_SIMPLE_TEXT, pld);
            put_three_message_bytes_to_payload("abc", pld);
            
            for(client_count = 0; client_count < NUM_AUTO_CLIENTS;
              client_count++)
            {
                one_net_int16_to_byte_stream(ONE_NET_INITIAL_CLIENT_DID
                  + (client_count << RAW_DID_SHIFT), dst);

                one_net_master_send_single(pld, sizeof(pld), sizeof(pld),
                  ONE_NET_LOW_PRIORITY, &dst, 0x00);
            } // loop to send the transaction //

            ont_set_timer(AUTO_MODE_TIMER, AUTO_INTERVAL);
        } // if the timer has expired //
    } // if auto mode //
} // send_auto_msg //
#endif


/*!
    \brief Checks to see if the state of any of the user pins changed
    
    \param void
    
    \return void

    dje: August 29, 2008
        Changed so that user_pin[0] (user pin 1) corresponds to src unit 1
                        user_pin[1] (user pin 2) corresponds to src unit 2
                        etc.
    rwm: December 16, 2008
        Changed so that user_pin[0] (user pin 0) corresponds to src unit 0
                        user_pin[1] (user pin 1) corresponds to src unit 1
                        etc.
*/
static void master_check_user_pins(void)
{
    if(user_pin[0].pin_type == ONCLI_INPUT_PIN
      && USER_PIN0 != user_pin[0].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 0;
        user_pin_peer_idx = 0;
        user_pin[0].old_state = USER_PIN0;
    } // if the user0 pin has been toggled //
    else if(user_pin[1].pin_type == ONCLI_INPUT_PIN
      && USER_PIN1 != user_pin[1].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 1;
        user_pin_peer_idx = 0;
        user_pin[1].old_state = USER_PIN1;
    } // if the user1 pin has been toggled //
    else if(user_pin[2].pin_type == ONCLI_INPUT_PIN
      && USER_PIN2 != user_pin[2].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 2;
        user_pin_peer_idx = 0;
        user_pin[2].old_state = USER_PIN2;
    } // if the user2 pin has been toggled //
    else if(user_pin[3].pin_type == ONCLI_INPUT_PIN
      && USER_PIN3 != user_pin[3].old_state)
    {
        master_user_pin_state = M_SEND_USER_PIN_INPUT;
        user_pin_src_unit = 3;
        user_pin_peer_idx = 0;
        user_pin[3].old_state = USER_PIN3;
    } // if the user3 pin has been toggled //
} // master_check_user_pins //


/*!
    \brief Sends the user pin state to the assigned peers
    
    \param void
    
    \return void
dje: changes so that user_pin_src_unit goes from 1 to 4, not 0 to 3
rwm: changes so that user_pin_src_unit goes from 0 to 3, not 1 to 4
That is: It src_unit 1 indexes into master_peer[0], etc.
*/
static void master_send_user_pin_input(void)
{
    one_net_status_t status;
    one_net_raw_did_t raw_did;

    for(; user_pin_peer_idx < NUM_MASTER_PEER; user_pin_peer_idx++)
    {
        if(master_peer[user_pin_peer_idx].src_unit
          == ONE_NET_DEV_UNIT)
        {
            break;
        } // if no more peers to send to //
        else if(master_peer[user_pin_peer_idx].src_unit
          != user_pin_src_unit)
        {
            continue;
        } // peer does not apply here //

        // this is a peer for the relevant source unit
		else
		{
            on_decode(raw_did, master_peer[user_pin_peer_idx].peer_did, ONE_NET_RAW_DID_LEN);
			
            if((status
                = send_switch_command(user_pin[user_pin_src_unit].old_state,
                    user_pin_src_unit, master_peer[user_pin_peer_idx].peer_unit,
                    &raw_did)) != ONS_SUCCESS)
            {
                if(status == ONS_RSRC_FULL)
                {
                    return;
                } // if the resource is full //
            
                // Don't try this peer again for any other return type since they
                // didn't work the first time, they wouldn't work any other time
                // for the same reason, so we would be stuck in an endless loop
                // if we tried them again (endless loop in that we would never
                // check the user pins again).
            } // if queueing the transaction was not successful //
		}
    } // loop to try to queue the switch messages //

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
    send_auto_msg();
#endif
    master_user_pin();
    one_net_master();
} // master_eval //

//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_master_eval
