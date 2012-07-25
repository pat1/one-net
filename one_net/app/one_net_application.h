#ifndef ONE_NET_APPLICATION_H
#define ONE_NET_APPLICATION_H

#include "config_options.h"
#include "one_net_message.h"


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

#include "one_net_types.h"
#include "one_net_packet.h"
#include "one_net_status_codes.h"
#include "one_net_features.h"
#include "one_net_xtea.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_const
//! \ingroup ONE-NET_APP
//! @{




//! @} ONE-NET_APP_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_typedefs
//! \ingroup ONE-NET_APP
//! @{
    
   
    
/*!
    \brief Contains the base set of parameters needed to run.

    These are the parameters needed by devices if they are already part of the
    network.
*/
typedef struct
{
    //! crc over the parameters
    UInt8 crc;

    //! Version of the non--volatile memory structures
    UInt16 version;

    //! The encoded address it had been assigned
    on_encoded_sid_t sid;

    //! The channel the network is on
    UInt8 channel;
    
    //! The features that this device supports
    on_features_t features;

    //! Data rate the device is currently using
    UInt8 data_rate;

    //! The last key used
    one_net_xtea_key_fragment_t old_key;
    
    //! The current xtea key being used.
    one_net_xtea_key_t current_key;
    
    #ifdef ONE_NET_MULTI_HOP
    //! The number of multi-hop capable devices in the network
    UInt8 num_mh_devices;

    //! The number of multi-hop repeater clients in the network
    UInt8 num_mh_repeaters;
    #endif
    
    #ifdef BLOCK_MESSAGES_ENABLED
    //! Low priority fragment delay
    UInt16 fragment_delay_low;

    //! High priority fragment delay
    UInt16 fragment_delay_high;
    #endif
} on_base_param_t;


typedef enum _ona_unit_type
{
    //! Simple ON/OFF switch
    ONA_SIMPLE_SWITCH = 0x00,

    //! Switch w/ ON/OFF & dimmer
    ONA_DIMMER_SWITCH = 0x01,

    //! Simple ON/OFF switch w/ display
    ONA_DISPLAY_SWITCH = 0x02,

    //! Switch w/ ON/OFF, dimmer & display
    ONA_DISPLAY_DIMMER_SWITCH = 0x03,

    //! Simple ON/OFF light
    ONA_SIMPLE_LIGHT = 0x04,

    //! Light w/ ON/OFF & dimmer
    ONA_DIMMING_LIGHT = 0x05,

    //! ON/OFF outlet
    ONA_OUTLET = 0x06,

    //! Speaker module
    ONA_SPEAKER = 0x07,

    //! Temperature sensor
    ONA_TEMPERATURE_SENSOR = 0x08,

    //! Humidity sensor
    ONA_HUMIDITY_SENSOR = 0x09,

    //! Door/Window sensor
    ONA_DOOR_WINDOW_SENSOR = 0x0A,

    //! Motion sensor
    ONA_MOTION_SENSOR = 0x0B,

    //! ONE-NET/X10 bridge
    ONA_ONE_NET_X10_BRIDGE = 0x0C,

    //! ONE-NET/INSTEON bridge
    ONA_ONE_NET_INSTEON_BRIDGE = 0x0D
} ona_unit_type_t;


//! Structure that holds information on the unit type count
typedef struct
{
    //! The type of unit
    ona_unit_type_t type;

    //! The number of this type of unit this device supports.
    UInt8 count;
} ona_unit_type_count_t;


//! settings flags
typedef enum
{
    //! Flag set when the device is part of the network
    ON_JOINED = 0x80,

    //! Flag to indicate a CLIENT should inform the master of all state changes
    //! (i.e. pin changes)
    ON_SEND_TO_MASTER = 0x40,
    
    //! Flag to indicate whether a client should reject invalid message ids
    //! Mainly used to prevent replay attacks.
    ON_REJECT_INVALID_MSG_ID = 0x20,
    
    //! Flag to indicate whether the data rate should elevate during long block
    //! / stream msgs.  Relevant only when block / stream is enabled.
    ON_BS_ELEVATE_DATA_RATE = 0x10,
    
    //! Flag to indicate whether the channel should change during long block
    //! / stream msgs.  Relevant only when block / stream is enabled.
    ON_BS_CHANGE_CHANNEL = 0x08,
    
    //! Flag to indicate whether long block / stream transfers should proceed
    //! at low or high priority.  When flag is set, high prioriity is the
    //! default.  Relevant only when block / stream is enabled.
    ON_BS_HIGH_PRIORITY = 0x04,
    
    //! Flag to indicate whether long block / stream transfers should proceed
    //! at all for this device
    ON_BS_ALLOWED = 0x02
} on_master_flag_t;


enum
{
    //! Unit number that refers to device as a whole
    ONE_NET_DEV_UNIT = 0x0F
};


//! pin constants
enum
{
    INPUT = 0,                      //!< Value when setting a pin as an input
    OUTPUT = 1                      //!< Value when setting a pin as an output
};


typedef enum
{
    ON_INPUT_PIN = INPUT,        //!< Value when setting a pin as an input
    ON_OUTPUT_PIN = OUTPUT,      //!< Value when setting a pin as an output
    ON_DISABLE_PIN = 2           //!< Indicates the pin is not being used
} on_pin_state_t;


//! On / Off / Toggle Constants
#define ONA_OFF 0
#define ONA_ON 1
#define ONA_TOGGLE 2



//! @} ONE-NET_APP_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_pub_var
//! \ingroup ONE-NET_APP
//! @{


//! @} ONE-NET_APP_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_pub_func
//! \ingroup ONE-NET_APP
//! @{


#ifdef __cplusplus
extern "C"
{
#endif


// address functions
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS);
BOOL on_encoded_nid_equal(const on_encoded_nid_t * const LHS,
  const on_encoded_nid_t * const RHS);
BOOL is_my_nid(const on_encoded_nid_t* nid);
BOOL is_master_did(const on_encoded_did_t* did);
BOOL is_my_did(const on_encoded_did_t* did);
BOOL is_broadcast_did(const on_encoded_did_t* did);


// parsing functions
#ifndef ONE_NET_SIMPLE_CLIENT
BOOL on_parse_app_pld(const UInt8* const payload, UInt8 app_msg_type, UInt8* const src_unit,
  UInt8* const dst_unit, ona_msg_class_t* const msg_class, UInt8* const
  msg_type, SInt32* const msg_data);
#else
BOOL on_parse_app_pld(const UInt8* const payload, UInt8* const src_unit,
  UInt8* const dst_unit, ona_msg_class_t* const msg_class, UInt8* const
  msg_type, SInt32* const msg_data);
#endif
#ifdef BLOCK_MESSAGES_ENABLED
BOOL on_parse_block_pld(UInt8* buffer, block_pkt_t* block_pkt);
#endif
#ifdef STREAM_MESSAGES_ENABLED
BOOL on_parse_stream_pld(UInt8* buffer, stream_pkt_t* stream_pkt);
#endif


extern on_base_param_t* const on_base_param;
ONE_NET_INLINE on_encoded_did_t* get_encoded_did_from_sending_device(
  const on_sending_device_t* device)
{
    if(!device)
    {
        return (on_encoded_did_t*) (&on_base_param->sid[ON_ENCODED_NID_LEN]);
    }
    return (on_encoded_did_t*) &(device->did);
}


/* get the 12-bit message header (message class, message type) */
ONE_NET_INLINE UInt16 get_msg_hdr(const UInt8* payload)
{
    UInt16 msg_hdr = payload[ONA_MSG_HDR_IDX];
    msg_hdr <<= ONA_MSG_CLASS_TYPE_SHIFT;
    return msg_hdr + (payload[ONA_MSG_HDR_IDX + 1] >> ONA_MSG_CLASS_TYPE_SHIFT);
}

/* put the 12-byte message header into the payload */
void put_msg_hdr(UInt16 hdr, UInt8* payload);

ONE_NET_INLINE ona_msg_class_t get_msg_class(const UInt8* payload)
{
    UInt16 class_no_shift = (payload[ONA_MSG_HDR_IDX] & 0xF0);
    return (class_no_shift << ONA_MSG_CLASS_TYPE_SHIFT);
}

ONE_NET_INLINE void put_msg_class(ona_msg_class_t msg_class,
  UInt8 *payload)
{
    msg_class &= ONA_MSG_CLASS_MASK;
    payload[ONA_MSG_HDR_IDX] &= 0x0F;
    payload[ONA_MSG_HDR_IDX] |= (msg_class >> ONA_MSG_CLASS_TYPE_SHIFT);
}

ONE_NET_INLINE UInt8 get_msg_type(const UInt8 *payload)
{
    return ((payload[ONA_MSG_HDR_IDX] << ONA_MSG_CLASS_TYPE_SHIFT) +
      (payload[ONA_MSG_HDR_IDX+1] >> ONA_MSG_CLASS_TYPE_SHIFT));
}

ONE_NET_INLINE void put_msg_type(UInt8 msg_type, UInt8* payload)
{
    payload[ONA_MSG_HDR_IDX] &= 0xF0;
    payload[ONA_MSG_HDR_IDX] |= (msg_type >> 4);
    payload[ONA_MSG_HDR_IDX+1] &= 0x0F;
    payload[ONA_MSG_HDR_IDX+1] |= (msg_type << 4);
}


/* get the 20- or 28-bit message data from the payload buffer */
#ifdef ONE_NET_SIMPLE_CLIENT
SInt32 get_msg_data(const UInt8* payload);
#else
SInt32 get_msg_data(const UInt8* payload, UInt8 app_msg_type);
#endif

/* store the 20- or 28-bit message data in the payload buffer */
#ifndef ONE_NET_SIMPLE_CLIENT
void put_msg_data(SInt32 data, UInt8 *payload, UInt8 app_msg_type);
#else
void put_msg_data(SInt32 data, UInt8 *payload);
#endif

