//! \defgroup ADI ADI ADF7025 driver.
//! \ingroup TAL
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
    \file adi.c
    \brief Driver for the ADI ADF7025.

    This file implements the communications necessary for communicating with the
    ADI ADF7025.  It abstracts the actual processor pin assignments using
    definitions found in io_port_mapping.h.  This file also contains the
    implementation for the transceiver specific functions.
*/

#include "config_options.h"
#include "tal.h"
#include "tal_adi.h"
#include "hal_adi.h"
#include "io_port_mapping.h"
#include "one_net_status_codes.h"
#include "one_net_channel.h"
#include "one_net_data_rate.h"
#include "one_net_features.h"
#include "one_net_packet.h"
#include "tick.h"
#include "one_net_port_specific.h"
#ifdef UART
#include "cb.h"
#endif
#include "one_net_encode.h"
#ifdef HAS_LEDS
    #include "one_net_led.h"
#endif
#ifdef DEBUGGING_TOOLS
    #include "one_net_timer.h"
    #include "oncli.h"
#endif
#include "one_net.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ADI_const
//! \ingroup ADI
//! @{



enum
{
    NUM_INIT_REGS = 9,              //!< The number of registers to initialize
    REG_SIZE = 4,                   //!< Number of bytes in each register

    //! If the RSSI is below this level, the channel is clear
    RSSI_CLR_LEVEL = -75,
};

// transmit/receive register values
enum
{
    //! The byte index into the register to switch the transceiver between
    //! transmit and receive mode.
    TX_RX_BYTE_IDX = 0x00,

    //! The byte index into the register where the address nibble is
    ADDR_IDX = 3,

    //! Mask out the tx/rx bit
    TX_RX_MASK = 0xf7,

    //! Set the transceiver to transmit
    TX_RX_TRANSMIT = 0x00,

    //! Set the transceiver to receive
    TX_RX_RECEIVE = 0x08
};

/*!
    \brief Initial register values for ADF7025

    Output Frequency  =     918.500 MHz
    Deviation         = +/- 240.000 kHz
    Bit Rate          =      38.400 kBaud
    Crystal Frequency =      22.118 MHz
*/
const UInt8 INIT_REG_VAL[NUM_INIT_REGS][REG_SIZE] =
{
#ifdef US_CHANNELS
    {0x79, 0x4C, 0x36, 0x40},
#endif //US_CHANNELS
#ifdef EUROPE_CHANNELS
   #ifndef US_CHANNELS
    {0x79, 0x39, 0x26, 0xB0},       // european channel 0 only if US channels not included
   #endif //US_CHANNELS
#endif //EUROPE_CHANNELS
#if defined(_CLOCK_OUT_DIVIDE_BY_TWO)
    {0x00, 0xbc, 0x91, 0x11},       // clockout divide by 2 for board testing
#else
    {0x10, 0xBC, 0x91, 0x11},       // configuration before DO_CLOCK_OUT_DIVIDE_BY_TWO was added
#endif
    {0x80, 0x59, 0x7E, 0x32},
    {0x00, 0xDD, 0x09, 0xA3},
    {0x01, 0x00, 0x04, 0x44},
    
    // note that {0x55, 0x55, 0x33} are the last 3 bytes of the preamble / header
    // Every transceiver handles things differently.  The ADI only needs three bytes to sync,
    // But others need more.
    {0x55, 0x55, 0x33, 0x35},
    {0x1B, 0xA8, 0x00, 0xC6},
    {0x1B, 0xA0, 0x00, 0xC6},
    {0x00, 0x02, 0x78, 0xF9}
};

