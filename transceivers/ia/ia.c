//! \defgroup IA Integration Associates IA4421 driver.
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
    \file ia4421.c
    \brief Driver for the Integration Associates IA4421.

    This file implements the communications necessary for communicating with the
    Integration Associates IA4421.  It abstracts the actual processor pin
    assignments using definitions found in io_port_mapping.h.  This file also
    contains the implementation for the transceiver specific functions (such as
    look_for_packet, one_net_read, one_net_write) declared in
    one_net_port_specific.h
*/

/*
    Note from RFM on SPI speed : 
    When reading the RX FIFO, the internal access time is limited to Fxtal/4, or
    2.5MHz, so you can only run the SPI (FIFO read ONLY!) at 2.5MHz max.  If you
    attempt to read it faster then you'll get lots of data errors.  Writing 
    internal registers and TX register can be done at full speed, which is
    around 20MHz. 
*/

#include "hal_ia.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "spi.h"
#include "tal.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup IA_const
//! \ingroup IA
//! @{

//! Status register values
enum
{
    //! Indicates tx ready in tx mode, or rx fifo reached it's limit in rx mode
    STATUS_TX_RX = 0x8000,

    //! Power on reset occured
    STATUS_POR = 0x4000,

    //! Buffer underflow in tx mode, or buffer overflow in receive mode
    STATUS_BUF_ERR = 0x2000,
    
    //! Wake up timer overflow
    STATUS_WAKE = 0x1000,
    
    //! High to low logic level change on interrupt pin (pin 16)
    STATUS_INT_PIN_16 = 0x0800,
    
    //! Battery level low
    STATUS_BATT = 0x0400,
    
    //! rx fifo empty
    STATUS_RX_FIFO_EMPTY = 0x0200,
    
    //! RSSI above preprogrammed threshold
    STATUS_RSSI = 0x0100,
    
    //! Indicates good data
    STATUS_GOOD_DATA = 0x0080,
    
    //! Clock recovery is locked
    STATUS_CLK_LOCK = 0x0040,
    
    //! AFC cycle bit
    STATUS_AFC_CYCLE = 0x0020,
    
    //! Frequency difference (1 - higher, 0 - lower)
    STATUS_FREQ_DIF = 0x0010,
    
    //! Frequency offset mask
    STATUS_FREQ_OFFSET_MASK = 0x000F
};

//! Configuration Register
enum
{
    CFG_REG = 0x8000,               //!< Configuration register address
    
    CFG_DATA_REG_ENABLE = 0x0080,   //!< Enables the data register
    CFG_FIFO_ENABLE = 0x0040,       //!< Enable receive fifo

    // band select
    CFG_433_BAND = 0x0010,          //!< The 433Mhz band
    CFG_868_BAND = 0x0020,          //!< The 868Mhz band
    CFG_915_BAND = 0x0030,          //!< The 915Mhz band
    
    // crystal load capacapties
    CFG_CRYSTAL_LOAD_10_5 = 0x0004  //!< Crystal has a 10.5pF capacitance
};

//! Automatic Frequency Adjust Register Values
enum
{
    AFA_REG = 0xC400,               //!< The register address
    
    // Mode Selection
    AFM_OFF = 0x0000,               //!< AFA off
    AFM_PWR_UP = 0x0040,            //!< Run once after power up
    AFM_RCV_ONLY = 0x0080,          //!< Offset during receive only
    AFM_VID_INDEPENDENT = 0x00C0,   //!< Offset independent of VDI state
    
    // Allowable frequence offset
    AFO_NO_LIMIT = 0x0000,          //!< No allowable frequency offset limit
    AFO_15_16_LIMIT = 0x0010,       //!< (+15/-16)*fres allowable freq offset
    AFO_7_8_LIMIT = 0x0020,         //!< (+7/-8)*fres allowable freq offset
    AFO_3_4_LIMIT = 0x0030,         //!< (+3/-4)*fres allowable freq offset
    
