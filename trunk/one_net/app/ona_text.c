//! \addtogroup ONE-NET_APP_text
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
    \file ona_text.c
    \brief Implementation of text msg functions.

    This is the implementation of functions to send an parse text msgs.
*/

#include <one_net/app/ona_text.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_text_const
//! \ingroup ONE-NET_APP_text
//! @{

//! @} ONE-NET_APP_text_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_text_typedefs
//! \ingroup ONE-NET_APP_text
//! @{

//! @} ONE-NET_APP_text_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_text_pri_var
//! \ingroup ONE-NET_APP_text
//! @{

//! @} ONE-NET_APP_text_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_text_pri_func
//! \ingroup ONE-NET_APP_text
//! @{

//! @} ONE-NET_APP_text_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_text_pub_func
//! \ingroup ONE-NET_APP_text
//! @{

/*!
    \brief Send a simple text command msg

    \param[in] TEXT, The characters to send (up to ONA_MSG_DATA_LEN chars)
    \param[in] LEN, The number of characters passed in.
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_simple_text_command(const UInt8 * const TEXT,
  const UInt8 LEN, const one_net_raw_did_t * const RAW_DST)
{
    const UInt16 CLASS_TYPE = ONA_COMMAND | ONA_SIMPLE_TEXT;

    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    if(!TEXT || !LEN || LEN > ONA_MSG_DATA_LEN || !RAW_DST)
    {
        return ONS_BAD_PARAM;
    } // if the parameters are invalid //

    one_net_int16_to_byte_stream(CLASS_TYPE, &payload[ONA_MSG_HDR_IDX]);
    one_net_memmove(&(payload[ONA_MSG_DATA_IDX]), TEXT, LEN);

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT);

    return rv;
} // ona_send_text_command //

//! @} ONE-NET_APP_text_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_text_pri_func
//! \ingroup ONE-NET_APP_text
//! @{

//! @} ONE-NET_APP_text_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_text