//! The register settings for setting the desired base frequency.
const UInt8 CHANNEL_SETTING[ONE_NET_NUM_CHANNELS][REG_SIZE] =
{
#ifdef US_CHANNELS
    {0x79, 0x46, 0x9B, 0x20},       // channel=  US1, frequency= 903.0 MHz
    {0x79, 0x46, 0xF7, 0xB0},       // channel=  US2, frequency= 904.0 MHz
    {0x79, 0x47, 0x54, 0x50},       // channel=  US3, frequency= 905.0 MHz
    {0x79, 0x47, 0xB0, 0xE0},       // channel=  US4, frequency= 906.0 MHz
    {0x79, 0x48, 0x0D, 0x80},       // channel=  US5, frequency= 907.0 MHz
    {0x79, 0x48, 0x6A, 0x10},       // channel=  US6, frequency= 908.0 MHz
    {0x79, 0x48, 0xC6, 0xB0},       // channel=  US7, frequency= 909.0 MHz
    {0x79, 0x49, 0x23, 0x40},       // channel=  US8, frequency= 910.0 MHz
    {0x79, 0x49, 0x7F, 0xE0},       // channel=  US9, frequency= 911.0 MHz
    {0x79, 0x49, 0xDC, 0x70},       // channel= US10, frequency= 912.0 MHz
    {0x79, 0x4A, 0x39, 0x10},       // channel= US11, frequency= 913.0 MHz
    {0x79, 0x4A, 0x95, 0xA0},       // channel= US12, frequency= 914.0 MHz
    {0x79, 0x4A, 0xF2, 0x40},       // channel= US13, frequency= 915.0 MHz
    {0x79, 0x4B, 0x4E, 0xD0},       // channel= US14, frequency= 916.0 MHz
    {0x79, 0x4B, 0xAB, 0x70},       // channel= US15, frequency= 917.0 MHz
    {0x79, 0x4C, 0x08, 0x00},       // channel= US16, frequency= 918.0 MHz
    {0x79, 0x4C, 0x64, 0xA0},       // channel= US17, frequency= 919.0 MHz
    {0x79, 0x4C, 0xC1, 0x30},       // channel= US18, frequency= 920.0 MHz
    {0x79, 0x4D, 0x1D, 0xC0},       // channel= US19, frequency= 921.0 MHz
    {0x79, 0x4D, 0x7A, 0x60},       // channel= US20, frequency= 922.0 MHz
    {0x79, 0x4D, 0xD6, 0xF0},       // channel= US21, frequency= 923.0 MHz
    {0x79, 0x4E, 0x33, 0x90},       // channel= US22, frequency= 924.0 MHz
    {0x79, 0x4E, 0x90, 0x20},       // channel= US23, frequency= 925.0 MHz
    {0x79, 0x4E, 0xEC, 0xC0},       // channel= US24, frequency= 926.0 MHz
    {0x79, 0x4F, 0x49, 0x50},       // channel= US25, frequency= 927.0 MHz
#endif
#ifdef EUROPE_CHANNELS
    {0x79, 0x39, 0x26, 0xB0},       // channel= EUR1, frequency= 865.8 MHz
    {0x79, 0x39, 0x67, 0x80},       // channel= EUR2, frequency= 866.5 MHz
    {0x79, 0x39, 0xA8, 0x50}        // channel= EUR3, frequency= 867.2 MHz
#endif
};


/*!
    \brief The register settings for setting the desired data rate.

    These values need to coincide with the data rates to be supported.  See
    data_rate_t in one_net.h for data rates.  The values to set the data rate
    are based on XTAL == 22.1184 Mhz
*/
const UInt8 DATA_RATE_SETTING[ONE_NET_DATA_RATE_LIMIT][REG_SIZE] =
{
    {0x00, 0xdd, 0x09, 0xa3},        // 38400 bps
    {0x00, 0xdd, 0x03, 0xe3},        // 76800 bps
    {0x00, 0xdd, 0x03, 0xa3},        // 115200 bps
    {0x00, 0x00, 0x00, 0x00},        // 153600  bps unachievable -- fill with 0
    {0x00, 0x00, 0x00, 0x00},        // 192000  bps unachievable -- fill with 0
    {0x00, 0xdd, 0x01, 0xe3},        // 230400  bps
};


//! Low battery voltage threshold.  This is used when reading the battery status
const UInt16 BATTERY_THRESHOLD = 0x003D;

//! Low voltage threshold.  This is used when reading the adcin status
const UInt16 VOLTAGE_THRESHOLD = 0x0028;



//! @} ADI_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ADI_typedefs
//! \ingroup ADI
//! @{

//! @} ADI_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ADI_pri_var
//! \ingroup ADI
//! @{

//! @} ADI_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================



//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ADI_pub_var
//! \ingroup ADI
//! @{


//! The current ONE-NET channel
UInt8 current_channel = 0;

//! The current data rate
UInt8 current_data_rate = ONE_NET_DATA_RATE_38_4;

//! number of bytes received that have been requested from ONE-NET code
UInt16 rx_rf_idx = 0;

//! number of bytes received from the transceiver (does not include Preamble /
//! Header.
UInt16 rx_rf_count = 0;

