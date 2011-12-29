//! \addtogroup ONE-NET_PEER ONE-NET PEER functionality
//! \ingroup ONE-NET_PEER
//! @{

/*
    Copyright (c) 2010, Threshold Corporation
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
#include "one_net.h" // June 24, 2011 - for on_is_my_DID function
#include "one_net_application.h"
#include "one_net_peer.h"
#include "one_net_port_specific.h"
#ifdef _ONE_NET_MASTER
#include "one_net_master.h"
#include "one_net_encode.h"
#endif


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{

#ifdef _ONE_NET_USE_ENCODING
// same as ON_ENCODED_BROADCAST_DID
const on_encoded_did_t INVALID_PEER = {0xB4, 0xB4};
#else
// same as ON_ENCODED_BROADCAST_DID
const on_encoded_did_t INVALID_PEER = {0x00, 0x00};
#endif


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



UInt8 peer_storage[PEER_STORAGE_SIZE_BYTES];


// TODO - should mster_peer and peer be const?
#ifdef _ONE_NET_MASTER
on_peer_unit_t* master_peer = (on_peer_unit_t*) peer_storage;
#endif
#ifdef _ONE_NET_CLIENT
on_peer_t* peer = (on_peer_t*) peer_storage;
peer_msg_mgr_t peer_msg_mgr;
#endif


//! @} ONE-NET_PEER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PEER_pri_func
//! \ingroup ONE-NET_PEER
//! @{


static int on_client_net_peer_dev_idx(const on_encoded_did_t *DID);
static void on_client_net_rm_dev(const on_encoded_did_t* const did);
static on_peer_unit_t * on_client_net_next_peer(peer_msg_mgr_t *mgr);

static one_net_status_t unassign_peer_adjust_peer_list(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
  on_peer_unit_t* const peer_unit_list, const UInt8 MAX_PEER_UNITS,
  const BOOL deviceIsMaster);
  
static one_net_status_t assign_peer_adjust_peer_list(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
  on_peer_unit_t* const peer_unit_list, const UInt8 MAX_PEER_UNITS,
  const BOOL deviceIsMaster);

static int peer_unit_cmp(const on_peer_unit_t* const unit1, const on_peer_unit_t* const unit2);

static UInt8 highest_peer_priority(void);
static BOOL peer_ready_to_send(const on_peer_unit_t* const unit,
    const UInt8 priority);


//! @} ONE-NET_PEER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PEER_pub_func
//! \ingroup ONE-NET_PEER
//! @{


#ifdef _ONE_NET_CLIENT
/*!
    \brief Returns the transaction nonce for the given peer.
    
    \param[in] PEER_DID The peer to retrieve the transaction nonce for.
    
    \return The transaction nonce to use when sending to the peer.
      ON_INVALID_NONCE will be returned if the peer does not exist.
*/
UInt8 on_client_net_txn_nonce_for_peer(const on_encoded_did_t * const PEER_DID)
{
    UInt16 peer_dev_idx = -1;

    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != -1)
    {
        return peer->dev[peer_dev_idx].nonce;
    } // if peer_dev //

    return ON_INVALID_NONCE;
} // on_client_net_txn_nonce_for_peer //


/*!
    \brief Sets the transaction nonce for the given peer
    
    \param[in] DID The encoded destination device id.
    \param[in] NEXT_NONCE The transaction noce to set for the peer.
    
    \return TRUE if the next transaction nonce was set.
            FALSE if the next transaction nonce was not set.
*/
BOOL on_client_net_set_peer_txn_nonce(const on_encoded_did_t * const DID,
  const UInt8 NEXT_NONCE)
{
    int dev_idx = -1;

    if(DID && NEXT_NONCE <= ON_MAX_NONCE
      && (dev_idx = on_client_net_peer_dev_idx(DID)) != -1)
    {
        peer->dev[dev_idx].nonce = NEXT_NONCE;
        return TRUE;
    } // if the parameters are good, and the peer was found //
    
    return FALSE;
} // on_client_net_set_peer_txn_nonce //


#ifdef _DATA_RATE
/*!
    \brief Returns the data rate to use when sending to the given peer.
    
    \param[in] PEER_DID The peer to retrieve the data rate for.
    
    \return The data rate to use when sending to the peer.
      ONE_NET_DATA_RATE_LIMIT will be returned if the peer does not exist.
*/
UInt8 on_client_net_data_rate_for_peer(const on_encoded_did_t * const PEER_DID)
{
    int peer_dev_idx = -1;

    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != -1)
    {
        return peer->dev[peer_dev_idx].data_rate;
    } // if peer_dev //

    return ONE_NET_DATA_RATE_LIMIT;
} // on_client_net_data_rate_for_peer //


