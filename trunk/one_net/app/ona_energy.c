//! \addtogroup ONE-NET_APP_energy
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
    \file ona_energy.c
    \brief Implementation of energy messaging functions.

    This is the implementation of functions to send energy messages.
*/

#include <one_net/app/ona_energy.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_energy_const
//! \ingroup ONE-NET_APP_energy
//! @{

//! @} ONE-NET_APP_energy_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_energy_typedefs
//! \ingroup ONE-NET_APP_energy
//! @{

//! @} ONE-NET_APP_energy_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_energy_pri_var
//! \ingroup ONE-NET_APP_energy
//! @{

//! @} ONE-NET_APP_energy_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_energy_pri_func
//! \ingroup ONE-NET_APP_energy
//! @{

#if 0 // 4/29/08: Energy messages will be changing, we need the program memory now,
      // so I am #if'g the old functions out
static one_net_status_t ona_send_energy(const UInt16 CLASS, const UInt8 TYPE,
  const UInt8 SRC_UNIT, const UInt8 DST_UNIT, const UInt8 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST);
#endif

static UInt32 convert_to_watt_seconds(const ona_msg_class_t MSG_TYPE, UInt16 raw_energy);

static one_net_status_t ona_send_energy(const UInt16 CLASS, const ona_msg_type_t TYPE,
  const UInt8 SRC_UNIT, const UInt8 DST_UNIT, const UInt16 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST);

//! @} ONE-NET_APP_energy_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_energy_pub_func
//! \ingroup ONE-NET_APP_energy
//! @{

#if 0 // 4/29/08: Energy messages will be changing, we need the program memory now,
      // so I am #if'g the old functions out
/*!
    \brief Send an energy (in watt minutes) status msg

    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in watt minutes)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_watt_minutes_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    return ona_send_energy(ONA_STATUS, ONA_ENERGY_WATT_MINUTES, SRC_UNIT,
      DST_UNIT, ENERGY_STATUS, RAW_DST);
} // ona_send_energy_watt_minutes_status //


/*!
    \brief Send an energy (in watt minutes) command msg

    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in watt minutes)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_watt_minutes_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    return ona_send_energy(ONA_COMMAND, ONA_ENERGY_WATT_MINUTES, SRC_UNIT,
      DST_UNIT, ENERGY_STATUS, RAW_DST);
} // ona_send_energy_watt_minutes_command //


/*!
    \brief Send an energy (in kilowatt minutes) status msg

    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in kilowatt minutes)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_kilowatt_minutes_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    return ona_send_energy(ONA_STATUS, ONA_ENERGY_KILOWATT_MINUTES, SRC_UNIT,
      DST_UNIT, ENERGY_STATUS, RAW_DST);
} // ona_send_energy_kilowatt_minutes_status //


/*!
    \brief Send an energy (in kilowatt minutes) command msg

    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in kilowatt minutes)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_kilowatt_minutes_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    return ona_send_energy(ONA_COMMAND, ONA_ENERGY_KILOWATT_MINUTES, SRC_UNIT,
      DST_UNIT, ENERGY_STATUS, RAW_DST);
} // ona_send_energy_kilowatt_minutes_command //


/*!
    \brief Queries a device for it's energy status.

    \param[in] RAW_DST, The destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_query(const one_net_raw_did_t * const RAW_DST)
{
    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_ENERGY;

    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    // send payload
    return (*ona_net_send_single)(payload, sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);
} // ona_send_energy_query //
#endif


/*!
    \brief parse an energy msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] MSG_TYPE, messgae type of the received message
    \param[in] LEN, the length of the msg data
    \param[out] src_unit, the source unit of the msg
    \param[out] dst_unit, the destination unit of the msg
    \param[out] incremental_energy, energy status of unit

    \return the status of the parse action
*/
one_net_status_t ona_parse_energy(
  UInt8 * MSG_DATA,
  const ona_msg_type_t MSG_TYPE,
  const UInt8 LEN,
  UInt8 * src_unit,
  UInt8 * dst_unit,
  UInt32 * incremental_energy)
{

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        return ONS_BAD_PARAM;
    }

    // get units
    *src_unit = get_src_unit(MSG_DATA);
    *dst_unit = get_dst_unit(MSG_DATA);

    // convert the 16 bit value in the payload to a 32 bit value in watt-seconds
    *incremental_energy = convert_to_watt_seconds(MSG_TYPE, get_msg_data(MSG_DATA));

    return ONS_SUCCESS;
} // ona_parse_energy //

/*!
    \brief Send an energy (in 2 watt seconds) status msg

    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in 2 watt seconds)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_energy_2_watt_seconds_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt16 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    return ona_send_energy(ONA_STATUS, ONA_ENERGY_2_WATT_SECONDS, SRC_UNIT,
      DST_UNIT, ENERGY_STATUS, RAW_DST);
} // ona_send_energy_2_watt_seconds_status //


//! @} ONE-NET_APP_energy_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_energy_pri_func
//! \ingroup ONE-NET_APP_energy
//! @{

/*!
    \brief Send an incremental energy message.

    Send an incremental energy message to the master. This function needs to
    be provided the type of energy message so the receiver will know how
    to interpret the 16 bit value contained in the message. See convert_to_watt_seconds
    for a better understanding of how to interpret the 16 bit value based on
    the incremental energy message type.

    \param[in] CLASS, The class of message being sent.  Must be either
      ONA_COMMAND or ONA_STATUS
    \param[in] TYPE, The type of energy message being sent.  Must be either
      ONA_ENERGY_WATT_MINUTES or ONA_ENERGY_KILOWATT_MINUTES
    \param[in] SRC_UNIT, the source unit for energy msg
    \param[in] DST_UNIT, the destination unit for energy msg
    \param[in] ENERGY_STATUS, the energy reading (in a multiple of watt seconds )
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
static one_net_status_t ona_send_energy(const UInt16 CLASS, const ona_msg_type_t TYPE,
  const UInt8 SRC_UNIT, const UInt8 DST_UNIT, const UInt16 ENERGY_STATUS,
  const one_net_raw_did_t * const RAW_DST)
{
    const UInt16 CLASS_TYPE = CLASS | TYPE;

    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    if(((CLASS != ONA_COMMAND) && (CLASS != ONA_STATUS)) ||
      ((TYPE != ONA_ENERGY_2_WATT_SECONDS) &&
      (TYPE != ONA_ENERGY_20_WATT_SECONDS)) )
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // set the message class and type fields in the payload
    put_msg_hdr(CLASS_TYPE, payload);

    // set source and destination unit numbers
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);

    // set the energy reading value
    put_msg_data(ENERGY_STATUS, payload);

    // send a single packet with this payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONA_ENERGY_DST_UNIT_IDX + ONA_MSG_DATA_IDX,
      ONE_NET_LOW_PRIORITY, RAW_DST, SRC_UNIT);
} // ona_send_energy //

/*!
 *     \brief Convert a 16 bit incremental energy value to a 32 bit incremental
 *     energy value in watt-seconds.
 *
 *     Using the message type and a 16 bit incremental energy value calculate the
 *     a 32 bit incremental energy value. The message type tells us how many
 *     watt-seconds are represented by the least significant bit in the 16 bit
 *     value in the message payload.
 *
 *     \param[in] msg_type One of the incremental energy message types.
 *     \param[in] payload Pointer to the position of the 16 bit value within the payload.
 *     \return The number of watt-seconds represented by the 16 bit incremental energy value provided.
 */
static UInt32 convert_to_watt_seconds(const ona_msg_class_t MSG_TYPE, UInt16 raw_energy)
{
    UInt32 watt_seconds;

    watt_seconds = (UInt32) raw_energy;
    switch (MSG_TYPE)
    {
        case ONA_ENERGY_2_WATT_SECONDS:
            watt_seconds *= 2;
            break;

        case ONA_ENERGY_20_WATT_SECONDS:
            watt_seconds *= 20;
            break;

        case ONA_ENERGY_200_WATT_SECONDS:
            watt_seconds *= 200;
            break;

        case ONA_ENERGY_2000_WATT_SECONDS:
            watt_seconds *= 2000;
            break;

        case ONA_ENERGY_20000_WATT_SECONDS:
            watt_seconds *= 20000;
            break;

       default:
#if !defined(_R8C_TINY)
            TLOG("ona_energy", LOG_ERR, "ERROR: %s: unrecognized MSG_TYPE of [0x%04x], "
              "watt_seconds=%d", __FUNCTION__, MSG_TYPE, watt_seconds);
#endif
            em(EM_BAD_INCR_ENERGY_MSG_TYPE);
            watt_seconds = 0;
    }

    return(watt_seconds);
} // convert_to_watt_seconds //
//! @} ONE-NET_APP_energy_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_energy

