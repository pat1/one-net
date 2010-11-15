//! \addtogroup HAL_SEMTECH Processor Abstraction Layer for Semtech XE1205
//! \ingroup SEMTECH
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
    \file hal_semtech.c
    \brief Processor abstraction layer for the Semtech XE1205.

    This file implements the processor specific functionality needed by the
    Semtech transceiver (such as interrupts for the various communication).
*/

#include "hal_semtech.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "spi.h"
#include "tal.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup HAL_SEMTECH_const
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_SEMTECH_typedefs
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup HAL_SEMTECH_pri_var
//! \ingroup HAL_SEMTECH
//! @{

// These are derived from semtech.c
extern UInt16 rf_idx;
extern UInt16 rf_count;
extern UInt8 rf_data[];

extern UInt8 bit_mask;

//! @} HAL_SEMTECH_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup HAL_SEMTECH_pri_func
//! \ingroup HAL_SEMTECH
//! @{

//! @} HAL_SEMTECH_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup HAL_SEMTECH_pub_func
//! \ingroup HAL_SEMTECH
//! @{

/*!
    \brief Initialize ports used by the SEMTECH.

    \param void

    \return void
*/
void tal_init_ports(void)
{
    RF_DATA_DIR = INPUT;

    IRQ0_DIR = INPUT;
    DCLK_IRQ1_DIR = INPUT;
    
    NSS_CFG_DIR = OUTPUT;
    NSS_DATA_DIR = OUTPUT;
    
    NSS_CFG = 1;
    NSS_DATA = 1;
    
    init_spi(TRUE, 0, 0, TRUE);
} // tal_init_ports //


/*!
    \brief Initialize interrupts needed for XE1205 operation.

    Timer Z is used as the bit timer for transmitting data.  The INT0 interrupt
    is triggered by the IRQ1 signal from the Semtech when receiving data.

    \param void

    \return void
*/
void init_rf_interrupts(void)
{
    // initialize Timer RZ to timer mode for rf output
    // TIMER z count source = f1
    DISABLE_TX_BIT_INTERRUPTS();
    DISABLE_RX_BIT_INTERRUPTS();
    
    tck0_tramr = 0;
    tck1_tramr = 0;
    tck2_tramr = 0;
    
    /*
        Prescaler and timer register initialization
        
        19.5MHz : 254 * 2 ~ 26.05us for 38400 bits/sec
        possible)
    */
    trapre = 254 - 1;               // Setting Prescaler A register
    tra = 2 - 1;                    // Setting timer A register
    
    tramr = 0x02;                   // Timer A : event counter mode
    traic = 0x05;                   // Int priority level = 5, clear request
    
    // initialize the INT1 interrupt to handle DATACLK for receive mode
    // rising edge instead of falling edge
    int0pl = 0;
    pol_int0ic = 1;
    int0ic |= 0x07;
    ir_int0ic = 0;
} // init_rf_interrupts //

//! @} HAL_SEMTECH_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup HAL_SEMTECH_pri_func
//! \ingroup HAL_SEMTECH
//! @{

/*!
    \brief ISR for bit timer to transmit data

    \param void

    \return void
*/
#pragma interrupt tx_bit_isr
void tx_bit_isr(void)
{
    if(rf_data[rf_idx] & bit_mask)
    {
        RF_DATA = 1;
    } // if the bit is high //
    else
    {
        RF_DATA = 0;
    } // else the bit is low //
    
    bit_mask >>= 1;
    if(bit_mask == 0x00)
    {
        // reset to first bit of next byte
        bit_mask = 0x80;
        rf_idx++;
    } // if done with current byte //
} // tx_bit_isr //


/*!
    \brief ISR for the data clock

    The ISR that should be called once every bit time during rf transmission or
    reception.

    \param void
    \return  void
*/
#pragma interrupt dataclk_isr
void dataclk_isr(void)
{
    if(RF_DATA)
    {
        rf_data[rf_count] |= bit_mask;
    } // if a 1 was receeived //
    else
    {
        rf_data[rf_count] &= ~bit_mask;
    } // else a 0 was received //

    bit_mask >>= 1;
    if(bit_mask == 0)
    {
        bit_mask = 0x80;
        if(++rf_count >= ONE_NET_MAX_ENCODED_PKT_LEN)
        {
            rf_count = 0;
        } // if the end of the receive buffer has been passed //
    } // if done receiving a byte //
} // dataclk_isr //

//! @} HAL_SEMTECH_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} SEMTECH