//! length of tx_rf_data
UInt16 tx_rf_len = 0;

//! index into tx_rf_data
UInt16 tx_rf_idx = 0;

//! Buffer to transmit data from the rf interface
const UInt8 * tx_rf_data;

//! Masks each bit in a byte in the interrupt routines when data is sent
//! or received.
UInt8 bit_mask = 0;

#ifdef UART
//! From uart.c.  Used by tal_write_packet to check whether the uart is
//! clear.
extern cb_rec_t uart_tx_cb;
#endif


//! @} ADI_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ADI_pri_func
//! \ingroup ADI
//! @{



static void write_reg(const UInt8 * const REG, const BOOL CLR_SLE);
static void turn_off_agc(void);
static void turn_on_agc(void);
static UInt16 adi_7025_read(const UInt8 * const MSG);
static UInt16 calc_rssi(const UInt16 READBACK_CODE);
static void tal_turn_on_receiver(void);
static void tal_turn_on_transmitter(void);



//! @} ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================



//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ADI_pub_func
//! \ingroup ADI
//! @{


UInt16 read_rssi(void);


//! @} ADI_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================





//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ADI_pub_func
//! \ingroup ADI
//! @{



/*!
    \brief Initializes the ADI transceiver.

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
    enum
    {
        CALIBRATION_REGISTER = 6,   //!< The calibration register
        REGISTER_MASK = 0x0F        //!< Mask to get register address
    };

    UInt8 msg_count;                   
    UInt8 calibration = 0;
    
    tal_init_ports();

    tal_enable_transceiver();

    // write registers after register 0
    for(msg_count = 0; msg_count < NUM_INIT_REGS; msg_count++)
    {
        write_reg(INIT_REG_VAL[msg_count], TRUE);

        // if we just programmed the calibration register delay 200 us to
        // allow the calibration to be performed, but don't delay when we
        // write the calibration register to turn calibration off.
        if((INIT_REG_VAL[msg_count][ADDR_IDX] & REGISTER_MASK)
          == CALIBRATION_REGISTER && !calibration)
        {
            // delay 200us
            delay_100s_us(2);
            calibration = 1;
        } // if this is the calibration register //
    } // for each register (or message) //

    init_rf_interrupts(ONE_NET_DATA_RATE_38_4); // initialize to lowest data
                      // rate.  It can be changed to something higher later.
} // tal_init_transceiver //


void tal_enable_transceiver(void)
{
    #ifdef CHIP_ENABLE
    CHIP_ENABLE_PIN = 1;
    #endif
} // tal_enable_transceiver //


void tal_disable_transceiver(void)
{
    #ifdef CHIP_ENABLE
    CHIP_ENABLE_PIN = 0;
    #endif
} // tal_disable_transceiver //


BOOL tal_channel_is_clear(void)
{
    return (SInt16)read_rssi() < RSSI_CLR_LEVEL;
} // tal_channel_is_clear //


UInt8 tal_write_packet(const UInt8 * data, const UInt8 len)
{
    #ifdef UART
    BOOL uart_pause_needed = FALSE;
    #endif
    
    #ifdef DEBUGGING_TOOLS
    if(pause || ratchet || write_pause)
    {
        proceed = FALSE;
        synchronize_last_tick();
        
        #if DEBUG_VERBOSE_LEVEL > 2
        if(verbose_level > 2)
        {
            oncli_send_msg("\n\n%lu sending %u bytes:\n",
              TICK_TO_MS(get_tick_count()), len);
        }
        #endif
        
        #if DEBUG_VERBOSE_LEVEL > 5
        if(verbose_level > 5)
        {
            display_pkt(data, len, NULL, 0, NULL, 0);
        }
        #endif
        #if DEBUG_VERBOSE_LEVEL > 2
        if(verbose_level <= 5 && verbose_level > 2)
        {
            xdump(data, len);
        }
        #endif
        #if DEBUG_VERBOSE_LEVEL > 1
        if(verbose_level == 2)
        {
            UInt16 raw_pid;
            if(get_raw_pid(&data[ON_ENCODED_PID_IDX], &raw_pid))
            {
                oncli_send_msg("\n\nWrite Raw PID 0x%02X\n", raw_pid);
            }            
        }
        #endif
    }
    
    while(pausing = (pause || (ratchet && !proceed)))
    {
        synchronize_last_tick();
        oncli();
    }
    proceed = FALSE;
    
    if(write_pause > 0)
    {
        pausing = TRUE;
        ont_set_timer(WRITE_PAUSE_TIMER, write_pause);
        
        while(!ont_inactive_or_expired(WRITE_PAUSE_TIMER))
        {
            oncli();  // alow the user to enter commands while pausing
        }
        #if DEBUG_VERBOSE_LEVEL > 1
        if(verbose_level > 1)
        {
            oncli_send_msg("Pause done\n");
        }
        #endif
        pausing = FALSE;
    }
    #endif    
    
    #ifdef WRITE_PAUSE
        #if WRITE_PAUSE_FACTOR > 0
        {
            tick_t write_tick = get_tick_count() + MS_TO_TICK(WRITE_PAUSE_FACTOR);
            while(get_tick_count() < write_tick)
            {
            }
        }
        #endif
    #endif
    
    
    tx_rf_idx = 0;
    tx_rf_data = data;
    tx_rf_len = len;

    #ifdef UART
    while(cb_bytes_queued(&uart_tx_cb))
    {
        uart_pause_needed = TRUE;
    }
    if(uart_pause_needed)
    {
        #ifdef DEBUGGING_TOOLS
        pausing = TRUE;
        #endif
        delay_ms(2); // slight pause to let the uart clear so nothing
                     // gets garbled.
        #ifdef DEBUGGING_TOOLS
        pausing = FALSE;
        #endif
    }
    #endif //  if UART is enabled //

    tal_turn_on_transmitter();
    ENABLE_TX_BIT_INTERRUPTS();

    return len;
} // tal_write_packet //


BOOL tal_write_packet_done()
{
    if(tx_rf_idx < tx_rf_len)
    {
        return FALSE;
    } // if not done //

    DISABLE_TX_BIT_INTERRUPTS();
    tal_turn_on_receiver();

    return TRUE;
} // tal_write_packet_done //


UInt8 tal_read_bytes(UInt8 * data, const UInt8 len)
{
    UInt8 bytes_to_read;
    
    // check the parameters, and check to see if there is data to be read
    if(!data || !len || rx_rf_idx >= rx_rf_count)
    {
        return 0;
    } // if the parameters are invalid, or there is no more data to read //
    
    if(rx_rf_idx + len > rx_rf_count)
    {
        // more bytes have been requested than are available, so give the
        // caller what is available
        bytes_to_read = rx_rf_count - rx_rf_idx;
    } // if more by requested than available //
    else
    {
        bytes_to_read = len;
    } // else read number of bytes requested //
    
    // TODO -- this shouldn't actualy be moving anywhere?
    one_net_memmove(data, &(encoded_pkt_bytes[ONE_NET_PREAMBLE_HEADER_LEN +
      rx_rf_idx]), bytes_to_read);
    rx_rf_idx += bytes_to_read;
    
    return bytes_to_read;
}


one_net_status_t tal_look_for_packet(tick_t duration)
{
    UInt8 blks_to_rx = ON_MAX_ENCODED_PKT_SIZE;

    tick_t end = get_tick_count() + duration;

    tal_turn_on_receiver();   

    rx_rf_count = 0;
    rx_rf_idx = 0;

    while(!SYNCDET)
    {   
        if(get_tick_count() >= end)
        {
		    return ONS_TIME_OUT;
        } // if done looking //
    } // while no sync detect //
    
    #ifdef HAS_LEDS
    set_rx_led(TRUE);
    #endif
    ENABLE_RX_BIT_INTERRUPTS();

    do
    {
        // the id length should be the location in the byte stream where the
        // PID will be received.  Look for the PID so we know how many bytes
        // to receive.
        if(rx_rf_count == ON_ENCODED_PID_IDX - ONE_NET_PREAMBLE_HEADER_LEN +
          ON_ENCODED_PID_SIZE)
        {
            // All packet size constants below are including the PREAMBLE &
            // SOF.  Since these cause the sync detect, these won't be read
            // in, so the packet size that is being read in is shorter, so
            // subtract the ON_ENCODED_DST_DID_IDX since that is where the
            // read is being started.
            UInt16 raw_pid;
            if(!get_raw_pid(&encoded_pkt_bytes[ON_ENCODED_PID_IDX], &raw_pid))
            {
                DISABLE_RX_BIT_INTERRUPTS();
                #ifdef HAS_LEDS
                set_rx_led(FALSE);
                #endif                
                return ONS_BAD_ENCODING;
            }
              
            blks_to_rx = get_encoded_packet_len(raw_pid, FALSE);
            if(blks_to_rx == 0)
            {
                // bad packet type
                DISABLE_RX_BIT_INTERRUPTS();
                #ifdef HAS_LEDS
                set_rx_led(FALSE);
                #endif
                return ONS_BAD_PKT_TYPE;
            }
        } // if PID read //
    } while(rx_rf_count < blks_to_rx);

    DISABLE_RX_BIT_INTERRUPTS();
    #ifdef HAS_LEDS
    set_rx_led(FALSE);
    #endif
    return ONS_SUCCESS;
}


one_net_status_t tal_set_data_rate(UInt8 data_rate)
{
    one_net_status_t status;
    
    #ifndef DATA_RATE_CHANNEL
    if(data_rate != ONE_NET_DATA_RATE_38_4)
    {
        return ONS_DEVICE_NOT_CAPABLE;
    }
    #else
    
    if(!features_data_rate_capable(THIS_DEVICE_FEATURES, data_rate))
    {
        return ONS_DEVICE_NOT_CAPABLE;
    }
    #endif
    
    current_data_rate = data_rate;
    
    if((status = init_rf_interrupts(current_data_rate)) == ONS_SUCCESS)
    {
        write_reg(DATA_RATE_SETTING[current_data_rate], TRUE);
    } // if the data rate is valid //    
    
    return status;
}


one_net_status_t tal_set_channel(const UInt8 channel)
{
    if(channel < ONE_NET_NUM_CHANNELS)
    {
        // see config_options.h.  Make sure CHANNEL_OVERRIDE and
        // CHANNEL_OVERRIDE_CHANNEL are defined if overriding the channel
        // parameter here.  Make sure CHANNEL_OVERRIDE is NOT defined if
        // NOT overriding the channel.
        #ifdef CHANNEL_OVERRIDE
        current_channel = CHANNEL_OVERRIDE_CHANNEL
        #else
        current_channel = channel;
        #endif
        return ONS_SUCCESS;
    } // if the parameter is valid //
    
    return ONS_BAD_PARAM;
} // tal_set_channel //


/*!
    \brief Returns the RSSI reported by the ADI in dBm.

    \param void

    \return The RSSI reading from the ADI in dBm.
*/
UInt16 read_rssi(void)
{
    const UInt8 MSG[REG_SIZE] = {0x00, 0x00, 0x01, 0x47};

    return calc_rssi(adi_7025_read(MSG));
} // read_rssi //


