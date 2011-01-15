#include <string.h>
#include <one_net_types.h>
#include <one_net.h>
#include <Tick.h>
#include <one_net_status_codes.h>
#include <ctype.h>
#include <sfr_r81B.h>
#include <demo.h>

#include "config_options.h"

/*
    Note from RFM on SPI speed : 
    When reading the RX FIFO, the internal access time is limited to Fxtal/4, or 2.5MHz, so you can only run the
    SPI (FIFO read ONLY!) at 2.5MHz max.  If you attempt to read it faster then you'll get lots of data errors.
    Writing internal registers and TX register can be done at full speed, which is around 20MHz. 
*/

/*
 * data and clock and pattern detect signals
 * DATIO is an output for transmitting and an input for receiving 
 */
#define DATAIO     p3_3   /* input data for rx bit bang, output data for tx */
#define TRC_DATA   p3_3   /* input data for rx, output data for tx */
#define DATAIODIR  pd3_3
#define DATACLK    p1_7   /* clk from TRC102 for bit banging read */

#define SDI        p3_7 /* CPU output to TRC102 SPI data input pin 1  */
#define SCK        p3_5 /* CPU output to TRC102 SPI data clock pin 2  */
#define CS         p3_4 /* CPU output to TRC102 SPI chip select pin 3 */
#define SDO        p1_6 /* TRC102 pin 4 SPI data output to CPU        */
#define FFIT       p1_7 /* RX FIFO interrupt from TRC102 pin 7        */
#define VDI        p1_0   /* Valid Data Indicator from TRC102 pin 16  */

//=============================================================================
//                              CONSTANTS

/* Derek_S 1/15/2011 - I count 27 channels here.  ONE-NET has 28 channels (25
   U.S. channels and 3 European channels.  This array seems one channel off.
   There are no comments to say which values go with which channels. */
/* TO-DO : Add comments that match each value to a ONE-NET channel/frequency */
/* TO-DO : Add _US_CHANNELS and _EUROPE_CHANNELS #define guards.  See config_options.h
   file. */
//! The values that need to be set into FREQ2, FREQ1, FREQ0 to get the desired
//! ONE-NET channel setting
static const UInt16 channel_setting[ONE_NET_NUM_CHANNELS] =
{
    0xA14D, 0xA1D3, 0xA258, 0xA2DD, 0xA363, 0xA3E8, 0xA46D, 0xA4F3, 0xA578,
    0xA5FD, 0xA683, 0xA708, 0xA78D, 0xA813, 0xA898, 0xA91D, 0xA9A3, 0xAA28,
    0xAAAD, 0xAB33, 0xABB8, 0xAC3D, 0xACC3, 0xAD48, 0xADCD, 0xAE53
};
   
//                              CONSTANTS END
//=============================================================================

static void write_reg(const UInt16 reg);          /* Normal 16-bit spi port write */
static char read_rx_fifo(void);
void sfr_init(void);
static void write_txdata_first(const UInt8 data);
static void write_txdata(const UInt8 data);

#pragma INTERRUPT timer_z
void timer_z(void) {
}

//! ONE-NET channel the device is currently using
static UInt8 channel = 13;

//! index into rx_rf_data
static UInt16 rx_rf_idx = 0;
//! bytes currently in rx_rf_data
static UInt16 rx_rf_count = 0;
//! Buffer to receive data from the rf interface
static UInt8 rx_rf_data[MAX_ENCODED_PKT_SIZE];

/*
 *  Defines initial set of registers sent to the TRC102 at startup.
 *  These values are common to all modes.
 *  Certain registers such as the power mgmt. reg. and config. reg. 
 *  are written after thisinitial set.
 */
static const UInt16 RegistersCfg[] = {

    /* AFA; mode on */
    0b1100010010110111,

    /* TX config.; positive polarity, +/-240 KHz deviation, max. power */
    0b1001100011110000,

    /* freq. setting; 915.5MHz */
    0b1010100000010011,
    //0b1010100110100011, /* dje: 918.5 MHz */

        /* recv. control; pin 16 is valid data out, fast valid data detect,
         * 400kHz baseband BW, 0dB LNA gain, RSSI -103dB
         */
  /*0b1001010000100000, */ /* fast valid data detect */
  /*0b1001010100100011,*/ /* dje med valid data detect, rssi = -85 dBm */
  0b1001010000100011, /* dje slow valid data detect, rssi = -85 dBm */
  /*0b1001011000100110,*/ /* dje slow valid data detect, bandwidth=67 */

        /* baseband; auto. clock recovery, slow clock recovery mode,
         * digital filter, data detect threshold 4
         */
  /*0b1100001010101100, */
    0b1100001010101110,    /* threshold = 6 */
  /*0b1100001010101010,*/ /* dje: change threshold to 2 */

        /* synch. char; (lower 8 bits is 0x33) */
    0b1100111000110011,

        /* data rate 38314 baud (about 0.2% error with respect to 38400) */
    0b1100011000001000,

        /* wake up period; not used */
    0b1110000000000000,
    
        /* duty cycle; not used */
    0b1100100000000000,

        /* batt. detect & clk out; clk out 10MHz, low batt. detect not used */
    0b1100000011100001,
        /* PLL config. (not on IA datasheet); clock buf. slew > 5MHz, 
         * crystal startup 1ms, phase detector delay enabled, dithering
         * disabled, PLL BW reduced
         */
    0b1100110001010111
};

