//! \addtogroup ONE-NET_APP_seal
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
    \file ona_seal.c
    \brief Implementation of seal msg functions.

    This is the implementation of functions to send an parse seal msgs.
*/

#include "one_net_application.h"
#include "one_net.h"
#include "seal.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_seal_const
//! \ingroup ONE-NET_APP_seal
//! @{

//! @} ONE-NET_APP_seal_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_seal_typedefs
//! \ingroup ONE-NET_APP_seal
//! @{

//! @} ONE-NET_APP_seal_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_seal_pri_var
//! \ingroup ONE-NET_APP_seal
//! @{

//! @} ONE-NET_APP_seal_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_seal_pri_func
//! \ingroup ONE-NET_APP_seal
//! @{

//! @} ONE-NET_APP_seal_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_seal_pub_func
//! \ingroup ONE-NET_APP_seal
//! @{

/*!
    \brief Send a seal status msg


    \param[in] SRC_UNIT, the source unit for seal msg
    \param[in] DST_UNIT, the destination unit for seal msg
    \param[in] SEAL_STATUS, seal status (BROKEN/INTACT)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_seal_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_seal_status_t SEAL_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_SEAL;
    
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_SEAL_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_SEAL_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_SEAL_STATUS_IDX + ONA_MSG_DATA_IDX] = (UInt8)SEAL_STATUS;
    
    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_seal_status //


/*!
    \brief Send a seal command msg

    \param[in] SRC_UNIT, the source unit for seal msg
    \param[in] DST_UNIT, the destination unit for seal msg
    \param[in] SEAL_STATUS, seal status (BROKEN/INTACT)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_seal_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_seal_status_t SEAL_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_COMMAND | ONA_SEAL;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    
    payload[ONA_SEAL_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_SEAL_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_SEAL_STATUS_IDX + ONA_MSG_DATA_IDX] = (UInt8)SEAL_STATUS;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_seal_command //


/*!
    \brief Send a seal query msg

    \param[in] SRC_UNIT, the source unit for seal msg
    \param[in] DST_UNIT, the destination unit for seal msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_seal_query(const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_SEAL;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    
    payload[ONA_SEAL_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_SEAL_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;

    // TBD: make this random ?
    payload[ONA_SEAL_STATUS_IDX + ONA_MSG_DATA_IDX] = 0x00;
    
    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_seal_query //


/*!
    \brief parse a seal msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] src_unit, the source unit of the msg
    \param[out] dst_unit, the destination unit of the msg
    \param[out] seal_status, status of seal

    \return the status of the send action
*/
one_net_status_t ona_parse_seal(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * src_unit, UInt8 * dst_unit, 
  ona_seal_status_t * seal_status)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(proceed)
    {
        // get units
        *src_unit = MSG_DATA[ONA_SEAL_SRC_UNIT_IDX];
        *dst_unit = MSG_DATA[ONA_SEAL_DST_UNIT_IDX];

        // seal status
        *seal_status = (seal_status_t)MSG_DATA[ONA_SEAL_STATUS_IDX];
    
    } // if proceed //
    
    return rv;
} // ona_parse_seal //

//! @} ONE-NET_APP_seal_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_seal_pri_func
//! \ingroup ONE-NET_APP_seal
//! @{

//! @} ONE-NET_APP_seal_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_seal

