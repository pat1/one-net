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
#include "nv_hal.h"
#include "tick.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_client_port_specific.h"
#include "oncli_str.h"
#include "oncli.h"
#ifdef _NON_VOLATILE_MEMORY
#include "dfi.h"
#endif

#ifdef _PEER
#include "one_net_peer.h"
#endif



//=============================================================================
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
    
    
void one_net_client_invite_result(const on_raw_did_t * const RAW_DID,
  one_net_status_t status)
{
    switch(status)
    {
        case ONS_CANCELED: oncli_send_msg("Invite process cancelled.\n");
          return;
        case ONS_TIME_OUT: oncli_send_msg("Invite process timed out.\n");
          return;
        case ONS_SUCCESS:
          // print the joined message
          oncli_send_msg(ONCLI_JOINED_FMT, did_to_u16(RAW_DID));
    }
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
    oncli_send_msg("Removed from network by master.  No longer joined.\n");
    oncli_send_msg("Now resetting the device and looking for an invite.\n");
} // one_net_client_client_remove_device //


#ifdef _AUTO_MODE
/*!
    \brief Initializes the device as a CLIENT in auto mode
    
    \param index the client index of the device from which to assign the DID.

    \return void
*/
void init_auto_client(UInt8 index)
{
    if(index >= NUM_AUTO_CLIENTS)
    {
        return; // out of range
    }

    on_encode(&on_base_param->sid[ON_ENCODED_NID_LEN],
      RAW_AUTO_CLIENT_DID[index], ON_ENCODED_DID_LEN);
    master->keep_alive_interval = DEFAULT_EVAL_KEEP_ALIVE_MS;
    master->flags = ON_JOINED;
    master->device.data_rate = ONE_NET_DATA_RATE_38_4;
    master->device.features = THIS_DEVICE_FEATURES;
    one_net_memmove(master->device.did, MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);
    
    master->device.expected_nonce = ON_INVALID_NONCE;
    master->device.last_nonce = ON_INVALID_NONCE;
    master->device.send_nonce = ON_INVALID_NONCE;
    master->device.msg_id = ON_MAX_MSG_ID + 1; // invalid

    #ifndef _PEER
    one_net_client_init(NULL, 0);
    #else
    one_net_reset_peers();
    one_net_client_init(NULL, 0, NULL, 0);
    #endif
} // init_auto_client //
#endif


/*!
    \brief Initializes the client in serial mode
  
    \param void
    
    \return void
*/
void init_serial_client(void)
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


    memory_loaded = eval_load(DFI_ST_APP_DATA_1, &user_pin_memory_len,
      &user_pin_memory);
      
    if(user_pin_memory_len != sizeof(user_pin))
    {
        memory_loaded = FALSE;
    }

    if(memory_loaded)
    {
        memory_loaded = eval_load(DFI_ST_ONE_NET_CLIENT_SETTINGS,
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
        
        #ifndef _PEER
        status = one_net_client_init(nv_memory, nv_memory_len);
        #else
        status = one_net_client_init(nv_memory, nv_memory_len, peer_memory,
          peer_memory_len);
        #endif
        
        if(status != ONS_SUCCESS)
        {
            oncli_send_msg("Parameters have not been loaded from flash.\n");
            one_net_client_reset_client(one_net_client_get_invite_key());
        }
        else
        {
            // so far, so good.  Copy the pin info and we should be done.
            one_net_memmove(user_pin, user_pin_memory, sizeof(user_pin));
            oncli_send_msg("Parameters have been loaded from flash.\n");
        }
    }
    else
#endif
    {
        one_net_client_reset_client(one_net_client_get_invite_key());
    }
}


one_net_status_t one_net_client_reset_client(one_net_xtea_key_t* invite_key)
{
    initialize_default_pin_directions(FALSE);
    
#ifdef _STREAM_MESSAGES_ENABLED
  #ifdef _ENHANCED_INVITE
      return one_net_client_look_for_invite(invite_key, ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
        ONE_NET_STREAM_ENCRYPT_XTEA8, 0, ONE_NET_MAX_CHANNEL, 0);
  #else
      return one_net_client_look_for_invite(invite_key, ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
        ONE_NET_STREAM_ENCRYPT_XTEA8);
  #endif
#else
  #ifdef _ENHANCED_INVITE
      return one_net_client_look_for_invite(invite_key, ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
        0, ONE_NET_MAX_CHANNEL, 0);
  #else
      return one_net_client_look_for_invite(invite_key, ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32);
  #endif
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
    UInt8 * invite_key_ptr;

    invite_key_ptr = dfi_find_last_segment_of_type(DFI_ST_DEVICE_MFG_DATA);
    if (invite_key_ptr == (UInt8 *) 0)
    {
    #endif
        //
        // no manufacturing data was found use a default invite key
        //
        return (one_net_xtea_key_t*) &DEFAULT_INVITE_KEY[0];
    #ifdef _NON_VOLATILE_MEMORY
    }
    else
    {
        //
        // there is an invite key in data flash, return a pointer to it.
        //
        return (one_net_xtea_key_t*)(invite_key_ptr +
          sizeof(dfi_segment_hdr_t) + ON_RAW_SID_LEN);
    }
    #endif
}


#ifndef _ONE_NET_MULTI_HOP
on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack)
#else
on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
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
on_message_status_t one_net_client_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries)
#else
on_message_status_t one_net_client_handle_ack_nack_response(
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


#ifndef _ONE_NET_MULTI_HOP
void one_net_client_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack)
#else
void one_net_client_single_txn_status(on_message_status_t status,
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



//! @} ONE-NET_client_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_client_eval



#endif // #ifdef _ONE_NET_CLIENT //
