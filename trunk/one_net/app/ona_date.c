//! \addtogroup ONE-NET_APP_date
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
    \file ona_date.c
    \brief Implementation of date msg functions.

    This is the implementation of functions to send an parse date msgs.
*/

#include <one_net/app/ona_date.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_date_const
//! \ingroup ONE-NET_APP_date
//! @{

enum
{
    //! Index into data portion of msg that contains the date value
    ONA_YEAR_IDX = 0,

    //! Index into data portion of msg that contains the high portion of the
    //! month value
    ONA_MONTH_HIGH_IDX = 0,

    //! Index into data portion of msg that contains the low portion of the
    //! month value
    ONA_MONTH_LOW_IDX = 1,

    //! Index into data portion of msg that contains the day value
    ONA_DAY_IDX = 1,

    //! Index into data portion of msg that contains the day of the week value
    ONA_DOW_IDX = 2,

    //! Number of bits to shift year value when building/parsing date msgs.
    ONA_YEAR_SHIFT = 1,

    //! Mask to use for year value when building date msgs
    ONA_YEAR_BUILD_MASK = 0xFE,

    //! Mask to use for year value when parsing date msgs
    ONA_YEAR_PARSE_MASK = 0x7F,

    //! Number of bits to shift high part of month value when
    //! building/parsing date msgs
    ONA_MONTH_HIGH_SHIFT = 3,

    //! Number of bits to shift low part of MONTH value when
    //! building/parsing date msgs
    ONA_MONTH_LOW_SHIFT = 5,

    //! Mask to use for high part of month value when building msgs
    ONA_MONTH_BUILD_HIGH_MASK = 0x01,

    //! Mask to use for high part of month value when parsing msgs
    ONA_MONTH_PARSE_HIGH_MASK = 0x08,

    //! Mask to use for low part of month value when building msgs
    ONA_MONTH_BUILD_LOW_MASK = 0xE0,

    //! Mask to use for low part of month value when parsing msgs
    ONA_MONTH_PARSE_LOW_MASK = 0x07,

    //! Mask to use for day value when building or parsing date msgs
    ONA_DAY_MASK = 0x1F,

    //! Number of bits to shift the day of the week when building/parsing date
    //! msgs
    ONA_DOW_SHIFT = 5,

    //! Mask to use for day of the week value when parsing date msgs.
    ONA_DOW_PARSE_MASK = 0x07
};

//! @} ONE-NET_APP_date_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_date_typedefs
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_date_pri_var
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_date_pri_func
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_date_pub_func
//! \ingroup ONE-NET_APP_date
//! @{


/*!
    \brief Send a date status msg

    \param[in] YEAR, the year
    \param[in] MONTH, the month
    \param[in] DAY, The day.
    \param[in] DOW, The day of the week
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_date_status(const UInt16 YEAR,
  const ona_month_t MONTH, const UInt8 DAY, const ona_day_of_week_t DOW,
  const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_DATE;

    if(YEAR > ONA_MAX_YEAR || MONTH > ONA_DECEMBER || DAY > ONA_MAX_DAY
      || DOW > ONA_SUNDAY)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_YEAR_IDX + ONA_MSG_DATA_IDX]
      = (((UInt8)(YEAR - ONA_BASE_YEAR)) << ONA_YEAR_SHIFT)
      & ONA_YEAR_BUILD_MASK;
    payload[ONA_MONTH_HIGH_IDX + ONA_MSG_DATA_IDX]
      |= ((UInt8)MONTH >> ONA_MONTH_HIGH_SHIFT) & ONA_MONTH_BUILD_HIGH_MASK;
    payload[ONA_MONTH_LOW_IDX + ONA_MSG_DATA_IDX]
      = ((UInt8)MONTH << ONA_MONTH_LOW_SHIFT) & ONA_MONTH_BUILD_LOW_MASK;
    payload[ONA_DAY_IDX + ONA_MSG_DATA_IDX] |= DAY & ONA_DAY_MASK;
    payload[ONA_DOW_IDX + ONA_MSG_DATA_IDX] = ((UInt8)DOW << ONA_DOW_SHIFT)
      & ONA_DAY_MASK;

    // send payload
    return (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);
} // ona_send_date_status //


/*!
    \brief Send a date query msg

    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_date_query(const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONA_MSG_HDR_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_DATE;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    // send payload
    return (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);
} // ona_send_date_query //


/*!
    \brief parse a date msg

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] year, the year received in the msg
    \param[out] month, the month received from the msg
    \param[out] day, the day that was received in the msg
    \param[out] dow, the day of the week that was received in the msg

    \return the status of the send action
*/
one_net_status_t ona_parse_date(const UInt8 * const MSG_DATA, const UInt8 LEN,
  UInt16 * const year, ona_month_t * const month, UInt8 * const day,
  ona_day_of_week_t * dow)
{
    // error checking
    if(!MSG_DATA || LEN != ONA_MSG_DATA_LEN || !year || !month || !day || !dow)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid

    *year = (((MSG_DATA[ONA_YEAR_IDX] >> ONA_YEAR_SHIFT) & ONA_YEAR_PARSE_MASK)
      + ONA_BASE_YEAR);
    *month = ((MSG_DATA[ONA_MONTH_HIGH_IDX] << ONA_MONTH_HIGH_SHIFT)
      & ONA_MONTH_PARSE_HIGH_MASK)
      | ((MSG_DATA[ONA_MONTH_LOW_IDX] >> ONA_MONTH_LOW_SHIFT)
      & ONA_MONTH_PARSE_LOW_MASK);
    *day = MSG_DATA[ONA_DAY_IDX] & ONA_DAY_MASK;
    *dow = (MSG_DATA[ONA_DOW_IDX] >> ONA_DOW_SHIFT) & ONA_DOW_PARSE_MASK;

    return ONS_SUCCESS;
} // ona_parse_date //

//! @} ONE-NET_APP_date_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_date_pri_func
//! \ingroup ONE-NET_APP_date
//! @{

//! @} ONE-NET_APP_date_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_date

