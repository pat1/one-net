#ifndef _ONE_NET_ACKNOWLEDGE_H
#define _ONE_NET_ACKNOWLEDGE_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_packet.h"
#include "one_net_features.h"
#include "one_net_xtea.h"



//! \defgroup ONE-NET_ACKNOWLEDGE ONE-NET ACK / NACK constants and functions
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
    \file one_net_acknowledge.h
    \brief Constants and typedefs dealing with acks and nacks

    Functions, constants and typedefs dealing with acks and nacks.
*/




//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_ACKNOWLEDGE_typedefs
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

/*!
    Nack Reasons
*/
typedef enum
{
    // 0 is no error
    ON_NACK_RSN_NO_ERROR,            //! Not an error.  Should not be part of a packet, but defined so that we can use it as "success" when coding

    // 00x01 through 0x17 are non-fatal ONE-NET errors
    ON_NACK_RSN_NONCE_ERR,           //! received nonce does not match expected one
	ON_NACK_RSN_RSRC_UNAVAIL_ERR,    //! resource(s) necessary to complete the transaction not currently available
	ON_NACK_RSN_INTERNAL_ERR,        //! something unanticipated occurred - Under normal circumstances, this should never be received (as it indicates an implementation fault in the sender); Devices are required to process it, however.	
	ON_NACK_RSN_BUSY_TRY_AGAIN,      //! Application level code specifying that the device cannot service the request at this time, but will likely be able to do so very soon.  Considered "non-fatal" by ONE-NET.
	ON_NACK_RSN_BUSY_TRY_AGAIN_TIME, //! Application level code specifying that the device cannot service the request at this time, but will likely be able to do so very soon.  Considered "non-fatal" by ONE-NET, but application code may override.
    ON_NACK_RSN_BAD_POSITION_ERROR,  //! Position/Offset is invalid and/or does not match what is expected by the device.
    ON_NACK_RSN_BAD_SIZE_ERROR,      //! Length/Size is invalid and/or does not mach what is expected by the device.  Different from ON_NACK_RSN_INVALID_LENGTH_ERRON_NACK_RSN_INVALID_LENGTH_ERR, which means that the device cannot HANDLE something this size.
    ON_NACK_RSN_BAD_ADDRESS_ERR,     //! DID or NID is either not decodable or is incorrect.
    ON_NACK_RSN_INVALID_MAX_HOPS,    //! Packet either has a "max hops" that is too large or there  is some other problem dealing with max hops.
    ON_NACK_RSN_INVALID_HOPS,        //! Packet has a "hops" field that is invalid for whatever reason.
    ON_NACK_RSN_INVALID_PEER,        //! Some problem dealing with an invalid peer.  Peer is not in the peer table, peer is out of range or cannot handle this message, etc.
    ON_NACK_RSN_OUT_OF_RANGE,        //! Device is out of range, is asleep, or cannot be reached for some reason.
    ON_NACK_RSN_ROUTE_ERROR,         //! message is not routed properly or no repeaters are available that can reach the device.
    ON_NACK_RSN_INVALID_DATA_RATE,   //! The transaction cannot be completed at this data rate either because device(s) cannot handle the rate it or currently unabled / not allowed to to swith to it. 
    ON_NACK_RSN_NO_RESPONSE,         //! A single message within the transaction has timed out with no response.
    ON_NACK_RSN_INVALID_MSG_ID,      //! Invalid message id
    ON_NACK_RSN_NEED_FEATURES,       //! Do not have the device's features
    ON_NACK_RSN_FEATURES,            //! A general error involving features / capabilities other than not having them.
    ON_NACK_RSN_BAD_CRC,             //! A CRC calculation failed
    ON_NACK_RSN_OLD_KEY,             //! Sent when the other device is using the old key rather than the new one.

    ON_NACK_RSN_UNSET = 0x16,        //! NACK Reason is not set yet.
    ON_NACK_RSN_GENERAL_ERR = 0x17,  //! If no specific reason is known


    // fatal ONE-NET Errors
    ON_NACK_RSN_MIN_FATAL = 0x18,
    ON_NACK_RSN_MIN_ONE_NET_FATAL = ON_NACK_RSN_MIN_FATAL,
	ON_NACK_RSN_INVALID_LENGTH_ERR = 0x18,  //! specified Device/Unit cannot handle a transaction with the specified length
	ON_NACK_RSN_DEVICE_FUNCTION_ERR, //! specified Device lacks the functionality to properly process the received packet
	ON_NACK_RSN_UNIT_FUNCTION_ERR,   //! specified Unit lacks the functionality to properly process the received packet (although the Device itself does)
	ON_NACK_RSN_INVALID_UNIT_ERR,    //! nonexistent Unit specified
	ON_NACK_RSN_MISMATCH_UNIT_ERR,   //! Unit pair specified in Block Data Segment does not match that in Request to Receive Block
	ON_NACK_RSN_BAD_DATA_ERR,        //! improperly formatted data
	ON_NACK_RSN_TRANSACTION_ERR,     //! invalid transaction specified (such as a Block Data packet in the absence of a previous Block Request)
    ON_NACK_RSN_MAX_FAILED_ATTEMPTS_REACHED, //! Attempted and failed too many times.
	ON_NACK_RSN_BUSY,                //! Application level code specifying that the device cannot service the request at this time.  No specification of when to try again.  Considered "fatal" by ONE-NET.
    ON_NACK_RSN_NO_RESPONSE_TXN,     //! The transaction has timed out with no response.
    
    // 0x22 through 0x26 are currently unused and are available for assignment as fatal ONE-NET NACKs 
    
    ON_NACK_RSN_FATAL_ERR = 0x27,    //! Some unspecified fatal error occurred.  Don't try to resend.
    ON_NACK_RSN_MAX_ONE_NET_FATAL = 0x27,   
	ON_NACK_RSN_MIN_USR_FATAL, //! NACK Reasons 0x28 through 0x2B are user-defined fatal NACKs
	ON_NACK_RSN_MAX_USR_FATAL = 0x2B, //! NACK Reasons 0x28 through 0x2B are user-defined fatal NACKs
    ON_NACK_RSN_MAX_FATAL = ON_NACK_RSN_MAX_USR_FATAL,
    

    // 0x2C through 0x33 are currently unused and are available for assignment.

    
    ON_NACK_RSN_MIN_USR_GENERAL = 0x34, //! NACK Reasons 0x34 through 0x3B are user-defined NACKs.  They are not pre-defined as fatal or non-fatal.
	ON_NACK_RSN_MAX_USR_GENERAL = 0x3B, //! NACK Reasons 0x34 through 0x3B are user-defined NACKs.  They are not pre-defined as fatal or non-fatal.
	ON_NACK_RSN_MIN_USR_NON_FATAL = 0x3C, //! NACK Reasons 0x3C through 0x3F are user-defined non-fatal NACKs
	ON_NACK_RSN_MAX_USR_NON_FATAL = 0x3F, //! NACK Reasons 0x3C through 0x3F are user-defined non-fatal NACKs
	ON_NACK_RSN_MAX_NACK_RSN_VALUE = ON_NACK_RSN_MAX_USR_FATAL
} on_nack_rsn_t;



