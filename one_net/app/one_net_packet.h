#ifndef _ONE_NET_PACKET_H
#define _ONE_NET_PACKET_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_constants.h"
#include "one_net_xtea.h"


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


#ifndef _ONA_MSG_TYPE_MASK
    //! Mask the message type bits
    #define ONA_MSG_TYPE_MASK 0xFFF
#endif
   
    
/*!
    \brief Encoded Packet Types
*/
typedef enum
{
    //! MASTER broadcast inviting a new CLIENT to join the network.
    ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT = 0xB4,

    //! Acknowledgment of successful reception of a single data packet.
    ONE_NET_ENCODED_SINGLE_DATA_ACK = 0xBC,

    //! Acknowledgement of successful reception of a single data packet.  Also
    //! tells the sender of the single data packet not to go to sleep until
    //! a certain time.
    ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE = 0xB3,

    //! Acknowledges that a single data packet was received, but not 
    //! successfully parsed.  Also tells the sender of the single data packet
    //! not to go to sleep until a certain time.
    ONE_NET_ENCODED_SINGLE_DATA_NACK_STAY_AWAKE = 0xBA,

    //! Single Data Packet.
    ONE_NET_ENCODED_SINGLE_DATA = 0xB9,
    
    //! Extended Single Data Packet
    ONE_NET_ENCODED_LARGE_SINGLE_DATA = 0xB6,
    
    //! Extended Single Data Packet
    ONE_NET_ENCODED_EXTENDED_SINGLE_DATA = 0xC6,

    //! Block Data Packet
    ONE_NET_ENCODED_BLOCK_DATA = 0xB2,

    //! Acknowledgment of successful reception of a single data packet.
    ONE_NET_ENCODED_BLOCK_DATA_ACK = 0xCC,

    //! The sender of the block data is acknowledging that the transaction is
    //! complete (necessary acks received), and that the receiver can stop
    //! listening
    ONE_NET_ENCODED_BLOCK_TXN_ACK = 0xCA,

    //! Stream Data Packet
    ONE_NET_ENCODED_STREAM_DATA = 0xC5,

    //! Sent by the receiver of a stream data packet to alert the sender that
    //! it is still receiving the stream.
    ONE_NET_ENCODED_STREAM_KEEP_ALIVE = 0xC9,

    //! Acknowledgment of successful reception of a large single data packet.
    ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK = 0xD3,

    //! Acknowledgment of successful reception of an extended single data packet.
    ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK = 0xDA,

    //! Acknowledgment of an unsuccessful reception of a large single data packet.
    ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_RSN = 0xDC,

    //! Acknowledgment of an unsuccessful reception of an extended single data packet.
    ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_RSN = 0xD5,

    // Multi-hop packets
    //! Multi-hop version of MASTER broadcast inviting a new CLIENT to join the
    //! network.
    ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT = 0x54,

    //! Multi-hop version of Acknowledgment of successful reception of a single
    //! data packet.
    ONE_NET_ENCODED_MH_SINGLE_DATA_ACK = 0x5C,

    //! Multi-hop version of Acknowledgement of successful reception of a single
    //! data packet.  Also tells the sender of the single data packet to listen
    //! for a single data packet from the CLIENT that received the single data
    //! packet.
    ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE = 0x53,

    //! Multi-hop version of Acknowledgement that a single data packet was
    //! received, but not successfully parsed.
    ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_STAY_AWAKE = 0x5A,

    //! Multi-hop version of Single Data Packet.
    ONE_NET_ENCODED_MH_SINGLE_DATA = 0x59,

    //! Multi-hop version of Block Data Packet
    ONE_NET_ENCODED_MH_BLOCK_DATA = 0x52,

    //! Multi-hop version of Acknowledgment of successful reception of a single
    //! data packet.
    ONE_NET_ENCODED_MH_BLOCK_DATA_ACK = 0x9C,

    //! Multi-hop version of The sender of the block data acknowledging that
    //! the transaction is complete (necessary acks received), and that the
    //! receiver can stop listening
    ONE_NET_ENCODED_MH_BLOCK_TXN_ACK = 0x9A,

    //! Multi-hop version Stream Data Packet
    ONE_NET_ENCODED_MH_STREAM_DATA = 0x95,

    //! Multi-hop version of Sent by the receiver of a stream data packet to
    //! alert the sender that it is still receiving the stream.
    ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE = 0x99,

    //! Multi-Hop Single Data Packet which does not follow a ONE-NET convention
    ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA = 0x92,
    
    //! Multi-Hop Extended Single Data Packet    
    ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA = 0xD4,
    
    //! Acknowledgment of successful reception of a multi-hop large single data packet.
    ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_ACK = 0x35,

    //! Acknowledgment of an unsuccessful reception of a multi-hop large single data packet.
    ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_NACK_RSN = 0x39,
    
    //! Acknowledgment of successful reception of a multi-hop extended single data packet.
    ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_ACK = 0x36,

    //! Acknowledgment of an unsuccessful reception of a multi-hop extended single data packet.
    ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_NACK_RSN = 0x32,

    //! Acknowledges that a single data packet was received, but an
    //! error was encountered. The NACK reason field on this NACK specifies
    //! the error condition that resulted in the NACK.
    ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN = 0x64,

    //! Acknowledges that a block data packet was received, but an
    //! error was encountered. The NACK reason field on this NACK specifies
    //! the error condition that resulted in the NACK.
    ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN = 0x6C,

    //! Acknowledges that a single data packet was received, but an
    //! error was encountered. The NACK reason field on this NACK specifies
    //! the error condition that resulted in the NACK.
    ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN = 0x63,

    //! Multi-hop version of Acknowledgment that a block data packet was received, but an
    //! error was encountered. The NACK reason field on this NACK specifies
    //! the error condition that resulted in the NACK.
    ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN = 0x6A
} on_pid_t;


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

    //! Send Simple Text (3 ASCII chars)
    ONA_SIMPLE_TEXT = 0x0F,

    //! Query/Report Date
    ONA_DATE = 0x10,

    //! Query/Report Time
    ONA_TIME = 0x11,

    //! Query Voltage
    ONA_VOLTAGE = 0x12,

    //! Report Voltage (in volts)
    ONA_VOLTAGE_VOLTS = 0x13,

    //! Report Voltage (in 10ths of a volt) 
    ONA_VOLTAGE_10THS_VOLTS = 0x14,

    //! Report Voltage (in 100ths of a volt)
    ONA_VOLTAGE_100THS_VOLTS = 0x15,

    //! Report Voltage (in simple good/bad)
    ONA_VOLTAGE_SIMPLE = 0x16,

    //! Incremental Energy Query 
    ONA_ENERGY = 0x17,

    //! Report Incremental Energy (in 2 watt seconds units)
    ONA_ENERGY_2_WATT_SECONDS = 0x18,

    //! Report Incremental Energy (in 20 watt seconds units)
    ONA_ENERGY_20_WATT_SECONDS = 0x19,

    //! Report Incremental Energy (in 200 watt seconds units)
    ONA_ENERGY_200_WATT_SECONDS = 0x1A,

    //! Report Incremental Energy (in 2000 watt seconds units)
    ONA_ENERGY_2000_WATT_SECONDS = 0x1B,

    //! Report Incremental Energy (in 20000 watt seconds units)
    ONA_ENERGY_20000_WATT_SECONDS = 0x1C,

    //! Accumulated Energy Query 
    ONA_ACCUM_ENERGY = 0x1D,

    //! Report Accumulated Energy (in 2 watt seconds units)
    ONA_ACCUM_ENERGY_2_WATT_SECONDS = 0x1E,

    //! Report Accumulated Energy (in 20 watt seconds units)
    ONA_ACCUM_ENERGY_20_WATT_SECONDS = 0x1F,

    //! Report Accumulated Energy (in 200 watt seconds units)
    ONA_ACCUM_ENERGY_200_WATT_SECONDS = 0x20,

    //! Report Accumulated Energy (in 2000 watt seconds units)
    ONA_ACCUM_ENERGY_2000_WATT_SECONDS = 0x21,

    //! Report Accumulated Energy (in 20000 watt seconds units)
    ONA_ACCUM_ENERGY_20000_WATT_SECONDS = 0x22,
    
    //! Peak Energy Query 
    ONA_PEAK_ENERGY = 0x23,

    //! Report Peak Energy (in 2 watt seconds units)
    ONA_PEAK_ENERGY_2_WATT_SECONDS = 0x24,

    //! Report Peak Energy (in 20 watt seconds units)
    ONA_PEAK_ENERGY_20_WATT_SECONDS = 0x25,

    //! Report Peak Energy (in 200 watt seconds units)
    ONA_PEAK_ENERGY_200_WATT_SECONDS = 0x26,

    //! Report Peak Energy (in 2000 watt seconds units)
    ONA_PEAK_ENERGY_2000_WATT_SECONDS = 0x27,

    //! Report Peak Energy (in 20000 watt seconds units)
    ONA_PEAK_ENERGY_20000_WATT_SECONDS = 0x28,

    //! Instantaneous Gas Query 
    ONA_GAS = 0x29,

    //! Instantaneous Gas Therm-Seconds
    ONA_GAS_THERM_SECS = 0x2A,

    //! Instantaneous Gas Therm-Minutes
    ONA_GAS_THERM_MINS = 0x2B,

    //! Instantaneous Gas Therm-Hours
    ONA_GAS_THERM_HRS = 0x2C,

    //! Accumulated Gas Query 
    ONA_ACCUM_GAS = 0x2D,

    //! Accumulated Gas Therm-Seconds
    ONA_ACCUM_GAS_THERM_SECS = 0x2E,

    //! Accumulated Gas Therm-Minutes
    ONA_ACCUM_GAS_THERM_MINS = 0x2F,

    //! Accumulated Gas Therm-Hours
    ONA_ACCUM_GAS_THERM_HRS = 0x30,

    //! Average Gas Query 
    ONA_AVER_GAS = 0x31,

    //! Average Gas Therm-Seconds
    ONA_AVER_GAS_THERM_SECS = 0x32,

    //! Average Gas Therm-Minutes
    ONA_AVER_GAS_THERM_MINS = 0x33,

    //! Average Gas Therm-Hours
    ONA_AVER_GAS_THERM_HRS = 0x34,

    //! Peak Gas Query 
    ONA_PEAK_GAS = 0x35,

    //! Peak Gas Therm-Seconds
    ONA_PEAK_GAS_THERM_SECS = 0x36,

    //! Peak Gas Therm-Minutes
    ONA_PEAK_GAS_THERM_MINS = 0x37,

    //! Peak Gas Therm-Hours
    ONA_PEAK_GAS_THERM_HRS = 0x38,
    
    //! Power Query
    ONA_POWER = 0x39,

    //! Power 2 Milliwatts
    ONA_POWER_2_MILLIWATTS = 0x3A,
    
    //! Power 20 Milliwatts
    ONA_POWER_20_MILLIWATTS = 0x3B,
    
    //! Power 50 Milliwatts
    ONA_POWER_50_MILLIWATTS = 0x3C,
    
    //! Power 200 Milliwatts
    ONA_POWER_200_MILLIWATTS = 0x3D,
    
    //! Power Watts
    ONA_POWER_WATTS = 0x3E,
    
    //! Power 2 Watts
    ONA_POWER_2_WATTS = 0x3F
} ona_msg_type_t;