/*!
    \brief Sets the data rate a peer receives at.
    
    \param[in] PEER_DID The peer whose data rate is being set.
    \param[in] DATA_RATE The data rate to set for the peer.
    
    \return TRUE if the peer's data rate was set.
            FALSE if the peer's data rate was not set.
*/
BOOL on_client_net_set_peer_data_rate(
  const on_encoded_did_t * const PEER_DID, const UInt8 DATA_RATE)
{
    int peer_dev_idx = -1;
    
    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != -1)
    {
        peer->dev[peer_dev_idx].data_rate = DATA_RATE;
        return TRUE;
    } // if peer_dev //
    
    return FALSE;
} // on_client_net_set_peer_data_rate //
#endif


/*!
    \brief Saves the peer assignment that is being made.
    
    \param[in] SRC_UNIT The unit on this device being assigned the peer.
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
one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT)
{
    return assign_peer_adjust_peer_list(SRC_UNIT, PEER_DID, PEER_UNIT,
      peer->unit, ONE_NET_MAX_PEER_UNIT, FALSE);
} // on_client_net_assign_peer //


/*!
    \brief Unassigns peer connection(s)

    SRC_UNIT is a wildcard if it equals ONE_NET_DEV_UNIT.
    PEER_UNIT is a wildcard if it equals ONE_NET_DEV_UNIT.
    PEER_DID is a wildcard if it is NULL or equals ON_ENCODED_BROADCAST_DID.
	
	The peer table is traversed record by record and three comparisons are
	made for each record/peer assignment in the table:
	
	1. Does the source unit in the record match SRC_UNIT?
	2. Does the peer unit in the record match PEER_UNIT?
    3. Does the DID in the record match PEER_DID?
	
	If the parameter passed to the function is a wildcard, the comparison is
	considered true.
	
	If all three comparisons are true, then the record/peer assignment is deleted.



    \param[in] SRC_UNIT The source unit(s) of the peer connection to remove.
    \param[in] PEER_DID The device ID(s) of the peer whose connection is being
      removed.
    \param[in] PEER_UNIT The peer unit(s) of the connection being removed
    
    \return ONS_SUCCESS If the table was successfully adjusted (or the
              no adjustment was needed).
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_INVALID_DATA If the peer source unit does not exist.
*/
one_net_status_t on_client_net_unassign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT)
{
    return unassign_peer_adjust_peer_list(SRC_UNIT, PEER_DID, PEER_UNIT,
        peer->unit, ONE_NET_MAX_PEER_UNIT, FALSE);
} // on_client_net_unassign_peer //


/*!
    \brief Clears a peer message manager and makes it available
    
    \param[in/out] The peer message manager to clear.
    
    \return void
*/
void on_client_net_clear_peer_msg_mgr(peer_msg_mgr_t *mgr)
{
    if(!mgr)
    {
        return;
    } // if the parameter is invalid //
    
    mgr->active = FALSE;
} // on_client_net_clear_peer_msg_mgr //


/*!
    \brief Checks whether the iteration through the peer
	list is complete(i.e. checks whether there are any
	more peers for a particular source unit.
    
    \param[in/out] The peer message manager to check.
    
    \return 0/FALSE if there are no more peers in the list.
	        1/TRUE if there is at least one more peer in the list.
*/
UInt8 have_more_peers(peer_msg_mgr_t *mgr)
{
    UInt8 index;
	
    if(!mgr)
    {
        return 0;
    } // if the parameter is invalid //
	
    index = mgr->current_idx;
	
    while(index < ONE_NET_MAX_PEER_UNIT)
    {
		if(peer->unit[index].src_unit == mgr->src_unit)
        {
            return 1;
        }
		
       index++;
    }
	
    return 0;
}


static BOOL get_flag_shift_and_mask(const UInt8 flag, UInt8* const shift,
    UInt8* const mask)
{    
    switch(flag)
    {
        case PEER_SEND:
            *mask = PEER_SENT_MASK;
            *shift = PEER_SENT_SHIFT;
            break;
        case PEER_DELIVERED:
            *mask = PEER_DELIVERED_MASK;
            *shift = PEER_DELIVERED_SHIFT;
            break;
        case PEER_FAIL:
            *mask = PEER_FAIL_MASK;
            *shift = PEER_FAIL_SHIFT;
            break;
        case PEER_PRIORITY:
            *mask = PEER_PRIORITY_MASK;
            *shift = PEER_PRIORITY_SHIFT;
            break;
        #if 0
        case PEER_RETRY:
            *mask = PEER_RETRY_MASK;
            *shift = PEER_RETRY_SHIFT;
            break;
        #endif
        default:
            return FALSE;
    }
    
    return TRUE;       
}


static UInt8 get_flag_value(const on_peer_unit_t* const unit, const UInt8 flag)
{
    UInt8 mask, shift;
    if(!get_flag_shift_and_mask(flag, &shift, &mask))
    {
        return 0;
    }
    
    return (unit->flags & mask) >> shift;
}


static BOOL set_flag_value(on_peer_unit_t* const unit, const UInt8 flag,
    const UInt8 value)
{
    UInt8 mask, shift;
    if(!get_flag_shift_and_mask(flag, &shift, &mask))
    {
        return FALSE;
    }
    
    unit->flags &= ~mask;
    unit->flags |= (value << shift);
    return TRUE;
}


/*!
    \brief Sets up the message to be sent to the next peer.
    
    The next peer for the source unit will be abtained.  The message will be
    updated with the destination unit of the peer (if needed), and the encoded
    did of the peer will be returned.
    
    \param[in/out] data The message being sent.
    \param[in/out] mgr The peer manager for the message
    \param[in] result The result of the last transaction, if any.  If this is the
               FIRST transaction, this should be PEER_SEND.               
    \param[out] dst_did The encoded destination did for the message.
    
    \return ONS_SUCCESS If the operation was successful
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_END If there were no more peers to send the message to
*/
one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
  peer_msg_mgr_t *mgr, UInt8 result, on_encoded_did_t *dst_did)
{
    on_peer_unit_t * peer_unit = &(peer->unit[mgr->current_idx]);

    if(!data || !mgr || !dst_did)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(result != PEER_SEND)
    {
        // PEER_SEND represents the FIRST message to be sent, so there is no
        // result from the LAST message.       
        
        if(result == PEER_DELIVERED)
        {
            set_flag_value(peer_unit, PEER_DELIVERED, TRUE);
        }
        #if 0
        else if(result == PEER_RETRY)
        {
            UInt8 retry = get_flag_value(peer_unit, PEER_RETRY);
            retry++;
            
            if(retry > PEER_NUM_RETRY)
            {
                result = PEER_FAIL;
            }
            
            set_flag_value(peer_unit, PEER_RETRY, retry);
        }
        #endif
        else if(result == PEER_FAIL)
        {
            // do nothing.  handle below
        }
        else
        {
            return ONS_BAD_PARAM;
        }
        
        if(result == PEER_FAIL)
        {
            set_flag_value(peer_unit, PEER_PRIORITY, PEER_LOW_PRIORITY);
        }
        
        // no longer sending, so clear it
        set_flag_value(peer_unit, PEER_SEND, FALSE);
        
        // now increment the index since we just did this one
        mgr->current_idx++;
        if(mgr->current_idx >= ONE_NET_MAX_PEER_UNIT)
        {
            mgr->current_idx = 0;
        }
    }

    // find the next peer
    if(!(peer_unit = on_client_net_next_peer(mgr)))
    {
        return ONS_END;
    } // if there are no more peers to send to //
    
    // copy the destination did
    one_net_memmove(*dst_did, peer_unit->peer_did,
      sizeof(on_encoded_did_t));
    
    // adjust the message
    put_dst_unit(peer_unit->peer_unit, data);

    set_flag_value(peer_unit, PEER_SEND, TRUE);
    return ONS_SUCCESS;
} // on_client_net_setup_msg_for_peer //
#endif


#ifdef _ONE_NET_MASTER
/*!
    \brief Called when the MASTER is assigned a peer.
    
    \param[in] src_unit The unit in the MASTER being assigned the peer.
    \param[in] peer_did The did of the peer the MASTER is being assigned.
    \param[in] peer_unit The unit in the peer the MASTER is being assigned.   
    
    \return ONS_SUCCESS If the assignent was successfully made
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_RSRC_FULL If there is no more room on the peer table
*/
one_net_status_t master_assigned_peer(const UInt8 src_unit,
      const on_encoded_did_t * const peer_did, const UInt8 peer_unit)
{
    return assign_peer_adjust_peer_list(src_unit, peer_did, peer_unit,
      master_peer, NUM_MASTER_PEER, TRUE);
} // master_assigned_peer //



/*!
    \brief Initializes the Master's peer settings.
    
    \param void
    
    \return void
*/
void init_master_peer()
{
    UInt8 i;
    
    for(i = 0; i < NUM_MASTER_PEER; i++)
    {
        master_peer[i].src_unit = ONE_NET_DEV_UNIT;
        master_peer[i].peer_unit = ONE_NET_DEV_UNIT;
        one_net_memmove(master_peer[i].peer_did, ON_ENCODED_BROADCAST_DID,
            ON_ENCODED_DID_LEN);
    }
} // init_master_peer //


/*!
    \brief Unassigns peer connection(s)

    src_unit is a wildcard if it equals ONE_NET_DEV_UNIT.
    peer_unit is a wildcard if it equals ONE_NET_DEV_UNIT.
    peer_did is a wildcard if it is NULL or equals ON_ENCODED_BROADCAST_DID.
	
	The peer table is traversed record by record and three comparisons are
	made for each record/peer assignment in the table:
	
	1. Does the source unit in the record match src_unit?
	2. Does the peer unit in the record match peer_did?
    3. Does the DID in the record match peer_did?
	
	If the parameter passed to the function is a wildcard, the comparison is
	considered true.
	
	If all three comparisons are true, then the record/peer assignment is deleted.



    \param[in] src_unit The source unit(s) of the peer connection to remove.
    \param[in] peer_did The device ID(s) of the peer whose connection is being
      removed.
    \param[in] peer_unit The peer unit(s) of the connection being removed
    \param[in] deviceIsMaster True if this physical device is a master, false otherwise
    
    \return ONS_SUCCESS If the table was successfully adjusted (or the
              no adjustment was needed).
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_INVALID_DATA If the peer source unit does not exist.
*/
one_net_status_t master_unassigned_peer(const UInt8 src_unit,
  const on_encoded_did_t* const peer_did, const UInt8 peer_unit,
  const BOOL deviceIsMaster)
{
    return unassign_peer_adjust_peer_list(src_unit, peer_did, peer_unit,
	    master_peer, NUM_MASTER_PEER, TRUE);
} // master_unassigned_peer //
#endif


#ifdef _ONE_NET_CLIENT
one_net_status_t initialize_peer_mgr(peer_msg_mgr_t* const mgr,
    const UInt8 priority, const UInt8 src_unit)
{
    UInt8 i;
    BOOL found_peer = FALSE;
    UInt8 peer_priority = PEER_LOW_PRIORITY << PEER_PRIORITY_SHIFT;
	
    if(!mgr)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //
    
    if(mgr->active)
    {
        return ONS_RSRC_FULL;
    }
    
    switch(priority)
    {
        case ONE_NET_LOW_PRIORITY: break;
        case ONE_NET_HIGH_PRIORITY: peer_priority =
                 PEER_HIGH_PRIORITY << PEER_PRIORITY_SHIFT; break;
        default: return ONS_BAD_PARAM;
    }
    
    if(src_unit != ONE_NET_DEV_UNIT && src_unit >= ONE_NET_NUM_UNITS)
    {
        return ONS_BAD_PARAM;
    }
    
    mgr->src_unit = src_unit;
    mgr->current_idx = 0;
    
    for(i = 0; i < ONE_NET_MAX_PEER_UNIT; i++)
    {
        peer->unit[i].flags = 0;
		if(peer->unit[i].src_unit != ONE_NET_DEV_UNIT &&
               peer->unit[i].src_unit == mgr->src_unit)
        {
            if(!found_peer)
            {
                mgr->current_idx = i;
            }
            found_peer = TRUE;
            // this is a peer we want to send to.  Set priority.
            // Everything is already 0, so no need to set.
            peer->unit[i].flags |= peer_priority;
        }
    }
	
    if(!found_peer)
    {
        mgr->active = FALSE;
        return ONS_END;
    }

    mgr->active = TRUE;
    return ONS_SUCCESS;
}
#endif


#ifdef _ONE_NET_MASTER
UInt8 get_master_peer_count(const UInt8 src_unit)
{
    UInt8 peer_count = 0;
    UInt8 i;
    
    for(i = 0; i < NUM_MASTER_PEER; i++)
    {
        if(master_peer[i].src_unit == src_unit)
        {
            peer_count++;
        }
    }
    
    return peer_count;
}


one_net_status_t master_send_to_peer_list(UInt8* const data,
    const UInt8 data_len, const UInt8 priority, const UInt8 src_unit)
{
    UInt8 i;
    one_net_raw_did_t raw_did;
    

    // first check whether there are enough free transactions to send them 
    // all at once.  If not, reject.  This is an all or none thing.
    if(get_master_peer_count(src_unit) > master_get_num_free_send_txn())
    {
        // do not have the resources to send to all peers at the moment.
        return ONS_RSRC_FULL;
    }
    
    for(i = 0; i < NUM_MASTER_PEER; i++)
    {
        if(master_peer[i].src_unit == src_unit)
        {
            // note that the send_to_peer_list flag is set FALSE here.
            // We're already doing it here.  We don't want THESE messages
            // to go through the whole peer list.  Then we'd have an infinite
            // process.
            put_dst_unit(master_peer[i].peer_unit, data);
            on_decode(raw_did, master_peer[i].peer_did, ON_ENCODED_DID_LEN);
            
            // we aren't going to put them in the queue.  We're going to send
            // them directly because 1) we know we can 2) If we try to put it
            // in the queue there will be a data race since the request that
            // caused THIS function to get called won't get popped off the queue
            // until this function returns and every time you insert something onto
            // the queue, there is the potential to pop something OFF the queue so
            // we'll pop the SAME element off twice.  We might be checking that
            // queue too often, but for now, a good way to avoid the problems is
            // to avoid adding anything to the queue.
            
            // We avoid the queue by calling one_net_master_send_single directly
            // with the last two parameters NULL.
            one_net_master_send_single(data, data_len, FALSE, priority,
                &raw_did, src_unit, NULL, NULL);                 
        }
    }
    
    return ONS_SUCCESS;
}
#endif


//! @} ONE-NET_PEER_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PEER_pri_func
//! \ingroup ONE-NET_PEER
//! @{


#ifdef _ONE_NET_CLIENT
/*!
    \brief Returns the index into the peer device list for the addressed peer.

    \param[in] DID The device id of the peer to look up.

    \return The index into the peer device list for DID if DID is in the list.
            -1 if the DID is not in the list
*/
static int on_client_net_peer_dev_idx(const on_encoded_did_t *DID)
{
    int i;

    if(!DID)
    {
        return -1;
    } // if parameter is invalid //

    for(i = 0; i < ON_NUM_PEER_DEV; i++)
    {
        if(on_encoded_did_equal(DID, (const on_encoded_did_t * const)
          &(peer->dev[i].did)))
        {
            return i;
        } // if this device is responding to its peer //
    } // loop through all peer devices //

    return -1;
} // on_client_net_peer_dev_idx //


/*!
    \brief Removes a device from the list of peer devices that this device sends
      to.

    This also updates the peers to use the correct index since the device
    indexes change to ensure there are no holes in the device list.
	
    This DOES NOT check to make sure tht peer units that used to point to this
    index have already been deleted.  This deletion must occur before this function is
    called.

    \param[in] did The did to be removed
      being removed.

    \return void
*/
static void on_client_net_rm_dev(const on_encoded_did_t* const did)
{
    int index = on_client_net_peer_dev_idx(did);
	
    if(index == -1)
    {
        return;
    }
	
    if(index < ONE_NET_MAX_PEER_DEV - 1)
    {
        one_net_memmove(&(peer->dev[index]), &(peer->dev[index+1]),
            (ONE_NET_MAX_PEER_DEV - index - 1) * sizeof(on_peer_dev_t));
	}

    one_net_memmove(peer->dev[ON_NUM_PEER_DEV - 1].did,
      ON_ENCODED_BROADCAST_DID, sizeof(on_encoded_did_t));
} // on_client_net_rm_dev //


static UInt8 highest_peer_priority(void)
{
    UInt8 i, j;
    UInt8 priorities[3] =
    {
        PEER_HIGH_PRIORITY << PEER_PRIORITY_SHIFT,
        PEER_LOW_PRIORITY << PEER_PRIORITY_SHIFT,
        PEER_NO_PRIORITY << PEER_PRIORITY_SHIFT,
    };
    
    for(i = 0; i < 2; i++)
    {
        for(j = 0; j < ONE_NET_MAX_PEER_UNIT; j++)
        {
            // if they've been delivered or failed, we're no longer interested
            // in them.
            // If they've been sent, presumably we shouldn't eer be CALLING this
            // function?  At any rate, we don't want to send it twice, so we'll
            // mask "send" as well.
            if((peer->unit[j].flags) & PEER_SENT_DELIVERED_FAIL_MASK)
            {
                continue;
            }
            if(((peer->unit[j].flags) & PEER_PRIORITY_MASK) == priorities[i])
            {
                return priorities[i];
            }
        }
    }
    
    return priorities[2];
}


static BOOL peer_ready_to_send(const on_peer_unit_t* const unit,
    const UInt8 priority)
{
    if(unit->flags & PEER_PRIORITY_MASK != priority)
    {
        return FALSE;
    }
    
    if(unit->flags & PEER_SENT_DELIVERED_FAIL_MASK)
    {
        return FALSE;
    }
    
    return TRUE;
}


