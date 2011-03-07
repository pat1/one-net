//! \addtogroup HAL_ADI Processor Abstraction Layer for ADI ADF7025.
//! \ingroup ADI
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
    \file hal_adi.c
    \brief Processor abstraction layer for the ADI ADF7025.

    This file implements the processor specific functionality needed by the ADI
    transceiver (such as interrupts for the various communication).  This file
    is for R8C devices that have the ADI set up to use TimerZ has the transmit
    bit interrupt, and INT1 as the data clock interrupt for receive mode.
*/

#include "hal_adi.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "tal.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup HAL_ADI_const
//! \ingroup HAL_ADI
//! @{

//! @} HAL_ADI_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup HAL_ADI_typedefs
//! \ingroup HAL_ADI
//! @{

//! @} HAL_ADI_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup HAL_ADI_pri_var
//! \ingroup HAL_ADI
//! @{

// These are derived from adi.c
extern UInt16 rx_rf_idx;
extern UInt16 rx_rf_count;
extern UInt8 rx_rf_data[];

extern UInt16 tx_rf_len;
extern UInt16 tx_rf_idx;
extern UInt8 * TX_RF_DATA;

extern UInt8 bit_mask;

//! @} HAL_ADI_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup HAL_ADI_pri_func
//! \ingroup HAL_ADI
//! @{

// Considered private, but used by adi.c.
void tx_byte(const UInt8 VAL);

//! @} HAL_ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup HAL_ADI_pub_func
//! \ingroup HAL_ADI
//! @{

/*!
    \brief Initialize ports used by the ADI.

    \param void

    \return void
*/
void tal_init_ports(void)
{
    // pins used to configure registers
    SCLK_DIR = OUTPUT;
    SDATA_DIR = OUTPUT;
    SLE_DIR = OUTPUT;
    SREAD_DIR = INPUT;

    // set up data and clock pins
    RF_DATA_DIR = INPUT;
    RX_BIT_CLK_DIR = INPUT;

    // set up sync detect pin
    SYNCDET_DIR = INPUT;

    // define out compile time if desired
    #ifdef _CHIP_ENABLE
        CHIP_ENABLE_DIR = OUTPUT;
    #endif // ifdef SYNC_DET //
} // tal_init_ports //


/*!
    \brief Initialize interrupts needed for ADI 7025 operation.

    Timer Z is used as the bit timer for transmitting data.  The INT1 interrupt
    is triggered by the DATACLK signal from the ADI when receiving data.

    \param void

    \return void
*/
void init_rf_interrupts(void)
{
    // initialize Timer RA to to timer mode for rf output
    // Timer A count source = Event Counter
    DISABLE_TX_BIT_INTERRUPTS();

    tck0_tramr = 0;
    tck1_tramr = 0;
    tck2_tramr = 0;
    
    /* 
         Prescaler and timer register initialization
      
        11.0592MHz : 1/1 * 16 * 18 = 26 us for 38400 bits/sec
    */
    trapre = 16 - 1;                // Setting Prescaler A register 
    tra   = 18 - 1;                 // Setting timer A register 
    
    tramr = 0x02;                   // Timer A : event counter mode
    traic = 0x05;                   // Int priority level = 5, clear request
    
    // initialize the INT0 interrupt to handle DATACLK for receive mode
    // rising edge instead of falling edge
    tedgf_tracr = 0;
    int0ic = 0x05;
} // init_rf_interrupts //

//! @} HAL_ADI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup HAL_ADI_pri_func
//! \ingroup HAL_ADI
//! @{

#if ON_RF_TRANSFER == ON_POLLED

#error "The tx_byte function needs to be verified that it is set up correctly."

/*!
    \brief Transmits a byte over the rf channel.

    Polls the transmit bit clock and sets the rf data line.  This function
    assumes interrupts have been disabled when it is called, and will also
    update the tick count since interrupts are disabled.

    \param[in] VAL The value to be transmitted.

    \return void
*/
void tx_byte(const UInt8 VAL)
{
    for (UInt8_temp2 = 0x80; UInt8_temp2; UInt8_temp2 >>= 1)
    {
        // global interrupts are disabled, so check the
        // IR bits in a polling fashion
        while (!ir_taic);
        RF_TX_DATA = ((UInt8_temp2 & val) != 0);
        ir_taic = 0;

        // check timer_x's flag to see if should adjust the tick count
        if(ir_tbic)
        {
            TickCount++;
            ir_tbic = 0;
        }
    }
} // tx_byte (for R8C1_PROCESSOR) //

#endif // #ifdef polled //

/*!
    \brief ISR for bit timer to transmit data.

    \param void

    \return void
*/

#pragma interrupt tx_bit_isr
void tx_bit_isr(void)
{
    if(bit_mask == 0)
    {
        // reset to first bit of next byte
        bit_mask = 0x80;    
        tx_rf_idx++;
    } // if done with current byte //
    RF_DATA = ((TX_RF_DATA[tx_rf_idx] & bit_mask) && 1);

    bit_mask >>= 1;
} // clkout_timer_ra_isr //


/*!
    \brief ISR for the data clock

    The ISR that should be called once every bit time during rf receive.  The
    ADI provides this data clock signal when the transceiver is in receive mode.

    \param void
    \return  void
*/
#pragma interrupt dataclk_isr
void dataclk_isr(void)
{
    if(RF_DATA)
    {
        rx_rf_data[rx_rf_count] |= bit_mask;
    } // if a 1 was receeived //
    else
    {
        rx_rf_data[rx_rf_count] &= ~bit_mask;
    } // else a 0 was received //

    bit_mask >>= 1;
    if(bit_mask == 0)
    {
        bit_mask = 0x80;
        if(++rx_rf_count >= ONE_NET_MAX_ENCODED_PKT_LEN)
        {
            rx_rf_count = 0;
        } // if the end of the receive buffer has been passed //
    } // if done receiving a byte //
} // dataclk_isr //

//! @} HAL_ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ADI
