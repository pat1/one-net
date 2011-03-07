//! \addtogroup ONE-NET_APP_direction
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
    \file ona_direction.c
    \brief Implementation of direction msg functions.

    This is the implementation of functions to send an parse direction msgs.
*/

#include "ona_direction.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_direction_const
//! \ingroup ONE-NET_APP_direction
//! @{

//! @} ONE-NET_APP_direction_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_direction_typedefs
//! \ingroup ONE-NET_APP_direction
//! @{

//! @} ONE-NET_APP_direction_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_direction_pri_var
//! \ingroup ONE-NET_APP_direction
//! @{

//! @} ONE-NET_APP_direction_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_direction_pri_func
//! \ingroup ONE-NET_APP_direction
//! @{

//! @} ONE-NET_APP_direction_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_direction_pub_func
//! \ingroup ONE-NET_APP_direction
//! @{


/*!
    \brief Send a direction status msg

    \param[in] SRC_UNIT, the source unit of direction msg
    \param[in] DST_UNIT, the destination unit for direction msg
    \param[in] DIRECTION, direction status (in 0.01 degrees)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_direction_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt16 DIRECTION,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_DIRECTION;
    
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_DIRECTION_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    one_net_int16_to_byte_stream(DIRECTION,
      &payload[ONA_DIRECTION_DIRECTION_IDX + ONA_MSG_DATA_IDX]);
    
    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_direction_status //


/*!
    \brief Send a direction command msg

    \param[in] SRC_UNIT, the source unit of direction msg
    \param[in] DST_UNIT, the destination unit for direction msg
    \param[in] DIRECTION, direction status (in 0.01 degrees)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_direction_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt16 DIRECTION,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_COMMAND | ONA_DIRECTION;
    
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_DIRECTION_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    one_net_int16_to_byte_stream(DIRECTION, &payload[ONA_DIRECTION_DIRECTION_IDX
      + ONA_MSG_DATA_IDX]);

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_direction_command //


/*!
    \brief Send a direction query msg

    \param[in] SRC_UNIT, the source unit for direction msg
    \param[in] DST_UNIT, the destination unit for direction msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_direction_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_DIRECTION;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    
    payload[ONA_DIRECTION_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_direction_query //


/*!
    \brief parse a direction msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] unit, the unit (src or dst)
    \param[out] direction, the direction

    \return the status of the send action
*/
one_net_status_t ona_parse_direction(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * unit, UInt16 * direction)
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
        // get unit
        *unit = MSG_DATA[ONA_DIRECTION_UNIT_IDX];

        // get direction
        *direction = one_net_byte_stream_to_int16(
          &MSG_DATA[ONA_DIRECTION_DIRECTION_IDX]);
    
    } // if proceed //
    
    return rv;
} // ona_parse_direction //


//! @} ONE-NET_APP_direction_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_direction_pri_func
//! \ingroup ONE-NET_APP_direction
//! @{

//! @} ONE-NET_APP_direction_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_direction

