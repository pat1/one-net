#ifndef _ONA_VOLTAGE_H
#define _ONA_VOLTAGE_H

//! \defgroup ONE-NET_APP_volt ONE-NET Application Layer - Voltage
//! \ingroup ONE-NET_APP
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
    \file ona_voltage.h
    \brief Declarations for ONE-NET voltage messages

    These functions should be used to send and parse voltage messages.
*/

#include <one_net/one_net.h>
#include <one_net/one_net_types.h>
#include <one_net/one_net_status_codes.h>
#include <one_net/one_net_application.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_volt_const
//! \ingroup ONE-NET_APP_volt
//! @{

//! @} ONE-NET_APP_volt_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_volt_typedefs
//! \ingroup ONE-NET_APP_volt
//! @{

typedef enum _ona_voltage_src
{
    //! The device is line powered
    ONA_VOLTAGE_EXTERNAL_STATUS = 0x40,

    //! The device is battery powered
    ONA_VOLTAGE_BATTERY_STATUS = 0x80,

    //! External and Battery voltage status
    ONA_VOLTAGE_EXT_BAT_STATUS
      = ONA_VOLTAGE_EXTERNAL_STATUS | ONA_VOLTAGE_BATTERY_STATUS
} ona_voltage_src_t;

//! @} ONE-NET_APP_volt_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_volt_pub_var
//! \ingroup ONE-NET_APP_volt
//! @{

//! @} ONE-NET_APP_volt_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_volt_pub_func
//! \ingroup ONE-NET_APP_volt
//! @{

one_net_status_t ona_send_voltage_volts_status(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST);

one_net_status_t ona_send_voltage_10ths_volts(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST);

one_net_status_t ona_send_voltage_100ths_volts(UInt8 VOLTAGE_SRC,
  UInt16 BATTERY_VOLTAGE, UInt16 EXTERNAL_VOLTAGE,
  const one_net_raw_did_t * RAW_DST);

one_net_status_t ona_send_voltage_query( const one_net_raw_did_t * RAW_DST);

one_net_status_t ona_parse_voltage_simple(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_status);

one_net_status_t ona_parse_voltage_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage);

one_net_status_t ona_parse_voltage_10ths_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage);

one_net_status_t ona_parse_voltage_100ths_volts(const UInt8 * MSG_DATA,
  UInt8 LEN, UInt8 * voltage_src, UInt16 * battery_voltage,
  UInt16 * external_voltage);

//! @} ONE-NET_APP_volt_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_volt

#endif // _ONA_VOLTAGE_H //
