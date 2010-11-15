//! \defgroup dfi_port_specific Data Flash Interface
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
    \file dfi_port_specific.c

    \brief Contains port specific declarations for functions associated with data flash interface
      routines.

    A list of dfi segment types that are being used must be defined so that
    the dfi functions know which segment types should be copied to the new
    data flash block when the current data flash block becomes full.
*/

#include "one_net_types.h"
#include "dfi.h"


//==============================================================================
//								CONSTANTS
//! \defgroup dfi_const
//! \ingroup dfi_port_specific
//! @{


//! @} dfi_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup dfi_typedefs
//! \ingroup dfi_port_specific
//! @{


//! @} dfi_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup dfi_pub_var
//! \ingroup dfi_port_specific
//! @{

//! Only the dfi_segment_type_t's in the following table will be copied
//! to the new data flash block when the current data flash block is full.
const UInt8 dfi_segment_types_used[] =
{
    DFI_ST_DEVICE_MFG_DATA,
    DFI_ST_ONE_NET_MASTER_SETTINGS,
    DFI_ST_ONE_NET_CLIENT_SETTINGS,
    DFI_ST_APP_DATA_1,
    DFI_ST_APP_DATA_2,
    DFI_ST_APP_DATA_3,
    DFI_ST_APP_DATA_4
};

//! the number of entries in dfi_segment_types_used
const UInt8 dfi_segment_types_used_count = sizeof(dfi_segment_types_used);

//! Only the dfi_segment_type_t's in the following table will be copied
//! to the new data flash block when the CLI erase command is executed.
const dfi_segment_type_t dfi_segment_types_permanent[] =
{
    DFI_ST_DEVICE_MFG_DATA
};

//! the number of entries in dfi_segment_types_used
const UInt8 dfi_segment_types_permanent_count = sizeof(dfi_segment_types_permanent);


//! @} dfi_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup dfi_pub_func
//! \ingroup dfi_port_specific
//! @{


//! @} dfi_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} dfi_port_specific
