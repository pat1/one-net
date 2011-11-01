//! \addtogroup ONE-NET_CLIENT ONE-NET CLIENT device functionality
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
    \file one_net_client.c
    \brief ONE-NET CLIENT functionality implementation

    Derives from ONE-NET.  CLIENT dependent functionality.  This module is
    dependent on one_net_client_net, and must initialize one_net_client_net.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_client.h"
#include "one_net_client_port_const.h"
#include "one_net.h"
#include "one_net_port_specific.h"
#include "tick.h"



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_const
//! \ingroup ONE-NET_CLIENT
//! @{



//! @} ONE-NET_CLIENT_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_CLIENT_typedefs
//! \ingroup ONE-NET_CLIENT
//! @{

/*!
    \brief Keeps track of CLIENT transactions
*/



//! @} ONE-NET_CLIENT_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_CLIENT_pub_var
//! \ingroup ONE-NET_CLIENT
//! @{

//! Flag to signify that this client is part of a network.
BOOL client_joined_network = FALSE;

//! Flag to signify that the client is not part of a network and is looking for
//! an invitation.
BOOL client_looking_for_invite = FALSE;

#ifdef _ENHANCED_INVITE
    //! Flag to signify that an invitation attempt has expired without successfully
    //! joining a network.
    BOOL client_invite_timed_out = FALSE;
	
    //! Lowest channel to consider when looking for an invite
	one_net_channel_t low_invite_channel;
	
	one_net_channel_t high_invite_channel;	
#endif



//! @} ONE-NET_CLIENT_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================




//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_CLIENT_pri_var
//! \ingroup ONE-NET_CLIENT
//! @{


//! The MASTER device
on_master_t * const master
  = (on_master_t * const)(nv_param + sizeof(on_base_param_t));

//! The ONE_NET_RX_FROM_DEVICE_COUNT devices that have most recently sent data
//! to this device.
on_sending_dev_list_item_t sending_dev_list[ONE_NET_RX_FROM_DEVICE_COUNT];


//! @} ONE-NET_CLIENT_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================




//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{


static on_sending_device_t * sender_info(const on_encoded_did_t * const DID);


//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_pub_func
//! \ingroup ONE-NET_CLIENT
//! @{

/*!
    \brief Initializes a CLIENT to start looking for an invite message.

    This function should be called the first time a device starts up, or when
    the device should attempt to join another network.  Once a device has
    joined a network, one_net_client_init should be called to reinitialize the
    CLIENT.

    \param INVITE_KEY The unique key of this CLIENT to decrypt invite packets
      with.
    \param[in] SINGLE_BLOCK_ENCRYPT_METHOD The method to use to encrypt single
      and block packets when they are sent.
    \param[in] STREAM_ENCRYPT_METHOD The method to use to encrypt stream packets
      when they are sent.
    \param[in] min_channel lowest channel to scan on.
    \param[in] max_channel highest channel to scan on.
    \param[in] timeout_time Length of time in milliseconds to listen for the invite.
	  0 means indefinite.

    \return ONS_SUCCESS Successfully initialized the CLIENT.
            ONS_BAD_PARAM if the parameter is invalid.
*/
#if !defined(_ENHANCED_INVITE)
#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD)
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD)
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#else

#ifdef _STREAM_MESSAGES_ENABLED
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
      const UInt8 STREAM_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time)
#else // ifdef _STREAM_MESSAGES_ENABLED //
    one_net_status_t one_net_client_look_for_invite(
      const one_net_xtea_key_t * const INVITE_KEY,
      const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
	  const one_net_channel_t min_channel,
	  const one_net_channel_t max_channel,
	  const tick_t timeout_time)	  
#endif // else _STREAM_MESSAGES_ENABLED is not defined //
#endif
{
    return ONS_SUCCESS;
} // one_net_client_look_for_invite //


/*!
    \brief Initializes the CLIENT to run in a network that it has previously
      joined.

    If the CLIENT has not yet joined a network, one_net_client_look_for_invite
    needs to be called instead of this function.

    \param[in] PARAM The parameters that were saved to non-volatile memory.
    \param[in] PARAM_LEN The sizeof PARAM in bytes.

    \return ONS_SUCCESS If initializing the CLIENT was successful
            ONS_BAD_PARAM If any of the parameters are invalid
*/
one_net_status_t one_net_client_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN)
{
    return ONS_SUCCESS;
} // one_net_client_init //


/*!
    \brief The main function for the ONE-NET CLIENT.

    \param void

    \return The number of ticks the device can sleep for.
*/
tick_t one_net_client(void)
{
    return 0;
} // one_net_client //



//! @} ONE-NET_CLIENT_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_pri_func
//! \ingroup ONE-NET_CLIENT
//! @{



/*!
    \brief Initializes internal data structures.

    This will also initialize the base one_net functionality.

    \param void

    \return ONS_SUCCESS if the internals were successfully initialized
            ONS_INTERNAL_ERR if initializing the net layer was not successful
*/
static one_net_status_t init_internal(void)
{
    return ONS_SUCCESS;
} // init_internal //


/*!
    \brief Finds the sender info (or a location for the sender info).

    Loops through and finds the information for the sending device.  If the
    device has not heard from the sender before, a new location shall be
    returned.

    The return value should be checked for 0.  The expected_nonce and last nonce
    should then be compared.  If these two values are equal, then it is a new
    location so a NACK should be sent to the sender, and the new nonce filled
    out.  The last nonce value should not be a valid nonce value and should be
    left unchanged for the time being.

    \param[in] DID The device id of the sender.

    \return Pointer to location that holds the sender information (should be
      checked for 0, and should be checked if a new location).
*/
static on_sending_device_t * sender_info(const on_encoded_did_t * const DID)
{
    // indexes
    UInt8 i, match_idx, max_lru_idx;

    // either the lru of the matched device, or the max lru in the list
    UInt8 lru = 0;

    if(!DID)
    {
        return 0;
    } // if parameter is invalid //

    max_lru_idx = match_idx = 0;

    if(on_encoded_did_equal(DID,
      (const on_encoded_did_t * const)&(master->device.did)))
    {
        return &master->device;
    } // if the MASTER is the sender //

    // loop through and find the sender's information
    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        if(on_encoded_did_equal(DID,
          (const on_encoded_did_t * const)&(sending_dev_list[i].sender.did)))
        {
            match_idx = i;
            lru = sending_dev_list[i].lru;
            break;
        } // if the sender info was found //

        if(lru < sending_dev_list[i].lru)
        {
            lru = sending_dev_list[i].lru;
            max_lru_idx = i;
        } // if the device has a higher lru than the current max //
    } // loop to find sender info //

    if(match_idx != i)
    {
        // replace the least recently used device
        match_idx = max_lru_idx;
        one_net_memmove(sending_dev_list[match_idx].sender.did, *DID,
          sizeof(sending_dev_list[match_idx].sender.did));
        sending_dev_list[match_idx].sender.expected_nonce
          = one_net_prand(get_tick_count(), ON_MAX_NONCE);
        sending_dev_list[match_idx].sender.last_nonce = ON_INVALID_NONCE;
    } // if the device was not found in the list //

    if(lru)
    {
        // update the lru's in the list
        for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
        {
            if(i == match_idx)
            {
                sending_dev_list[i].lru = 0;
            } // if it is the index that matched //
            else
            {
                sending_dev_list[i].lru++;
            } // else it is not the index that matched //
        } // loop through devices and update the lrus //
    } // if the device was not the least recently used //

    return &(sending_dev_list[match_idx].sender);
} // sender_info //



//! @} ONE-NET_CLIENT_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT