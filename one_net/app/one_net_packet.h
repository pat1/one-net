#ifndef ONE_NET_PACKET_H
#define ONE_NET_PACKET_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"
#include "one_net_encode.h"


//! \defgroup ONE-NET_PACKET ONE-NET Packet Definitions and Lengths
//! \ingroup ONE-NET
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
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
    \brief Constants and typedefs dealing with packets and packet lengths.

    Lengths of packets should now all be in one file instead of in
     transceiver-specific files.  I'm not entirely sure what should go in
     here as opposed to one_net_application.h and one_net_message.h, but the
     general idea is to get the packet-length constants out of one_net.h and
     even more importantly, get them out of files list adi.c.  Packet lengths
     are not application-specific, transceiver-specific, or processor-specific
     so we want them out of those files.
*/




//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_PACKET_typedefs
//! \ingroup ONE-NET_PACKET
//! @{



#define ONE_NET_RAW_PID_PACKET_TYPE_MASK 0x3F
#define ONE_NET_RAW_PID_MH_MASK 0x80
#define ONE_NET_RAW_PID_MH_SHIFT 7
#define ONE_NET_RAW_PID_STAY_AWAKE_MASK 0x40
#define ONE_NET_RAW_PID_STAY_AWAKE_SHIFT 6
#define ONE_NET_RAW_PID_SIZE_MASK 0xF00
#define ONE_NET_RAW_PID_SIZE_SHIFT 8




/*!
    \brief Raw Packet Types
*/

 

//! Single Data Packet.
#define ONE_NET_RAW_SINGLE_DATA 0x00

//! Acknowledgment of successful reception of a single data packet.
#define ONE_NET_RAW_SINGLE_DATA_ACK 0x01

//! Acknowledges that a single data packet was received, but an
//! error was encountered. The NACK reason field on this NACK specifies
//! the error condition that resulted in the NACK.
#define ONE_NET_RAW_SINGLE_DATA_NACK_RSN 0x02



#ifdef ROUTE
//! Route Packet
#define ONE_NET_RAW_ROUTE      0x03

//! Route Packet ACK
#define ONE_NET_RAW_ROUTE_ACK  0x04

//! Route Packet NACK
#define ONE_NET_RAW_ROUTE_NACK 0x05
#endif



#ifdef BLOCK_MESSAGES_ENABLED
//! Block Data Packet
#define ONE_NET_RAW_BLOCK_DATA 0x06

//! Acknowledgment of successful reception of a block data packet.
#define ONE_NET_RAW_BLOCK_DATA_ACK 0x07

//! Acknowledges that a block data packet was received, but an
//! error was encountered. The NACK reason field on this NACK specifies
//! the error condition that resulted in the NACK.
#define ONE_NET_RAW_BLOCK_DATA_NACK_RSN 0x08

//! A block transfer is being terminated for some reason.  This could be due
//! to a problem or due to a successful completion.  This should not be
//! assumed to be either an ACK or a NACK.  The packet must be parsed for
//! further meaning.
#define ONE_NET_RAW_BLOCK_TERMINATE 0x09
#endif



#ifdef STREAM_MESSAGES_ENABLED
//! Stream Data Packet
#define ONE_NET_RAW_STREAM_DATA 0x0A

//! Acknowledgment of successful reception of a stream data packet.
#define ONE_NET_RAW_STREAM_DATA_ACK 0x0B

//! Acknowledges that a stream data packet was received, but an
//! error was encountered. The NACK reason field on this NACK specifies
//! the error condition that resulted in the NACK.
#define ONE_NET_RAW_STREAM_DATA_NACK_RSN 0x0C

//! A stream transfer is being terminated for some reason.  This could be due
//! to a problem or due to a successful completion.  This should not be
//! assumed to be either an ACK or a NACK.  The packet must be parsed for
//! further meaning.
#define ONE_NET_RAW_STREAM_TERMINATE 0x0D
#endif



//! MASTER broadcast inviting a new CLIENT to join the network.
#define ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT 0x0E

//! CLIENT message sent requesting an invitation
#define ONE_NET_RAW_CLIENT_REQUEST_INVITE 0x0F



typedef enum _ona_msg_type
{
    //! Set/Query/Report Switch Status
    ONA_SWITCH = 0x00,

    //! Set/Query/Report Percent Status
    ONA_PERCENT = 0x01,

    //! Set/Query/Report Temperature Status
    ONA_TEMPERATURE = 0x02,

    //! Set/Query/Report Humidity Status
    ONA_HUMIDITY = 0x03,

    //! Set/Query/Report Pressure Status
    ONA_PRESSURE = 0x04,

    //! Set/Query/Report Rainfall Status
    ONA_RAINFALL = 0x05,

    //! Set/Query/Report Speed Status
    ONA_SPEED_M_S = 0x06,

    //! Set/Query/Report Direction Status
    ONA_DIRECTION = 0x07,

    //! Query status interval
    ONA_STATUS_INTERVAL = 0x08,

    //! Set/Query/Report Opening Status (called Access in the specification)
    ONA_OPENING = 0x09,

    //! Set/Query/Report Seal Status
    ONA_SEAL = 0x0A,

    //! Unused message
    ONA_UNUSED0 = 0x0B,

    //! Set/Query/Report Unit Type Count 
    ONA_UNIT_TYPE_COUNT = 0x0C,

    //! Set/Query/Report Unit Type
    ONA_UNIT_TYPE = 0x0D,

    //! Set/Query/Report Color
    ONA_COLOR = 0x0E,

    //! Query/Report Date
    ONA_DATE = 0x10,

    //! Query/Report Time
    ONA_TIME = 0x11,

    //! Query Voltage
    ONA_VOLTAGE = 0x12,

    //! Report Voltage (in volts)
    ONA_VOLTAGE_VOLTS = 0x13,

    //! Report Voltage (in 100ths of a volt)
    ONA_VOLTAGE_100THS_VOLTS = 0x15,

    //! Report Voltage (in simple good/bad)
    ONA_VOLTAGE_SIMPLE = 0x16,
    
    
    
    //! Send Simple Text (2 ASCII chars, no NULL terminator)
    ONA_SIMPLE_TEXT = 0x18,
    
    //! Send Text (ASCII string with C-Style NULL terminator)
    ONA_TEXT = 0x19,



    //! Incremental Energy Query 
    ONA_ENERGY = 0x20,
    
    //! Report Incremental Energy (in 1 Watt-Second units)
    ONA_ENERGY_1_WATT_SECONDS = 0x21,

    //! Report Incremental Energy (in 100 Watt-Seconds units)
    ONA_ENERGY_100_WATT_SECONDS = 0x22,

    //! Report Incremental Energy (in 10 Kilowatt-Seconds units)
    ONA_ENERGY_10_KWATT_SECONDS = 0x23,

    //! Report Incremental Energy (in 1000 Kilowatt-Seconds units)
    ONA_ENERGY_1000_KWATT_SECONDS = 0x24,

    //! Report Incremental Energy (in Kilowatt-Hour units)
    ONA_ENERGY_1_KWATT_HOUR = 0x25,

    //! Report Incremental Energy (in 100 Kilowatt-Hour units)
    ONA_ENERGY_100_KWATT_HOUR = 0x26,

    //! Report Incremental Energy (in 10,000 Kilowatt-Hour units)
    ONA_ENERGY_10000_KWATT_HOUR = 0x27,



    //! Accumulated Energy Query 
    ONA_ACCUM_ENERGY = 0x28,
    
    //! Report Accumulated Energy (in 1 Watt-Second units)
    ONA_ACCUM_ENERGY_1_WATT_SECONDS = 0x29,

    //! Report Accumulated Energy (in 100 Watt-Seconds units)
    ONA_ACCUM_ENERGY_100_WATT_SECONDS = 0x2A,

    //! Report Accumulated Energy (in 10 Kilowatt-Seconds units)
    ONA_ACCUM_ENERGY_10_KWATT_SECONDS = 0x2B,

    //! Report Accumulated Energy (in 1000 Kilowatt-Seconds units)
    ONA_ACCUM_ENERGY_1000_KWATT_SECONDS = 0x2C,

    //! Report Accumulated Energy (in Kilowatt-Hour units)
    ONA_ACCUM_ENERGY_1_KWATT_HOUR = 0x2D,

    //! Report Accumulated Energy (in 100 Kilowatt-Hour units)
    ONA_ACCUM_ENERGY_100_KWATT_HOUR = 0x2E,

    //! Report Accumulated Energy (in 10,000 Kilowatt-Hour units)
    ONA_ACCUM_ENERGY_10000_KWATT_HOUR = 0x2F,
    
    

    //! Peak Energy Query 
    ONA_PEAK_ENERGY = 0x30,
    
    //! Report Peak Energy (in 1 Watt-Second units)
    ONA_PEAK_ENERGY_1_WATT_SECONDS = 0x31,

    //! Report Peak Energy (in 100 Watt-Seconds units)
    ONA_PEAK_ENERGY_100_WATT_SECONDS = 0x32,

    //! Report Peak Energy (in 10 Kilowatt-Seconds units)
    ONA_PEAK_ENERGY_10_KWATT_SECONDS = 0x33,

    //! Report Peak Energy (in 1000 Kilowatt-Seconds units)
    ONA_PEAK_ENERGY_1000_KWATT_SECONDS = 0x34,

    //! Report Peak Energy (in Kilowatt-Hour units)
    ONA_PEAK_ENERGY_1_KWATT_HOUR = 0x35,

    //! Report Peak Energy (in 100 Kilowatt-Hour units)
    ONA_PEAK_ENERGY_100_KWATT_HOUR = 0x36,

    //! Report Peak Energy (in 10,000 Kilowatt-Hour units)
    ONA_PEAK_ENERGY_10000_KWATT_HOUR = 0x37,
    
    

    //! Power Query 
    ONA_POWER = 0x38,
    
    //! Report Power (in 1 Watt units)
    ONA_POWER_1_WATT = 0x39,

    //! Report Power (in 100 Watt units)
    ONA_POWER_100_WATTS = 0x3A,

    //! Report Power (in 10 Kilowatt units)
    ONA_POWER_10_KWATTS = 0x3B,

    //! Report Power (in 1000 Kilowatt units)
    ONA_POWER_1000_KWATTS = 0x3C,




    //! Instantaneous Gas Query 
    ONA_GAS = 0x40,

    //! Instantaneous Gas Therm-Seconds
    ONA_GAS_THERM_SECS = 0x41,

    //! Instantaneous Gas Therm-Minutes
    ONA_GAS_THERM_MINS = 0x42,

    //! Instantaneous Gas Therm-Hours
    ONA_GAS_THERM_HRS = 0x43,

    //! Accumulated Gas Query 
    ONA_ACCUM_GAS = 0x44,

    //! Accumulated Gas Therm-Seconds
    ONA_ACCUM_GAS_THERM_SECS = 0x45,

    //! Accumulated Gas Therm-Minutes
    ONA_ACCUM_GAS_THERM_MINS = 0x46,

    //! Accumulated Gas Therm-Hours
    ONA_ACCUM_GAS_THERM_HRS = 0x47,

    //! Average Gas Query 
    ONA_AVER_GAS = 0x48,

    //! Average Gas Therm-Seconds
    ONA_AVER_GAS_THERM_SECS = 0x49,

    //! Average Gas Therm-Minutes
    ONA_AVER_GAS_THERM_MINS = 0x4A,

    //! Average Gas Therm-Hours
    ONA_AVER_GAS_THERM_HRS = 0x4B,

    //! Peak Gas Query 
    ONA_PEAK_GAS = 0x4C,

    //! Peak Gas Therm-Seconds
    ONA_PEAK_GAS_THERM_SECS = 0x4D,

    //! Peak Gas Therm-Minutes
    ONA_PEAK_GAS_THERM_MINS = 0x4E,

    //! Peak Gas Therm-Hours
    ONA_PEAK_GAS_THERM_HRS = 0x4F
} ona_msg_type_t;