// note : below are some named constants.  Since this is C, not C++, enumerated types are simply aliases for
// integers, so we'll use on_ack_handle_t, on_nack_handle_t, and on_ack_nack_handle_t interchangably.
// ON_ACK_STATUS only makes sense for an ACK, so don't use it for an on_nack_handle_t.  The
// compiler could not care less, but consistency makes it easier for us humans to read.

// Note that "handle" does not refer to the computer science term "handle".  Rather, it refers to
// how ONE-NET should interpret and "handle" packet information (either as random padding (ON_ACK/ON_NACK),
// a generic data array (ON_ACK_DATA, ON_NACK_DATA), a 32-but integer value (ON_ACK_32_BIT_INT/ON_NACK_32_BIT),
// or a status message(ON_ACK_STATUS), possibly in response to a query.
/*!
    Specifies what an ACK means and whether there is any data accompanying it
*/
typedef enum
{
	ON_ACK,                //! Normal ACK with no accompanying data
    ON_ACK_FEATURES,       //! Normal ACK accompanied with four bytes of features
	ON_ACK_DATA,           //! The ACK is accompanied by 5 bytes of data.
	ON_ACK_VALUE,          //! The ACK is accompanied by 8 bit and 32 bit unsigned integers.
	ON_ACK_TIME_MS,        //! The ACK is accompanied by a 32 bit unsigned integer representing generic time in milliseconds
	ON_ACK_TIMEOUT_MS,     //! Same as ON_ACK_TIME_MS, but represents the fact that something has timed out.
	ON_ACK_SLOW_DOWN_TIME_MS, //! Same as ON_ACK_TIME_MS, but represents a request to send the packets slower by
                              //! the time specified.
	ON_ACK_SPEED_UP_TIME_MS, //! Same as ON_ACK_TIME_MS, but represents a request to send the packets faster by
                              //! the time specified.
	ON_ACK_PAUSE_TIME_MS, //! Same as ON_ACK_TIME_MS, but represents a pause in milliseconds.
	ON_ACK_STATUS,        //! The ACK is accompanied by the device's current status.  This will usually be in response to a "fast query" request
    ON_NACK_OLD_KEY = ON_ACK_STATUS, //! This NACK is sent when the device is using an old key and contains the new key fragment.
    ON_ACK_MIN_APPLICATION_HANDLE, //! Application-specific handles are allowable and will be treated by ONE-NET
                                  //! as ON_ACK_DATA when building and parsing packets.  They are provided by ONE-NET
                                  //! but their meanings are to be interpreted by the application code.
                                  
    // TODO -- handle is 4 bits.  We need a constant for this, not 15.
    ON_ACK_MAX_APPLICATION_HANDLE = 15 //! Application-specific handles are allowable and will be treated by ONE-NET
                                  //! as ON_ACK_DATA when building and parsing packets.  They are provided by ONE-NET
                                  //! but their meanings are to be interpreted by the application code.
} on_ack_handle_t;


