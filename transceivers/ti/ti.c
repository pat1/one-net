//! \defgroup TI TI CC1100 driver.
//! \ingroup TAL
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
    \file ti.c
    \brief Driver for the TI CC1100.

    This file implements the communications necessary for communicating with the
    TI CC1100.  It abstracts the actual processor pin assignment using
    definitions found in io_port_mapping.h.  This file also contains the
    implementation for the transceiver specific functions (such as
    look_for_pkt, one_net_read, one_net_write) declared in
    one_net_port_specific.h.

    In this implementation, the receive state is not shut off when the packet
    is read in, which will result in a receive FIFO overflow.  We are not
    concerned since the next time the application wants to read, the FIFO is
    flushed to ensure a clean read is done.
*/

#include "config_options.h"

#include "hal_ti.h"
#include "io_port_mapping.h"
#include "one_net.h"
#include "one_net_port_const.h"
#include "one_net_port_specific.h"
#include "spi.h"
#include "tal.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup TI_const
//! \ingroup TI
//! @{

enum
{
    //! Number of registers to initialize.  Note that not all of them will be
    //! initialized as there are a few placeholders.
    NUM_INIT_REGS = 0x2F,

    //! Number of registers to write when setting the channel
    NUM_CHANNEL_REG = 3,

    //! Number of registers to write when setting the data rate
    NUM_DATA_RATE_REG = 1,

    //! The base data rate (see one_net_data_rate_t in one_net.h)
    BASE_DATA_RATE = ONE_NET_DATA_RATE_38_4
};

//! CC1100 internal states (as stored in the status register)
enum
{
    CC1100_IDLE = 0x00,             //!< IDLE state
    CC1100_RX = 0x10,               //!< Receive mode
    CC1100_TX = 0x20,               //!< Transmit mode
    CC1100_FSTXON = 0x30,           //!< Fast TX ready
    CC1100_CALIBRATE = 0x40,        //!< Freq. synthesizer cal is running
    CC1100_SETTLING = 0x50,         //!< PLL is settling
    CC1100_RX_FIFO_OVERFLOW = 0x60, //!< RX FIFO has overflow
    CC1100_TX_FIFO_UNDERFLOW = 0x70,//!< TX FIFO has underflowed.

    CC1100_STATE_MASK = 0x70        //!< Masks state info from status byte
};

//! Header bit definitions.  A header consists of a R/W bit, Burst Bit, and
//! 6 address bits
enum
{
    READ_BIT = 0x80,                //!< Indicates reading(1) or writing(0)
    BURST_BIT = 0x40                //!< Indicates burst access
};

enum
{
    FIFO_SIZE = 64                  //!< The size of the FIFOs
};

//! Registers
enum
{
    IOCFG2 = 0x00,                  //!< GDO2 output pin configuration

    IOCFG1 = 0x01,                  //!< GDO1 output pin configuration

    FREQ2 = 0x0D,                   //!< Frequency control word, high byte

    MDMCFG3 = 0x11,                 //!< Modem configuration (data rate)

    MDMCFG2 = 0x12,                 //!< Modem configuration

    // status registers
    RXBYTES = 0x3B,                 //!< Overflow & number of bytes in RX FIFO

    // data registers
    FIFO_REG = 0x3F                 //!< RX & TX FIFO
};

//! Command strobes
enum
{
    //! Resets the chip
    SRES = 0x30,

    //! Enable & cal frequency synthesizer
    SFSTXON = 0x31,

    //! Turn off crystal oscillator
    SXOFF = 0x32,

    //! Cal frequency synthesizer and turn it off
    SCAL = 0x33,

    //! goto RX state
    SRX = 0x34,

    //! goto TX state (unless RX state, CCA enabled, and channel is not clear)
    STX = 0x35,

    //! Exit RX/TX, turn off frequency synthesizer, and exit Wake-On-Radio
    SIDLE = 0x36,

    //! Start auto RX polling sequence (Wake-On-Radio)
    SWOR = 0x38,

    //! Enter power down when SSNOT goes high
    SPWD = 0x39,

    //! Flush RX FIFO
    SFRX = 0x3A,

    //! flush TX FIFO
    SFTX = 0x3B,

    //! reset real time clock to Event1 value
    SWORRST = 0x3C,

    //! NOP (may be used to get chip status byte)
    SNOP = 0x3D
};

//! GDOx configuration values
enum
{
    //! Asserted when RX FIFO >= RX FIFO THRESHOLD.  De-asserted when
    //! RX FIFO < RX FIFO THRESHOLD
    GDO_CFG_RX_FIFO_THRESHOLD = 0x00,

    //! ASSERTED when RX FIFO >= RX FIFO THRESHOLD || end of packet.
    //! De-asserted when RX FIFO empty
    GDO_CFG_RX_FIFO_THRESHOLD_OR_PKT = 0x01,

    //! Asserted when TX FIFO >= TX FIFO THRESHOLD.  De-asserted when
    //! TX FIFO < TX FIFO THRESHOLD
    GDO_CFG_TX_FIFO_THRESHOLD = 0x02,

    //! Asserted when TX FIFO is full.  De-asserted when
    //! TX FIFO < TX FIFO THRESHOLD
    GDO_CFG_TX_FIFO_FULL = 0x03,

    //! Asserted when RX FIFO has overflowed.  De-asserted when the FIFO has
    //! been flushed
    GDO_CFG_RX_OVERFLOW = 0x04,

    //! Asserted when TX FIFO underflowed.  De-asserted when the FIFO has
    //! been flushed
    GDO_CFG_TX_UNDERFLOW = 0x05,

    //! Asserted when the sync word has been sent/received.  De-asserted at the
    //! end of the packet
    GDO_CFG_SYNC = 0x06,

    //! Asserted when a packet with a valid CRC has been received.  De-asserted
    //! when the first byte is read from the RX FIFO
    GDO_CFG_RX_VALID_CRC = 0x07,

    //! Preamble Quality Reached.  Asserted when the PQI is above the programmed
    //! PQT value
    GDO_CFG_PQI = 0x08,

    //! Clear Channel Assessment.  High when RSSI level is below threshold
    GDO_CFG_CCA = 0x09,

    //! Lock detector output
    GDO_CFG_LOCK_DETECT = 0x0A,

    //! Serial clock.  Synchronous to data in serial mode.  In RX mode, mcu
    //! should read on rising edge.  In TX mode, mcu should write on falling
    //! edge.
    GDO_CFG_DATACLK = 0x0B,

    //! Serial Synchronous data output
    GDO_CFG_SYNC_SERIAL_DATA_OUT = 0x0C,

    //! Serial data output
    GDO_CFG_SRIAL_DATA_OUT = 0x0D,

    //! Carrier Sense.  High if RSSI > threshold
    GDO_CFG_CS = 0x0E,

    //! Last CRC comparison matched.  Cleared when entering/restarting RX mode.
    GDO_CFG_CRC_OK = 0x0F,

    //! RX_HARD_DATA[1].  Can be used together with RX_SYMBOL_TICK for
    //! alternative serial RX output
    GDO_CFG_RX_HARD_DATA_1 = 0x16,

