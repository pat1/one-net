#ifndef _ONE_NET_MESSAGE_H
#define _ONE_NET_MESSAGE_H

#include "config_options.h"
#include "one_net_types.h"


//! \defgroup ONE-NET_MESSAGE ONE-NET Message Definitions
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
    \file one_net_message.h
    \brief Declarations for ONE-NET messages

    This is global ONE-NET message information.
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MESSAGE_const
//! \ingroup ONE-NET_MESSAGE
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



//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{


// note : ON_STATUS_QUERY_RESP and ON_STATUS_FAST_QUERY_RESP are subsets of status messages.
// They just add a little more information, which the application is free to ignore.
// Note the values 0, 1, and 2 in a 4 bit "message class" field.  If you do a mask on the
// left-most 2 bits of "message class", you get the following "families"...

// Thus if an application wants to treat all status updates the same, it should not check
// to see whether the message class is ONA_STATUS.  It should instead do a mask of the two
// left-most bits of the message class and see if that is equal to ONA_STATUS.

// 0 = Status
// 1 = Command
// 2 = Query
// 3 = Poll/Fast Query
typedef enum _ona_msg_class
{
    ONA_STATUS   = 0x0000, //!< Status of a unit (not part of an ACK)
    ONA_STATUS_QUERY_RESP = 0x1000, //!< Status of a unit (in response to a query and not part of the ACK)
#ifdef _POLL
    ONA_STATUS_FAST_QUERY_RESP = 0x2000, //!< Status of a unit (in the ACK in response to a fast query)
#endif
    ONA_STATUS_ACK_RESP = 0x3000, //!< Status of a unit (as part of an ACK and NOT in response to a query or fast query)
    ONA_COMMAND  = 0x4000, //!< Command to change status of a unit
    ONA_QUERY    = 0x8000, //!< Query status of a unit
#ifdef _POLL
    ONA_POLL     = 0xC000, //!< Poll status of a unit
#endif

    ONA_MSG_CLASS_MASK = 0xF000,    //!< Used to mask message class bits
} ona_msg_class_t;



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

    //! Power Query
    ONA_POWER = 0x039,

    //! Power 2 Milliwatts
    ONA_POWER_2_MILLIWATTS = 0x03A,
    
    //! Power 20 Milliwatts
    ONA_POWER_20_MILLIWATTS = 0x03B,
    
    //! Power 50 Milliwatts
    ONA_POWER_50_MILLIWATTS = 0x03C,
    
    //! Power 200 Milliwatts
    ONA_POWER_200_MILLIWATTS = 0x03D,
    
    //! Power Watts
    ONA_POWER_WATTS = 0x03E,
    
    //! Power 2 Watts
    ONA_POWER_2_WATTS = 0x03F,
    
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


// TODO - ona_block_msg_type_t and ona_unit_type_t should probably be different for
// vesions 1.x and 2.x(12 bits vs. 14 bits?).  Holding off for now since everything
// is less thatn 0xFFF, so 12-bit and 14-bit masks will be the same.

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


//! @} ONE-NET_MESSAGE_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MESSAGE_pub_var
//! \ingroup ONE-NET_MESSAGE
//! @{

//! @} ONE-NET_MESSAGE_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MESSAGE_pub_func
//! \ingroup ONE-NET_MESSAGE
//! @{

//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MESSAGE

#endif // _ONE_NET_MESSAGE_H //
