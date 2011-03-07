//! \addtogroup ONE-NET_APP_volt_simple
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
    \file ona_voltage_simple.c
    \brief Implementation of voltage simple msg functions.

    This is the implementation of functions to send voltage simple msgs.
*/

#include <one_net/app/ona_voltage_simple.h>



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_volt_simple_const
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_const_APP_volt_simple
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_volt_simple_typedefs
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_APP_volt_simple_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_volt_simple_pri_var
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_APP_volt_simple_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_volt_simple_pri_func
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_APP_volt_simple_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_volt_simple_pub_func
//! \ingroup ONE-NET_APP_volt_simple
//! @{

/*!
    \brief Send a switch status msg


    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] SWITCH_STATUS, switch status (ON/OFF/TOGGLE)
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_voltage_simple_status(
  UInt8 voltage_simple_status, const one_net_raw_did_t *raw_dst)
{
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};

    put_msg_hdr(ONA_STATUS|ONA_VOLTAGE_SIMPLE, payload);
    put_first_msg_byte(voltage_simple_status, payload);

    // send payload
    return (*one_net_send_single)(payload, sizeof(payload), sizeof(payload),
      ONE_NET_HIGH_PRIORITY, raw_dst, ONE_NET_DEV_UNIT);
} // ona_send_voltage_simple_status //

//! @} ONE-NET_APP_volt_simple_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_volt_simple_pri_func
//! @{

//! @} ONE-NET_APP_volt_simple_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET

