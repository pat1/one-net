//! \addtogroup ONE-NET_MASTER ONE-NET MASTER device functionality
//! \ingroup ONE-NET
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice,
          this list of conditions, and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of Threshold Corporation (trustee of ONE-NET) nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHEWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!
    \file one_net_master.c
    \brief ONE-NET MASTER functionality implementation

    Derives from ONE-NET.  MASTER dependent functionality.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


#include "config_options.h"
#include "one_net_master.h"
#include "one_net_timer.h"
#include "tick.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{


//! Number of bits to shift the initial CLIENT address to use as a 16-bit
//! or raw address value
#define ON_INIT_CLIENT_SHIFT RAW_DID_SHIFT


#ifdef _ONE_NET_MULTI_HOP
    #define ON_INVITES_BEFORE_MULTI_HOP 5
#endif


//! @} ONE-NET_MASTER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MASTER_typedefs
//! \ingroup ONE-NET_MASTER
//! @{

//! @} ONE-NET_MASTER_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MASTER_pub_var
//! \ingroup ONE-NET_MASTER
//! @{


//! MASTER specific parameters.  These need to be saved in non-volatile memory.
on_master_param_t * const master_param =
  (on_master_param_t * const)(&nv_param[0] + sizeof(on_base_param_t));
  
//! List of the CLIENTS
on_client_t * const client_list = (on_client_t * const)(&nv_param[0] +
  sizeof(on_base_param_t) + sizeof(on_master_param_t));
  
//! location to store the encoded data for an invite transaction.
UInt8 invite_pkt[ON_INVITE_ENCODED_PKT_SIZE];

//! The current invite transaction
on_txn_t invite_txn = {ONE_NET_NO_PRIORITY, 0, ON_INVALID_MSG_TYPE,
  ONT_INVITE_TIMER, 0, sizeof(invite_pkt), invite_pkt, NULL};


//! @} ONE-NET_MASTER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================



//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_MASTER_pri_var
//! \ingroup ONE-NET_MASTER
//! @{



/*!
    The length of time in ticks that the channel must be clear for before
    determining that the channel is ok for the network to operate on.
*/
static tick_t new_channel_clear_time_out = 0;

//! flag that indicates if any Multi-Hop Repeaters have joined the network
static BOOL mh_repeater_available = FALSE;



//! @} ONE-NET_MASTER_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
//! @{



static void init_internal(void);
static on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID);
static one_net_status_t rm_client(const on_encoded_did_t * const CLIENT_DID);
static void sort_client_list_by_encoded_did(void);
static UInt16 find_lowest_vacant_did(void);



//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MASTER_pub_func
//! \ingroup ONE-NET_MASTER
//! @{

/*!
    \brief Starts a new ONE-NET network.

    This should be called the very first time the MASTER starts up.  It creates
    a new ONE-NET network.  Once the network has been created, call
    one_net_master_init to initialize the MASTER with the network created when
    this function is called.

    \param[in] SID The raw SID of the MASTER.
    \param[in] KEY The xtea key to use for single and block transactions.
    \param[in] SINGLE_BLOCK_ENCRYPT_METHOD The method to use to encrypt single
      and block packets when they are sent.
    \param[in] STREAM_KEY The xtea key to use for stream transactions.
    \param[in] STREAM_ENCRYPT_METHOD The method to use to encrypt stream packets
      when they are sent.

    \return ONS_SUCCESS if the network was created.
            ONS_BAD_PARAM if the parameter was invalid
*/

#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
  const one_net_xtea_key_t * const STREAM_KEY,
  const UInt8 STREAM_ENCRYPT_METHOD)
#else
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD)
#endif  
{
    return ONS_SUCCESS;
} // one_net_master_create_network //