/*!
    \brief Retrieves the next peer connection for the manager.
    
    \param[in/out] mgr The manager to get the next peer for.  This will be
      updated so that it is ready to be passed in the next time.  If there are
      no more peers, mgr will NOT be updated.

    \return The next peer to send to.  0 will be returned if there are no more
      peers left, or the parameter is invalid.
*/
static on_peer_unit_t * on_client_net_next_peer(peer_msg_mgr_t *mgr)
{
    UInt8 priority = highest_peer_priority();   
    UInt8 original_idx; // shouldn't need this

    if(priority == PEER_NO_PRIORITY << PEER_PRIORITY_SHIFT)
    {
        return 0;
    }
    
    if(!mgr)
    {
        return 0;
    } // if the parameter is invalid //
    
    original_idx = mgr->current_idx; // shouldn't need this, but check anyway
    
    while(!peer_ready_to_send(&(peer->unit[mgr->current_idx]), priority))
    {
        (mgr->current_idx)++;
        if((mgr->current_idx) >= ONE_NET_MAX_PEER_UNIT)
        {
            mgr->current_idx = 0;
            
            // hopefully do not need this, but add this to prevent infinite loop
            if(original_idx == mgr->current_idx)
            {
                return 0;
            }
        }
    }

    return &(peer->unit[mgr->current_idx]);
} // on_client_net_next_peer //
#endif


