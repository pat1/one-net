#ifndef _ONE_NET_APPLICATION_H
#define _ONE_NET_APPLICATION_H

#include <one_net/port_specific/config_options.h>


//! \defgroup ONE-NET_APP ONE-NET Application Layer
//! \ingroup ONE-NET
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
    \file one_net_application.h
    \brief Declarations for ONE-NET application layer

    This is global ONE-NET application layer information.  
*/

#include <one_net/one_net.h>
#include <one_net/one_net_types.h>
#include <one_net/one_net_status_codes.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_const
//! \ingroup ONE-NET_APP
//! @{

/* Changes for new unit and message data fields */
enum
{
    //! Index of header within single packet payload (header is message
    //! class and message type
    //dje: 4-bit src/destination addresses are now first in the payload
    ONA_MSG_FIRST_IDX      = 0,
    ONA_MSG_SECOND_IDX     = 3,
    ONA_MSG_THIRD_IDX      = 4,

    ONA_MSG_SRC_UNIT_IDX   = ONA_MSG_FIRST_IDX, // Where the byte is
    ONA_MSG_SRC_UNIT_MASK  = 0xf0,  // Where the bits are in the byte
    ONA_MSG_SRC_UNIT_SHIFT = 4,     // Shift left this much to put them in

    ONA_MSG_DST_UNIT_IDX   = ONA_MSG_FIRST_IDX,     // Where the byte is
    ONA_MSG_DST_UNIT_MASK  = 0x0f,  // Where the bits are in the byte
    ONA_MSG_DST_UNIT_SHIFT = 0,     // Shift left this much to put them in

    // Header now follows src/dst addresses
    ONA_MSG_HDR_IDX = 1,

    //! Length of the header within single packet payload
    ONA_MSG_HDR_LEN = 2,

    //! Index of Message Data within payload
    ONA_MSG_DATA_IDX = ONA_MSG_SECOND_IDX,

    //! Length of Message Data
    ONA_MSG_DATA_LEN = 2,

    ONA_MSG_NUM_BYTES = 3, // three of the five bites are msg stuff

    ONA_SINGLE_PACKET_PAYLOAD_LEN = 5 // 1 + msg_hdr_len + msg_data_len

};

ONE_NET_INLINE void get_three_message_bytes_from_payload(UInt8 *msg, const UInt8 *payload)
{
    *(msg++) = payload[ONA_MSG_FIRST_IDX];
    *(msg++) = payload[ONA_MSG_SECOND_IDX];
    *msg     = payload[ONA_MSG_THIRD_IDX];
}

ONE_NET_INLINE void put_three_message_bytes_to_payload(const UInt8 *msg, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX]  = *(msg++);
    payload[ONA_MSG_SECOND_IDX] = *(msg++);
    payload[ONA_MSG_THIRD_IDX]  = *msg;
}


ONE_NET_INLINE UInt8 get_first_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_FIRST_IDX];
}

ONE_NET_INLINE void put_first_msg_byte(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX] = data;
}

ONE_NET_INLINE UInt8 get_second_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_SECOND_IDX];
}

ONE_NET_INLINE void put_second_msg_byte(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_SECOND_IDX] = data;
}

ONE_NET_INLINE UInt8 get_third_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_THIRD_IDX];
}

ONE_NET_INLINE void put_third_msg_byte(const UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_THIRD_IDX] = data;
}


ONE_NET_INLINE UInt16 get_first_two_msg_bytes(const UInt8 *payload)
{
    return (((UInt16)payload[ONA_MSG_FIRST_IDX]<< 8) |
             (UInt16)payload[ONA_MSG_SECOND_IDX]);
}

ONE_NET_INLINE void put_first_two_msg_bytes(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX]  = data >> 8;
    payload[ONA_MSG_SECOND_IDX] = data;
}

/* get the 16-bit message header (message class, message type) */
ONE_NET_INLINE UInt16 get_msg_hdr(const UInt8 *payload)
{
    return ((UInt16)payload[ONA_MSG_HDR_IDX]<< 8) | 
            (UInt16)payload[ONA_MSG_HDR_IDX+1];
}

ONE_NET_INLINE void put_msg_hdr(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_HDR_IDX]   = data >> 8;
    payload[ONA_MSG_HDR_IDX+1] = data;
}


