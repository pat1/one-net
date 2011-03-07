//! \addtogroup ONE-NET_APP_Insteon
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
    \file ona_insteon.c
    \brief Implementation of insteon msg functions.

    This is the implementation of functions to send an parse
    insteon msgs.
*/

#include <one_net/app/ona_insteon.h>

//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_Insteon_const
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_Insteon_typedefs
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_Insteon_pri_var
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_Insteon_pri_func
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_Insteon_pub_func
//! \ingroup ONE-NET_APP_Insteon
//! @{

/*!
    \brief Send a insteon to address msg

    \param[in] SRC_UNIT, the source unit of the humidity message
    \param[in] DST_UNIT, the destination unit for humidity message
    \param[in] ADDR, insteon to address
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_insteon_to_addr(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const insteon_addr_t TO_ADDR,
  const one_net_raw_did_t * RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    one_net_int16_to_byte_stream(ONA_INSTEON_TO_ADDRESS,
      &payload[ONA_MSG_HDR_IDX]);
    one_net_memmove(&(payload[ONA_ION_ADDR_IDX + ONA_MSG_DATA_IDX]), TO_ADDR,
      INSTEON_ADDR_LEN);

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_insteon_to_addr //


/*!
    \brief Send an insteon command msg

    \param[in] SRC_UNIT, the source unit of the humidity message
    \param[in] DST_UNIT, the destination unit for humidity message
    \param[in] FLAGS, insteon flags
    \param[in] CMD, the Insteon command1/command2 field
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_insteon_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 FLAGS, const insteon_command_t CMD,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    one_net_int16_to_byte_stream(ONA_INSTEON_COMMAND,
      &payload[ONA_MSG_HDR_IDX]);

    payload[ONA_ION_FLAG_IDX + ONA_MSG_DATA_IDX] = FLAGS;
    payload[ONA_ION_CMD1_IDX + ONA_MSG_DATA_IDX] = CMD[0];
    payload[ONA_ION_CMD2_IDX + ONA_MSG_DATA_IDX] = CMD[1];

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_insteon_command //


/*!
    \brief parse a bind msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] to_addr, insteon to address

    \return the status of the send action
*/
one_net_status_t ona_parse_insteon_to_addr(const UInt8 * const MSG_DATA,
  const UInt8 LEN, insteon_addr_t * to_addr)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    }

    if(proceed)
    {
        // get addr
        one_net_memmove(*to_addr, MSG_DATA, INSTEON_ADDR_LEN);
    } // if proceed //

    return rv;
} // ona_parse_insteon_to_addr //


/*!
    \brief parse a insteon command msg

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] flags, insteon flags
    \param[out] insteon_command, command 1,2 field of insteon msg

    \return the status of the send action
*/
one_net_status_t ona_parse_insteon_command(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * flags, insteon_command_t * insteon_command)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    }

    if(proceed)
    {
        // get addr
        *flags = MSG_DATA[ONA_ION_FLAG_IDX];
        *insteon_command[0] = MSG_DATA[ONA_ION_CMD1_IDX];
        *insteon_command[1] = MSG_DATA[ONA_ION_CMD2_IDX];

    } // if proceed //

    return rv;
} // ona_parse_insteon_command //


#ifndef _ONE_NET_SIMPLE_CLIENT
    /*!
        \brief Sends a requeust to send insteon extended block message

        \param[in] SRC_UNIT The unit sending the message.
        \param[in] RAW_DST The did of the device that will receive the Insteon

        \return SUCCESS if the request was successfully queued.
                BAD_PARAM if any of the parameters are invalid
    */
    one_net_status_t ona_send_insteon_extended_command_request(
      const UInt8 SRC_UNIT, const one_net_raw_did_t * const RAW_DST)
    {
        if(!RAW_DST)
        {
            return ONS_BAD_PARAM;
        } // if any of the parameters are invalid //

        return one_net_block_stream_request(ON_BLOCK, TRUE,
          ONA_BLOCK_INSTEON_EXTENDED, INSTEON_EXTENDED_LEN,
          ONE_NET_LOW_PRIORITY, RAW_DST, ONE_NET_DEV_UNIT);
    } // ona_send_insteon_extended_command_request //


    /*!
        \brief Parses an Insteon Extended command message.

        \param[in] MSG_DATA The data portion of the block data message.
        \param[in] LEN The number of bytes MSG_DATA points to.
        \param[out] to_addr The Insteon address contained in the message.
        \param[out] flags The flags field from the message.
        \param[out] cmd The command field from the message.
        \param[out] user_data The user data field from the message.  This must
          be at least INSTEON_USER_DATA_LEN bytes.
        \param[in] USER_DATA_LEN The size in bytes of user_data

        \return SUCCESS if parsing the data was successful
                BAD_PARAM If any of the parameters are invalid
    */
    one_net_status_t ona_parse_insteon_extended_command(
      const UInt8 * const MSG_DATA, const UInt8 LEN,
      insteon_addr_t * const to_addr,  UInt8 * const flags,
      insteon_command_t * const cmd, UInt8 * const user_data,
      const UInt8 USER_DATA_LEN)
    {
        if(!MSG_DATA || LEN < INSTEON_EXTENDED_LEN  || !to_addr || !flags
          || !cmd || !user_data || USER_DATA_LEN < INSTEON_USER_DATA_LEN)
        {
            return ONS_BAD_PARAM;
        } // if any of the parameters are invalid //

        one_net_memmove(*to_addr, &(MSG_DATA[ONA_IEX_ADDR_IDX]),
          sizeof(*to_addr));
        *flags = MSG_DATA[ONA_IEX_FLAG_IDX];
        one_net_memmove(cmd, &(MSG_DATA[ONA_IEX_CMD1_IDX]), sizeof(*cmd));
        one_net_memmove(user_data, &(MSG_DATA[ONA_IEX_USER_DATA_IDX]),
          USER_DATA_LEN);

        return ONS_SUCCESS;
    } // ona_parse_insteon_extended_command //
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! @} ONE-NET_APP_Insteon_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_Insteon_pri_func
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_Insteon

