#ifndef _ONE_NET_APPLICATION_H
#define _ONE_NET_APPLICATION_H

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


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_const
//! \ingroup ONE-NET_APP
//! @{



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


//! @} ONE-NET_APP_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_typedefs
//! \ingroup ONE-NET_APP
//! @{


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


//! flags for settings admin packet 
enum
{
    //! Flag set when the device is part of the network
    ON_JOINED = 0x80,

    //! Flag to indicate a CLIENT should send a message that it sent to its peer
    //! to the MASTER too.
    ON_SEND_TO_MASTER = 0x40
};


enum
{
    //! Unit number that refers to device as a whole
    ONE_NET_DEV_UNIT = 0x0F
};


// pin constants
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


//! Send Function
typedef one_net_status_t (*one_net_send_single_func_t)(UInt8 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t* send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t* expire_time_from_now
  #endif
  );



//! @} ONE-NET_APP_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_pub_var
//! \ingroup ONE-NET_APP
//! @{


extern one_net_send_single_func_t one_net_send_single;


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
  

// address functions
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS);
BOOL on_encoded_nid_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS);


//! @} ONE-NET_APP_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP

#endif // _ONE_NET_APPLICATION_H //