    //! RX_HARD_DATA[0].  Can be used together with RX_SYMBOL_TICK for
    //! alternative serial RX output
    GDO_CFG_RX_HARD_DATA_0 = 0x17,

    //! See data sheet for notes
    GDO_CFG_PA_PD = 0x1B,

    //! See data sheet for notes
    GDO_CFG_LNA_PD = 0x1C,

    //! Can be used with RX_HARD_DATA for alternative serial RX output
    GDO_CFG_RX_SYMBOL_TICK = 0x1D,

    //! Wake on Reset event 0
    GDO_CFG_WOR0 = 0x24,

    //! Wake on Reset event 1
    GDO_CFG_WOR1 = 0x25,

    //! 32kHz clock
    GDO_CFG_CLK_32K = 0x27,

    //! Chip ready (not)
    GDO_CFG_CHIP_READY_NOT = 0x29,

    //! XOSC stable
    GDO_CFG_XOSC_STABLE = 0x2B,

    //! When output is 0, GDO0 is configured as input (for serial TX data)
    GDO_CFG_GDO0_Z_EN_N = 0x2D,

    //! High impedance (3-state)
    GDO_CFG_UNUSED = 0x2E,

    //! See data sheet
    GDO_CFG_HW_TO_0 = 0x2F,

    //! CLOCK OSCILLATOR / 1
    GDO_CFG_CLK_XOSC_1 = 0x30,

    //! CLOCK OSCILLATOR / 1.5
    GDO_CFG_CLK_XOSC_1_5 = 0x31,

    //! CLOCK OSCILLATOR / 2
    GDO_CFG_CLK_XOSC_2 = 0x32,

    //! CLOCK OSCILLATOR / 3
    GDO_CFG_CLK_XOSC_3 = 0x33,

    //! CLOCK OSCILLATOR / 4
    GDO_CFG_CLK_XOSC_4 = 0x34,

    //! CLOCK OSCILLATOR / 6
    GDO_CFG_CLK_XOSC_6 = 0x35,

    //! CLOCK OSCILLATOR / 8
    GDO_CFG_CLK_XOSC_8 = 0x36,

    //! CLOCK OSCILLATOR / 12
    GDO_CFG_CLK_XOSC_12 = 0x37,

    //! CLOCK OSCILLATOR / 16
    GDO_CFG_CLK_XOSC_16 = 0x38,

    //! CLOCK OSCILLATOR / 24
    GDO_CFG_CLK_XOSC_24 = 0x39,

    //! CLOCK OSCILLATOR / 32
    GDO_CFG_CLK_XOSC_32 = 0x3A,

    //! CLOCK OSCILLATOR / 48
    GDO_CFG_CLK_XOSC_48 = 0x3B,

    //! CLOCK OSCILLATOR / 64
    GDO_CFG_CLK_XOSC_64 = 0x3C,

    //! CLOCK OSCILLATOR / 96
    GDO_CFG_CLK_XOSC_96 = 0x3D,

    //! CLOCK OSCILLATOR / 128
    GDO_CFG_CLK_XOSC_128 = 0x3E,

    //! CLOCK OSCILLATOR / 192
    GDO_CFG_CLK_XOSC_192 = 0x3F
};

//! Register settings for the GDO (0x00-0x02) registers
enum
{
    //! For use in all IOCFG registers (0x00-0x02).  Inverts output of GDO pin
    IOCFG_INVERT_PIN_OUTPUT = 0x40,


    //! For use in IOCFG1 register (0x01).  Sets high drive strength of GDO pin
    IOCFG1_HIGH_DRIVE_STRENGTH = 0x80,

    //! For use in IOCFG1 register (0x01).  Sets low drive strength of GDO pin
    IOCFG1_LOW_DRIVE_STRENGTH = 0x00,


    //! For use in IOCFG0 register (0x02).  Enables the temperature sensor.
    IOCFG0_TEMPERATURE_ENABLE = 0x80,
};

/*!
    \brief FIFOTHR (0x03 RX FIFO and TX FIFO Thresholds) register settings

    The FIFO thresholds are exceeded when the number of bytes in the FIFO
    is >= the threshold value.
*/
enum
{
    //! TX FIFO threshold = 61 bytes.  RX FIFO threshold = 4 bytes
    FIFOTHR_TX_61_RX_4 = 0x00,

    //! TX FIFO threshold = 57 bytes.  RX FIFO threshold = 8 bytes
    FIFOTHR_TX_57_RX_8 = 0x01,

    //! TX FIFO threshold = 53 bytes.  RX FIFO threshold = 12 bytes
    FIFOTHR_TX_53_RX_12 = 0x02,

    //! TX FIFO threshold = 49 bytes.  RX FIFO threshold = 16 bytes
    FIFOTHR_TX_49_RX_16 = 0x03,

    //! TX FIFO threshold = 45 bytes.  RX FIFO threshold = 20 bytes
    FIFOTHR_TX_45_RX_20 = 0x04,

    //! TX FIFO threshold = 41 bytes.  RX FIFO threshold = 24 bytes
    FIFOTHR_TX_41_RX_24 = 0x05,

    //! TX FIFO threshold = 37 bytes.  RX FIFO threshold = 28 bytes
    FIFOTHR_TX_37_RX_28 = 0x06,

    //! TX FIFO threshold = 33 bytes.  RX FIFO threshold = 32 bytes
    FIFOTHR_TX_33_RX_32 = 0x07,

    //! TX FIFO threshold = 29 bytes.  RX FIFO threshold = 36 bytes
    FIFOTHR_TX_29_RX_36 = 0x08,

    //! TX FIFO threshold = 25 bytes.  RX FIFO threshold = 40 bytes
    FIFOTHR_TX_25_RX_40 = 0x09,

    //! TX FIFO threshold = 21 bytes.  RX FIFO threshold = 44 bytes
    FIFOTHR_TX_21_RX_44 = 0x0A,

    //! TX FIFO threshold = 17 bytes.  RX FIFO threshold = 48 bytes
    FIFOTHR_TX_17_RX_48 = 0x0B,

    //! TX FIFO threshold = 13 bytes.  RX FIFO threshold = 52 bytes
    FIFOTHR_TX_13_RX_52 = 0x0C,

    //! TX FIFO threshold = 9 bytes.  RX FIFO threshold = 56 bytes
    FIFOTHR_TX_9_RX_56 = 0x0D,

    //! TX FIFO threshold = 5 bytes.  RX FIFO threshold = 60 bytes
    FIFOTHR_TX_5_RX_60 = 0x0E,

    //! TX FIFO threshold = 1 bytes.  RX FIFO threshold = 56 bytes
    FIFOTHR_TX_1_RX_64 = 0x0F
};

//! PKTCTRL1 (0x07 Packet Automation Control) Register settings
enum
{
    //! Preamble quality estimator threshold of 7
    PKTCTRL1_PQT_7 = 0xE0,

    //! Preamble quality estimator threshold of 6
    PKTCTRL1_PQT_6 = 0xC0,

    //! Preamble quality estimator threshold of 5
    PKTCTRL1_PQT_5 = 0xA0,

    //! Preamble quality estimator threshold of 4
    PKTCTRL1_PQT_4 = 0x80,

    //! Preamble quality estimator threshold of 3
    PKTCTRL1_PQT_3 = 0x60,

    //! Preamble quality estimator threshold of 2
    PKTCTRL1_PQT_2 = 0x40,

    //! Preamble quality estimator threshold of 1
    PKTCTRL1_PQT_1 = 0x20,

    //! Preamble quality estimator threshold of 0
    PKTCTRL1_PQT_0 = 0x00,


    //! Enable automatic flush of RX FIFO when the crc is not OK.
    PKTCTRL1_CRC_AUTO_FLUSH_ENABLE = 0x08,


    //! Enable appending of 2 status bytes to payload of packet
    PKTCTRL1_APPEND_STATUS_ENABLE = 0x04,


    //! Does not perform address checking
    PKTCTRL1_NO_ADDRESS_CHECK = 0x00,

    //! Checks address, no broadcast
    PKTCTRL1_ADDRESS_BROADCAST0 = 0x01,

    //! Checks address and 0x00 broadcast
    PKTCTRL1_ADDRESS_BROADCAST1 = 0x02,

    //! Checks address and 0x00 & 0xFF broadcast
    PKTCTRL1_ADDRESS_BROADCAST2 = 0x03
};

//! PKTCTRL0 (0x08 Packet Automation Control) Register settings
enum
{
    //! Enables data whitening
    PKTCTRL0_WHITENING_ENABLE = 0x40,


    //! rf data sent/received using FIFOs
    PKTCTRL0_RF_DATA_IO_FIFO = 0x00,

    //! rf data sent/received using synchronous serial mode (data in on GDO0)
    PKTCTRL0_RF_DATA_IO_SYNC = 0x10,

    //! rf data sent randomly using PN9 generator.
    PKTCTRL0_RF_DATA_IO_RANDOM = 0x20,

    //! rf data sent/received using asynchronous serial mode (data in on GDO0,
    //! data out on one of the other GDO pins).
    PKTCTRL0_RF_DATA_IO_ASYNC = 0x30,


    //! Enables crc checking
    PKTCTRL0_CRC_ENABLE = 0x04,


    //! fixed length packets
    PKTCTRL0_PKT_LEN_FIXED = 0x00,

    //! variable length packet configured by first byte after sync word
    PKTCTRL0_PKT_LEN_VARIABLE = 0x01,

    //! infinite packet length
    PKTCTRL0_PKT_LEN_INFINITE = 0x02
};

//! MDMCFG2 (0x12 Modem Configuration) Register Settings
enum
{
    //! Disables DC blocking filter before demodulator
    MDMCFG2_DEM_DCFILT_OFF = 0x80,


    //! 2-FSK modulation format of radio signal
    MDMCFG2_RADIO_2FSK = 0x00,

    //! GFSK modulation format of radio signal
    MDMCFG2_RADIO_GFSK = 0x10,

    //! ASK/OOK modulation format of radio signal
    MDMCFG2_RADIO_ASK_OOK = 0x30,

    //! MSK modulation format of radio signal
    MDMCFG2_RADIO_MSK = 0x70,


    //! Enables Manchester encoding
    MDMCFG2_MANCHESTER_ENABLE = 0x08,


    //! No Preamble/sync word
    MDMCFG2_NO_PREAMBLE_SYNC = 0x00,

    //! 15 out of 16 sync word bits detected
    MDMCFG2_15_OF_16_SYNC = 0x01,

    //! 16 out of 16 sync word bits detected
    MDMCFG2_16_OF_16_SYNC = 0x02,

    //! 30 out of 32 synce word bits detected
    MDMCFG2_30_OF_32_SYNC = 0x03,

    //! Carrier Sense above threshold detected (no preamble/sync word)
    MDMCFG2_CS = 0x04,

    //! 15 out of 16 sync word bits and carrier sense above threshold
    MDMCFG2_15_OF_16_SYNC_CS = 0x05,

    //! 16 out of 16 sync word bits and carrier sense above threshold
    MDMCFG2_16_OF_16_SYNC_CS = 0x06,

    //! 30 out of 32 sync word bits and carrier sense above threshold
    MDMCFG2_30_OF_32_SYNC_CS = 0x07
};

//! MDMCFG1 (0x13 Modem Configuration) Register Settings
enum
{
    //! Enables Forward Error Correction
    MSMCFG1_FEC_ENABLE = 0x80,


    //! 2 bit preamble
    MSMCFG1_NUM_PREAMBLE_2 = 0x00,

    //! 3 bit preamble
    MSMCFG1_NUM_PREAMBLE_3 = 0x01,

    //! 4 bit preamble
    MSMCFG1_NUM_PREAMBLE_4 = 0x02,

    //! 6 bit preamble
    MSMCFG1_NUM_PREAMBLE_6 = 0x03,

    //! 8 bit preamble
    MSMCFG1_NUM_PREAMBLE_8 = 0x04,

    //! 12 bit preamble
    MSMCFG1_NUM_PREAMBLE_12 = 0x05,

    //! 16 bit preamble
    MSMCFG1_NUM_PREAMBLE_16 = 0x06,

    //! 24 bit preamble
    MSMCFG1_NUM_PREAMBLE_24 = 0x07
};

//! MCSM2 (0x16 Main Radio Control State Machine Configuration) Register
//! Settings
enum
{
    //! Direct RX termination based on carrier sense
    MCSM2_RX_TIME_RSSI_ENABLE = 0x10,

    //! Enable checking if PQI is set when RX_TIME timer expires and sync word
    //! was not found
    MCSM2_RX_TIME_QUAL_ENABLE = 0x08,

    //! sync word search timeout does not occur until the end of packet
    MCSM2_SYNC_TIME_OUT_END_OF_PKT = 0x07
};

//! MCSM1 (0x17 Main Radio Control State Machine Configuration) Register
//! Settings
enum
{
    //! always indicate channel is clear
    MCSM1_CCA_ALWAYS = 0x00,

    //! indicate channel is clear when RSSI < threshold
    MCSM1_CCA_RSSI = 0x10,

    //! indicate channel is clear unless currently receiving a packet
    MCSM1_CCA_RX_PKT = 0x20,

    //! indicate channel is clear if RSSI < threshold unless currently
    //! receiving a packet
    MCSM1_CCA_RSSI_RX_PKT = 0x30,


    //! Go to idle mode when a packet has been received
    MCSM1_RX_PKT_IDLE = 0x00,

    //! Go to fast transmit on mode when a packet has been received
    MCSM1_RX_PKT_FSTXON = 0x04,

    //! Go to transmit mode when a packet has been received
    MCSM1_RX_PKT_TX = 0x08,

    //! stay in rx mode when a packet has been received
    MCSM1_RX_PKT_RX = 0x0C,


    //! Go to idle mode when a packet has been sent
    MCSM1_TX_PKT_IDLE = 0x00,

    //! Go to fast transmit on mode when a packet has been sent
    MCSM1_TX_PKT_FSTXON = 0x01,

    //! stay in transmit mode when a packet has been sent
    MCSM1_TX_PKT_TX = 0x02,

