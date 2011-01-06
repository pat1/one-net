//! \addtogroup ONE-NET_CLIENT_NET ONE-NET CLIENT network functionality
//! \ingroup ONE-NET_CLIENT
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
    \file one_net_client_net.c
    \brief ONE-NET CLIENT network functionality declarations.

    Handles sending to devices either directly or through the peer list for the
    CLIENT.  This module is a companion to one_net_client and must be
    initialized by one_net_client.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_client_net.h"

#include "one_net_client_port_specific.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_NET_const
//! \ingroup ONE-NET_CLIENT_NET
//! @{

enum
{
    //! Number of messages that can be sent simultanously
    MAX_SINGLE_TXN = 1,
};


// same as ON_ENCODED_BROADCAST_DID
const on_encoded_did_t INVALID_PEER = {0xB4, 0xB4};

//! @} ONE-NET_CLIENT_NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_CLIENT_NET_typedefs
//! \ingroup ONE-NET_CLIENT_NET
//! @{



//! @} ONE-NET_CLIENT_NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_CLIENT_NET_pri_var
//! \ingroup ONE-NET_CLIENT_NET
//! @{


#if 0
void clear_global_peer_msg_mgr(on_txn_t **txn)
{
    peer_msg_mgr.src_unit = ONE_NET_DEV_UNIT;
    peer_msg_mgr.peer_idx = 0;
    (*txn)->priority = ONE_NET_NO_PRIORITY;

}
#endif


//! @} ONE-NET_CLIENT_NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_NET_pri_func
//! \ingroup ONE-NET_CLIENT_NET
//! @{

// from one_net_client
one_net_status_t on_client_send_single(const UInt8 *DATA,
  UInt8 DATA_LEN, UInt8 PRIORITY,
  const on_encoded_did_t *DST, UInt8 TXN_ID);
void on_client_encoded_master_did(on_encoded_did_t * const master_did);
BOOL on_client_send_to_master(void);

static UInt8 have_more_peers(peer_msg_mgr_t *mgr);
static one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
  peer_msg_mgr_t *mgr, on_encoded_did_t *dst_did);

static on_peer_unit_t * on_client_net_next_peer(peer_msg_mgr_t *mgr);
static void on_client_net_clear_peer_msg_mgr(peer_msg_mgr_t *mgr);


//! @} ONE-NET_CLIENT_NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_NET_pub_func
//! \ingroup ONE-NET_CLIENT_NET
//! @{

/*!
    \brief Sends a single data message.
    
    The message is either set to the peer list, or the specific device that
    is passed in.
    
    \param[in/out] data The data to send.
    \param[in] DATA_LEN The length of DATA (in bytes).
    \param[in] DST_UNIT_IDX The index into the data that contains the
      destination unit field.  This value should be set to something greater
      than or equal to the DATA_LEN if DATA is not to be changed when sending
      to peers.
    \param[in] PRIORITY The priority of the transaction.
    \param[in] RAW_DST The device the message is destined for.  This should be
      NULL if the message is to be sent to the peer list.
    \param[in] SRC_UNIT The unit that the message originated from.
    
    \return ONS_SUCCESS If the single data has been queued successfully.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_RSRC_FULL If no resources are currently available to handle the
              request.
            See on_client_send_single for more possible return codes.
*/

one_net_status_t one_net_client_send_single(UInt8 *data,
  UInt8 DATA_LEN, UInt8 DST_UNIT_IDX, UInt8 PRIORITY,
  const one_net_raw_did_t *RAW_DST, UInt8 SRC_UNIT)

