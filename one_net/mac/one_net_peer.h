#ifndef _ONE_NET_PEER_H
#define _ONE_NET_PEER_H

//! \defgroup ONE-NET_PEER ONE-NET PEER functionality.
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
#include "one_net_port_specific.h"
#include "one_net_constants.h"
#include "one_net_status_codes.h"
#include "one_net_application.h"
#include "one_net_port_const.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PEER_const
//! \ingroup ONE-NET_PEER
//! @{


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
} on_peer_unit_t;


enum
{
    PEER_STORAGE_SIZE_BYTES = ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_unit_t)
};



//! @} ONE-NET_PEER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_PEER_pub_var
//! \ingroup ONE-NET_PEER
//! @{


extern UInt8 peer_storage[];
extern on_peer_unit_t* const peer;


//! @} ONE-NET_PEER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PEER_pub_func
//! \ingroup ONE-NET_PEER
//! @{


one_net_status_t one_net_reset_peers(void);
void add_peers_to_recipient_list(const on_single_data_queue_t*
  msg, on_recipient_list_t* send_list, const on_peer_unit_t* peer_list);
one_net_status_t one_net_add_peer_to_list(const UInt8 SRC_UNIT,
  on_peer_unit_t* peer_list, const on_encoded_did_t * const PEER_DID,
  const UInt8 PEER_UNIT);
on_single_data_queue_t* load_next_recipient(on_single_data_queue_t* msg_ptr);


//! @} ONE-NET_PEER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_PEER
#endif // _ONE_NET_PEER defined //
#endif // _ONE_NET_PEER_H //
