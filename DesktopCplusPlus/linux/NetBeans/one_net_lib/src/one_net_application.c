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




#define DESKTOP_CLI



extern BOOL device_is_master;
// just to get things to compile?
#ifdef DESKTOP_CLI
on_base_param_t make_it_compile;
on_base_param_t* const on_base_param = &make_it_compile;
#else
extern on_base_param_t* const on_base_param;
#endif
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

/*!
    \brief Parse a single packet payload to obtain message class and type.

    @depricates parse_msg_class_and_type

    \param[in] MSG_DATA, msg data
    \param[out] msg_class, the message class of the payload provided (MSG_DATA).
    \param[out] msg_type, the message type of the payload provided (MSG_DATA).

    \return the status of the parse
*/
one_net_status_t ona_parse_msg_class_and_type(const UInt8 *MSG_DATA,
        ona_msg_class_t *msg_class, ona_msg_type_t *msg_type)
{
    UInt16 class_and_type;
    
    class_and_type = get_msg_hdr(MSG_DATA);

    *msg_class = (ona_msg_class_t)class_and_type & ONA_MSG_CLASS_MASK;
    *msg_type  = (ona_msg_type_t)class_and_type  & ONA_MSG_TYPE_MASK;

    return ONS_SUCCESS;
} // parse_msg_class_and_type //



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
    on_encoded_nid_t* my_nid = (on_encoded_nid_t*) (on_base_param->sid);
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
    on_encoded_did_t* my_did = (on_encoded_did_t*)
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
  UInt8* const dst_unit, ona_msg_class_t* const msg_class, UInt16* const
  msg_type, UInt16* const msg_data)
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


void get_three_message_bytes_from_payload(UInt8 *msg, const UInt8 *payload)
{
    *(msg++) = payload[ONA_MSG_FIRST_IDX];
    *(msg++) = payload[ONA_MSG_SECOND_IDX];
    *msg     = payload[ONA_MSG_THIRD_IDX];
}

void put_three_message_bytes_to_payload(const UInt8 *msg, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX]  = *(msg++);
    payload[ONA_MSG_SECOND_IDX] = *(msg++);
    payload[ONA_MSG_THIRD_IDX]  = *msg;
}


UInt8 get_first_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_FIRST_IDX];
}

void put_first_msg_byte(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX] = data;
}

UInt8 get_second_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_SECOND_IDX];
}

void put_second_msg_byte(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_SECOND_IDX] = data;
}

UInt8 get_third_msg_byte(const UInt8 *payload)
{
    return payload[ONA_MSG_THIRD_IDX];
}

void put_third_msg_byte(const UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_THIRD_IDX] = data;
}

UInt16 get_first_two_msg_bytes(const UInt8 *payload)
{
    return (((UInt16)payload[ONA_MSG_FIRST_IDX]<< 8) |
             (UInt16)payload[ONA_MSG_SECOND_IDX]);
}

void put_first_two_msg_bytes(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_FIRST_IDX]  = data >> 8;
    payload[ONA_MSG_SECOND_IDX] = data;
}

/* get the 16-bit message header (message class, message type) */
UInt16 get_msg_hdr(const UInt8 *payload)
{
    return ((UInt16)payload[ONA_MSG_HDR_IDX]<< 8) |
            (UInt16)payload[ONA_MSG_HDR_IDX+1];
}

void put_msg_hdr(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_HDR_IDX]   = data >> 8;
    payload[ONA_MSG_HDR_IDX+1] = data;
}

ona_msg_class_t get_msg_class(const UInt8 *payload)
{
    UInt16 class_and_type = get_msg_hdr(payload);
    return class_and_type & ONA_MSG_CLASS_MASK;
}

void put_msg_class(ona_msg_class_t msg_class,
  UInt8 *payload)
{
    UInt16 class_and_type = get_msg_hdr(payload);
    class_and_type &= ~ONA_MSG_CLASS_MASK;
    class_and_type |= msg_class;
    put_msg_hdr(class_and_type, payload);
}

UInt16 get_msg_type(const UInt8 *payload)
{
    UInt16 class_and_type = get_msg_hdr(payload);
    return class_and_type & ~ONA_MSG_CLASS_MASK;
}

void put_msg_type(ona_msg_class_t msg_type,
  UInt8 *payload)
{
    UInt16 class_and_type = get_msg_hdr(payload);
    class_and_type &= ONA_MSG_CLASS_MASK;
    class_and_type |= msg_type;
    put_msg_hdr(class_and_type, payload);
}

/* get the 16-bit message data from the payload buffer */
UInt16 get_msg_data(const UInt8 *payload)
{
    return ((UInt16)payload[ONA_MSG_SECOND_IDX] << 8) |
            (UInt16)payload[ONA_MSG_THIRD_IDX];
}

