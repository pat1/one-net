//! \addtogroup ONE-NET
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
    \file one_net.c
    \brief Basis for ONE-NET implementation.

    This file is application independent.  The functionality implemented here
    is also independent of the device being a MASTER or CLIENT.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_packet.h"
#include "one_net.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"
#include "one_net_xtea.h"



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Preamble
static const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
static const UInt8 SOF[] = {0x33};


enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};


const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0xB4, 0xB4};
const on_raw_did_t ON_RAW_BROADCAST_DID = {0x00, 0x00};
const on_raw_did_t MASTER_RAW_DID = {0x00, 0x10};
const on_encoded_did_t MASTER_ENCODED_DID = {0xB4, 0xBC};

//! @} ONE-NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_typedefs
//! \ingroup ONE-NET
//! @{

//! @} ONE-NET_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES


//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
UInt8 nv_param[NV_PARAM_SIZE_BYTES];

//! The base parameters for the device
on_base_param_t* const on_base_param = (on_base_param_t* const) nv_param;


//! location to store the encoded data for an ack/nack packet
UInt8 response_pkt[ON_ACK_NACK_ENCODED_PKT_SIZE];

//! Used to send a response
on_txn_t response_txn = {ONE_NET_NO_PRIORITY, 0, ON_INVALID_MSG_TYPE, 0,
  0, sizeof(response_pkt), response_pkt, NULL};

//! location to store the encoded data for the single transaction
UInt8 single_pkt[ON_SINGLE_ENCODED_PKT_SIZE];

//! Used to send a single message
on_txn_t single_txn = {ONE_NET_NO_PRIORITY, 0, ON_INVALID_MSG_TYPE, 0, 0,
  sizeof(single_pkt), single_pkt, NULL};

#ifdef _BLOCK_MESSAGES_ENABLED
    //! location to store the encoded data for a block transaction.
    UInt8 block_pkt[ON_BLOCK_ENCODED_PKT_SIZE];
    
    //! The current block transaction
    on_txn_t block_txn = {ONE_NET_NO_PRIORITY, 0, ON_INVALID_MSG_TYPE,
      ONT_BLOCK_TIMER, 0, sizeof(block_pkt), block_pkt, NULL};

    #ifdef _STREAM_MESSAGES_ENABLED
    //! location to store the encoded data for a stream transaction.
    UInt8 stream_pkt[ON_STREAM_ENCODED_PKT_SIZE];
    
    //! The current stream transaction
    on_txn_t stream_txn = {ONE_NET_NO_PRIORITY, 0, ON_INVALID_MSG_TYPE,
      ONT_STREAM_TIMER, 0, sizeof(stream_pkt), stream_pkt, NULL};    
    #endif
#endif // if block messages are not enabled //


//! true if device is functioning as a master, false otherwise
#ifndef _ONE_NET_MASTER
BOOL device_is_master = FALSE;
#else
BOOL device_is_master = TRUE; // if device cvan be master OR client, the
                              // initialization code will need to set this
                              // value
#endif


one_net_startup_status_t startup_status = ON_STARTUP_IN_PROGRESS;


//                              PUBLIC VARIABLES
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{


//! The current state.  This is a "protected" variable.
on_state_t on_state = ON_INIT_STATE;


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    // fill in the preamble in the Multi-Hop packet to be sent.  The rest will
    // be filled in when the received Multi-Hop packet is read in over the rf
    // interface.
    static UInt8 mh_pkt[ON_MAX_ENCODED_PKT_SIZE] = {0x55, 0x55, 0x55, 0x33};

    // Transaction for forwarding on MH packets.
    static on_txn_t mh_txn = {ONE_NET_LOW_PRIORITY, 0, ON_INVALID_MSG_TYPE,
      ONT_MH_TIMER, 0, sizeof(mh_pkt), mh_pkt};
#endif


//! @} ONE-NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{


//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{


/*!
    \brief The main ONE-NET function.

    This is the main state machine for ONE-NET.  It handles sending/receiving
    the various transactions.

    \param[in/out] txn The transaction currently being carried out

    \return TRUE if the current transaction is completed.  This should coincide
              with the priority for the transaction being set to
              ONE_NET_NO_PRIORITY unless another transaction resulted from the
              one that just finished.
            FALSE if the current transaction is not complete.
*/
BOOL one_net(on_txn_t ** txn)
{
    return TRUE;
} // one_net //

//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{


//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET
