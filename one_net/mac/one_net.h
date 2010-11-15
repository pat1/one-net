#ifndef _ONE_NET_H
#define _ONE_NET_H

//! \defgroup ONE-NET ONE-NET
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    \file ONE_NET.h
    \brief ONE-NET declarations.

    Basis for ONE-NET.  Everything in this file is application independent
    (including independent from being a MASTER or a CLIENT).  The only places
    these functions should be called from is in one_net_master.c &
    one_net_client.c.
    
    \note These functions SHOULD NOT be called by any application code!
    
*/

#include "one_net_status_codes.h"
#include "one_net_types.h"
#include "one_net_xtea.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{

//! ONE-NET Version numbers<br>
//! There are two types of ONE-NET vesion numbers. One set of version numbers is used
//! to identify a release of the ONE-NET source code. There are three levels to this
//! source code release version number, major, minor, and revision. These version numbers
//! appear in the one_net.h file as the three #defines ONE_NET_VERSION_MAJOR, ONE_NET_VERSION_MINOR,
//! and ONE_NET_VERSION_REVISION. For example, the source code that was released on CD at Rensas
//! DevCon on 10/13/08 was labeled as ONE_NET_SRC_V1_3_7.zip. This filename was constructed
//! based on the ONE-NET source code release version numbers of 1.3.7 (major=1, minor=3,
//! revision=7). 
//! There is a second type of ONE-NET version number named ON_VERSION. This is a one byte
//! version number which is really a ONE-NET protocol version number. This version number is
//! included in Status Response basic administrative message and is used to verfiy that the
//! master and client are communicating with compatible versions of the ONE-NET protocol.
//! Ideally, we would have only one ONE-NET version number and use that to determine both
//! the source code release version and the ONE-NET protocol compatibility. But ONE-NET 
//! efficiency requires that we use as small a number as possible for the protocol compatibility
//! number so we use one byte. We do not expect this ONE-NET compatibilty version number to 
//! change very often. However, one byte is not sufficient to represent the number of
//! iterations we expect to have for ONE-NET source code. So, we end up with two different
//! ONE-NET version numbers. Most people will be using the source code release version number
//! to identify releases of ONE-NET most of the time. 
//! In order to facilitate final testing of a release candidate, we also include have a 
//! ONE_NET_VERSION_BUILD number. This number will be inremented when a new load is put
//! on any piece of hardware. When a particular load has completed pre-release testing,
//! the build number for the load that was tested becomes the build number for the release.
#define ONE_NET_VERSION_MAJOR       1
#define ONE_NET_VERSION_MINOR       5
#define ONE_NET_VERSION_REVISION    0
#define ONE_NET_VERSION_BUILD       22

//! ONE-NET compile time options<br>
//! Currently (version 1.5.0 and earlier), ONE-NET used the Renesas HEW IDE to specify 
//! compile time options. In the future, we plan to move these options into an include
//! file so that building ONE-NET is less dependent on the IDE being used.
//! In the meantime, below are explanations of some of the compile time options for 
//! controlling the type of load that is built.
//!
//! _EVAL_0005_NO_REVISION<br>
//! This option is used to build a ONE-NET eval board load for the older eval board
//! hardware. This older hardware had a part number of 0005 where newer eval boards
//! have part numbers of the form 06-0005-XX where XX is a revision number. The oder
//! 0005 board layout mapped the user pins to different processor pins than the newer
//! eval boards.
//!
//! _ENABLE_DUMP_COMMAND<br>
//! This option causes the dump command to be included in the CLI. The dump command
//! was used during development of code that accesses data flash. 
//!
//! _ENABLE_LIST_COMMAND<br>
//! This option causes the list command to be included in the CLI. The list command was added 
//! to the CLI to supply information about the current network and i/o pin configuration. 
//!
//! _CLOCK_OUT_DIVIDE_BY_TWO<br>
//! This option is used to build a version that modifies the frequency of the 
//! clock generated by the ADF7025 to make it easier to tune the RF section
//! of the circuit.
//!
//! _ONE_NET_TEST_NACK_WITH_REASON_FIELD<br>
//! This option should only be used when generating a test load for the ONE-NET eval
//! board. The test load generated when this option is specified will send the new
//! NACK with reason field instead of the basic NACK message. This was needed to help
//! ensure that version 1.5.0 of ONE-NET will continue to communicate with devices in
//! in the future running ONE-NET code that uses the NACK with reason field message
//! in place of the basic NACK message included in versions of ONE-NET up through 
//! and including 1.5.0.
//!
//! _ONE_NET_DEBUG<br>
//! This option can be used to turn on debugging of low level ONE-NET operations.
//! For example, when used on an eval board load, debugging information will be
//! sent to the UART. Since the eval board is very tight on program memory, you
//! may need to disable some features such as CLI commands in order to make room
//! for the additional debugging code. The baud rate of the serial port is increased 
//! to 115,200 bps with this option so that debug information has as little impact
//! as possible on performance.
//!
//! _ENABLE_RSINGLE_COMMAND<br>
//! This option enables the rsingle command. It can be used to
//! to send multiple single messages. The syntax of the rsingle
//! command is the same as the syntax of the single command. The only difference is
//! the rsingle command will send the single data message 20 times waiting approximately
//! 1 second between each message.
//!
//! _ENABLE_RSSI_COMMAND<br>
//! This option enables the rssi commmand. The rssi command constantly monitors the RSSI
//! reading from the ADF7025 transceiver using the read_rssi() funtion. Once this command
//! is executed it runs continuously. The board must be reset to stop the command. 
//! Approximately every 16 ticks this command prints the raw RSSI value in hex,
//! followed my bits 4 through 11 of the tick count, followed by 50 C or B charcters
//! representing whether the channel is clear (C) or busy (B). The example below
//! was recorded just after an invite command was issued. The channel command should
//! be used to make sure the eval board running the rssi command is listening to the
//! correct channel.
//! <verbatim>
//! rssi=FFA2 90 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA3 A1 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA3 B2 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCBBBCCCCCC
//! rssi=FFA4 C3 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCBBBCCCCCCCCC
//! rssi=FFA5 D4 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA0 E5 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCBBBCCCCCCCCCCCCCCCCCC
//! rssi=FFA4 F6 CCCCCCCCCCCCCCCCCCCCCCCCCCBBBCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA3 07 CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA1 18 CCCCCCCCCCCCCCCCCBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA2 29 CCCCCCCCCCCCCCBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA6 3A CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA2 4B CCCCCBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! rssi=FFA5 5C CCBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
//! </verbatim>
//!
//! _ONE_NET_DEBUG_STACK<BR>
//! This option can be used to help determine how much of the normal stack is being used.
//! The stack analysis tools provided by Rensas analyze stack usage but ONE_NET code
//! makes us of pointers to functions which cannot be factored into the static analyses
//! performed my the Renesas utilities.
//!

enum
{
    //! The ONE-NET version
    ON_VERSION = 0x04,

    //! The version of the on_base_param_t, on_master_param_t, client_t,
    //! on_master_t and on_peer_t structures
    ON_PARAM_VERSION = 0x0001,
    
    //! The version of the MASTER Invite New CLIENT packet.
    ON_INVITE_PKT_VERSION = 0x00,

    //! The maximum number of times to try a transaction
    ON_MAX_RETRY = 8,

    //! The number of times to send a data rate test packet when permoring a
    //! data rate test
    ON_MAX_DATA_RATE_TRIES = 8,

    //! The max nonce
    ON_MAX_NONCE = 63,

    //! Value to use to mark an invalid nonce
    ON_INVALID_NONCE = 0xFF,

    //! The maximum number of hops
    ON_MAX_HOPS_LIMIT = 7,

    //! Represents an invalid hops value
    ON_INVALID_HOPS = ON_MAX_HOPS_LIMIT + 1,

    //! The number of hops to try when sending the first Multi-Hop packet
    ON_FIRST_MH_MAX_HOPS_COUNT = 2,
    
    //! Size in bytes of a XTEA key fragment
    ONE_NET_XTEA_KEY_FRAGMENT_SIZE = 4
};

//! Address related constants
enum
{
    //! Raw Network ID length (in bytes needed to store the value)
    ONE_NET_RAW_NID_LEN = 5,

    //! Raw Device ID length (in bytes needed to store the value)
    ONE_NET_RAW_DID_LEN = 2,

    //! Raw System ID length (in bytes)
    ONE_NET_RAW_SID_LEN = 6,


    //! Encoded Network ID length (in bytes)
    ON_ENCODED_NID_LEN = 6,

    //! Encoded Device ID length (in bytes)
    ON_ENCODED_DID_LEN = 2,

    //! Encoded SID length (in bytes)
    ON_ENCODED_SID_LEN = 8
};

//! Packet related constants
enum
{
    //! The size of the encoded Packet ID field
    ON_ENCODED_PID_SIZE = 1,

    //! The number of bytes required to store the raw hops field
    ON_RAW_HOPS_SIZE = 1,

    //! The size of the encoded hops field (in bytes)
    ON_ENCODED_HOPS_SIZE = 1,

    //! The length of an encoded Invite MASTER Invite new CLIENT
    ON_ENCODED_INVITE_PKT_LEN = 48,

    //! The length of an encoded single data packet
    ON_ENCODED_SINGLE_DATA_LEN = 26,

    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! The maximum length of a raw payload field (not including the extra
        //! byte needed to store the 2 bits for the encryption method type).
        ON_MAX_RAW_PLD_LEN = 32,
    #else // ifndef _ONE_NET_SIMPLE_CLIENT //
        //! The maximum length of a raw payload field (not including the extra
        //! byte needed to store the 2 bits for the encryption method type).
        ON_MAX_RAW_PLD_LEN = 8,
    #endif // else _ONE_NET_SIMPLE_CLIENT is defined //

    #ifdef _ONE_NET_MULTI_HOP
        //! Maximum length of an encoded packet (in bytes)
        ONE_NET_MAX_ENCODED_PKT_LEN = 58 + ON_ENCODED_HOPS_SIZE,
    #else // ifdef _ONE_NET_MULTI_HOP //
        //! Maximum length of an encoded packet (in bytes)
        ONE_NET_MAX_ENCODED_PKT_LEN = 58,
    #endif // else _ONE_NET_MULTI_HOP is not defined //

    //! The length of an ack/nack packet (in bytes)
    ON_ACK_NACK_LEN = 17,

    //! The length of a transaction ack packet (in bytes)
    ON_TXN_ACK_LEN = 15,

    #ifdef _ONE_NET_MULTI_HOP
        //! The minimum length the size of the data location that stores packets
        //! must be (in bytes).  This is not the length of the actual packet
        //! (which may be shorter than this value).
        ON_MIN_ENCODED_PKT_SIZE = ON_ACK_NACK_LEN + ON_ENCODED_HOPS_SIZE,
    #else // ifdef _ONE_NET_MULTI_HOP //
        //! The minimum length the size of the data location that stores packets
        //! must be (in bytes).  This is not the length of the actual packet
        //! (which may be shorter than this value).
        ON_MIN_ENCODED_PKT_SIZE = ON_ACK_NACK_LEN,
    #endif // else _ONE_NET_MULTI_HOP is not defined //

    //! The length of an encoded data rate packet
    ON_DATA_RATE_PKT_LEN = 20,

    //! The index into the encoded packet where the destination DID starts.
    ONE_NET_ENCODED_DST_DID_IDX = 4,

    //! The index into the encoded packet where the NID starts.
    ON_ENCODED_NID_IDX = ONE_NET_ENCODED_DST_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the source DID starts.
    ON_ENCODED_SRC_DID_IDX = ON_ENCODED_NID_IDX + ON_ENCODED_NID_LEN,

    //! The index into the encoded packet where the PID starts.
    ONE_NET_ENCODED_PID_IDX = ON_ENCODED_SRC_DID_IDX + ON_ENCODED_DID_LEN,

    //! The index into the encoded packet where the payload starts
    ON_PLD_IDX = 15
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
    ON_INVITE_VERSION_IDX = 0,

    //! The assigned DID index
    ON_INVITE_ASSIGNED_DID_IDX = 1,

    //! The key index
    ON_INVITE_KEY_IDX = 3,

    //! The keep alive index
    ON_INVITE_KEEP_ALIVE_IDX = 19,

    //! The crc index
    ON_INVITE_CRC_IDX = 23
};

//! Payload & response packet related constants
enum
{
    //! The size of the payload header (crc, txn & response nonce,
    //! message type) in bytes.
    ON_RAW_PLD_HDR_SIZE = 3,


    //! Length of the payload field in a raw single data packet (in bytes needed
    //! to store all the information)
    ON_RAW_SINGLE_PLD_SIZE = 9,

    //! Length of the payload field in an encoded single data packet (in bytes)
    ON_ENCODED_SINGLE_PLD_SIZE = 11,

    //! Length of the actual app/admin data portion of a raw single packet
    //! (in bytes)
    ONE_NET_RAW_SINGLE_DATA_LEN = 5,

    //! Length of the payload field in block and stream packets (in bytes needed
    //! to store all the information)
    ON_RAW_BLOCK_STREAM_PLD_SIZE = 33,

    //! Length of the payload field in block and stream packets (in bytes)
    ON_ENCODED_BLOCK_STREAM_PLD_SIZE = 43,

    //! Length of the actual app/admin data portion of a raw block or stream
    //! packet (in bytes)
    ONE_NET_RAW_BLOCK_STREAM_DATA_LEN = 29,

    //! The maximum number of bytes that can be sent in an entire block
    //! transaction.
    ON_MAX_BLOCK_TRANSFER_LEN = 65535,

    //! The maximum admin payload size (in bytes)
    ON_MAX_ADMIN_PLD_LEN = 4,

    //! The length (in bytes) of the raw data (nonces) in a response packet that
    //! includes the nonces
    ON_RESP_NONCE_LEN = 2,

#ifdef _ONE_NET_TEST_NACK_WITH_REASON_FIELD
    //! The length (in bytes) of the raw data (nonces) in a response packet that
    //! includes the nonces and the NACK reason field
    ON_RESP_NACK_WITH_REASON_LEN = 3,
#endif

    //! The size (in 6 or 8 bit words) of the raw data (nonces) in a response
    //! packet that includes the nonces
    ON_RESP_NONCE_WORD_SIZE = 2
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

//! Payload and Response Parsing Constants
enum
{
    //! Index for the transaction nonce in a response packet
    ON_RESP_TXN_NONCE_IDX = 0,

    //! Index for the high portion of the response nonce in a response packet
    ON_RESP_RESP_NONCE_HIGH_IDX = 0,

    //! Index for the low portion of the response nonce in a response packet
    ON_RESP_RESP_NONCE_LOW_IDX = 1,

#ifdef _ONE_NET_TEST_NACK_WITH_REASON_FIELD

    //! To pack the 6 bit NACK reason field into the NACK message paylaod field,
    //! the following steps are used:<br>
    //! bits 2 to 5 (high order) of the NACK reason field are packed into bits 0 to 3 of the 
    //! second byte of the NACK message payload.<br>
    //! bits 0 to 1 (low order) of the NACK reason field are packed into bits 6 to 7 of the
    //! third byte of the NACK message payload.<br>
    //!<p>
    //! index of the byte in the destination paylaod that will contain the high order
    //! portion of the NACK reason field.
    ON_RESP_NACK_RSN_HIGH_IDX = 1,

    //! index of the byte in the destination payload that will contain the low order
    //! portion of the NACK reason field.
    ON_RESP_NACK_RSN_LOW_IDX = 2,

    //! number of bits of the NACK reason field to shift right when packing it into
    //! the second byte of the NACK message payload.
    ON_RESP_NACK_RSN_BUILD_HIGH_SHIFT = 2,

    //! the mask to use for the high order portion of the NACK reason field after
    //! it has been shifted before it is or'd into the second byte of the 
    //! NACK message payload.
    ON_RESP_NACK_RSN_BUILD_HIGH_MASK = 0x0f,

    //! number of bits of the NACK reason field to shift left when packing it into
    //! the third byte of the NACK message payload.
    ON_RESP_NACK_RSN_BUILD_LOW_SHIFT = 6,

    //! the mask to use for the low order portion of the NACK reason field after
    //! it has been shifted before it is or'd into the third byte of the 
    //! NACK message payload.
    ON_RESP_NACK_RSN_BUILD_LOW_MASK = 0xc0,

#endif


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

/*!
    \brief Admin packet related constants

    The index into the data fields are based on building the data field, not
    an index into the received packet which has the admin message id
    preceding the data field.  For indexing the data fields in received
    packets, ADMIN_DATA_IDX will need to be added to the data field indexes.
*/
enum
{
    //! Index into an admin message for the message id
    ON_ADMIN_MSG_ID_IDX = 0,

    //! Index into an admin message where the data starts.
    ON_ADMIN_DATA_IDX = 1,


    //! Index for version field in Status Response message
    ON_STATUS_VER_IDX = 0,

    //! Index for Max Data Rate field in Status Response message
    ON_STATUS_MAX_DATA_RATE_IDX = 1,

    //! Index for featurs field in Status Response message
    ON_STATUS_FEATURES_IDX = 2,


    //! Index for data rate in a Settings message
    ON_SETTINGS_DATA_RATE_IDX = 0,

    //! Index for MASTER data rate in Settings message
    ON_SETTINGS_MASTER_DATA_RATE_IDX = 1,

    //! Index for flags in Settings message
    ON_SETTINGS_FLAG_IDX = 2,


    //! Index for encoded did of peer device for (un)assignment of peers
    ON_PEER_DID_IDX = 0,

    //! Index for peer unit for (un)assignment of peers.  This is the unit in
    //! the peer device being assigned to a unit in this device
    ON_PEER_PEER_UNIT_IDX = 2,

    //! Index for destination unit for (un)assignment of peers.  This is the
    //! unit in the device that is being assigned a peer
    ON_PEER_DST_UNIT_IDX = 3,

    //! Index for the peer data rate field in a peer setting packet
    ON_PEER_SETTING_DATA_RATE_IDX = 2,


    //! Index for the data type for the block/stream request admin transaction
    ON_BLOCK_STREAM_DATA_TYPE_IDX = 0,

    //! Index for the block length for the block request admin transaction.
    ON_BLOCK_LEN_IDX = 2,


    //! Index for the data rate for data rate test message
    ON_DATA_RATE_DATA_RATE_IDX = 0,

    //! Index for the did in a data rate test message
    ON_DATA_RATE_DID_IDX = 1,

    //! Index for the Flags field in Initiate Data Rate Test messages
    ON_DATA_RATE_FLAG_IDX = 3,

    //! Index for the data rate result in a data rate test message
    ON_DATA_RATE_RESULT_IDX = 3,
};

//! Flags for the features the device supports (for FEATURES field in Status
//! Response admin packets).
enum
{
    ON_MH_REPEATER = 0x40,          //!< Multi-Hop Repeater
    ON_MH_CAPABLE = 0x20,           //!< Device is capable of rx/tx MH packets
    ON_MAC_FEATURES = 0x10          //!< Block/Stream capable flag.
};

//! flags for settings admin packet 
enum
{
    //! Flag set when the device is part of the network
    ON_JOINED = 0x80,

    //! Flag to indicate a CLIENT should send a message that it sent to its peer
    //! to the MASTER too.
    ON_SEND_TO_MASTER = 0x40
};

//! Flags for Initiate Data Rate admin packets
enum
{
    //! Set if the data rate test should be a Multi-Hop data rate test.
    ON_MH_DATA_RATE_TEST_FLAG = 0x80
};

//! Data Rate constants
enum
{
    //! The size of the data rate field in 6 or 8 bit words
    ON_DATA_RATE_WORD_SIZE = 1,

    //! The index of the data rate field in the payload of a data rate packet
    ON_DATA_RATE_IDX = 15,

    //! The number of bits to shift the raw data rate before encoding or after
    //! decoding
    ON_DATA_RATE_SHIFT = 2,

    //! Index of the start of the test pattern in a data rate packet
    ON_TEST_PATTERN_IDX = 1,

    //! Size of the test pattern in bytes
    ON_TEST_PATTERN_SIZE = 4,

    //! The test pattern
    ON_TEST_PATTERN = 0xE1
};

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

enum
{
    //! Unit number that refers to device as a whole
    ONE_NET_DEV_UNIT = 0x0F
};
    
//! @} ONE-NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_typedefs
//! \ingroup ONE-NET
//! @{

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
    //! tells the sender of the single data packet to listen for a single data
    //! packet from the CLIENT that received the single data packet.
    ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE = 0xB3,

    //! Acknowledges that a single data packet was received, but not 
    //! successfully parsed.
    ONE_NET_ENCODED_SINGLE_DATA_NACK = 0xBA,

    //! The sender of the single data is acknowledging that the transaction is
    //! complete (necessary acks received), and that the receiver can stop
    //! listening
    ONE_NET_ENCODED_SINGLE_TXN_ACK = 0xB5,

    //! Single Data Packet.
    ONE_NET_ENCODED_SINGLE_DATA = 0xB9,

    //! Repeat of a single data packet
    ONE_NET_ENCODED_REPEAT_SINGLE_DATA = 0xB6,

    //! Block Data Packet
    ONE_NET_ENCODED_BLOCK_DATA = 0xB2,

    //! Repeat of a block data packet
    ONE_NET_ENCODED_REPEAT_BLOCK_DATA = 0xC4,

    //! Acknowledgment of successful reception of a single data packet.
    ONE_NET_ENCODED_BLOCK_DATA_ACK = 0xCC,

    //! Acknowledges that a block data packet was received, but was not
    //! successfully parsed
    ONE_NET_ENCODED_BLOCK_DATA_NACK = 0xC3,

    //! The sender of the block data is acknowledging that the transaction is
    //! complete (necessary acks received), and that the receiver can stop
    //! listening
    ONE_NET_ENCODED_BLOCK_TXN_ACK = 0xCA,

    //! Stream Data Packet
    ONE_NET_ENCODED_STREAM_DATA = 0xC5,

    //! Sent by the receiver of a stream data packet to alert the sender that
    //! it is still receiving the stream.
    ONE_NET_ENCODED_STREAM_KEEP_ALIVE = 0xC9,

    //! Sent during a data rate test.
    ONE_NET_ENCODED_DATA_RATE_TEST = 0xC6,


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
    ONE_NET_ENCODED_MH_SINGLE_DATA_NACK = 0x5A,

    //! Multi-hop version of The sender of the single data acknowledging that
    //! the transaction is complete (necessary acks received), and that the
    //! receiver can stop listening
    ONE_NET_ENCODED_MH_SINGLE_TXN_ACK = 0x55,

    //! Multi-hop version of Single Data Packet.
    ONE_NET_ENCODED_MH_SINGLE_DATA = 0x59,

    //! Multi-hop version of Repeat of a single data packet
    ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA = 0x56,

    //! Multi-hop version of Block Data Packet
    ONE_NET_ENCODED_MH_BLOCK_DATA = 0x52,

    //! Multi-hop version of Repeat of a block data packet
    ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA = 0x94,