    AFA_FREQ_STROBE = 0x0008,       //!< Starts a manual freq adjustment strobe
    AFA_HIGH_ACCURACY = 0x0004,     //!< freq adjustment to high accuracy mode
    AFA_FREQ_REG_ENABLE = 0x0002,   //!< Offset sample added to freq ctl of PLL
    AFA_FREQ_OFFSET_ENABLE = 0x0001 //!< Enable offset frequency calculation
};

//! Trasmit Configuration Register
enum
{
    TXCR_REG = 0x9800,              //!< Register address
    
    TXCR_MODULATION_POL = 0x0100,   //!< Modulation polarity
    
    // Modulation bandwidth
    TXCR_FREQ_DEV_15 = 0x0000,      //!< 15kHz frequency deviation
    TXCR_FREQ_DEV_30 = 0x0010,      //!< 30kHz frequency deviation
    TXCR_FREQ_DEV_45 = 0x0020,      //!< 45kHz frequency deviation
    TXCR_FREQ_DEV_60 = 0x0030,      //!< 60kHz frequency deviation
    TXCR_FREQ_DEV_75 = 0x0040,      //!< 75kHz frequency deviation
    TXCR_FREQ_DEV_90 = 0x0050,      //!< 90kHz frequency deviation
    TXCR_FREQ_DEV_105 = 0x0060,     //!< 105kHz frequency deviation
    TXCR_FREQ_DEV_120 = 0x0070,     //!< 120kHz frequency deviation
    TXCR_FREQ_DEV_135 = 0x0080,     //!< 135kHz frequency deviation
    TXCR_FREQ_DEV_150 = 0x0090,     //!< 150kHz frequency deviation
    TXCR_FREQ_DEV_165 = 0x00A0,     //!< 165kHz frequency deviation
    TXCR_FREQ_DEV_180 = 0x00B0,     //!< 180kHz frequency deviation
    TXCR_FREQ_DEV_195 = 0x00C0,     //!< 195kHz frequency deviation
    TXCR_FREQ_DEV_210 = 0x00D0,     //!< 210kHz frequency deviation
    TXCR_FREQ_DEV_225 = 0x00E0,     //!< 225kHz frequency deviation
    TXCR_FREQ_DEV_240 = 0x00F0,     //!< 240kHz frequency deviation
    
    // Output transmit power
    TXCR_TX_PWR_MAX = 0x0000,       //!< max transmit output power
    TXCR_TX_PWR_3 = 0x0001,         //!< -3dB transmit output power
    TXCR_TX_PWR_6 = 0x0020,         //!< -6dB transmit output power
    TXCR_TX_PWR_9 = 0x0003,         //!< -9dB transmit output power
    TXCR_TX_PWR_12 = 0x0004,        //!< -12dB transmit output power
    TXCR_TX_PWR_15 = 0x0005,        //!< -15dB transmit output power
    TXCR_TX_PWR_18 = 0x0006,        //!< -18dB transmit output power
    TXCR_TX_PWR_21 = 0x0007,        //!< -21dB transmit output power
};

//! Transmit Register
enum
{
    TX_DATA_REG = 0xB800            //!< Trasmit data register address
};

//! Receiver Control Register
enum
{
    RXCR_REG = 0x9000,              //!< Receiver Control Register
    
    // Pin16 function select
    RXCR_P16_INT_INPUT = 0x0000,    //!< Pin16 Interrupt input
    RXCR_P16_VDI_OUTPUT = 0x0400,   //!< Pin16 Valid data indicator output
    
    // valid data detector response time
    RXCR_DATA_RESP_FAST = 0x0000,   //!< fast sync detect
    RXCR_DATA_RESP_MID = 0x0100,    //!< Medium sync detect
    RXCR_DATA_RESP_SLOW = 0x0200,   //!< Slow sync detect
    RXCR_DATA_RESP_CONT = 0x0300,   //!< Continuous sync detect
    
