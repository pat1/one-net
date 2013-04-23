//! \defgroup ONE-NET_master_eval ONE-NET MASTER Evaluation
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file master_eval.c
    \brief The MASTER part of the ONE-NET evaluation project.
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/

#include "config_options.h"

#ifdef ONE_NET_MASTER


#include "one_net_status_codes.h"
#include "one_net_constants.h"
#include "one_net.h"
#include "one_net_master.h"
#include "one_net_application.h"
#include "one_net_timer.h"
#include "io_port_mapping.h"
#include "tick.h"
#ifdef NON_VOLATILE_MEMORY
#include "eeprom_driver.h"
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
#ifdef PEER
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



#ifdef AUTO_MODE
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
    
    
#ifdef NON_VOLATILE_MEMORY
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
    return (eval_erase_data_eeprom() ? ONS_SUCCESS : ONS_FAIL);
} // one_net_master_erase_settings //
#endif // ifdef NON_VOLATILE_MEMORY //


/*!
    \brief Returns a pointer to the SID to use.
    
    \return A pointer to the SID to use.
*/
on_raw_sid_t * one_net_master_get_raw_sid(void)
{
    #ifdef NON_VOLATILE_MEMORY
    if(eeprom_manufacturing_saved)
    {
        //
        // there is SID in data eeprom, return a pointer to it.
        //
        UInt16  segment_size = DFI_EEPROM_DEVICE_MFG_DATA_SIZE;

        UInt16 address = DFI_EEPROM_DEVICE_MFG_DATA_OFFSET;

        // get manufacturing settings from eeprom
        eeprom_read_block ((UInt8 *)manufacturing_settings, (UInt16)address, segment_size);
        return (on_raw_sid_t*)manufacturing_settings;
    }
    else
    {
        //
        // there is not an SID in data eeprom, use a default SID
        //
        return((on_raw_sid_t*) &DEFAULT_RAW_SID[0]);
    }
    
    #else
        // Non-volatile memory is not used, use a default SID
        return((on_raw_sid_t*) &DEFAULT_RAW_SID[0]);
    #endif
} // on_master_get_raw_sid //
    


BOOL one_net_master_device_is_awake(BOOL responding,
  const on_raw_did_t *DID)
{
    #ifdef UART
    if(verbose_level > 1)
    {
        oncli_send_msg("Device %03X has checked in.\n", did_to_u16(DID));
        ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    }
    #endif
    
    #ifdef COMPILE_WO_WARNINGS
    if(responding)
    {
        return FALSE;
    }
    #endif
        
    return FALSE;
} // one_net_master_device_is_awake //


void one_net_master_invite_result(one_net_status_t STATUS,
  one_net_xtea_key_t* KEY, const on_raw_did_t *CLIENT_DID)
{
    #ifdef UART
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
    #ifdef COMPILE_WO_WARNINGS
    if(!DID && !SUCCEEDED)
    {
        return TRUE;
    }
    #endif    
    
    return TRUE;
} // one_net_master_remove_device_result //