/* NUM_CONFIG_REGS determined and calculated at compile time
 * not run time
 */
#define NUM_CONFIG_REGS (sizeof(RegistersCfg) / sizeof(RegistersCfg[0]))

/*!
    \brief  Get the current ONE_NET channel.

    This function is transceiver specific. The ONE-NET channel
    number is be used to configure the frequency 
    used by the transceiver.

    \return UInt8 channel_number An integer between 1 and 26.

*/
UInt8 one_net_get_channel(void)
{
    return channel + 1;
} /* one_net_get_channel */

/*!
    \brief  Sets the ONE_NET channel to be used.

    This function is transceiver specific. The ONE-NET channel
    number supplied will be used to configure the frequency 
    used by the transceiver.

    \param[in] channel_number An integer between 1 and 26.

*/
void one_net_set_channel(const UInt8 CHANNEL_NUMBER)
{
    if(CHANNEL_NUMBER > ONE_NET_NUM_CHANNELS)
    {
        return;
    }
    channel = CHANNEL_NUMBER;
    if(CHANNEL_NUMBER)
    {
        channel--;
    }
    
    write_reg(channel_setting[channel]);
} /* one_net_set_channel */

// Derek_S 1/15/2011 - Why do we have a one_net_mem_move function that simply calls
// one_net_memmove?  Why not just have the calling function call one_net_memmove
// instead and delete this function?  Leaving it as is for now.
void * one_net_memmove(void * dst, const void * SRC, const size_t LEN)
{
    one_net_memmove(dst, SRC, LEN);
}

tick_t one_net_tick(void)
{
    return TickCount;
}

void one_net_sleep(tick_t DURATION)
{
    set_cdt2(DURATION);
    while(!get_cdt2());
}

/*!
    \brief Waits up to MS milliseconds for receiption of a packet
    
    \param[in] DURATION Time in ticks to look for a packet
    \param[in] ID_LEN Length of the ADDR that is expected
    \return SUCCESS if a packet has been received
*/
one_net_status_t one_net_look_for_pkt(const tick_t DURATION, const UInt8 ID_LEN)
{
    
	UInt8 map;
    UInt8 blks_to_rx = MAX_ENCODED_PKT_SIZE;

        
    // enable receiver
    write_reg(0x8074); /* configuration: enable fifo         */
    write_reg(0x82d8); /* power management rcv enable        */
    write_reg(0xca19); /* reset fifo fill stuff              */
    write_reg(0xca1b); /* enable fifo with one bit threshold */

    rx_rf_idx = 0;
    rx_rf_count = 0;

    set_cdt1(DURATION);
    
    while (!VDI) {
        write_reg(0xca19);/* reset fifo fill stuff */
        write_reg(0xca1b); /* enable fifo */
        
	    if (get_cdt1()) {
            TURN_OFF(RX_LED);
            if(mode_select_value != MASTER)
            {
                TURN_OFF(TX_LED);
            }
		    return TIME_OUT;
        }
    }
        
    while (!FFIT) {
	    if (get_cdt1()) {
            TURN_OFF(RX_LED);
            if(mode_select_value != MASTER)
            {
                TURN_OFF(TX_LED);
            }
		    return TIME_OUT;
        }
    }
    
    // rest of this function copied from semtech_xe1205.c
    
    for(; rx_rf_count < blks_to_rx; rx_rf_count++)
    {
        rx_rf_data[rx_rf_count] = read_rx_fifo();
            
        // the id length should be the location in the byte stream where the
        // PID will be received.  Look for the PID so we know how many bytes
        // to receive
        if(rx_rf_count == ID_LEN)
        {
            if(mode_select_value == MASTER)
            {
                // use the SLAVE IDs since the MASTER is receiving pkts from
                // the SLAVE
                switch(rx_rf_data[rx_rf_count])
                {
                    case S_REQ_L_SINGLE:                // fall through
                    case S_REQ_H_SINGLE:                // fall through
                    case S_REQ_REPEAT_L_SINGLE:         // fall through
                    case S_REQ_REPEAT_H_SINGLE:
                    {
                        blks_to_rx = 3;
                        break;
                    }

                    case S_RESPOND_SINGLE_DATA:         // fall through
                    case S_SINGLE_DATA:
                    {
                        blks_to_rx = 22;
                        break;
                    }

                    case S_RESPOND_JOIN_NETWORK:
                    {
                        blks_to_rx = 11;
                        break;
                    }

                    default:
                    {
                       return BAD_PKT_TYPE;
                       break;
                    }
                }
           }
           else
           {
                // use the MASTER IDs since the SLAVE is receiving pkts from
                // the MASTER
                switch(rx_rf_data[rx_rf_count])
                {
                    case M_ACK:                 // fall through
                    case M_INVITE_REQ:          // fall through
                    case M_GRANT_SINGLE:
                    {
                        blks_to_rx = 11;
                        break;
                    }

                    case M_SINGLE_DATA:
                    {
                        blks_to_rx = 22;
                        break;
                    }

                    case M_TIME_BROADCAST:
                    {
                        blks_to_rx = 17;
                        break;
                    }
                        
                    case M_INVITE_NEW_SLAVE:
                    {
                        blks_to_rx = 43;
                        break;
                    }

                    default:
                    {
                        return BAD_PKT_TYPE;
                        break;
                    }
                }
            }
        }
    }
    
    write_reg(0xca19); /* reset fifo fill stuff */
    write_reg(0xca1b); /* enable fifo */
    
    TOGGLE(RX_LED);
    return SUCCESS;
}

