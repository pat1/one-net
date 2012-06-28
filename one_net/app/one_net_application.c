//! \addtogroup ONE-NET_APP
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
    \file one_net_application.c
    \brief Global ONE-NET application layer implementation.

    This is the global implementation of the application layer
    of ONE-NET.  Any ONE-NET device will want to include and use
    this code for their application.
*/

#include "one_net_application.h"
#include "one_net_packet.h"
#include "one_net_port_specific.h"
#include "one_net_constants.h"


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

//! @} ONE-NET_APP_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_pub_var
//! \ingroup ONE-NET_APP
//! @{



extern BOOL device_is_master;
extern on_base_param_t* const on_base_param;
extern const on_encoded_did_t MASTER_ENCODED_DID;
extern const on_encoded_did_t ON_ENCODED_BROADCAST_DID;



//! @} ONE-NET_APP_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_pri_func
//! \ingroup ONE-NET_APP
//! @{

//! @} ONE-NET_APP_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_pub_func
//! \ingroup ONE-NET_APP
//! @{



// address functions
/*!
    \brief Compares two encoded Device IDs.

    \param[in] LHS The left hand side of the compare equation.
    \param[in] RHS The right hand side of the compare equation.

    \return TRUE if the DIDs match.
            FALSE if the DIDs do not match.
*/
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS)
{
    if(!LHS || !RHS)
    {
        return FALSE;
    } // if parameters are invalid //

    return (one_net_memcmp(*LHS, *RHS, ON_ENCODED_DID_LEN) == 0);
} // on_encoded_did_equal //

/*!
    \brief Compares two encoded Network IDs.

    \param[in] LHS The left hand side of the compare equation.
    \param[in] RHS The right hand side of the compare equation.

    \return TRUE if the NIDs match.
            FALSE if the NIDs do not match.
*/
BOOL on_encoded_nid_equal(const on_encoded_nid_t * const LHS,
  const on_encoded_nid_t * const RHS)
{
    if(!LHS || !RHS)
    {
        return FALSE;
    } // if parameters are invalid //

    return (one_net_memcmp(*LHS, *RHS, ON_ENCODED_NID_LEN) == 0);
} // on_encoded_nid_equal //  
  

/*!
    \brief Compares the NID passed to the function to this device's NID
    
    \param[in] nid The network ID to compare.
    
    \return TRUE if the NID is this device's NID.
    \return FALSE if NID is NULL or the NIDs do not match
*/
BOOL is_my_nid(const on_encoded_nid_t* nid)
{
    const on_encoded_nid_t* my_nid = (const on_encoded_nid_t*)
      (on_base_param->sid);
    return on_encoded_nid_equal(nid, my_nid);
}


/*!
    \brief Compares the DID passed to the function to the master's DID
    
    \param[in] did The Device ID to compare.
    
    \return TRUE if the DID is not the master device's DID.
    \return FALSE if DID is NULL or the DID is not the master device's DID.
*/
BOOL is_master_did(const on_encoded_did_t* did)
{
    return on_encoded_did_equal(did, &MASTER_ENCODED_DID);
}


/*!
    \brief Compares the DID passed to the function to this device's DID
    
    \param[in] did The Device ID to compare.
    
    \return TRUE if the DID is this device's DID.
    \return FALSE if DID is NULL or the DID is not this device's DID.
*/
BOOL is_my_did(const on_encoded_did_t* did)
{
    const on_encoded_did_t* my_did = (const on_encoded_did_t*)
      &(on_base_param->sid[ON_ENCODED_NID_LEN]);
    return on_encoded_did_equal(did, my_did);
}


/*!
    \brief Compares the DID passed to the function to the broadcast DID
    
    \param[in] did The Device ID to compare.
    
    \return TRUE if the DID is the broadcast DID
    \return FALSE if DID is NULL or the DID is not the broadcast DID
*/
BOOL is_broadcast_did(const on_encoded_did_t* did)
{
    return on_encoded_did_equal(did, &ON_ENCODED_BROADCAST_DID);
}


/*!
    \brief Parses a single application message
    
    \param[in] payload the payload of the message
    \param[out] src_unit the source unit of the message
    \param[out] dst_unit the destination unit of the message
    \param[out] msg_class the message class of the message
    \param[out] msg_type the message type of the message
    \param[out] msg_data the message data of the message
    
    \return TRUE if parsed successfully
    \return FALSE if not parsed successfully
*/
BOOL on_parse_app_pld(const UInt8* const payload, UInt8* const src_unit,
  UInt8* const dst_unit, ona_msg_class_t* const msg_class, UInt8* const
  msg_type, SInt32* const msg_data)
{
    if(!payload || !src_unit || !dst_unit || !msg_class || !msg_type ||
      !msg_data)
    {
        return FALSE;
    }
    
    *src_unit = get_src_unit(payload);
    *dst_unit = get_dst_unit(payload);
    *msg_class = get_msg_class(payload);
    *msg_type = get_msg_type(payload);
    *msg_data = get_msg_data(payload);
    return TRUE;
}


