#ifndef _FLASH_H
#define _FLASH_H

#include "config_options.h"


//! \defgroup FLASH_R8C Data Flash Functionality for R8C
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
    \file flash.h
    \brief Contains functionality for interacting with the Data Flash on the
      R8C family of processors.
*/

#include "one_net_types.h"
#include "pal.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup FLASH_const
//! \ingroup FLASH
//! @{

/*!
    \brief Data Flash Block sizes & starting addresses.

    These values assume the data flash is layed out in contiguous blocks.
    These values are based on the R8C/1B.  
*/
//! Size of a Data Flash Block
#define DF_BLOCK_SIZE 0x0400


//! Starting address for Data Flash Block A
#define DF_BLOCK_A_START 0x2400

//! Starting address for Data Flash Block B
#define DF_BLOCK_B_START (DF_BLOCK_A_START + DF_BLOCK_SIZE)


//! The very first data block address
#define DF_BLOCK_START DF_BLOCK_A_START

//! The very last data block address
#define DF_BLOCK_END (DF_BLOCK_B_START + DF_BLOCK_SIZE)


//! @} FLASH_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup FLASH_typedefs
//! \ingroup FLASH
//! @{

//! @} FLASH_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup FLASH_pub_var
//! \ingroup FLASH
//! @{

//! @} FLASH_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup FLASH_pub_func
//! \ingroup FLASH
//! @{

UInt16 read_data_flash(const UInt16 ADDR, UInt8 * data, const UInt8 LEN);
UInt16 write_data_flash(const UInt16 ADDR, const UInt8 * data, const UInt8 LEN);
BOOL erase_data_flash(const UInt16 ADDR);

//! @} FLASH_pub_func
//!                     PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} FLASH

#endif // _FLASH_H //