/* get the 16-bit message data from the payload buffer */
ONE_NET_INLINE UInt16 get_msg_data(const UInt8 *payload)
{
    return ((UInt16)payload[ONA_MSG_SECOND_IDX] << 8) |
            (UInt16)payload[ONA_MSG_THIRD_IDX];
}

/* store the 16-bit message data in the payload buffer
 * Use platform-dependent function in one_net_port_specific.c
 */
ONE_NET_INLINE void put_msg_data(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_SECOND_IDX] = data >> 8;
    payload[ONA_MSG_THIRD_IDX]  = data;
}

/* get the 8-bit source unit data value from the payload buffer */
ONE_NET_INLINE UInt8 get_src_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_SRC_UNIT_IDX] & ONA_MSG_SRC_UNIT_MASK) >> 
            ONA_MSG_SRC_UNIT_SHIFT;
}

/* store the 8-bit source unit data value in the payload buffer */
ONE_NET_INLINE void put_src_unit(UInt8 data , UInt8 *payload)
{
    payload[ONA_MSG_SRC_UNIT_IDX] = 
        (payload[ONA_MSG_SRC_UNIT_IDX]    & ~ONA_MSG_SRC_UNIT_MASK) |
        ((data << ONA_MSG_SRC_UNIT_SHIFT) &  ONA_MSG_SRC_UNIT_MASK);
}

/* get the 8-bit destination unit data value from the payload buffer */
ONE_NET_INLINE UInt8 get_dst_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_DST_UNIT_IDX] & ONA_MSG_DST_UNIT_MASK) >> 
            ONA_MSG_DST_UNIT_SHIFT;
}

/* store the 8-bit destination unit data value in the payload buffer */
ONE_NET_INLINE void put_dst_unit(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_DST_UNIT_IDX] = 
        (payload[ONA_MSG_DST_UNIT_IDX]    & ~ONA_MSG_DST_UNIT_MASK) |
        ((data << ONA_MSG_DST_UNIT_SHIFT) &  ONA_MSG_DST_UNIT_MASK);
}

//
// Block Data Payload Header Definitions
//

enum
{
    //! Index of the message type field within a Block Data payload header 
    ONA_BLK_DATA_HDR_MSG_TYPE_IDX = 0,

    //! Index of the block length field within a Block Data payload header 
    ONA_BLK_DATA_HDR_BLK_LEN_IDX = 2,

    //! Index of the source and destination unit fields within a Block Data payload header 
    ONA_BLK_DATA_HDR_SRC_DST_IDX = 4,

    //! Index of the data portion of the payload in the first block data message
    ONA_BLK_DATA_HDR_DATA_IDX = 5,

    //! Length of the data portion of the payload in the first block data message
    ONA_BLK_DATA_HDR_DATA_LEN = 5,

    //! Mask for source unit field within the byte that holds source and destination units
    ONA_BLK_DATA_HDR_SRC_UNIT_MASK = 0xf0,

    //! Mask for destination unit field within the byte that holds source and destination units
    ONA_BLK_DATA_HDR_DST_UNIT_MASK = 0x0f,

    //! Number of bits to shift the source unit to the right so that it can be used as a byte value
    ONA_BLK_DATA_HDR_SRC_UNIT_SHIFT = 4

};

ONE_NET_INLINE void put_block_data_payload_hdr(UInt16 msg_type, UInt16 block_len,
  UInt8 src_unit, UInt8 dst_unit, UInt8 *payload)
{
    payload[ONA_BLK_DATA_HDR_MSG_TYPE_IDX] = msg_type >> 8;
    payload[ONA_BLK_DATA_HDR_MSG_TYPE_IDX+1] = msg_type;

    payload[ONA_BLK_DATA_HDR_BLK_LEN_IDX] = block_len >> 8;
    payload[ONA_BLK_DATA_HDR_BLK_LEN_IDX+1] = block_len;

    payload[ONA_BLK_DATA_HDR_SRC_DST_IDX] = (src_unit << ONA_BLK_DATA_HDR_SRC_UNIT_SHIFT) & 
      ONA_BLK_DATA_HDR_SRC_UNIT_MASK;

    payload[ONA_BLK_DATA_HDR_SRC_DST_IDX] |= dst_unit & ONA_BLK_DATA_HDR_DST_UNIT_MASK;
} //put_block_data_payload_hdr //

ONE_NET_INLINE void get_block_data_payload_hdr(UInt16 * msg_type, UInt16 * block_len,
  UInt8 * src_unit, UInt8 * dst_unit, UInt8 *payload)
{
    if (msg_type)
    {
        *msg_type = ((UInt16)payload[ONA_BLK_DATA_HDR_MSG_TYPE_IDX] << 8) | 
                (UInt16)payload[ONA_BLK_DATA_HDR_MSG_TYPE_IDX+1];
    }

    if (block_len)
    {
        *block_len = ((UInt16)payload[ONA_BLK_DATA_HDR_BLK_LEN_IDX] << 8) | 
                (UInt16)payload[ONA_BLK_DATA_HDR_BLK_LEN_IDX+1];
    }

    if (src_unit)
    {
        *src_unit = (payload[ONA_BLK_DATA_HDR_SRC_DST_IDX] & ONA_BLK_DATA_HDR_SRC_UNIT_MASK) >> 
                ONA_BLK_DATA_HDR_SRC_UNIT_SHIFT;
    }

    if (dst_unit)
    {
        *dst_unit = payload[ONA_BLK_DATA_HDR_SRC_DST_IDX] & ONA_BLK_DATA_HDR_DST_UNIT_MASK;
    }
} // get_block_data_payload_hdr //




//! @} ONE-NET_APP_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_typedefs
//! \ingroup ONE-NET_APP
//! @{
#ifndef _ONE_NET_VERSION_2_X
typedef enum _ona_msg_class
{
    ONA_STATUS   = 0x0000, //!< Status of a unit
    ONA_COMMAND  = 0x4000, //!< Command to change status of a unit
    ONA_QUERY    = 0x8000, //!< Query status of a unit
    ONA_NOT_USED = 0xC000, //!< Not used

    ONA_MSG_CLASS_MASK = 0xC000,    //!< Used to mask message class bits
} ona_msg_class_t;
#else
typedef enum _ona_msg_class
{
    ONA_STATUS   = 0x0000, //!< Status of a unit
    ONA_COMMAND  = 0x4000, //!< Command to change status of a unit
    ONA_QUERY    = 0x8000, //!< Query status of a unit
#ifdef _POLL
    ONA_POLL     = 0xC000, //!< Poll status of a unit
#endif

    ONA_MSG_CLASS_MASK = 0x8000,    //!< Used to mask message class bits
} ona_msg_class_t;
#endif


