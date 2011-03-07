//! \defgroup SEMTECH Semtech XE1205 driver.
//! \ingroup TAL
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
    \file semtech_xe1205.c
    \brief Driver for the Semtech XE1205.

    This file implements the communications necessary for communicating with the
    Semtech XE1205.  It abstracts the actual processor pin assignments using
    definitions found in io_port_mapping.h.  This file also contains the
    implementation for the transceiver specific functions (such as
    look_for_packet, one_net_read, one_net_write) declared in
    one_net_port_specific.h
*/

#include "hal_semtech.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "spi.h"
#include "tal.h"

//=============================================================================
//                                  CONSTANTS
//! \defgroup SEMTECH_const
//! \ingroup SEMTECH
//! @{

#define SYNCDET IRQ0


enum
{
    //! Number of bytes to write to a register
    REG_SIZE = 2,
    
    //! Number of registers to initialize when starting up
    NUM_INIT_REGS = 14,
    
    //! Max number of registers that can be read at a time
    MAX_READ_REG_COUNT = 2,

    //! The base data rate in the list (see data_rate_t in one_net.h)
    BASE_DATA_RATE = ONE_NET_DATA_RATE_38_4
};

enum
{
    //! The BASE_DATA_RATE setting for the Semtech.
    BASE_DATA_RATE_SETTING = 0x03,
};

//! MCPARAM 0 register settings
enum
{
    // transceiver modes
    CHIP_MODE_SLEEP = 0x00,         //! sleep mode
    CHIP_MODE_RX = 0x40,            //! receive mode
    CHIP_MODE_TX = 0x80,            //! transmit mode
    CHIP_MODE_IDLE = 0xC0,          //! stand-by mode
    CHIP_MODE_MASK = 0xC0,          //! mask the chip mode bits
    
    // select modes (how the different transceiver modes are selected)
    SELECT_MODE_CHIP = 0x00,        //! Transceiver mode defined by CHIP_MODE
    SELECT_MODE_SW = 0x20,          //! Transceiver mode defined by SW(1:0) pins
    
    // buffer mode
    BUFFER_MODE_CONTINUOUS = 0x00,  //! Continuous mode
    BUFFER_MODE_BUFFER = 0x10,      //! Buffered mode
    
    // data direction
    DATA_BI_DIR = 0x00,             //! Data line is bidirectional
    DATA_UNI_DIR = 0x08,            //! Data line is unidirectional
    
    // frequency band
    FREQ_BAND_433 = 0x02,           //! 433 Mhz frequency band
    FREQ_BAND_868 = 0x04,           //! 868 Mhz frequency band
    FREQ_BAND_915 = 0x06            //! 915 Mhz frequency band
};

//! The MCPARAM4 & MCPARAM5 (thus 2 *) register settings for a given base
//! frequency.
static const UInt8 CHANNEL_SETTING[ONE_NET_NUM_CHANNELS][REG_SIZE << 1] =
{
    {0x07, 0xA2, 0x09, 0x40},       // channel = 1, frequency = 903.0 Mhz
    {0x07, 0xAA, 0x09, 0x10},       // channel = 2, frequency = 904.0 Mhz
    {0x07, 0xB1, 0x09, 0xE0},       // channel = 3, frequency = 905.0 Mhz
    {0x07, 0xB9, 0x09, 0xB0},       // channel = 4, frequency = 906.0 Mhz
    {0x07, 0xC1, 0x09, 0x80},       // channel = 5, frequency = 907.0 Mhz
    {0x07, 0xC9, 0x09, 0x50},       // channel = 6, frequency = 908.0 Mhz
    {0x07, 0xD1, 0x09, 0x20},       // channel = 7, frequency = 909.0 Mhz
    {0x07, 0xD8, 0x09, 0xF0},       // channel = 8, frequency = 910.0 Mhz
    {0x07, 0xE0, 0x09, 0xC0},       // channel = 9, frequency = 911.0 Mhz
    {0x07, 0xE8, 0x09, 0x90},       // channel = 10, frequency = 912.0 Mhz
    {0x07, 0xF0, 0x09, 0x60},       // channel = 11, frequency = 913.0 Mhz
    {0x07, 0xF8, 0x09, 0x30},       // channel = 12, frequency = 914.0 Mhz
    {0x07, 0x00, 0x09, 0x00},       // channel = 13, frequency = 915.0 Mhz
    {0x07, 0x07, 0x09, 0xD0},       // channel = 14, frequency = 916.0 Mhz
    {0x07, 0x0F, 0x09, 0xA0},       // channel = 15, frequency = 917.0 Mhz
    {0x07, 0x17, 0x09, 0x70},       // channel = 16, frequency = 918.0 Mhz
    {0x07, 0x1F, 0x09, 0x40},       // channel = 17, frequency = 919.0 Mhz
    {0x07, 0x27, 0x09, 0x10},       // channel = 18, frequency = 920.0 Mhz
    {0x07, 0x2E, 0x09, 0xE0},       // channel = 19, frequency = 921.0 Mhz
    {0x07, 0x36, 0x09, 0xB0},       // channel = 20, frequency = 922.0 Mhz
    {0x07, 0x3E, 0x09, 0x80},       // channel = 11, frequency = 923.0 Mhz
    {0x07, 0x46, 0x09, 0x50},       // channel = 22, frequency = 924.0 Mhz
    {0x07, 0x4E, 0x09, 0x20},       // channel = 23, frequency = 925.0 Mhz
    {0x07, 0x55, 0x09, 0xF0},       // channel = 24, frequency = 926.0 Mhz
    {0x07, 0x5D, 0x09, 0xC0}        // channel = 25, frequency = 927.0 Mhz
};

/*!
    \brief The register settings for setting the desired data rate.
    
    These values need to coincide with the data rates to be supported.  See
    data_rate_t in one_net.h for data rates.
*/
const UInt8 DATA_RATE_SETTING[ONE_NET_MAX_DATA_RATE + 1][REG_SIZE] =
{
    {0x05, BASE_DATA_RATE_SETTING}  // 38.4 kbps
};

/*!
    \brief The initial register values for the xe1205
    
    Output Frequency    =       918.500 MHz
    Deviation           = +/-   240.000 kHz
    Bit Rate            =       38.400 kBaud
*/
const UInt8 INIT_REG_VAL[NUM_INIT_REGS][REG_SIZE] =
{
    // MCParam 0
    {0x01, CHIP_MODE_RX | SELECT_MODE_CHIP | BUFFER_MODE_CONTINUOUS
      | DATA_BI_DIR | FREQ_BAND_915 | 0x01},

    // MCPARAM 1, frequency deviation
    {0x03, 0xEA},

    // MCPARAM 2, Bit rate (38.4 kbps)
    {0x05, BASE_DATA_RATE_SETTING},

    // IRQs, rx mode: irq0 - pattern, irq1 - DCLK
    //       tx mode: irq0 - low irq1 - DCLK
    {0x0B, 0xD0},

    // Address 6, disable of RSSI interrupsts
    {0x0D, 0x00},
    
    // transmit cfg, power 10dBm, modulation on, no filter, bit sync normal
    {0x0F, 0x80},
    
    // rx param addr 8, bit sync on, 200kHz bandwidth for base band filter,
    // max b/w of base band filter, cal at startup, default init at start up
    {0x11, 0x70},
    
    // rx param addr 9, rssi on, low range, input pwr < VTHR1, fei off
    {0x13, 0x80},
    
    // rx param addr 10, iq amp off, high sensitivity, pattern recognition on,
    // 32 bit pattern, 0 tolerated errors
    {0x15, 0x1C},
    
    // pattern reg, 1rst byte
    {0x1B, 0x55},
    
    // pattern reg, 2nd byte
    {0x1D, 0x55},
    
    // pattern reg 3rd byte
    {0x1F, 0x55},
    
    // pattern reg 4rth byte
    {0x21, 0x33},
    
    // osc reg, internal oscillator, enable clock out, clock out freq = 19.5 Mhz
    {0x23, 0x60}
};
 
//! @} SEMTECH_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup SEMTECH_typedefs
//! \ingroup SEMTECH
//! @{

//! @} SEMTECH_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup SEMTECH_pri_var
//! \ingroup SEMTECH
//! @{

//! index into rx_rf_data
UInt16 rf_idx = 0;

//! bytes currently in rx_rf_data
UInt16 rf_count = 0;

//! Buffer to receive data from the rf interface
UInt8 rf_data[ONE_NET_MAX_ENCODED_PKT_LEN];

//! Masks each bit in a byte in the interrupt routines when data is transfered.
UInt8 bit_mask;

//! Current value of MCParam0
static UInt8 mcparam0[REG_SIZE] = {0x01, 0x00};

//! The current ONE-NET channel
static UInt8 current_channel = 12;

//! @} SEMTECH_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup SEMTECH_pri_func
//! \ingroup SEMTECH
//! @{

static BOOL read_cfg_reg(const UInt8 * const REG, UInt8 * const data,
  const UInt8 NUM_REGS);
static BOOL write_cfg_reg(const UInt8 * const REG_DATA, const UInt8 NUM_REGS);

//! @} SEMTECH_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup SEMTECH_pub_func
//! \ingroup SEMTECH
//! @{

/*!
    \brief Initializes the Semtech transceiver.

    This function is transceiver specific. It should configure the transceiver
    for ONE-NET operation so that switching between modes (receive/transmit)
    can be accomplished with as few commands as possible.
    
    This function also makes the call to initialize the pins needed by this
    transceiver.

    \param void

    \return void
*/
void tal_init_transceiver(void)
{
    TAL_INIT_PORTS();

    ENABLE_TRANSCEIVER();

    // store the current state of MCPARAM0 which is the first register in
    // init_reg_val
    mcparam0[1] = INIT_REG_VAL[0][1];
    write_cfg_reg(INIT_REG_VAL[0], NUM_INIT_REGS);
    write_cfg_reg(CHANNEL_SETTING[current_channel], 2);

    INIT_RF_INTERRUPTS();
} // tal_init_transceiver //    


/*!
    \brief Sets the transceiver to receive mode

    \param void

    \return void
*/
void tal_turn_on_receiver(void)
{
    mcparam0[1] &= ~CHIP_MODE_MASK;
    mcparam0[1] |= CHIP_MODE_RX;

    write_cfg_reg(mcparam0, 1);
    RF_DATA_DIR = INPUT;            // set the data line to an input
} // tal_turn_on_receiver //


/*!
    \brief Sets the transceiver to transmit mode

    \param void

    \return void
*/
void tal_turn_on_transmitter(void)
{
    mcparam0[1] &= ~CHIP_MODE_MASK;
    mcparam0[1] |= CHIP_MODE_TX;

    write_cfg_reg(mcparam0, 1);
    RF_DATA_DIR = OUTPUT;           // set the data line to an output
} // tal_turn_on_transmitter //


/*!
    \brief Get the current ONE_NET channel.

    \param void

    \return The current one_net_channel (0-based)

*/
UInt8 one_net_get_channel(void)
{
    return current_channel;
} // one_net_get_channel //


/*!
    \brief Changes the channel the device is on

    \param[in] CHANNEL The channel to change to (0-based).

    \return void
*/
void one_net_set_channel(const UInt8 CHANNEL)
{
    if(CHANNEL < ONE_NET_NUM_CHANNELS)
    {
        current_channel = CHANNEL;
        write_cfg_reg(CHANNEL_SETTING[current_channel], 2);
    } // if the parameter is valid //
} // one_net_set_channel //


