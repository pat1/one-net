#ifndef _ONA_VOLTAGE_SIMPLE_H
#define _ONA_VOLTAGE_SIMPLE_H

//! \defgroup ONE-NET_APP_volt_simple ONE-NET Application Layer - Voltage Simple
//! \ingroup ONE-NET_APP
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    \file ona_voltage_simple.h
    \brief Declarations for ONE-NET voltage simple msgs

    These functions should be used to send and parse voltage simple messgaes.
*/

#include "one_net.h"
#include "one_net_types.h"
#include "one_net_status_codes.h"
#include "one_net_application.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_volt_simple_const
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_APP_volt_simple_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_volt_simple_typedefs
//! \ingroup ONE-NET_APP_volt_simple
//! @{

typedef enum _ona_voltage_simple_status
{
    ONA_VOLTAGE_BAD  = 0x00,         //!< Voltage status bad
    ONA_VOLTAGE_GOOD = 0x01,        //!< Voltage status good
} ona_voltage_simple_status_t;

//! @} ONE-NET_APP_volt_simple_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_volt_simple_pub_var
//! \ingroup ONE-NET_APP_volt_simple
//! @{

//! @} ONE-NET_APP_volt_simple_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_volt_simple_pub_func
//! \ingroup ONE-NET_APP_volt_simple
//! @{

one_net_status_t ona_send_voltage_simple_status(
  UInt8 voltage_simple_status, const one_net_raw_did_t *raw_dst);

//! @} ONE-NET_APP_volt_simple_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_volt_simple

#endif // _ONA_VOLTAGE_SIMPLE_H //