    //! Multi-hop version of Acknowledgment of successful reception of a single
    //! data packet.
    ONE_NET_ENCODED_MH_BLOCK_DATA_ACK = 0x9C,

    //! Multi-hop version of Acknowledgment that a block data packet was
    //! received, but was not successfully parsed
    ONE_NET_ENCODED_MH_BLOCK_DATA_NACK = 0x93,

    //! Multi-hop version of The sender of the block data acknowledging that
    //! the transaction is complete (necessary acks received), and that the
    //! receiver can stop listening
    ONE_NET_ENCODED_MH_BLOCK_TXN_ACK = 0x9A,

    //! Multi-hop version Stream Data Packet
    ONE_NET_ENCODED_MH_STREAM_DATA = 0x95,

    //! Multi-hop version of Sent by the receiver of a stream data packet to
    //! alert the sender that it is still receiving the stream.
    ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE = 0x99,

    //! Multo hop version of data rate test.
    ONE_NET_ENCODED_MH_DATA_RATE_TEST = 0x96,

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


/*!
    \brief Data Packet Message Types
*/
typedef enum
{
    ON_APP_MSG,                     //!< Application message type
    ON_ADMIN_MSG,                   //!< Admin message type
    ON_EXTENDED_ADMIN_MSG,          //!< Extended admin message type (for block)
    
    ON_INVALID_MSG_TYPE             //!< Indicates the message type is not valid
} on_msg_type_t;


/*!
    \brief Raw Admin Message Type
*/
typedef enum
{
    //! Queries the status of the device
    ON_STATUS_QUERY = 0x00,

    //! Response to a Status Query.  This is also the message sent when a device
    //! hears a MASTER Invite New CLIENT when it is joining the network to let
    //! the MASTER know it has joined the network, and it's capabilities.
    ON_STATUS_RESP,

    //! Queries a device for it's communication settings.
    ON_SETTINGS_QUERY,

    //! Response to a Settings Query.  This is also the response to a Change
    //! Settings so the MASTER knows that the changes it proposed took affect.
    ON_SETTINGS_RESP,

    //! Sent to change a device's settings.  The devices settings should not
    //! be considered changed until a SETTINGS_RESP is received.
    ON_CHANGE_SETTINGS,

    //! Query for the devices fragment delay.
    ON_FRAGMENT_DELAY_QUERY,

    //! Response to a fragment delay query or change fragment delay
    ON_FRAGMENT_DELAY_RESP,

    //! Sent to change a devices low priority fragment delay
    ON_CHANGE_LOW_FRAGMENT_DELAY,

    //! Query for the Keep Alive Timeout.  This is the interval at which a
    //! CLIENT must attempt to check in with the MASTER.  Any communication
    //! with the MASTER resets this timer.
    ON_KEEP_ALIVE_QUERY,

    //! Response to the Keep Alive Query.  This is how often a client attempts
    //! to check in with the MASTER.  Any communication with the MASTER resets
    //! this timer.
    ON_KEEP_ALIVE_RESP,

    //! Sent to change the keep alive interval
    ON_CHANGE_KEEP_ALIVE,

    //! Sent by the MASTER to update a CLIENT's key.  The key is updated by
    //! removing the upper 32 bits, and appending the 32 bits in this message
    //! to the remaining 96 bits of the key.
    ON_NEW_KEY_FRAGMENT,

    //! Sent by the MASTER to assign a peer to the receiving CLIENT.  The CLIENT
    //! can then send directly to the peer.
    ON_ASSIGN_PEER,

    //! Sent by the MASTER to un-assign a peer from the receiving CLIENT.  The
    //! CLIENT must not send directly to that peer anymore.
    ON_UNASSIGN_PEER,

    //! Sent by a device that wishes to receive a low priority block transaction
    //! from the device that receives this message.
    ON_SEND_BLOCK_LOW,

    //! Sent by a device that wishes to send a low priority block transaction to
    //! the device that receives this message.
    ON_RECV_BLOCK_LOW,

    //! Sent by a device that wishes to receive a low priority stream
    //! transaction from the device that receives this message
    ON_SEND_STREAM_LOW,

    //! Sent by a device that wishes to send a low priority stream transaction
    //! to the device that receives this message.
    ON_RECV_STREAM_LOW,

    //! Sent by a device to end a stream transaction
    ON_END_STREAM,

    //! The MASTER is initiating a data rate test with a CLIENT.
    ON_INIT_DATA_RATE_TEST,

    //! The test result of a data rate test.
    ON_DATA_RATE_RESULT,

    //! Not used
    ON_ADMIN_UNUSED0,

    //! Not Used
    ON_ADMIN_UNUSED1,

    //! Not Used
    ON_ADMIN_UNUSED2,

    //! Not Used
    ON_ADMIN_UNUSED3,

    //! Not Used
    ON_ADMIN_UNUSED4,

    //! Sent to update the data rate to use when sending to a specific peer.
    ON_CHANGE_PEER_DATA_RATE,

    //! Sent to change a devices high priority fragment delay
    ON_CHANGE_HIGH_FRAGMENT_DELAY,

    //! Sent by a device that wishes to receive a high priority block
    //! transaction from the device that receives this message.
    ON_SEND_BLOCK_HIGH,

    //! Sent by a device that wishes to send a high priority block
    //! transaction to the device that receives this message.
    ON_RECV_BLOCK_HIGH,

    //! Sent by a device that wishes to receive a high priority stream
    //! transaction from the device that receives this message
    ON_SEND_STREAM_HIGH,

    //! Sent by a device that wishes to send a high priority stream
    //! transaction to the device that receives this message.
    ON_RECV_STREAM_HIGH,

    //! Same as the ASSIGN_PEER message with the addition that it is alerting
    //! the receiver that the assigned peer has Multi-Hop capability.
    ON_ASSIGN_MH_PEER,
    
    //! Queries the MASTER for the stream key
    ON_STREAM_KEY_QUERY,

    //! Sent by the MASTER when it is removing the receiver from the network
    ON_RM_DEV
} on_admin_msg_t;


/*!
    Extended admin types
*/
typedef enum
{
    //! Changes the stream key
    ON_CHANGE_STREAM_KEY
} on_extended_admin_msg_t;


//! Data rate type
typedef enum
{
    ONE_NET_DATA_RATE_38_4,         //!< 38400 bps
    ONE_NET_DATA_RATE_76_8,         //!< 76800 bps
    ONE_NET_DATA_RATE_115_2,        //!< 115200 bps
    ONE_NET_DATA_RATE_153_6,        //!< 153600 bps
    ONE_NET_DATA_RATE_192_0,        //!< 192000 bps
    ONE_NET_DATA_RATE_230_4,        //!< 230400 bps

    //! 1 more than the max data rate.  Data rates must be added before this
    //! value
    ONE_NET_DATA_RATE_LIMIT
} on_data_rate_t;
// TODO: should this now be called one_net_data_rate_t since data rates are now in the public interface
//       for the one-net functions that were added to support Blue Spot


//! Data type being sent/received
typedef enum
{
    ON_NO_TXN,                      //!< No data being sent or received
    ON_INVITE,                      //!< CLIENT Invitation packet.
    ON_DATA_RATE_TXN,               //!< Carrying out a data rate test
    ON_SINGLE,                      //!< Sending a single data packet
    ON_STREAM,                      //!< Sending a stream data packet
    ON_BLOCK                        //!< Sending a block data packet
} on_data_t;


/*!
    \brief Priority Levels

    \Note Changing these may cause the implementation of the Fragment delays
      admin messages to be changed.
*/
typedef enum
{
    ONE_NET_NO_PRIORITY,            //!< No current transaction
    ONE_NET_LOW_PRIORITY,           //!< Low priority transaction
    ONE_NET_HIGH_PRIORITY,          //!< High priority transaction
    ONE_NET_SEND_SINGLE_PRIORITY = ONE_NET_HIGH_PRIORITY

} on_priority_t;


/*!
    \brief States.

    Changing the states will likely require changing the code were they are
    used. All the WRITE_WAIT states need to be 1 more than the send states.
    All the states after the WRITE_WAIT states need to be 1 more than the
    WRITE_WAIT states.
*/
typedef enum
{
    //! Listen for single, block, or stream (or repeat) data packets
    ON_LISTEN_FOR_DATA,


    //! Sends a packet that does not expect a response back
    ON_SEND_PKT = 10,

    //! Waits for the write to end
    ON_SEND_PKT_WRITE_WAIT,


    //! Sends a Single Data Packet
    ON_SEND_SINGLE_DATA_PKT = 20,

    //! Waits for the write to end
    ON_SEND_SINGLE_DATA_WRITE_WAIT,

    //! Waits for the response to a single data packet
    ON_WAIT_FOR_SINGLE_DATA_RESP,


    //! State to send the single data response
    ON_SEND_SINGLE_DATA_RESP = 30,

    //! Waits for the write to end
    ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT,

    //! Listens for the end of a single transaction (txn ack, or next single
    //! data packet)
    ON_WAIT_FOR_SINGLE_DATA_END,


    //! Sends a block data packet
    ON_SEND_BLOCK_DATA_PKT = 40,

    //! Waits for the write to end
    ON_SEND_BLOCK_DATA_WRITE_WAIT,

    //! Waits for the response to a block data packet
    ON_WAIT_FOR_BLOCK_DATA_RESP,


    //! State to send the block data response
    ON_SEND_BLOCK_DATA_RESP = 50,

    //! Waits for the write to end
    ON_SEND_BLOCK_DATA_RESP_WRITE_WAIT,

    //! Listens for the end of the block transaction
    ON_WAIT_FOR_BLOCK_DATA_END,


    //! Sends a stream data packet
    ON_SEND_STREAM_DATA_PKT = 60,

    //! Waits for the write to end
    ON_SEND_STREAM_DATA_WRITE_WAIT,

    //! Waits for the response to a stream data packet
    ON_WAIT_FOR_STREAM_DATA_RESP,


    //! State to send the stream data response
    ON_SEND_STREAM_DATA_RESP = 70,

    //! Waits for the write to end
    ON_SEND_STREAM_DATA_RESP_WRITE_WAIT,


    //! State to set up to send a data rate test
    ON_INIT_SEND_DATA_RATE = 80,

    //! Send the data rate packet
    ON_SEND_DATA_RATE,

    //! Waits for the write to end
    ON_SEND_DATA_RATE_WRITE_WAIT,

    //! Receive the response data rate packet
    ON_RX_DATA_RATE_RESP,


    //! State to set up to receive a data rate test
    ON_INIT_RX_DATA_RATE = 90,

    //! Receives the data rate packet
    ON_RX_DATA_RATE,

    //! Sends the data rate response
    ON_SEND_DATA_RATE_RESP,

    //! Waits for the write to end
    ON_SEND_DATA_RATE_RESP_WRITE_WAIT,


    //! State when the device has not yet joined the netwrok.  If the device is
    //! a client, it looks for the invite from the MASTER.  If the device is a
    //! MASTER, it looks for a clear channel to establish its network on.
    ON_JOIN_NETWORK = 500,


    //! The default state when the device starts up (to ensure the proper
    //! initialization routines are called)
    ON_INIT_STATE = 1000
} on_state_t;

/*!
    \brief Encryption method used to encrypt single and block transactions

    The encryption method for a packets payload is only 2 bits.  These
    enumerations represent the 2 bit field indicating what method of
    encryption was used (already shifted to their position in the payload).
*/
typedef enum
{
    //! No encryption used. DEBUG ONLY
    ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE = 0x00,

    //! 32 round XTEA
    ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32 = 0x40,

    //! TBD
    ONE_NET_SINGLE_BLOCK_ENCRYPT_TBD0 = 0x80,

    //! TBD
    ONE_NET_SINGLE_BLOCK_ENCRYPT_TBD1 = 0xC0
} one_net_single_blk_encryption_t;


/*!
    \brief Encryption method used to encrypt stream transactions

    The encryption method for a packets payload is only 2 bits.  These
    enumerations represent the 2 bit field indicating what method of
    encryption was used (already shifted to their position in the payload).
*/
typedef enum
{
    //! No encryption used. DEBUG ONLY
    ONE_NET_STREAM_ENCRYPT_NONE = 0x00,

    //! 32 round XTEA
    ONE_NET_STREAM_ENCRYPT_XTEA8 = 0x40,

    //! TBD
    ONE_NET_STREAM_ENCRYPT_TBD0 = 0x80,

    //! TBD
    ONE_NET_STREAM_ENCRYPT_TBD1 = 0xC0
} one_net_stream_encryption_t;


//! type of a raw Network ID
typedef UInt8 one_net_raw_nid_t[ONE_NET_RAW_NID_LEN];

//! type of a raw Device ID
typedef UInt8 one_net_raw_did_t[ONE_NET_RAW_DID_LEN];

//! type of a raw System ID
typedef UInt8 one_net_raw_sid_t[ONE_NET_RAW_SID_LEN];

//! type of an encoded Network ID
typedef UInt8 on_encoded_nid_t[ON_ENCODED_NID_LEN];

//! type of an encoded Device ID
typedef UInt8 on_encoded_did_t[ON_ENCODED_DID_LEN];

//! type of an encoded System ID
typedef UInt8 on_encoded_sid_t[ON_ENCODED_SID_LEN];

//! type of the XTEA key fragment
typedef UInt8 one_net_xtea_key_fragment_t[ONE_NET_XTEA_KEY_FRAGMENT_SIZE];

/*!
    \brief Contains the base set of parameters needed to run.

    These are the parameters needed by devices if they are already part of the
    network.
*/
typedef struct
{
    //! crc over the parameters
    UInt8 crc;

    //! Version of the on_base_param_t, on_master_param_t, client_t,
    //! on_master_t, and on_peer_t structures
    UInt16 version;

    //! The encoded address it had been assigned
    on_encoded_sid_t sid;

    //! The channel the network is on
    UInt8 channel;

    //! Data rate the device receives at
    UInt8 data_rate;

    //! The current xtea key being used.
    one_net_xtea_key_t current_key;

    //! Method to encrypt single or block data
    UInt8 single_block_encrypt;

    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! Key to use for stream data transfers
        one_net_xtea_key_t stream_key;

        //! Method used to encrypt stream data
        UInt8 stream_encrypt;

        //! Low priority fragment delay
        tick_t fragment_delay_low;

        //! High priority fragment delay
        tick_t fragment_delay_high;
    #endif
} on_base_param_t;

/*!
    \brief Info for receiving from a device.

    This structure holds the information needed to receive from a device.
*/
typedef struct
{
    on_encoded_did_t did;           //!< Encoded Device ID of the sender

    UInt8 expected_nonce;           //!< The nonce expected.
    UInt8 last_nonce;               //!< The last nonce received.

    UInt8 send_nonce;               //!< nonce to use in case the device sends
                                    //!< a response txn to the sender.

    #ifdef _ONE_NET_MULTI_HOP
        UInt8 max_hops;             //!< max # of hops if sending back to sender
    #endif // ifdef _ONE_NET_MULTI_HOP //
} on_sending_device_t;

//! Transaction structure
typedef struct
{
    //! The priority of the transaction
    UInt8 priority;

    //! How many times this txn has been tried
    UInt8 retry;

    //! The txn nonce we expect from the recipient
    UInt8 expected_nonce;
    
    //! The type of message.  See msg_type_t for values
    UInt8 msg_type;

    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! TRUE if sending this transaction, FALSE if receiving
        BOOL send;

        //! The timer that contains the time the next block/stream transaction
        //! is supposed to occur if this is a block or stream transaction.
        UInt8 next_txn_timer;

        //! The number of bytes remaining to be transferred in a block txn, or
        //! the continue flag for the stream transaction (1 - continue with
        //! transaction, 0 - end transaction).
        UInt16 remaining;
    #endif // ifndef _ONE_NET_SIMPLE_CLIENT //

    #ifdef _ONE_NET_MULTI_HOP
        //! Indicates if the transaction being sent is a Multi-Hop transaction
        UInt8 max_hops;
    #endif // ifdef _ONE_NET_MULTI_HOP //

    //! The length of the data the packet contains (in bytes).  This will not
    //! contain the hops field, so if a multihop packet is being sent, the
    //! hop field size will need to be added to this value.  This value can
    //! also be used to find the hops field in the packet since the hops field
    //! is at a different index for each packet type
    UInt8 data_len;

    //! The size of the location pkt points to (in bytes)
    UInt8 pkt_size;

    //! The packet to be sent
    UInt8 * pkt;
} on_txn_t;


#ifdef _ONE_NET_MULTI_HOP
    //! Packet Handling Function for single or block transactions
    typedef one_net_status_t (*on_pkt_hdlr_t)(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn, const UInt8 HOPS_TAKEN);

    //! Transaction handler
    typedef one_net_status_t (*on_txn_hdlr_t)(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS,
      const UInt8 HOPS_TAKEN);
#else // ifdef _ONE_NET_MULTI_HOP //
    //! Packet Handling Function for single or block transactions
    typedef one_net_status_t (*on_pkt_hdlr_t)(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, UInt8 * const pld,
      on_txn_t ** txn);

    //! Transaction handler
    typedef one_net_status_t (*on_txn_hdlr_t)(on_txn_t ** txn,
      const UInt8 NEXT_NONCE, const one_net_status_t STATUS);
#endif // else _ONE_NET_MULTI_HOP is not defined //

//! Data rate results handler
typedef void (*on_data_rate_result_hdlr_t)(const UInt8 RATE,
  const on_encoded_did_t * const DID, const UInt8 RESULT);

//! The set of needed packet handlers
typedef struct
{
    //! Single Data Packet Handler
    on_pkt_hdlr_t single_data_hdlr;

    //! Single transaction handler
    on_txn_hdlr_t single_txn_hdlr;

    #ifndef _ONE_NET_SIMPLE_CLIENT
        //! block data packet handler
        on_pkt_hdlr_t block_data_hdlr;

        //! block transaction handler
        on_txn_hdlr_t block_txn_hdlr;

        //! stream data packet handler
        on_pkt_hdlr_t stream_data_hdlr;

        //! stream transaction handler
        on_txn_hdlr_t stream_txn_hdlr;
    #endif // ifdef _ONE_NET_SIMPLE_CLIENT //

    //! Data Rate Test results handler
    on_data_rate_result_hdlr_t data_rate_hdlr;
} on_pkt_hdlr_set_t;

//! @} ONE-NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_pub_var
//! \ingroup ONE-NET
//! @{

//! The encoded broadcast did.
extern const on_encoded_did_t ON_ENCODED_BROADCAST_DID;

//! @} ONE-NET_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{

void one_net_init(const on_pkt_hdlr_set_t * const PKT_HDLR);

// address functions
BOOL on_is_my_NID(const on_encoded_nid_t * const NID);
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS);
one_net_status_t on_validate_dst_DID(const on_encoded_did_t * const DID);

// encryption functions
one_net_status_t on_encrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const key);
one_net_status_t on_decrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const key);

