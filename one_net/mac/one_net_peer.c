//! \addtogroup ONE-NET_PEER ONE-NET PEER functionality
//! \ingroup ONE-NET_PEER
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
    \file one_net_peer.c
    \brief ONE-NET peer functionality implementation.

    Handles peer assignments functionality
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"

#ifdef _PEER


#include "one_net_types.h"
#include "one_net_peer.h"
#include "one_net_application.h"
#include "one_net_constants.h"
#include "one_net_status_codes.h"
#include "one_net_message.h"



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{


// same as ON_ENCODED_BROADCAST_DID
const on_encoded_did_t INVALID_PEER = {0xB4, 0xB4};



//! @} ONE-NET_PEER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_PEER_typedefs
//! \ingroup ONE-NET_PEER
//! @{


//! @} ONE-NET_PEER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_PEER_pri_var
//! \ingroup ONE-NET_PEER
//! @{

//! @} ONE-NET_PEER_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_PEER_pub_var
//! \ingroup ONE-NET_PEER
//! @{
    
    
extern BOOL device_is_master;
extern const on_encoded_did_t MASTER_ENCODED_DID;
#ifdef _ONE_NET_CLIENT
#include "one_net.h"
extern on_master_t * const master;
extern BOOL client_joined_network;
extern BOOL send_to_master;
#endif


UInt8 peer_storage[PEER_STORAGE_SIZE_BYTES];

on_peer_unit_t* const peer = (on_peer_unit_t* const) &peer_storage[0];



//! @} ONE-NET_PEER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PEER_pri_func
//! \ingroup ONE-NET_PEER
//! @{


//! @} ONE-NET_PEER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PEER_pub_func
//! \ingroup ONE-NET_PEER
//! @{


/*!
    \brief Resets the peer memory.
    
    This should be called by the device when the device's init function is
    called or anytime the device peer memory is to be wiped out.
    
    \return ONS_SUCCESS if the peer memory was reset
            ONS_FAILURE otherwise
*/
one_net_status_t one_net_reset_peers(void)
{
    on_peer_unit_t empty_peer = {{0xB4, 0xB4}, ONE_NET_DEV_UNIT,
      ONE_NET_DEV_UNIT};
    one_net_memset_block(peer, sizeof(empty_peer),
      ONE_NET_MAX_PEER_UNIT, &empty_peer);
    return ONS_SUCCESS;
} // one_net_reset_peers //


/*!
    \brief Adds matching peer did / units to a message recipient list
    
    \param[in] msg the message being sent
    \param[out] send_list The list of devices and units to send to
    \param[in] peer_list the peer send list to uise when deciding
                 what dids / units matches the message.
*/
void add_peers_to_recipient_list(const on_single_data_queue_t*
  msg, on_recipient_list_t* send_list, const on_peer_unit_t* peer_list)
{
    UInt8 i;
    on_did_unit_t dst_did_unit;
    
    if(msg->msg_type != ON_APP_MSG || !msg->send_to_peer_list)
    {
        return; // not sending to peer list
    }

    for(i = 0; i < ONE_NET_MAX_PEER_PER_TXN; i++)
    {
        if(is_broadcast_did((const on_encoded_did_t*)
          (&(peer_list[i].peer_did))))
        {
            break; // end of peer list reached.
        }

        if(peer_list[i].src_unit != msg->src_unit)
        {
            continue;
        }
        
        // we have a match.
        one_net_memmove(dst_did_unit.did, peer_list[i].peer_did,
          ON_ENCODED_DID_LEN);
        dst_did_unit.unit = peer_list[i].peer_unit;
        
        add_recipient_to_recipient_list(send_list, &dst_did_unit);
    }
}


/*!
    \brief Saves the peer assignment that is being made.
    
    \param[in] SRC_UNIT The unit on this device being assigned the peer.
    \param[in/out] The peer list to update
    \param[in] PEER_DID The peer device being assigned to this device.
    \param[in] PEER_UNIT The unit in the peer device being assigned to this
      device.
      
    \return ONS_SUCCESS If the peer was successfully assigned
            ONS_BAD_PARAM If the parameters are invalid
            ONS_INVALID_DATA If the data is incorrect (such as a source unit
              that is out of range).
            ONS_INTERNAL_ERR If something unexpected happened
            ONS_RSRC_FULL If no more peer assignments can be assigned
*/
one_net_status_t one_net_add_peer_to_list(const UInt8 SRC_UNIT,
  on_peer_unit_t* peer_list, const on_encoded_did_t * const PEER_DID,
  const UInt8 PEER_UNIT)
{
    UInt8 index, unit_list_index;
    SInt8 insertion_index = -1; // negative means unset
    UInt8 num_peers_for_src = 0; // number of peers for this source

    if(!PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!peer_list)
    {
        peer_list = peer;
    }
    
    // make sure we have room
    if(one_net_memcmp(INVALID_PEER,
      peer_list[ONE_NET_MAX_PEER_UNIT - 1].peer_did,
      ON_ENCODED_DID_LEN) != 0)
    {
        // Last element is not empty.  List is full.
        return ONS_RSRC_FULL;
    }
	
    // There are a few possibilities
    // 1.  This peer assignment is already on the unit list.  If so, do nothing,
	//     return ONS_SUCCESS
    // 2.  This peer assignment is not on the unit list, but the unit list is
	//     full.  If so, do nothing and return ONS_RSRC_FULL.
    // 3.  This peer assignment is not on the unit list, and the unit list is
	//     not full.  If so, find the index it should be inserted into and
    //     proceed to step 4.	
    // 4.  Insert the new peer assignment in the unit list and return ONS_SUCCESS.
    
    // find a spot to add the peer.  List is ordered by source
    // unit, then peer unit.  Empty spots should all be at the end and are
    // represented by ONE_NET_DEV_UNIT.
    for(unit_list_index = 0; unit_list_index < ONE_NET_MAX_PEER_UNIT;
      unit_list_index++)
    {
        if(one_net_memcmp(INVALID_PEER, peer_list[unit_list_index].peer_did,
          ON_ENCODED_DID_LEN) == 0)
        {
            // found end of list.
            // found our insertion index if it's not already set
            if(insertion_index == -1)
            {
                insertion_index = unit_list_index;
            }
            break;
        }        
        
        if(peer_list[unit_list_index].src_unit > SRC_UNIT)
        {
            // found our insertion index if it's not already set
            if(insertion_index == -1)
            {
                insertion_index = unit_list_index;
            }
            break;
        }
		else if(peer_list[unit_list_index].src_unit < SRC_UNIT)
        {
            continue;
        }
        
        // source unit matches
        if(peer_list[unit_list_index].peer_unit == PEER_UNIT)
        {
            // this peer is already on the list.  Nothing to do.
            // Return ONS_SUCCESS
            return ONS_SUCCESS;
        }
        
        num_peers_for_src++; // found a peer for this source unit.
        if(num_peers_for_src >= ONE_NET_MAX_PEER_PER_TXN)
        {
            // no more room for peers for this source unit
            return ONS_RSRC_FULL;
        }
        
        if(peer_list[unit_list_index].peer_unit > PEER_UNIT)
		{
            // found our insertion index if it's not already set
            if(insertion_index == -1)
            {
                insertion_index = unit_list_index;
            }
		}
	}
     
    // we should never have an invalid index here, but check just to make sure
    if(insertion_index < 0 || insertion_index >= ONE_NET_MAX_PEER_UNIT)
    {
        return ONS_INTERNAL_ERR;
    }
    
    if(insertion_index < ONE_NET_MAX_PEER_UNIT - 1)
	{
        // We're about to insert.  Move any elements that may be in the way down one.
        one_net_memmove(&peer_list[insertion_index + 1],
          &peer[insertion_index], sizeof(peer_list[insertion_index]) *
          (ONE_NET_MAX_PEER_UNIT - insertion_index - 1));
	}

    // now fill in the new information.
    peer_list[insertion_index].src_unit = SRC_UNIT;
    peer_list[insertion_index].peer_unit = PEER_UNIT;
    one_net_memmove(peer_list[insertion_index].peer_did, *PEER_DID,
      ON_ENCODED_DID_LEN);
	return ONS_SUCCESS;
} // one_net_add_peer_to_list //



//! @} ONE-NET_PEER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PEER_pri_func
//! \ingroup ONE-NET_PEER
//! @{


//! @} ONE-NET_PEER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


#endif // if _PEER is defined //


//! @} ONE-NET_PEER