/*!
    \brief MASTER initializer

    This needs to be called before the MASTER is run.  Due to memory constraints
    of embedded systems, this function can be repeatedly called with only a
    subset of the parameters.  The calls must preserve the byte order of the
    parameters.

    \param[in] PARAM The parameters (or part) that were saved.
    \param[in] PARAM_LEN The size in bytes of the parameters being loaded.

    \return ONS_SUCCESS If loading all of the parameters have completed
              successfully.
            ONS_MORE If not done initializing the parameters and this
              function needs to be called again.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_INVALID_DATA If the data passed in is not valid (including being
              too long).  Initialization is reset and the parameters must be
              passed in from the beginning.
*/
one_net_status_t one_net_master_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN)
{
    return ONS_SUCCESS;
} // one_net_master_init //


one_net_status_t one_net_master_change_key(
  const one_net_xtea_key_fragment_t KEY_FRAGMENT)
{
    return ONS_SUCCESS;
} // one_net_master_change_key //


#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_change_stream_key(
  const one_net_xtea_key_t * const NEW_STREAM_KEY)
{
    return ONS_SUCCESS;
} // one_net_master_change_stream_key //
#endif


/*!
    \brief Starts the network process to invite a CLIENT to join the network.

    \param[in] KEY The unique key of the device to invite to join the network.

    \return ONS_SUCCESS if the process was successfully started
            ONS_BAD_PARAM if the parameter is invalid
            ONS_NOT_INIT The network has not been fully created.
            ONS_DEVICE_LIMIT If the MASTER cannot handle adding another device.
            ONS_RSRC_FULL if there is not an available transaction to send the
              invite.
            See on_encrypt & on_build_pkt for more return codes
*/
one_net_status_t one_net_master_invite(const one_net_xtea_key_t * const KEY)
{
    return ONS_SUCCESS;
} // one_net_master_invite //


/*!
    \brief Cancels an invite request

    Even though there can only be 1 outstanding invite request at a time, pass
    in the KEY in case later on there can be multiple outstanding invite
    requests.  This will prevent the interface from changing later on if the
    implementation is changed.

    \param[in] KEY The unique key of the device to cancel the invite request for

    \return ONS_SUCCESS if the invite request was canceled.
            ONS_BAD_PARAM if the parameter is invalid.
*/
one_net_status_t one_net_master_cancel_invite(
  const one_net_xtea_key_t * const KEY)
{
    return ONS_SUCCESS;
} // one_net_master_cancel_invite //


/*!
    \brief Starts the process to remove a device from the network.

    \param[in] RAW_PEER_DID The device to remove from the network

    \return ONS_SUCCESS If the process to remove the device was started
            ONS_BAD_PARAM If the parameter was invalid
*/
one_net_status_t one_net_master_remove_device(
  const on_raw_did_t * const RAW_PEER_DID)
{
    return ONS_SUCCESS;
} // one_net_master_remove_device //


/*!
    \brief The main function for the ONE-NET MASTER.

    \param void

    \return void
*/
void one_net_master(void)
{
    // The current transaction
    static on_txn_t * txn = 0;

    // Do the appropriate action for the state the device is in.
    switch(on_state)
    {
#ifdef _IDLE
		case ON_IDLE:
		{
			break;
		}
#endif

        case ON_JOIN_NETWORK:
        {
            break;
        } // ON_JOIN_NETWORK case //


        default:
        {
            one_net(&txn);
            break;
        } // default case //
    } // switch(on_state) //
} // one_net_master //


/*!
    \brief Add a client to the current network.

    This function can be used when you need to add a client
    to the current network and you do not want to use the normal
    invite/join process. A client is added with the information
    supplied and the information that the client will need to
    communicate on the current network is returned.

    \param[in] features The client's features / capabilities.
    \param[out] out_base_param Pointer to the base parameters the new client should use.
    \param[out] out_master_param Pointer to the master parameters the new client should use.

    \return ONS_SUCCESS if the client was added.
            ONS_BAD_PARAM if the parameter was invalid.
            ONS_DEVICE_LIMIT if there is no room to hold another client.
*/
one_net_status_t one_net_master_add_client(const on_features_t features,
  on_base_param_t* out_base_param, on_master_t* out_master_param)
{
    return ONS_SUCCESS;
} // one_net_master_add_client //