#ifndef _ONE_NET_VERSION_2_X
typedef enum _ona_msg_type
{
#ifdef _NEED_SWITCH_MESSAGE
    //! Set/Query/Report Switch Status
    ONA_SWITCH = 0x0000,
#endif

#ifdef _NEED_PERCENT_MESSAGE
    //! Set/Query/Report Percent Status
    ONA_PERCENT = 0x0001,
#endif

#ifdef _NEED_TEMPERATURE_MESSAGE
    //! Set/Query/Report Temperature Status
    ONA_TEMPERATURE = 0x0002,
#endif

#ifdef _NEED_HUMIDITY_MESSAGE
    //! Set/Query/Report Humidity Status
    ONA_HUMIDITY = 0x0003,
#endif

#ifdef _NEED_PRESSURE_MESSAGE
    //! Set/Query/Report Pressure Status
    ONA_PRESSURE = 0x0004,
#endif

#ifdef _NEED_RAINFALL_MESSAGE
    //! Set/Query/Report Rainfall Status
    ONA_RAINFALL = 0x0005,
#endif

#ifdef _NEED_SPEED_MESSAGE
    //! Set/Query/Report Speed Status
    ONA_SPEED_M_S = 0x0006,
#endif

#ifdef _NEED_DIRECTION_MESSAGE
    //! Set/Query/Report Direction Status
    ONA_DIRECTION = 0x0007,
#endif

    //! Query status interval
    ONA_STATUS_INTERVAL = 0x0008,

#ifdef _NEED_OPENING_MESSAGE
    //! Set/Query/Report Opening Status (called Access in the specification)
    ONA_OPENING = 0x0009,
#endif

#ifdef _NEED_SEAL_MESSAGE
    //! Set/Query/Report Seal Status
    ONA_SEAL = 0x000A,
#endif

    //! Unused message
    ONA_UNUSED0 = 0x000B,

    //! Set/Query/Report Unit Type Count 
    ONA_UNIT_TYPE_COUNT = 0x000C,

    //! Set/Query/Report Unit Type
    ONA_UNIT_TYPE = 0x000D,

#ifdef _NEED_COLOR_MESSAGE
    //! Set/Query/Report Color
    ONA_COLOR = 0x000E,
#endif

#ifdef _NEED_SIMPLE_TEXT_MESSAGE
    //! Send Simple Text (3 ASCII chars)
    ONA_SIMPLE_TEXT = 0x000F,
#endif

#ifdef _NEED_DATE_MESSAGE
    //! Query/Report Date
    ONA_DATE = 0x0010,
#endif

#ifdef _NEED_TIME_MESSAGE
    //! Query/Report Time
    ONA_TIME = 0x0011,
#endif

#ifdef _NEED_VOLTAGE_MESSAGE
    //! Query Voltage
    ONA_VOLTAGE = 0x0012,

    //! Report Voltage (in volts)
    ONA_VOLTAGE_VOLTS = 0x0013,

    //! Report Voltage (in 10ths of a volt) 
    ONA_VOLTAGE_10THS_VOLTS = 0x0014,

    //! Report Voltage (in 100ths of a volt)
    ONA_VOLTAGE_100THS_VOLTS = 0x0015,

    //! Report Voltage (in simple good/bad)
    ONA_VOLTAGE_SIMPLE = 0x0016,
#endif

#ifdef _NEED_ENERGY_MESSAGE
    //! Incremental Energy Query 
    ONA_ENERGY = 0x0017,

    // Replacing old values with new ones
    //! Report Energy (in watt minutes)
    // ONA_ENERGY_WATT_MINUTES = 0x001B,
    //! Report Energy (in kilowatt minutes)
    // ONA_ENERGY_KILOWATT_MINUTES = 0x001C,

    //! Report Incremental Energy (in 2 watt seconds units)
    ONA_ENERGY_2_WATT_SECONDS = 0x0018,

    //! Report Incremental Energy (in 20 watt seconds units)
    ONA_ENERGY_20_WATT_SECONDS = 0x0019,

    //! Report Incremental Energy (in 200 watt seconds units)
    ONA_ENERGY_200_WATT_SECONDS = 0x001A,

    //! Report Incremental Energy (in 2000 watt seconds units)
    ONA_ENERGY_2000_WATT_SECONDS = 0x001B,

    //! Report Incremental Energy (in 20000 watt seconds units)
    ONA_ENERGY_20000_WATT_SECONDS = 0x001C,
#endif

#ifdef _NEED_ACCUM_ENERGY_MESSAGE
    //! Accumulated Energy Query 
    ONA_ACCUM_ENERGY = 0x001D,

    //! Report Accumulated Energy (in 2 watt seconds units)
    ONA_ACCUM_ENERGY_2_WATT_SECONDS = 0x001E,

    //! Report Accumulated Energy (in 20 watt seconds units)
    ONA_ACCUM_ENERGY_20_WATT_SECONDS = 0x001F,

    //! Report Accumulated Energy (in 200 watt seconds units)
    ONA_ACCUM_ENERGY_200_WATT_SECONDS = 0x0020,

    //! Report Accumulated Energy (in 2000 watt seconds units)
    ONA_ACCUM_ENERGY_2000_WATT_SECONDS = 0x0021,

    //! Report Accumulated Energy (in 20000 watt seconds units)
    ONA_ACCUM_ENERGY_20000_WATT_SECONDS = 0x0022,
#endif

#ifdef _NEED_PEAK_ENERGY_MESSAGE
    //! Peak Energy Query 
    ONA_PEAK_ENERGY = 0x0023,

    //! Report Peak Energy (in 2 watt seconds units)
    ONA_PEAK_ENERGY_2_WATT_SECONDS = 0x0024,

    //! Report Peak Energy (in 20 watt seconds units)
    ONA_PEAK_ENERGY_20_WATT_SECONDS = 0x0025,

    //! Report Peak Energy (in 200 watt seconds units)
    ONA_PEAK_ENERGY_200_WATT_SECONDS = 0x0026,

    //! Report Peak Energy (in 2000 watt seconds units)
    ONA_PEAK_ENERGY_2000_WATT_SECONDS = 0x0027,

    //! Report Peak Energy (in 20000 watt seconds units)
    ONA_PEAK_ENERGY_20000_WATT_SECONDS = 0x0028,
#endif

#ifdef _NEED_GAS_MESSAGE
    //! Instantaneous Gas Query 
    ONA_GAS = 0x0029,

    //! Instantaneous Gas Therm-Seconds
    ONA_GAS_THERM_SECS = 0x002A,

    //! Instantaneous Gas Therm-Minutes
    ONA_GAS_THERM_MINS = 0x002B,

    //! Instantaneous Gas Therm-Hours
    ONA_GAS_THERM_HRS = 0x002C,
#endif

#ifdef _NEED_ACCUM_GAS_MESSAGE
    //! Accumulated Gas Query 
    ONA_ACCUM_GAS = 0x002D,

    //! Accumulated Gas Therm-Seconds
    ONA_ACCUM_GAS_THERM_SECS = 0x002E,

    //! Accumulated Gas Therm-Minutes
    ONA_ACCUM_GAS_THERM_MINS = 0x002F,

    //! Accumulated Gas Therm-Hours
    ONA_ACCUM_GAS_THERM_HRS = 0x0030,
#endif

#ifdef _NEED_AVERAGE_GAS_MESSAGE
    //! Average Gas Query 
    ONA_AVER_GAS = 0x0031,

    //! Average Gas Therm-Seconds
    ONA_AVER_GAS_THERM_SECS = 0x0032,

    //! Average Gas Therm-Minutes
    ONA_AVER_GAS_THERM_MINS = 0x0033,

    //! Average Gas Therm-Hours
    ONA_AVER_GAS_THERM_HRS = 0x0034,
#endif

#ifdef _NEED_PEAK_GAS_MESSAGE
    //! Peak Gas Query 
    ONA_PEAK_GAS = 0x0035,

    //! Peak Gas Therm-Seconds
    ONA_PEAK_GAS_THERM_SECS = 0x0036,

    //! Peak Gas Therm-Minutes
    ONA_PEAK_GAS_THERM_MINS = 0x0037,

    //! Peak Gas Therm-Hours
    ONA_PEAK_GAS_THERM_HRS = 0x0038,
#endif

#ifdef _NEED_INSTEON_MESSAGE
    //! Send INSTEON to address
    ONA_INSTEON_TO_ADDRESS = 0x3FFB,

    //! Send INSTEON standard command
    ONA_INSTEON_COMMAND = 0x3FFC,
#endif

#ifdef _NEED_X10_MESSAGE
    //! Send X10 simple msg
    ONA_X10_SIMPLE = 0x3FFD,

    //! Send an extended X10 msg
    ONA_X10_EXTENDED = 0x3FFE,
#endif

    //! Mask the message type bits
    ONA_MSG_TYPE_MASK = 0x3FFF
} ona_msg_type_t;
#else
typedef enum _ona_msg_type
{
#ifdef _NEED_SWITCH_MESSAGE
    //! Set/Query/Report Switch Status
    ONA_SWITCH = 0x000,
#endif

#ifdef _NEED_PERCENT_MESSAGE
    //! Set/Query/Report Percent Status
    ONA_PERCENT = 0x001,
#endif

#ifdef _NEED_TEMPERATURE_MESSAGE
    //! Set/Query/Report Temperature Status
    ONA_TEMPERATURE = 0x002,
#endif

#ifdef _NEED_HUMIDITY_MESSAGE
    //! Set/Query/Report Humidity Status
    ONA_HUMIDITY = 0x003,
#endif

#ifdef _NEED_PRESSURE_MESSAGE
    //! Set/Query/Report Pressure Status
    ONA_PRESSURE = 0x004,
#endif

#ifdef _NEED_RAINFALL_MESSAGE
    //! Set/Query/Report Rainfall Status
    ONA_RAINFALL = 0x005,
#endif

#ifdef _NEED_SPEED_MESSAGE
    //! Set/Query/Report Speed Status
    ONA_SPEED_M_S = 0x006,
#endif

#ifdef _NEED_DIRECTION_MESSAGE
    //! Set/Query/Report Direction Status
    ONA_DIRECTION = 0x007,
#endif

    //! Query status interval
    ONA_STATUS_INTERVAL = 0x008,

#ifdef _NEED_OPENING_MESSAGE
    //! Set/Query/Report Opening Status (called Access in the specification)
    ONA_OPENING = 0x009,
#endif

#ifdef _NEED_SEAL_MESSAGE
    //! Set/Query/Report Seal Status
    ONA_SEAL = 0x00A,
#endif

    //! Unused message
    ONA_UNUSED0 = 0x00B,

    //! Set/Query/Report Unit Type Count 
    ONA_UNIT_TYPE_COUNT = 0x00C,

    //! Set/Query/Report Unit Type
    ONA_UNIT_TYPE = 0x00D,

#ifdef _NEED_COLOR_MESSAGE
    //! Set/Query/Report Color
    ONA_COLOR = 0x00E,
#endif

#ifdef _NEED_SIMPLE_TEXT_MESSAGE
    //! Send Simple Text (3 ASCII chars)
    ONA_SIMPLE_TEXT = 0x00F,
#endif

#ifdef _NEED_DATE_MESSAGE
    //! Query/Report Date
    ONA_DATE = 0x010,
#endif

#ifdef _NEED_TIME_MESSAGE
    //! Query/Report Time
    ONA_TIME = 0x011,
#endif

#ifdef _NEED_VOLTAGE_MESSAGE
    //! Query Voltage
    ONA_VOLTAGE = 0x012,

    //! Report Voltage (in volts)
    ONA_VOLTAGE_VOLTS = 0x013,

    //! Report Voltage (in 10ths of a volt) 
    ONA_VOLTAGE_10THS_VOLTS = 0x014,

    //! Report Voltage (in 100ths of a volt)
    ONA_VOLTAGE_100THS_VOLTS = 0x015,

    //! Report Voltage (in simple good/bad)
    ONA_VOLTAGE_SIMPLE = 0x016,
#endif

#ifdef _NEED_ENERGY_MESSAGE
    //! Incremental Energy Query 
    ONA_ENERGY = 0x017,

    // Replacing old values with new ones
    //! Report Energy (in watt minutes)
    // ONA_ENERGY_WATT_MINUTES = 0x01B,
    //! Report Energy (in kilowatt minutes)
    // ONA_ENERGY_KILOWATT_MINUTES = 0x01C,

    //! Report Incremental Energy (in 2 watt seconds units)
    ONA_ENERGY_2_WATT_SECONDS = 0x018,

    //! Report Incremental Energy (in 20 watt seconds units)
    ONA_ENERGY_20_WATT_SECONDS = 0x019,

    //! Report Incremental Energy (in 200 watt seconds units)
    ONA_ENERGY_200_WATT_SECONDS = 0x01A,

    //! Report Incremental Energy (in 2000 watt seconds units)
    ONA_ENERGY_2000_WATT_SECONDS = 0x01B,

    //! Report Incremental Energy (in 20000 watt seconds units)
    ONA_ENERGY_20000_WATT_SECONDS = 0x01C,
#endif

#ifdef _NEED_ACCUM_ENERGY_MESSAGE
    //! Accumulated Energy Query 
    ONA_ACCUM_ENERGY = 0x01D,

    //! Report Accumulated Energy (in 2 watt seconds units)
    ONA_ACCUM_ENERGY_2_WATT_SECONDS = 0x01E,

    //! Report Accumulated Energy (in 20 watt seconds units)
    ONA_ACCUM_ENERGY_20_WATT_SECONDS = 0x01F,

    //! Report Accumulated Energy (in 200 watt seconds units)
    ONA_ACCUM_ENERGY_200_WATT_SECONDS = 0x020,

    //! Report Accumulated Energy (in 2000 watt seconds units)
    ONA_ACCUM_ENERGY_2000_WATT_SECONDS = 0x021,

    //! Report Accumulated Energy (in 20000 watt seconds units)
    ONA_ACCUM_ENERGY_20000_WATT_SECONDS = 0x022,
#endif

#ifdef _NEED_PEAK_ENERGY_MESSAGE
    //! Peak Energy Query 
    ONA_PEAK_ENERGY = 0x023,

    //! Report Peak Energy (in 2 watt seconds units)
    ONA_PEAK_ENERGY_2_WATT_SECONDS = 0x024,

    //! Report Peak Energy (in 20 watt seconds units)
    ONA_PEAK_ENERGY_20_WATT_SECONDS = 0x025,

    //! Report Peak Energy (in 200 watt seconds units)
    ONA_PEAK_ENERGY_200_WATT_SECONDS = 0x026,

    //! Report Peak Energy (in 2000 watt seconds units)
    ONA_PEAK_ENERGY_2000_WATT_SECONDS = 0x027,

    //! Report Peak Energy (in 20000 watt seconds units)
    ONA_PEAK_ENERGY_20000_WATT_SECONDS = 0x028,
#endif

#ifdef _NEED_GAS_MESSAGE
    //! Instantaneous Gas Query 
    ONA_GAS = 0x029,

    //! Instantaneous Gas Therm-Seconds
    ONA_GAS_THERM_SECS = 0x02A,

    //! Instantaneous Gas Therm-Minutes
    ONA_GAS_THERM_MINS = 0x02B,

    //! Instantaneous Gas Therm-Hours
    ONA_GAS_THERM_HRS = 0x02C,
#endif

#ifdef _NEED_ACCUM_GAS_MESSAGE
    //! Accumulated Gas Query 
    ONA_ACCUM_GAS = 0x02D,

    //! Accumulated Gas Therm-Seconds
    ONA_ACCUM_GAS_THERM_SECS = 0x02E,

    //! Accumulated Gas Therm-Minutes
    ONA_ACCUM_GAS_THERM_MINS = 0x02F,

    //! Accumulated Gas Therm-Hours
    ONA_ACCUM_GAS_THERM_HRS = 0x030,
#endif

#ifdef _NEED_AVERAGE_GAS_MESSAGE
    //! Average Gas Query 
    ONA_AVER_GAS = 0x031,

    //! Average Gas Therm-Seconds
    ONA_AVER_GAS_THERM_SECS = 0x032,

    //! Average Gas Therm-Minutes
    ONA_AVER_GAS_THERM_MINS = 0x033,

    //! Average Gas Therm-Hours
    ONA_AVER_GAS_THERM_HRS = 0x034,
#endif

#ifdef _NEED_PEAK_GAS_MESSAGE
    //! Peak Gas Query 
    ONA_PEAK_GAS = 0x035,

    //! Peak Gas Therm-Seconds
    ONA_PEAK_GAS_THERM_SECS = 0x036,

    //! Peak Gas Therm-Minutes
    ONA_PEAK_GAS_THERM_MINS = 0x037,

    //! Peak Gas Therm-Hours
    ONA_PEAK_GAS_THERM_HRS = 0x038,
#endif

#ifdef _NEED_INSTEON_MESSAGE
    //! Send INSTEON to address
    ONA_INSTEON_TO_ADDRESS = 0xFFB,

    //! Send INSTEON standard command
    ONA_INSTEON_COMMAND = 0xFFC,
#endif

#ifdef _NEED_X10_MESSAGE
    //! Send X10 simple msg
    ONA_X10_SIMPLE = 0xFFD,

    //! Send an extended X10 msg
    ONA_X10_EXTENDED = 0xFFE,
#endif

    //! Mask the message type bits
    ONA_MSG_TYPE_MASK = 0xFFF
} ona_msg_type_t;
#endif


//! Block application message types
typedef enum _ona_block_msg_type
{
    //! Insteon Extended block message type
    ONA_BLOCK_INSTEON_EXTENDED = 0x0000,

    //! Block text message type (used for block messages where the bytes represent ASCII characters
    ONA_BLOCK_TEXT = 0x0001,
	
	//! Block binary message type (used for block messages where the bytes can represent anything)
    ONA_BLOCK_BINARY = 0x0002
} ona_block_msg_type_t;


typedef enum _ona_unit_type
{
#ifdef _NEED_SIMPLE_SWITCH_TYPE
    //! Simple ON/OFF switch
    ONA_SIMPLE_SWITCH = 0x0000,
#endif

#ifdef _NEED_DIMMER_SWITCH_TYPE
    //! Switch w/ ON/OFF & dimmer
    ONA_DIMMER_SWITCH = 0x0001,
#endif

#ifdef _NEED_DISPLAY_SWITCH_TYPE
    //! Simple ON/OFF switch w/ display
    ONA_DISPLAY_SWITCH = 0x0002,
#endif

#ifdef _NEED_DISPLAY_DIMMER_TYPE
    //! Switch w/ ON/OFF, dimmer & display
    ONA_DISPLAY_DIMMER_SWITCH = 0x0003,
#endif

#ifdef _NEED_SIMPLE_LIGHT_TYPE
    //! Simple ON/OFF light
    ONA_SIMPLE_LIGHT = 0x0004,
#endif

#ifdef _NEED_DIMMING_LIGHT_TYPE
    //! Light w/ ON/OFF & dimmer
    ONA_DIMMING_LIGHT = 0x0005,
#endif

#ifdef _NEED_OUTLET_TYPE
    //! ON/OFF outlet
    ONA_OUTLET = 0x0006,
#endif

#ifdef _NEED_SPEAKER_TYPE
    //! Speaker module
    ONA_SPEAKER = 0x0007,
#endif

#ifdef _NEED_TEMPERATURE_SENSOR_TYPE
    //! Temperature sensor
    ONA_TEMPERATURE_SENSOR = 0x0008,
#endif

#ifdef _NEED_HUMIDITY_SENSOR_TYPE
    //! Humidity sensor
    ONA_HUMIDITY_SENSOR = 0x0009,
#endif

#ifdef _NEED_DOOR_WINDOW_SENSOR_TYPE
    //! Door/Window sensor
    ONA_DOOR_WINDOW_SENSOR = 0x000A,
#endif

#ifdef _NEED_MOTION_SENSOR_TYPE
    //! Motion sensor
    ONA_MOTION_SENSOR = 0x000B,
#endif

#ifdef _NEED_X10_BRIDGE_TYPE
    //! ONE-NET/X10 bridge
    ONA_ONE_NET_X10_BRIDGE = 0x000C,
#endif

#ifdef _NEED_INSTEON_BRIDGE_TYPE
    //! ONE-NET/INSTEON bridge
    ONA_ONE_NET_INSTEON_BRIDGE = 0x000D
#endif
} ona_unit_type_t;