/*!
    Specifies what a NACK means and whether there is any data accompanying it.  Same as ACK values.
	Just defined here so you can use whichever you like.  These MUST correspond to the on_ack_handle_t values.
*/
#define ON_NACK ON_ACK
#define ON_NACK_FEATURES ON_ACK_FEATURES
#define ON_NACK_DATA ON_ACK_DATA
#define ON_NACK_VALUE ON_ACK_VALUE
#define ON_NACK_TIME_MS ON_ACK_TIME_MS
#define ON_NACK_TIMEOUT_MS ON_ACK_TIMEOUT_MS
#define ON_NACK_SLOW_DOWN_TIME_MS ON_ACK_SLOW_DOWN_TIME_MS
#define ON_NACK_SPEED_UP_TIME_MS ON_ACK_SPEED_UP_TIME_MS
#define ON_NACK_PAUSE_TIME_MS ON_ACK_PAUSE_TIME_MS
#define ON_NACK_MIN_APPLICATION_HANDLE ON_ACK_MIN_APPLICATION_HANDLE
#define ON_NACK_MAX_APPLICATION_HANDLE ON_ACK_MAX_APPLICATION_HANDLE

typedef on_ack_handle_t on_ack_nack_handle_t; // it's all ints anyway
typedef on_ack_handle_t on_nack_handle_t; // it's all ints anyway


typedef struct
{
	UInt8 uint8;
	UInt32 uint32;
} ack_value_t;


/*!
    The payload of an ACK or a NACK.
*/
typedef union
{
    UInt8 status_resp[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN]; //! only valid for
      //! ACKs.  Generally, but not exclusively intended for "fast query"/
      //! "poll" responses.
	UInt8 ack_payload[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN];
	ack_value_t ack_value;
    tick_t ack_time_ms;
	UInt8 nack_payload[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN - 1];
      // subtract 1 byte for the nack reason
	UInt32 nack_value;
    tick_t nack_time_ms;
    one_net_xtea_key_fragment_t key_frag;
    on_features_t features;
} ack_nack_payload_t;



// to preserve stack space for function calls, combining nack reason,
// nack handle, and nack payload into single structure.  I wish I didn't
// have to do this and maybe it shouldn't be done, but stack space is at
// a premium.  This requires being careful about which elements of the
// struct to adjust and which not to when the structure is passed.
typedef struct
{
    on_nack_rsn_t nack_reason;
    on_ack_nack_handle_t handle;
    ack_nack_payload_t* payload;
} on_ack_nack_t;



//! @} ONE-NET_ACKNOWLEDGE_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_ACKNOWLEDGE_const
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{


//! @} ONE-NET_ACKNOWLEDGE_const
//                                  CONSTANTS END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_ACKNOWLEDGE_pub_func
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

BOOL nack_reason_is_fatal(const on_nack_rsn_t nack_reason);

//! @} ONE-NET_ACKNOWLEDGE_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================



//! @} ONE-NET_ACKNOWLEDGE

#endif // _ONE_NET_PACKET_H //
