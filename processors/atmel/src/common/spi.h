#ifndef _SPI_H
#define _SPI_H

//! \defgroup SPI SPI Driver for the R8C series microcontroller.
//! \ingroup PAL
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
    \file spi.h
    \brief Driver for the spi port on the R8C series microcontroller.
*/


//=============================================================================
//                                  CONSTANTS
//! \defgroup SPI_const
//! \ingroup SPI
//! @{

//! @} SPI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup SPI_typedefs
//! \ingroup SPI
//! @{

//! @} SPI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup SPI_pub_var
//! \ingroup SPI
//! @{

//! @} SPI_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup SPI_pub_func
//! \ingroup SPI
//! @{
    
void init_spi(const BOOL MASTER, const UInt8 CLK_POLARITY, const UInt8 PHASE,
  const BOOL MSB);

BOOL spi_transfer(const UInt8 * const TX_DATA, const UInt8 TX_LEN,
  UInt8 * const rx_data, const UInt8 RX_LEN);

//! @} SPI_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} SPI

#endif // _SPI_H //
