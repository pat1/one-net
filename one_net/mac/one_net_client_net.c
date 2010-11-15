//! \addtogroup ONE-NET_CLIENT_NET ONE-NET CLIENT network functionality
//! \ingroup ONE-NET_CLIENT
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    
    //! Invalid peer index
    ON_INVALID_PEER = 0xFFFF
};

//! @} ONE-NET_CLIENT_NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_CLIENT_NET_typedefs
//! \ingroup ONE-NET_CLIENT_NET
//! @{

/*!
    \brief Manages messages sent to the peers of a given source unit.
*/
typedef struct
{
    //! The source unit for the message.  A value of DEV_unit indicates that
    //! it is not in use.
    UInt8 src_unit;
    
    //! Index into the peer list for the given source unit to determine the
    //! destination peer that is to receive the message
    UInt8 peer_idx;
    
    //! Index in the message that contains the destination did that needs to
    //! be changed when the message is sent to the next peer.  A value of
    //! ON_MAX_RAW_PLD_LEN indicates that the message is not to be changed.
    UInt8 msg_dst_unit_idx;
} peer_msg_mgr_t;

//! @} ONE-NET_CLIENT_NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_CLIENT_NET_pri_var
//! \ingroup ONE-NET_CLIENT_NET
//! @{

//! The peer device to communicate with (if set up by the MASTER).  This needs
//! to be assigned a location in the init function (from a parameter).
static on_peer_t * peer;

//! Manages messages sent to the peer connections that have been set up.
static peer_msg_mgr_t peer_msg_mgr;/* Note: It's initialized to all zeros */

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

static UInt16 on_client_net_peer_dev_idx(const on_encoded_did_t *DID);
static BOOL on_client_net_send_to_peer_dev(UInt16 DEV_IDX);
static void on_client_net_rm_dev(UInt16 DEV_IDX);

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
        peer_msg_mgr.peer_idx = 0;
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
    \brief Saves the peer assignment that is being made.
    
    \param[in] SRC_UNIT The unit on this device being assigned the peer.
    \param[in] PEER_DID The peer device being assigned to this device.
    \param[in] PEER_UNIT The unit in the peer device being assigned to this
      device.
    \param[in] MH Only present if _ONE_NET_MULTI_HOP has been defined.  TRUE
      is the peer device is capable of handling multi-hop transactions.
      
    \return ONS_SUCCESS If the peer was successfully assigned
            ONS_BAD_PARAM If the parameters are invalid
            ONS_INVALID_DATA If the data is incorrect (such as a source unit
              that is out of range).
            ONS_INTERNAL_ERR If something unexpected happened
            ONS_RSRC_FULL If no more peers to the SRC_UNIT can be assigned
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
      const BOOL MH)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT)
#endif // else _ONE_NET_MULTI_HOP is not defined //
{
    UInt8 unit_idx;

    if(!PEER_DID)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(SRC_UNIT > ONE_NET_NUM_UNITS)
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //
    
    // find a spot to add the peer.  There are no holes in the list, so finding
    // an empty slot indicates the peer is not yet in the list
    for(unit_idx = 0; unit_idx < ONE_NET_PEER_PER_UNIT; unit_idx++)
    {
        // check if the slot is free
        if(peer->unit[SRC_UNIT][unit_idx].peer_unit == ONE_NET_DEV_UNIT)
        {
            UInt8 dev_idx;

            peer->unit[SRC_UNIT][unit_idx].peer_unit = PEER_UNIT;
            
            // A slot for the peer unit has been found, and set, so now find a
            // free slot for the peer device (which we are guaranteed since a
            // free slot for the unit was found).
            for(dev_idx = 0; dev_idx < ON_NUM_PEER_DEV; dev_idx++)
            {
                if(on_encoded_did_equal(&ON_ENCODED_BROADCAST_DID,
                  (const on_encoded_did_t * const)&(peer->dev[dev_idx].did)))
                {
                    one_net_memmove(peer->dev[dev_idx].did, *PEER_DID,
                      sizeof(peer->dev[dev_idx].did));
                      
                    #ifdef _ONE_NET_MULTI_HOP
                        peer->dev[dev_idx].max_hops = 0;
                        peer->dev[dev_idx].mh_peer = MH;
                    #endif // ifdef _ONE_NET_MULTI_HOP //
                    peer->unit[SRC_UNIT][unit_idx].peer_dev_idx = dev_idx;
                    return ONS_SUCCESS;
                } // if a device slot is free //
                else if(on_encoded_did_equal(PEER_DID,
                  (const on_encoded_did_t * const)&(peer->dev[dev_idx].did)))
                {
                    peer->unit[SRC_UNIT][unit_idx].peer_dev_idx = dev_idx;
                    return ONS_SUCCESS;
                } // else if the device already exists //
            } // loop through all peer devices //

            peer->unit[SRC_UNIT][unit_idx].peer_unit = ONE_NET_DEV_UNIT;
            return ONS_INTERNAL_ERR;
        } // if a unit slot is free //
        else if(peer->unit[SRC_UNIT][unit_idx].peer_unit == PEER_UNIT
          && on_encoded_did_equal(PEER_DID,
          (const on_encoded_did_t * const)
          &(peer->dev[peer->unit[SRC_UNIT][unit_idx].peer_dev_idx].did)))
        {
            return ONS_SUCCESS;
        } // else if the binding already exists //
    } // loop to find a spot to add the peer unit //

    return ONS_RSRC_FULL;
} // on_client_net_assign_peer //


