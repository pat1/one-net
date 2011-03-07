//! \addtogroup ONE-NET_APP_color
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
    \file ona_color.c
    \brief Implementation of color msg functions.

    This is the implementation of functions to send an parse color msgs.
*/

#include "ona_color.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_color_const
//! \ingroup ONE-NET_APP_color
//! @{

enum
{
    //! Index into data portion of msg that contains the red value
    ONA_COLOR_RED_IDX = ONA_COLOR_COLOR_IDX + 0,

    //! Index into data portion of msg that contains the high portion of the
    //! green value
    ONA_COLOR_GREEN_HIGH_IDX = ONA_COLOR_COLOR_IDX + 0,

    //! Index into data portion of msg that contains the low portion of the
    //! green value
    ONA_COLOR_GREEN_LOW_IDX = ONA_COLOR_COLOR_IDX + 1,

    //! Index into data portion of msg that contains the blue value
    ONA_COLOR_BLUE_IDX = ONA_COLOR_COLOR_IDX + 1,

    //! Number of bits to shift red value when building/parsing color msgs.
    ONA_COLOR_RED_SHIFT = 3,

    //! Mask to use for red value when building color msgs
    ONA_COLOR_RED_BUILD_MASK = 0xF8,

    //! Mask to use for red value when parsing color msgs
    ONA_COLOR_RED_PARSE_MASK = 0x1F,

    //! Number of bits to shift high part of green value when
    //! building/parsing color msgs
    ONA_COLOR_GREEN_HIGH_SHIFT = 3,

    //! Number of bits to shift low part of green value when
    //! building/parsing color msgs
    ONA_COLOR_GREEN_LOW_SHIFT = 5,

    //! Mask to use for high part of green value when building msgs
    ONA_COLOR_GREEN_BUILD_HIGH_MASK = 0x07,

    //! Mask to use for high part of green value when parsing msgs
    ONA_COLOR_GREEN_PARSE_HIGH_MASK = 0x38,

    //! Mask to use for low part of green value when building msgs
    ONA_COLOR_GREEN_BUILD_LOW_MASK = 0xE0,

    //! Mask to use for low part of green value when parsing msgs
    ONA_COLOR_GREEN_PARSE_LOW_MASK = 0x07,

    //! Mask to use for blue value when building or parsing color msgs
    ONA_COLOR_BLUE_MASK = 0x1F
};

//! @} ONE-NET_APP_color_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_color_typedefs
//! \ingroup ONE-NET_APP_color
//! @{

//! @} ONE-NET_APP_color_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_color_pri_var
//! \ingroup ONE-NET_APP_color
//! @{

//! @} ONE-NET_APP_color_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_color_pri_func
//! \ingroup ONE-NET_APP_color
//! @{

//! @} ONE-NET_APP_color_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_color_pub_func
//! \ingroup ONE-NET_APP_color
//! @{


