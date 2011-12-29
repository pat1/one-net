#ifndef _ONE_NET_APPLICATION_H
#define _ONE_NET_APPLICATION_H

#include "config_options.h"


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
#include "one_net_message.h"
#include "one_net.h"
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


ONE_NET_INLINE UInt16 get_last_two_msg_bytes(const UInt8 *payload)
{
    return (((UInt16)payload[ONA_MSG_SECOND_IDX]<< 8) |
             (UInt16)payload[ONA_MSG_THIRD_IDX]);
}

ONE_NET_INLINE void put_last_two_msg_bytes(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_SECOND_IDX]  = data >> 8;
    payload[ONA_MSG_THIRD_IDX] = data;
}

/* get the 16-bit message header (message class, message type) */
ONE_NET_INLINE UInt16 get_msg_hdr(const UInt8 *payload)
{
    return ((UInt16)payload[ONA_MSG_HDR_IDX]<< 8) |
            (UInt16)payload[ONA_MSG_HDR_IDX+1];
}

/* get the 16-bit message class */
ONE_NET_INLINE UInt16 get_msg_class(const UInt8 *payload)
{
    return (get_msg_hdr(payload)) & ONA_MSG_CLASS_MASK;
}

/* get the 16-bit message type */
ONE_NET_INLINE UInt16 get_msg_type(const UInt8 *payload)
{
    return (get_msg_hdr(payload)) & ~ONA_MSG_CLASS_MASK;
}

ONE_NET_INLINE void put_msg_hdr(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_HDR_IDX]   = data >> 8;
    payload[ONA_MSG_HDR_IDX+1] = data;
}

// test these two functions more
ONE_NET_INLINE void put_msg_class(UInt16 data, UInt8 *payload)
{
    // TODO - more efficient?
    data &= ONA_MSG_CLASS_MASK;
    data |= get_msg_type(payload);
    put_msg_hdr(data, payload);
}

ONE_NET_INLINE void put_msg_type(UInt16 data, UInt8 *payload)
{
    // TODO - more efficient?
    data &= ~ONA_MSG_CLASS_MASK;
    data |= get_msg_class(payload);
    put_msg_hdr(data, payload);
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


//! Send Function
typedef one_net_status_t (*one_net_send_single_func_t)(UInt8 *data,
  UInt8 DATA_LEN, const BOOL send_to_peer_list, UInt8 PRIORITY,
  const one_net_raw_did_t *RAW_DST, UInt8 SRC_UNIT,
  tick_t* send_time_from_now, tick_t* expire_time_from_now);

#ifdef _BLOCK_MESSAGES_ENABLED
    //! Block/stream request function
    typedef one_net_status_t (*one_net_block_stream_request_func_t)(
      UInt8 TYPE, BOOL SEND, UInt16 DATA_TYPE,
      UInt16 LEN, UInt8 PRIORITY,
      const one_net_raw_did_t *DID, UInt8 SRC_UNIT);
#endif // ifdef _BLOCK_MESSAGES_ENABLED //

//! @} ONE-NET_APP_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_pub_var
//! \ingroup ONE-NET_APP
//! @{

extern one_net_send_single_func_t one_net_send_single;

#ifdef _BLOCK_MESSAGES_ENABLED
    extern one_net_block_stream_request_func_t one_net_block_stream_request;
#endif // ifdef _BLOCK_MESSAGES_ENABLED //

//! @} ONE-NET_APP_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_pub_func
//! \ingroup ONE-NET_APP
//! @{

// TODO - Do we want this function?  It doesn't appear to exist.  Should we write it?
one_net_status_t ona_parse_msg_class_and_type(const UInt8 *MSG_DATA,
  ona_msg_class_t *msg_class, ona_msg_type_t *msg_type);



on_nack_rsn_t on_parse_single_app_pld(const UInt8 * const pld, UInt8* const src_unit,
  UInt8* const dst_unit, ona_msg_class_t* const msg_class,
  ona_msg_type_t* const msg_type, UInt16* const msg_data);


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

BOOL special_payload_format(const ona_msg_type_t msg_type);

//! @} ONE-NET_APP_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP

#endif // _ONE_NET_APPLICATION_H //