/*!
    \brief Unassigns a peer connection
    
    If SRC_UNIT and PEER_UNIT both equal ONE_NET_DEV_UNIT, then all peer
    connections that send to PEER_DID are removed.

    \param[in] SRC_UNIT The source unit of the peer connection to remove.
    \param[in] PEER_DID The device ID of the peer whose connection is being
      removed.
    \param[in] PEER_UNIT The peer unit of the connection being removed
    
    \return ONS_SUCCESS If the device was successfully removed (or the
              connection did not exist).
            ONS_BAD_PARAM If any of the parameters are invalid
            ONS_INVALID_DATA If the peer source unit does not exist.
*/
one_net_status_t on_client_net_unassign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT)
{
    UInt16 dev_idx;
    UInt8 peer_unit_idx;
    UInt8 src_unit = SRC_UNIT;
    BOOL rm_device = FALSE;

    if((dev_idx = on_client_net_peer_dev_idx((const on_encoded_did_t * const)
      PEER_DID)) == ON_INVALID_PEER)
    {
        // the connection doesn't exist, so consider the removal successful
        return ONS_SUCCESS;
    } // if the device does not exist //

    if(src_unit == ONE_NET_DEV_UNIT && PEER_UNIT == ONE_NET_DEV_UNIT)
    {
        // Remove all peer connections to the given device.
        rm_device = TRUE;
        src_unit = 0;               // start from the first source peer
    } // if both units are the ONE_NET_DEV_UNIT //
    else if(src_unit > ONE_NET_NUM_UNITS || PEER_UNIT == ONE_NET_DEV_UNIT)
    {
        return ONS_INVALID_DATA;
    } // else if the received data is not valid //

    do
    {
        peer_unit_idx = 0;
        while(peer_unit_idx < ONE_NET_PEER_PER_UNIT)
        {
            if(peer->unit[src_unit][peer_unit_idx].peer_dev_idx
              == dev_idx && (rm_device
              || peer->unit[src_unit][peer_unit_idx].peer_unit
              == PEER_UNIT))
            {
                // Move the items down in the list so that there are no holes
                if(peer_unit_idx < ONE_NET_PEER_PER_UNIT - 1)
                {
                    one_net_memmove(&(peer->unit[src_unit][peer_unit_idx]),
                      &(peer->unit[src_unit][peer_unit_idx + 1]),
                      (ONE_NET_PEER_PER_UNIT - peer_unit_idx - 1)
                      * sizeof(on_peer_unit_t));
                } // if not the last unit //

                peer->unit[src_unit][ONE_NET_PEER_PER_UNIT - 1].peer_dev_idx
                  = ON_INVALID_PEER;
                peer->unit[src_unit][ONE_NET_PEER_PER_UNIT - 1].peer_unit
                  = ONE_NET_DEV_UNIT;
                
                if(!rm_device)
                {
                    // Not removing all the devices, so don't need
                    // to search anymore peer assignments.
                    break;
                } // if not removing the entire device //
                
                // don't adjust the peer_unit_idx since we shifted everything
                // down by 1 (so if we started in slot N, then N + 1 is now in
                // N, so check slot N again since it's really a new record
                continue;
            } // if a match was found //
            
            peer_unit_idx++;
        } // loop through the units peers //
    } while(rm_device && ++src_unit < ONE_NET_NUM_UNITS);

    if(rm_device || !on_client_net_send_to_peer_dev(dev_idx))
    {
        on_client_net_rm_dev(dev_idx);
    } // if no one else sends to that device //

    return ONS_SUCCESS;
} // on_client_net_unassign_peer //