#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Parses a single block packet
    
    \param[in] buffer the packet bytes
    \param[out] bs_pkt the parsed packet
    
    \return TRUE if parsed successfully
    \return FALSE if not parsed successfully
*/
BOOL on_parse_block_pld(UInt8* buffer, block_pkt_t* block_pkt)
{
    if(!buffer || !block_pkt)
    {
        return FALSE;
    }
    
    block_pkt->msg_id = get_payload_msg_id(buffer);
    block_pkt->chunk_idx = get_bs_chunk_idx(buffer);
    block_pkt->chunk_size = get_bs_chunk_size(buffer);
    block_pkt->byte_idx = get_block_pkt_idx(buffer);
    block_pkt->data = &buffer[ON_BS_DATA_PLD_IDX];
    return TRUE;
}
#endif


#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Parses a single stream packet
    
    \param[in] buffer the packet bytes
    \param[out] bs_pkt the parsed packet
    
    \return TRUE if parsed successfully
    \return FALSE if not parsed successfully
*/
BOOL on_parse_stream_pld(UInt8* buffer, stream_pkt_t* stream_pkt)
{
    if(!buffer || !stream_pkt)
    {
        return FALSE;
    }
    
    stream_pkt->msg_id = get_payload_msg_id(buffer);
    stream_pkt->response_needed = get_stream_response_needed(buffer);
    stream_pkt->elapsed_time = get_stream_elapsed_time(buffer);
    stream_pkt->data = &buffer[ON_BS_DATA_PLD_IDX];
    return TRUE;
}
#endif



/* store the 4-bit message type value in the raw payload buffer */
void put_payload_msg_type(UInt8 msg_type, UInt8 *payload)
{
    payload[ON_PLD_MSG_TYPE_IDX] = 
        (payload[ON_PLD_MSG_TYPE_IDX]    & ~ON_PLD_MSG_TYPE_MASK) |
        (msg_type & ON_PLD_MSG_TYPE_MASK);
}

/* store the 8-bit destination unit data value in the payload buffer */
void put_dst_unit(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_DST_UNIT_IDX] = 
        (payload[ONA_MSG_DST_UNIT_IDX]    & ~ONA_MSG_DST_UNIT_MASK) |
        ((data << ONA_MSG_DST_UNIT_SHIFT) &  ONA_MSG_DST_UNIT_MASK);
}

/* put the 12-byte message header into the payload */
void put_msg_hdr(UInt16 hdr, UInt8* payload)
{
    payload[ONA_MSG_HDR_IDX]   = hdr >> ONA_MSG_CLASS_TYPE_SHIFT;
    payload[ONA_MSG_HDR_IDX+1] &= 0x0F;
    payload[ONA_MSG_HDR_IDX+1] |= (hdr << ONA_MSG_CLASS_TYPE_SHIFT);
}

/* store the 32-bit message data in the payload buffer */
void put_msg_data(SInt32 data, UInt8 *payload)
{
    // TODO -- can we make this function more efficient?
    UInt8 sign = 0;
    if(data < 0)
    {
        sign = 0x08;
        data = -data;
    }
    
    // data is now non-negative
    data &= 0x0007FFFF;
    payload[2] &= 0xF0;
    payload[2] |= (data >> 16); 
    payload[2] |= sign;
    payload[3] = data >> 8;
    payload[4] = data;
}


/* store the 12-bit message id in the raw payload buffer */
void put_payload_msg_id(UInt16 msg_id, UInt8 *payload)
{
    payload[ON_PLD_MSG_ID_IDX] = (UInt8) ((msg_id & 0x0FF0) >> 4);
    payload[ON_PLD_MSG_ID_IDX + 1] &= ON_RESP_HANDLE_BUILD_MASK;
    payload[ON_PLD_MSG_ID_IDX + 1] |= ((UInt8) ((msg_id &
      ON_RESP_HANDLE_BUILD_MASK) << 4));
}


#ifdef BLOCK_MESSAGES_ENABLED
/* stores the packet index in the raw payload buffer */
void put_block_pkt_idx(UInt32 pkt_idx, UInt8* payload)
{
    UInt8 temp = payload[ON_BS_PLD_PKT_IDX-1];
    pkt_idx &= 0x00FFFFFF;
    one_net_int32_to_byte_stream(pkt_idx, &payload[ON_BS_PLD_PKT_IDX-1]);
    payload[ON_BS_PLD_PKT_IDX-1] = temp;
}
#endif




//! @} ONE-NET_APP_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_pri_func
//! \ingroup ONE-NET_APP
//! @{

//! @} ONE-NET_APP_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET_APP