/*!
    \brief Reads bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[out] data Byte array to store the receive data in.
    \param[in] LEN The number of bytes to receive (data is at least this long).
    \return The number of bytes read
*/
UInt16 one_net_read(UInt8 * data, const UInt16 LEN) {

    // copied from semtech_xe1205.c

    UInt16 bytes_to_read;
    
    // check the parameters, and check to see if there is data to be read
    if(!data || !LEN || rx_rf_idx >= rx_rf_count)
    {
        return 0;
    }
    
    if(rx_rf_idx + LEN > rx_rf_count)
    {
        // more bytes have been requested than are available, so give the
        // caller what is available
        bytes_to_read = rx_rf_count - rx_rf_idx;
    }
    else
    {
        bytes_to_read = LEN;
    }
    
    one_net_memmove(data, &(rx_rf_data[rx_rf_idx]), bytes_to_read);
    rx_rf_idx += bytes_to_read;
    
    return bytes_to_read;
}

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
    int i;
    
    TOGGLE(TX_LED);
    
    write_reg(0x80b4); /* tx setup with data register  */
    write_reg(0xb8ff); /* try a civilized start "idle" */
    write_reg(0x8278); /* tx enable                    */
    
    write_txdata_first(DATA[0]);
    for (i = 0;i < (LEN - 1);i++) {
        write_txdata(DATA[i + 1]);
    }

    // hack; give the transmitter some time to send the bits out
    write_txdata(0);
    write_txdata(0);
    write_txdata(0);

    write_reg(0x8034); /* disable tx */
    write_reg(0xca19); /* stop rx fifo and reset the sync byte detector */
    write_reg(0x8208); /* power management disable tx and rx            */

    return LEN;
}

void init_io(void) {

    // set port directions, 1 for output, 0 for input

    pd1=0x3C; // 0b000111100 bits 2, 3, 4 and 5 are set as output pins on Port 1  SEAN QN : shouldn't bit 5 be an input? (Roger says yes but leave as is for now)
    pd3=0xB0; /*bits 4, 5, 7 are outputs on port 3:CS, SCK, SDI */
    /* all port 4 bits are inputs */
    pd4 = 0;

    // do timer x initialization
    sfr_init();
}

void init_SPI(void) {
}

void init_transceiver(void) {

    UInt8 i;
    /* 
     * Write an initial set of register values to the TRC102, EXCEPT
     * we don't write the config. reg. or power mgmt. reg. until later
     * in main() when we've determined if we're acting as a TX or RX
     */
    for (i = 0; i < NUM_CONFIG_REGS; i++) {
        write_reg(RegistersCfg[i]);
    }
    
    DATAIO    = 1; /* nFFS for reading fifo (p3_3) */
    DATAIODIR = 1; /* Output direction for p3_3    */
}

void init_interrupts(void) {
    // Don't think we need anything here
}

void write_DI(UInt8 rval) {
    // Don't think we need anything here
}

void check_transceiver(BOOL dump)
{
}

UInt8 self_test( void )
{
}