{
    one_net_status_t status;
    on_encoded_did_t encoded_dst_did;
    
    UInt8 txn_id = SINGLE_DST_TXN_ID;

    if(!data || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN
      || PRIORITY < ONE_NET_LOW_PRIORITY || PRIORITY > ONE_NET_HIGH_PRIORITY
      || (SRC_UNIT != ONE_NET_DEV_UNIT && SRC_UNIT >= ONE_NET_NUM_UNITS)
      || (SRC_UNIT == ONE_NET_DEV_UNIT && !RAW_DST))
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //
    
    // check to see if sending to the peer list
    if(!RAW_DST)
    {
        // make sure the peer list resource is available
        if(peer_msg_mgr.src_unit != ONE_NET_DEV_UNIT)
        {
            return ONS_RSRC_FULL;
        } // if the resource is not available //

        peer_msg_mgr.src_unit = SRC_UNIT;
        peer_msg_mgr.current_idx = 0;
        peer_msg_mgr.msg_dst_unit_idx = DST_UNIT_IDX < DATA_LEN ? DST_UNIT_IDX
          : ON_MAX_RAW_PLD_LEN;

        // get the peer unit to place in the message
        status = on_client_net_setup_msg_for_peer(data, &peer_msg_mgr,
          &encoded_dst_did);
        switch(status)
        {
            case ONS_SUCCESS:
            {
                txn_id = 0;
                break;
            } // success case //
            
            case ONS_END:
            {
                on_client_encoded_master_did(&encoded_dst_did);
                if(peer_msg_mgr.msg_dst_unit_idx != ON_MAX_RAW_PLD_LEN)
                {
                    put_dst_unit(ONE_NET_DEV_UNIT, data);
                } // if the message needs to be updated //

                // since it's going to the MASTER, the message manager is no
                // longer needed
                on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
                break;
            } // end case //

            default:
            {
                on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
                return status;
            } // default case //
        } // switch(on_client_net_setup_msg_for_peer) //
    } // if sending to the peer list //
    else if((status = on_encode(encoded_dst_did, *RAW_DST,
      sizeof(encoded_dst_did))) != ONS_SUCCESS)
    {
        return status;
    } // if encoding the did failed //

    // try sending
    status = on_client_send_single(data, DATA_LEN, PRIORITY,
                      &encoded_dst_did, txn_id);
    if((status != ONS_SUCCESS) && !RAW_DST)
    {
        // sending to peer list failed, so free list if there are no more
        if (!have_more_peers(&peer_msg_mgr))
        {
            on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
        }
    } // if queueing the transaction failed //
    
    return status;
} // one_net_client_send_single //