// returns TRUE if the channel is clear (declared in one_net_port_specific.h).
BOOL one_net_channel_is_clear(void)
{
    const UInt8 RSSI_OUT_MASK = 0x30;
    const UInt8 RSSI_THRESHOLD = 0x20;

    const UInt8 reg = 0x13;

    UInt8 rssi_out;
    
    if(read_cfg_reg(&reg, &rssi_out, 1))
    {
        return (rssi_out & RSSI_OUT_MASK) <= RSSI_THRESHOLD;
    } // if reading the register was successful //
    
    return FALSE;
} // one_net_channel_is_clear //


// sets the data rate the transceiver operates at (see one_net_port_specefic.h).
void one_net_set_data_rate(const UInt8 DATA_RATE)
{
    if(DATA_RATE <= ONE_NET_MAX_DATA_RATE)
    {
        write_cfg_reg(DATA_RATE_SETTING[DATA_RATE], 1);
    } // if the data rate is valid //
} // one_net_set_data_rate //


one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    UInt8 blks_to_rx = ONE_NET_MAX_ENCODED_PKT_LEN;

    tick_t end = one_net_tick() + DURATION;

    TAL_TURN_ON_RECEIVER();   

    rf_count = 0;
    rf_idx = 0;

    while(!SYNCDET)
    {   
        if(one_net_tick() >= end)
        {
		    return ONS_TIME_OUT;
        } // if done looking //
    } // while no sync detect //

    ENABLE_RX_BIT_INTERRUPTS();

    do
    {
        // the id length should be the location in the byte stream where the
        // PID will be received.  Look for the PID so we know how many bytes
        // to receive.
        if(rf_count == ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX
          + 1)
        {
            // All packet size constants below are including the PREAMBLE &
            // SOF.  Since these cause the sync detect, these won't be read
            // in, so the packet size that is being read in is shorter, so
            // subtract the ONE_NET_ENCODED_DST_DID_IDX since that is where the
            // read is being started.
            switch(rf_data[ONE_NET_ENCODED_PID_IDX
              - ONE_NET_ENCODED_DST_DID_IDX])
            {
                case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
                {
                    blks_to_rx = 48 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // MASTER invite new CLIENT case //

                case ONE_NET_ENCODED_SINGLE_DATA_ACK:   // fall through
                case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
                case ONE_NET_ENCODED_SINGLE_DATA_NACK:  // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_ACK:    // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_NACK:   // fall through
                case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
                {
                    blks_to_rx = 17 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // acks/nacks/keep alive case for data packets //

                case ONE_NET_ENCODED_SINGLE_TXN_ACK:    // fall through
                case ONE_NET_ENCODED_BLOCK_TXN_ACK:
                {
                    blks_to_rx = 15 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // single/block txn ack case //

                case ONE_NET_ENCODED_SINGLE_DATA:       // fall through
                case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
                {
                    blks_to_rx = 26 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // (repeat)single data case //
                
                case ONE_NET_ENCODED_BLOCK_DATA:        // fall through
                case ONE_NET_ENCODED_REPEAT_BLOCK_DATA: // fall through
                case ONE_NET_ENCODED_STREAM_DATA:
                {
                    blks_to_rx = 58 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // (repeat)block, stream data case //
                
                case ONE_NET_ENCODED_DATA_RATE_TEST:
                {
                    blks_to_rx = 20 - ONE_NET_ENCODED_DST_DID_IDX;
                    break;
                } // data rate test case //

                default:
                {
		            DISABLE_RX_BIT_INTERRUPTS();
                    return ONS_BAD_PKT_TYPE;
                    break;
                } // default case //
            } // switch on PID //
        } // if PID read //
    } while(rf_count < blks_to_rx);

    DISABLE_RX_BIT_INTERRUPTS();

    return ONS_SUCCESS;
} // one_net_look_for_pkt //


UInt16 one_net_read(UInt8 * data, const UInt16 LEN)
{
    UInt16 bytes_to_read;
    
    // check the parameters, and check to see if there is data to be read
    if(!data || !LEN || rf_idx >= rf_count)
    {
        return 0;
    } // if the parameters are invalid, or there is no more data to read //
    
    if(rf_idx + LEN > rf_count)
    {
        // more bytes have been requested than are available, so give the
        // caller what is available
        bytes_to_read = rf_count - rf_idx;
    } // if more by requested than available //
    else
    {
        bytes_to_read = LEN;
    } // else read number of bytes requested //
    
    one_net_memmove(data, &(rf_data[rf_idx]), bytes_to_read);
    rf_idx += bytes_to_read;
    
    return bytes_to_read;
} // one_net_read //


/*!
    \brief Sends bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[in] DATA An array of bytes to be sent out of the rf interface
    \param[in] LEN The number of bytes to send

    \return The number of bytes sent.
*/
UInt16 one_net_write(const UInt8 * DATA, const UInt16 LEN)
{
    rf_idx = 0;
    rf_count = LEN < sizeof(rf_data) ? LEN : sizeof(rf_data);
    one_net_memmove(rf_data, DATA, rf_count);

    TAL_TURN_ON_TRANSMITTER();
    ENABLE_TX_BIT_INTERRUPTS();

    return rf_count;
} // one_net_write //


BOOL one_net_write_done(void)
{
    if(rf_idx < rf_count)
    {
        return FALSE;
    } // if not done //

    DISABLE_TX_BIT_INTERRUPTS();
    TAL_TURN_ON_RECEIVER();

    return TRUE;
} // one_net_write_done //

//! @} SEMTECH_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup SEMTECH_pri_func
//! \ingroup SEMTECH
//! @{

/*!
    \brief Reads a list of configuration registers

    The registers to read do not have to be in any order.

    \param[in] REG The list of registers to read.
    \param[out] data The data from the registers
    \param[in] NUM_REGS The number of registers to read.
    
    \return TRUE if the read was successful
            FALSE if the read failed.
*/
static BOOL read_cfg_reg(const UInt8 * const REG, UInt8 * const data,
  const UInt8 NUM_REGS)
{
    // Read the data into a seperate buffer since it needs to have 1 more
    // location to account for reading the garbage byte when the first register
    // is addressed.
    UInt8 reg_data[MAX_READ_REG_COUNT + 1];
    
    // Need to add the read bit to the registers.
    UInt8 reg_to_read[MAX_READ_REG_COUNT];
    
    BOOL rv;

    UInt8 i;
    
    if(!REG || !data || !NUM_REGS || NUM_REGS > MAX_READ_REG_COUNT)
    {
        return FALSE;
    } // if the parameters are invalid //

    for(i = 0; i < NUM_REGS; i++)
    {
        reg_to_read[i] = REG[i] | 0x40;
    } // loop to add read bit to registers //

    NSS_CFG = 0;
    rv = spi_transfer(reg_to_read, NUM_REGS, reg_data, NUM_REGS + 1);
    NSS_CFG = 1;

    if(rv)
    {
        // return the data (minus the garbage byte)
        one_net_memmove(data, &(reg_data[1]), NUM_REGS);
    } // if the transfer was successful //
    
    return rv;
} // read_cfg_reg //


/*!
    \brief Write to the configuration register.
    
    REG_DATA contains a register, register data pair.
    
    \param[in] REG_DATA The registers & data to set.
    \param[in] NUM_REGS The number of registers to write
    
    \return TRUE if the write was successful
            FALSE if the data was not successfully written
*/
static BOOL write_cfg_reg(const UInt8 * const REG_DATA, const UInt8 NUM_REGS)
{
    BOOL rv;
    
    if(!REG_DATA || !NUM_REGS)
    {
        return FALSE;
    } // if the parameters are invalid //
    
    NSS_CFG = 0;
    // twice as many bytes to send as registers being written
    rv = spi_transfer(REG_DATA, NUM_REGS << 1, 0, 0);
    NSS_CFG = 1;
    
    return rv;
} // write_cfg_reg //

//! @} SEMTECH_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} SEMTECH
