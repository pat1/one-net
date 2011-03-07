//! \addtogroup ONE-NET_APP_volt
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
    \file ona_voltage.c
    \brief Implementation of voltage messaging functions.

    This is the implementation of functions to send voltage messages.

    10/30/08: Roger is changing ona_parse_voltage and ona_send_voltage to
    use the get_* and put_* functions in one_net_application.h to access
    fields in the payload of the ONA_VOLTAGE_VOLTS, ONA_VOLTAGE_10THS_VOLTS,
    and ONA_VOLTAGE_100THS_VOLTS message types.
*/

#include "ona_voltage.h"

#include "ona_voltage_simple.h"
#include "one_net_port_specific.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_volt_const
//! \ingroup ONE-NET_APP_volt
//! @{

enum
{
    //! The maximum value a voltage reading can be
    ONA_VOLTAGE_MAX = 2047
};

enum
{
    //! byte index into msg data section for the simple voltage status
    ONA_VOLTAGE_SIMPLE_IDX = 0,

    //! byte index into msg data section for the voltage source
    ONA_VOLTAGE_SRC_IDX = 0,

    //! byte index into msg data section for the high portion of the battery
    //! status
    ONA_VOLTAGE_BATTERY_HIGH_IDX = 0,

    //! byte index into msg data section for the low portion of the battery
    //! status
    ONA_VOLTAGE_BATTERY_LOW_IDX = 1,

    //! byte index into msg data section for the high portion of the external
    //! voltage status
    ONA_VOLTAGE_EXTERNAL_HIGH_IDX = 1,

    //! byte index into msg data section for the low portion of the external
    //! voltage status
    ONA_VOLTAGE_EXTERNAL_LOW_IDX = 2,

    //! Mask for the voltage source
    ONA_VOLTAGE_SRC_MASK = 0xC0,

    //! shift to use on high portion of the battery status
    ONA_VOLTAGE_BATTERY_HIGH_SHIFT = 5,

    //! shift to use on the low portion of battery status
    ONA_VOLTAGE_BATTERY_LOW_SHIFT = 3,

    //! Mask to use on high portion of battery status when building the msg.
    ONA_VOLTAGE_BATTERY_BUILD_HIGH_MASK = 0x3F,

    //! Mask to use on low portion of battery status when building the msg.
    ONA_VOLTAGE_BATTERY_BUILD_LOW_MASK = 0xF8,

    //! Mask to use on high portion of battery status when parsing the msg.
    ONA_VOLTAGE_BATTERY_PARSE_HIGH_MASK = 0x07E0,

    //! Mask to use on low portion of battery status when parsing the msg.
    ONA_VOLTAGE_BATTERY_PARSE_LOW_MASK = 0x001F,

    //! shift to use on high portion of the external voltage status
    ONA_VOLTAGE_EXTERNAL_HIGH_SHIFT = 8,

    //! Mask to use on high portion of external voltage status when building the
    //! msg.
    ONA_VOLTAGE_EXTERNAL_BUILD_HIGH_MASK = 0x07,

    //! Mask to use on low portion of external voltage status when building the
    //! msg.
    ONA_VOLTAGE_EXTERNAL_BUILD_LOW_MASK = 0xFF,

    //! Mask to use on high portion of external voltage status when parsing the
    //! msg.
    ONA_VOLTAGE_EXTERNAL_PARSE_HIGH_MASK = 0x0700,

    //! Mask to use on low portion of external voltage status when parsing the
    //! msg.
    ONA_VOLTAGE_EXTERNAL_PARSE_LOW_MASK = 0x00FF
};

//! @} ONE-NET_APP_volt_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_volt_typedefs
//! \ingroup ONE-NET_APP_volt
//! @{

//! @} ONE-NET_APP_volt_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_volt_pri_var
//! \ingroup ONE-NET_APP_volt
//! @{

//! @} ONE-NET_APP_volt_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_volt_pri_func
//! \ingroup ONE-NET_APP_volt
//! @{

static one_net_status_t ona_send_voltage(UInt8 TYPE,
  UInt8 VOLTAGE_SRC, UInt16 BATTERY_VOLTAGE,
  UInt16 EXTERNAL_VOLTAGE, const one_net_raw_did_t * RAW_DST);

static one_net_status_t ona_parse_voltage(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage);

//! @} ONE-NET_APP_volt_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_volt_pub_func
//! \ingroup ONE-NET_APP_volt
//! @{

/*!
    \brief Send a voltage (in volts) status msg

    \param[in] VOLTAGE_SRC, The voltage status being sent.  Can be external
      voltage, battery voltage, or both
    \param[in] BATTERY_VOLTAGE, The battery status (in volts)
    \param[in] EXTERNAL_VOLTAGE, The external voltage status (in volts)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_voltage_volts_status(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST)
{
    return ona_send_voltage(ONA_VOLTAGE_VOLTS, VOLTAGE_SRC, BATTERY_VOLTAGE,
      EXTERNAL_VOLTAGE, RAW_DST);
} // ona_send_voltage_volts_status //


/*!
    \brief Send a voltage (in 10ths of a volt) status msg

    \param[in] VOLTAGE_SRC, The voltage status being sent.  Can be external
      voltage, battery voltage, or both
    \param[in] BATTERY_VOLTAGE, The battery status (in 10ths of a volt)
    \param[in] EXTERNAL_VOLTAGE, The external voltage status (in 10ths of a
      volt)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_voltage_10ths_volts(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST)
{
    return ona_send_voltage(ONA_VOLTAGE_10THS_VOLTS, VOLTAGE_SRC,
      BATTERY_VOLTAGE, EXTERNAL_VOLTAGE, RAW_DST);
} // ona_send_voltage_10ths_volts //


/*!
    \brief Send a voltage (in 100ths of a volt) status msg

    \param[in] VOLTAGE_SRC, The voltage status being sent.  Can be external
      voltage, battery voltage, or both
    \param[in] BATTERY_VOLTAGE, The battery status (in 100ths of a volt)
    \param[in] EXTERNAL_VOLTAGE, The external voltage status (in 100ths of a
      volt)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_voltage_100ths_volts(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST)
{
    return ona_send_voltage(ONA_VOLTAGE_100THS_VOLTS, VOLTAGE_SRC,
      BATTERY_VOLTAGE, EXTERNAL_VOLTAGE, RAW_DST);
} // ona_send_voltage_100ths_volts //


/*!
    \brief Queries a device for it's voltage status.

    \param[in] RAW_DST, The destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_voltage_query(const one_net_raw_did_t * RAW_DST)
{
    const UInt16 CLASS_TYPE = ONA_QUERY | ONA_VOLTAGE;

    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
} // ona_send_voltage_query //


/*!
    \brief parse a voltage simple message

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] voltage_status, The sending devices voltage status.  This can be
      either ONA_VOLTAGE_GOOD, or ONA_VOLTAGE_BAD (see voltage_simple_status_t
      in voltage_simple.h).

    \return the status of the send action
*/
one_net_status_t ona_parse_voltage_simple(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_status)
{
    if(!MSG_DATA || LEN != ONA_MSG_DATA_LEN || !voltage_status)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    *voltage_status = MSG_DATA[ONA_VOLTAGE_SIMPLE_IDX];

    if(*voltage_status != ONA_VOLTAGE_GOOD
      && *voltage_status != ONA_VOLTAGE_BAD)
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //

    return ONS_SUCCESS;
} // ona_parse_voltage_simple //


/*!
    \brief parse a voltage volts msg

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] voltage_src, the voltage source being reported
    \param[out] battery_voltage, the batter voltage in volts
    \param[out] external_voltage, the external voltage in volts

    \return the status of parsing the msg
*/
one_net_status_t ona_parse_voltage_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage)
{
    return ona_parse_voltage(MSG_DATA, LEN, voltage_src, battery_voltage,
      external_voltage);
} // ona_parse_voltage_volts //


/*!
    \brief parse a voltage 10ths of a volt msg

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] voltage_src, the voltage source being reported
    \param[out] battery_voltage, the batter voltage in 10ths of a volt
    \param[out] external_voltage, the external voltage in 10ths of a volt

    \return the status of parsing the msg
*/
one_net_status_t ona_parse_voltage_10ths_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage)
{
    return ona_parse_voltage(MSG_DATA, LEN, voltage_src, battery_voltage,
      external_voltage);
} // ona_parse_voltage_10ths_volts //


/*!
    \brief parse a voltage 100ths of a volt msg

    \param[in] MSG_DATA, message data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] voltage_src, the voltage source being reported
    \param[out] battery_voltage, the batter voltage in 100ths of a volt
    \param[out] external_voltage, the external voltage in 100ths of a volt

    \return the status of parsing the msg
*/
one_net_status_t ona_parse_voltage_100ths_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage)
{
    return ona_parse_voltage(MSG_DATA, LEN, voltage_src, battery_voltage,
      external_voltage);
} // ona_parse_voltage_100ths_volts //

//! @} ONE-NET_APP_volt_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_volt_pri_func
//! \ingroup ONE-NET_APP_volt
//! @{