//! Structure that holds information on the unit type count
typedef struct
{
    //! The type of unit
    ona_unit_type_t type;

    //! The number of this type of unit this device supports.
    UInt8 count;
} ona_unit_type_count_t;

//! Send Function
typedef one_net_status_t (*one_net_send_single_func_t)(UInt8 *data,
  UInt8 DATA_LEN, UInt8 DST_UNIT_IDX, UInt8 PRIORITY,
  const one_net_raw_did_t *RAW_DST, UInt8 SRC_UNIT);

#ifndef _ONE_NET_SIMPLE_CLIENT
    //! Block/stream request function
    typedef one_net_status_t (*one_net_block_stream_request_func_t)(
      UInt8 TYPE, BOOL SEND, UInt16 DATA_TYPE,
      UInt16 LEN, UInt8 PRIORITY,
      const one_net_raw_did_t *DID, UInt8 SRC_UNIT);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! @} ONE-NET_APP_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_pub_var
//! \ingroup ONE-NET_APP
//! @{

extern one_net_send_single_func_t one_net_send_single;

#ifndef _ONE_NET_SIMPLE_CLIENT
    extern one_net_block_stream_request_func_t one_net_block_stream_request;
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! @} ONE-NET_APP_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_pub_func
//! \ingroup ONE-NET_APP
//! @{