/*!
    \brief Handles the end of a single data transaction.

    Alerts the app of the status of sending to the specific device.  Will
    forward the message on to any other devices that are supposed to get
    messages sent to the peer list.

    \param[in] STATUS The result of the transaction.
    \param[in] DST The device the transaction was sent to.
    \param[in] data The data that was sent to the device.  This may be updated
      if the message is supposed to be sent on to another device.
    \param[in] TRIES The number of times the packet was tried before STATUS was
      returned.
    \param[in] PRIORITY The priority of the transaction
    \param[in] TXN_ID The transaction id that was passed to
      on_client_send_single.

    \return ONS_SUCCESS If handling the transaction was successful
            ONS_BAD_PARAM if any of the parameters are invalid.
*/
one_net_status_t on_client_net_single_txn_hdlr(const one_net_status_t STATUS,
  const on_encoded_did_t * const DST, UInt8 * data, const UInt8 TRIES,
  const UInt8 PRIORITY, const UInt8 TXN_ID)
{
    one_net_status_t status;
    on_encoded_did_t encoded_dst_did;
    one_net_raw_did_t raw_did;
    
    UInt8 forward_txn_id = SINGLE_DST_TXN_ID;


    if(!DST || !data || (TXN_ID >= MAX_SINGLE_TXN
      && TXN_ID != SINGLE_DST_TXN_ID))
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(!DST)
    {
        one_net_client_single_txn_status(STATUS, TRIES, data, 0);
       return ONS_SUCCESS;
    } // if the destination is not valid //

    if((status = on_decode(raw_did, *DST, ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
    {
        one_net_client_single_txn_status(ONS_INTERNAL_ERR, 0, data, 0);
        return status;
    } // if decoding the CLIENT did was not successful //

    one_net_client_single_txn_status(STATUS, TRIES,
      data, &raw_did);

    if(TXN_ID < MAX_SINGLE_TXN)
    {
        BOOL forward = FALSE;

        // get the peer unit to place in the message
        switch((status = on_client_net_setup_msg_for_peer(data, &peer_msg_mgr,
          &encoded_dst_did)))
        {
            case ONS_SUCCESS:
            {
                forward = TRUE;
                forward_txn_id = TXN_ID;
                break;
            } // success case //
            
            case ONS_END:
            {
                if(on_client_send_to_master())
                {
                    forward = TRUE;
                    on_client_encoded_master_did(&encoded_dst_did);
                    if(peer_msg_mgr.msg_dst_unit_idx != ON_MAX_RAW_PLD_LEN)
                    {
                        put_dst_unit(ONE_NET_DEV_UNIT, data);
                    } // if the message needs to be updated //
                } // if the message should be sent on to the MASTER //

                // Even if the message is going to the MASTER, the message
                // manager is no longer needed
                on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
                break;
            } // end case //

            default:
            {
                on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
                return status;
            } // default case //
        } // switch(on_client_net_setup_msg_for_peer) //

        if(forward)
        {
            UInt8 priority = PRIORITY;
            //
            // dje: Note that if message to previous device in the 
            // peer list failed, this function was called witn PRIORITY
            // set to ONE_NET_NO_PRIORITY.
            // In order to continue to the next peer, we can't have that
            // so send_single with high priority
            //
            if (PRIORITY == ONE_NET_NO_PRIORITY) {
                priority = ONE_NET_SEND_SINGLE_PRIORITY;
            }
            if((status = on_client_send_single(data,
              ONE_NET_RAW_SINGLE_DATA_LEN, 
              priority,// djeThis is the biggie: 
              // on_client_send_single shouldn't see ONE_NET_NO_PRIORITY
              &encoded_dst_did, forward_txn_id))
              != ONS_SUCCESS)
            {
                // sending to peer list failed, so free list
                //dje: Don't clear it unless there are no more
                //peers on the list!
                one_net_client_single_txn_status(status, 0, data, &raw_did);
                if (have_more_peers(&peer_msg_mgr))
                {
                    //dje: This is not be the end of the transaction,
                    //since there are more peers to be serviced,
                    //therefore, let it continue
                    status = ONS_SUCCESS;
                }

                else { // this is really the end; clean up a little
                    on_client_net_clear_peer_msg_mgr(&peer_msg_mgr);
                    one_net_client_single_txn_status(ONS_SINGLE_END, 0, 
                                                 data, &raw_did);
                }
            } // if queueing the transaction failed //

            return status;
        } // if the message should be sent to another device //
    } // if sending to the peer list //
    
    // Not sending the message on, so send a single complete status to the app
    one_net_client_single_txn_status(ONS_SINGLE_END, 0, data, &raw_did);

    return ONS_SUCCESS;
} // on_client_net_single_txn_hdlr //

//! @} ONE-NET_CLIENT_NET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_CLIENT_NET_pri_func
//! \ingroup ONE-NET_CLIENT_NET
//! @{

/*!
    \brief Initializes the CLIENT net layer.
    
    This should be called by the CLIENT when the CLIENT's init function is
    called.
    
    \param[in] peer_location  The location that the peer data has been assigned
    \param[in] LEN The number of bytes allocated to keep track of peer
      connections
    
    \return ONS_SUCCESS if the CLIENT net layer was successfully initialized.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_INVALID_DATA If the space allocated to hold peer connections
*/
one_net_status_t on_client_net_init(void * const peer_location,
  const UInt16 LEN)
{
    UInt8 i;
    
    if(!peer_location)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(LEN < sizeof(on_peer_t))
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //
    
    peer = peer_location;

    for(i = 0; i < ONE_NET_MAX_PEER_DEV; i++)
    {
        one_net_memmove(peer->dev[i].did, ON_ENCODED_BROADCAST_DID,
          sizeof(peer->dev[i].did));
    } // 1 of 2 loops to initialize on_peer_t //
    
    for(i = 0; i < ONE_NET_MAX_PEER_UNIT; i++)
    {
        one_net_memmove(peer->unit[i].peer_dev, ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
        peer->unit[i].src_unit = ONE_NET_DEV_UNIT;
        peer->unit[i].peer_unit = ONE_NET_DEV_UNIT;
    } // loop to initialize peer_t //

    peer_msg_mgr.src_unit = ONE_NET_DEV_UNIT;
    peer_msg_mgr.current_idx  = 0;
    
    return ONS_SUCCESS;
} // on_client_net_init //


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
static one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
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


static UInt8 have_more_peers(peer_msg_mgr_t *mgr)
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
    \brief Clears a peer message manager and makes it available
    
    \param[in/out] The peer message manager to clear.
    
    \return void
*/
static void on_client_net_clear_peer_msg_mgr(peer_msg_mgr_t *mgr)
{
    if(!mgr)
    {
        return;
    } // if the parameter is invalid //
    
    mgr->src_unit = ONE_NET_DEV_UNIT;
    mgr->current_idx  = 0;
} // on_client_net_clear_peer_msg_mgr //



//! @} ONE-NET_CLIENT_NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT_NET
