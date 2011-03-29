//! \defgroup ADI ADI ADF7025 driver.
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
    \file adi.c
    \brief Driver for the ADI ADF7025.

    This file implements the communications necessary for communicating with the
    ADI ADF7025.  It abstracts the actual processor pin assignments using
    definitions found in io_port_mapping.h.  This file also contains the
    implementation for the transceiver specific functions (such as
    look_for_packet, one_net_read, one_net_write) declared in
    one_net_port_specific.h.
*/

#include "config_options.h"

#include "hal_adi.h"
#include "io_port_mapping.h"
#include "one_net_port_specific.h"
#include "tal.h"

#if defined(_ONE_NET_DEBUG) 
    #include "oncli.h"
#endif

#if defined(_SNIFFER_FRONT_END)
    #include "uart.h"
#endif


// TODO: REMOVE THIS?
#ifndef ROM
    #define ROM const
#endif

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
    
    //! The base data rate in the list (see data_rate_t in one_net.h)
    BASE_DATA_RATE = ONE_NET_DATA_RATE_38_4
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

ROM UInt8 INIT_REG_VAL[NUM_INIT_REGS][REG_SIZE] =
{
#ifdef _US_CHANNELS
    {0x79, 0x4C, 0x36, 0x40},
#endif //_US_CHANNELS
#ifdef _EUROPE_CHANNELS
   #ifndef _US_CHANNELS
    {0x79, 0x39, 0x26, 0xB0},       // european channel 0 only if US channels not included
   #endif //_US_CHANNELS
#endif //_EUROPE_CHANNELS
#if defined(_CLOCK_OUT_DIVIDE_BY_TWO)
    {0x00, 0xbc, 0x91, 0x11},       // clockout divide by 2 for board testing
#else
    {0x10, 0xBC, 0x91, 0x11},       // configuration before DO_CLOCK_OUT_DIVIDE_BY_TWO was added
#endif
    {0x80, 0x59, 0x7E, 0x32},
    {0x00, 0xDD, 0x09, 0xA3},
    {0x01, 0x00, 0x04, 0x44},
    {0x55, 0x55, 0x33, 0x35},
    {0x1B, 0xA8, 0x00, 0xC6},
    {0x1B, 0xA0, 0x00, 0xC6},
    {0x00, 0x02, 0x78, 0xF9}
};

//! The register settings for setting the desired base frequency.
ROM UInt8 CHANNEL_SETTING[ONE_NET_NUM_CHANNELS][REG_SIZE] =
{
#ifdef _US_CHANNELS
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
#ifdef _EUROPE_CHANNELS
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
ROM UInt8 DATA_RATE_SETTING[ONE_NET_MAX_DATA_RATE - BASE_DATA_RATE + 1][REG_SIZE] =
{
    {0x00, 0xdd, 0x09, 0xa3}        // 38400 kbps
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

//! index into rx_rf_data
UInt16 rx_rf_idx = 0;

//! bytes currently in rx_rf_data
UInt16 rx_rf_count = 0;

#ifdef _SNIFFER_FRONT_END
UInt16 rx_rf_sent = 0;
#define SNIFFER_DURATION        5000
#define SNIFFER_LOOP_START_BYTE 0xF3
#define SNIFFER_VERSION_BYTE    0xF2
#define SNIFFER_CHANNEL_BYTE    0xF1
#define SNIFFER_SYNCDET_BYTE    0xF0
#define SNIFFER_BAD_PACKET_BYTE 0xF9

#define SNIFFER_VERSION_NUMBER  0x03
#endif

//! Buffer to receive data from the rf interface
#ifdef RECEIVE_TEST
    UInt8 rx_rf_data[255] = {0x00};
#else
    UInt8 rx_rf_data[ONE_NET_MAX_ENCODED_PKT_LEN] = {0x00};
#endif

#if ON_RF_TRANSFER == ON_INTERRUPT
    //! length of TX_RF_DATA
    UInt16 tx_rf_len = 0;

    //! index into TX_RF_DATA
    UInt16 tx_rf_idx = 0;

    //! Buffer to transmit data from the rf interface
    const UInt8 * TX_RF_DATA;

    //! Masks each bit in a byte in the interrupt routines when data is sent
    //! or received.
    UInt8 bit_mask = 0;
#endif // if ON_RF_TRANSFER == ON_INTERRUPT //

//! The current ONE-NET channel
static UInt8 current_channel = 0;

//! @} ADI_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ADI_pri_func
//! \ingroup ADI
//! @{

// read a register.  Considered protected and is derived by adi_dbg_code.c.
UInt16 adi_7025_read(const UInt8 * const MSG);

// write to a register
static void write_reg(const UInt8 * const REG, const BOOL CLR_SLE);

#if ON_RF_TRANSFER == ON_POLLED
    // This function is actually defined in pal_adi.c since it depends on the
    // timers and interrupts that have been assigned to the transceiver.
    void tx_byte(const UInt8 VAL);
#endif

// calculate dBm
static UInt16 calc_rssi(const UInt16 READBACK_CODE);

static void turn_off_agc(void);
static void turn_on_agc(void);

//! @} ADI_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
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
    
    TAL_INIT_PORTS();

    ENABLE_TRANSCEIVER();

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

    INIT_RF_INTERRUPTS();
} // tal_init_transceiver //


/*!
    \brief Sets the transceiver to receive mode

    \param void

    \return void
*/
void tal_turn_on_receiver(void)
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
void tal_turn_on_transmitter(void)
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
    const UInt8 MSG[REG_SIZE] = {0x00, 0x00, 0x01, 0x57};

    UInt16 battery;

    turn_off_agc();
    battery = adi_7025_read(MSG) & 0x007F;
    turn_on_agc();

    return battery;
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
    } // if the parameter is valid //
} // one_net_set_channel //


