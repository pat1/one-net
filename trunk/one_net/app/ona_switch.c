//! \addtogroup ONE-NET_APP_switch
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
    \file ona_switch.c
    \brief Implementation of switch msg functions.

    This is the implementation of functions to send an parse switch msgs.
*/

#include <one_net/app/ona_switch.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_switch_const
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_switch_typedefs
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_switch_pri_var
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_switch_pri_func
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_switch_pub_func
//! \ingroup ONE-NET_APP_switch
//! @{

/*!
    \brief Send a switch status msg


    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] SWITCH_STATUS, switch status (ON/OFF/TOGGLE)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_switch_status(
        UInt8 SRC_UNIT, UInt8 DST_UNIT, UInt16 SWITCH_STATUS,
        const one_net_raw_did_t *RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    put_msg_hdr(ONA_STATUS|ONA_SWITCH,payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    put_msg_data(SWITCH_STATUS, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONA_MSG_NUM_BYTES, ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT);
} // ona_send_switch_status //


/*!
    \brief Send a switch command msg

    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] SWITCH_STATUS, switch status (ON/OFF/TOGGLE)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_switch_command(
        UInt8 SRC_UNIT, UInt8 DST_UNIT, UInt16 SWITCH_STATUS,
        const one_net_raw_did_t *RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    put_msg_hdr(ONA_COMMAND|ONA_SWITCH,payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    put_msg_data(SWITCH_STATUS, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONA_MSG_NUM_BYTES, ONE_NET_SEND_SINGLE_PRIORITY,
      RAW_DST, SRC_UNIT);
} // ona_send_switch_command //


/*!
    \brief Send a switch query msg

    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_switch_query(
        UInt8 SRC_UNIT, UInt8 DST_UNIT,
        const one_net_raw_did_t *RAW_DST)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    put_msg_hdr(ONA_QUERY|ONA_SWITCH, payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    //put_msg_data(???, payload); // Maybe make it random???

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload),
      ONA_MSG_NUM_BYTES, ONE_NET_HIGH_PRIORITY,
      RAW_DST, SRC_UNIT);
} // ona_send_switch_query //


/*!
    \brief parse a switch msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[out] src_unit, the source unit of the msg
    \param[out] dst_unit, the destination unit of the msg
    \param[out] switch_status, status of switch

    \return the status of the send action

    dje: Note that this is used for parsing queries as well as
         commands. Therefore, the status field of the query
         must hold a valid value (ON, OFF, TOGGLE)
*/
one_net_status_t ona_parse_switch(
        const UInt8 *MSG_DATA,
        UInt8 *src_unit, UInt8 *dst_unit, UInt16 *switch_status)
{
    // error checking
    if(!MSG_DATA || !src_unit || !dst_unit
      || !switch_status)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // get units
    *src_unit = get_src_unit(MSG_DATA);
    *dst_unit = get_dst_unit(MSG_DATA);

    // switch status
    *switch_status = get_msg_data(MSG_DATA);

    // make sure a valid value was received
    switch(*switch_status)
    {
        case ONA_OFF:               // fall through
        case ONA_ON:                // fall through
        case ONA_TOGGLE:
        {
            break;
        } // valid values case //

        default:
        {
            return ONS_INVALID_DATA;
        } // default case //
    } // switch(switch_status) //

    return ONS_SUCCESS;
} // ona_parse_switch //

//! @} ONE-NET_APP_switch_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_switch_pri_func
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_switch