    //! go to rx mode when a packet has been sent
    MCSM1_TX_PKT_RX = 0x03
};

//! MCSM0 (0x18 Main Radio Control State Machine Configuration) Register
//! Settings
enum
{
    //! Never automatically calibrate
    MCSM0_AUTOCAL_NEVER = 0x00,

    //! Automatically calibrate when going from IDLE to RX or TX
    MCSM0_AUTOCAL_FROM_IDLE = 0x10,

    //! Automatically calibrate when going to IDLE from RX or TX
    MCSM0_AUTOCAL_TO_IDLE = 0x20,

    //! Automatically calibrate every 4rth time when going to idle from RX or TX
    MCSM0_AUTOCAL_4RTH_TIME_TO_IDLE = 0x30,


    //! XOSC stabalize expire count of 1
    MCSM0_PO_TIMEOUT_1 = 0x00,

    //! XOSC stabalize expire count of 16
    MCSM0_PO_TIMEOUT_16 = 0x04,

    //! XOSC stabalize expire count of 64
    MCSM0_PO_TIMEOUT_64 = 0x08,

    //! XOSC stabalize expire count of 256
    MSCM0_PO_TIMEOUT_256 = 0x0C,


    //! Enables pin radio control
    MSCM0_PIN_RADIO_CTL_ENABLE = 0x02,


    //! Forces the XOSC to stay on in sleep mode
    MSCM0_XOSC_FORCE_ON = 0x01
};

//! FOCCFG (0x19 Frequency Offset Compensation Configuration)
enum
{
    //! freeze the frequency offset compensation and clock recovery
    //! feedback loops until the CS signal goes high
    FOCCFG_BS_CS_GATE_ENABLE = 0x20,


    //! The frequency compensation loop gain to be used before a sync word
    //! is detected - 1k
    FOCCFG_PRE_SYNC_1K = 0x00,

    //! The frequency compensation loop gain to be used before a sync word
    //! is detected - 2k
    FOCCFG_PRE_SYNC_2K = 0x08,

    //! The frequency compensation loop gain to be used before a sync word
    //! is detected - 3k
    FOCCFG_PRE_SYNC_3K = 0x10,

    //! The frequency compensation loop gain to be used before a sync word
    //! is detected - 4k
    FOCCFG_PRE_SYNC_4K = 0x18,


    //! The frequency compensation loop gain to be used after a sync word is
    //! the same as FOC_PRE_K
    FOCCFG_POST_SYNC_K_SAME = 0x00,

    //! The frequency compensation loop gain to be used after a sync word is
    //! K / 2
    FOCCFG_POST_SYNC_K_DIV_2 = 0x04,


    //! no frequency offset limit
    FOCCFG_NO_COMPENSATION = 0x00,

    //! +/-BWchan / 8 max frequency compensation offset
    FOCCFG_BW_DIV_8_COMPENSATION = 0x01,

    //! +/-BWchan / 4 max frequency compensation offset
    FOCCFG_BW_DIV_4_COMPENSATION = 0x02,

    //! +/-BWchan / 2 max frequency compensation offset
    FOCCFG_BW_DIV_2_COMPENSATION = 0x03
};

//! BSCFG (0x1A Bit Synchronization Configuration) Register Settings
enum
{
    //! 1Ki clock recovery feedback loop integral gain to be used before sync
    //! word is detected (to correct the data rates)
    BSCFG_PRE_1KI = 0x00,

    //! 2Ki clock recovery feedback loop integral gain to be used before sync
    //! word is detected (to correct the data rates)
    BSCFG_PRE_2KI = 0x40,

    //! 3Ki clock recovery feedback loop integral gain to be used before sync
    //! word is detected (to correct the data rates)
    BSCFG_PRE_3KI = 0x80,

    //! 4Ki clock recovery feedback loop integral gain to be used before sync
    //! word is detected (to correct the data rates)
    BSCFG_PRE_4KI = 0xC0,


    //! 1Kp clock recovery feedback loop proportional gain to be used before
    //! sync word is detected
    BSCFG_PRE_1KP = 0x00,

    //! 2Kp clock recovery feedback loop proportional gain to be used before
    //! sync word is detected
    BSCFG_PRE_2KP = 0x10,

    //! 3Kp clock recovery feedback loop proportional gain to be used before
    //! sync word is detected
    BSCFG_PRE_3KP = 0x20,

    //! 4Kp clock recovery feedback loop proportional gain to be used before
    //! sync word is detected
    BSCFG_PRE_4KP = 0x30,


    //! The clock recovery feedback loop integral gain to be used after a sync
    //! word is detected is the same as BS_PRE_KI
    BSCFG_POST_KI_SAME = 0x00,

    //! The clock recovery feedback loop integral gain to be used after a sync
    //! word is detected is Ki / 2
    BSCFG_POST_KI_DIV_2 = 0x08,


    //! The clock recovery feedback loop proportional gain to be used after a
    //! sync word is detected is the same as BS_PRE_KP
    BSCFG_POST_KP_SAME = 0x00,

    //! The clock recovery feedback loop proportional gain to be used after a
    //! sync word is detected is Kp / 2
    BSCFG_POST_KP_DIV_2 = 0x04,


    //! +/-0% data rate offset
    BSCFG_DATA_RATE_OFFSET_0 = 0x00,

    //! +/-3.123% data rate offset
    BSCFG_DATA_RATE_OFFSET_3_125 = 0x01,

    //! +/-6.25% data rate offset
    BSCFG_DATA_RATE_OFFSET_6_25 = 0x02,

    //! +/-12.5% data rate offset
    BSCFG_DATA_RATE_OFFSET_12_5 = 0x03,
};

//! AGCCTRL2 (0x1B AGC Control) Register Settings
enum
{
    //! All DVGA gain settings can be used
    AGCCTRL2_DVGA_ALL = 0x00,

    //! The highest DVGA gain setting cannot be used
    AGCCTRL2_DVGA_NOT_HIGHEST = 0x40,

    //! The 2 highest DVGA gain settings cannot be used
    AGCCTRL2_DVGA_NOT_2_HIGHEST = 0x80,

    //! The 3 highest DVGA gain settings cannot be used
    AGCCTRL2_DVGA_NOT_3_HIGHEST = 0xC0,


    //! Maximum allowable LNA + LNA 2 gain
    AGCCTRL2_LNA_MAX = 0x00,

    //! LNA + LNA 2 gain approx. 2.6dB below max possible gain
    AGCCTRL2_LNA_2_6 = 0x08,

    //! LNA + LNA 2 gain approx. 6.1dB below max possible gain
    AGCCTRL2_LNA_6_1 = 0x10,

    //! LNA + LNA 2 gain approx. 7.4dB below max possible gain
    AGCCTRL2_LNA_7_4 = 0x18,

    //! LNA + LNA 2 gain approx. 9.2dB below max possible gain
    AGCCTRL2_LNA_9_2 = 0x20,

    //! LNA + LNA 2 gain approx. 11.5dB below max possible gain
    AGCCTRL2_LNA_11_5 = 0x28,

    //! LNA + LNA 2 gain approx. 14.6dB below max possible gain
    AGCCTRL2_LNA_14_6 = 0x30,