    // Receiver baseband bandwidth
    RXCR_RX_BW_400 = 0x0020,        //!< 400kHz baseband bandwidth
    RXCR_RX_BW_340 = 0x0040,        //!< 340kHz baseband bandwidth
    RXCR_RX_BW_270 = 0x0060,        //!< 270kHz baseband bandwidth
    RXCR_RX_BW_200 = 0x0080,        //!< 200kHz baseband bandwidth
    RXCR_RX_BW_134 = 0x00A0,        //!< 134kHz baseband bandwidth
    RXCR_RX_BW_67 = 0x00C0,         //!< 67kHz baseband bandwidth
    
    // Receiver LNA Gain
    RXCR_LNA_GAIN_0 = 0x0000,       //!< 0dB LNA Gain
    RXCR_LNA_GAIN_14 = 0x0008,      //!< -14dB LNA Gain
    RXCR_LNA_GAIN_6 = 0x0010,       //!< -6dB LNA Gain
    RXCR_LNA_GAIN_20 = 0x0018,      //!< -20dB LNA Gain
    
    // Digital RSSI threshold
    RXCR_RSSI_103 = 0x0000,         //!< -103 digital rssi threshold
    RXCR_RSSI_97 = 0x0001,          //!< -97 digital rssi threshold
    RXCR_RSSI_91 = 0x0002,          //!< -91 digital rssi threshold
    RXCR_RSSI_85 = 0x0003,          //!< -85 digital rssi threshold
    RXCR_RSSI_79 = 0x0004,          //!< -79 digital rssi threshold
    RXCR_RSSI_73 = 0x0005,          //!< -73 digital rssi threshold
    RXCR_RSSI_67 = 0x0006,          //!< -67 digital rssi threshold
    RXCR_RSSI_61 = 0x0007           //!< -61 digital rssi threshold
};

//! Baseband filter register
enum
{
    BASEBAND_FILTER_REG = 0xC200,   //!< Baseband filter register
    
    BASEBAND_AUTO_CLK = 0x0080,     //!< Automatic clock recovery
    BASEBAND_AUTO_CLK_CTL = 0x0040, //!< Manual clock recovery lock control
    BASEBAND_D_FILTER = 0x0010,     //!< Digital filter
};

//! FIFO & reset configuration reg
enum
{
    FCFG_REG = 0xCA00,              //!< FIFO & resent configuration reg
    
    // FIFO fill bit count threshold values
    FCFG_FILL_8 = 0x0080,           //!< Rx 8 bits before generating interrupt
    
    // Sync detect length
    FCFG_SYNC_WORD = 0x0000,        //!< Sync on a 2 byte value (0x2DXX)
    FCFG_SYNC_BYTE = 0x0008,        //!< Sync on a 1 byte value
    
    // FIFO fill conditions
    FCFG_FILL_SYNC = 0x0000,        //!< FIFO fills when sync is detected
    FCFG_FILL_CONT = 0x0004,        //!< FIFO fills continuously
    
    // Sync pattern fifo fill
    FCFG_SYNC_FILL_EN = 0x0002,     //!< Enable FIFO filling on sync detect
    FCFG_SYNC_FILL_DIS = 0x0000,    //!< Disable FIFO filling on sync detect
    
    FCFG_RST_DISABLE = 0x0001       //!< Disable reset mode
};

//! Sync byte configuration register
enum
{
    SYNC_REG = 0xCE00               //!< Sync byte configuration register
};

//! Data rate setup register
enum
{
    DR_REG = 0xC600,                //!< Data rate setup register

    DR_PRE_ENABLE = 0x0080,         //!< Enable data rate prescaler
    
    // Data rate settings
    DR_38_4 = 0x0008                //!< 38400 (38314) baud
};

//! Power Management settings
enum
{
    PWR_REG = 0x8200,               //!< Power Management register
    
    PWR_RX_EN = 0x0080,             //!< Enables everything needed to rx
    PWR_TX_EN = 0x0020,             //!< Enables everything needed to tx