/*!
    \brief Send a color status msg

    \param[in] SRC_UNIT, the source unit of color msg
    \param[in] DST_UNIT, the destination unit for color msg
    \param[in] RED, The red color setting.
    \param[in] GREEN, The green color setting
    \param[in] BLUE, The blue color setting
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_color_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 RED, const UInt8 GREEN, const UInt8 BLUE,
  const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_STATUS | ONA_COLOR;

    if(RED > ONA_COLOR_MAX_RED || GREEN > ONA_MAX_COLOR_G
      || BLUE > ONA_COLOR_MAX_BLUE)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_COLOR_UNIT_IDX + ONA_MSG_DATA_IDX] = SRC_UNIT;
    payload[ONA_COLOR_RED_IDX + ONA_MSG_DATA_IDX] = (RED << ONA_COLOR_RED_SHIFT)
      & ONA_COLOR_RED_BUILD_MASK;
    payload[ONA_COLOR_GREEN_HIGH_IDX + ONA_MSG_DATA_IDX]
      |= (GREEN >> ONA_COLOR_GREEN_HIGH_SHIFT)
      & ONA_COLOR_GREEN_BUILD_HIGH_MASK;
    payload[ONA_COLOR_GREEN_LOW_IDX + ONA_MSG_DATA_IDX]
      = (GREEN << ONA_COLOR_GREEN_LOW_SHIFT) & ONA_COLOR_GREEN_BUILD_LOW_MASK;
    payload[ONA_COLOR_BLUE_IDX + ONA_MSG_DATA_IDX]
      |= BLUE & ONA_COLOR_BLUE_MASK;

    // send payload
    return (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);
} // ona_send_color_status //


/*!
    \brief Send a color command msg

    \param[in] SRC_UNIT, the source unit of color msg
    \param[in] DST_UNIT, the destination unit for color msg
    \param[in] RED, The red color setting.
    \param[in] GREEN, The green color setting
    \param[in] BLUE, The blue color setting
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_color_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 RED, const UInt8 GREEN, const UInt8 BLUE,
  const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_COMMAND | ONA_COLOR;

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    payload[ONA_COLOR_UNIT_IDX + ONA_MSG_DATA_IDX] = DST_UNIT;
    payload[ONA_COLOR_RED_IDX + ONA_MSG_DATA_IDX] = (RED << ONA_COLOR_RED_SHIFT)
      & ONA_COLOR_RED_BUILD_MASK;
    payload[ONA_COLOR_GREEN_HIGH_IDX + ONA_MSG_DATA_IDX]
      |= (GREEN >> ONA_COLOR_GREEN_HIGH_SHIFT)
      & ONA_COLOR_GREEN_BUILD_HIGH_MASK;
    payload[ONA_COLOR_GREEN_LOW_IDX + ONA_MSG_DATA_IDX]
      = (GREEN << ONA_COLOR_GREEN_LOW_SHIFT) & ONA_COLOR_GREEN_BUILD_LOW_MASK;
    payload[ONA_COLOR_BLUE_IDX + ONA_MSG_DATA_IDX]
      |= BLUE & ONA_COLOR_BLUE_MASK;

    // send payload
    return (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);
} // ona_send_color_command //


/*!
    \brief Send a color query msg

    \param[in] SRC_UNIT, the source unit for color msg
    \param[in] DST_UNIT, the destination unit for color msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_color_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_COLOR;
    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    payload[ONA_COLOR_UNIT_IDX] = SRC_UNIT;

    // send payload
    return (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);
} // ona_send_color_query //


/*!
    \brief parse a color msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] unit, the unit (src or dst)
    \param[out] red, the red color value
    \param[out] green, the green color value
    \param[out] blue, the blue color value

    \return the status of the send action
*/
one_net_status_t ona_parse_color(const UInt8 * const MSG_DATA, const UInt8 LEN,
  UInt8 * unit, UInt8 * const red, UInt8 * const green, UInt8 * const blue)
{
    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        return ONS_BAD_PARAM;
    }

    // get unit
    *unit = MSG_DATA[ONA_COLOR_UNIT_IDX];
    *red = (MSG_DATA[ONA_COLOR_RED_IDX] >> ONA_COLOR_RED_SHIFT)
      & ONA_COLOR_RED_PARSE_MASK;
    *green = ((MSG_DATA[ONA_COLOR_GREEN_HIGH_IDX] << ONA_COLOR_GREEN_HIGH_SHIFT)
      & ONA_COLOR_GREEN_PARSE_HIGH_MASK) | ((MSG_DATA[ONA_COLOR_GREEN_LOW_IDX]
      >> ONA_COLOR_GREEN_LOW_SHIFT) & ONA_COLOR_GREEN_PARSE_LOW_MASK);
    *blue = MSG_DATA[ONA_COLOR_BLUE_IDX] & ONA_COLOR_BLUE_MASK;

    return ONS_SUCCESS;
} // ona_parse_color //

//! @} ONE-NET_APP_color_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_color_pri_func
//! \ingroup ONE-NET_APP_color
//! @{

//! @} ONE-NET_APP_color_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_color

