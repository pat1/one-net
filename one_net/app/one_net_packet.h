#ifndef _ONE_NET_PACKET_H
#define _ONE_NET_PACKET_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_constants.h"


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
        ON_MAX_RAW_PLD_LEN = ON_BLOCK_ENCODED_PLD_LEN - 1
    #else // if block messages are defined //
        //! The maximum length of a raw payload field (not including the extra
        //! byte needed to store the 2 bits for the encryption method type).
        ON_MAX_RAW_PLD_LEN = ON_INVITE_ENCODED_PLD_LEN - 1
    #endif // else if block messages are not defined //
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
UInt8 get_encoded_packet_len(UInt8 pid, BOOL include_header);

#ifdef _ONE_NET_MULTI_HOP
BOOL packet_is_multihop(UInt8 pid);
#endif

pkt_group_t get_pkt_family(UInt8 pid);

    
//! @} ONE-NET_PACKET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================


//! @} ONE-NET_PACKET

#endif // _ONE_NET_PACKET_H //
