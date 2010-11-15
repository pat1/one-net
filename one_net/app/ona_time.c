//! \addtogroup ONE-NET_APP_time
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
    \file ona_time.c
    \brief Implementation of time msg functions.

    This is the implementation of functions to send an parse time msgs.
*/

#include "ona_time.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_time_const
//! \ingroup ONE-NET_APP_time
//! @{

enum
{
    //! Index into data portion of msg that contains the hour value
    ONA_HOUR_IDX = 0,

    //! Index into data portion of msg that contains the high portion of the
    //! minutes value
    ONA_MINUTE_HIGH_IDX = 0,

    //! Index into data portion of msg that contains the low portion of the
    //! minute value
    ONA_MINUTE_LOW_IDX = 1,

    //! Index into data portion of msg that contains the high portion of the
    //! seconds value
    ONA_SECOND_HIGH_IDX = 1,

    //! Index into data portion of msg that contains the low portion of the
    //! seconds value
    ONA_SECOND_LOW_IDX = 2,

    //! Number of bits to shift hour value when building/parsing time msgs.
    ONA_HOUR_SHIFT = 3,

    //! Mask to use for hour value when building time msgs
    ONA_HOUR_BUILD_MASK = 0xF8,

    //! Mask to use for hour value when parsing time msgs
    ONA_HOUR_PARSE_MASK = 0x1F,

    //! Number of bits to shift high part of minute value when
    //! building/parsing time msgs
    ONA_MINUTE_HIGH_SHIFT = 3,

    //! Number of bits to shift low part of minute value when
    //! building/parsing time msgs
    ONA_MINUTE_LOW_SHIFT = 5,

    //! Mask to use for high part of minute value when building msgs
    ONA_MINUTE_BUILD_HIGH_MASK = 0x07,

    //! Mask to use for high part of minute value when parsing msgs
    ONA_MINUTE_PARSE_HIGH_MASK = 0x31,

    //! Mask to use for low part of minute value when building msgs
    ONA_MINUTE_BUILD_LOW_MASK = 0xE0,

    //! Mask to use for low part of minute value when parsing msgs
    ONA_MINUTE_PARSE_LOW_MASK = 0x07,

    //! Number of bits to shift high part of seconds value when
    //! building/parsing time msgs
    ONA_SECOND_HIGH_SHIFT = 1,

    //! Number of bits to shift low part of seconds value when
    //! building/parsing time msgs
    ONA_SECOND_LOW_SHIFT = 7,

    //! Mask to use for high part of seconds value when building msgs
    ONA_SECOND_BUILD_HIGH_MASK = 0x1F,

    //! Mask to use for high part of seconds value when parsing msgs
    ONA_SECOND_PARSE_HIGH_MASK = 0x3E,

    //! Mask to use for low part of seconds value when building msgs
    ONA_SECOND_BUILD_LOW_MASK = 0x80,

    //! Mask to use for low part of seconds value when parsing msgs
    ONA_SECOND_PARSE_LOW_MASK = 0x01
};

//! @} ONE-NET_APP_time_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_time_typedefs
//! \ingroup ONE-NET_APP_time
//! @{

//! @} ONE-NET_APP_time_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_time_pri_var
//! \ingroup ONE-NET_APP_time
//! @{

//! @} ONE-NET_APP_time_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_time_pri_func
//! \ingroup ONE-NET_APP_time
//! @{

//! @} ONE-NET_APP_time_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_time_pub_func
//! \ingroup ONE-NET_APP_time
//! @{


/*!
    \brief Send a time status msg

    \param[in] HOUR, the hour in 24-hour format
    \param[in] MINUTE, the minute 
    \param[in] SECOND, The second.
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_time_status(const UInt8 HOUR, const UInt8 MINUTE,
  const UInt8 SECOND, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_TIME;

    if(HOUR > ONA_MAX_HOUR || MINUTE > ONA_MAX_MINUTE
      || SECOND > ONA_MAX_SECOND)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[HOUR + ONA_MSG_DATA_IDX]
      = (HOUR << ONA_HOUR_SHIFT) & ONA_HOUR_BUILD_MASK;
    payload[ONA_MINUTE_HIGH_IDX + ONA_MSG_DATA_IDX]
      |= ((UInt8)MINUTE >> ONA_MINUTE_HIGH_SHIFT) & ONA_MINUTE_BUILD_HIGH_MASK;
    payload[ONA_MINUTE_LOW_IDX + ONA_MSG_DATA_IDX]
      = ((UInt8)MINUTE << ONA_MINUTE_LOW_SHIFT) & ONA_MINUTE_BUILD_LOW_MASK;
    payload[ONA_SECOND_HIGH_IDX + ONA_MSG_DATA_IDX]
      |= (SECOND >> ONA_SECOND_HIGH_SHIFT) & ONA_SECOND_BUILD_HIGH_MASK;
    payload[ONA_SECOND_LOW_IDX + ONA_MSG_DATA_IDX]
      = (SECOND << ONA_SECOND_LOW_SHIFT) & ONA_SECOND_BUILD_LOW_MASK;
    
    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);
} // ona_send_time_status //


/*!
    \brief Send a time query msg

    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_time_query(const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONA_MSG_HDR_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_TIME;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);
} // ona_send_time_query //


/*!
    \brief parse a time msg

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] hour, the hour received in the msg (24-hour format)
    \param[out] minute, the minute received from the msg
    \param[out] second, the second that was received in the msg

    \return the status of the send action
*/
one_net_status_t ona_parse_time(const UInt8 * const MSG_DATA, const UInt8 LEN,
  UInt8 * const hour, UInt8 * const minute, UInt8 * const second)
{
    // error checking
    if(!MSG_DATA || LEN != ONA_MSG_DATA_LEN || !hour || !minute || !second)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid

    *hour = (MSG_DATA[ONA_HOUR_IDX] >> ONA_HOUR_SHIFT) & ONA_HOUR_PARSE_MASK;
    *minute = ((MSG_DATA[ONA_MINUTE_HIGH_IDX] << ONA_MINUTE_HIGH_SHIFT)
      & ONA_MINUTE_PARSE_HIGH_MASK)
      | ((MSG_DATA[ONA_MINUTE_LOW_IDX] >> ONA_MINUTE_LOW_SHIFT)
      & ONA_MINUTE_PARSE_LOW_MASK);
    *second = ((MSG_DATA[ONA_SECOND_HIGH_IDX] << ONA_SECOND_HIGH_SHIFT)
      & ONA_SECOND_PARSE_HIGH_MASK)
      | ((MSG_DATA[ONA_SECOND_LOW_IDX] >> ONA_SECOND_LOW_SHIFT)
      & ONA_SECOND_PARSE_LOW_MASK);

    if(*hour > ONA_MAX_HOUR || *minute > ONA_MAX_MINUTE
      || *second > ONA_MAX_SECOND)
    {
        return ONS_INVALID_DATA;
    } // if the received data was invalid //

    return ONS_SUCCESS;
} // ona_parse_time //

//! @} ONE-NET_APP_time_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_time_pri_func
//! \ingroup ONE-NET_APP_time
//! @{

//! @} ONE-NET_APP_time_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_time

