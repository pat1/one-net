#ifndef _ONE_NET_CLIENT_NET_H
#define _ONE_NET_CLIENT_NET_H

#include "config_options.h"


//! \defgroup ONE-NET_CLIENT_NET ONE-NET CLIENT network functionality.
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
    \file one_net_client_net.h
    \brief ONE-NET CLIENT network functionality declarations.

    Handles sending to devices either directly or through the peer list for the
    CLIENT.  This is meant to only be viewed by one_net_client.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_client_port_const.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_CLIENT_NET_const
//! \ingroup ONE-NET_CLIENT_NET
//! @{

enum
{
    //! The number of peer devices in the peer device list
    ON_NUM_PEER_DEV = ONE_NET_PEER_PER_UNIT * ONE_NET_NUM_UNITS,
    
    //! Value that represents a message being sent directly to a device (not
    //! through the peer list).  This is also to be used if an error occurs and
    //! the transaction ID can't be returned from the client to the client net.
    SINGLE_DST_TXN_ID = 0xFF
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
    //! Index into peer_dev for the device the peer resides on
    UInt16 peer_dev_idx;

    //! The unit in the peer that this unit sends to.
    UInt8 peer_unit;
} on_peer_unit_t;


/*!
    \brief Keeps track of all the peers for every unit in this device
*/
typedef struct
{
    //! List of all the possible different peer devices this device can send to
    on_peer_dev_t dev[ON_NUM_PEER_DEV];

    //! The peer information for each unit.
    on_peer_unit_t unit[ONE_NET_NUM_UNITS][ONE_NET_PEER_PER_UNIT];
} on_peer_t;

//! @} ONE-NET_CLIENT_NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_CLIENT_NET_pub_var
//! \ingroup ONE-NET_CLIENT_NET
//! @{

//! @} ONE-NET_CLIENT_NET_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_CLIENT_NET_pub_func
//! \ingroup ONE-NET_CLIENT_NET
//! @{

one_net_status_t on_client_net_init(void * const peer_location,
  const UInt16 LEN);

// functions from one_net_client_net
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT,
      const BOOL MH);
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
      const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT);
#endif // else _ONE_NET_MULTI_HOP is not defined //

one_net_status_t on_client_net_unassign_peer(const UInt8 SRC_UNIT,
 const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT);

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

one_net_status_t on_client_net_single_txn_hdlr(const one_net_status_t STATUS,
  const on_encoded_did_t * const DST, UInt8 * data, const UInt8 TRIES,
  const UInt8 PRIORITY, const UInt8 TXN_ID);

//! @} ONE-NET_CLIENT_NET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_CLIENT_NET

#endif // _ONE_NET_CLIENT_NET_H //