// packet reception functions
one_net_status_t on_rx_data_pkt(const on_encoded_did_t * const EXPECTED_SRC_DID,
  on_txn_t ** txn);

// functions to build packets
one_net_status_t on_build_nonces(UInt8 * const data, const UInt8 TXN_NONCE,
  const UInt8 RESP_NONCE);
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_admin_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 MSG_ID,
      const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 RESP_NONCE,
      const UInt8 * const RAW_DATA, const UInt8 DATA_LEN,
      const one_net_xtea_key_t * const KEY, const UInt8 MAX_HOPS);
    one_net_status_t on_build_data_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 PID,
      const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
      const UInt8 RESP_NONCE, const UInt8 * const RAW_DATA,
      const UInt8 DATA_LEN, const one_net_xtea_key_t * const KEY,
      const UInt8 MAX_HOPS);
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE, const UInt8 MAX_HOPS);
    one_net_status_t on_build_data_rate_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const on_encoded_did_t * const ENCODED_DST, UInt8 data_rate,
      const UInt8 MAX_HOPS);
    one_net_status_t on_build_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 * const RAW_DATA, const UInt8 DATA_WORD_SIZE,
      const UInt8 MAX_HOPS);
    one_net_status_t on_build_hops(UInt8 * const hops, const UInt8 MAX_HOPS,
      const UInt8 HOPS_LEFT);
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_admin_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 MSG_ID,
      const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 RESP_NONCE,
      const UInt8 * const RAW_DATA, const UInt8 DATA_LEN,
      const one_net_xtea_key_t * const KEY);
    one_net_status_t on_build_data_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 PID,
      const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
      const UInt8 RESP_NONCE, const UInt8 * const RAW_DATA,
      const UInt8 DATA_LEN, const one_net_xtea_key_t * const KEY);
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE);
    one_net_status_t on_build_data_rate_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const on_encoded_did_t * const ENCODED_DST, UInt8 data_rate);
    one_net_status_t on_build_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 * const RAW_DATA, const UInt8 DATA_WORD_SIZE);