/*!
    \brief Returns the battery status reported by the ADI

    \param void

    \return The battery status.
*/
UInt16 read_battery(void)
{
    return 0;
} // read_battery //


/*!
    \brief Returns the adc value reported by the ADI

    Returns the ADC reading on the ADCIN pin.

    \param void

    \return The adc reading.
*/
UInt16 read_adc(void)
{
    const UInt8 MSG[REG_SIZE] = {0x00, 0x00, 0x01, 0x77};

    UInt16 adc;

    turn_off_agc();
    adc = adi_7025_read(MSG) & 0x007F;
    turn_on_agc();

    return adc;
} // read_adc //


UInt16 read_revision(void)
{
    return 0;
} // read_revision //



//! @} ADI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ADI_pri_func
//! \ingroup ADI
//! @{


/*!
    \brief Writes the 4 byte register value.
    
    Sets up the lines before and sends the register data.  After the write is
    complete, the SLE line is set.  Optionionally clears the SLE based on
    CLR_SLE.
    
    \param[in] REG The 4 bytes to write out.
    \param[in] CLR_SLE TRUE if write_reg should clear the SLE line before
                         returning.
                       FALSE if write_reg should leave SLE high

    \return void
*/
static void write_reg(const UInt8 * const REG, const BOOL CLR_SLE)
{
    UInt8 byte_count;
    UInt8 mask;

    SCLK = 0;
    SDATA = 0;
    SLE = 0;

    for(byte_count = 0; byte_count < REG_SIZE; byte_count++)
    {
        for(mask = 0x80; mask; mask >>= 1)
        {
            SDATA  = ((REG[byte_count] & mask) != 0);
            SCLK   = 1;
            SCLK   = 0;
        } // loop through the bits in the byte //
    } // loop through the bytes //
    
    SLE = 1;
    if(CLR_SLE)
    {
        SLE = 0;
    } // if clear the SLE flag //    
} // write_reg //


