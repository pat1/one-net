#ifndef _ONA_SWITCH_H
#define _ONA_SWITCH_H

//! \defgroup ONE-NET_APP_switch ONE-NET Application Layer - Switch
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
    \file ona_switch.h
    \brief Declarations for ONE-NET switch msgs

    These functions should be used to send and parse switch messages.
*/

#include <one_net/one_net.h>
#include <one_net/port_specific/one_net_types.h>
#include <one_net/one_net_status_codes.h>
#include <one_net/one_net_application.h>


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

typedef enum _ona_switch_status
{
    ONA_OFF    = 0x00,  //!< Switch Onff
    ONA_ON     = 0x01,  //!< Switch On
    ONA_TOGGLE = 0x02,  //!< Switch Toggle (on->off, off->on)
} ona_switch_status_t;

//! @} ONE-NET_APP_switch_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_switch_pub_var
//! \ingroup ONE-NET_APP_switch
//! @{

//! @} ONE-NET_APP_switch_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_switch_pub_func
//! \ingroup ONE-NET_APP_switch
//! @{

one_net_status_t ona_send_switch_status(
        UInt8 SRC_UNIT, UInt8 DST_UNIT, UInt16 SWITCH_STATUS,
        const one_net_raw_did_t *RAW_DST
);

one_net_status_t ona_send_switch_command(
        UInt8 SRC_UNIT, UInt8 DST_UNIT, UInt16 SWITCH_STATUS,
        const one_net_raw_did_t *RAW_DST
);

one_net_status_t ona_send_switch_query(
        UInt8 SRC_UNIT, UInt8 DST_UNIT,
        const one_net_raw_did_t *RAW_DST);

one_net_status_t ona_parse_switch(
        const UInt8 *MSG_DATA,
        UInt8 *src_unit, UInt8 *dst_unit, UInt16 *switch_status
);

//! @} ONE-NET_APP_switch_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_switch

#endif // _ONA_SWITCH_H //

