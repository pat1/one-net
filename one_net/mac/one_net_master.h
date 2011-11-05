#ifndef _ONE_NET_MASTER_H
#define _ONE_NET_MASTER_H

#include "config_options.h"
#include "one_net.h"



//! \defgroup ONE-NET_MASTER ONE-NET MASTER device functionality
//! \ingroup ONE-NET
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
    \file one_net_master.h
    \brief ONE-NET MASTER functionality declarations.

    Derives from ONE-NET.  MASTER dependent functionality.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MASTER_const
//! \ingroup ONE-NET_MASTER
//! @{



enum
{
    //! The initial CLIENT DID.  Since a DID is only 12 bits, only the upper 12
    //! bits of this value is used.  Note the order this is stored in memory may
    //! not be the proper order for being sent.
    ONE_NET_INITIAL_CLIENT_DID = 0x0020
};


//! Value to increment the next CLIENT did by each time a device is added
//! to the network
#define ON_CLIENT_DID_INCREMENT 0x0010



//! @} ONE-NET_MASTER_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MASTER_typedefs
//! \ingroup ONE-NET_MASTER
//! @{



//! @} ONE-NET_MASTER_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MASTER_pub_var
//! \ingroup ONE-NET_MASTER
//! @{


extern on_master_param_t * const master_param;
extern on_client_t * const client_list;
extern UInt8 invite_pkt[];
extern on_txn_t invite_txn;


//! @} ONE-NET_MASTER_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MASTER_pub_func
//! \ingroup ONE-NET_MASTER
//! @{



#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD,
  const one_net_xtea_key_t * const STREAM_KEY,
  const UInt8 STREAM_ENCRYPT_METHOD);
#else
one_net_status_t one_net_master_create_network(
  const on_raw_sid_t * const SID, const one_net_xtea_key_t * const KEY,
  const UInt8 SINGLE_BLOCK_ENCRYPT_METHOD);
#endif

one_net_status_t one_net_master_init(const UInt8 * const PARAM,
  const UInt16 PARAM_LEN);

one_net_status_t one_net_master_change_key(
  const one_net_xtea_key_fragment_t KEY_FRAGMENT);
  
#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t one_net_master_change_stream_key(
  const one_net_xtea_key_t * const NEW_STREAM_KEY);
#endif

one_net_status_t one_net_master_invite(const one_net_xtea_key_t * const KEY);
one_net_status_t one_net_master_cancel_invite(
  const one_net_xtea_key_t * const KEY);

one_net_status_t one_net_master_remove_device(
  const on_raw_did_t * const RAW_PEER_DID);

void one_net_master(void);

one_net_status_t one_net_master_add_client(const on_features_t features,
  on_base_param_t* out_base_param, on_master_t* out_master_param);



//! @} ONE-NET_MASTER_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MASTER

#endif // _ONE_NET_MASTER_H //
