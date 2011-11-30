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
#endif


UInt8 peer_storage[PEER_STORAGE_SIZE_BYTES];

on_peer_unit_t* const peer = (on_peer_unit_t* const) &peer_storage[0];

//! The list of peers to send to for THIS message
on_peer_send_list_t peer_send_list;

//! Pointer to the list of peers to send to for THIS message.  Generally
//! will point either to NULL or peer_send_list.  However, the user is
//! allowed to provide their own peer lists.
on_peer_send_list_t* peer_send_list_ptr = NULL;



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



on_peer_send_list_t* setup_send_list(on_single_data_queue_t* msg_ptr,
  const on_peer_unit_t* peer_list, on_peer_send_list_t* send_list)
{
    #ifdef _ONE_NET_CLIENT
    // only send status-type messages to the master if only
    // sending due to the ON_SEND_TO_MASTER flag
    BOOL send_to_master;
    #endif    

    peer_send_list_ptr = NULL;

    if(msg_ptr == NULL)
    {
        return NULL;
    }
    
    if(send_list == NULL)
    {
        send_list = &peer_send_list;
    }
    
    #ifdef _ONE_NET_CLIENT
    send_to_master = !device_is_master && client_joined_network
      && (master->flags & ON_SEND_TO_MASTER)
      && msg_ptr->msg_type == ON_APP_MSG && 
      ONA_IS_STATUS_MESSAGE(get_msg_class(msg_ptr->payload));
    #endif
    
    peer_send_list_ptr = send_list;
    
    if(peer_list == NULL)
    {
        peer_list = peer;
    }

    fill_in_peer_send_list(&(msg_ptr->dst_did), get_dst_unit(
      msg_ptr->payload), peer_send_list_ptr,
      msg_ptr->send_to_peer_list ? peer_list : NULL,
      #ifdef _ONE_NET_CLIENT
      &(msg_ptr->src_did), msg_ptr->src_unit, send_to_master);
      #else
      &(msg_ptr->src_did), msg_ptr->src_unit);
      #endif

    if(peer_send_list_ptr->num_send_peers <= 0)
    {
        return NULL;
    }
    
    peer_send_list_ptr->peer_send_index = -1;
    return send_list;
}


on_single_data_queue_t* load_next_peer(on_single_data_queue_t* msg_ptr)
{
    on_peer_send_item_t* peer_send_item;
    
    if(!msg_ptr || !peer_send_list_ptr)
    {
        return NULL;
    }
    
    (peer_send_list_ptr->peer_send_index)++;
    if(peer_send_list_ptr->peer_send_index >=
      peer_send_list_ptr->num_send_peers
      || peer_send_list_ptr->peer_send_index < 0)
    {
        peer_send_list_ptr->peer_send_index = -2; // end of list
        return NULL;
    }
    
    if(msg_ptr->msg_type != ON_APP_MSG)
    {
        return msg_ptr; // no changes.
    }
    
    peer_send_item =
      &(peer_send_list_ptr->peer_list[peer_send_list_ptr->peer_send_index]);
    one_net_memmove(msg_ptr->dst_did, peer_send_item->peer_did,
      ON_ENCODED_DID_LEN);
    put_dst_unit(peer_send_item->peer_unit, msg_ptr->payload);
    return msg_ptr;
}


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
      
    peer_send_list.peer_send_index = -2; // inactive 
    peer_send_list_ptr = NULL; 
    return ONS_SUCCESS;
} // one_net_reset_peers //