//! Packet related constants
enum
{
    //! Length of preamble and header
    ONE_NET_PREAMBLE_HEADER_LEN = 4,
    
    //! Length of encoded message CRC
    ONE_NET_ENCODED_MSG_CRC_LEN = 1,
    
    //! Length of encoded message id
    ONE_NET_ENCODED_MSG_ID_LEN = 1,
    
    //! The size of the encoded Packet ID field
    ON_ENCODED_PID_SIZE = 1,

    //! The number of bytes required to store the raw hops field
    ON_RAW_HOPS_SIZE = 1,

    //! The size of the encoded hops field (in bytes)
    ON_ENCODED_HOPS_SIZE = 1,    
    
    //! The index into the encoded packet where the repeater did lies
    ONE_NET_ENCODED_RPTR_DID_IDX = ONE_NET_PREAMBLE_HEADER_LEN,
    
    //! The index into the encoded packet where the message CRC lies
    ONE_NET_ENCODED_MSG_CRC_IDX = ONE_NET_ENCODED_RPTR_DID_IDX + ON_ENCODED_DID_LEN,
        
    ONE_NET_ENCODED_DST_DID_IDX = ONE_NET_ENCODED_MSG_CRC_IDX + ONE_NET_ENCODED_MSG_CRC_LEN,
        