    //! LNA + LNA 2 gain approx. 17.1dB below max possible gain
    AGCCTRL2_LNA_17_1 = 0x38,


    //! target amplitude from channel filter is 24dB
    AGCCTRL2_TARGET_24 = 0x00,

    //! target amplitude from channel filter is 27dB
    AGCCTRL2_TARGET_27 = 0x01,

    //! target amplitude from channel filter is 30dB
    AGCCTRL2_TARGET_30 = 0x02,

    //! target amplitude from channel filter is 33dB
    AGCCTRL2_TARGET_33 = 0x03,

    //! target amplitude from channel filter is 36dB
    AGCCTRL2_TARGET_36 = 0x04,

    //! target amplitude from channel filter is 38dB
    AGCCTRL2_TARGET_38 = 0x05,

    //! target amplitude from channel filter is 40dB
    AGCCTRL2_TARGET_40 = 0x06,

    //! target amplitude from channel filter is 42dB
    AGCCTRL2_TARGET_42 = 0x07
};

//! AGCCTRL1 (0x1C AGC Control) Register Settings
enum
{
    //! LNA decreased before LNA2
    AGCCTRL1_LNA_BEFORE_LNA2 = 0x40,

    //! LNA2 decreased before LNA
    AGCCTRL1_LNA2_BEFORE_LNA = 0x00,


    //! Disable relative carrier sense threshold
    AGCCTRL1_CS_REL_DISABLE = 0x00,

    //! Carrier sense relative threshold is 6dB increase in RSSI
    AGCCTRL1_CS_REL_6 = 0x10,

    //! Carrier sense relative threshold is 10dB increase in RSSI
    AGCCTRL1_CS_REL_10 = 0x20,

    //! Carrier sense relative threshold is 14dB increase in RSSI
    AGCCTRL1_CS_REL_14 = 0x30,


    //! Carrier Sense absolute threshold disabled
    AGCCTRL1_CS_ABS_DISABLE = 0x08,

    //! Carrier Sense absolute threshold 7dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_7 = 0x09,

    //! Carrier Sense absolute threshold 6dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_6 = 0x0A,

    //! Carrier Sense absolute threshold 5dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_5 = 0x0B,

    //! Carrier Sense absolute threshold 4dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_4 = 0x0C,

    //! Carrier Sense absolute threshold 3dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_3 = 0x0D,

    //! Carrier Sense absolute threshold 2dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_2 = 0x0E,

    //! Carrier Sense absolute threshold 1dB below MAGN_TARGET
    AGCCTRL1_CS_ABS_NEG_1 = 0x0F,

    //! Carrier Sense absolute threshold at MAGN_TARGET
    AGCCTRL1_CS_ABS_0 = 0x00,

    //! Carrier Sense absolute threshold 1dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_1 = 0x01,

    //! Carrier Sense absolute threshold 2dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_2 = 0x02,

    //! Carrier Sense absolute threshold 3dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_3 = 0x03,

    //! Carrier Sense absolute threshold 4dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_4 = 0x04,

    //! Carrier Sense absolute threshold 5dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_5 = 0x05,

    //! Carrier Sense absolute threshold 6dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_6 = 0x06,

    //! Carrier Sense absolute threshold 7dB above MAGN_TARGET
    AGCCTRL1_CS_ABS_7 = 0x07
};

//! AGCCTRL0 (0x1D AGC Control) Register Settings
enum
{
    //! No hysteresis
    AGCCTRL0_HYSTERESIS_NONE = 0x00,

    //! low hysteresis
    AGCCTRL0_HYSTERESIS_LOW = 0x40,

    //! medium hysteresis
    AGCCTRL0_HYSTERESIS_MEDIUM = 0x80,

    //! high hysteresis
    AGCCTRL0_HYSTERESIS_HIGH = 0xC0,


    //! 8 channel filter samples
    AGCCTRL0_WAIT_TIME_8 = 0x00,

    //! 16 channel filter samples
    AGCCTRL0_WAIT_TIME_16 = 0x10,

    //! 24 channel filter samples
    AGCCTRL0_WAIT_TIME_24 = 0x20,

    //! 32 channel filter samples
    AGCCTRL0_WAIT_TIME_32 = 0x30,


    //! Normal operation for freezing AGC gain
    AGCCTRL0_FREEZE_NORMAL = 0x00,

    //! gain frozen when sync received
    AGCCTRL0_FREEZE_SYNC = 0x04,

    //! manually freeze analog, continue to adjust digital as normal
    AGCCTRL0_FREEZE_MANUAL_ANALOG = 0x08,

    //! manually freeze analog and digital
    AGCCTRL0_FREEZE_MANUAL_ANALOG_DIGITAL = 0x0C,


    //! 8 channel filter samples, 4dB OOK decision
    AGCCTRL0_8_OOK4 = 0x00,

    //! 16 channel filter samples, 8dB OOK decision
    AGCCTRL0_16_OOK8 = 0x01,

    //! 32 channel filter samples, 12dB OOK decision
    AGCCTRL0_32_OOK12 = 0x02,

    //! 64 channel filter samples, 16dB OOK decision
    AGCCTRL0_64_OOK16 = 0x03
};

//! WORCTRL (0x20 Wake on Radio Control) Register Settings
enum
{
    //! Power down signal to RC oscillator
    WORCTRL_RC_POWER_DOWN = 0x80,

    //! Automatic initial calibration of the RC oscillator will be performed
    WORCTRL_RC_AUOT_INIT_CAL = 0x00,


    //! event 1 times out 4 clock periods after event 0
    WORCTRL_EVENT_4 = 0x00,

    //! event 1 times out 6 clock periods after event 0
    WORCTRL_EVENT_6 = 0x10,

    //! event 1 times out 8 clock periods after event 0
    WORCTRL_EVENT_8 = 0x20,

    //! event 1 times out 12 clock periods after event 0
    WORCTRL_EVENT_12 = 0x30,

    //! event 1 times out 16 clock periods after event 0
    WORCTRL_EVENT_16 = 0x40,

    //! event 1 times out 24 clock periods after event 0
    WORCTRL_EVENT_24 = 0x50,

    //! event 1 times out 32 clock periods after event 0
    WORCTRL_EVENT_32 = 0x60,

    //! event 1 times out 48 clock periods after event 0
    WORCTRL_EVENT_48 = 0x70,


    //! Enables RC oscillator calibration
    WORCTRL_RC_OSCILLATOR_CAL_ENABLE = 0x08,

    //! Disables RC oscillator calibration
    WORCTRL_RC_OSCILLATOR_CAL_DISABLE = 0x00,


    //! Event 0 resolution 28-29us, max timeout 1.8-1.9s
    WORCTRL_RESOLUTION_TIMEOUT_0 = 0x00,

    //! Event 0 resolution 0.89-0.92ms, max timeout 58-61s
    WORCTRL_RESOLUTION_TIMEOUT_1 = 0x01,

    //! Event 0 resolution 28-30ms, max timeout 31-32min
    WORCTRL_RESOLUTION_TIMEOUT_2 = 0x02,

    //! Event 0 resolution 0.91-0.94s, max timeout 16.5-17.2hours
    WORCTRL_RESOLUTION_TIMEOUT_3 = 0x03
};

//! FSCAL2 (0x24 Frequency Synthesizer Calibration) Register Settings
enum
{
    //! Use high VCO
    FSCAL2_HIGH_VCO = 0x20,

    //! Use low VCO
    FSCAL2_LOW_VCO = 0x00
};

//! RX & TX status reg masks
enum
{
    FIFO_ERR = 0x80                 //!< Overflow in RX FIFO, underflow in TX
};

/*!
    \brief Initial register values for the CC1100.

    These values were computed using the SmartRF Studio SRFS2500.dll revision
    1.4.1.0 provided by Texas Instruments.  Each location in this array
    contains the value to set for the register that corresponds to the index
    (ie. location 0 is register 0, location 1 is register 1...).
*/
const UInt8 INIT_REG_VAL[NUM_INIT_REGS] =
{
    // Reg 0x00, IOCFG2
    GDO_CFG_SYNC,

    // Reg 0x01, IOCFG1
    IOCFG1_LOW_DRIVE_STRENGTH,

    // Reg 0x02, IOCFG0
    GDO_CFG_CCA,

    // Reg 0x03, FIFOTHR
    FIFOTHR_TX_33_RX_32,

    // Reg 0x04, SYNC1
    0x55,

    // Reg 0x05, SYNC0
    0x33,

    // Reg 0x06, PKTLEN, Maximum packet length allowed is 59 bytes
    0x3B,

    // Reg 0x07, PKTCTRL1
    PKTCTRL1_PQT_0 | PKTCTRL1_NO_ADDRESS_CHECK,

    // Reg 0x08, PKTCTRL0
    PKTCTRL0_RF_DATA_IO_FIFO | PKTCTRL0_PKT_LEN_INFINITE,

    // Reg 0x09, ADDR
    0x00,

    // Reg 0x0A, CHANNR
    0x00,

    // Reg 0x0B, FSCTRL1, from SmartRF Studio 152.34kHz
    0x06,

    // Reg 0x0C, FSCTRL0, from SmartRF Studio
    0x00,

    // Reg 0x0D Freq2
    0x22,

    // Reg 0x0E Freq1
    0xB6,

    // Reg 0x0F Freq0
    0x27,

    // Reg 0x10, MDMCFG4, from SmartRF Studio 541.667kHz
    0x2A,

    // Reg 0x11, MDMCFG3 based on 38.4kbps
    0x83,

    // Reg 0x12, MDMCFG2
    MDMCFG2_RADIO_2FSK,

    // Reg 0x13, MDMCFG1
    MSMCFG1_NUM_PREAMBLE_2,

    // Reg 0x14, MDMCFG0
    0xFF,

    // Reg 0x15, DEVIATN, 0x72 - 253.91kHz
    0x72,

    // Reg 0x16, MCSM2
    MCSM2_SYNC_TIME_OUT_END_OF_PKT,

    // Reg 0x17, MCSM1
    MCSM1_CCA_RSSI | MCSM1_RX_PKT_RX | MCSM1_TX_PKT_TX,

    // Reg 0x18, MCSM0
    MCSM0_AUTOCAL_FROM_IDLE | MCSM0_PO_TIMEOUT_64,

    // Reg 0x19, FOCCFG
    FOCCFG_PRE_SYNC_3K | FOCCFG_POST_SYNC_K_DIV_2
      | FOCCFG_BW_DIV_4_COMPENSATION,

    // Reg 0x1A, BSCFG
    BSCFG_PRE_2KI | BSCFG_PRE_3KP | BSCFG_POST_KI_DIV_2 | BSCFG_POST_KP_SAME,

    // Reg 0x1B, AGCCTRL2
    AGCCTRL2_DVGA_NOT_HIGHEST | AGCCTRL2_LNA_MAX | AGCCTRL2_TARGET_33,

    // Reg 0x1C, AGCCTRL1
    AGCCTRL1_LNA_BEFORE_LNA2 | AGCCTRL1_CS_REL_DISABLE | AGCCTRL1_CS_ABS_0,

    // Reg 0x1D, AGCCTRL0
    AGCCTRL0_HYSTERESIS_MEDIUM | AGCCTRL0_WAIT_TIME_16
     | AGCCTRL0_FREEZE_NORMAL | AGCCTRL0_16_OOK8,

    // Reg 0x1E, WOREVT1
    0x87,

    // Reg 0x1F, WOREVT0
    0x6B,

    // Reg 0x20, WORCTL
    WORCTRL_RC_POWER_DOWN | WORCTRL_EVENT_48 | WORCTRL_RC_OSCILLATOR_CAL_ENABLE,

    // Reg 0x21, FREND1, from SmartRF Studio
    0x56,

    // Reg 0x22, FREND0, from SmartRF Studio
    0x10,

    // Reg 0x23, FSCAL3, from SmartRF Studio
    0xA9,

    // Reg 0x24, FSCAL2, from SmartRF Studio
    FSCAL2_HIGH_VCO | 0x0A,

    // Reg 0x25, FSCAL1, from SmartRF Studio
    0x00,

    // Reg 0x26, FSCAL0, from SmartRF Studio
    0x13,

    // Reg 0x27, RCCTRL1, from SmartRF Studio
    0x41,

    // Reg 0x28, RCCTRL0, from SmartRF Studio
    0x00,

    // Reg 0x29, FSTEST, from SmartRF Studio, test only, not supposed to write
    0x57,

    // Reg 0x2A, PTEST, from SmartRF Studio
    0x7F,

    // Reg 0x2B, AGCTEST, from SmartRF Studio, test only, not supposed to write
    0x3F,

    // Reg 0x2C, TEST2, from SmartRF Studio
    0x81,

    // Reg 0x2D, TEST1, from SmartRF Studio
    0x35,

    // Reg 0x2E, TEST0, from SmartRF Studio
    0x09,
};

