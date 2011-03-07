//! \addtogroup ONE-NET_APP_status_interval
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
    \file ona_status_interval.c
    \brief Status interval msgs implementation.

    This is the implementation to send and parse status interval msgs.
    Any ONE-NET devices that sends or receives status interval msgs will
    include and use this code.
*/

#include <one_net/app/ona_status_interval.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_status_interval_const
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_status_interval_typedefs
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_status_interval_pri_var
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_status_interval_pri_func
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_status_interval_pub_func
//! \ingroup ONE-NET_APP_status_interval
//! @{

/*!
    \brief send a status interval query

    \param[in] SRC_UNIT, source unit of msg
    \param[in] DST_UNIT, destination unit of msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t send_status_interval_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    one_net_status_t rv = ONS_SUCCESS;

    const UInt16 CLASS_TYPE = ONA_STATUS_INTERVAL | ONA_QUERY;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    payload[ONA_STI_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_STI_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;

    // TBD: make this random ?
    payload[ONA_STI_STATUS_IDX + ONA_MSG_DATA_IDX] = 0x00;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_status_interval_query //


/*!
    \brief send a status interval status (seconds, minutes, or hours)

    \param[in] SCALE, the scale of the interval (seconds, minutes, hours)
    \param[in] SRC_UNIT, source unit of msg
    \param[in] DST_UNIT, destination unit of msg
    \param[in] INTERVAL, status interval
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_status_interval_status(
  const ona_interval_scale_t SCALE, const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 INTERVAL, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    one_net_status_t rv = ONS_SUCCESS;
    BOOL proceed = TRUE;

    UInt16 class_type = ONA_STATUS;

    payload[ONA_STI_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_STI_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_STI_STATUS_IDX + ONA_MSG_DATA_IDX] = INTERVAL;

    switch(SCALE)
    {
        case ONA_SECONDS:
        {
            class_type |= ONA_STATUS_INTERVAL_SECONDS;
            break;
        } // ONA_SECONDS //

        case ONA_MINUTES:
        {
            class_type |= ONA_STATUS_INTERVAL_MINUTES;
            break;
        } // ONA_MINUTES //

        case ONA_HOURS:
        {
            class_type |= ONA_STATUS_INTERVAL_HOURS;
            break;
        } // ONA_HOURS //

        default:
        {
            rv = ONS_BAD_PARAM;
            proceed = FALSE;
            break;
        } // default //
    } // switch SCALE //

    one_net_int16_to_byte_stream(class_type, &payload[ONA_MSG_HDR_IDX]);

    if(proceed)
    {
        // send payload
        rv = (*one_net_send_single)(payload, sizeof(payload),
          ONE_NET_LOW_PRIORITY, RAW_DST, SRC_UNIT, DST_UNIT);
    } // if proceed //

    return rv;
} // ona_send_status_interval_status //


/*!
    \brief send a status interval command (seconds, minutes, or hours)

    \param[in] SCALE, the scale of the interval (seconds, minutes, hours)
    \param[in] SRC_UNIT, source unit of msg
    \param[in] DST_UNIT, destination unit of msg
    \param[in] INTERVAL, status interval
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_status_interval_command(
  const ona_interval_scale_t SCALE, const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 INTERVAL, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    one_net_status_t rv = ONS_SUCCESS;
    BOOL proceed = TRUE;

    UInt16 class_type = ONA_COMMAND;

    payload[ONA_STI_SRC_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_STI_DST_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_STI_STATUS_IDX + ONA_MSG_DATA_IDX] = INTERVAL;

    switch(SCALE)
    {
        case ONA_SECONDS:
        {
            class_type |= ONA_STATUS_INTERVAL_SECONDS;
            break;
        } // ONA_SECONDS //

        case ONA_MINUTES:
        {
            class_type |= ONA_STATUS_INTERVAL_MINUTES;
            break;
        } // ONA_MINUTES //

        case ONA_HOURS:
        {
            class_type |= ONA_STATUS_INTERVAL_HOURS;
            break;
        } // ONA_HOURS //

        default:
        {
            rv = ONS_BAD_PARAM;
            proceed = FALSE;
            break;
        } // default //
    } // switch SCALE //

    one_net_int16_to_byte_stream(class_type, &payload[ONA_MSG_HDR_IDX]);

    if(proceed)
    {
        // send payload
        rv = (*one_net_send_single)(payload, sizeof(payload),
          ONE_NET_LOW_PRIORITY, RAW_DST, SRC_UNIT, DST_UNIT);
    } // if proceed //

    return rv;
} // ona_send_status_interval_command //


/*!
    \brief Parse a status interval msg

    \param[in] MSG_DATA, msg data to be parsed
    \param[in] LEN, length of msg data (3)
    \param[in] src_unit, source unit of msg
    \param[in] dst_unit, destination unit of msg
    \param[in] interval, status interval

    \return the status of the send action
*/
one_net_status_t ona_parse_status_interval(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * src_unit, UInt8 * dst_unit, UInt8 * interval)
{
    one_net_status_t rv = ONS_SUCCESS;
    BOOL proceed = TRUE;

    // error check
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    }

    if(proceed)
    {
        *src_unit = MSG_DATA[ONA_STI_SRC_UNIT_IDX];
        *dst_unit = MSG_DATA[ONA_STI_DST_UNIT_IDX];
        *interval = MSG_DATA[ONA_STI_STATUS_IDX];
    } // if proceed //

    return rv;
} // ona_parse_status_interval //

//! @} ONE-NET_APP_status_interval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_status_interval_pri_func
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_status_interval

