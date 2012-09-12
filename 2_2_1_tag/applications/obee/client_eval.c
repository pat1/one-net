//! \defgroup ONE-NET_client_eval ONE-NET CLIENT Evaluation
//! \ingroup ONE-NET_eval
//! @{

/*!
    \file client_eval.c
    \brief The CLIENT part of the ONE-NET evaluation project.
*/

#include "config_options.h"


#ifdef _ONE_NET_CLIENT


#include "one_net_client.h"
#include "one_net_port_const.h"
#include "one_net_constants.h"
#include "one_net_eval.h"
#ifdef _NON_VOLATILE_MEMORY
#include "nv_hal.h"
#endif
#include "tick.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_client_port_specific.h"
#include "oncli_str.h"
#include "oncli.h"
#ifdef _NON_VOLATILE_MEMORY
#include "dfi.h"
#endif
#include "one_net_prand.h"

#ifdef _PEER
#include "one_net_peer.h"
#endif

#include "one_net_timer.h"
#include "hal.h"

#include "eeprom_driver.h"

//                                  CONSTANTS
//! \defgroup ONE-NET_client_eval_const
//! \ingroup ONE-NET_client_eval
//! @{



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


//! @} ONE-NET_client_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================



//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_client_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



//! @} ONE-NET_client_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_client_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{



static void client_user_pin(void);



//! @} ONE-NET_client_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_client_eval_pub_func
//! \ingroup ONE-NET_client_eval
//! @{


void one_net_client_client_removed(const on_raw_did_t * const raw_did,
    BOOL this_device_removed)
{
    #ifdef _UART
    if(verbose_level)
    {
        if(this_device_removed)
        {
            oncli_send_msg("This device has been removed from the network.\n");
        }
        else
        {
            oncli_send_msg("Device %03d has been removed from the network.\n",
              did_to_u16(raw_did));
        }
    }
    #endif
}


void one_net_client_client_added(const on_raw_did_t * const raw_did)
{
    #ifdef _UART
     if(verbose_level)
    {
        oncli_send_msg("Device %03d has been added to the network.\n",
          did_to_u16(raw_did));
    }
    #endif
}


void one_net_client_invite_result(const on_raw_did_t * const RAW_DID,
  one_net_status_t status)
{
    #ifdef _UART
    if(!verbose_level)
    {
        return;
    }
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);

    switch(status)
    {
        case ONS_CANCELED: oncli_send_msg("Invite process cancelled.\n");
          return;
        case ONS_TIME_OUT: oncli_send_msg("Invite process timed out.\n");
          return;
        case ONS_SUCCESS:
          // print the joined message
          oncli_send_msg(ONCLI_JOINED_FMT, did_to_u16(RAW_DID));
        default:
         break;
    }
    #endif
}



#ifdef _NON_VOLATILE_MEMORY
one_net_status_t one_net_client_save_settings(void)
{
    return (eval_save() ? ONS_SUCCESS : ONS_FAIL);
} // one_net_client_save_settings //


one_net_status_t one_net_client_load_settings(void)
{
    return ONS_FAIL;
} // one_net_client_load_settings //


one_net_status_t one_net_client_erase_settings(void)
{
    return (eval_erase_data_flash() ? ONS_SUCCESS : ONS_FAIL);
} // one_net_client_erase_settings //
#endif // ifdef _NON_VOLATILE_MEMORY //


void one_net_client_client_remove_device(void)
{
    #ifdef _UART
    if(verbose_level)
    {
        oncli_send_msg("Removed from network by master.  No longer joined.\n");
        oncli_send_msg("Now resetting the device and looking for an invite.\n");
     }
    ont_set_timer(PROMPT_TIMER, SERIAL_PROMPT_PERIOD);
    #endif
} // one_net_client_client_remove_device //


#ifdef _AUTO_MODE
/*!
    \brief Initializes the device as a CLIENT in auto mode

    \param index the client index of the device from which to assign the DID.

    \return void
*/
/*
void init_auto_client(UInt8 index)
{
    if(index >= NUM_AUTO_CLIENTS)
    {
        return; // out of range
    }

    on_encode(&on_base_param->sid[ON_ENCODED_NID_LEN],
      RAW_AUTO_CLIENT_DID[index], ON_ENCODED_DID_LEN);
    master->keep_alive_interval = 0; // don't check in for auto mode
    master->flags = ON_JOINED;
    master->device.data_rate = ONE_NET_DATA_RATE_38_4;
    master->device.features = THIS_DEVICE_FEATURES;
    one_net_memmove(master->device.did, MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);

    #ifndef _PEER
    one_net_client_init(NULL, 0);
    #else
    one_net_reset_peers();
    one_net_client_init(NULL, 0, NULL, 0);
    reset_msg_ids();
    #endif
} // init_auto_client //
*/
#endif