//! @} ONE-NET_MASTER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MASTER_pri_func
//! \ingroup ONE-NET_MASTER
//! @{

/*!
    \brief Initializes internal data structures.

    This will also initialize the base one_net functionality.

    \param void

    \return void
*/
static void init_internal(void)
{
} // init_internal //


/*!
    \brief Returns the CLIENT information for the given CLIENT.

    \param[in] CLIENT_DID The enocded device id of the CLIENT to retrieve the
      information for.

    \return The CLIENT information if the information was found
            0 If an error occured.
*/
static on_client_t * client_info(const on_encoded_did_t * const CLIENT_DID)
{
    UInt16 i;

    if(!CLIENT_DID)
    {
        return 0;
    } // if the parameter is invalid //

    for(i = 0; i < master_param->client_count; i++)
    {
        if(on_encoded_did_equal(CLIENT_DID,
          (const on_encoded_did_t * const)&client_list[i].device_send_info.did))
        {
            return &(client_list[i]);
        } // if the CLIENT was found //
    } // loop to find the CLIENT //

    return 0;
} // client_info //


/*!
    \brief Removes the CLIENT from the network (by removing it from the table) 

    \param[in] DID The device ID of the CLIENT to remove

    \return ONS_SUCCESS if the client was deleted.
            ONS_INVALID_DATA if the client was not deleted.
*/
static one_net_status_t rm_client(const on_encoded_did_t * const DID)
{
	return ONS_SUCCESS;
} // rm_client //


/*!
    \brief Sorts the client list by did for a cleaner printout.
*/
static void sort_client_list_by_encoded_did(void)
{
    UInt16 i;
    on_raw_did_t raw_did1;
    on_raw_did_t raw_did2;
    
    // the client list will almost always be in order already.  When it's not,
    // generally only one element will be out of order, so instead of having a
    // nested loop, if we ever find an element out of order, we'll just iterate
    // through the whole list again.
    for(i = 1; i < master_param->client_count; i++)
    {
        on_decode(raw_did1, client_list[i-1].device_send_info.did,
          ON_ENCODED_DID_LEN);
        on_decode(raw_did2, client_list[i].device_send_info.did,
          ON_ENCODED_DID_LEN);
          
        if(one_net_byte_stream_to_int16(raw_did1) >
          one_net_byte_stream_to_int16(raw_did2))
        {
            // swap.
            on_client_t temp = client_list[i-1];
            client_list[i-1] = client_list[i];
            client_list[i] = temp;
            i = 0; // make it 0, not 1 because the loop increments i.
        }
    }
}


/*!
    \brief Find the lowest vacant did that can be assigned to the next client.

    \return the next available client did that can be assigned in raw form
            stored as a UInt16 (i.e. 0020, 0030, 0040, etc.).  Returns 0
            if the list is full.
*/
static UInt16 find_lowest_vacant_did(void)
{
    UInt16 i;
    on_raw_did_t raw_did;
    UInt16 vacant_did = ONE_NET_INITIAL_CLIENT_DID;
    
    sort_client_list_by_encoded_did();
    
    if(master_param->client_count >= ONE_NET_MASTER_MAX_CLIENTS)
    {
        return 0; // list is full.
    }
    
    // Note that the list is already sorted.  We'll go through it and see if
    // there are any gaps.  If not, we'll add ONE_NET_INITIAL_CLIENT_DID to
    // the last client and return that.
    for(i = 0; i < master_param->client_count; i++)
    {
        on_decode(raw_did, client_list[i].device_send_info.did,
          ON_ENCODED_DID_LEN);
        if(one_net_byte_stream_to_int16(raw_did) != vacant_did)
        {
            // a vacant did has been found
            return vacant_did;
        }
        
        vacant_did += ON_CLIENT_DID_INCREMENT;
    }
    
    // no gaps were found.
    return vacant_did;
}



//! @} ONE-NET_MASTER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE-NET_MASTER