/* store the 16-bit message data in the payload buffer
 * Use platform-dependent function in one_net_port_specific.c
 */
void put_msg_data(UInt16 data, UInt8 *payload)
{
    payload[ONA_MSG_SECOND_IDX] = data >> 8;
    payload[ONA_MSG_THIRD_IDX]  = data;
}

/* get the 8-bit source unit data value from the payload buffer */
UInt8 get_src_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_SRC_UNIT_IDX] & ONA_MSG_SRC_UNIT_MASK) >>
            ONA_MSG_SRC_UNIT_SHIFT;
}

/* store the 8-bit source unit data value in the payload buffer */
void put_src_unit(UInt8 data , UInt8 *payload)
{
    payload[ONA_MSG_SRC_UNIT_IDX] =
        (payload[ONA_MSG_SRC_UNIT_IDX]    & ~ONA_MSG_SRC_UNIT_MASK) |
        ((data << ONA_MSG_SRC_UNIT_SHIFT) &  ONA_MSG_SRC_UNIT_MASK);
}

/* get the 8-bit destination unit data value from the payload buffer */
UInt8 get_dst_unit(const UInt8 *payload)
{
    return (payload[ONA_MSG_DST_UNIT_IDX] & ONA_MSG_DST_UNIT_MASK) >>
            ONA_MSG_DST_UNIT_SHIFT;
}

/* store the 8-bit destination unit data value in the payload buffer */
void put_dst_unit(UInt8 data, UInt8 *payload)
{
    payload[ONA_MSG_DST_UNIT_IDX] =
        (payload[ONA_MSG_DST_UNIT_IDX]    & ~ONA_MSG_DST_UNIT_MASK) |
        ((data << ONA_MSG_DST_UNIT_SHIFT) &  ONA_MSG_DST_UNIT_MASK);
}

/* get the 4-bit message type value from the payload buffer */
UInt8 get_payload_msg_type(const UInt8 *payload)
{
    return (payload[ON_PLD_MSG_TYPE_IDX] & ON_PLD_MSG_TYPE_MASK);
}

/* store the 4-bit message type value in the raw payload buffer */
void put_payload_msg_type(UInt8 msg_type, UInt8 *payload)
{
    payload[ON_PLD_MSG_TYPE_IDX] =
        (payload[ON_PLD_MSG_TYPE_IDX]    & ~ON_PLD_MSG_TYPE_MASK) |
        (msg_type & ON_PLD_MSG_TYPE_MASK);
}

/* get the 6-bit transaction nonce from the payload buffer */
UInt8 get_payload_txn_nonce(const UInt8 *payload)
{
    return (payload[ON_PLD_TXN_NONCE_IDX] >> ON_TXN_NONCE_SHIFT);
}

/* store the 6-bit transaction nonce in the raw payload buffer */
void put_payload_txn_nonce(UInt8 txn_nonce, UInt8 *payload)
{
    payload[ON_PLD_TXN_NONCE_IDX] = (payload[ON_PLD_TXN_NONCE_IDX] &
      ~ON_TXN_NONCE_BUILD_MASK) |
      ((txn_nonce << ON_TXN_NONCE_SHIFT) & ON_TXN_NONCE_BUILD_MASK);
}

/* get the 6-bit response nonce from the payload buffer */
UInt8 get_payload_resp_nonce(const UInt8 *payload)
{
    return ((payload[ON_PLD_RESP_NONCE_HIGH_IDX] &
      ON_RESP_NONCE_BUILD_HIGH_MASK) << ON_RESP_NONCE_HIGH_SHIFT) +
      (payload[ON_PLD_RESP_NONCE_LOW_IDX] >> ON_RESP_NONCE_LOW_SHIFT);
}

/* store the 6-bit response nonce in the raw payload buffer */
void put_payload_resp_nonce(UInt8 resp_nonce, UInt8 *payload)
{
    payload[ON_PLD_RESP_NONCE_HIGH_IDX] = (payload[ON_PLD_RESP_NONCE_HIGH_IDX]
      & ~ON_RESP_NONCE_BUILD_HIGH_MASK) | ((resp_nonce >>
      ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK);
    payload[ON_PLD_RESP_NONCE_LOW_IDX] = (payload[ON_PLD_RESP_NONCE_LOW_IDX]
      & ~ON_RESP_NONCE_BUILD_LOW_MASK) | ((resp_nonce <<
      ON_RESP_NONCE_LOW_SHIFT)& ON_RESP_NONCE_BUILD_LOW_MASK);
}

    



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