/*!
    \brief Returns the transaction nonce for the given peer.
    
    \param[in] PEER_DID The peer to retrieve the transaction nonce for.
    
    \return The transaction nonce to use when sending to the peer.
      ON_INVALID_NONCE will be returned if the peer does not exist.
*/
UInt8 on_client_net_txn_nonce_for_peer(const on_encoded_did_t * const PEER_DID)
{
    UInt16 peer_dev_idx = ON_INVALID_PEER;

    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != ON_INVALID_PEER)
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
    UInt16 dev_idx = ON_INVALID_PEER;

    if(DID && NEXT_NONCE <= ON_MAX_NONCE
      && (dev_idx = on_client_net_peer_dev_idx(DID)) != ON_INVALID_PEER)
    {
        peer->dev[dev_idx].nonce = NEXT_NONCE;
        return TRUE;
    } // if the parameters are good, and the peer was found //
    
    return FALSE;
} // on_client_net_set_peer_txn_nonce //


/*!
    \brief Returns the data rate to use when sending to the given peer.
    
    \param[in] PEER_DID The peer to retrieve the data rate for.
    
    \return The data rate to use when sending to the peer.
      ONE_NET_DATA_RATE_LIMIT will be returned if the peer does not exist.
*/
UInt8 on_client_net_data_rate_for_peer(const on_encoded_did_t * const PEER_DID)
{
    UInt16 peer_dev_idx = ON_INVALID_PEER;

    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != ON_INVALID_PEER)
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
    UInt16 peer_dev_idx = ON_INVALID_PEER;
    
    if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
      != ON_INVALID_PEER)
    {
        peer->dev[peer_dev_idx].data_rate = DATA_RATE;
        return TRUE;
    } // if peer_dev //
    
    return FALSE;
} // on_client_net_set_peer_data_rate //


#ifdef _ONE_NET_MULTI_HOP
    /*!
        \brief Returns a pointer to the hops field for the given peer.
    
        The hops field is the max number of hops to allow a packet to take when
        sent to the peer.
    
        \param[in] DID The peer whose hops field is to be retrieved.
    
        \return A pointer to the hops field for the given peer.  If the peer
          could not be found, 0 is returned.
    */
    UInt8 * on_client_net_peer_hops_field(const on_encoded_did_t * const DID)
    {
        UInt16 dev_idx = ON_INVALID_PEER;

        if(DID && (dev_idx = on_client_net_peer_dev_idx(DID)) != ON_INVALID_PEER
          && peer->dev[dev_idx].mh_peer)
        {
            return &(peer->dev[dev_idx].max_hops);
        } // if the parameters are good, and the peer was found //
    
        return 0;
    } // on_client_net_peer_hops_field //


    /*!
        \brief Returns the max_hops to use when sending to the given peer.
    
        \param[in] PEER_DID The peer to retrieve the max hops for.
    
        \return The max hops to use when sending to the peer.
          ON_INVALID_HOPS will be returned if the peer does not exist.
    */
    UInt8 on_client_net_max_hops_for_peer(
      const on_encoded_did_t * const PEER_DID)
    {
        UInt16 peer_dev_idx = ON_INVALID_PEER;

        if(PEER_DID && (peer_dev_idx = on_client_net_peer_dev_idx(PEER_DID))
          != ON_INVALID_PEER)
        {
            return peer->dev[peer_dev_idx].max_hops;
        } // if peer_dev //

        return ON_INVALID_HOPS;
    } // on_client_net_max_hops_for_peer //
#endif // ifdef _ONE_NET_MULTI_HOP //


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
                //    delay_ms(100);
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
    UInt8 i, j;
    
    if(!peer_location)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    if(LEN < sizeof(on_peer_t))
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //
    
    peer = peer_location;

    for(i = 0; i < ONE_NET_PEER_PER_UNIT * ONE_NET_NUM_UNITS; i++)
    {
        one_net_memmove(peer->dev[i].did, ON_ENCODED_BROADCAST_DID,
          sizeof(peer->dev[i].did));
    } // 1 of 2 loops to initialize on_peer_t //
    
    for(i = 0; i < ONE_NET_NUM_UNITS; i++)
    {
        for(j = 0; j < ONE_NET_PEER_PER_UNIT; j++)
        {
            peer->unit[i][j].peer_dev_idx = ON_INVALID_PEER;
            peer->unit[i][j].peer_unit = ONE_NET_DEV_UNIT;
        } // loop to initialize on_peer_unit_t in on_peer_t //
    } // 2 of 2 loops to initialize peer_t //

    peer_msg_mgr.src_unit = ONE_NET_DEV_UNIT;
    peer_msg_mgr.peer_idx  = 0;
    
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
    one_net_memmove(*dst_did, peer->dev[peer_unit->peer_dev_idx].did,
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
    UInt8 retval = 1;
    if(!mgr)
    {
        retval = 0;
    } // if the parameter is invalid //
    if ((mgr->peer_idx >= ONE_NET_PEER_PER_UNIT) ||
        (peer->unit[mgr->src_unit][mgr->peer_idx].peer_unit == ONE_NET_DEV_UNIT))
    {
        retval = 0;
    }
    return retval;
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
    if(!mgr)
    {
        return 0;
    } // if the parameter is invalid //

    // check the peer count
    if(mgr->peer_idx < ONE_NET_PEER_PER_UNIT)
    {
        UInt8 peer_idx = mgr->peer_idx++;
        // check the peer
        if(peer->unit[mgr->src_unit][peer_idx].peer_unit != ONE_NET_DEV_UNIT)
        {

            return &(peer->unit[mgr->src_unit][peer_idx]);
        } // if the peer exists
    } // if the list has not been completed //

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
    mgr->peer_idx  = 0;
} // on_client_net_clear_peer_msg_mgr //