/*!
    \brief Turns off the agc.

    \param void

    \return void
*/
static void turn_off_agc(void)
{
} // turn_off_agc //


/*!
    \brief Turns on the agc

    \param void

    \return void
*/
static void turn_on_agc(void)
{
} // turn_on_agc //


/*!
    \brief Reads information from the ADI.

    Write the four bytes passed in the array to the registers, then reads two
    back from the transceiver.

    \param[in] MSG The information to read.

    \return The information read.
*/
static UInt16 adi_7025_read(const UInt8 * const MSG)
{
    enum
    {
        NUM_RESPONSE_BYTE = 2
    };

    UInt8 resp_byte[NUM_RESPONSE_BYTE];
    UInt8 byte_count, bit;

    write_reg(MSG, FALSE);

    // manual ignore the first clock cycle.
    SCLK   = 1;
    SCLK   = 0;
    
    // read the 2 byte response
    for(byte_count = 0; byte_count < NUM_RESPONSE_BYTE; byte_count++)
    {
        resp_byte[byte_count] = 0;
        for(bit = 0; bit < 8; bit++)
        {
            SCLK   = 1;
            SCLK   = 0;
            resp_byte[byte_count] = (resp_byte[byte_count] << 1) | SREAD;
        } // loop to read in each bit of the byte //
    } // loop to read the bytes //

    SLE = 0;
    return (((UInt16)resp_byte[0] << 8) | (UInt16)resp_byte[1]);
} /* adi_7025_read */