#ifdef AUTO_MODE
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
        #ifdef ONE_NET_MULTI_HOP
        client_list[i].device.hops = 0;
        client_list[i].device.max_hops =
          features_max_hops(THIS_DEVICE_FEATURES);
        #endif
        client_list[i].flags = ON_JOINED;
        client_list[i].use_current_key = TRUE;
        client_list[i].keep_alive_interval = 0; // don't check in for auto mode
    }

    #ifdef PEER
    one_net_reset_peers();
    #endif
    one_net_master_init(NULL, 0, MEMORY_GENERIC);
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
#ifdef NON_VOLATILE_MEMORY
void init_serial_master(BOOL load_nv_memory, SInt8 channel)
#else
void init_serial_master(SInt8 channel)
#endif
{
#ifdef NON_VOLATILE_MEMORY
    UInt8 i;
    UInt16 address = 0;
    UInt8 read_byte = 0;
    one_net_status_t status;

    eeprom_manufacturing_saved = load_nv_memory;
    eeprom_master_saved = load_nv_memory;
    
    if(load_nv_memory)
    {
        // Reading the eeprom_manufacturing_saved,eeprom_master_saved and eeprom_client_saved values
        // from EEPROM
        // these locations in the eeprom are set to 0xFF when it was never written to,
        // or, when the eeprom_master_saved or eeprom_client_saved values are erased.
        
        // get eeprom_manufacturing_saved value from eeprom
        address = DFI_EEPROM_ONE_NET_EEPROM_MANUFACTURING_SAVED_OFFSET;
        read_byte = eeprom_read_byte((UInt16)address);
        if(read_byte == 0xFF)
        {
            eeprom_manufacturing_saved = FALSE;
        }
        
        // set eeprom_user_pins_saved address in eeprom
        address = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET;

        // get user pins settings from eeprom
        eeprom_read_block ((UInt8 *)user_pins_settings, (UInt16)address, DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE);

        // set eeprom_master_saved address in eeprom
        address = DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET;

        // get eeprom_master_saved value from eeprom
        read_byte = eeprom_read_byte((UInt16)address);
        if(read_byte == 0xFF)
        {
            eeprom_master_saved = FALSE;
        }
        
        if(eeprom_master_saved)
        {
            #ifdef PEER
            // set peer settings address in eeprom
            UInt16 peer_address = DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET;
            #endif

            for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
            {
                client_list[i].device.did[0] = 0xB4;
                client_list[i].device.did[1] = 0xB4;
            }
            
            // set master settings address in eeprom
            address = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET;
            // get master settings from eeprom
            eeprom_read_block ((UInt8 *)master_settings, (UInt16)address, DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE);
            #ifdef PEER
            // get peer settings from eeprom
            eeprom_read_block ((UInt8 *)peer_settings, (UInt16)peer_address, DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE);
			#endif
              
			// 2-14-13 //////////////////////////////////////////////////////////////////////////////////////////
			// status = one_net_master_init(master_settings, MAX_MASTER_NV_PARAM_SIZE_BYTES, MEMORY_NON_PEER);
			#ifdef PEER
            status = one_net_master_init(master_settings, MAX_MASTER_NV_PARAM_SIZE_BYTES, MEMORY_NON_PEER);
			#else
			status = one_net_master_init(master_settings, MAX_MASTER_NV_PARAM_SIZE_BYTES);
			#endif
            /////////////////////////////////////////////////////////////////////////////////////////////////////
            #ifdef PEER
            if(status == ONS_MORE)
            {
                // Note that if the function call above had returned ONS_SUCCESS, that would be a BAD thing.  Why?
                // We have not passed one_net_master_init the peer memory yet, so it should not be done yet.
                // ONS_SUCCESS at this stage, since PEER is enabled, would be a problem because it would mean that
                // ONE-NET thinks it has initialized the peer memory, but it actually has not.  Any return value
                // other than ONS_MORE at this point signifies an error somewhere since PEER is enabled.
                
                // So far, so good.  Everything should be loaded except for PEER memory.  Load that now.
                // If everything works, the call below will return ONS_SUCCESS, signifying that the memory
                // loaded from EEPROM has been successfully loaded into RAM where it needs to go.
                status = one_net_master_init(peer_settings, PEER_STORAGE_SIZE_BYTES, MEMORY_PEER);
            }
            else
            {
                status = ONS_FAIL; // something went wrong somewhere.  Non-volatile memory appears to be corrupted.
                                   // Therefore we will reset the system as an empty network below.
            }
            #endif
            
            if(status == ONS_SUCCESS)
            {
                // so far, so good.  Set the pins and we should be done.
                one_net_memmove(user_pin, user_pins_settings, sizeof(user_pin));
                
                // TODO -- Is this function needed for the Atmel port?  Is it written yet?
                initialize_pin_states_and_directions_from_user_pin();
            }
            else
            {
                eeprom_master_saved = FALSE;
            }
        }
    }
    
    if(eeprom_master_saved)
    {
        #ifdef UART
        oncli_send_msg("Parameters have been loaded from eeprom.\n");
        #endif
    }
    else
    {
        #ifdef UART
        oncli_send_msg("Parameters have not been loaded from eeprom.\n");
        #endif
        // start a brand new network
        one_net_master_reset_master((const on_raw_sid_t*) one_net_master_get_raw_sid(), channel);
    }
#else
    // non-volatile memory is not being used. Start a new network.
    one_net_master_reset_master((const on_raw_sid_t*) one_net_master_get_raw_sid(), channel);
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
    #ifdef AUTO_MODE
    if(in_auto_mode)
    {
        send_auto_msg();
    }
    #endif
    master_user_pin();
    one_net_master();
} // master_eval //


one_net_status_t one_net_master_reset_master(const on_raw_sid_t* raw_sid,
  SInt8 channel)
{
	// 2-12-13 //////////////////////
	// set device to be a client
	device_is_master = TRUE;
	in_sniffer_mode = FALSE;
	/////////////////////////////////
	
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
    #ifdef UART
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
        #ifdef PEER
        case ONE_NET_UPDATE_ASSIGN_PEER:
            result_type = ONCLI_M_UPDATE_RESULT_ASSIGN_PEER_STR;
            break;
        case ONE_NET_UPDATE_UNASSIGN_PEER:
            result_type = ONCLI_M_UPDATE_RESULT_UNASSIGN_PEER_STR;
            break;
        #endif
        #ifdef BLOCK_MESSAGES_ENABLED
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
    #ifdef UART
    on_raw_did_t raw_did;
    on_decode(raw_did, client->device.did, ON_ENCODED_DID_LEN);
    oncli_send_msg(ONCLI_CLIENT_MISS_CHECK_IN_FMT, did_to_u16((const on_raw_did_t*)&raw_did));
    
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    #endif
    return FALSE;
}


#ifdef BLOCK_MESSAGES_ENABLED
#ifdef ONE_NET_MULTI_HOP
void one_net_master_repeater_requested(on_client_t* src_client,
  on_client_t* dst_client, on_client_t* repeater_client, UInt8 channel,
  UInt8 data_rate, UInt8 priority, UInt32 estimated_time,
  on_ack_nack_t* ack_nack)
{
}
#endif


on_nack_rsn_t one_net_master_get_default_block_transfer_values(
  const on_client_t* src, const on_client_t* dst, UInt32 transfer_size,
  UInt8* priority, UInt8* chunk_size, UInt16* frag_delay, UInt16* chunk_delay,
  UInt8* data_rate, UInt8* channel, UInt16* timeout, on_ack_nack_t* ack_nack)
{
    return ON_NACK_RSN_NO_ERROR;
}
#endif


#ifdef STREAM_MESSAGES_ENABLED
on_nack_rsn_t one_net_master_get_default_stream_transfer_values(
  const on_client_t* src, const on_client_t* dst, UInt32 time_ms,
  UInt8* priority, UInt16* frag_delay, UInt8* data_rate, UInt8* channel,
  UInt16* timeout, on_ack_nack_t* ack_nack)
{
    return ON_NACK_RSN_NO_ERROR;
}
#endif


//! @} ONE-NET_master_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_master_eval_pri_func
//! \ingroup ONE-NET_master_eval
//! @{



#ifdef AUTO_MODE
/*!
    \brief Automatically sends a message when in auto mode.
    
    This function also checks the time interval before sending the message.
    
    \param void
    
    \return void
*/
static void send_auto_msg(void)
{
    #ifdef UART
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


#endif // ifdef ONE_NET_MASTER //