/*!
    \brief Send a voltage status msg

    \param[in] TYPE, The type of voltage status message being sent.  Must be one
      of ONA_VOLTAGE_VOLTS, ONA_VOLTAGE_10THS_VOLTS, or
      ONA_VOLTAGE_100THS_VOLTS.
    \param[in] VOLTAGE_SRC, The voltage status being sent.  Can be external
      voltage, battery voltage, or both
    \param[in] BATTERY_VOLTAGE, The battery status
    \param[in] EXTERNAL_VOLTAGE, The external voltage status
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
static one_net_status_t ona_send_voltage(UInt8 TYPE,
  UInt8 VOLTAGE_SRC, UInt16 BATTERY_VOLTAGE,
  UInt16 EXTERNAL_VOLTAGE, const one_net_raw_did_t * RAW_DST)
{
    const UInt16 CLASS_TYPE = ONA_STATUS | TYPE;

    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    UInt8 voltage_data[ONA_MSG_NUM_BYTES];

    if((TYPE != ONA_VOLTAGE_VOLTS && TYPE != ONA_VOLTAGE_10THS_VOLTS
      && TYPE != ONA_VOLTAGE_100THS_VOLTS)
      || VOLTAGE_SRC & ~(ONA_VOLTAGE_EXT_BAT_STATUS)
      || BATTERY_VOLTAGE > ONA_VOLTAGE_MAX
      || EXTERNAL_VOLTAGE > ONA_VOLTAGE_MAX
      || !RAW_DST)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    put_msg_hdr(CLASS_TYPE, payload);

    //
    // concatenate the three voltage fields in a three byte buffer
    // 2 bits voltage source, 11 bits battery voltage,
    // 11 bits external voltage
    //

    // first byte contains 2 bits voltage source and high order 6 bits
    // of battery voltage
    voltage_data[0] = VOLTAGE_SRC & ONA_VOLTAGE_SRC_MASK;
    voltage_data[0] |= (((UInt8)(BATTERY_VOLTAGE >> ONA_VOLTAGE_BATTERY_HIGH_SHIFT))
      & ONA_VOLTAGE_BATTERY_BUILD_HIGH_MASK);

    // second byte contains low order 5 bits of battery voltage and high
    // order 3 bits of external voltage
    voltage_data[1] = ((UInt8)(BATTERY_VOLTAGE << ONA_VOLTAGE_BATTERY_LOW_SHIFT))
      & ONA_VOLTAGE_BATTERY_BUILD_LOW_MASK;
    voltage_data[1] |= (((UInt8)(EXTERNAL_VOLTAGE >> ONA_VOLTAGE_EXTERNAL_HIGH_SHIFT))
      & ONA_VOLTAGE_EXTERNAL_BUILD_HIGH_MASK);

    // third byte contains low order 8 bits of external voltage
    voltage_data[2] = (UInt8)EXTERNAL_VOLTAGE & ONA_VOLTAGE_EXTERNAL_BUILD_LOW_MASK;

    // store the concatenated voltage fields in the payload
    put_three_message_bytes_to_payload(voltage_data, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
} // ona_send_voltage //


/*!
    \brief parse a voltage msg

    \param[in] PAYLOAD, pointer to the received payload
    \param[in] LEN, the length of the single packet payload
    \param[out] voltage_src, the voltage source being reported
    \param[out] battery_voltage, the batter voltage
    \param[out] external_voltage, the external voltage

    \return the status of parsing the msg
*/
static one_net_status_t ona_parse_voltage(const UInt8 * PAYLOAD,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage)
{
    UInt8 msg_byte;

    if(!PAYLOAD || LEN != ONA_SINGLE_PACKET_PAYLOAD_LEN || !voltage_src || !battery_voltage
      || !external_voltage)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // parse voltage source
    msg_byte = get_first_msg_byte(PAYLOAD);
    *voltage_src = (msg_byte & ONA_VOLTAGE_SRC_MASK);

    // parse battery voltage
    *battery_voltage = ((UInt16) (msg_byte & ONA_VOLTAGE_BATTERY_BUILD_HIGH_MASK))
      << ONA_VOLTAGE_BATTERY_HIGH_SHIFT;
    msg_byte = get_second_msg_byte(PAYLOAD);
    *battery_voltage |=
      (UInt16) ((msg_byte >> ONA_VOLTAGE_BATTERY_LOW_SHIFT)) & ONA_VOLTAGE_BATTERY_PARSE_LOW_MASK;

    // parse external voltage
    *external_voltage = ((UInt16) (msg_byte) & ONA_VOLTAGE_EXTERNAL_BUILD_HIGH_MASK)
      << ONA_VOLTAGE_EXTERNAL_HIGH_SHIFT;
    msg_byte = get_third_msg_byte(PAYLOAD);
    *external_voltage |= ((UInt16) msg_byte) & ONA_VOLTAGE_EXTERNAL_PARSE_LOW_MASK;

    if(*battery_voltage > ONA_VOLTAGE_MAX
      || *external_voltage > ONA_VOLTAGE_MAX)
    {
        return ONS_INVALID_DATA;
    } // if the data is invalid //

    return ONS_SUCCESS;
} // ona_parse_voltage //

//! @} ONE-NET_APP_volt_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_volt