    PWR_CLK_DIS = 0x0001,           //!< Disables clock output
    
    //! Power management settings that do not change when either the reciever
    //! or the transmitter are turned on
    PWR_CONST_SETTINGS = PWR_CLK_DIS
};

//! Wakeup Timer period register
enum
{
    WAKE_REG = 0xE000               //!< Wake up timer period register
};

//! Duty Cycle Register
enum
{
    DUTY_REG = 0xC800               //!< Duty cycle register
};

//! Battery Detection & Clock Output register
enum
{
    BAT_CLK_REG = 0xC000,           //!< Battery detection & clk out reg

    // clock out settings
    CLK_OUT_1 = 0x0000,             //!< Set clock output to 1Mhz
    CLK_OUT_1_25 = 0x0020,          //!< Set clock output to 1.25Mhz
    CLK_OUT_1_66 = 0x0040,          //!< Set clock output to 1.66Mhz
    CLK_OUT_2 = 0x0060,             //!< Set clock output to 2Mhz
    CLK_OUT_2_5 = 0x0080,           //!< Set clock output to 2.5Mhz
    CLK_OUT_3_33 = 0x00A0,          //!< Set clock output to 3.33Mhz
    CLK_OUT_5 = 0x00C0,             //!< Set clock output to 5Mhz
    CLK_OUT_10 = 0x00E0             //!< Set clock output to 10Mhz
};

//! PLL Configuration Register
enum
{
    PLL_REG = 0xCC00,               //!< PLL register

    // PLL slew
    PLL_SLEW_5 = 0x0000,            //!< >5Mhz
    PLL_SLEW_3 = 0x0020,            //!< 3 Mhz
    PLL_SLEW_2_5 = 0x0040,          //!< <2.5Mhz
    
    // Crystal startup time
    PLL_CRYSTAL_START_1 = 0x0000,   //!< 1ms startup, 620uA
    PLL_CRYSTAL_START_2 = 0x0010,   //!< 2ms startup, 460uA
    
    PLL_PHASE_DETECT_DLY = 0x0008,  //!< Phase detector delay enable
    PLL_PHASE_DITHER_DIS = 0x0004,  //!< Disable dithering
    
    PLL_BW_L90 = 0x0000,            //!< PLL Bandwidth < 90kbps
    PLL_BW_G90 = 0x0001             //!< PLL bandwidth > 90kbps
};

//! The values that need to be set into FREQ2, FREQ1, FREQ0 to get the desired
//! ONE-NET channel setting
static const UInt16 CHANNEL_SETTING[ONE_NET_NUM_CHANNELS] =
{
    0xA190,                         // channel = 1, frequency = 903.0 Mhz
    0xA215,                         // channel = 2, frequency = 904.0 Mhz
    0xA29B,                         // channel = 3, frequency = 905.0 Mhz
    0xA320,                         // channel = 4, frequency = 906.0 Mhz
    0xA3A5,                         // channel = 5, frequency = 907.0 Mhz
    0xA42B,                         // channel = 6, frequency = 908.0 Mhz
    0xA4B0,                         // channel = 7, frequency = 909.0 Mhz
    0xA535,                         // channel = 8, frequency = 910.0 Mhz
    0xA5BB,                         // channel = 9, frequency = 911.0 Mhz
    0xA640,                         // channel = 10, frequency = 912.0 Mhz
    0xA6C5,                         // channel = 11, frequency = 913.0 Mhz
    0xA74B,                         // channel = 12, frequency = 914.0 Mhz
    0xA7D0,                         // channel = 13, frequency = 915.0 Mhz
    0xA855,                         // channel = 14, frequency = 916.0 Mhz
    0xA8DB,                         // channel = 15, frequency = 917.0 Mhz
    0xA960,                         // channel = 16, frequency = 918.0 Mhz
    0xA9E5,                         // channel = 17, frequency = 919.0 Mhz
    0xAA6B,                         // channel = 18, frequency = 920.0 Mhz
    0xAAF0,                         // channel = 19, frequency = 921.0 Mhz
    0xAB75,                         // channel = 20, frequency = 922.0 Mhz
    0xABFB,                         // channel = 21, frequency = 923.0 Mhz
    0xAC80,                         // channel = 22, frequency = 924.0 Mhz
    0xAD05,                         // channel = 23, frequency = 925.0 Mhz
    0xAD8B,                         // channel = 24, frequency = 926.0 Mhz
    0xAE10                          // channel = 25, frequency = 927.0 Mhz
};

/*!
    \brief The initial register values for the xe1205
    
    Output Frequency    =       918.500 MHz
    Deviation           = +/-   240.000 kHz
    Bit Rate            =       38.400 kBaud
*/
static const UInt16 INIT_REG_VAL[] =
{
    // configuration register
    CFG_REG | CFG_DATA_REG_ENABLE | CFG_FIFO_ENABLE | CFG_915_BAND
      | CFG_CRYSTAL_LOAD_10_5,

    // Auto Frequency adjust register.
    AFA_REG | AFM_RCV_ONLY | AFO_3_4_LIMIT | AFA_HIGH_ACCURACY
      | AFA_FREQ_REG_ENABLE | AFA_FREQ_OFFSET_ENABLE,

    // TX config register
    TXCR_REG | TXCR_FREQ_DEV_240,

    // RX control register
    RXCR_REG | RXCR_P16_VDI_OUTPUT | RXCR_RX_BW_400 | RXCR_RSSI_85,

    // baseband filter register
    // 0x0028 are 2 unused bits, write 1's.  0x0006 is data quality threshold
    BASEBAND_FILTER_REG | BASEBAND_AUTO_CLK | 0x0028 | 0x0006,

    // FIFO and reset configuration register
    FCFG_REG | FCFG_FILL_8 | FCFG_SYNC_BYTE | FCFG_FILL_SYNC
      | FCFG_SYNC_FILL_DIS | FCFG_RST_DISABLE,

    // synce byte configuration
    SYNC_REG | 0x0033,

    // data rate setup register
    DR_REG | DR_38_4,

    // Wake-up Timer period register
    WAKE_REG,
    
    // Duty cycle register
    DUTY_REG,

    // battery detection & clock out
    BAT_CLK_REG | CLK_OUT_10,

    // PLL configuration register.  The 0x0002 is for an unused bit, write 1
    PLL_REG | PLL_SLEW_2_5 | PLL_CRYSTAL_START_2 | PLL_PHASE_DITHER_DIS
      | PLL_BW_G90 | 0x0002
};

enum
{
    //! The BASE_DATA_RATE setting for the IA.
    BASE_DATA_RATE_SETTING = 0x03,
};

/*!
    \brief The register settings for setting the desired data rate.
    
    These values need to coincide with the data rates to be supported.  See
    data_rate_t in one_net.h for data rates.
*/
const UInt16 DATA_RATE_SETTING[ONE_NET_MAX_DATA_RATE + 1] =
{
    DR_REG | DR_38_4
};

//! @} IA_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup IA_typedefs
//! \ingroup IA
//! @{

//! @} IA_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup IA_pri_var
//! \ingroup IA
//! @{

//! index into rx_rf_data
UInt16 rf_idx = 0;

//! bytes currently in rx_rf_data
UInt16 rf_count = 0;

//! Buffer to receive data from the rf interface.  Added 1 to send NULL byte to
//! transceiver since it buffers 2 bytes.  NULL is sent so the last byte will be
//! transmitted.
UInt8 rf_data[ONE_NET_MAX_ENCODED_PKT_LEN + 1];

//! Masks each bit in a byte in the interrupt routines when data is transfered.
UInt8 bit_mask;

//! The current ONE-NET channel
static UInt8 current_channel = 12;

