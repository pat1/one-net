//! \addtogroup ONE-NET_APP_temperature
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
    \file ona_temperature.c
    \brief Implementation of temperature msg functions.

    This is the implementation of functions to send an parse temperature msgs.
*/

#include "ona_temperature.h"

#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_temperature_const
//! \ingroup ONE-NET_APP_temperature
//! @{

//! @} ONE-NET_APP_temperature_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_temperature_typedefs
//! \ingroup ONE-NET_APP_temperature
//! @{

//! @} ONE-NET_APP_temperature_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_temperature_pri_var
//! \ingroup ONE-NET_APP_temperature
//! @{

//! @} ONE-NET_APP_temperature_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_temperature_pri_func
//! \ingroup ONE-NET_APP_temperature
//! @{

//! @} ONE-NET_APP_temperature_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_temperature_pub_func
//! \ingroup ONE-NET_APP_temperature
//! @{

/*!
    \brief Send a temperature status msg

    \param[in] SRC_UNIT, the source unit of temperature msg
    \param[in] DST_UNIT, the destination unit for temperature msg
    \param[in] TEMPERATURE, temperature status (in 20ths of a degree Kelvin)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_temperature_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt16 TEMPERATURE,
  const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    tick_t time_from_now = 0;

    put_msg_hdr(ONA_STATUS | ONA_TEMPERATURE, payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    put_msg_data(TEMPERATURE, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      FALSE, ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT, &time_from_now, &time_from_now);
} // ona_send_temperature_status //


/*!
    \brief Send a temperature command msg

    \param[in] SRC_UNIT, the source unit of temperature msg
    \param[in] DST_UNIT, the destination unit for temperature msg
    \param[in] TEMPERATURE, temperature status (in 20ths of a degree Kelvin)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_temperature_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt16 TEMPERATURE,
  const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    tick_t time_from_now = 0;

    put_msg_hdr(ONA_COMMAND | ONA_TEMPERATURE, payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    put_msg_data(TEMPERATURE, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      FALSE, ONE_NET_HIGH_PRIORITY, RAW_DST, SRC_UNIT, &time_from_now,
      &time_from_now);
} // ona_send_temperature_command //


/*!
    \brief Send a temperature query msg

    \param[in] SRC_UNIT, the source unit for temperature msg
    \param[in] DST_UNIT, the destination unit for temperature msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_temperature_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    tick_t time_from_now = 0;

    put_msg_hdr(ONA_QUERY | ONA_TEMPERATURE, payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload), FALSE,
      ONE_NET_HIGH_PRIORITY, RAW_DST, SRC_UNIT,&time_from_now, &time_from_now);
} // ona_send_temperature_query //


/*!
    \brief parse a temperature msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] unit, the unit (src or dst)
    \param[out] temp, the temperature

    \return the status of the send action
*/
one_net_status_t ona_parse_temperature(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * unit, UInt16 * temperature)
{
    // error checking
    if(!MSG_DATA || !unit || !temperature || LEN != ONA_MSG_DATA_LEN)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // TODO - this should be changed.  We need the destination unit too?
    *unit = get_src_unit(MSG_DATA);

    // get temperature
    *temperature = get_msg_data(MSG_DATA);

    return ONS_SUCCESS;
} // ona_parse_temperature //

//! @} ONE-NET_APP_temperature_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_temperature_pri_func
//! \ingroup ONE-NET_APP_temperature
//! @{

//! @} ONE-NET_APP_temperature_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_temperature