//! Packet related constants
enum
{
    //! Length of preamble and header
    ONE_NET_PREAMBLE_HEADER_LEN = 4,
    
    //! Length of encoded message CRC
    ONE_NET_ENCODED_MSG_CRC_LEN = 1,
    
    //! The size of the encoded Packet ID field
    ON_ENCODED_PID_SIZE = 2,

    //! The number of bytes required to store the raw hops field
    ON_RAW_HOPS_SIZE = 1,

    //! The size of the encoded hops field (in bytes)
    ON_ENCODED_HOPS_SIZE = 1,    
    
    //! The index into the encoded packet where the repeater did lies
    ON_ENCODED_RPTR_DID_IDX = ONE_NET_PREAMBLE_HEADER_LEN,
    
    //! The index into the encoded packet where the message CRC lies
    ON_ENCODED_MSG_CRC_IDX = ON_ENCODED_RPTR_DID_IDX + ON_ENCODED_DID_LEN,
        
    ON_ENCODED_DST_DID_IDX = ON_ENCODED_MSG_CRC_IDX + ONE_NET_ENCODED_MSG_CRC_LEN,

    //! The index into the encoded packet where the NID starts.
    ON_ENCODED_NID_IDX = ON_ENCODED_DST_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the source DID starts.
    ON_ENCODED_SRC_DID_IDX = ON_ENCODED_NID_IDX + ON_ENCODED_NID_LEN,

    //! The index into the encoded packet where the PID starts.
    ON_ENCODED_PID_IDX = ON_ENCODED_SRC_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the payload starts
    ON_PLD_IDX = ON_ENCODED_PID_IDX + ON_ENCODED_PID_SIZE
};