//! The register settings for setting the desired base frequency.  These are
//! meant to be loaded consecutavily starting with FREQ2 (0x0D).
// 1/15/2011 - Adding #define guards - Note.  Has this code been touched in a while?
// TO-DO : Add the REAL values for the European settings.  The ones below are fake.
static const UInt8 CHANNEL_SETTING[ONE_NET_NUM_CHANNELS][NUM_CHANNEL_REG] =
{
#ifdef _US_CHANNELS
    {0x22, 0xBB, 0x14},             //! channel =   US1, frequency = 903.0 Mhz
    {0x22, 0xC4, 0xEC},             //! channel =   US2, frequency = 904.0 Mhz
    {0x22, 0xCE, 0xC5},             //! channel =   US3, frequency = 905.0 Mhz
    {0x22, 0xD8, 0x9E},             //! channel =   US4, frequency = 906.0 Mhz
    {0x22, 0xE2, 0x76},             //! channel =   US5, frequency = 907.0 Mhz
    {0x22, 0xEC, 0x4F},             //! channel =   US6, frequency = 908.0 Mhz
    {0x22, 0xF6, 0x27},             //! channel =   US7, frequency = 909.0 Mhz
    {0x23, 0x00, 0x00},             //! channel =   US8, frequency = 910.0 Mhz
    {0x23, 0x09, 0xD9},             //! channel =   US9, frequency = 911.0 Mhz
    {0x23, 0x13, 0xB1},             //! channel =  US10, frequency = 912.0 Mhz
    {0x23, 0x1D, 0x8A},             //! channel =  US11, frequency = 913.0 Mhz
    {0x23, 0x27, 0x62},             //! channel =  US12, frequency = 914.0 Mhz
    {0x23, 0x31, 0x3B},             //! channel =  US13, frequency = 915.0 Mhz
    {0x23, 0x3B, 0x14},             //! channel =  US14, frequency = 916.0 Mhz
    {0x23, 0x44, 0xEC},             //! channel =  US15, frequency = 917.0 Mhz
    {0x23, 0x4E, 0xC5},             //! channel =  US16, frequency = 918.0 Mhz
    {0x23, 0x58, 0x9E},             //! channel =  US17, frequency = 919.0 Mhz
    {0x23, 0x62, 0x76},             //! channel =  US18, frequency = 920.0 Mhz
    {0x23, 0x6C, 0x4F},             //! channel =  US19, frequency = 921.0 Mhz
    {0x23, 0x76, 0x27},             //! channel =  US20, frequency = 922.0 Mhz
    {0x23, 0x80, 0x00},             //! channel =  US21, frequency = 923.0 Mhz
    {0x23, 0x89, 0xD9},             //! channel =  US22, frequency = 924.0 Mhz
    {0x23, 0x93, 0xB1},             //! channel =  US23, frequency = 925.0 Mhz
    {0x23, 0x9D, 0x8A},             //! channel =  US24, frequency = 926.0 Mhz
    {0x23, 0xA7, 0x62},             //! channel =  US25, frequency = 927.0 Mhz
#endif
// TO-DO : Add the REAL values for the European settings.  The ones below are fake.
#ifdef _EUROPE_CHANNELS
    {0x00, 0x00, 0x00},             //! channel= EUR1, frequency= 865.8 MHz
    {0x00, 0x00, 0x00},             //!	channel= EUR2, frequency= 866.5 MHz
    {0x00, 0x00, 0x00},             //! channel= EUR3, frequency= 867.2 MHz
#endif
};

/*!
    \brief The register settings for the desired data rate.

    These values need to coincide with the data rates to be supported.  See
    data_rate_t in one_net.h for data rates.  The values to set the data rate
    are based on XTAL = 26.0Mhz.  These values are to be written to the
    MDMCFG3 (0x11) register.
*/
const UInt8 DATA_RATE_SETTING[ONE_NET_MAX_DATA_RATE - BASE_DATA_RATE + 1] =
{
    0x83                            // 38400 kbps
};

//! @} TI_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup TI_typedefs
//! \ingroup TI
//! @{

//! @} TI_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup TI_pri_var
//! \ingroup TI
//! @{

//! Clear Channel Indicator
#define CCA GDO0

//! Sync detect indicator
#define SYNC_DET GDO2

//! The current ONE-NET channel
static UInt8 current_channel = 0;

//! @} TI_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup TI_pri_func
//! \ingroup TI
//! @{

static BOOL command_strobe(const UInt8 CMD_STROBE);
static UInt8 status_reg(UInt8 reg);
static BOOL read_reg(UInt8 reg, UInt8 * data, const UInt8 SIZE);
static BOOL write_reg(UInt8 reg, const UInt8 * DATA, const UInt8 LEN);

//! @} TI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup TI_pub_func
//! \ingroup TI
//! @{

/*!
    \brief Initializes the TI transceiver.

    This function is transceiver specific.  It should configure the transceiver
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
    write_reg(0x00, INIT_REG_VAL, sizeof(INIT_REG_VAL));
    write_reg(FREQ2, CHANNEL_SETTING[current_channel], NUM_CHANNEL_REG);
    INIT_RF_INTERRUPTS();
} // tal_init_transceiver //


/*!
    \brief Sets the transceiver to receive mode.

    \param void

    \return void
*/
void tal_turn_on_receiver(void)
{
    UInt8 reg_data = INIT_REG_VAL[MDMCFG2] | MDMCFG2_16_OF_16_SYNC_CS;

    // do SIDLE & SFRX just to make sure the fifo is clear
    command_strobe(SIDLE);
    command_strobe(SFRX);
    write_reg(MDMCFG2, &reg_data, sizeof(reg_data));
    command_strobe(SRX);
} // tal_turn_on_receiver //


/*!
    \brief Sets the transceiver to transmit mode

    \param void

    \return void
*/
void tal_turn_on_transmitter(void)
{
    UInt8 reg_data = INIT_REG_VAL[IOCFG1] | GDO_CFG_TX_FIFO_THRESHOLD;

    write_reg(IOCFG1, &reg_data, sizeof(reg_data));

    reg_data = INIT_REG_VAL[MDMCFG2] | MDMCFG2_NO_PREAMBLE_SYNC;
    write_reg(MDMCFG2, &reg_data, sizeof(reg_data));

    command_strobe(STX);
} // tal_turn_on_transmitter //


/*!
    \brief  Get the current ONE_NET channel.

    This function is transceiver specific. The ONE-NET channel
    number is be used to configure the frequency
    used by the transceiver.

    \param void

    \return The channel the device is operating on.  This is one of the values
      from on_channel_t

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
        write_reg(FREQ2, CHANNEL_SETTING[current_channel], NUM_CHANNEL_REG);
    } // if the parameter is valid //
} // one_net_set_channel //


// returns TRUE if the channel is clear (declared in one_net_port_specific.h).
BOOL one_net_channel_is_clear(void)
{
    return CCA;
} // one_net_channel_is_clear //


// sets the data rate the transceiver operates at (see one_net_port_specific.h).
void one_net_set_data_rate(const UInt8 DATA_RATE)
{
    if(DATA_RATE <= ONE_NET_MAX_DATA_RATE)
    {
        write_reg(MDMCFG3, &(DATA_RATE_SETTING[DATA_RATE - BASE_DATA_RATE]),
          NUM_DATA_RATE_REG);
    } // if the data rate is valid //
} // one_net_set_data_rate //


one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    tick_t end = one_net_tick() + DURATION;

    TAL_TURN_ON_RECEIVER();

    while(!SYNC_DET)
    {
        if(one_net_tick() >= end)
        {
            return ONS_TIME_OUT;
        } // if timed out looking for sync //
    } // while the sync byte has not been detected //

    TURN_ON_RX_LED();
    return ONS_SUCCESS;
} // one_net_look_for_pkt //

/*
    This function blocks until LEN bytes are read.

    This needs to be called fast enough to not overflow the rx fifo as the data
    is left in the fifo until it is read.  Also, this routine ensures that when
    reading the fifo, the last byte in the fifo is not read to avoid the bug
    mentioned in the data sheet when the last byte is read (internal fifo pointer
    may not be updated correctly when a byte is received over the rf interface
    at the same time).
*/
UInt16 one_net_read(UInt8 * data, const UInt16 LEN)
{
    UInt16 bytes_remaining = LEN;
    UInt16 bytes_in_fifo, bytes_to_read;

    // Loop to read in the data from the CC1100 receive FIFO.
    // Note that too much SPI activity can diminish the receiver sensitivity,
    // so delays may need to be put in if this prooves to be the case.
    do
    {
        bytes_in_fifo = status_reg(RXBYTES);

        if(bytes_in_fifo & FIFO_ERR)
        {
            command_strobe(SIDLE);
            return 0;
        } // if an overflow has occurred //

        // make sure more than 1 byte is available to avoid the error mentioned
        // in the data sheet about emptying the receive FIFO at the same time
        // the next byte is received causing a problem with the receive FIFO
        // pointer.
        if(bytes_in_fifo > 1)
        {
            bytes_in_fifo--;
            bytes_to_read = bytes_in_fifo < bytes_remaining ? bytes_in_fifo
              : bytes_remaining;

            if(!read_reg(FIFO_REG, &(data[LEN - bytes_remaining]),
              bytes_to_read))
            {
                return LEN - bytes_remaining;
            } // if reading failed //

            bytes_remaining -= bytes_to_read;
        } // if more than 1 byte available //
    } while(bytes_remaining);

    return LEN;
} // one_net_read //


/*
    This function blocks until LEN bytes are written.

    At a later date, may convert to storing a pointer to the data and enabling
    the TX interrupt based on the tx fifo level.  If the fifo falls below the
    threshold, an interrupt will be generated, and the fifo filled as much
    as possible.  Reminder: This MAY be implemented at a later time.  The
    current implementation is a blocks until all data is written to the FIFO.
*/
UInt16 one_net_write(const UInt8 * DATA, const UInt16 LEN)
{
    #define TX_FIFO_THRESHOLD_INDICATOR MISO

    const UInt8 reg_data = INIT_REG_VAL[IOCFG1] | GDO_CFG_TX_UNDERFLOW;

    UInt16 bytes_to_write;
    UInt16 bytes_written = 0;

    TURN_ON_TX_LED();

    // do SIDLE & SFTX just to make sure the fifo is clear
    command_strobe(SIDLE);
    command_strobe(SFTX);

    // prime the data to send
    bytes_to_write = LEN < FIFO_SIZE ? LEN : FIFO_SIZE;
    if(!write_reg(FIFO_REG, DATA, bytes_to_write))
    {
        command_strobe(SIDLE);
        return 0;
    } // if writing failed //

    bytes_written += bytes_to_write;

    // turn on the transmitter
    TAL_TURN_ON_TRANSMITTER();

    // block while waiting to load the rest of the data
    while(bytes_written < LEN)
    {
        if(!TX_FIFO_THRESHOLD_INDICATOR)
        {
            if(!write_reg(FIFO_REG, &(DATA[bytes_written++]), 1))
            {
                command_strobe(SIDLE);

                // -1 since this byte was not successfully written
                return bytes_written - 1;
            } // if writing failed //
        } // if transmit fifo is below the threshold //
    } // loop to write the rest of the bytes to the FIFO //

    // use GDO1 to indicate when the transmit register underflows.  Once it
    // does, the last byte has been transmitted.  This register was chosen over
    // one of the others, since it is already being re-written depending on rx
    // or tx mode, and if another register was used, then we'd have 2 registers
    // depending on modes, so this is slightly less work.  If a different pin is
    // used, TX_FIFO_UNDERFLOW_INDICATOR needs to be updated.
    write_reg(IOCFG1, &reg_data, sizeof(reg_data));
    return bytes_written;
} // one_net_write //


/*
    Once the tx FIFO underflows, we know we are done transmitting.
*/
BOOL one_net_write_done(void)
{
    #define TX_FIFO_UNDERFLOW_INDICATOR MISO

    if(!TX_FIFO_UNDERFLOW_INDICATOR)
    {
        return FALSE;
    } // if write is not done //

    TAL_TURN_ON_RECEIVER();

    return TRUE;
} // one_net_write_done //

//! @} TI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup TI_pri_func
//! \ingroup TI
//! @{

/*!
    \brief Sends a command strobe to the CC1100.

    This does not retrieve the status byte that is returned.

    \param[in] CMD_STROBE The command strobe to send to the CC1100.

    \return TRUE if the command probe was successfully sent.
            FALSE if the command probe was not successfully sent.
*/
static BOOL command_strobe(const UInt8 CMD_STROBE)
{
    BOOL rv;

    return write_reg(CMD_STROBE, 0, 0);
} // command_strobe //


/*!
    \brief Returns the value of one of the status registers.

    The status registers range from 0x30-0x3D and are read only.  Burst access
    to status registers is not allowed by the CC1100.

    \param[in] reg The status register to read.

    \return The value the status register returns
*/
static UInt8 status_reg(UInt8 reg)
{
    UInt8 status;

    reg |= BURST_BIT;

    if(!read_reg(reg, &status, sizeof(status)))
    {
        return 0;
    } // if reading the status register failed //

    return status;
} // status_reg //


/*!
    \brief Reads data starting at reg.

    This function sets the burst bit if more than 1 register is to be returned.
    Registers must be consecutive.  The status byte back from the CC1100 is
    ignored.

    \param[in] reg The starting register to read.
    \param[out] data Buffer to store the register data that was returned.
    \param[in] SIZE The size of data in bytes.  This is also the number of
      consecutive registers to read.

    \return TRUE if reading the register(s) was successful
            FALSE if reading the register(s) was not successful
*/
static BOOL read_reg(UInt8 reg, UInt8 * data, const UInt8 SIZE)
{
    BOOL rv;

    if(!data || !SIZE)
    {
        return FALSE;
    } // if any of the parameters are invalid //

    reg |= READ_BIT;

    if(SIZE > 1)
    {
        reg |= BURST_BIT;
    } // if burst read //

    SSNOT = 0;

    // need to wait for SO to go low to indicate the crystal is running
    while(MISO);
    if((rv = spi_transfer(&reg, sizeof(reg), 0, 0)) == TRUE)
    {
        rv = spi_transfer(0, 0, data, SIZE);
    } // if writing the header byte was successful //
    SSNOT = 1;

    return rv;
} // read_reg //


/*!
    \brief Writes data starting at reg.

    This function sets the write bit, as well as the burst bit if it needs to.
    Registers must be consecutive.  The status byte back from the CC1100 is
    ignored.

    \param[in] reg The starting register to write
    \param[in] DATA The data to write.
    \param[in] LEN The number of bytes to write (the size of data).
*/
static BOOL write_reg(UInt8 reg, const UInt8 * DATA, const UInt8 LEN)
{
    BOOL rv;

    if(LEN > 1)
    {
        reg |= BURST_BIT;
    } // if burst write //

    SSNOT = 0;

    // need to wait for SO to go low to indicate the crystal is running
    while(MISO);
    if((rv = spi_transfer(&reg, sizeof(reg), 0, 0)) == TRUE && DATA && LEN)
    {
        rv = spi_transfer(DATA, LEN, 0, 0);
    } // if writing the header byte was successful //
    SSNOT = 1;

    return rv;
} // write_reg //

//! @} TI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} TI
