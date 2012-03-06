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
#ifdef _NON_VOLATILE_MEMORY
#include "nv_hal.h"
#endif
#include "hal.h"
#include "one_net_eval.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_master_port_specific.h"
#include "dfi.h"
#include "oncli_port.h"
#include "oncli.h"
#include "oncli_str.h"
#include "tal.h"
#ifdef _PEER
#include "one_net_peer.h"
#endif
#include "one_net_prand.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_master_eval_const
//! \ingroup ONE-NET_master_eval
//! @{



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



//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================




//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



#ifdef _AUTO_MODE
static void send_auto_msg(void);
#endif
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
    return (eval_save() ? ONS_SUCCESS : ONS_FAIL);
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


/*!
    \brief Returns a pointer to the SID to use.
    
    \return A pointer to the SID to use.
*/
on_raw_sid_t * one_net_master_get_raw_sid(void)
{
    #ifdef _NON_VOLATILE_MEMORY
    UInt8 * ptr_raw_sid = dfi_find_last_segment_of_type(
      DFI_ST_DEVICE_MFG_DATA);
      
    if (ptr_raw_sid == (UInt8 *) 0)
    {
    #endif
        //
        // no manufacturing data was found so use default raw SID.
        //
        return((on_raw_sid_t*) &DEFAULT_RAW_SID[0]);
    #ifdef _NON_VOLATILE_MEMORY
    }
    else
    {
        //
        // there is SID in data flash, return a pointer to it.
        //
        return((on_raw_sid_t*)(ptr_raw_sid + sizeof(dfi_segment_hdr_t)));
    }
    #endif
} // on_master_get_raw_sid //
    


BOOL one_net_master_device_is_awake(BOOL responding,
  const on_raw_did_t *DID)
{
    #ifdef _UART
    if(verbose_level > 1)
    {
        oncli_send_msg("Device %03X has checked in.\n", did_to_u16(DID));
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    }
    #endif
    return FALSE;
} // one_net_master_device_is_awake //


void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t* KEY, const on_raw_did_t *CLIENT_DID)
{
    #ifdef _UART
    if(verbose_level)
    {
        if(!KEY)
        {
            oncli_send_msg(ONCLI_INTERNAL_ERR_FMT,
              &one_net_master_invite_result);
            ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
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
    
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    }
    #endif
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
        on_encode(client_list[i].device.did,
          RAW_AUTO_CLIENT_DID[i], ON_ENCODED_DID_LEN);
        client_list[i].device.features = THIS_DEVICE_FEATURES;
        client_list[i].device.data_rate = ONE_NET_DATA_RATE_38_4;
        #ifdef _ONE_NET_MULTI_HOP
        client_list[i].device.hops = 0;
        client_list[i].device.max_hops =
          features_max_hops(THIS_DEVICE_FEATURES);
        #endif
        client_list[i].flags = ON_JOINED;
        client_list[i].use_current_key = TRUE;
        client_list[i].keep_alive_interval = 0; // don't check in for auto mode
    }

    #ifdef _PEER
    one_net_reset_peers();
    #endif
    one_net_master_init(NULL, 0);
    reset_msg_ids();
    
    // initialize timer for auto mode to send right away
    ont_set_timer(AUTO_MODE_TIMER, 0);
} // init_auto_master //
#endif


/*!
    \brief Initializes the device as a MASTER in serial mode
    
    \param[in] load_nv_memory If true, an attempt should be made to load
               the non-volatile memory into RAM.  If false, not attempt will
               be made.
    \param[in] channel Relevant only if non-volatile memory is not loaded.
                       If channel is non-negative, the master will use this
                       channel.  If channel is negative, it will be
                       disregarded.

    \return void
*/
#ifdef _NON_VOLATILE_MEMORY
void init_serial_master(BOOL load_nv_memory, SInt8 channel)
#else
void init_serial_master(SInt8 channel)
#endif
{
#ifdef _NON_VOLATILE_MEMORY
    BOOL memory_loaded;
    const UInt8* nv_memory;
    const UInt8* user_pin_memory;
    const on_base_param_t* base_param;
    UInt16 nv_memory_len, user_pin_memory_len;
    #ifdef _PEER
    const UInt8* peer_memory;
    UInt16 peer_memory_len;
    #endif

    if(load_nv_memory)
    {
        memory_loaded = eval_load(DFI_ST_APP_DATA_1, &user_pin_memory_len,
          &user_pin_memory);
        if(user_pin_memory_len != sizeof(user_pin))
        {
            memory_loaded = FALSE;
        }
    }
    else
    {
        memory_loaded = FALSE;
    }

    if(memory_loaded)
    {
        // first initialize all clients to broadcast
        UInt8 i;
        for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
        {
            client_list[i].device.did[0] = 0xB4;
            client_list[i].device.did[1] = 0xB4;
        }
        memory_loaded = eval_load(DFI_ST_ONE_NET_MASTER_SETTINGS,
          &nv_memory_len, &nv_memory);
    }
    
    #ifdef _PEER
    if(memory_loaded)
    {
        memory_loaded = eval_load(DFI_ST_ONE_NET_PEER_SETTINGS,
          &peer_memory_len, &peer_memory);
    }
    #endif
    
    if(memory_loaded)
    {
        one_net_status_t status;
        status = one_net_master_init(nv_memory, nv_memory_len);
        
        #ifdef _PEER
        if(status != ONS_MORE)
        {
            memory_loaded = FALSE;
        }
        else
        {
            status = one_net_master_init(peer_memory, peer_memory_len);
        }
        #endif
        
        if(status != ONS_SUCCESS)
        {
            memory_loaded = FALSE;
        }
        else
        {
            // so far, so good.  Copy the pin info and we should be done.
            one_net_memmove(user_pin, user_pin_memory, sizeof(user_pin));
        }
    }
    
    if(memory_loaded)
    {
        #ifdef _UART
        oncli_send_msg("Parameters have been loaded from flash.\n");
        #endif
    }
    else
    {
        #ifdef _UART
        oncli_send_msg("Parameters have not been loaded from flash.\n");
        #endif
#endif
        // start a brand new network
        one_net_master_reset_master(one_net_master_get_raw_sid(), channel);
#ifdef _NON_VOLATILE_MEMORY
    }
#endif
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
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


one_net_status_t one_net_master_reset_master(on_raw_sid_t* raw_sid,
  SInt8 channel)
{
    node_loop_func = &master_eval;
    initialize_default_pin_directions(TRUE);
    
    // TODO -- what if this fails?
    one_net_master_create_network(raw_sid, &EVAL_KEY);
    
    if(channel >= 0)
    {
        if(one_net_set_channel(channel) == ONS_SUCCESS)
        {
            on_base_param->channel = channel;
            on_state = ON_LISTEN_FOR_DATA;
        }
    }
    return ONS_SUCCESS;
}


void one_net_master_update_result(one_net_mac_update_t update,
  const on_raw_did_t* did, const on_ack_nack_t* ack_nack)
{
    #ifdef _UART
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
        if(update == ONE_NET_UPDATE_NETWORK_KEY || update ==
          ONE_NET_UPDATE_REMOVE_DEVICE || update == ONE_NET_UPDATE_ADD_DEVICE)
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
        case ONE_NET_UPDATE_ADD_DEVICE:
            result_type = ONCLI_M_UPDATE_RESULT_ADD_DEV_STR;
            break;
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
    
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    #endif
} // one_net_master_update_result //


BOOL one_net_master_client_missed_check_in(on_client_t* client)
{
    #ifdef _UART
    on_raw_did_t raw_did;
    on_decode(raw_did, client->device.did, ON_ENCODED_DID_LEN);
    oncli_send_msg(ONCLI_CLIENT_MISS_CHECK_IN_FMT, did_to_u16(&raw_did));
    
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    #endif
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



#ifdef _AUTO_MODE
/*!
    \brief Automatically sends a message when in auto mode.
    
    This function also checks the time interval before sending the message.
    
    \param void
    
    \return void
*/
static void send_auto_msg(void)
{
    #ifdef _UART
    if(oncli_user_input())
    {
        ont_set_timer(AUTO_MODE_TIMER, MS_TO_TICK(AUTO_MANUAL_DELAY));
        return;
    } // if there is user input //
    #endif

    if(ont_expired(AUTO_MODE_TIMER))
    {
        UInt8 i;
        
        // each client gets a different text message
        char* auto_messages[NUM_AUTO_CLIENTS] = {"11", "22", "33"};

        for(i = 0; i < NUM_AUTO_CLIENTS; i++)
        {
            send_simple_text_command(auto_messages[i], ONE_NET_DEV_UNIT,
              ONE_NET_DEV_UNIT, &ENC_AUTO_CLIENT_DID[i]);
        }
        ont_set_timer(AUTO_MODE_TIMER, MS_TO_TICK(AUTO_INTERVAL));
    } // if the timer has expired //
} // send_auto_msg //
#endif


/*!
    \brief Checks the user pins and sends messages if the state has changed.
    
    \param void
    
    \return void
*/
static void master_user_pin(void)
{
    switch(user_pin_state)
    {
        case SEND_USER_PIN_INPUT:
        {
            send_user_pin_input();
            break;
        } // SEND_USER_PIN_INPUT state //

        default:
        {
            check_user_pins();
            break;
        } // default case //
    } // switch(master_user_pin_state) //
} // master_user_pin //



//! @} ONE-NET_master_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_master_eval


#endif // ifdef _ONE_NET_MASTER //