// returns TRUE if the channel is clear (declared in one_net_port_specific.h).
BOOL one_net_channel_is_clear(void)
{
    return (SInt16)read_rssi() < RSSI_CLR_LEVEL;
} // one_net_channel_is_clear //


// sets the data rate the transceiver operates at (see one_net_port_specefic.h).
void one_net_set_data_rate(const UInt8 DATA_RATE)
{
    if(DATA_RATE <= ONE_NET_MAX_DATA_RATE)
    {
        write_reg(DATA_RATE_SETTING[DATA_RATE - BASE_DATA_RATE], TRUE);
    } // if the data rate is valid //
} // one_net_set_data_rate //


#if ON_RF_TRANSFER == ON_POLLED

one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    tick_t end;

	UInt8 map;
    UInt8 blks_to_rx = ONE_NET_MAX_ENCODED_PKT_LEN;

    DISABLE_GLOBAL_INTERRUPTS();
    TAL_TURN_ON_RECEIVER();
    ENABLE_RX_BIT_INTERRUPTS();

    rx_rf_idx = 0;
    rx_rf_count = 0;

    end = one_net_tick() + DURATION;

    while(!SYNCDET)
    {
       	POLLED_TICK_UPDATE();

	    if(end < one_net_tick())
        {
    	    DISABLE_RX_BIT_INTERRUPTS();
            ENABLE_GLOBAL_INTERRUPTS();
		    return ONS_TIME_OUT;
        } // if timer expired //
    } // while no sync detect //

    for(; rx_rf_count < blks_to_rx; rx_rf_count++)
    {
        for(map = 0x80; map; map >>= 1)
        {            
            // wait for DCLK to go high to read the data
            while(!BIT_CLK);

    	    // read the data
    	    if(RF_RX_DATA)
            {
                rx_rf_data[rx_rf_count] |= map;
	        }
	        else
	        {
                rx_rf_data[rx_rf_count] &= ~map;
	        }

	        // check the timer interrupt here since these loops should be a lot
            // faster than a tick, and this is where we are doing the least
            // between clock edges, so this would be the place were we would
            // have the most time
	        POLLED_TICK_UPDATE();
	        while(BIT_CLK);
	    }

        // the id length should be the location in the byte stream where the
        // PID will be received.  Look for the PID so we know how many bytes
        // to receive
        if(rx_rf_count == ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX)
        {
            // figure out how much to read
            switch(rx_rf_data[rx_rf_count])
            {
                case MASTER_INVITE_NEW_CLIENT:
                {
                    blks_to_rx = 48; // TODO - isn't this missing a break?
                } // MASTER invite new CLIENT case //

                case ONE_NET_ENCODED_SINGLE_DATA_ACK:
                case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
                case ONE_NET_ENCODED_SINGLE_DATA_NACK:  // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_ACK:    // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_NACK:   // fall through
                case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
			#ifdef _ONE_NET_VERSION_2_X
			    case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:  // fall through	
                case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:   // fall through	
			#endif		
                {
					#ifdef _ONE_NET_VERSION_2_X
                        blks_to_rx = 26; /* As of 2.0, this is an encrypted packet */
					#else
					    blks_to_rx = 17;
					#endif
                    break;
                } // acks/nacks/keep alive case for data packets //

                case ONE_NET_ENCODED_SINGLE_TXN_ACK:    // fall through
                case ONE_NET_ENCODED_BLOCK_TXN_ACK:
                {
                    blks_to_rx = 15;
                    break;
                } // single/block txn ack case //

                case ONE_NET_ENCODED_SINGLE_DATA:       // fall through
                case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
                {
                    blks_to_rx = 26;
                    break;
                } // (repeat)single data case //
                
                case ONE_NET_ENCODED_BLOCK_DATA:        // fall through
                case ONE_NET_ENCODED_REPEAT_BLOCK_DATA: // fall through
                case ONE_NET_ENCODED_STREAM_DATA:
                {
                    blks_to_rx = 58;
                    break;
                } // (repeat)block, stream data case //
                
                case ONE_NET_ENCODED_DATA_RATE_TEST:
                {
                    blks_to_rx = 20;
                    break;
                } // data rate test case //

                default:
                {
		            ENABLE_GLOBAL_INTERRUPTS();
		            DISABLE_RX_BIT_INTERRUPTS();
                    return BAD_PKT_TYPE;
                    break;
                } // default case //
            } // switch(rx_rf_data[rx_rf_count]) //
        }
    } // while we need more bytes //

    ENABLE_GLOBAL_INTERRUPTS();
    DISABLE_RX_BIT_INTERRUPTS();

    return SUCCESS;
} // one_net_look_for_pkt //

#else

one_net_status_t one_net_look_for_pkt(const tick_t DURATION)
{
    // these constants represents how many bytes need to be read in for the
    // various groups of packets that are received.  The 
    enum
    {
        // Number of bytes - Preamble & SOF to receive for invite
        RX_INVITE_LEN = 48 - ONE_NET_ENCODED_DST_DID_IDX,

        // Number of bytes - Preamble & SOF to receive for single, block, and
        // stream data acks, nacks, and keep alives
		#ifdef _ONE_NET_VERSION_2_X
            RX_DATA_ACK_LEN = 26 - ONE_NET_ENCODED_DST_DID_IDX,
		#else
            RX_DATA_ACK_LEN = 17 - ONE_NET_ENCODED_DST_DID_IDX,
		#endif

        // Number of bytes - Preamble & SOF to receive for single, block, and
        // stream transaction acks.
        RX_TXN_ACK_LEN = 15 - ONE_NET_ENCODED_DST_DID_IDX,
        
        // Number of bytes - Preamble & SOF to receive for single data packets
        RX_SINGLE_DATA_LEN = 26 - ONE_NET_ENCODED_DST_DID_IDX,
        
        // Numbeer of bytes - Preamble & SOF to receive for block and stream
        // data packets
        RX_BLOCK_STREAM_DATA_LEN = 58 - ONE_NET_ENCODED_DST_DID_IDX,
        
        // Number of bytes - Preamble & SOF to receive for data rate packets
        RX_DATA_RATE_TEST_LEN = 20 - ONE_NET_ENCODED_DST_DID_IDX
    };

    UInt8 blks_to_rx = ONE_NET_MAX_ENCODED_PKT_LEN;

    tick_t end = one_net_tick() + DURATION;

    TAL_TURN_ON_RECEIVER();   

    rx_rf_count = 0;
    rx_rf_idx = 0;

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
        if(rx_rf_count == ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX
          + 1)
        {
            //
            // The purpose of the following code is to make the 1.5 version of ONE-NET
            // compatible with versions of ONE-NET after 1.5 when the new NACK with
            // reason field is implemented.
            //
            // This code looks for one of the four types of NACK with reason field packet
            // types and replaces the packet type in the receive buffer with the
            // corresponding NACK (without reason field) packet types.
            //
            // The remainder of the code in this funtion will only see one of the older 
            // NACK (without reason field) packet types. So, in the future if a board 
            // running this code communicates with a board that uses the new
            // NACK with reason, it will simply treat the NACK with reason as a
            // NACK (without reason field) so that synchronization of nonce fields
            // will still permit communications.
            //
			// Note - Versions after 2.0 do not make this conversion.
			//
			// TODO - decide when exactly we should do this conversion (i.e. what versions)
			//
#ifndef _ONE_NET_VERSION_2_X
            if (rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
              == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN)
            {
                rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
                  = ONE_NET_ENCODED_SINGLE_DATA_NACK;
            }
#ifndef _BLOCK_MESSAGES_ENABLED
            else if (rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
              == ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN)
            {
                rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
                  = ONE_NET_ENCODED_BLOCK_DATA_NACK;
            }
#endif
#ifdef _ONE_NET_MULTI_HOP
            else if (rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
              == ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN)
            {
                rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
                  = ONE_NET_ENCODED_MH_SINGLE_DATA_NACK;
            }
#endif
#if defined(_ONE_NET_MULTI_HOP) && defined(_BLOCK_MESSAGES_ENABLED)
            else if (rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
              == ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN)
            {
                rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX] 
                  = ONE_NET_ENCODED_MH_BLOCK_DATA_NACK;
            }
#endif
#endif

            // All packet size constants below are including the PREAMBLE &
            // SOF.  Since these cause the sync detect, these won't be read
            // in, so the packet size that is being read in is shorter, so
            // subtract the ONE_NET_ENCODED_DST_DID_IDX since that is where the
            // read is being started.
            switch(rx_rf_data[ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX])
            {
                case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
                {
                    blks_to_rx = RX_INVITE_LEN;
                    break;
                } // MASTER invite new CLIENT case //

                case ONE_NET_ENCODED_SINGLE_DATA_ACK:            // fall through
                case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE: // fall through
                case ONE_NET_ENCODED_SINGLE_DATA_NACK:           // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_ACK:             // fall through
                case ONE_NET_ENCODED_BLOCK_DATA_NACK:            // fall through
                case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:          // fall through
			#ifdef _ONE_NET_VERSION_2_X
			    case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:       // fall through
			    case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:
			#endif					
                {
                    blks_to_rx = RX_DATA_ACK_LEN;
                    break;
                } // acks/nacks/keep alive case for data packets //

                case ONE_NET_ENCODED_SINGLE_TXN_ACK:            // fall through
                case ONE_NET_ENCODED_BLOCK_TXN_ACK:
                {
                    blks_to_rx = RX_TXN_ACK_LEN;
                    break;
                } // single/block txn ack case //

                case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
                case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
                {
                    blks_to_rx = RX_SINGLE_DATA_LEN;
                    break;
                } // (repeat)single data case //
                
                case ONE_NET_ENCODED_BLOCK_DATA:                // fall through
                case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:         // fall through
                case ONE_NET_ENCODED_STREAM_DATA:
                {
                    blks_to_rx = RX_BLOCK_STREAM_DATA_LEN;
                    break;
                } // (repeat)block, stream data case //
                
                case ONE_NET_ENCODED_DATA_RATE_TEST:
                {
                    blks_to_rx = RX_DATA_RATE_TEST_LEN;
                    break;
                } // data rate test case //

                #ifdef _ONE_NET_MULTI_HOP
                    case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_INVITE_LEN + 1;
                        break;
                    } // MASTER invite new CLIENT case //

                    case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:    // fall through
                    case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE: // fall through
                    case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK:   // fall through
                    case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:     // fall through
                    case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK:    // fall through
                    case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:  // fall through
				#ifdef _ONE_NET_VERSION_2_X
				    case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN:   // fall through
				    case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN:
				#endif	
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_DATA_ACK_LEN + 1;
                        break;
                    } // acks/nacks/keep alive case for data packets //

                    case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
                    case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_TXN_ACK_LEN + 1;
                        break;
                    } // single/block txn ack case //

                    case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
                    case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_SINGLE_DATA_LEN + 1;
                        break;
                    } // (repeat)single data case //
                
                    case ONE_NET_ENCODED_MH_BLOCK_DATA:         // fall through
                    case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
                    case ONE_NET_ENCODED_MH_STREAM_DATA:
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_BLOCK_STREAM_DATA_LEN + 1;
                        break;
                    } // (repeat)block, stream data case //
                
                    case ONE_NET_ENCODED_MH_DATA_RATE_TEST:
                    {
                        // need to add 1 for multi-hop byte
                        blks_to_rx = RX_DATA_RATE_TEST_LEN + 1;
                        break;
                    } // data rate test case //
                #endif // ifdef _ONE_NET_MULTI_HOP //

                default:
                {
		            DISABLE_RX_BIT_INTERRUPTS();
                    return ONS_BAD_PKT_TYPE;
                    break;
                } // default case //
            } // switch on PID //
        } // if PID read //
    } while(rx_rf_count < blks_to_rx);

    DISABLE_RX_BIT_INTERRUPTS();

    return ONS_SUCCESS;
} // one_net_look_for_pkt //