void rf_test(void)
{
    enum
    {
        DATA_LEN = 15
    };

    UInt8 data[DATA_LEN] = {0x55, 0x55, 0x55, 0x33,
      0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xBC,
      0xB4, 0xB4};
    UInt8 i;

    // range testing
    if(mode_select_value == MASTER)
    {
        tick_t next_send_time = 0;

        write_reg(0x80b4); /* tx setup with data register  */
        write_reg(0xb8ff); /* try a civilized start "idle" */
        write_reg(0x8278); /* tx enable                    */
        while(1)
        {
            while(one_net_tick() < next_send_time);
            next_send_time = one_net_tick() + 16;
            one_net_write(data, DATA_LEN);
        }
    } // if master
    else
    {
        while(1)
        {
            if(one_net_look_for_pkt(32, 8) == SUCCESS)
            {
                enum
                {
                    RX_SIZE = 11
                };
            
                UInt8 rx_data[RX_SIZE];
                BOOL on = TRUE;

                TURN_ON(RX_LED);
                one_net_read(rx_data, RX_SIZE);

                for(i = 0; i < RX_SIZE; i++)
                {
                    if(rx_data[i] != data[i + 4])
                    {
                        on = FALSE;
                        break;
                    }
                }
                
                if(on)
                {
                    TURN_ON(RX_LED);
                }
                else
                {
                    TURN_OFF(RX_LED);
                }
            }
            else
            {
                TURN_OFF(RX_LED);
            }
        }
    } // else slave
}

/*
 *  Writes a 16 bit register value to the TRC102 SPI interface using manual
 *  bit banging of the I/O lines.
 *
 *  The address of the register is implicit in the upper bits of its 16 bit
 *  value (i.e. this function has no separate address argument).
 */
static void write_reg(const UInt16 reg) 
{
    UInt16 mask;
    
    CS  = 1;     /* CS is p3_4    */
    SCK = 0;
    
    CS  = 0; /* start of write */
    for (mask = 0x8000; mask; mask >>= 1) {
        /* data read by TRC102 from SDI as SCK transitions 0->1 */
        SDI = ((reg & mask) != 0); /* SDI is p3_7 */
        SCK = 1;
        SCK = 0;
    }
    CS = 1; /* end of write */
}

/*
 *  Reads single byte from the RX FIFO by toggling fsel (dataio)
 *  Monitors FFIT to get next bit. FIFO threshold was set for
 *  one bit, so we wait for the next one each time.
 */
static char read_rx_fifo(void) 
{
    
    char ret = 0;
    int i;
    SCK = 0;
    CS  = 1;    /* nSEL = 1 to TRC102 pin 3 */
    DATAIO = 0; /* nFFS = 0 to TRC102 pin 6 */
    

    /* 
     * Now read the data byte from the RX FIFO 
     *
     * Note that this assumes that this function will never be
     * called unless FFIT was 1. Therefore, the fifo has
     * started, and will shift in more bits until we reset
     * the fifo fill stuff. Therefore it's safe to wait for FFIT
     * for each bit.
     * I hate to repeat myself, but:
     * This is safe only if FFIT was 1 before calling this function.
     *
     */
     

    for (i = 0; i < 8; i++) {
        while (!FFIT)
            ;
        SCK = 1;
        ret = (ret << 1) | SDO;
        SCK = 0;
    }
    DATAIO = 1; /* nFFS to 1 for end of read */
    return ret; 
}

/* 
 * Write word to SPI transmit data register and keep cs low 
 * for subsequent byte writes
 */
static void write_txdata_first(const UInt8 data)
{
    UInt8 mask;
    UInt8 val = 0xb8; /* tx register command byte */
    
    CS  = 1;     /* CS is p3_4    */
    SCK = 0;
    
    CS  = 0; /* start of write */
    for (mask = 0x80; mask; mask >>= 1) {
        SDI = ((val & mask) != 0);
        SCK = 1;
        SCK = 0;
    }
    /* now write first data byte */
    for (mask = 0x80; mask; mask >>= 1) {
        SDI = ((data & mask) != 0); /* SDI is p3_7 */
        SCK = 1;
        SCK = 0;
    }
}

/* 
 * Write byte to SPI and keep cs low.
 *
 * First tx byte is written by write_txdata_first().
 * write_txdata() assumes that CS is low and a byte
 * has been written with a "normal" SPI write, except
 * that CS has been kept low.
 *
 * After that, successive bytes are written by write_txdata.
 */
static void write_txdata(const UInt8 data)
{
    UInt8 mask;
    while (!SDO)
        ;
    for (mask = 0x80; mask; mask >>= 1) {
        /* data read by TRC102 from SDI as SCK transitions 0->1 */
        SDI = ((data & mask) != 0); /* SDI is p3_7 */
        SCK = 1;
        SCK = 0;
    }
}
