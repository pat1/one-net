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
#include "one_net_types.h"
#include "one_net_peer.h"
#include "one_net.h"
#include "one_net_port_specific.h"



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{

//! same as ON_ENCODED_BROADCAST_DID
on_encoded_did_t INVALID_PEER = {0xB4, 0xB4};


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

// see one_net_peer.h for public variable documentation 
on_peer_t* peer;
peer_msg_mgr_t peer_msg_mgr;


//! @} ONE-NET_PEER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================


//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PEER_pri_func
//! \ingroup ONE-NET_PEER
//! @{
	
static on_peer_unit_t * on_client_net_next_peer(peer_msg_mgr_t *mgr);

//! @} ONE-NET_PEER_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PEER_pub_func
//! \ingroup ONE-NET_PEER
//! @{



/*!
    \brief Checks whether there are more peers to send to.
    
    The next peer for the source unit will be obtained.
    
    \param[in/out] mgr The peer manager for the message
    
    \return 1/TRUE if there are more peers
            0/FALSE if there are no more peers
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
    
    mgr->src_unit = ONE_NET_DEV_UNIT;
    mgr->current_idx  = 0;
} // on_client_net_clear_peer_msg_mgr //


/*!
    \brief Returns the index into the peer device list for the addressed peer.

    \param[in] DID The device id of the peer to look up.

    \return The index into the peer device list for DID if DID is in the list.
            -1 if the DID is not in the list
*/
int on_client_net_peer_dev_idx(const on_encoded_did_t *DID)
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
    \brief Retrieves the next peer connection for the manager.
    
    \param[in/out] mgr The manager to get the next peer for.  This will be
      updated so that it is ready to be passed in the next time.  If there are
      no more peers, mgr will NOT be updated.

    \return The next peer to send to.  0 will be returned if there are no more
      peers left, or the parameter is invalid.
*/
static on_peer_unit_t * on_client_net_next_peer(peer_msg_mgr_t *mgr)
{
    UInt8 index;
    if(!mgr)
    {
        return 0;
    } // if the parameter is invalid //
	
	index = (mgr->current_idx)++;

    // check the peer count
    while(mgr->current_idx < ONE_NET_MAX_PEER_UNIT)
    {
        if(peer->unit[index].src_unit == mgr->src_unit)
        {
            return &(peer->unit[index]);
        } // if the peer exists
		
        index = (mgr->current_idx)++;
    }

    return 0;
} // on_client_net_next_peer //


/*!
    \brief Sets up the message to be sent to the next peer.
    
    The next peer for the source unit will be abtained.  The message will be
    updated with the destination unit of the peer (if needed), and the encoded
    did of the peer will be returned.
    
    \param[in/out] The message being sent.
    \param[in/out] The peer manager for the message
    \param[out] dst_did The encoded destination did for the message.
    
    \return ONS_SUCCESS If the operation was successful
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_END If there were no more peers to send the message to
*/
one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
  peer_msg_mgr_t *mgr, on_encoded_did_t *dst_did)
{
    on_peer_unit_t * peer_unit;

    if(!data || !mgr || !dst_did)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    // find the next peer
    if(!(peer_unit = on_client_net_next_peer(mgr)))
    {
        return ONS_END;
    } // if there are no more peers to send to //
    
    // copy the destination did
    one_net_memmove(*dst_did, peer_unit->peer_dev,
      sizeof(on_encoded_did_t));
    
    // adjust the message if need be
    if(mgr->msg_dst_unit_idx != ON_MAX_RAW_PLD_LEN)
    {
        put_dst_unit(peer_unit->peer_unit, data);
    } // if the message needs to be updated //
    
    return ONS_SUCCESS;
} // on_client_net_setup_msg_for_peer //


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
void on_client_net_rm_dev(const on_encoded_did_t* const did)
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

//! @} ONE-NET_PEER