#endif // #if/elif ON_RF_TRANSFER for one_net_write_done function //

#ifdef _SNIFFER_FRONT_END

void look_for_all_packets(void)
{
    // these constants represents how many bytes need to be read in for the
    // various groups of packets that are received.  The 
    enum
    {
        // Number of bytes - Preamble & SOF to receive for invite
        RX_INVITE_LEN = 48 - ONE_NET_ENCODED_DST_DID_IDX,

        // Number of bytes - Preamble & SOF to receive for single, block, and
        // stream data acks, nacks, and keep alives
		#ifdef _ONE_NET_VERSION_2_X
            RX_DATA_ACK_LEN = 26 - ONE_NET_ENCODED_DST_DID_IDX,
		#else
            RX_DATA_ACK_LEN = 17 - ONE_NET_ENCODED_DST_DID_IDX,
		#endif

        // Number of bytes - Preamble & SOF to receive for single, block, and
        // stream transaction acks.
        RX_TXN_ACK_LEN = 15 - ONE_NET_ENCODED_DST_DID_IDX,

        // NACK with reason field length       
		#ifdef _ONE_NET_VERSION_2_X
            RX_NACK_RSN_LEN = 26 - ONE_NET_ENCODED_DST_DID_IDX,
		#else
            RX_NACK_RSN_LEN = 18 - ONE_NET_ENCODED_DST_DID_IDX,
		#endif
        
        // Multi hop NACK with reason field length     
		#ifdef _ONE_NET_VERSION_2_X
            RX_NACK_MH_RSN_LEN = 27 - ONE_NET_ENCODED_DST_DID_IDX,
		#else
            RX_NACK_MH_RSN_LEN = 19 - ONE_NET_ENCODED_DST_DID_IDX,
		#endif
        
        // Number of bytes - Preamble & SOF to receive for single data packets
        RX_SINGLE_DATA_LEN = 26 - ONE_NET_ENCODED_DST_DID_IDX,
        
        // Numbeer of bytes - Preamble & SOF to receive for block and stream
        // data packets
        RX_BLOCK_STREAM_DATA_LEN = 58 - ONE_NET_ENCODED_DST_DID_IDX,
        
        // Number of bytes - Preamble & SOF to receive for data rate packets
        RX_DATA_RATE_TEST_LEN = 20 - ONE_NET_ENCODED_DST_DID_IDX
    };


    RX_LED = 0;
    TX_LED = 0;


    DISABLE_TX_INTR();
    ENABLE_TX_INTR();

    u0tbl = SNIFFER_LOOP_START_BYTE;
    while(!(BOOL)ti_u0c1);
    
    u0tbl = SNIFFER_VERSION_BYTE;
    while(!(BOOL)ti_u0c1);
    u0tbl = SNIFFER_VERSION_NUMBER;
    while(!(BOOL)ti_u0c1);

    u0tbl = SNIFFER_CHANNEL_BYTE;
    while(!(BOOL)ti_u0c1);
    u0tbl = current_channel;
    while(!(BOOL)ti_u0c1);

    while (1)
    {
        UInt8 blks_to_rx = ONE_NET_MAX_ENCODED_PKT_LEN;
		#ifdef _AUTO_MODE
        	UInt8 sw_mode_select_state = SW_MODE_SELECT;
		#endif
        UInt8 sw_addr_select1_state = SW_ADDR_SELECT1;
        UInt8 switch_channels = FALSE;
        UInt8 send_channel = FALSE;

        tick_t end = one_net_tick() + SNIFFER_DURATION;

        TAL_TURN_ON_RECEIVER();   
        TX_LED = 0;
        RX_LED = 1;

        rx_rf_count = 0;
        rx_rf_sent = 0;
        rx_rf_idx = 0;

        while(!SYNCDET)
        {   
            // TODO: RWM: maybe we should just wait forever listening for the SYNCDET?
            //if(one_net_tick() >= end)
            //{
            //    break;
            //} // if done looking //

#ifdef _AUTO_MODE
	        if(SW_MODE_SELECT != sw_mode_select_state)
            {
                switch_channels = TRUE;
                break;
            }
#endif
            if (SW_ADDR_SELECT1 != sw_addr_select1_state)
            {
                send_channel = TRUE;
                break;
            }

        } // while no sync detect //

        ENABLE_RX_BIT_INTERRUPTS();

        if (switch_channels == TRUE)
        {
            current_channel += 1;
            if (current_channel >= ONE_NET_NUM_CHANNELS)
            {
                current_channel = 0;
            }
            switch_channels = FALSE;
			
			#ifdef _AUTO_MODE
            	sw_mode_select_state = SW_MODE_SELECT;
			#endif
			
            u0tbl = SNIFFER_CHANNEL_BYTE;
            while(!(BOOL)ti_u0c1);
            u0tbl = current_channel;
            while(!(BOOL)ti_u0c1);
            delay_100s_us(200);
            continue;
        }

        if (send_channel == TRUE)
        {
            send_channel = FALSE;
			#ifdef _AUTO_MODE
            	sw_addr_select1_state = SW_MODE_SELECT;
			#endif
            u0tbl = SNIFFER_CHANNEL_BYTE;
            while(!(BOOL)ti_u0c1);
            u0tbl = current_channel;
            while(!(BOOL)ti_u0c1);
            delay_100s_us(200);
            continue;
        }

        TX_LED = 1;
        RX_LED = 0;
        
        //
        // send a byte to indicate that SYNCDET 
        //
        u0tbl = SNIFFER_SYNCDET_BYTE;

        do
        {
            //
            // see if we have read a full byte
            //
            if (rx_rf_count > rx_rf_sent)
            {
                // send the current byte out the UART
                u0tbl = rx_rf_data[rx_rf_sent];
                rx_rf_sent++;
            }

            // the id length should be the location in the byte stream where the
            // PID will be received.  Look for the PID so we know how many bytes
            // to receive.
            if(rx_rf_count == ONE_NET_ENCODED_PID_IDX - ONE_NET_ENCODED_DST_DID_IDX
              + 1)
            {
                // All packet size constants below are including the PREAMBLE &
                // SOF.  Since these cause the sync detect, these won't be read
                // in, so the packet size that is being read in is shorter, so
                // subtract the ONE_NET_ENCODED_DST_DID_IDX since that is where the
                // read is being started.
                switch(rx_rf_data[ONE_NET_ENCODED_PID_IDX
                  - ONE_NET_ENCODED_DST_DID_IDX])
                {
                    case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
                    {
                        blks_to_rx = RX_INVITE_LEN;
                        break;
                    } // MASTER invite new CLIENT case //

                    case ONE_NET_ENCODED_SINGLE_DATA_ACK:           // fall through
                    case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
                    case ONE_NET_ENCODED_SINGLE_DATA_NACK:          // fall through
                    case ONE_NET_ENCODED_BLOCK_DATA_ACK:            // fall through
                    case ONE_NET_ENCODED_BLOCK_DATA_NACK:           // fall through
                    case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
                    {
                        blks_to_rx = RX_DATA_ACK_LEN;
                        break;
                    } // acks/nacks/keep alive case for data packets //

                    case ONE_NET_ENCODED_SINGLE_TXN_ACK:            // fall through
                    case ONE_NET_ENCODED_BLOCK_TXN_ACK:
                    {
                        blks_to_rx = RX_TXN_ACK_LEN;
                        break;
                    } // single/block txn ack case //

                    case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
                    case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
                    {
                        blks_to_rx = RX_SINGLE_DATA_LEN;
                        break;
                    } // (repeat)single data case //
                    
                    case ONE_NET_ENCODED_BLOCK_DATA:                // fall through
                    case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:         // fall through
                    case ONE_NET_ENCODED_STREAM_DATA:
                    {
                        blks_to_rx = RX_BLOCK_STREAM_DATA_LEN;
                        break;
                    } // (repeat)block, stream data case //
                    
                    case ONE_NET_ENCODED_DATA_RATE_TEST:
                    {
                        blks_to_rx = RX_DATA_RATE_TEST_LEN;
                        break;
                    } // data rate test case //

                    case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
                    case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:
                    {
                        blks_to_rx = RX_NACK_RSN_LEN;
                        break;
                    }

                    #ifdef _ONE_NET_MULTI_HOP
                        case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_INVITE_LEN + 1;
                            break;
                        } // MASTER invite new CLIENT case //

                        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:    // fall through
                        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
                        case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK:   // fall through
                        case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:     // fall through
                        case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK:    // fall through
                        case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_DATA_ACK_LEN + 1;
                            break;
                        } // acks/nacks/keep alive case for data packets //

                        case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
                        case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_TXN_ACK_LEN + 1;
                            break;
                        } // single/block txn ack case //

                        case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
                        case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_SINGLE_DATA_LEN + 1;
                            break;
                        } // (repeat)single data case //
                    
                        case ONE_NET_ENCODED_MH_BLOCK_DATA:         // fall through
                        case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
                        case ONE_NET_ENCODED_MH_STREAM_DATA:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_BLOCK_STREAM_DATA_LEN + 1;
                            break;
                        } // (repeat)block, stream data case //
                    
                        case ONE_NET_ENCODED_MH_DATA_RATE_TEST:
                        {
                            // need to add 1 for multi-hop byte
                            blks_to_rx = RX_DATA_RATE_TEST_LEN + 1;
                            break;
                        } // data rate test case //

                        case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN:
                        case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN:
                        {
                            blks_to_rx = RX_NACK_MH_RSN_LEN;
                            break;
                        }

                    #endif // ifdef _ONE_NET_MULTI_HOP //

                    default:
                    {
                        DISABLE_RX_BIT_INTERRUPTS();
                        // return ONS_BAD_PKT_TYPE;
                        u0tbl = SNIFFER_BAD_PACKET_BYTE;
                        rx_rf_count = blks_to_rx+1;
                        //break;
                    } // default case //
                } // switch on PID //
            } // if PID read //

            //
            // see if we have read a full byte since the last time we checked
            //
            if (rx_rf_count > rx_rf_sent)
            {
                // send the current byte out the UART
                while(!(BOOL)ti_u0c1);
                u0tbl = rx_rf_data[rx_rf_sent];
                while(!(BOOL)ti_u0c1);
                rx_rf_sent++;
            }
        } while(rx_rf_count < blks_to_rx);

        //
        // send any remaining received bytes
        //
        while (rx_rf_count > rx_rf_sent)
        {
            // send the current byte out the UART
            while(!(BOOL)ti_u0c1);
            u0tbl = rx_rf_data[rx_rf_sent];
            while(!(BOOL)ti_u0c1);
            rx_rf_sent++;
        }

        DISABLE_RX_BIT_INTERRUPTS();

    } // while(1) //
} // look_for_all_packets //