//! @} IA_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup IA_pri_func
//! \ingroup IA
//! @{

static UInt16 rx_rf_data_bytes(UInt8 * const data, const UInt16 LEN);
static UInt8 send_rf_data(const UInt8 * DATA, const UInt16 LEN);

static BOOL read_status_reg(UInt16 * const status);
static BOOL write_reg(const UInt16 REG);

//! @} IA_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup IA_pub_func
//! \ingroup IA
//! @{

/*!
    \brief Initializes the IA transceiver.

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
    UInt8 i;

    TAL_INIT_PORTS();

    ENABLE_TRANSCEIVER();

    for(i = 0; i < sizeof(INIT_REG_VAL) / sizeof(INIT_REG_VAL[0]); i++)
    {
        write_reg(INIT_REG_VAL[i]);
    } // loop to initialize transceiver //

    write_reg(CHANNEL_SETTING[current_channel]);
} // tal_init_transceiver //    


/*!
    \brief Sets the transceiver to receive mode

    \param void

    \return void
*/
void tal_turn_on_receiver(void)
{
    write_reg(CFG_REG | CFG_DATA_REG_ENABLE | CFG_FIFO_ENABLE | CFG_915_BAND
      | CFG_CRYSTAL_LOAD_10_5);
    write_reg(PWR_REG | PWR_CONST_SETTINGS | PWR_RX_EN);

    write_reg(FCFG_REG | FCFG_FILL_8 | FCFG_SYNC_BYTE | FCFG_RST_DISABLE
      | FCFG_SYNC_FILL_DIS);
    write_reg(FCFG_REG | FCFG_FILL_8 | FCFG_SYNC_BYTE | FCFG_RST_DISABLE
      | FCFG_SYNC_FILL_EN);
} // tal_turn_on_receiver //


/*!
    \brief Sets the transceiver to transmit mode

    \param void

    \return void
*/
void tal_turn_on_transmitter(void)
{
    write_reg(PWR_REG | PWR_CONST_SETTINGS | PWR_TX_EN);
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
        write_reg(CHANNEL_SETTING[current_channel]);
    } // if the parameter is valid //
} // one_net_set_channel //


// returns TRUE if the channel is clear (declared in one_net_port_specific.h).
BOOL one_net_channel_is_clear(void)
{
    UInt16 status;                  // status register value

    if(read_status_reg(&status))
    {
        // If bit is set, another device is transmitting, so the channel is
        // not clear
        return !(status & STATUS_RSSI);
    } // if reading the register was successful //
    
    return FALSE;
} // one_net_channel_is_clear //


// sets the data rate the transceiver operates at (see one_net_port_specefic.h).
void one_net_set_data_rate(const UInt8 DATA_RATE)
{
    if(DATA_RATE <= ONE_NET_MAX_DATA_RATE)
    {
        write_reg(DATA_RATE_SETTING[DATA_RATE]);
    } // if the data rate is valid //
} // one_net_set_data_rate //