/*!
    \brief Initializes the client in serial mode

    \param void

    \return void
*/
void init_serial_client(void)
{
#ifdef _NON_VOLATILE_MEMORY
   UInt16 address = 0;
   UInt8 read_byte = 0;

   // Reading the eeprom_manufacturing_saved,eeprom_master_saved and eeprom_client_saved values
   // from EEPROM
   // this locations in the eeprom is set to 0xFF when it was never written to,
   // or, when the eeprom_master_saved or eeprom_client_saved values are erased.

   address = DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET;

   // get eeprom_manufacturing_saved value from eeprom
   read_byte = eeprom_read_byte((UInt16)address);

   if(read_byte == 0xFF)
   {
     eeprom_manufacturing_saved = FALSE;
   }
   else if(read_byte == 0x01)
   {
     eeprom_manufacturing_saved = TRUE;
   }


   // set eeprom_user_pins_saved address in eeprom
   address = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET;

   // get user pins settings from eeprom
   eeprom_read_block ((UInt8 *)user_pins_settings, (UInt16)address, DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE);



   // set eeprom_client_saved address in eeprom
   address = DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET;

   // get eeprom_client_saved value from eeprom
   read_byte = eeprom_read_byte((UInt16)address);

   if(read_byte == 0xFF)
   {
     eeprom_client_saved = FALSE;
   }
   else if(read_byte == 0x01)
   {
     eeprom_client_saved = TRUE;
   }

   if(eeprom_client_saved)
   {
      // set client settings address in eeprom
      UInt16 client_address = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET;

      // get client seetings from eeprom
      eeprom_read_block ((UInt8 *)client_settings, (UInt16)client_address, DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE);

      #ifdef _PEER
      // set peer settings address in eeprom
      UInt16 peer_address = DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET;
      // get peer seetings from eeprom
      eeprom_read_block ((UInt8 *)peer_settings, (UInt16)peer_address, DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE);
      #endif

      one_net_status_t status;

      #ifndef _PEER
      status = one_net_client_init(client_settings, DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE);
      #else
      status = one_net_client_init(client_settings, DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE, peer_settings,
       DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE);
      #endif

      if(status != ONS_SUCCESS)
      {
          #ifdef _UART
          oncli_send_msg("Parameters have not been loaded from eeprom.\n");
          #endif
          #ifndef _ENHANCED_INVITE
          one_net_client_reset_client((const one_net_xtea_key_t*)one_net_client_get_invite_key());
          #else
          one_net_client_reset_client((const one_net_xtea_key_t*)one_net_client_get_invite_key(), 0,
            ONE_NET_MAX_CHANNEL, 0);
          #endif
        }
      else
      {
          // so far, so good.  Copy the pin info and we should be done.
          one_net_memmove(user_pin, user_pins_settings, sizeof(user_pin));

          #ifdef _UART
          oncli_send_msg("Parameters have been loaded from eeprom.\n");
          #endif
      }
   }
   else  // if(eeprom_client_saved)
#endif // _NON_VOLATILE_MEMORY is defined
   {
        #ifdef _UART
        oncli_send_msg("Parameters have not been loaded from eeprom.\n");
        #endif
        #ifndef _ENHANCED_INVITE
        one_net_client_reset_client(one_net_client_get_invite_key());
        #else
        one_net_client_reset_client((const one_net_xtea_key_t*)one_net_client_get_invite_key(), 0,
          ONE_NET_MAX_CHANNEL, 0);
        #endif
   }

} // void init_serial_client(void)


#ifdef _ENHANCED_INVITE
one_net_status_t one_net_client_reset_client(const one_net_xtea_key_t* invite_key,
UInt8 low_channel, UInt8 high_channel, tick_t timeout_time)
#else
one_net_status_t one_net_client_reset_client(const one_net_xtea_key_t* invite_key)
#endif
{
	initialize_default_pin_directions(FALSE);
 	node_loop_func = &client_eval;

	#ifdef _ENHANCED_INVITE
	return one_net_client_look_for_invite(invite_key, low_channel, high_channel,
	timeout_time);
	#else
	return one_net_client_look_for_invite(invite_key);
    #endif
}