#ifdef ONE_NET_MULTI_HOP
//! Hops field related constants
enum
{
    //! The number of bits to shift the max hops field
    ON_MAX_HOPS_BUILD_SHIFT = 2,

    //! Number of bits to shift the hops field
    ON_HOPS_BUILD_SHIFT = 5,

    //! Mask to use on the max hops field when building the packet
    ON_MAX_HOPS_BUILD_MASK = 0x1C,

    //! Mask to use on the hops field when building the packet
    ON_HOPS_BUILD_MASK = 0xE0,

    //! Shift to use when parsing hops
    ON_PARSE_HOPS_SHIFT = 5,

    //! Shift to use when parsing max hops
    ON_PARSE_MAX_HOPS_SHIFT = 2,
    
    //! Mask to use when parsing hops / max hops once shifted
    ON_PARSE_RAW_HOPS_FIELD_MASK = 0x07
};
#endif


enum
{
    ON_ACK_NACK_ENCODED_PLD_LEN = 11,
    #ifdef EXTENDED_SINGLE
    ON_LARGE_ACK_NACK_ENCODED_PLD_LEN = 22,
    ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN = 33,
    ON_MAX_ENCODED_ACK_NACK_LEN = ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN,
    #else
    ON_MAX_ENCODED_ACK_NACK_LEN = ON_ACK_NACK_ENCODED_PLD_LEN,
    #endif
    
    
    ON_SINGLE_ENCODED_PLD_LEN = 11,
    #ifdef EXTENDED_SINGLE
    ON_LARGE_SINGLE_ENCODED_PLD_LEN = 22,
    ON_EXTENDED_SINGLE_ENCODED_PLD_LEN = 33,
    ON_MAX_ENCODED_SINGLE_LEN = ON_EXTENDED_SINGLE_ENCODED_PLD_LEN,
    #else
    ON_MAX_ENCODED_SINGLE_LEN = ON_SINGLE_ENCODED_PLD_LEN,
    #endif
    
    
    ON_INVITE_ENCODED_PLD_LEN = 33,
    #ifdef BLOCK_MESSAGES_ENABLED
    ON_BLOCK_ENCODED_PLD_LEN = 43,
    #endif
    #ifdef STREAM_MESSAGES_ENABLED
    ON_STREAM_ENCODED_PLD_LEN = 43,
    #endif
    
    #ifdef BLOCK_MESSAGES_ENABLED
        //! The maximum length of a raw payload field (not including the extra
        //! byte needed to store the 2 bits for the encryption method type).
        ON_MAX_RAW_PLD_LEN = ON_BLOCK_ENCODED_PLD_LEN - 1,
    #else // if block messages are defined //
        //! The maximum length of a raw payload field (not including the extra
        //! byte needed to store the 2 bits for the encryption method type).
        ON_MAX_RAW_PLD_LEN = ON_INVITE_ENCODED_PLD_LEN - 1,
    #endif // else if block messages are not defined //
    
    
    // TODO -- there appear to be a lot of constants with the word "raw" in
    // them which appear to be "encoded" lengths.  I'm already confused.  We
    // need to clean up the language.  Adding a constant called
    // "ON_MAX_ENCODED_PLD_LEN_WITH_TECH", which is the maximum length of the
    // payload portion of a packet WITH the encryption technique.  This is
    // calculated by adding 1 to "ON_MAX_RAW_PLD_LEN".  In other words, we
    // are supposedly adding 1 to a "raw" length and getting an "encoded"
    // length.  Clearly this is not true.  (I hope) the math works out and
    // everything is grand, but clearly the names need to change.  I'm
    // confused and I wrote most of it.  Anyone new READING this code is in
    // for a fun time.
    ON_MAX_ENCODED_PLD_LEN_WITH_TECH = ON_MAX_RAW_PLD_LEN + 1
};


enum
{
    ON_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_ACK_NACK_ENCODED_PLD_LEN,
    #ifdef EXTENDED_SINGLE
    ON_LARGE_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_LARGE_ACK_NACK_ENCODED_PLD_LEN,
    ON_EXTENDED_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN,
    #endif
    ON_MAX_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_MAX_ENCODED_ACK_NACK_LEN,


    ON_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_SINGLE_ENCODED_PLD_LEN,
    #ifdef EXTENDED_SINGLE
    ON_LARGE_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_LARGE_SINGLE_ENCODED_PLD_LEN,
    ON_EXTENDED_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_EXTENDED_SINGLE_ENCODED_PLD_LEN,
    #endif    
    ON_MAX_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_MAX_ENCODED_SINGLE_LEN,
    
    
    ON_INVITE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_INVITE_ENCODED_PLD_LEN,
    #ifdef BLOCK_MESSAGES_ENABLED
    ON_BLOCK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_BLOCK_ENCODED_PLD_LEN,
    #endif
    #ifdef STREAM_MESSAGES_ENABLED
    ON_STREAM_ENCODED_PKT_LEN = ON_PLD_IDX + ON_STREAM_ENCODED_PLD_LEN
    #endif
};