// Receives the packet from the rf interface
one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    tick_t end = one_net_tick() + DURATION;

    TAL_TURN_ON_RECEIVER();   

    rf_count = 0;
    rf_idx = 0;

    while(!VDI)
    {   
        if(one_net_tick() >= end)
        {
		    return ONS_TIME_OUT;
        } // if done looking //
    } // while no sync detect //

    // the id length should be the location in the byte stream where the
    // PID will be received.  Look for the PID so we know how many bytes
    // to receive.
    if((rf_count = rx_rf_data_bytes(rf_data,
      ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX + 1))
      == ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX + 1)
    {
        // All packet size constants below are including the PREAMBLE &
        // SOF.  Since these cause the sync detect, these won't be read
        // in, so the packet size that is being read in is shorter, so
        // subtract the ONE_NET_ENCODED_DST_DID_IDX since that is where the
        // read is being started.
        switch(rf_data[rf_count - 1])
        {
            case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
            {
                rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                  48 - ONE_NET_ENCODED_DST_DID_IDX);
                break;
            } // MASTER invite new CLIENT case //

            case ONE_NET_ENCODED_SINGLE_DATA_ACK:       // fall through
            case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
            case ONE_NET_ENCODED_SINGLE_DATA_NACK:      // fall through
            case ONE_NET_ENCODED_BLOCK_DATA_ACK:        // fall through
            case ONE_NET_ENCODED_BLOCK_DATA_NACK:       // fall through
            case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
            {
                rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                  17 - ONE_NET_ENCODED_DST_DID_IDX);
                break;
            } // acks/nacks/keep alive case for data packets //

            case ONE_NET_ENCODED_SINGLE_TXN_ACK:        // fall through
            case ONE_NET_ENCODED_BLOCK_TXN_ACK:
            {
                rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                  15 - ONE_NET_ENCODED_DST_DID_IDX);
                break;
            } // single/block txn ack case //

            case ONE_NET_ENCODED_SINGLE_DATA:           // fall through
            case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
            {
                rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                  26 - ONE_NET_ENCODED_DST_DID_IDX);
                break;
            } // (repeat)single data case //
            
            #ifndef _ONE_NET_SIMPLE_CLIENT
                case ONE_NET_ENCODED_BLOCK_DATA:        // fall through
                case ONE_NET_ENCODED_REPEAT_BLOCK_DATA: // fall through
                case ONE_NET_ENCODED_STREAM_DATA:
                {
                    rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                      58 - ONE_NET_ENCODED_DST_DID_IDX);
                    break;
                } // (repeat)block, stream data case //
            #endif // ifndef _ONE_NET_SIMPLE_CLIENT //
            
            case ONE_NET_ENCODED_DATA_RATE_TEST:
            {
                rf_count += rx_rf_data_bytes(&(rf_data[rf_count]),
                  20 - ONE_NET_ENCODED_DST_DID_IDX);
                break;
            } // data rate test case //

            default:
            {
                return ONS_BAD_PKT_TYPE;
                break;
            } // default case //
        } // switch on PID //

        TURN_ON_RX_LED();
        return ONS_SUCCESS;
    } // if PID read //

    return ONS_TIME_OUT;
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
    
    Does not return until all the data has been transmitted out of the rf
    interface.

    \param[in] DATA An array of bytes to be sent out of the rf interface
    \param[in] LEN The number of bytes to send

    \return The number of bytes sent.
*/
UInt16 one_net_write(const UInt8 * DATA, const UInt16 LEN)
{
    const UInt8 TX_DATA_REG_ADDR = (UInt8)(TX_DATA_REG >> 8);

    TURN_ON_TX_LED();

    rf_idx = 0;
    // subtract 1 from sizeof(rf_data) to save space for the NULL byte
    rf_count = LEN < sizeof(rf_data) - 1 ? LEN : sizeof(rf_data) - 1;
    one_net_memmove(rf_data, DATA, rf_count);

    // add NULL byte so the last data byte will be transmitted correctly.
    rf_data[rf_count++] = 0x00;

    // Prime the transmit data address with the first 2 bytes.  This needs to
    // be done before enabling the transmitter
    send_rf_data(&(rf_data[rf_idx++]), 1);

    TAL_TURN_ON_TRANSMITTER();
    SSNOT = 0;
    spi_transfer(&TX_DATA_REG_ADDR, sizeof(TX_DATA_REG_ADDR), 0, 0);
    while(rf_idx < rf_count)
    {
        while(!MISO);
        spi_transfer(&(rf_data[rf_idx++]), 1, 0, 0);
    }
    SSNOT = 1;

    TAL_TURN_ON_RECEIVER();
    return rf_count - 1;
} // one_net_write //


BOOL one_net_write_done(void)
{
    return TRUE;
} // one_net_write_done //

//! @} IA_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup IA_pri_func
//! \ingroup IA
//! @{