/*!
    \brief The CLIENT evaluation application

    This is the main function for the CLIENT evaluation.  The CLIENT is
    initialized and ran from this function.

    \param void

    \return void
*/
void client_eval(void)
{
    client_user_pin();
    one_net_client();
} // client_eval //


/*!
    \brief Returns a pointer to the invite key to use in for joining a network.

    \return A pointer to the invite key to use.
*/

one_net_xtea_key_t* one_net_client_get_invite_key(void)
{
    #ifdef _NON_VOLATILE_MEMORY
    if(eeprom_manufacturing_saved)
    {
        //
        // there is an invite key in data eeprom, return a pointer to it.
        //
        UInt16  segment_size = DFI_EEPROM_DEVICE_MFG_DATA_SIZE;
        UInt16 address = DFI_EEPROM_DEVICE_MFG_DATA_OFFSET;

        // get client settings from eeprom
        eeprom_read_block ((UInt8 *)manufacturing_settings, (UInt16)address, segment_size);
        return (one_net_xtea_key_t*)(&manufacturing_settings[ON_RAW_SID_LEN]);
    }
    else
    {
    #endif
        //
        // manufacturing data was not saved, use a default invite key
        //
        return (one_net_xtea_key_t*) &DEFAULT_INVITE_KEY[0];
    #ifdef _NON_VOLATILE_MEMORY
    }
    #endif
}



#ifdef _BLOCK_MESSAGES_ENABLED
on_nack_rsn_t one_net_client_get_default_block_transfer_values(
  const on_encoded_did_t* dst,
  UInt32 transfer_size, UInt8* priority, UInt8* chunk_size, UInt16* frag_delay,
  UInt16* chunk_delay, UInt8* data_rate, UInt8* channel, UInt16* timeout,
  on_ack_nack_t* ack_nack)
{
    return ON_NACK_RSN_NO_ERROR;
}


#ifdef _ONE_NET_MH_CLIENT_REPEATER
/*!
    \brief Application-level code called byu ONE-NET when this device is
           requested to function as a repeater for a block / stream message

    This function is called when another device is attempting to set up a
    block / stream message and has requested this device to reserve itself
    as a repeater for that purpose. This function will be passed the parameters
    of the proposed block / stream transfer.  Possible parameters of interest
    will be the estimated time of the transfer and the devices involved.

    Generally this function should reject the request if it feels it cannot
    comply for any reason.  Reasons could include not being reasonably certain
    that it will be able to function as a repeater for at least the time
    requested for whatever reason (low power, busy with its own messages,
    not expected to be powered up for the entire message, a high percentage
    of dropped message, it is reserved as a repeater for someone else, etc.)

    The ack_nack parameter is pre-loaded to assume acceptance.  If the repeater
    rejects, it should change the ack_nack variable to indicate rejection along
    with a reason, if any, is to be sent.

    \param[in] bs_msg The paramters of the block/ stream message this device is
               supposed to serve as a repeater for.
    \param[out] ack_nack This is pre-loaded for acceptance.  If accepting, no
                changes are needed.  If rejecting, the ack_nack variable should
                be changed in this function.

    \return void
*/
void one_net_client_repeater_requested(block_stream_msg_t* bs_msg,
  on_ack_nack_t* ack_nack)
{
    // just some debugging for now.
    oncli_send_msg("src=%02X%02X dst=%02X%02X est=%ld to=%d\n",
      bs_msg->src->did[0], bs_msg->src->did[1], bs_msg->dst->did[0],
      bs_msg->dst->did[1], bs_msg->time, bs_msg->timeout);
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
on_nack_rsn_t one_net_client_get_default_stream_transfer_values(
  const on_encoded_did_t* dst, UInt32 time_ms, UInt8* priority,
  UInt16* frag_delay, UInt8* data_rate, UInt8* channel, UInt16* timeout,
  on_ack_nack_t* ack_nack)
{
    return ON_NACK_RSN_NO_ERROR;
}
#endif
#endif


//! @} ONE-NET_client_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_client_eval_pri_func
//! \ingroup ONE-NET_client_eval
//! @{



/*!
    \brief Checks the user pins and sends messages if the state has changed.

    \param void

    \return void
*/
//#if 0
static void client_user_pin(void)
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
    } // switch(user_pin_state) //
} // client_user_pin //
//#endif



//! @} ONE-NET_client_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_client_eval



#endif // #ifdef _ONE_NET_CLIENT //