/* get the 4-bit source unit data value from the payload buffer */
ONE_NET_INLINE UInt8 get_src_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_SRC_UNIT_IDX] & ONA_MSG_SRC_UNIT_MASK) >> 
            ONA_MSG_SRC_UNIT_SHIFT;
}

/* store the 4-bit source unit data value in the payload buffer */
void put_src_unit(UInt8 data , UInt8 *payload);

/* get the 4-bit destination unit data value from the payload buffer */
ONE_NET_INLINE UInt8 get_dst_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_DST_UNIT_IDX] & ONA_MSG_DST_UNIT_MASK) >> 
            ONA_MSG_DST_UNIT_SHIFT;
}

/* store the 4-bit destination unit data value in the payload buffer */
void put_dst_unit(UInt8 data, UInt8 *payload);

/* get the 4-bit message type value from the payload buffer */
ONE_NET_INLINE UInt8 get_payload_msg_type(const UInt8 *payload)
{
    return (payload[ON_PLD_MSG_TYPE_IDX] & ON_PLD_MSG_TYPE_MASK);
}

/* store the 4-bit message type value in the raw payload buffer */
void put_payload_msg_type(UInt8 msg_type, UInt8 *payload);

// for responses, the ack / nack handle is precisely where the messge
// type is for data packets, so we'll define some macros.
#define get_ack_nack_handle(X) get_payload_msg_type(X)
#define put_ack_nack_handle(X, Y) put_payload_msg_type(X, Y)


/* get the 12-bit message id from the payload buffer */
ONE_NET_INLINE UInt16 get_payload_msg_id(const UInt8 *payload)
{
    UInt16 msg_id = (((UInt16)payload[ON_PLD_MSG_ID_IDX]) << 4)
      + (payload[ON_PLD_MSG_ID_IDX + 1] >> 4);
    return msg_id;
}

/* store the 12-bit message id in the raw payload buffer */
void put_payload_msg_id(UInt16 msg_id, UInt8 *payload);

/* stores the chunk index in the raw payload buffer */
ONE_NET_INLINE void put_bs_chunk_idx(UInt8 chunk_idx, UInt8* payload)
{
    payload[ON_BS_PLD_CHUNK_IDX] = 
        ((payload[ON_BS_PLD_CHUNK_IDX] & 0xF0) | (chunk_idx >> 2));
    payload[ON_BS_PLD_CHUNK_IDX+1] = 
        ((payload[ON_BS_PLD_CHUNK_IDX+1] & 0x3F) | (chunk_idx << 6));
}


/* gets the chunk index from the raw payload buffer */
ONE_NET_INLINE UInt8 get_bs_chunk_idx(UInt8* payload)
{
    UInt8 chunk_idx = ((payload[ON_BS_PLD_CHUNK_IDX] & 0x0F) << 2);
    chunk_idx += (payload[ON_BS_PLD_CHUNK_IDX+1] >> 6);
    return chunk_idx;
}


/* stores the chunk index in the raw payload buffer */
ONE_NET_INLINE void put_bs_chunk_size(UInt8 chunk_size, UInt8* payload)
{
    payload[ON_BS_PLD_CHUNK_IDX+1] = 
        ((payload[ON_BS_PLD_CHUNK_IDX+1] & 0xC0) | (chunk_size & 0x3F));
}


/* gets the chunk index from the raw payload buffer */
ONE_NET_INLINE UInt8 get_bs_chunk_size(UInt8* payload)
{
    return (payload[ON_BS_PLD_CHUNK_IDX+1] & 0x3F);
}


/* stores whether a response is needed in the raw payload buffer */
ONE_NET_INLINE void put_stream_response_needed(BOOL response_needed,
  UInt8* payload)
{
    payload[ON_BS_PLD_CHUNK_IDX+1] = 
        ((payload[ON_BS_PLD_CHUNK_IDX+1] & 0xC0) | response_needed);
}


/* extracts whether a response is needed from the raw payload buffer */
ONE_NET_INLINE BOOL get_stream_response_needed(UInt8* payload)
{
    return (payload[ON_BS_PLD_CHUNK_IDX+1] & 0x3F);
}



#define put_stream_elapsed_time put_block_pkt_idx
#define get_stream_elapsed_time get_block_pkt_idx


/* stores the packet index in the raw payload buffer */
void put_block_pkt_idx(UInt32 pkt_idx, UInt8* payload);


/* gets the chunk index from the raw payload buffer */
ONE_NET_INLINE UInt32 get_block_pkt_idx(UInt8* payload)
{
    UInt32 pkt_idx = one_net_byte_stream_to_int32(
      &payload[ON_BS_PLD_PKT_IDX-1]);
    return (pkt_idx & 0x00FFFFFF);
}


#ifdef __cplusplus
}
#endif




//! @} ONE-NET_APP_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP

#endif // ONE_NET_APPLICATION_H //