/*!
    \brief Returns the index into the peer device list for the addressed peer.

    \param[in] DID The device id of the peer to look up.

    \return The index into the peer device list for DID if DID is in the list.
            ON_INVALID_PEER if the DID is not in the list
*/
static UInt16 on_client_net_peer_dev_idx(const on_encoded_did_t *DID)
{
    UInt16 i;

    if(!DID)
    {
        return ON_INVALID_PEER;
    } // if parameter is invalid //

    for(i = 0; i < ON_NUM_PEER_DEV; i++)
    {
        if(on_encoded_did_equal(DID, (const on_encoded_did_t * const)
          &(peer->dev[i].did)))
        {
            return i;
        } // if this device is responding to its peer //
    } // loop through all peer devices //

    return ON_INVALID_PEER;
} // on_client_net_peer_dev_idx //


/*!
    \brief Returns if any units send to a device.

    \param[in] DEV_IDX Index into the device list of the device to see if any
      units send to that device.

    \return TRUE If at least one unit sends to the device indexed by DEV_IDX
            FALSE if no device sends to the device indexed by DEV_IDX
*/
static BOOL on_client_net_send_to_peer_dev(UInt16 DEV_IDX)
{
    UInt8 unit_idx, peer_idx;

    if(DEV_IDX >= ON_NUM_PEER_DEV)
    {
        // make sure the caller doesn't remove the device that is indexed beyond
        // the end of the list.
        return TRUE;
    } // if the parameter is invalid //

    for(unit_idx = 0; unit_idx < ONE_NET_NUM_UNITS; unit_idx++)
    {
        for(peer_idx = 0; peer_idx < ONE_NET_PEER_PER_UNIT; peer_idx++)
        {
            if(peer->unit[unit_idx][peer_idx].peer_dev_idx == DEV_IDX)
            {
                return TRUE;
            } // if a unit does send to the peer device //
        } // loop through each peer for the unit //
    } // loop through the units //

    return FALSE;
} // on_client_net_send_to_peer_dev //


/*!
    \brief Removes a device from the list of peer devices that this device sends
      to.

    This also updates the peers to use the correct index since the device
    indexes change to ensure there are no holes in the device list.

    \param[in] DEV_IDX The index into the peer->dev list of the device that is
      being removed.

    \return void
*/
static void on_client_net_rm_dev(UInt16 DEV_IDX)
{
    UInt8 unit_idx, peer_idx;

    if(DEV_IDX >= ON_NUM_PEER_DEV)
    {
        // make sure the caller doesn't remove the device that is indexed beyond
        // the end of the list.
        return;
    } // if the parameter is invalid //

    // Move the items down in the list so that there are no holes
    if(DEV_IDX < ON_NUM_PEER_DEV - 1)
    {
        one_net_memmove(&(peer->dev[DEV_IDX]), &(peer->dev[DEV_IDX + 1]),
          (ON_NUM_PEER_DEV - (DEV_IDX + 1)) * sizeof(on_peer_dev_t));
    } // if the devices need to be shifted down //

    one_net_memmove(peer->dev[ON_NUM_PEER_DEV - 1].did,
      ON_ENCODED_BROADCAST_DID, sizeof(on_encoded_did_t));

    // Since the dev indexes got shifted down, need to adjust the peer units
    // device index.
    for(unit_idx = 0; unit_idx < ONE_NET_NUM_UNITS; unit_idx++)
    {
        for(peer_idx = 0; peer_idx < ONE_NET_PEER_PER_UNIT &&
          peer->unit[unit_idx][peer_idx].peer_unit != ONE_NET_DEV_UNIT;
          peer_idx++)
        {
            if(peer->unit[unit_idx][peer_idx].peer_dev_idx > DEV_IDX)
            {
                peer->unit[unit_idx][peer_idx].peer_dev_idx--;
            } // if a unit does send to the peer device //
        } // loop through each peer for the unit //
    } // loop through the units //
} // on_client_net_rm_dev //

//! @} ONE-NET_CLIENT_NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_CLIENT_NET
