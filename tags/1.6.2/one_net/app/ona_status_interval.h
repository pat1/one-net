#ifndef _ONA_STATUS_INTERVAL_H
#define _ONA_STATUS_INTERVAL_H

//! \defgroup ONE-NET_APP_status_interval ONE-NET Application Layer - Status
//!   Interval
//! \ingroup ONE_NET_APP
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
    \file status_interval.h
    \brief Declarations for ONE-NET status interval msgs

    These functions SHOULD be called by application code to send and
    process status interval msgs.
*/

#include "one_net.h"
#include "one_net_types.h"
#include "one_net_status_codes.h"
#include "one_net_application.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_status_interval_const
//! \ingroup ONE-NET_APP_status_interval
//! @{

enum
{
    //! Index for source unit field in data portion of status interval payload.
    ONA_STI_SRC_UNIT_IDX,

    //! Index for destination unit field in data portion of status interval
    //! payload
    ONA_STI_DST_UNIT_IDX,

    //! Index for status field in data portion of status interval payload
    ONA_STI_STATUS_IDX
};

//! @} ONE-NET_APP_status_interval_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_status_interval_typedefs
//! \ingroup ONE-NET_APP_status_interval
//! @{

typedef enum _ona_interval_scale
{
    ONA_SECONDS,
    ONA_MINUTES,
    ONA_HOURS,
    ONA_INVALID
} ona_interval_scale_t;

//! @} ONE-NET_APP_status_interval_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_status_interval_pub_var
//! \ingroup ONE-NET_APP_status_interval
//! @{

//! @} ONE-NET_APP_status_interval_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_status_interval_pub_func
//! \ingroup ONE-NET_APP_status_interval
//! @{

one_net_status_t ona_send_status_interval_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_status_interval_status(
  const ona_interval_scale_t SCALE, const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 INTERVAL, const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_status_interval_command(
  const ona_interval_scale_t SCALE, const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 INTERVAL, const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_parse_status_interval(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * src_unit, UInt8 * dst_unit, UInt8 * interval);

//! @} ONE-NET_APP_status_interval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_status_interval

#endif // _ONA_STATUS_INTERVAL_H //