enum
{
    #ifdef ONE_NET_MULTI_HOP
        ON_ACK_NACK_ENCODED_PKT_SIZE = ON_MAX_ACK_NACK_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        ON_SINGLE_ENCODED_PKT_SIZE = ON_MAX_SINGLE_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #ifdef BLOCK_MESSAGES_ENABLED
        ON_BLOCK_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #endif
        #ifdef STREAM_MESSAGES_ENABLED
        ON_STREAM_ENCODED_PKT_SIZE = ON_STREAM_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #endif
        ON_INVITE_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,              
    #else // ifdef ONE_NET_MULTI_HOP //
        ON_ACK_NACK_ENCODED_PKT_SIZE = ON_MAX_ACK_NACK_ENCODED_PKT_LEN,
        ON_SINGLE_ENCODED_PKT_SIZE = ON_MAX_SINGLE_ENCODED_PKT_LEN,
        #ifdef BLOCK_MESSAGES_ENABLED
        ON_BLOCK_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_LEN,
        #endif
        #ifdef STREAM_MESSAGES_ENABLED
        ON_STREAM_ENCODED_PKT_SIZE = ON_STREAM_ENCODED_PKT_LEN,
        #endif
        ON_INVITE_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_LEN,
    #endif // else ONE_NET_MULTI_HOP is not defined //
    
    ON_MIN_ENCODED_PKT_SIZE = ON_PLD_IDX + 11,
    #if defined(BLOCK_MESSAGES_ENABLED)
    ON_MAX_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_SIZE,
    #else
    ON_MAX_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_SIZE,
    #endif
    
    #ifdef BLOCK_MESSAGES_ENABLED
    ON_MAX_ENCODED_DATA_PKT_SIZE = ON_BLOCK_ENCODED_PKT_SIZE
    #else
    ON_MAX_ENCODED_DATA_PKT_SIZE = ON_SINGLE_ENCODED_PKT_SIZE
    #endif
};


//! Nonce, Message type, Payload indexes, masks, shifts
enum
{
    //! Index into the payload where the crc starts
    ON_PLD_CRC_IDX = 0,
    
    //! Index into the payload of the Message ID
    ON_PLD_MSG_ID_IDX = 1,

    //! Index for the ack / nack handle
    ON_PLD_RESP_HANDLE_IDX = 2,

    //! Index for the message type
    ON_PLD_MSG_TYPE_IDX = 2,

    //! Index for the data portion
    ON_PLD_DATA_IDX = 3,
    
    //! Index for the admin type in an admin message
    ON_PLD_ADMIN_TYPE_IDX = ON_PLD_DATA_IDX,
    
    //! Index of the data portion of an admin message
    ON_PLD_ADMIN_DATA_IDX = ON_PLD_ADMIN_TYPE_IDX + 1,

    //! The mask to use for the handle when
    //! building the payload field of a data packet
    ON_RESP_HANDLE_BUILD_MASK = 0x0F,

    //! The mask to use for the message type when building or parsing the
    //! payload field of a data packet
    ON_PLD_MSG_TYPE_MASK = ON_RESP_HANDLE_BUILD_MASK
};


enum
{
    //! Index for the payload chunk index and size
    ON_BS_PLD_CHUNK_IDX = ON_PLD_MSG_TYPE_IDX,
    
    //! Index for the packet index of time in a block payload
    ON_BS_PLD_PKT_IDX = 4,
    
    //! Index for the time in a stream payload
    ON_BS_STREAM_TIME_IDX = ON_BS_PLD_PKT_IDX,
    
    //! Index for the data
    ON_BS_DATA_PLD_IDX = ON_BS_PLD_PKT_IDX + 3,
    
    //! Data size in a block / stream data packet
    ON_BS_DATA_PLD_SIZE = 4 * ONE_NET_XTEA_BLOCK_SIZE - ON_BS_DATA_PLD_IDX
};


//! Admin packet indexes
enum
{
    #ifdef BLOCK_MESSAGES_ENABLED
    //! Index for the low priority fragment delay in an admin message
    //! containing both fragment delays
    ON_FRAG_LOW_IDX = 0,

    //! Index for the high priority fragment delay in an admin message
    //! containing both fragment delays
    ON_FRAG_HIGH_IDX = 2,
    #endif

    //! Index for encoded did of peer device for (un)assignment of peers
    ON_PEER_DID_IDX = 0,

    //! Index for peer unit for (un)assignment of peers.  This is the unit in
    //! the peer device being assigned to a unit in this device
    ON_PEER_PEER_UNIT_IDX = 2,

    //! Index for destination unit for (un)assignment of peers.  This is the
    //! unit in the device that is being assigned a peer
    ON_PEER_SRC_UNIT_IDX = 3,
};



//! Payload CRC releated constants
enum
{
    //! The size of the crc in the payload (in bytes)
    ON_PLD_CRC_SIZE = 1,

    //! The initial payload crc
    ON_PLD_INIT_CRC = 0xFF,

    //! The order of the payload crc
    ON_PLD_CRC_ORDER = 8,

    //! The initial crc for the non-volatile parameters
    ON_PARAM_INIT_CRC = 0xFF,

    //! The order of the crc computed over the non-volatile parameters
    ON_PARAM_CRC_ORDER = 8
};


/* Normal Single message payload constants */
enum
{
    //! Five bytes are as follows...
    //!
    //! Byte 0 -- Source and destination units.  4 most significant bits are
    //!           source unit bits.  4 least significant bits are destination
    //!           unit bits.
    //! Bytes 1 and 2 -- Message class and type are the 12 left-most bytes.
    //!           Message class (i.e. ONA_COMMAND) are the 4 most significant
    //!           bits.  The remaining 8 bits are the message type
    //!           (i.e ONA_SWITCH)
    //! Bytes 2 thru 4 -- 20 bits.  Message data.  The "data" of the message.
    //!           For a normal switch message, this would be ONA_ON, ONA_OFF, or
    //!           ONA_TOGGLE
    //!
    //!
    //! First 12 bits(message type and class) are the "header".
    //!
    //!
    //! So a "Turn switch on command from unit 4 to unit 6 would be as follows
    //!
    //! Source Unit                 = 4 = 0100
    //! Destination Unit            = 6 = 0110
    //! Message Class = ONA_COMMAND = 5 = 0101 (note ONA_COMMAND is 0x500.
    //!                                         We are isolating the left-most
    //!                                         4 bits.)
    //!
    //! Message Type = ONA_SWITCH   = 0 = 00000000 (8 bits)
    //! Message Data = ONA_ON       = 1 = 00000000000000000001 (20 bits)
    //!
    //!
    //! 5 byte message is...
    //!
    //! SSSSDDDDCCCCTTTTTTTTTTTTXXXXXXXXXXXXXXXX
    //! 0100011001010000000000000000000000000001
    //!
    //! which is 0x4650000001 in Hex
    //!
    //! S = Source Unit
    //! D = Dest. Unit
    //! C = Message Class
    //! T = Message Type
    //! X = Message Data
    //!
    //! Again, Class and Type are combined into "Header".
    //!
    //! one_net_application.h defines "getters" and "setters" for source
    //! unit, destination unit, header, and data.
    //!
    //! 1) put_src_unit and get_src_unit
    //! 2) put_dst_unit and get_dst_unit
    //! 3) put_msg_hdr and get_msg_hdr
    //! 4) put_msg_data and get_msg_data
    //!
    //!
    //! To place the message above into a 5 byte payload, the following
    //! code may be used...
    //!
    //!
    //!   UInt8 payload[5];
    //!   put_src_unit(4, payload);
    //!   put_dst_unit(6, payload);
    //!   put_msg_hdr(ONA_COMMAND | ONA_SWITCH, payload);
    //!   put_msg_data(ONA_ON, payload);
    //!
    //!
    //!   The message classes are shifted 8 bits precisely so that the "OR"
    //!   operator will work as above.  ONA_COMMAND is defined as 0x500, that
    //!   is 5 followed by 8 empty bits.  ONA_SWITCH is 0x00, which is
    //!   exactly 8 bits long.  Thus the message class and message type do
    //!   not overlap and the message class is shifted in such a way that the
    //!   | ("OR") operator can be used.
    //!
    //!
    //!   The "get" functions work in precisely the reverse order, so they
    //!   take a five byte payload and parse it.
    //!
    //!
    //!   This 5-byte payload is specified in the ONE-NET specification and
    //!   should be used whenever for all "ON_APP_MSG" messages.  Users can send
    //!   5-byte payloads which do not follow this layout, but they must not
    //!   specify "ON_APP_MSG" when creating the message.  Not ethat there are
    //!   many message types that do not take full advantage of all 20 message
    //!   data bytes.  Examples are switch messages, which use only 2, and text
    //!   messages, which use 16 of the 20.  In fact, the majority of messages
    //!   will not use all 20 bits, but some do.  If this is the case, it may
    //!   be useful to change the code so taht message data does not use a
    //!   UInt32, but rather a UInt8 or a UInt16.  That can save stack space and
    //!   memory.

    
    
    //! Index of header within single packet payload (header is message
    //! class and message type
    ONA_UNIT_IDX      = 0,

    ONA_MSG_SRC_UNIT_IDX   = ONA_UNIT_IDX, // Where the byte is
    ONA_MSG_SRC_UNIT_MASK  = 0xf0,  // Where the bits are in the byte
    ONA_MSG_SRC_UNIT_SHIFT = 4,     // Shift left this much to put them in

    ONA_MSG_DST_UNIT_IDX   = ONA_UNIT_IDX,     // Where the byte is
    ONA_MSG_DST_UNIT_MASK  = 0x0f,  // Where the bits are in the byte
    ONA_MSG_DST_UNIT_SHIFT = 0,     // Shift left this much to put them in

    // Header now follows src/dst addresses
    ONA_MSG_HDR_IDX = 1,

    //! Index of Message Data within payload
    ONA_TEXT_DATA_IDX = 3,

    //! Length of Message Data
    ONA_SIMPLE_TEXT_DATA_LEN = 2,
};


//! constants dealing with the raw payload of a data packet (i.e. does not
//! include msg id, crc, or nack reason.
enum
{
    ONA_DATA_INDEX = 3, //! the index in a data packet where the actual
                        //! data starts
    ONA_SINGLE_PACKET_PAYLOAD_LEN = ONE_NET_XTEA_BLOCK_SIZE -
      ONA_DATA_INDEX, //! the number of data bytes in a single message
    
    #ifdef EXTENDED_SINGLE
    ONA_LARGE_SINGLE_PACKET_PAYLOAD_LEN = ONA_SINGLE_PACKET_PAYLOAD_LEN +
      ONE_NET_XTEA_BLOCK_SIZE, //! the number of data bytes in a large
                               //! single message
    ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN = ONA_LARGE_SINGLE_PACKET_PAYLOAD_LEN +
      ONE_NET_XTEA_BLOCK_SIZE, //! the number of data bytes in an extended
                               //! single message
                               
    ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN = ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN
    #else
    ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN = ONA_SINGLE_PACKET_PAYLOAD_LEN    
    #endif
};


//! Invite related constants
enum
{
    //! The length (in bytes) of the raw invite request packet.  The 25th byte
    //! contains the 2 bits for the method used to encrypt the data with.
    ON_RAW_INVITE_SIZE = 25,

    //! The length (in bytes) of the encoded invite portion of the invite
    //! request packet.  This includes the extra byte needed to store the 2 bits
    //! for the encryption type.  This is also the size of the invite in 6 bit
    //! words
    ON_ENCODED_INVITE_SIZE = 33,

    //! The number of bytes to compute the crc over in the invite message
    ON_INVITE_DATA_LEN = 23,

    //! The version index
    ON_INVITE_VERSION_IDX = 1,

    //! The assigned DID index
    ON_INVITE_ASSIGNED_DID_IDX = 2,

    //! The key index
    ON_INVITE_KEY_IDX = 4,

    //! The features index
    ON_INVITE_FEATURES_IDX = 20,

    //! The crc index
    ON_INVITE_CRC_IDX = 0,
    
    //! The index to start computing the CRC over
    ON_INVITE_CRC_START_IDX = ON_INVITE_CRC_IDX + 1
};


typedef struct
{
    UInt8* packet_bytes;
    UInt16 raw_pid;  //! raw pid of the packet
    UInt16 msg_id; //! raw message id of the packet (0 to 4095)
    UInt8 payload_len; //! length of the encoded payload in bytes
    
    // TODO -- can we get rid of these 2 hops fields?
    #ifdef ONE_NET_MULTI_HOP
    UInt8 hops; //! hops of the packet.  May or may not be relevant.
    UInt8 max_hops; //! Maximum hops of the packet.  May or may not be relevant
    #endif
} on_pkt_t;


//! @} ONE-NET_PACKET_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PACKET_const
//! \ingroup ONE-NET_PACKET
//! @{

//! @} ONE-NET_PACKET_const
//                                  CONSTANTS END
//==============================================================================



//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PACKET_pub_func
//! \ingroup ONE-NET_PACKET
//! @{

