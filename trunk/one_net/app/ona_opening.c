//! \addtogroup ONE-NET_APP_opening
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
    \file ona_opening.c
    \brief Implementation of opening msg functions.

    This is the implementation of functions to send an parse
    opening msgs.
*/

#include "ona_opening.h"

#include "one_net_application.h"
#include "one_net.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_opening_const
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_opening_typedefs
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_opening_pri_var
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_opening_pri_func
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_opening_pub_func
//! \ingroup ONE-NET_APP_opening
//! @{


/*!
    \brief Send a opening status msg


    \param[in] SRC_UNIT, the source unit for opening msg
    \param[in] DST_UNIT, the destination unit for opening msg
    \param[in] OPENING_STATUS, opening status (CLOSED/OPEN)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_opening_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_opening_status_t OPENING_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_OPENING;

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_OPEN_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_OPEN_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_OPEN_STATUS_IDX + ONA_MSG_DATA_IDX] = (UInt8)OPENING_STATUS;

    // send payload
    rv = (*ona_net_send_single)(payload, sizeof(payload), ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_opening_status //


/*!
    \brief Send a opening command msg

    \param[in] SRC_UNIT, the source unit for opening msg
    \param[in] DST_UNIT, the destination unit for opening msg
    \param[in] OPENING_STATUS, opening status (CLOSED/OPEN)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_opening_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_opening_status_t OPENING_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_COMMAND | ONA_OPENING;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    payload[ONA_OPEN_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_OPEN_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_OPEN_STATUS_IDX + ONA_MSG_DATA_IDX] = (UInt8)OPENING_STATUS;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_opening_command //


/*!
    \brief Send a opening query msg

    \param[in] SRC_UNIT, the source unit for opening msg
    \param[in] DST_UNIT, the destination unit for opening msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_opening_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_OPENING;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    payload[ONA_OPEN_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_OPEN_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;

    // TBD: make this random ?
    payload[ONA_OPEN_STATUS_IDX + ONA_MSG_DATA_IDX] = 0x00;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_opening_query //


/*!
    \brief parse a opening msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] src_unit, the source unit of the msg
    \param[out] dst_unit, the destination unit of the msg
    \param[out] opening_status, status of opening

    \return the status of the send action
*/
one_net_status_t ona_parse_opening(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * src_unit, UInt8 * dst_unit,
  ona_opening_status_t * opening_status)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    }

    if(proceed)
    {
        // get units
        *src_unit = MSG_DATA[ONA_OPEN_SRC_UNIT_IDX];
        *dst_unit = MSG_DATA[ONA_OPEN_SRC_UNIT_IDX];

        // opening status
        *opening_status = (opening_status_t)MSG_DATA[ONA_OPEN_STATUS_IDX];
    } // if proceed //

    return rv;
} // ona_parse_opening //


//! @} ONE-NET_APP_opening_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_opening_pri_func
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_opening

