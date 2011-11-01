//! \addtogroup TAL_ADI Processor Abstraction Layer for ADI ADF7025.
//! \ingroup ADI
//! @{

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
    \file tal_adi.c
    \brief Processor abstraction layer for the ADI ADF7025.

    This file implements the functionality needed by the ADI
    transceiver (such as interrupts for the various communication).  This file
    is for R8C devices that have the ADI set up to use TimerZ has the transmit
    bit interrupt, and INT1 as the data clock interrupt for receive mode.
*/

#include "tal_adi.h"
#include "hal_adi.h"
#include "one_net_data_rate.h"
#include "io_port_mapping.h"
#include "tal.h"

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
//                              PRIVATE VARIABLES
//! \defgroup TAL_ADI_pri_var
//! \ingroup TAL_ADI
//! @{

//! @} TAL_ADI_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TAL_ADI_pri_func
//! \ingroup TAL_ADI
//! @{



//! @} TAL_ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TAL_ADI_pub_func
//! \ingroup TAL_ADI
//! @{



/*!
    \brief Initialize interrupts needed for ADI 7025 operation.

    Timer Z is used as the bit timer for transmitting data.  The INT1 interrupt
    is triggered by the DATACLK signal from the ADI when receiving data.

    \param void

    \return TRUE if successful, FALSE otherwise
*/
one_net_status_t init_rf_interrupts(UInt8 DATA_RATE)
{
    /*
        Prescaler and timer register initialization
        Crystal Frequency =      22.118 MHz
        22.118 MHz / 2 = 11.0592 MHz
        Period = (1 / 11.0592MHz) = 0.00000009042245 seconds.
        Period for 38400 Hz transmission rate = (1 / 38400 Hz) = 0.0000260417 sec.

        0.0000260417 sec. / 0.00000009042245 sec. = 288.0004

        Note that this is the same as dividing 11.0592MHz by 38400 KHz =
        11059.2 KHz / 38.400 KHz = 288

        For Higher Frequencies...

        11059.2 KHz / 76.800 KHz = 144
        11059.2 KHz / 115.200 KHz = 96
        11059.2 KHz / 153.600 KHz = 72
        11059.2 KHz / 192.000 KHz = 57.6 (round to 58)
        11059.2 KHz / 230.400 KHz = 48

        There are some transmitters where there is a division that goes on
        as well.  In our case, there is not, but we'll keep it in the
        equation as a trivial (1 / 1) term that we can ignore.  We want to
        solve for x...

        (1/1) * x = 288   (replace 288 with 144, 96, 72, 58, or 48 i needed)

        Once x is calculated (trivial in our ccase since we have already done
        the bulk of the calculation earlier, we want to find two integers which
        multiply to equal x.

        16 * 18 = 288 (38400 Hz)  -- data rate 0
        16 *  9 = 144 (76800 Hz)  -- data rate 1
        12 *  8 =  96 (115200 Hz) -- data rate 2
         8 *  9 =  72 (153600 Hz) -- data rate 3
         2 * 29 =  58 (192000 Hz) -- data rate 4
         6 * 8 =  48 (230400 Hz)  -- data rate 5


        We'll need to subtract 1 from each of the two numbers and stick them in
        the registers.  Subtraction of 1 occurs in the code
    */

    const UInt8 register_values[ONE_NET_DATA_RATE_LIMIT][2] =
    {
        {16, 18},
        {16, 9},
        {12, 8},
        {8, 9},
        {2, 29},
        {6, 8}
    };


    // initialize Timer RA to to timer mode for rf output
    // Timer A count source = Event Counter
    DISABLE_TX_BIT_INTERRUPTS();

    tck0_tramr = 0;
    tck1_tramr = 0;
    tck2_tramr = 0;

    trapre = register_values[DATA_RATE][0] - 1; // Setting Prescaler A register
    tra   = register_values[DATA_RATE][1] - 1;  // Setting timer A register

    tramr = 0x02;                   // Timer A : event counter mode
    traic = 0x05;                   // Int priority level = 5, clear request

    // initialize the INT0 interrupt to handle DATACLK for receive mode
    // rising edge instead of falling edge
    tedgf_tracr = 0;
    int0ic = 0x05;

    return ONS_SUCCESS;
} // init_rf_interrupts //



//! @} TAL_ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ADI