    //! The index into the encoded packet where the message CRC lies
    ONE_NET_ENCODED_MSG_ID_IDX = ONE_NET_ENCODED_DST_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the NID starts.
    ON_ENCODED_NID_IDX = ONE_NET_ENCODED_MSG_ID_IDX + ONE_NET_ENCODED_MSG_ID_LEN,

    //! The index into the encoded packet where the source DID starts.
    ON_ENCODED_SRC_DID_IDX = ON_ENCODED_NID_IDX + ON_ENCODED_NID_LEN,

    //! The index into the encoded packet where the PID starts.
    ONE_NET_ENCODED_PID_IDX = ON_ENCODED_SRC_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the payload starts
    ON_PLD_IDX = ONE_NET_ENCODED_PID_IDX + ON_ENCODED_PID_SIZE
};


#ifdef _ONE_NET_MULTI_HOP
//! Hops field related constants
enum
{
    //! The number of bits to shift the max hops field
    ON_MAX_HOPS_SHIFT = 5,

    //! Number of bits to shift the hops left field
    ON_HOPS_LEFT_SHIFT = 2,

    //! Mask to use on the max hops field when building the packet
    ON_MAX_HOPS_BUILD_MASK = 0xE0,

    //! Mask to use on the max hops field when parsing the packet
    ON_MAX_HOPS_PARSE_MASK = 0x07,

