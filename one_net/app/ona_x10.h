#ifndef _ONA_X10_H
#define _ONA_X10_H

//! \defgroup ONE-NET_APP_X10 ONE-NET Application Layer - X10
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
    \file ona_x10.h
    \brief Declarations for ONE-NET x10 msgs

    These functions should be used to send and parse x10 messages.
*/

#include "one_net.h"
#include "one_net_types.h"
#include "one_net_status_codes.h"
#include "one_net_application.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_X10_const
//! \ingroup ONE-NET_APP_X10
//! @{

enum
{
    ONA_X10_MSG_LEN = 2,
    ONA_X10_EXTENDED_MSG_LEN = 3
};

enum
{
    ONA_X10_MAX_UNIT = 0x1E,        //!< The max x10 unit value
    ONA_X10_MAX_SIMPLE_CMD = 0x1F   //!< The max x10 simple command value
};

//! @} ONE-NET_APP_X10_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_X10_typedefs
//! \ingroup ONE-NET_APP_X10
//! @{

//! The X10 houses
typedef enum
{
    ONA_X10_HOUSE_M,
    ONA_X10_HOUSE_E,
    ONA_X10_HOUSE_C,
    ONA_X10_HOUSE_K,
    ONA_X10_HOUSE_O,
    ONA_X10_HOUSE_G,
    ONA_X10_HOUSE_A,
    ONA_X10_HOUSE_I,
    ONA_X10_HOUSE_N,
    ONA_X10_HOUSE_F,
    ONA_X10_HOUSE_D,
    ONA_X10_HOUSE_L,
    ONA_X10_HOUSE_P,
    ONA_X10_HOUSE_H,
    ONA_X10_HOUSE_B,
    ONA_X10_HOUSE_J,

    //! The last valid X10 house
    ONA_X10_MAX_HOUSE = ONA_X10_HOUSE_J
} ona_x10_house_t;

typedef UInt8 ona_x10_msg_t[ONA_X10_MSG_LEN];
typedef UInt8 ona_x10_extended_msg_t[ONA_X10_EXTENDED_MSG_LEN];

//! @} ONE-NET_APP_X10_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_X10_pub_var
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_X10_pub_func
//! \ingroup ONE-NET_APP_X10
//! @{

// SEND FUNCTIONS
one_net_status_t ona_send_x10_simple(const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 HOUSE, const UInt8 UNIT, const UInt8 CMD,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_x10_stream(const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const ona_msg_type_t MESSAGE_TYPE, const UInt8 HOUSE_1,
  const UInt8 UNIT_CMD_1, const UInt8 HOUSE_2, const UInt8 UNIT_CMD_2,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_x10_extended(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 HOUSE, const UInt8 UNIT,
  const UInt8 DATA_BYTE, const UInt8 CMD_BYTE,
  const one_net_raw_did_t * const RAW_DST);


// PARSE FUNCTIONS
one_net_status_t ona_parse_x10_simple(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * house, UInt8 * unit, UInt8 * x10_cmd);

one_net_status_t ona_parse_x10_stream(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * house_1, UInt8 * unit_cmd_1, 
  UInt8 * house_2, UInt8 * unit_cmd_2);

one_net_status_t ona_parse_x10_extended(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * house, UInt8 * unit, UInt8 * data_byte,
  UInt8 * cmd_byte);

//! @} ONE-NET_APP_X10_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_X10

#endif // _ONA_X10_H //

