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

#ifdef _PEER

#include "one_net_types.h"

#ifdef _ONE_NET_CLIENT
#include "one_net_client_port_const.h" // for ONE_NET_MAX_PEER_DEV
#endif

// TODO - require NUM_MASTER_PEER to be defined only in
//        one_net_master_port_const.h
#ifdef _ONE_NET_MASTER
#include "one_net_master_port_const.h" // for NUM_MASTER_PEER
#endif

//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{


#ifdef _ONE_NET_CLIENT
enum
{
    //! The number of peer devices in the peer device list
    ON_NUM_PEER_DEV = ONE_NET_MAX_PEER_DEV
};
#endif


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
    #ifdef _DATA_RATE
    UInt8 data_rate;                //!< The data rate the peer listens at.
    #endif
} on_peer_dev_t;


/*!
    \brief Peer Consants
*/
typedef enum
{
    PEER_SENT_LEN = 1,
    PEER_SENT_IDX = 0,
    PEER_SENT_SHIFT = 7,
    PEER_SENT_MASK = 0x80,
    PEER_DELIVERED_LEN = 1,
    PEER_DELIVERED_IDX = PEER_SENT_IDX + PEER_SENT_LEN,
    PEER_DELIVERED_SHIFT = 6,
    PEER_DELIVERED_MASK = 0x40,
    PEER_FAIL_LEN = 1,
    PEER_FAIL_IDX = PEER_DELIVERED_IDX + PEER_DELIVERED_LEN,
    PEER_FAIL_SHIFT = 5,
    PEER_FAIL_MASK = 0x20,
    PEER_SENT_DELIVERED_FAIL_MASK = PEER_SENT_MASK | PEER_DELIVERED_MASK | PEER_FAIL_MASK,
    PEER_PRIORITY_LEN = 2,
    PEER_PRIORITY_IDX = PEER_FAIL_IDX + PEER_FAIL_LEN,
    PEER_PRIORITY_SHIFT = 3,
    PEER_PRIORITY_MASK = 0x18,
    #if 0
    PEER_RETRY_LEN = 3,
    PEER_RETRY_IDX = PEER_PRIORITY_IDX + PEER_PRIORITY_LEN,
    PEER_RETRY_SHIFT = 0,
    PEER_RETRY_MASK = 0x07,
    PEER_NUM_RETRY = 3
    #endif
} peer_flag_constants_t;


typedef enum
{
    PEER_NO_PRIORITY,
    PEER_LOW_PRIORITY,
    PEER_HIGH_PRIORITY
} peer_priority_t;


typedef enum
{
    PEER_SEND,
    PEER_DELIVERED,
    PEER_FAIL,
    PEER_PRIORITY,
    #if 0
    PEER_RETRY
    #endif
};


typedef struct
{
    BOOL sent;
    BOOL delivered;
    BOOL fail;
    UInt8 priority;
    #if 0
    UInt8 retry;
    #endif
} peer_flag_t;



/*!
    \brief Represents a peer for a unit
*/
typedef struct
{
    //! did of the of the peer
    on_encoded_did_t peer_did;

    //! The unit in this device that triggers a message to a peer.
    UInt8 src_unit;
	
    //! The unit in the peer that src_unit sends to.
    UInt8 peer_unit;
    
    //! Flags that are set by the peer message manager.
    UInt8 flags;
} on_peer_unit_t;


#ifdef _ONE_NET_CLIENT
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


enum
{
    CLIENT_PEER_SIZE_BYTES = sizeof(on_peer_t)
};
#endif


#ifdef _ONE_NET_MASTER
enum
{
    MASTER_PEER_SIZE_BYTES = NUM_MASTER_PEER * sizeof(on_peer_unit_t)
};
#endif


typedef union
{
    #ifdef _ONE_NET_MASTER
    UInt8 master_peer_storage[MASTER_PEER_SIZE_BYTES];
    #endif
    #ifdef _ONE_NET_CLIENT
    UInt8 client_peer_storage[CLIENT_PEER_SIZE_BYTES];
    #endif
} peer_storage_t;

enum
{
    PEER_STORAGE_SIZE_BYTES = sizeof(peer_storage_t)
};

extern UInt8 peer_storage[];


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
    
    //! True if peer messages are active, false otherwise.
    BOOL active;
} peer_msg_mgr_t;


//! @} ONE-NET_PEER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_PEER_pub_var
//! \ingroup ONE-NET_PEER
//! @{

#ifdef _ONE_NET_MASTER
    extern on_peer_unit_t* master_peer;
#endif


#ifdef _ONE_NET_CLIENT
//! The peer device to communicate with (if set up by the MASTER).  This needs
//! to be assigned a location in the init function (from a parameter).
extern on_peer_t * peer;

//! Manages messages sent to the peer connections that have been set up.
extern peer_msg_mgr_t peer_msg_mgr;/* Note: It's initialized to all zeros */
#endif


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
#ifdef _DATA_RATE
UInt8 on_client_net_data_rate_for_peer(const on_encoded_did_t * const PEER_DID);
BOOL on_client_net_set_peer_data_rate(
  const on_encoded_did_t * const PEER_DID, const UInt8 DATA_RATE);
#endif

one_net_status_t on_client_net_assign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT);

one_net_status_t on_client_net_unassign_peer(const UInt8 SRC_UNIT,
  const on_encoded_did_t * const PEER_DID, const UInt8 PEER_UNIT);

void on_client_net_clear_peer_msg_mgr(peer_msg_mgr_t *mgr);

UInt8 have_more_peers(peer_msg_mgr_t *mgr);

one_net_status_t on_client_net_setup_msg_for_peer(UInt8 * data,
  peer_msg_mgr_t *mgr, const UInt8 result, on_encoded_did_t *dst_did);

#if defined(_ONE_NET_MASTER)
one_net_status_t master_assigned_peer(const UInt8 src_unit,
      const on_encoded_did_t * const peer_did, const UInt8 peer_unit);

void init_master_peer(void);
one_net_status_t master_unassigned_peer(const UInt8 src_unit,
  const on_encoded_did_t* const peer_did, const UInt8 peer_unit,
  const BOOL deviceIsMaster);
#endif

one_net_status_t initialize_peer_mgr(peer_msg_mgr_t* const mgr,
    const UInt8 priority, const UInt8 src_unit);
    
#ifdef _ONE_NET_MASTER
    UInt8 get_master_peer_count(const UInt8 src_unit);
    one_net_status_t master_send_to_peer_list(UInt8* const data,
        const UInt8 data_len, const UInt8 priority, const UInt8 src_unit);
#endif


//! @} ONE-NET_PEER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_PEER
#endif // _ONE_NET_PEER defined //
#endif // _ONE_NET_PEER_H //