    //! Mask to use on the hops remaining field when building the packet
    ON_HOPS_LEFT_BUILD_MASK = 0x1C,

    //! Mask to use on the hops remaining field when parsing the packet
    ON_HOPS_LEFT_PARSE_MASK = 0x07
};
#endif


typedef enum
{
    TXN_ACK_PKT_GRP,
    ACK_NACK_PKT_GRP,
    SINGLE_PKT_GRP,
    INVITE_PKT_GRP,
    #ifdef _BLOCK_MESSAGES_ENABLED
    BLOCK_PKT_GRP,
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    STREAM_PKT_GRP,
    #endif
    NUM_PKT_GROUPS
} pkt_group_t;


enum
{
    ON_TXN_ACK_ENCODED_PLD_LEN = 0,
    ON_ACK_NACK_ENCODED_PLD_LEN = 11,
    ON_LARGE_ACK_NACK_ENCODED_PLD_LEN = 22,
    ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN = 33,
    ON_MAX_ENCODED_ACK_NACK_LEN = ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN,
    ON_SINGLE_ENCODED_PLD_LEN = 11,
    ON_LARGE_SINGLE_ENCODED_PLD_LEN = 22,
    ON_EXTENDED_SINGLE_ENCODED_PLD_LEN = 33,
    ON_MAX_ENCODED_SINGLE_LEN = ON_EXTENDED_SINGLE_ENCODED_PLD_LEN,
    ON_INVITE_ENCODED_PLD_LEN = 33,
    #ifdef _BLOCK_MESSAGES_ENABLED
    ON_BLOCK_ENCODED_PLD_LEN = 43,
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    ON_STREAM_ENCODED_PLD_LEN = 43,
    #endif
    
    #ifdef _BLOCK_MESSAGES_ENABLED
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
    ON_TXN_ACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_TXN_ACK_ENCODED_PLD_LEN,
    ON_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_ACK_NACK_ENCODED_PLD_LEN,
    ON_LARGE_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_LARGE_ACK_NACK_ENCODED_PLD_LEN,
    ON_EXTENDED_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_EXTENDED_ACK_NACK_ENCODED_PLD_LEN,
    
    ON_MAX_ACK_NACK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_MAX_ENCODED_ACK_NACK_LEN,
    ON_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_SINGLE_ENCODED_PLD_LEN,
    ON_LARGE_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_LARGE_SINGLE_ENCODED_PLD_LEN,
    ON_EXTENDED_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_EXTENDED_SINGLE_ENCODED_PLD_LEN,
    
