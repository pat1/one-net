//! \defgroup PAL_ADI Processor Abstraction Layer for ADI ADF7025.
//! \ingroup ADI
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
    \file pal_adi.c
    \brief Processor abstraction layer for the ADI ADF7025.

    This file implements the processor specific functionality needed by the ADI
    transceiver (such as interrupts for the various communication).  This file
    is specific to the adi switch example board using a R8C1B processor.  The
    CLKOUT pin from the ADI is used as the clock for the processor.
*/


//=============================================================================
//                                  CONSTANTS
//! \defgroup PAL_ADI_const
//! \ingroup PAL_ADI
//! @{

//! @} PAL_ADI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup PAL_ADI_typedefs
//! \ingroup PAL_ADI
//! @{

//! @} PAL_ADI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup PAL_ADI_pri_var
//! \ingroup PAL_ADI
//! @{

//! @} PAL_ADI_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup PAL_ADI_pri_func
//! \ingroup PAL_ADI
//! @{

//! @} PAL_ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup PAL_ADI_pub_func
//! \ingroup PAL_ADI
//! @{

/*!
    \brief Initializes the interrupts need for the transceiver.

    For this board configuration, timer z is used as the transmit bit timer.
    INT1 is used as the receive bit interrupt.
    
    \param void
    
    \return void
*/
void init_rf_interrupts(void)
{
    tzmr = 0x00;                    // Timer Z : timer mode
    
    /* 
         Prescaler and timer register initialization
      
        11.0592MHz : 1/1 *   16 * 18 = 26 us for 38400 bits/sec
    */
    prez = 16 - 1;                  // Setting Prescaler for Timer Z register
    tzsc = 0;
    tzpr = 18 - 1;

    tzic = 5;                       // Interrupt priority level = 5

    tzck0 = 0;
    tzck1 = 0;

    ir_tzic = 0;
    DISABLE_BIT_TIMER;
} // init_rf_interrupts //

//! @} PAL_ADI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup PAL_ADI_pri_func
//! \ingroup PAL_ADI
//! @{

/*!
    \brief ISR for bit timer to transmit data.

    \param void

    \return void
*/

#pragma interrupt tx_bit_isr
void tx_bit_isr(void)
{
    if(map == 0)
    {
        // reset to first bit of next byte
        map = 0x80;    
        tx_rf_idx++;
    } // if done with current byte //
    RF_TX_DATA = ((TX_RF_DATA[tx_rf_idx] & map) && 1);

    map >>= 1;
} // clkout_timer_ra_isr //

//! @} PAL_ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} PAL_ADI