one_net_status_t ona_parse_msg_class_and_type(const UInt8 *MSG_DATA,
  ona_msg_class_t *msg_class, ona_msg_type_t *msg_type);

#ifndef _ONE_NET_MASTER
    one_net_status_t ona_send_unit_type_count_status(
      const one_net_raw_did_t *RAW_DST);
#endif // ifndef _ONE_NET_MASTER //

one_net_status_t ona_send_unit_type_count_query(
  const one_net_raw_did_t *RAW_DST);

#ifndef _ONE_NET_MASTER
    one_net_status_t ona_send_unit_type_status(UInt8 UNIT_TYPE_IDX,
      const one_net_raw_did_t *RAW_DST);
#endif // ifndef _ONE_NET_MASTER //

one_net_status_t ona_send_unit_type_query(UInt8 UNIT_TYPE_INDEX, 
  const one_net_raw_did_t *RAW_DST);

one_net_status_t ona_parse_unit_type_count(const UInt8 *MSG_DATA,
  UInt8 * unit_count, UInt8 * unit_type_count);

one_net_status_t ona_parse_unit_type_status(const UInt8 *MSG_DATA,
  ona_unit_type_t * unit_type, UInt8 * unit_count);

#ifndef _ONE_NET_MASTER
    one_net_status_t ona_parse_unit_type_query(const UInt8 *MSG_DATA,
      UInt8 *unit_type_index);
#endif // ifndef _ONE_NET_MASTER //

//! @} ONE-NET_APP_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP

#endif // _ONE_NET_APPLICATION_H //