#endif // else _ONE_NET_MULTI_HOP has not been defined // 

// functions to parse packets
one_net_status_t on_parse_pld(UInt8 * const txn_nonce, UInt8 * const resp_nonce,
  UInt8 * const msg_type, UInt8 * const pld, const UInt8 DATA_TYPE,
  const one_net_xtea_key_t * const KEY);
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_read_and_parse_hops(UInt8 * const max_hops,
      UInt8 * const hops_left);
    UInt8 on_hops_taken(void);
#endif // ifdef _ONE_NET_MULTI_HOP //

// transaction functions
void on_update_next_txn_time(on_txn_t * const txn);

// the main function
BOOL one_net(on_txn_t ** txn);

#if defined(_ONE_NET_DEBUG) 
    enum
    {
        ONE_NET_DEBUG_RF_WRITE = 1,
        ONE_NET_DEBUG_ONS_BAD_PKT_TYPE = 2,
        ONE_NET_DEBUG_NACK_WITH_RSN_TEST = 3,   // raw nonces and reason
        ONE_NET_DEBUG_NACK_WITH_RSN2_TEST = 4,  // encoded nonces and reason
        ONE_NET_DEBUG_NACK_WITH_RSN3_TEST = 5,  // txn nonce
        ONE_NET_DEBUG_NACK_WITH_RSN4_TEST = 6,  // response nonce
        ONE_NET_DEBUG_HANDLE_EXTENDED_ADMIN = 7,// status from handling extended admin
        ONE_NET_DEBUG_ONS_BAD_PARAM = 8,        // bad parameters passed toa function.
        ONE_NET_DEBUG_ONS_INTERNAL_ERR = 9,     // an internal error was detected
        ONE_NET_DEBUG_ONS_UNHANDLED_PKT = 10,   // unhandled packet
        ONE_NET_DEBUG_ONS_BAD_PKT = 11,         // bad packet
        ONE_NET_DEBUG_BUILD_RESP_PKT = 12       // status from on_build_response_pkt
    };

    void one_net_debug(UInt8 debug_type, UInt8 * data, UInt16 length);
#endif


//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET

#endif // _ONE_NET_H //