SInt8 get_encoded_payload_len(UInt16 raw_pid);
SInt8 get_raw_payload_len(UInt16 raw_pid);
SInt8 get_num_payload_blocks(UInt16 raw_pid);
UInt8 get_encoded_packet_len(UInt16 raw_pid, BOOL include_header);
BOOL set_ack_or_nack_pid(UInt16* raw_pid, BOOL is_ack);

#ifdef ONE_NET_MULTI_HOP
BOOL set_multihop_pid(UInt16* raw_pid, BOOL is_multihop);
#endif
BOOL packet_is_data(UInt16 raw_pid);

BOOL packet_is_ack(UInt16 raw_pid);
BOOL packet_is_nack(UInt16 raw_pid);

SInt16 get_single_response_pid(UInt16 raw_single_pid, BOOL isACK,
  BOOL stay_awake);
  
BOOL packet_length_is_valid(UInt16 raw_pid);
SInt8 get_default_num_blocks(UInt16 raw_pid);



// inline function implementation below //


#ifdef ONE_NET_MULTI_HOP
/*!
    \brief Determines whether a given PID represents a multi-hop packet.

    Determines whether a given PID represents a multi-hop packet.

    \param[in] raw_pid The pid to check

    \return True if encoded_pid is a multi-hop packet, false otherwise.
*/
ONE_NET_INLINE BOOL packet_is_multihop(UInt16 raw_pid)
{
    return ((raw_pid & ONE_NET_RAW_PID_MH_MASK) > 0);
}
#endif


/*!
    \brief Determines whether a given PID represents a single packet.

    Determines whether a given PID represents a single packet.

    \param[in] raw_pid The pid to check

    \return True if pid is a single packet, false otherwise.
*/
ONE_NET_INLINE BOOL packet_is_single(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    #ifdef ROUTE
    return (raw_pid <= ONE_NET_RAW_ROUTE_NACK);
    #else
    return (raw_pid <= ONE_NET_RAW_SINGLE_DATA_NACK_RSN);
    #endif
}


#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Determines whether a given PID represents a block packet.

    Determines whether a given PID represents a block packet.

    \param[in] raw_pid The pid to check

    \return True if pid is a block packet, false otherwise.
*/
ONE_NET_INLINE BOOL packet_is_block(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    return (raw_pid >= ONE_NET_RAW_BLOCK_DATA &&
      raw_pid <= ONE_NET_RAW_BLOCK_TERMINATE);
}
#endif


#ifdef STREAM_MESSAGES_ENABLED
/*!
    \brief Determines whether a given PID represents a stream packet.

    Determines whether a given PID represents a stream packet.

    \param[in] raw_pid The pid to check

    \return True if pid is a stream packet, false otherwise.
*/
ONE_NET_INLINE BOOL packet_is_stream(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    return (raw_pid >= ONE_NET_RAW_STREAM_DATA &&
      raw_pid <= ONE_NET_RAW_STREAM_TERMINATE);
}
#endif


/*!
    \brief Determines whether a given PID represents an invite packet.

    Determines whether a given PID represents an invite packet.

    \param[in] raw_pid The pid to check

    \return True if pid is an invite packet, false otherwise.
*/
ONE_NET_INLINE BOOL packet_is_invite(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    return (raw_pid == ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT);
}


#ifdef ROUTE
ONE_NET_INLINE BOOL packet_is_route(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    return (raw_pid >= ONE_NET_RAW_ROUTE && raw_pid <= ONE_NET_RAW_ROUTE_NACK);
}
#endif


// don't #include one_net_port_specific.h here due to some circular
// dependecies.  Just declare a few functions from it here to avoid compiler
// errors.

UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM);
void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream);
UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM);
void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream);


ONE_NET_INLINE BOOL get_raw_pid(UInt8* payload, UInt16* raw_pid)
{
    UInt8 raw_pld_arr[ON_ENCODED_PID_SIZE];
    if(on_decode(raw_pld_arr, payload, ON_ENCODED_PID_SIZE)
      != ONS_SUCCESS)
    {
        return FALSE;
    }
    
    *raw_pid = one_net_byte_stream_to_int16(raw_pld_arr);
    (*raw_pid) >>=  4;
    return TRUE;
}


ONE_NET_INLINE void put_raw_pid(UInt8* payload, UInt16 raw_pid)
{
    UInt8 raw_pld_arr[ON_ENCODED_PID_SIZE];    
    raw_pid <<= 4;
    one_net_int16_to_byte_stream(raw_pid, raw_pld_arr);
    on_encode(payload, raw_pld_arr, ON_ENCODED_PID_SIZE);
}

    
    
//! @} ONE-NET_PACKET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================


//! @} ONE-NET_PACKET

#endif // ONE_NET_PACKET_H //