#endif


UInt16 one_net_read(UInt8 * data, const UInt16 LEN)
{
    UInt16 bytes_to_read;
    
    // check the parameters, and check to see if there is data to be read
    if(!data || !LEN || rx_rf_idx >= rx_rf_count)
    {
        return 0;
    } // if the parameters are invalid, or there is no more data to read //
    
    if(rx_rf_idx + LEN > rx_rf_count)
    {
        // more bytes have been requested than are available, so give the
        // caller what is available
        bytes_to_read = rx_rf_count - rx_rf_idx;
    } // if more by requested than available //
    else
    {
        bytes_to_read = LEN;
    } // else read number of bytes requested //
    
    one_net_memmove(data, &(rx_rf_data[rx_rf_idx]), bytes_to_read);
    rx_rf_idx += bytes_to_read;
    
    return bytes_to_read;
} // one_net_read //


#if ON_RF_TRANSFER == ON_POLLED
    #error "one_net_write needs to be defined for a polled implementation"
#elif ON_RF_TRANSFER == ON_INTERRUPT

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
    tx_rf_idx = 0;
    TX_RF_DATA = DATA;
    tx_rf_len = LEN;

    #ifdef _ONE_NET_DEBUG
    #if 0   // turn on/off as memory is available
        if (DATA[14] == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN)
        {
            one_net_debug(ONE_NET_DEBUG_RF_WRITE, DATA, LEN);
        }
    #endif
    #endif

    TAL_TURN_ON_TRANSMITTER();
    ENABLE_TX_BIT_INTERRUPTS();

    return LEN;
} // one_net_write //