/*!
    \brief Saves the peer assignment that is being made.
    
    \param[in] SRC_UNIT The unit on this device being assigned the peer.
    \param[in] PEER_DID The peer device being assigned to this device.
    \param[in] PEER_UNIT The unit in the peer device being assigned to this
      device.
      
    \return ONS_SUCCESS If the peer was successfully assigned
            ONS_BAD_PARAM If the parameters are invalid
            ONS_INVALID_DATA If the data is incorrect (such as a source unit
              that is out of range).
            ONS_DID_FAILED If assigning to an invalid DID(i.e. yourself)
            ONS_INTERNAL_ERR If something unexpected happened
            ONS_RSRC_FULL If no more peer assignments can be assigned
*/
//TO-DO : add some more parameter comments.
static one_net_status_t assign_peer_adjust_peer_list(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
  on_peer_unit_t* const peer_unit_list, const UInt8 MAX_PEER_UNITS,
  const BOOL deviceIsMaster)
{
    UInt8 index, unit_list_index, dev_list_index;
	on_peer_unit_t peerToAdd;
    int compare;
	
#ifdef _ONE_NET_CLIENT
	BOOL vacantDevSpotExists;
#endif

    if(!PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(did_is_broadcast(*PEER_DID) || SRC_UNIT > ONE_NET_NUM_UNITS || PEER_UNIT == ONE_NET_DEV_UNIT)
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //
    
    // June 24, 2011 - Current disallowing a device to assign a peer to a
    // unit of the same device.
    // TODO - Possibly allow this later.
    if(on_is_my_DID(PEER_DID))
    {
        return ONS_DID_FAILED;
    }
	
    // There are a few possibilities
    // 1.  This peer assignment is already on the unit list.  If so, do nothing,
	//     return ONS_SUCCESS
    // 2.  This peer assignment is not on the unit list, but the unit list is
	//     full.  If so, do nothing and return ONS_RSRC_FULL.
    // 3.  This peer assignment is not on the unit list, and the unit list is
	//     not full.  If so, find the index it should be inserted into and
    //     proceed to step 4.
	
	// Note : Steps 4 through 6 only apply if deviceIsMaster is false.  If
	// deviceIsMaster is true, skip to step 7.
	
    // 4.  Check the dev list and see if PEER_DID is already there.  If it is,
    //     note the index and proceed to step 7.
    // 5.  PEER_DID is not on the dev list yet.  Check if the dev list is full.
    //     If it is, return ONS_RSRC full.
    // 6.  Add PEER_DID to the end of the dev list.  Note the new index.
    //     Proceed to step 7.
	
	
    // 7.  Insert the new peer assignment in the unit list and return ONS_SUCCESS.
    
    // find a spot to add the peer.  List is ordered by peer did, then source
    // unit, then peer unit.  Empty spots should all be at the end and are
    // represented by ONE_NET_DEV_UNIT.
	
	// First make sure there's actually room to add to the list.  Check the last spot to make sure
	// it is empty
	if(!did_is_broadcast(peer_unit_list[MAX_PEER_UNITS - 1].peer_did))
	{
        // unit list is full
		return ONS_RSRC_FULL;
	}
	
	one_net_memmove(peerToAdd.peer_did, *PEER_DID, ON_ENCODED_DID_LEN);
	peerToAdd.src_unit = SRC_UNIT;
	peerToAdd.peer_unit = PEER_UNIT;
	
	unit_list_index = 0;
	compare = peer_unit_cmp(&(peer_unit_list[unit_list_index]), &peerToAdd);
    while(compare < 0)
    {
        unit_list_index++;
		compare = peer_unit_cmp(&(peer_unit_list[unit_list_index]), &peerToAdd);
	}
	 
	if(compare == 0)
	{
        // unit is already on the list.  Just do nothing and return success
	    return ONS_SUCCESS;
	}

#ifdef _ONE_NET_CLIENT
	if(!deviceIsMaster)
    {
        // check whether we have any room on the dev list
		vacantDevSpotExists = did_is_broadcast(peer->dev[ONE_NET_MAX_PEER_DEV - 1].did);
		
        // we are a client, so we may need to adjust the peer->dev array.		
        // now let's look through the dev list and see if we need to insert anything.    
		dev_list_index = 0;
		compare = enc_did_cmp(&(peer->dev[dev_list_index].did), PEER_DID);
		// we checked everything before, so we should be safe from this while loop running
		// off the cliff into a seg fault or an infinite loop.
		while(compare < 0)
		{
			dev_list_index++;
			compare = enc_did_cmp(&(peer->dev[dev_list_index].did), PEER_DID);
		}
		
		if(compare != 0)
		{
            // device is not on the list.  Add it.
			if(!vacantDevSpotExists)
			{
			    return ONS_RSRC_FULL; // device list is full.
			}
			
            // Need to insert at dev_list_index.  First move everything down one spot
			one_net_memmove(&(peer->dev[dev_list_index + 1]), &(peer->dev[dev_list_index]),
			    (ONE_NET_MAX_PEER_DEV - dev_list_index - 1) * sizeof(on_peer_dev_t));
			
			// fill in the new data
			// TODO - what about the nonce and the data rate?
            one_net_memmove(peer->dev[dev_list_index].did, *PEER_DID,
                sizeof(peer->dev[dev_list_index].did));                     
		}
    }
#endif

    if(unit_list_index < MAX_PEER_UNITS - 1)
	{
        // We're about to insert.  Move any elements that may be in the way down one.
        one_net_memmove(&peer_unit_list[unit_list_index + 1], &peer_unit_list[unit_list_index],
          sizeof(on_peer_unit_t) * (MAX_PEER_UNITS - unit_list_index - 1));
	}

    // now fill in the new information.
    one_net_memmove(&(peer_unit_list[unit_list_index]), &peerToAdd, sizeof(on_peer_unit_t));
	
	return ONS_SUCCESS;
} // assign_peer_adjust_peer_list //



/*!
    \brief Unassigns peer connection(s)

    SRC_UNIT is a wildcard if it equals ONE_NET_DEV_UNIT.
    PEER_UNIT is a wildcard if it equals ONE_NET_DEV_UNIT.
    PEER_DID is a wildcard if it is NULL or equals ON_ENCODED_BROADCAST_DID.
	
	The peer table is traversed record by record and three comparisons are
	made for each record/peer assignment in the table:
	
	1. Does the source unit in the record match SRC_UNIT?
	2. Does the peer unit in the record match PEER_UNIT?
    3. Does the DID in the record match PEER_DID?
	
	If the parameter passed to the function is a wildcard, the comparison is
	considered true.
	
	If all three comparisons are true, then the record/peer assignment is deleted.



    \param[in] SRC_UNIT The source unit(s) of the peer connection to remove.
    \param[in] PEER_DID The device ID(s) of the peer whose connection is being
      removed.
    \param[in] PEER_UNIT The peer unit(s) of the connection being removed
    \param[in] peer_unit_list The list of peer units to adjust.
    \param[in] MAX_PEER_UNITS The maximum number of peer units the device can handle.
    \param[in] deviceIsMaster True if the device running this code is presently
	           functioning as a master, false otherwise
    
    \return ONS_SUCCESS If the table was successfully adjusted (or the
              no adjustment was needed).
            ONS_INVALID_DATA If the data is incorrect (such as a source unit
              that is out of range)
	        ONS_BAD_PARAM If any of the parameters are invalid.
*/
static one_net_status_t unassign_peer_adjust_peer_list(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
  on_peer_unit_t* const peer_unit_list, const UInt8 MAX_PEER_UNITS,
  const BOOL deviceIsMaster)
{
    UInt16 index;
	BOOL src_unit_match, peer_unit_match, did_match, did_wildcard;
	one_net_status_t status;

    // You will get a warning for the first comparison if
    // ONE_NET_NUM_UNITS is 0 saying the comparison cannnot be false.
    // ignore it.  We don't want to reject unit 0 because some functions
    // seem to use unit 0 instead of ONE_NET_DEV_UNIT.
    if((SRC_UNIT >= ONE_NET_NUM_UNITS) && (SRC_UNIT != ONE_NET_DEV_UNIT) &&
       (SRC_UNIT != 0))
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //
	
    did_wildcard = FALSE;
	
    if(!PEER_DID || on_encoded_did_equal(PEER_DID, &ON_ENCODED_BROADCAST_DID))
    {
        // peer did is a wildcard
        did_wildcard = TRUE;
    }
	
    // go through peer list a record at a time.
    // Note : index < 0xFFFF will be false when 1 is subtracted from 0 with index-- command.
    for(index = MAX_PEER_UNITS - 1; index < 0xFFFF; index--)
    {
		// check whether criteria for deleting this peer unit are fulfilled
        if(peer_unit_list[index].src_unit == ONE_NET_DEV_UNIT)
        {
            // Blank spot.  Nothing to do.
            continue;
        }
		
        src_unit_match = (SRC_UNIT == ONE_NET_DEV_UNIT || SRC_UNIT == peer_unit_list[index].src_unit);
        peer_unit_match = (PEER_UNIT == ONE_NET_DEV_UNIT || PEER_UNIT == peer_unit_list[index].peer_unit);
        did_match = (did_wildcard || on_encoded_did_equal(PEER_DID, &(peer_unit_list[index].peer_did)));
		  
        if(!src_unit_match || !peer_unit_match || !did_match)
        {
            continue;
        }
				
        // we want to delete.  Move everything farther down the list up one.
        if(index < MAX_PEER_UNITS - 1)
        {
            one_net_memmove(&peer_unit_list[index], &peer_unit_list[index + 1],
                sizeof(on_peer_unit_t) * (MAX_PEER_UNITS - index - 1));
        }

        // make the last spot blank		
        peer_unit_list[MAX_PEER_UNITS - 1].src_unit = ONE_NET_DEV_UNIT;
        peer_unit_list[MAX_PEER_UNITS - 1].peer_unit = ONE_NET_DEV_UNIT;
		one_net_memmove(peer_unit_list[MAX_PEER_UNITS - 1].peer_did,
            ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
    }
	
    #ifdef _ONE_NET_CLIENT
    // If the device is a master, all the did information is saved regardless of any peer
    // assignments.  Clients, however, only keep track of other devices if they are peers,
    // so we need to adjust that list.  deviceIsMaster should be TRUE if the device running
	// this code is running as a Master.
	if(!deviceIsMaster)
    {
        // The device must be a client.  Go through the peer device list.  See if anything
		// still sends to the did passed to this function.  If not, delete it altogether since
		// we no longer need to keep track of it.
        for(index = 0; index < MAX_PEER_UNITS; index++)
        {
			if(on_encoded_did_equal(PEER_DID, &(peer_unit_list[index].peer_did)))
            {
                 // at least one peer sends to this did.  Don't delete.
				 return ONS_SUCCESS;
            }
        }
		
        // no devices send to it.  Delete it.
		on_client_net_rm_dev(PEER_DID);
    }
    #endif
	
    return ONS_SUCCESS;
} // unassign_peer_adjust_peer_list //


/*!
    \brief Compares two peer units and sees which is "smaller"

    Peer units are ordered by this criteria
	1) Encoded Peer DID
	2) Source Unit
	3) Peer Unit
	
    \param[in] unit1 First peer unit to compare
    \param[in] unit2 Second peer unit to compare
    
    \return negative number if unit1 is "smaller" than did2
            postive number if unit1 is "larger" than did2
            0 if unit1 and unit2 have the same values
*/
static int peer_unit_cmp(const on_peer_unit_t* const unit1, const on_peer_unit_t* const unit2)
{
    int val1, val2;
    int didCmp = enc_did_cmp(&(unit1->peer_did), &(unit2->peer_did));
		
	if(didCmp == 0)
	{
        // dids are equal
		if(unit1->src_unit < unit2->src_unit)
		{
			return -1;
		}
		if(unit1->src_unit > unit2->src_unit)
		{
			return 1;
		}
		if(unit1->peer_unit < unit2->peer_unit)
		{
			return -1;
		}
		if(unit1->peer_unit > unit2->peer_unit)
		{
			return 1;
		}
		return 0;
	}
	
	return didCmp;
}

#endif // if _PEER is defined //

//! @} ONE-NET_PEER_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_PEER
