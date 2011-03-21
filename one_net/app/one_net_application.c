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

#include "one_net.h"
#include "one_net_application.h"
#include "one_net_port_specific.h"

// include proper heading, depending on Master/Client
#ifdef _ONE_NET_MASTER
    #include "one_net_master.h"
#else
    #include "one_net_client.h"
#endif

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
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_pri_var
//! \ingroup ONE-NET_APP
//! @{

#ifdef _ONE_NET_MASTER
    one_net_send_single_func_t one_net_send_single
      = &one_net_master_send_single;
#else // ifdef MASTER //
    one_net_send_single_func_t one_net_send_single
      = &one_net_client_send_single;
#endif // else it's not the MASTER //

#ifndef _ONE_NET_SIMPLE_CLIENT
    #ifdef _ONE_NET_MASTER
        one_net_block_stream_request_func_t one_net_block_stream_request
          = &one_net_master_block_stream_request;
    #else // ifdef MASTER //
        one_net_block_stream_request_func_t one_net_block_stream_request
          = &one_net_client_block_stream_request;
    #endif // else MASTER is not defined //
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! @} ONE-NET_APP_pri_var
//                              PRIVATE VARIABLES END
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


// TODO any reason why this needs to only be in 2.0?
#ifdef _ONE_NET_VERSION_2_X
/*!
    \brief Parses the 5 byte payload of a single application packet

    \param[in] pld The 5 bytes containg unit numbers, message class, type, and data
    \param[out] src_unit The source unit in this payload
    \param[out] dst_unit The source unit in this payload
    \param[out] msg_class The message class in this payload
    \param[out] msg_type The message type in this payload
    \param[out] msg_data The data in this payload

    \return ON_NACK_RSN_NO_ERROR if parsing was successful			
            See on_nack_rsn_t for more possible return values.
*/
on_nack_rsn_t on_parse_single_app_pld(const UInt8 * const pld, UInt8* const src_unit,
  UInt8* const dst_unt, ona_msg_class_t* const msg_class,
  ona_msg_type_t* const msg_type, const UInt16* msg_data)
{
	UInt16 hdr;
	
	if(!pld || !src_unit || !dst_unit || !msg_class || !msg_type || !msg_data)
	{
		return ON_NACK_RSN_INTERNAL_ERR;
	}
	
    *src_unit = get_src_unit(pld);
    *dst_unit = get_dst_unit(pld);
    *msg_data = get_msg_data(pld);
	
	// TODO perhaps call the parse_msg_class_and_type function
    hdr  =  get_msg_hdr(pld);
    *msg_class =  hdr & ONA_MSG_CLASS_MASK;
    *msg_type  = hdr & (~ONA_MSG_CLASS_MASK);
	
	// TODO : Put checking here for validity
	
	return ON_NACK_RSN_NO_ERROR;
}
//#endif


#ifndef _ONE_NET_MASTER
    /*!
        \brief Sends a ONA_UNIT_TYPE_COUNT msg

        Called by application code to send a ONA_UNIT_TYPE_COUNT msg

        \param[in] RAW_DST, the one_net_raw_did_t of the destination device

        \return the status of the send aciton
    */
    one_net_status_t ona_send_unit_type_count_status(const one_net_raw_did_t *RAW_DST)
    {
        UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

        put_msg_hdr(ONA_STATUS|ONA_UNIT_TYPE_COUNT, payload);

        //put_first_msg_byte(???) // maybe put random stuff here???
        put_second_msg_byte(ONE_NET_NUM_UNITS, payload);
        put_third_msg_byte(ONE_NET_NUM_UNIT_TYPES, payload);
        // send payload
        return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
          ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
    } // ona_send_unit_type_count_status //
#endif // ifndef MASTER //


/*!
    \brief Sends a ONA_UNIT_TYPE_COUNT msg

    Called by application code to send a ONA_UNIT_TYPE_COUNT query msg

    \param[in] RAW_DST, the one_net_raw_did_t of the destination device

    \return the status of the send aciton
*/
one_net_status_t ona_send_unit_type_count_query(
  const one_net_raw_did_t *RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    put_msg_hdr(ONA_QUERY|ONA_UNIT_TYPE_COUNT, payload);

    // TBD: make this random?
    // put_first_msg_byte(???);
    // put_second_msg_byte(???);
    // put_third_msg_byte(???);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
} // ona_send_unit_type_count_query //


#ifndef _ONE_NET_MASTER
    /*!
        \brief Sends a ONA_UNIT_TYPE status msg

        Called by application code to send a ONA_UNIT_TYPE status msg,
        usually in response to a ONA_UNIT_TYPE query msg.

        \param[in] UNIT_TYPE_IDX, The index of the unit type to send the status
          for.
        \param[in] RAW_DST, the one_net_raw_did_t of the destination device

        \return INVALID_PARAM If the parameter is not valid
                the status of the send action
    */
    one_net_status_t ona_send_unit_type_status(UInt8 UNIT_TYPE_IDX,
      const one_net_raw_did_t *RAW_DST)
    {
        UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

        if(UNIT_TYPE_IDX >= ONE_NET_NUM_UNIT_TYPES)
        {
            return ONS_BAD_PARAM;
        } // if parameter is invalid //

        // format TYPE & add UNIT_COUNT
        put_msg_hdr(ONA_STATUS|ONA_UNIT_TYPE, payload);
        put_first_two_msg_bytes(ONE_NET_DEVICE_UNIT_TYPE[UNIT_TYPE_IDX].type,
                payload);
        put_third_msg_byte(ONE_NET_DEVICE_UNIT_TYPE[UNIT_TYPE_IDX].count,
                payload);

        // send payload
        return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
          ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
    } // one_net_send_unit_type_status //
#endif // ifndef MASTER //


/*!
    \brief Sends a ONA_UNIT_TYPE query msg

    Called by application code to send a ONA_UNIT_TYPE query msg requesting a
    ONA_UNIT_TYPE status msg

    \param[in] UNIT_TYPE_INDEX, index of types you want
    \param[in] RAW_DST, the one_net_raw_did_t of the destination device

    \return the status of the send action
*/
one_net_status_t ona_send_unit_type_query(UInt8 UNIT_TYPE_INDEX,
  const one_net_raw_did_t *RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    // set msg class/type
    put_msg_hdr(ONA_QUERY|ONA_UNIT_TYPE, payload);

    put_first_msg_byte(UNIT_TYPE_INDEX, payload);

    // TBD: make this random?
    //put_second_msg_byte(???);
    //put_third_msg_byte(???);

    return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
      ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
} // ona_send_unit_type_query //


/*!
    \brief Parse a Unit Type Status msg

    \param[in] MSG_DATA, msg data
    \param[out] unit_count, total number of units
    \param[out] unit_type_count, number of unit types

    \return the status of the parse
*/
one_net_status_t ona_parse_unit_type_count(const UInt8 *MSG_DATA,
  UInt8 *unit_count, UInt8 *unit_type_count)
{
    *unit_count      = get_first_msg_byte(MSG_DATA);
    *unit_type_count = get_second_msg_byte(MSG_DATA);

    return ONS_SUCCESS;
} // ona_parse_unit_type_status //


/*!
    \brief Parse a Unit Type Status msg

    \param[in] MSG_DATA, msg data
    \param[out] unit_type, unit type
    \param[out] unit_count, number of units of unit_type

    \return the status of the parse
*/
one_net_status_t ona_parse_unit_type_status(const UInt8 *MSG_DATA,
  ona_unit_type_t *unit_type, UInt8 *unit_count)
{
    *unit_type = get_first_two_msg_bytes(MSG_DATA);
    *unit_count = get_third_msg_byte(MSG_DATA);
    return ONS_SUCCESS;
} // ona_parse_unit_type_status //


#ifndef _ONE_NET_MASTER
    /*!
        \brief Parse a Unit Type Query msg

        \param[in] MSG_DATA, msg data
        \param[out] unit_type_index, index of unit type for query

        \return the status of the parse
    */
    one_net_status_t ona_parse_unit_type_query(const UInt8 *MSG_DATA,
      UInt8 *unit_type_index)
    {
        *unit_type_index = get_first_msg_byte(MSG_DATA);
        return ONS_SUCCESS;
    } // ona_parse_unit_type_query //
#endif // ifndef MASTER //

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