#endif // #if/elif ON_RF_TRANSFER for one_net_write function //


#if ON_RF_TRANSFER == ON_POLLED

BOOL one_net_write_done(void)
{
    #ifdef DEMO_MODE
        TOGGLE(TX_LED);
    #endif // idef demo_mode //
    return TRUE;
} // one_net_write_done //

#else

BOOL one_net_write_done(void)
{
    if(tx_rf_idx < tx_rf_len)
    {
        return FALSE;
    } // if not done //

    DISABLE_TX_BIT_INTERRUPTS();
    TAL_TURN_ON_RECEIVER();

    return TRUE;
} // one_net_write_done //

#endif // #if/elif ON_RF_TRANSFER for one_net_write_done function //

//! @} ADI_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ADI_pri_func
//! \ingroup ADI
//! @{

/*!
    \brief Reads information from the ADI.

    Write the four bytes passed in the array to the registers, then reads two
    back from the transceiver.  This function is considered protected, and is
    derived by adi_dbg_code.c.

    \param[in] MSG The information to read.

    \return The information read.
*/
UInt16 adi_7025_read(const UInt8 * const MSG)
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
    \brief Calculates the rssi.
    
    The value passed in should be the readback_code returned by the transceiver.
    This function will convert the read back code to a RSSI reading in dBm.

    \param[in] READBACK_CODE The value returned by the ADI.

    \return The RSSI value in dBm.
*/
UInt16 calc_rssi(const UInt16 READBACK_CODE)
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
    \brief Turns off the agc.

    \param void

    \return void
*/
static void turn_off_agc(void)
{
    const UInt8 AGC_OFF[REG_SIZE] = {0x00, 0x0A, 0x78, 0xF9};

    write_reg(AGC_OFF, TRUE);
} // turn_off_agc //


/*!
    \brief Turns on the agc

    \param void

    \return void
*/
static void turn_on_agc(void)
{
    const UInt8 AGC_ON[REG_SIZE] = {0x00, 0x02, 0x78, 0xF9};

    write_reg(AGC_ON, TRUE);
} // turn_on_agc //

//! @} ADI_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ADI
UInt16 read_revision(void)
{
    const UInt8 MSG[REG_SIZE] = {0x00, 0x00, 0x01, 0xf7};

    return adi_7025_read(MSG);
} // read_revision //