/*!
    \brief Reads in data from the rf interface.

    This function will time out if there are no more characters to be read from
    the transceiver.  The timeout is hardcoded to be 50 ticks (25ms).

    \param[out] data Buffer to return the read data in.
    \param[in] LEN The number of bytes to read (data must be at least this big)
    
    \return The number of bytes read.
*/
static UInt16 rx_rf_data_bytes(UInt8 * const data, const UInt16 LEN)
{
    #define DATA_AVAILABLE DATA_CLK // data available pin

    enum
    {
        // Max number of ticks to wait in between characters
        MAX_CH_DLY = 50
    };
    
    tick_t timeout;
    UInt16 bytes_read;
    UInt8 mask;

    if(!data || !LEN)
    {
        return 0;
    } // if the data was not read //

    SCLK = 0;
    FSEL_NOT = 0;
    for(bytes_read = 0; bytes_read < LEN; bytes_read++)
    {
        data[bytes_read] = 0x00;
        for(mask = 0x80; mask; mask >>= 1)
        {
            timeout = one_net_tick() + MAX_CH_DLY;
            while(!DATA_AVAILABLE)
            {
                if(timeout < one_net_tick())
                {
                    FSEL_NOT = 1;
                    return bytes_read ? bytes_read - 1 : 0;
                } // if timed out waiting for data to become available //
            } // loop to wait for data to become available //
            
            SCLK = 1;
            if(MISO)
            {
                data[bytes_read] |= mask;
            } // if the bit is high //
            SCLK = 0;
        } // loop to read in each bit of the byte
    } // loop to read in the desired number of bytes //

    FSEL_NOT = 1;
    return bytes_read;
} // rx_rf_data_bytes //


/*!
    \brief Sends data to the IA to be transmitted through the rf interface.
    
    This function is considered "protected."  Currently, only 1 byte will be
    transmitted at a time.
    
    \param[in] DATA Pointer to data to send.  Currently, should only be 1 byte
    \param[in] LEN The number of bytes to send (again, will currently only send
      1 byte).

    \return The number of bytes sent.
*/
UInt8 send_rf_data(const UInt8 * DATA, const UInt16 LEN)
{
    UInt16 tx_len = LEN;            // number of bytes transmitted

    if(!DATA || !LEN)
    {
        return 0;
    } // if the parameters are invalid //
    
    write_reg(TX_DATA_REG | (UInt16)*DATA);
    return 1;
} // send_rf_data //


/*!
    \brief Reads the status register.
    
    \param[out] status Returns the value in the status register.
    
    \return TRUE if the register was successfully read.
            FALSE if there was an error reading the status register.
*/
static BOOL read_status_reg(UInt16 * const status)
{
    UInt16 status_byte_stream;
    BOOL rv;

    if(!status)
    {
        return FALSE;
    } // if the parameter is invalid //

    SSNOT = 0;
    rv = spi_transfer(0, 0, (UInt8 * const)&status_byte_stream,
      sizeof(status_byte_stream));
    SSNOT = 1;
    
    *status = one_net_byte_stream_to_int16(
      (const UInt8 * const)&status_byte_stream);
    return rv;
} // read_status_reg //


/*!
    \brief Write to a configuration register.
    
    The register and value are passed as 1 16bit value that is sent to the IA.
    Register addresses are not always the same size.
    
    \param[in] REG The register data to send to the IA.
    
    \return void
*/
static BOOL write_reg(const UInt16 REG)
{
    UInt16 reg_val;                 // byte stream register value to write
    BOOL rv;
    
    if(!REG)
    {
        return FALSE;
    } // if the parameters are invalid //

    one_net_int16_to_byte_stream(REG, (UInt8 * const)&reg_val);
    SSNOT = 0;
    rv = spi_transfer((const UInt8 * const)&reg_val, sizeof(reg_val), 0, 0);
    SSNOT = 1;
    
    return rv;
} // write_reg //

//! @} IA_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} IA
