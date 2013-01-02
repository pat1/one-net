//! \defgroup TAL_ADI Processor Abstraction Layer for ADI ADF7025.
//! \ingroup ADI
//! @{


#ifndef _TAL_ADI_H
#define _TAL_ADI_H


/*
    Copyright (c) 2011, Threshold Corporation
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
    \file tal_adi.h
    \brief TAL Abstraction layer for the ADI ADF7025.

    This file declares the transceiver specific functionality needed by
    the ADI transceiver.
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/

#include "one_net_status_codes.h"
#include "one_net_types.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup TAL_ADI_const
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup TAL_ADI_typedefs
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup TAL_ADI_pub_var
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup TAL_ADI_pub_func
//! \ingroup TAL_ADI
//! @{


one_net_status_t init_rf_interrupts(UInt8 DATA_RATE);



//! @} TAL_ADI_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ADI


#endif // ifndef _TAL_ADI_H
