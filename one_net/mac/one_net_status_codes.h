#ifndef _ONE_NET_STATUS_CODES_H
#define _ONE_NET_STATUS_CODES_H

#include "config_options.h"


//! \defgroup ONE-NET_status_codes ONE-NET Status Codes
//! \ingroup ONE-NET
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
    \file one_net_status_codes.h
    \brief ONE-NET status codes.

    Status codes returned by ONE-NET functions.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_status_codes_const
//! \ingroup ONE-NET_status_codes
//! @{

//! @} ONE-NET_status_codes_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_status_codes_typedefs
//! \ingroup ONE-NET_status_codes
//! @{

typedef enum
{
    ONS_SUCCESS,                    //!< [0] Functions was successful

    ONS_BAD_PARAM,                  //!< [1] The parameters passed in are invalid
    ONS_NOT_INIT,                   //!< [2] Was not initialized
    ONS_ALREADY_IN_PROGRESS,        //!< [3] The same operation is in progress
    ONS_INVALID_DATA,               //!< [4] Received invalid data
    ONS_MORE,                       //!< [5] If more data is expected
    ONS_END,                        //!< [6] The end has been reached.
    ONS_RSRC_FULL,                  //!< [7] The resources have reached capacity
    ONS_CANCELED,                   //!< [8] The operation was canceled
    ONS_TIME_OUT,                   //!< [9] The operation timed out
    ONS_INTERNAL_ERR,               //!< [10] An internal error occured.

    ONS_UNHANDLED_VER,              //!< [11] The version is not handled by this dev.
    ONS_CRC_FAIL,                   //!< [12] Computed crc did not match received
    ONS_RX_STAY_AWAKE,              //!< [13] A Stay Awake ACK was received.
    ONS_RX_NACK,                    //!< [14] Received a NACK
	ONS_TX_NACK,                    //!< [15] Send a NACK
    ONS_INCORRECT_NONCE,            //!< [16] The received nonce was not correct

    ONS_SINGLE_END,                 //!< [17] The transaction completed successfully
    ONS_SINGLE_FAIL,                //!< [18] Single transaction failed.

    ONS_BLOCK_END,                  //!< [19] Block txn completed successfully
    ONS_BLOCK_FAIL,                 //!< [20] The block transaction has failed

    ONS_STREAM_END,                 //!< [21] The other side has ended the stream
    ONS_STREAM_FAIL,                //!< [22] The stream transaction has failed

    ONS_TXN_QUEUED,                 //!< [23] If a txn has been queued to send back
    ONS_TXN_DOES_NOT_EXIST,         //!< [24] The transaction does not exist

    ONS_BAD_RAW_PKT_LEN,            //!< [25] Invalid length for a raw message
    ONS_BAD_RAW_DATA,               //!< [26] The data in a raw block was not valid
    ONS_BAD_ENCODING,               //!< [27] The encoding was not valid
    ONS_BAD_PKT_TYPE,               //!< [28] Received an unknown pkt type
    ONS_BAD_PKT,                    //!< [29] The packet was invalid
    ONS_UNHANDLED_PKT,              //!< [30] The packet is not being handled

    ONS_UNICAST_ADDR,               //!< [31] Our unicast address.
    ONS_MULTICAST_ADDR,             //!< [32] Received one of our group addresses
    ONS_BROADCAST_ADDR,             //!< [33] Received a broadcast address
    ONS_BAD_ADDR,                   //!< [34] The address format is invalid
    ONS_INCORRECT_ADDR,             //!< [35] Received sender addr not expected.
    ONS_NID_FAILED,                 //!< [36] The NID portion of the address failed
    ONS_DID_FAILED,                 //!< [37] The DID portion of the address failed
    
    ONS_DEVICE_LIMIT,               //!< [38] The MASTER can't handle anymore devices
    ONS_NOT_JOINED,                 //!< [39] The device has not joined the network.

    ONS_READ_ERR,                   //!< [40] Did not read specified number of bytes.
    ONS_WRITE_ERR,                  //!< [41] Didn't write specified number of bytes.
	ONS_REBUILD_PKT,                //!< [42] Packet must be rebuilt.
	ONS_SINGLE_CANCELED,            //!< [43] Single transaction cancelled for whatever reason
	ONS_BLOCK_CANCELED,             //!< [44] Block transaction cancelled for whatever reason
	ONS_STREAM_CANCELED,            //!< [45] Stream transaction cancelled for whatever reason
    ONS_RETRY,                      //!< [46] Some action should be tried again.
    
    #ifdef NON_BS_INVITE
    // Threshold Corporation proprietary software codes
    ONS_NON_BS_INVITE_IN_PROGRESS,  //!< [47] a non-Blue-Spot Invitation is in progress
    ONS_NON_BS_INVITE_IDLE,         //!< [48] No non-Blue-Spot Invitations are in progress
    ONS_NON_BS_INVITE_DEVICE_JOINED,//!< [49] A device has joined as a result of a
                                    //!<      non-Blue-Spot inviation.
    #endif
    
    ONS_SNGH_INTERNAL_ERR           //!< [50] "Should Not Get Here" in the code internal error. 
} one_net_status_t;

//! @} ONE-NET_status_codes_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_status_codes_pub_var
//! \ingroup ONE-NET_status_codes
//! @{

//! @} ONE-NET_status_codes_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_status_codes_pub_func
//! \ingroup ONE-NET_status_codes
//! @{

//! @} ONE-NET_status_codes_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================
//! @} ONE-NET_status_codes

#endif // _ONE_NET_STATUS_CODES_H //

