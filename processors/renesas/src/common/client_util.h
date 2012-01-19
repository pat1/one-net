#ifndef _CLIENT_UTIL_H
#define _CLIENT_UTIL_H

//! \defgroup CLIENT_util ONE-NET CLIENT utility functionality
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
    \file client_util.h
    \brief ONE-NET CLIENT utility functionality.

    This file contains the interface for utility functionality used by ONE-NET
    CLIENTs.  Examples of declarations that may be in this file are are the
    (re)storing of parameters to non-volatile memory.
*/

#include "one_net_types.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup client_util_const
//! \ingroup client_util
//! @{

//! @} client_util_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup client_util_typedefs
//! \ingroup client_util
//! @{


//! Values for representing the type of data stored in non-volatile memory
typedef enum
{
    //! Value that represents start of unused flash data
    INVALID_FLASH_DATA = 0xFF,
    
    //! Value that represents the start of ONE-NET parameters
    ONE_NET_CLIENT_FLASH_NV_DATA = 0x00,
    
    #ifdef _PEER
    //! Value that represents the start of peer parameters
    ONE_NET_CLIENT_FLASH_PEER_DATA = 0x01
    #endif
} nv_data_t;


//! @} client_util_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup client_util_pub_var
//! \ingroup client_util
//! @{

//! @} client_util_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup client_util_pub_func
//! \ingroup client_util
//! @{

const UInt8 * read_param(UInt16 * const len);

void clr_flash(void);
void flash_erase_check(void);


/*!
    \brief Checks to see if the data flash should be erased (and erases it if
      it does).

    \param void

    \return void
*/
#define FLASH_ERASE_CHECK() flash_erase_check()

//! @} client_util_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} CLIENT_util

#endif // _CLIENT_UTIL_H //
