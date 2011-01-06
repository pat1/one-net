#ifndef _ONE_NET_PEER_H
#define _ONE_NET_PEER_H

//! \defgroup ONE-NET_PEER ONE-NET PEER functionality.
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
    \file one_net_peer.h
    \brief ONE-NET Peer functionality declarations.

    Handles everything dealing with peer assignments
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_client_port_const.h" // for ONE_NET_MAX_PEER_DEV
#include "one_net_eval_hal.h" // for NUM_MASTER_PEER

//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{


enum
{
    //! The number of peer devices in the peer device list
    ON_NUM_PEER_DEV = ONE_NET_MAX_PEER_DEV
};


extern const on_encoded_did_t INVALID_PEER;


//! @} ONE-NET_PEER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_PEER_typedefs
//! \ingroup ONE-NET_PEER
//! @{

/*!
    \brief Peer device.

    This structure holds information about the peer device that this device
    sends to.
*/
typedef struct
{
    on_encoded_did_t did;           //!< The device ID of the peer to send to.
    UInt8 nonce;                    //!< Nonce to use when sending to the peer.
    UInt8 data_rate;                //!< The data rate the peer listens at.

    #ifdef _ONE_NET_MULTI_HOP
        UInt8 max_hops;             //!< Max # hops when sending to this peer.

        BOOL mh_peer;               //!< TRUE if the peer is Multi-Hop capable.
    #endif // ifdef _ONE_NET_MULTI_HOP //
} on_peer_dev_t;


/*!
    \brief Represents a peer for a unit
*/
typedef struct
{
    // We are now storing the encoded did of the peer, not an index.
/*    //! Index into peer_dev for the device the peer resides on
    UInt16 peer_dev_idx;*/
    on_encoded_did_t peer_dev;

    //! The unit in this device that triggers a message to a peer.
    UInt8 src_unit;
	
    //! The unit in the peer that src_unit sends to.
    UInt8 peer_unit;
} on_peer_unit_t;


/*!
    \brief Keeps track of all the peers for every unit in this device
*/
typedef struct
{
    //! List of all the possible different peer devices this device can send to
    on_peer_dev_t dev[ONE_NET_MAX_PEER_DEV];

    //! List of all peer units
    on_peer_unit_t unit[ONE_NET_MAX_PEER_UNIT];
} on_peer_t;




/*!
    \brief Master peer assignments
    
    Keeps track of the peers that have been assigned to the MASTER user pin
    units.  If a location is not used, ONE_NET_DEV_UNIT is stored in dst_unit.
    This list should not contain any holes.
*/
typedef struct _master_peer_t
{
    on_encoded_did_t dst_did;
    UInt8 src_unit;
    UInt8 dst_unit;
} master_peer_t;


/*!
    \brief Manages messages sent to the peers of a given source unit.
*/
typedef struct
{
    //! The source unit for the message.  A value of ONE_NET_DEV_UNIT indicates that
    //! it is not in use.
    UInt8 src_unit;
    
    //! Where we currently are when iterating through the peer unit list.
    UInt8 current_idx;
    
    //! Index in the message that contains the destination did that needs to
    //! be changed when the message is sent to the next peer.  A value of
    //! ON_MAX_RAW_PLD_LEN indicates that the message is not to be changed.
    UInt8 msg_dst_unit_idx;
} peer_msg_mgr_t;


//! @} ONE-NET_PEER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_PEER_pub_var
//! \ingroup ONE-NET_PEER
//! @{
	
extern master_peer_t master_peer[NUM_MASTER_PEER];


//! The peer device to communicate with (if set up by the MASTER).  This needs
//! to be assigned a location in the init function (from a parameter).
extern on_peer_t * peer;

//! Manages messages sent to the peer connections that have been set up.
extern peer_msg_mgr_t peer_msg_mgr;/* Note: It's initialized to all zeros */


//! @} ONE-NET_PEER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PEER_pub_func
//! \ingroup ONE-NET_PEER
//! @{

UInt8 on_client_net_txn_nonce_for_peer(const on_encoded_did_t * const PEER_DID);
BOOL on_client_net_set_peer_txn_nonce(const on_encoded_did_t * const DID,
  const UInt8 NEXT_NONCE);
UInt8 on_client_net_data_rate_for_peer(const on_encoded_did_t * const PEER_DID);
BOOL on_client_net_set_peer_data_rate(
  const on_encoded_did_t * const PEER_DID, const UInt8 DATA_RATE);

#ifdef _ONE_NET_MULTI_HOP
    UInt8 * on_client_net_peer_hops_field(const on_encoded_did_t * const DID);
    UInt8 on_client_net_max_hops_for_peer(
      const on_encoded_did_t * const PEER_DID);
#endif // else _ONE_NET_MULTI_HOP is not defined //


#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
      const BOOL MH);
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT);
#endif // else _ONE_NET_MULTI_HOP is not defined //

one_net_status_t on_client_net_unassign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT, BOOL deviceIsMaster);

void on_client_net_clear_peer_msg_mgr(peer_msg_mgr_t *mgr);

UInt8 have_more_peers(peer_msg_mgr_t *mgr);

one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
  peer_msg_mgr_t *mgr, on_encoded_did_t *dst_did);

#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t master_assigned_peer(const UInt8 src_unit,
      const on_encoded_did_t * const peer_did, const UInt8 peer_unit,
      const BOOL MH);
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t master_assigned_peer(const UInt8 src_unit,
      const on_encoded_did_t * const peer_did, const UInt8 peer_unit);
#endif // else _ONE_NET_MULTI_HOP is not defined //

void init_master_peer(void);

one_net_status_t master_unassigned_peer(const on_encoded_did_t *peer_did,
  UInt8 peer_unit, UInt8 src_unit, BOOL deviceIsMaster);

#ifdef _ONE_NET_EVAL
    BOOL master_get_peer_assignment_to_save(UInt8 **ptr, UInt16 *len);
#endif


//! @} ONE-NET_PEER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_PEER

#endif // _ONE_NET_PEER_H //
