#ifndef _ONA_OPENING_H
#define _ONA_OPENING_H

//! \defgroup ONE-NET_APP_opening ONE-NET Application Layer - Opening
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
    \file ona_opening.h
    \brief Declarations for ONE-NET opening msgs

    These functions should be used to send and parse opening messages.
*/

#include "one_net.h"
#include "one_net_types.h"
#include "one_net_status_codes.h"
#include "one_net_application.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_opening_const
//! \ingroup ONE-NET_APP_opening
//! @{

enum
{
    //! Index for source unit field in data portion of open payload.
    ONA_OPEN_SRC_UNIT_IDX,

    //! Index for destination unit field in data portion of open payload
    ONA_OPEN_DST_UNIT_IDX,

    //! Index for status field in data portion of open payload
    ONA_OPEN_STATUS_IDX
};

//! @} ONE-NET_APP_opening_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_opening_typedefs
//! \ingroup ONE-NET_APP_opening
//! @{

typedef enum _ona_opening_status
{
    ONA_CLOSED = 0x00,              //!< Opening CLOSED
    ONA_OPEN = 0x01,                //!< Opening OPEN
} ona_opening_status_t;

//! @} ONE-NET_APP_opening_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_opening_pub_var
//! \ingroup ONE-NET_APP_opening
//! @{

//! @} ONE-NET_APP_opening_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_opening_pub_func
//! \ingroup ONE-NET_APP_opening
//! @{

one_net_status_t ona_send_opening_status(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_opening_status_t OPENING_STATUS,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_opening_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_opening_status_t OPENING_STATUS,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_opening_query(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const one_net_raw_did_t * const RAW_DST);

one_net_status_t parse_opening(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * src_unit, UInt8 * dst_unit,
  ona_opening_status_t * opening_status);

//! @} ONE-NET_APP_opening_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_opening

#endif // _ONA_OPENING_H //