/*!
    \brief Calculates the rssi.
    
    The value passed in should be the readback_code returned by the transceiver.
    This function will convert the read back code to a RSSI reading in dBm.

    \param[in] READBACK_CODE The value returned by the ADI.

    \return The RSSI value in dBm.
*/
static UInt16 calc_rssi(const UInt16 READBACK_CODE)
{
    UInt16 rssi = READBACK_CODE & 0x007f;

    // switching on a 4 bit nibble with bits (LG2 LG1 FG2 FG1)
    switch((READBACK_CODE >> 7) & 0x000F)
    {
        case 0x00:
        {
            rssi += 113;
            break;
        } // case 0000 //

        case 0x04:
        {
            rssi += 90;
            break;
        } // case 0100 //

        case 0x08:
        {
            rssi += 65;
            break;
        } // case 1000 //

        case 0x09:
        {
            rssi += 53;
            break;
        } // case 1001 //

        case 0x0A:
        {
            rssi += 17;
            break;
        } // case 1010 //

        default:
        {
            // don't add anything
            break;
        } // default case //
    } // switch on LNA & Filter gain //

    return (rssi >> 1) - 98;
} // calc_rssi //


/*!
    \brief Sets the transceiver to receive mode

    \param void

    \return void
*/
static void tal_turn_on_receiver(void)
{
    UInt8 msg[REG_SIZE];

    // copy register 0 values for the current channel
    one_net_memmove(msg, CHANNEL_SETTING[current_channel], sizeof(msg));

    // set transmit/receive bit to transmit
    // by clearing the TR1 bit
    msg[TX_RX_BYTE_IDX] &= TX_RX_MASK;   
    // set the TR1 bit to receive
    msg[TX_RX_BYTE_IDX] |= TX_RX_RECEIVE; 

    write_reg(msg, TRUE);
    RF_DATA_DIR = 0;                // set the data line to an input
} // tal_turn_on_receiver //


/*!
    \brief Sets the transceiver to transmit mode

    \param void

    \return void
*/
static void tal_turn_on_transmitter(void)
{
    UInt8 msg[REG_SIZE];

    // copy register 0 values for the current channel
    one_net_memmove(msg, CHANNEL_SETTING[current_channel], sizeof(msg));

    // set transmit/receive bit to transmit
    // clear the TR1 bit
    msg[TX_RX_BYTE_IDX] &= TX_RX_MASK;   
    // set the TR1 bit to transmit
    msg[TX_RX_BYTE_IDX] |= TX_RX_TRANSMIT; 

    write_reg(msg, TRUE);
    RF_DATA_DIR = 1;                // set the data line to an output
    
    // give the transmitter some time to be ready to transmit.
    delay_100s_us(1);   
} // tal_turn_on_transmitter //



//! @} ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ADI