    ON_MAX_SINGLE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_MAX_ENCODED_SINGLE_LEN,
    ON_INVITE_ENCODED_PKT_LEN = ON_PLD_IDX + ON_INVITE_ENCODED_PLD_LEN,
    #ifdef _BLOCK_MESSAGES_ENABLED
    ON_BLOCK_ENCODED_PKT_LEN = ON_PLD_IDX + ON_BLOCK_ENCODED_PLD_LEN,
    #endif
    #ifdef _STREAM_MESSAGES_ENABLED
    ON_STREAM_ENCODED_PKT_LEN = ON_PLD_IDX + ON_STREAM_ENCODED_PLD_LEN
    #endif
};


enum
{
    #ifdef _ONE_NET_MULTI_HOP
        ON_ACK_NACK_ENCODED_PKT_SIZE = ON_MAX_ACK_NACK_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        ON_SINGLE_ENCODED_PKT_SIZE = ON_MAX_SINGLE_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #ifdef _BLOCK_MESSAGES_ENABLED
        ON_BLOCK_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        ON_STREAM_ENCODED_PKT_SIZE = ON_STREAM_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,
        #endif
        ON_INVITE_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_LEN + ON_ENCODED_HOPS_SIZE,              
    #else // ifdef _ONE_NET_MULTI_HOP //
        ON_ACK_NACK_ENCODED_PKT_SIZE = ON_MAX_ACK_NACK_ENCODED_PKT_LEN,
        ON_SINGLE_ENCODED_PKT_SIZE = ON_MAX_SINGLE_ENCODED_PKT_LEN,
        #ifdef _BLOCK_MESSAGES_ENABLED
        ON_BLOCK_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_LEN,
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        ON_STREAM_ENCODED_PKT_SIZE = ON_STREAM_ENCODED_PKT_LEN,
        #endif
        ON_INVITE_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_LEN,
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    
    ON_MIN_ENCODED_PKT_SIZE = ON_ACK_NACK_ENCODED_PKT_SIZE,
    #if defined(_BLOCK_MESSAGES_ENABLED)
    ON_MAX_ENCODED_PKT_SIZE = ON_BLOCK_ENCODED_PKT_SIZE
    #else
    ON_MAX_ENCODED_PKT_SIZE = ON_INVITE_ENCODED_PKT_SIZE
    #endif
};


//! Nonce, Message type, Payload indexes, masks, shifts
enum
{
    //! Index into the payload where the crc starts
    ON_PLD_CRC_IDX = 0,

    //! Index for the transaction nonce
    ON_PLD_TXN_NONCE_IDX = 1,

    //! Index for the high portion of the response nonce
    ON_PLD_RESP_NONCE_HIGH_IDX = 1,

    //! Index for the low portion of the response nonce
    ON_PLD_RESP_NONCE_LOW_IDX = 2,


    //! Index for the message type
    ON_PLD_MSG_TYPE_IDX = 2,

    //! Index for the data portion
    ON_PLD_DATA_IDX = 3,

    // If any of the shift values change, the masks where these values are used
    // will also need to change
    //! Number of bits to shift the transaction nonce
    ON_TXN_NONCE_SHIFT = 2,

    //! Number of bits to shift the high portion of the response nonce
    ON_RESP_NONCE_HIGH_SHIFT = 4,

    //! Number of bits to shift the low portion of the response nonce
    ON_RESP_NONCE_LOW_SHIFT = 4,

    //! The mask to use for the transaction nonce when building the payload
    //! field of a data packet
    ON_TXN_NONCE_BUILD_MASK = 0xFC,

    //! The mask to use for the transaction nonce when parsing the payload
    //! field of a data packet
    ON_TXN_NONCE_PARSE_MASK = 0x3F,

    //! The mask to use for the high portion of the response nonce when
    //! building the payload field of a data packet
    ON_RESP_NONCE_BUILD_HIGH_MASK = 0x03,

    //! The mask to use for the high portion of the response nonce when
    //! parsing the payload field of a data packet
    ON_RESP_NONCE_PARSE_HIGH_MASK = 0x30,

    //! The mask to use for the low portion of the response nonce when
    //! building the payload field of a data packet
    ON_RESP_NONCE_BUILD_LOW_MASK = 0xF0,

    //! The mask to use for the low portion of the response nonce when
    //! parsing the payload field of a data packet
    ON_RESP_NONCE_PARSE_LOW_MASK = 0x0F,