/*!
    \brief Fills in peer send list with the list of peers to send to
    
    \param[in] dst_did destination did of this message
    \param[in] dst_unit destination unit of this message
    \param[in] send_list the peer send list to fill in.  If NULL, the list
                 that ONE-NET provides will be used.  This parameter is
                 usually NULL.  Only applications that maintain their own
                 peer lists will have a non-NULL parameter here
    \param[in] peer_list The peer list.  If NULL, we are not sending to a
                 peer list.
    \param[in] src_did The "original" source of the message.  If NULL, the
                 source ID will be considered this device.  Used to prevent
                 circular messages of peers sending to the peers that
                 originally sent to THEM.
    \param[in] src_unit The source unit on THIS device for the message
    \param[in] src_unit The source unit on THIS device for the message
    \param[in] send_to_master True if master should be sent this message.
               Relevant only for clients.    
    \return pointer to a filled peer send list.  If send_list was not NULL,
                 it will be returned.  If it was NULL, the peer send list
                 that ONE-NET provides will be returned.
*/
on_peer_send_list_t* fill_in_peer_send_list(const on_encoded_did_t* dst_did,
  UInt8 dst_unit, on_peer_send_list_t* send_list, const on_peer_unit_t* peer_list,
  #ifdef _ONE_NET_CLIENT
  const on_encoded_did_t* src_did, UInt8 src_unit, BOOL send_to_master)
  #else
  const on_encoded_did_t* src_did, UInt8 src_unit)
  #endif
{
    UInt8 i;
    #ifdef _ONE_NET_CLIENT
    BOOL master_added = FALSE;
    #endif
    
    if(send_list == NULL)
    {
        send_list = &peer_send_list;
    }
    
    if(dst_did != NULL && on_encoded_did_equal(dst_did, &NO_DESTINALTION))
    {
        dst_did = NULL;
    }

    send_list->num_send_peers = 0;
    send_list->peer_send_index = -2;
    
    if(dst_did != NULL)
    {
        one_net_memmove(
          send_list->peer_list[send_list->num_send_peers].peer_did, *dst_did,
            ON_ENCODED_DID_LEN);
        send_list->peer_list[send_list->num_send_peers].peer_unit = dst_unit;
        (send_list->num_send_peers)++;
        
        #ifdef _ONE_NET_CLIENT
        if(!device_is_master && is_master_did(dst_did))
        {
            master_added = TRUE;
        }
        #endif
    }

    if(peer_list != NULL)
    {
        for(i = 0; send_list->num_send_peers < ONE_NET_MAX_PEER_PER_TXN &&
          i < ONE_NET_MAX_PEER_UNIT; i++)
        {
            if(src_unit != peer_list[i].src_unit)
            {
                continue;
            }
        
            if(src_did != NULL)
            {
                if(on_encoded_did_equal(src_did,
                  (on_encoded_did_t*) (peer_list[i].peer_did)))
                {
                    // This peer get the message before us or maybe even
                    // originated it, so don't send it.
                    continue;
                }
            }
        
            // now check to see if this did is the original destination
            if(dst_did != NULL && on_encoded_did_equal(dst_did,
              &(peer_list[i].peer_did)))
            {
                continue; // it's already been added
            }
        
            #ifdef _ONE_NET_CLIENT
            if(!device_is_master && is_master_did(&(peer_list[i].peer_did)))
            {
                if(master_added)
                {
                    // it's already been added.
                    continue;
                }
            
                // it hasn't been added yet, but it will be below, so flag it now
                master_added = TRUE;
            }
            #endif
        
            // we have a match.  Add it.
            one_net_memmove(
              send_list->peer_list[send_list->num_send_peers].peer_did,
              peer_list[i].peer_did, ON_ENCODED_DID_LEN);
            send_list->peer_list[send_list->num_send_peers].peer_unit =
              peer_list[i].peer_unit;
            (send_list->num_send_peers)++;
        }
    } // if peer_list is not NULL //

    #ifdef _ONE_NET_CLIENT
    if(send_to_master && !master_added)
    {
        // Add the master since we haven't yet and we need to.
        one_net_memmove(
          send_list->peer_list[send_list->num_send_peers].peer_did,
          MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);
        send_list->peer_list[send_list->num_send_peers].peer_unit =
          ONE_NET_DEV_UNIT;
        (send_list->num_send_peers)++;        
    }
    #endif

    return send_list;
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
