#ifndef _ONA_INSTEON_H
#define _ONA_INSTEON_H

//! \defgroup ONE-NET_APP_Insteon ONE-NET Application Layer - Insteon
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
    \file ona_insteon.h
    \brief Declarations for ONE-NET insteon msgs

    These functions should be used to send and parse insteon messages.
*/

#include <one_net/one_net.h>
#include <one_net/port_specific/one_net_types.h>
#include <one_net/one_net_status_codes.h>
#include <one_net/one_net_application.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_Insteon_const
//! \ingroup ONE-NET_APP_Insteon
//! @{

enum
{
    ONA_INSTEON_ADDR_LEN = 3,
    ONA_INSTEON_COMMAND_LEN = 2,

    ONA_INSTEON_USER_DATA_LEN = 14,

    //! Length in bytes of an INSTEON Extended block message
    ONA_INSTEON_EXTENDED_LEN = 62
};

enum
{
    //! Index of Insteon address in data portion of Insteon to Address payload
    ONA_ION_ADDR_IDX = 0,

    //! Index of flag field in data portion of Insteon Command payload
    ONA_ION_FLAG_IDX = 0,

    //! Index of command 1 field in data portion of Insteon Command payload
    ONA_ION_CMD1_IDX,

    //! Index of command 2 field in data portion of Insteon Command payload
    ONA_ION_CMD2_IDX,

    //! Index of the Insteon address field in the data portion of an Insteon
    //! Extended block packet
    ONA_IEX_ADDR_IDX = 0,

    //! Index of the flag field in the data portion of an Insteon Extended
    //! block packet
    ONA_IEX_FLAG_IDX = ONA_IEX_ADDR_IDX + ONA_INSTEON_ADDR_LEN,

    //! Index of command 1 field in the data portion of an Insteon Extended
    //! block packet
    ONA_IEX_CMD1_IDX,

    //! Index of command 2 field in the data portion of an Insteon Extended
    //! block packet
    ONA_IEX_CMD2_IDX,

    //! Index of the user data field in the data portion of an Insteon Extended
    //! block packet
    ONA_IEX_USER_DATA_IDX
};

//! @} ONE-NET_APP_Insteon_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_Insteon_typedefs
//! \ingroup ONE-NET_APP_Insteon
//! @{

typedef UInt8 ona_insteon_addr_t[ONA_INSTEON_ADDR_LEN];
typedef UInt8 ona_insteon_command_t[ONA_INSTEON_COMMAND_LEN];

//! @} ONE-NET_APP_Insteon_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_APP_Insteon_pub_var
//! \ingroup ONE-NET_APP_Insteon
//! @{

//! @} ONE-NET_APP_Insteon_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_Insteon_pub_func
//! \ingroup ONE-NET_APP_Insteon
//! @{

one_net_status_t ona_send_insteon_to_addr(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const ona_insteon_addr_t TO_ADDR,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_send_insteon_command(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 FLAGS, const ona_insteon_command_t CMD,
  const one_net_raw_did_t * const RAW_DST);

one_net_status_t ona_parse_insteon_to_addr(const UInt8 * const MSG_DATA,
  const UInt8 LEN, ona_insteon_addr_t * to_addr);

one_net_status_t ona_parse_insteon_command(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * flags, ona_insteon_command_t * insteon_command);

#ifndef _ONE_NET_SIMPLE_CLIENT
    one_net_status_t ona_send_insteon_extended_command_request(
      const UInt8 SRC_UNIT, const one_net_raw_did_t * const RAW_DST);

    one_net_status_t ona_parse_insteon_extended_command(
      const UInt8 * const MSG_DATA, const UInt8 LEN,
      ona_insteon_addr_t * const to_addr,  UInt8 * const flags,
      ona_insteon_command_t * const cmd, UInt8 * const user_data,
      const UInt8 USER_DATA_LEN);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //

//! @} ONE-NET_APP_Insteon_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_APP_Insteon

#endif // _ONA_INSTEON_H //