    //! The mask to use for the message type when building or parsing the
    //! payload field of a data packet
    ON_PLD_MSG_TYPE_MASK = 0x0F
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
    //! Bytes 1 and 2 -- Message class and type.  Message class
    //!           (i.e. ONA_COMMAND) are the 4 most significant bits of the
    //!           bits of bytes 1 and 2.  The remaining 12 bits are the
    //!           message type (i.e ONA_SWITCH)
    //! Bytes 3 and 4 -- Message data.  The "data" of the message.  For a
    //!           normal switch message, this would be ONA_ON, ONA_OFF, or
    //!           ONA_TOGGLE
    //!
    //!
    //! Bytes 1 and 2(message type and class) are the "header".
    //!
    //!
    //! So a "Turn switch on command from unit 4 to unit 6 would be as follows
    //!
    //! Source Unit                 = 4 = 0100
    //! Destination Unit            = 6 = 0110
    //! Message Class = ONA_COMMAND = 5 = 0101 (note ONA_COMMAND is 0x5000.
    //!                                         We are isolating the left-most
    //!                                         4 bits.)
    //!
    //! Message Type = ONA_SWITCH   = 0 = 000000000000 (12 bits)
    //! Message Data = ONA_ON       = 1 = 0000000000000001 (16 bits)
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
    //!   The message classes are shifted 12 bits precisely so that the "OR"
    //!   operator will work as above.  ONA_COMMAND is defined as 0x5000, that
    //!   is 5 followed by 12 empty bits.  ONA_SWITCH is 0x000, which is
    //!   exactly 12 bits long.  Thus the message class and message type do
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
    //!   specify "ON_APP_MSG" when creating the message.

    
    
    //! Index of header within single packet payload (header is message
    //! class and message type
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
};


//! constants dealing with the raw payload of a data packet (i.e. does not
//! include nonces, crc, or nack reason.
enum
{
    ONA_DATA_INDEX = 3, //! the index in a data packet where the actual
                        //! data starts
    ONA_SINGLE_PACKET_PAYLOAD_LEN = ONE_NET_XTEA_BLOCK_SIZE -
      ONA_DATA_INDEX, //! the number of data bytes in a single message
    
    #ifdef _EXTENDED_SINGLE
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


//! To save stack space when processing function calls, to simplify things,
//! and for (at times) fewer parameters, this is a structure containing
//! common portions of a packet.  Everything, even UInt8 portions, is a
//! pointer so that a structure can be set up pointing to an array and the
//! array can change, yet the on_pkt_t structure does not need to be changed
//! much, at least the encoded portions.  The pointer to the hops field may
//! change based on the payload_len field, which is based on the pid.
typedef struct
{
    UInt8* packet_header; //! pointer to the preamble and start of frame
    UInt8* pid; //! pid of the packet
    UInt8* enc_msg_id; //! encoded message id of the packet
    UInt8* enc_msg_crc; //! encoded message crc of the packet
    UInt8 msg_id; //! raw message id of the packet
    UInt8 msg_crc; //! raw message crc of the packet
    on_encoded_did_t* enc_src_did; //! encoded source did of the packet
    on_encoded_did_t* enc_dst_did; //! encoded destination did of the packet
    on_encoded_did_t* enc_repeater_did; //! encoded repeater did of the packet
    on_encoded_nid_t* enc_nid; //! encoded nid of the packet
    UInt8* payload; //! encoded payload (if any) of the packet.
    UInt8 payload_len; //! length of the encoded payload in bytes
    #ifdef _ONE_NET_MULTI_HOP
    UInt8* enc_hops_field; //! encoded hops field of the packet
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

SInt8 get_encoded_payload_len(UInt8 pid);
SInt8 get_raw_payload_len(UInt8 pid);
SInt8 get_num_payload_blocks(UInt8 pid);
UInt8 get_encoded_packet_len(UInt8 pid, BOOL include_header);

#ifdef _ONE_NET_MULTI_HOP
BOOL packet_is_multihop(UInt8 pid);
#endif
BOOL packet_is_invite(UInt8 pid);
BOOL packet_is_stream(UInt8 pid);
pkt_group_t get_pkt_family(UInt8 pid);

    
//! @} ONE-NET_PACKET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================


//! @} ONE-NET_PACKET

#endif // _ONE_NET_PACKET_H //
