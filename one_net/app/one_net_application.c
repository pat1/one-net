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
