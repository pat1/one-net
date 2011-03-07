//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file micrel.c
   \brief Hardware specific functions for the Micrel
    
   \author  Roger Meadows, Sean Machin
   \date $Date: 2005/04/27 08:37:39 $
   \version $Rev: 51 $
   \note Threshold Corporation

   \see lpw_definitions
   \todo
   Changes made since last checkin (9/27/06 v59)
   o - removed some debugging code from micrel.c (change_rf_regs and list_rf_regs).

*/ 

#include "demo.h"
#include "sp.h"
#include "string.h"
#include "sfr_r81B.h"
#include "transceiver.h"
#include "one_net_types.h"
#include "one_net_port_specific.h"
#include "rf_regs.h"
#include "micrel.h"
#include "chat.h"
#include "Tick.h"

#define RF_POWER_MASK       0x8e        // use & to clear Power bits and Load_en
#define RF_POWER_OFF        0x00
#define RF_POWER_LEVEL1     0x10
#define RF_POWER_LEVEL2     0x20
#define RF_POWER_LEVEL3     0x30
#define RF_POWER_LEVEL4     0x40
#define RF_POWER_LEVEL5     0x50
#define RF_POWER_LEVEL6     0x60
#define RF_POWER_LEVEL7     0x70

#define RF_MODE_MASK        0xf8        // use & to clear Mode bits and Load_en
#define RF_MODE_POWER_DOWN  0x00
#define RF_MODE_STANDBY     0x02
#define RF_MODE_RECEIVE     0x04
#define RF_MODE_TRANSMIT    0x06

//=============================================================================
//                                  CONSTANTS
//! \defgroup Threshold_One-Net_port_specific_const
//! \ingroup Threshold_One-Net_port_specific
//! @{
    
#define DEBUG_SPI_TRANSMIT 0
unsigned char index=0;
unsigned char data;

static UInt8 spi_write_failures = 0; 

//! The 32 bit pattern that shoudl appear after a preamble.
static UInt32 pattern = 0x55555533;

//! The last 32 bits of data we have received.
static UInt32 bits_32;

//! Flag to indicate if the sync pattern has been received over the rf interface.
static BOOL received_pattern = FALSE;

//! index into rx_rf_data
UInt16 rx_rf_idx = 0;
//! bytes currently in rx_rf_data
UInt16 rx_rf_count = 0;
//! Buffer to receive data from the rf interface
UInt8 rx_rf_data[MAX_ENCODED_PKT_SIZE];

static UInt8 data_to_send;

extern UInt8 spo_head;
extern UInt8 spo_tail;

#pragma INTERRUPT int1_isr
void int1_isr(void);

void set_cdt1( UInt16 MS );
UInt8 get_cdt1( void );
void turn_on_receiver( void );
void turn_on_transmitter( UInt8 power_level );
void turn_on_standby( void );
void write_register(UInt8 start_reg, const UInt8 * RVAL, UInt8 count,
  const BOOL COMMIT);
UInt8 read_register(UInt8 rnum);

void xtea_test( void );

void one_net_set_channel( const UInt8 );
UInt8 one_net_get_channel( void );

void init_interrupts(void)
{
    _asm("fclr i");
    
    ilvl0_int1ic=1; //set interrupt priority level to 1
    ilvl1_int1ic=1;
    ilvl2_int1ic=0;
    
//  int1ic_addr.bit.pol=1; //set ISR trigger on falling edge.
    r0edg=1;    
//  pol_int1ic=0;
    
    _asm("fset i");
    
    //ir_int1ic=1;      //enable interrupt request

} // init_interrupts //

/*******************************************************************************************************/
// Bit banging ISR routine
/*******************************************************************************************************/

void int1_isr(void)
{        
    
    if(!pd1_6){                 // data port direction flag is used to determine if currently a write or read is in progress.
        if(p1_6){
            data|=index;        // packing data as each bit comes in
        }
        index>>=1;
    }
    else{
        if(data&index){
            p1_6=1;
        }
        else{
            p1_6=0;
        }
        index>>=1;
    }
} // int1_isr //


// connect InitPorts to Renesas version
void init_io( void )
{
    // set port directions, 1 for output, 0 for input
	pd1=0x2C; // 0b000111100 bits 2, 3, 4 are set as output pins on Port 1  
    pd3=0xB0; // 0b010110000 CS, SCLK, IO (outputs 4, 5, 7 )
    pd3_4=1; // set CS high

    // p1_4 to p1_7 are configured as pulled up 
    // (SEAN : not sure why this is needed)?
    pu03=1; 
    
    // do timer x initialization
	sfr_init();
	
} // init_io //

    
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
    register UInt16 i;
    register UInt8 map;
    register UInt8 val;

    turn_on_transmitter( RF_POWER_LEVEL7 );
    disable_global_interrupts();

    TX_LED = 1;
    for(i = 0; i < LEN; i++)
    {
        val = *DATA++;
        for(map = 0x80; map; map >>= 1)
        {
            // look for a high pulse when we start, so we don't write when the clock
            // does go high in the time between the check & the write
            while(!BIT_CLK);

            // wait for clock to be low to write bit
            while(BIT_CLK);

            if(val & map)
            {
                RF_TX_DATA = 1;
            }
            else
            {
                RF_TX_DATA = 0;
            }
        }
    }

    enable_global_interrupts();

    TX_LED = 0;
    return LEN;
} /* one_net_write */

/*!
    \brief Sends the same byte out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer. It can be used in testing to check for a 
    fixed pattern from the transmitter and to the receiver.

    \param[in] byte The test pattern byte to send.

    \return void
*/
void one_net_write_test_pattern( UInt8 byte )
{
    register UInt16 i;
    register UInt8 map;
    register UInt8 val;

//    turn_on_transmitter( RF_POWER_LEVEL7 );
    disable_global_interrupts();

    TX_LED = 1;
    while( 1 )
    {
        for (i=0; i<8; i++)
        {
#ifdef MICREL_1s_AND_0s_TEST
            val = byte;
#else
            if ( i < 3 )
            {
                val = 0x55;
            }
            else if ( i == 3 )
            {
                val = 0x33;
            }
            else
            {
                val = 0xb4;
            }
#endif
            for(map = 0x80; map; map >>= 1)
            {
                // look for a high pulse when we start, so we don't write when the clock
                // does go high in the time between the check & the write
                while(!BIT_CLK);
                
                // wait for clock to be low to write bit
                while(BIT_CLK);
                
                if(val & map)
                {
                    RF_TX_DATA = 1;
                }
                else
                {
                    RF_TX_DATA = 0;
                }
            } // for map //
        } // for i //
    } // while 1 //

    enable_global_interrupts();

    return;
} /* one_net_write_test_pattern */


one_net_status_t one_net_look_for_pkt(const tick_t DURATION, const UInt8 ID_LEN)
{
    UInt16 rx_rf_ptr;
	UInt8 map;
    UInt32 ms = DURATION;

    #define BITS_PER_MS         38

    turn_on_receiver();
    disable_global_interrupts();

    set_cdt1(DURATION);
    received_pattern = FALSE;
    
    // first look for the pattern because the Micrel
    // transceiver does not do this for us
    bits_32 = 0;
    while(!received_pattern)
    {
        // collect bits looking for a sync pattern
        while(BIT_CLK);

        // wait for DCLK to go high to read the data
        while(!BIT_CLK)
        {
            // keep tick count up to date for possicle time out
            if(ir_txic)
            {
                TickCount++;
                
                // update count down timer one
                if ( cdt1 != 0 )
                {
                    --cdt1;
                }
                
                ir_txic = 0;
            }
        }

        // get a new bit and store it in bits_32
        bits_32 = (bits_32 << 1) | RF_RX_DATA;

        // see if we have received the pattern
        if ( bits_32 == pattern )
        {
            received_pattern = TRUE;
            break;
        }
       
        // see if we have run out of time waiting for pattern
	    if( get_cdt1() )
        {
            // timed out
            enable_global_interrupts();
		    return FALSE;
        }
    } // while(!received_pattern) //


    // once pattern is found, read the packet
    for(rx_rf_ptr = 0; rx_rf_ptr < MAX_ENCODED_PKT_SIZE + 1; rx_rf_ptr++)
    {
        for(map = 0x80; map; map >>= 1)
        {
            // wait for DCLK to go low so we don't read the same bit twice
            // (and we know we are seeing the first bit after the pattern
            // irq has gone high)
            while(BIT_CLK);
            // wait for DCLK to go high to read the data
            while(!BIT_CLK);

            // read the data
            if(RF_RX_DATA)
            {
                rx_rf_data[rx_rf_ptr ] |= map;
            }
            else
            {
                rx_rf_data[rx_rf_ptr] &= ~map;
            }
        }
    }
    enable_global_interrupts();

    return TRUE;
} /* one_net_look_for_pkt */

void turn_on_receiver( void )
{
    // register values starting at 0x0A
    const UInt8 reg_val [] = {0x0C, 0x00, 0x84, 0x00, 0x25, 0x0C, 0x00, 0x84,
      0x00, 0x25};

    rf_regs_current_r0 &= RF_MODE_MASK;
    rf_regs_current_r0 |= RF_MODE_RECEIVE;
    write_register(0x00, &rf_regs_current_r0, sizeof(rf_regs_current_r0),
      FALSE);

    // must set A, M and N register pairs (0 and 1) to the same value for RX
    // mode
    write_register(0x0a, reg_val, sizeof(reg_val), TRUE);

    // change the direction of the DATAIXO pin
    RF_SET_RX_DATA_DIRECTION();

} /* turn_on_receiver */

void turn_on_transmitter( UInt8 power_level )
{
    rf_regs_current_r0 &= RF_MODE_MASK;
    rf_regs_current_r0 |= RF_MODE_TRANSMIT;
    rf_regs_current_r0 &= RF_POWER_MASK;
    rf_regs_current_r0 |= power_level;
    write_register(0x00, &rf_regs_current_r0, sizeof(rf_regs_current_r0),
      TRUE);

    if ( wait_for_ld( 5000 ) == FALSE )
    {
        // timed out wating for LD, now what? TBD
        sp_write_rom_string( "Error: time out waiting for LD signal.\r\n", TRUE );
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms();
    }
    
    // change the direction of the DATAIXO pin
    RF_SET_TX_DATA_DIRECTION();

} /* turn_on_transmitter */

void turn_on_idle( void )
{

    // change the direction of the DATAIXO pin
    RF_SET_RX_DATA_DIRECTION();

    rf_regs_current_r0 &= RF_MODE_MASK;
    rf_regs_current_r0 |= RF_MODE_STANDBY;

    write_register(0x00, &rf_regs_current_r0, sizeof(rf_regs_current_r0),
      TRUE);

} /* turn_on_idle */

void set_pattern( UInt8 * data )
{

    pattern = *data++;
    pattern <<= 8;
    pattern |= *data++;
    pattern <<= 8;
    pattern |= *data++;
    pattern <<= 8;
    pattern |= *data++;

} /* set_pattern */

/*
   SPI initialization for the Micrel.
    SSI is sync. serial interface.
*/
void init_SPI(void)
{
    ssisel=0; //configure P3_4 (Micrel CS line) for SSI00 pin (SEAN : SSISEL 0 sets P3_3 for SSI00 pin, p26, but p3_5 goes to SCLK)
    iicsel=0; //IICSEL bit configured for sync serial (SEAN : clock sync. comms mode p.164)
    sser=0; //make sure transmit and receive are disabled (SEAN : verified, p.170, zeros disable TX, RX)
    ssmr=0x48; // p.169 MSB first, Clock polarity=L when stopped, Clock Phase on odd edge (SEAN : 0x48 = 01001000)
    ssmr2=0xc1;//11000001b; // p.172 single input/output pins, SSCK selected, P3_4 instead of SCS, BIDE Mode

    //  Setup SSCRH 
    mss_sscrh=1; //SPI master mode p.167
    // clock, 000 = osc/256, 110 = osc/4   p.167
    cks2_sscrh=1;
    cks1_sscrh=1;
    cks0_sscrh=0;
    rsstp_sscrh=1; //complete receive after 1 byte  p.167


    sssr=0; //initialize all flags p.171
    
    re_sser=0;  //disable receive    // Do not enable both transmit and receive in the BIDE mode. 
    te_sser=1;  //enable transmit
    
    rie_sser=0; // disable all Rx, Tx interupts
    teie_sser=0;
    tie_sser=0;
} /* init_SPI */

/*!
    \brief Write register values.
    
    The first value is written to start_reg.  Subsequent values are written to
    subsequent registers.
    
    \param[in] start_reg The first register that will be written.
    \param[in] RVAL The values to be written.
    \param[in] count The number of registers to write.  RVAL must point to at
      least this many values.

    \return void
*/
void basic_write_register(UInt8 start_reg, const UInt8 * RVAL,
  UInt8 count)
{
    if(RVAL && count)
    {
        // rwm 11/3/06, set_rf_reg( rnum, rval );
        if(start_reg == 0)
        {
            // save the most recent contents of register 0
            rf_regs_current_r0 = *RVAL;
        }

        // disable receive in BIDE mode since in this Fn call only Tx occurs
        re_sser = 0;
        te_sser = 1;                // enable transmit
        SPI_CS = 1;                 // enable CS (Chip Select)

        while(!tdre_sssr);          // wait until transmit buffer is clear
        if(orer_sssr)
        {
            // if overflow, clear all SPI flags
            sssr = 0;
        } // if overflow

        // write a value to register 0
        sstdr = start_reg << 1 | 0x0;

        for(; count; count--)
        {
            while(!tdre_sssr);      // wait until transmit buffer is clear
            sstdr = *RVAL;          // write this value
            RVAL++;
        }
    
        while(!tend_sssr);
        tend_sssr = 0;
        te_sser = 0;
        _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");

        //for(i=0; i<200; i++);
        // NSS_CONFIG disabled (p3_4 is Micrel CS line)
        SPI_CS = 0;
        //for (i=0;i<100;i++);
    }
} /* basic_write_register */


/*!
    \brief Sets the register values

    
    Sets the register values for 1 or more registers.  Optionally commits the
    settings by writing a 1 in bit 0 of reg 0.
    
    \param[in] start_reg The address of the first register to write
    \param[in] RVAL The value(s) to write to the register(s).
    \param[in] count The number of registers to write.  RVAL must point to at
      least this many values.
    \param[in] COMMIT TRUE if the transceiver should start using the new
      register settings.

    \return void
*/
void write_register(UInt8 start_reg, const UInt8 * RVAL, UInt8 count,
  const BOOL COMMIT)
{
    basic_write_register(start_reg, RVAL, count);
    if(COMMIT)
    {
        UInt8 reg0_val = rf_regs_current_r0 | 0x01;
        basic_write_register( 0x00, &reg0_val, sizeof(reg0_val));
    }
} /* write_register */


UInt8 read_register(UInt8 rnum)
{
      unsigned char rval;
      int i;
      
      re_sser=0;                    //disable Rx in BIDE mode initially while address Tx occurs
      _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");
      te_sser=1;                    //enable transmit
      _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");
        
        
    while(!tdre_sssr);      // wait till tx register set as empty

    
      SPI_CS=1;  // set chip select line to high ...//enable NSS_CONFIG
  
        sstdr=(rnum<<1)|0x1;
 
  while(!tend_sssr);        // wait final bit is currenlty being sent
        tend_sssr=0;

        
        te_sser=0;                  //disable transmit and enable receive here to allow data read from the peripheral
        _asm("nop");_asm("nop");_asm("nop");_asm("nop");
        re_sser=1;
        _asm("nop");_asm("nop");_asm("nop");_asm("nop");

      rval=ssrdr;                   // Dummy read to generate clock 
      
     
         if (orer_sssr) //if overflow, clear all SPI flags
                sssr=0;
 while (!rdrf_sssr); //wait for receive buffer to have data
        re_sser=0;
        _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");


      rval=ssrdr;  //p.173

      p1_5=0;

      for (i=0;i<200;i++);
      SPI_CS=0; // set chip select to low     //NSS_CONFIG disabled
      
        

      for (i=0;i<100;i++);
      return rval;
} /* read_register */


void write_all_registers( void )
{
    int i;
    UInt8 r1;
    UInt8 rnum, rval;

    re_sser=0;  //disable receive in BIDE mode since in this Fn call only Tx occurs
    te_sser=1;  //enable transmit
    
    
    SPI_CS=1;  //enable CS (Chip Select)

    while (!tdre_sssr); //wait until transmit buffer is clear
    if (orer_sssr) //if overflow, clear all SPI flags
        sssr=0;
    sstdr=0x00<<1|0x0;  //write address of register 0
                        // A6...A0 followed by RW (0 for writing)

    for ( rnum=0; rnum<rf_number_of_cfg_regs; rnum++ )
    {
        while (!tdre_sssr); //wait until transmit buffer is clear
        // rwm 11/3/06, sstdr=rf_regs_current[rnum]; //write this value
    }

    // wait until transmitting the last bit of transmit data
    while(!tend_sssr);
    tend_sssr=0;
    te_sser=0;
    _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");

    for (i=0;i<200;i++);
    // the following should result in a load-signal if the 
    // Load_en bit of register 0 is 1
    SPI_CS=0; //NSS_CONFIG disabled (p3_4 is Micrel CS line)
    for (i=0;i<100;i++);
    
} /* write_all_registers */

//*****************************************************************************
/*! \brief Returns the count of SPI write errors.

This function and incr_spi_write_errors can be used to track the 
number of times (up to 255) that we saw an error writing to the
XE1205 using the SPI interface.

\return void
*/
UInt8 get_spi_write_errors( void ){

    return( spi_write_failures );

} /* get_spi_write_errors */


//*****************************************************************************
/*! \brief Clears the count of SPI write errors.

This function sets the number of spi write errors to zero.
Both nibbles are cleared.

\return void
*/
void clear_spi_write_errors( void ){

    spi_write_failures = 0x00;

} /* clear_spi_write_errors */


//*****************************************************************************
/*! \brief Increments the count of SPI write errors.

This function and get_spi_write_errors can be used to track the 
number of times (up to 255) that we saw an error writing to the
XE1205 using the SPI interface.

\return void
*/
#if 0
    static void incr_spi_write_errors( void ){

        if ( (spi_write_failures & 0x0f) != 0x0f )
        {
            ++spi_write_failures;
        }

    } /* incr_spi_write_errors */
#endif

//*****************************************************************************
/*! \brief Initialize the XE1205.

The configuration values in the RegisterCfg data structure are programmed 
into the XE1205.

\return void
*/
void init_transceiver ( void )
{
    UInt16 i;
    const RF_REGS * reg_cfg;
    UInt8 val;
    
    // Initializes Micrel
    for(i = 0; i <= rf_number_of_cfg_regs - 1; i++)
    {
	    reg_cfg = &rf_regs_cfg[i];
// rwm 11/03/06,        rf_regs_current[i] = reg_cfg->initial_value;
        if(reg_cfg->initialize == TRUE)
        {
            val = (UInt8)reg_cfg->initial_value;
            write_register((UInt8)i, &val, sizeof(val),
              (const BOOL)(i == (rf_number_of_cfg_regs - 1)));
        }
    }

} /* init_transceiver */


BOOL wait_for_ld( UInt16 ms )
{

    set_cdt1( ms );
    
    // wait until we see the RF_LD
    while (!RF_LD )
    {
        if ( get_cdt1() == 1 )
        {
            return( FALSE );
        }
    }
    return( TRUE );

} /* wait_for_ld */

/*!
    \brief Copies LEN bytes from SRC to dst

    \param[out] dst The mem location to receive the bytes
    \param[in] SRC The bytes to copy.
    \param[in] LEN The number of bytes to copy.
    \return void
*/
void * one_net_memmove(void * dst, const void * SRC, const size_t LEN)
{
#if 1
    one_net_memmove(dst, SRC, LEN);
#else
    UInt8 * s;
    UInt8 * d;
    UInt16 len;

    len = LEN;
    s = SRC;
    d = (UInt8 *) dst;

    while ( *s != '\0' )
    {
        if ( len-- == 0 )
        {
            break;
        }
        *d++ = *s++;
    }
#endif
} // one_net_memmove //

/*!
    \brief Reads bytes out of the rf interface.

    This function is application specific and will need to be implemented by
    the application designer.

    \param[out] data Byte array to store the receive data in.
    \param[in] LEN The number of bytes to receive (data is at least this long).
    \return The number of bytes read
*/
UInt16 one_net_read(UInt8 * data, const UInt16 LEN)
{
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
    \brief Callback for the application to handle the received packet.

    This function is application dependent.  It is called by ONE-NET when it
    receives 
*/
void handle_single_pkt(const UInt8 * RX_PLD, const UInt16 RX_PLD_LEN,
 UInt8 * response, const UInt16 RESPONSE_SIZE)
{

}

/*!
    \brief Checks that we have good communication with the Micrel transceiver.

    This function is transceiver dependent.  It can be used during initial 
    testing to verify that the communication between the microcontroller
    and the transceiver is working.

    \return TRUE if communication is working, FALSE otherwise.
*/
BOOL check_xcvr_comm( void )
{
    #if 0
        UInt8 reg0_original;
        UInt8 reg0_current;
        UInt8 reg0_temp;
        BOOL result;

        // for the Micrel, we will check the power level, change it,
        // check the the change reports the new value, change back,
        // to the original value, and verify that the original
        // value is returned on a  read.
    
        result = TRUE;
        reg0_original = ReadRegister( 0x00 );
        // invert the power bits (PA2, PA1, PA0)
        reg0_current = reg0_original ^ 0x70;    

        WriteRegister( 0x00, reg0_current );
        reg0_temp = ReadRegister( 0x00 );
        if ( reg0_temp != reg0_current )
        {
            result = FALSE;
        }

        WriteRegister( 0x00, reg0_original );
        reg0_temp = ReadRegister( 0x00 );
        if ( reg0_temp != reg0_original )
        {
            result = FALSE;
        }

        return( result );
    #else
        return FALSE;
    #endif
} /* check_xcvr_comm */


#if 0
/*!
    \brief Checks that we have good bit clock signal from the Micrel transceiver.

    This function is transceiver dependent.  It can be used during initial 
    testing to verify that the communication between the microcontroller
    and the transceiver is providing a bit clock.

    \return TRUE if bit clock is working, FALSE otherwise.
*/
BOOL check_xcvr_bit_clock( void )
{
    UInt8 reg0_original;
    UInt8 reg0_current;
    UInt8 reg0_temp;
    BOOL result;

    // for the Micrel, we will put the transceiver in
    // receive mode and check for changes in the 
    // bit clock signal line.
    
    result = TRUE;

    return( result );

} /* check_xcvr_bit_clock */


/*!
    \brief Checks that the count down timer is functioning.

    This function is microcontroller dependent.  It can be used during initial 
    testing to verify that the count down timer is working.

    \return TRUE if count down time is working, FALSE otherwise.
*/
BOOL check_count_down_timer( void )
{
    UInt8 reg0_original;
    UInt8 reg0_current;
    UInt8 reg0_temp;
    BOOL result;

    // for the Micrel, we will put the transceiver in
    // receive mode and check for changes in the 
    // bit clock signal line.
    
    result = TRUE;

    return( result );

} /* check_count_down_timer */

/*!
    \brief List the rf configuration register values.

    This function is transceiver dependent.  It can be used during initial 
    testing to look at rf configuration values so that they can be changed.

    \return TRUE if communication is working, FALSE otherwise.
*/
void list_rf_regs( void )
{
    UInt8 rnum;

    for ( rnum=0; rnum < rf_number_of_cfg_regs; rnum++ )
    {
        sp_write_byte_hex( rnum, TRUE );
        sp_write_byte( '=', TRUE );
        sp_write_byte_hex( rf_regs_current[rnum], TRUE );
        sp_write_byte( ' ', TRUE );
        // until serial output TRUE option is fixed, add some delays
        delay_10ms();
        if ( rnum == 11 )
        {
            sp_write_rom_string( "\r\n", TRUE);
        }
    }
    sp_write_rom_string( "\r\n", TRUE);

} /* list_rf_regs */


/*!
    \brief Let user change the rf configuration register values.

    This function is transceiver dependent.  It can be used during initial 
    testing to change rf configuration values.

*/
void change_rf_regs( void )
{
    UInt8 char_count = 0;
    UInt8 nibble;
    UInt8 char_typed[2];

    char_count = 0;
    sp_write_rom_string( "reg address to change (in hex)? ", TRUE );
    delay_10ms();
    while ( char_count < 2 )
    {
        if ( sp_chars_read() >= 1 )
        {
            sp_read_buffer( &char_typed[char_count], 1 );  // read the char typed
            sp_write_byte( char_typed[char_count], TRUE ); // echo them
            char_count++;
        }
    }
    nibble = (char_typed[0] <= '9') ? (char_typed[0] - 0x30) : (char_typed[0] - 0x61) + 0x0a;
    rnum = nibble << 4;
    nibble = (char_typed[1] <= '9') ? (char_typed[1] - 0x30) : (char_typed[1] - 0x61) + 0x0a;
    rnum |= nibble & 0x0f;
    sp_write_rom_string( "  reg address=  ", TRUE );
    sp_write_byte_hex( rnum, TRUE ); // echo it
    delay_10ms();
    

    char_count = 0;
    sp_write_rom_string( "\r\nnew register value (in hex)   ? ", TRUE );
    delay_10ms();
    while ( char_count < 2 )
    {
        if ( sp_chars_read() >= 1 )
        {
            sp_read_buffer( &char_typed[char_count], 1 );  // read the char typed
            sp_write_byte( char_typed[char_count], TRUE ); // echo them
            char_count++;
        }
    }
    nibble = (char_typed[0] <= '9') ? (char_typed[0] - 0x30) : (char_typed[0] - 0x61) + 0x0a;
    rval = nibble << 4;
    nibble = (char_typed[1] <= '9') ? (char_typed[1] - 0x30) : (char_typed[1] - 0x61) + 0x0a;
    rval |= nibble & 0x0f;
    sp_write_rom_string( "  new reg value=", TRUE );
    sp_write_byte_hex( rval, TRUE ); // echo it
    delay_10ms();
    rf_regs_current[rnum] = rval;

} /* change_rf_regs */
#endif

UInt8 self_test( void )
{
    UInt8 result = 0;

    #if 0
        sp_write_rom_string( "\r\ncheck_xcvr_comm .... ", TRUE );
        if ( check_xcvr_comm( ) == TRUE )
        {
            sp_write_rom_string( "ok.", TRUE);
        }
        else
        {
            sp_write_rom_string( "FAILED.", TRUE);
            result |= 0x01;
        }
        delay_10ms();
    #endif

    return( result );
} /* self_test */


//*****************************************************************************
void check_transceiver ( BOOL dump )
{

} /* check_transceiver */

// test sending/receiving the pattern (0x55555533)

void rf_test(void)
{

    int i = 0;
    enum {BUF_SIZE = 16};
    UInt8 buf[BUF_SIZE];
    UInt8 map;
    UInt32 pattern = 0;

#ifdef MICREL_1s_AND_0s_TEST
    // the folowing rf_test code will send a constant stream of 0xAA's
    // when MODE_SELECT_SW_TOP is to the left and it will listen for
    // a contant stream of 0xAA's when MODE_SELECT_SW_TOP is to the 
    // right.
    if ( MODE_SELECT_SW_TOP == 0 )
    {
        // just send the bit pattern 101010101010...
        sp_write_rom_string( "rf test: transmitting 1's and 0's.\r\n", TRUE );
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 
        while ( 1 ) {
            // change the direction of the DATAIXO pin
            RF_SET_TX_DATA_DIRECTION();
            data_to_send = 0x55;
            one_net_write_test_pattern ( data_to_send );
        }
    }
    else
    {
        // just keep reading bytes into buf
        sp_write_rom_string( "rf test: receiving.\r\n", TRUE );
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 

        turn_on_receiver();
        disable_global_interrupts();
        RX_LED = 1;
            
        i = 0;
        while ( 1 )
        {
            for(map = 0x80; map; map >>= 1)
            {
                // wait for DCLK to go low so we don't read the same bit twice
                // (and we know we are seeing the first bit after the pattern
                // irq has gone high)
                while(BIT_CLK);
                // wait for DCLK to go high to read the data
                while(!BIT_CLK);

                // read the data
                if(RF_RX_DATA)
                {
                    buf[i] |= map;
                }
                else
                {
                    buf[i] &= ~map;
                }
            }
            i = (i + 1) % BUF_SIZE;
        }
    }
#else
    if ( MODE_SELECT_SW_TOP == 0 )
    {
        if ( NODE_SELECT_SW_BOTTOM == 0 )
        {
            data_to_send = 0x0f;
        }
        else
        {
            data_to_send = 0x55;
        }
        sp_write_rom_string( "rf test: transmitting test packet", TRUE );
        sp_write_byte_hex( data_to_send, TRUE );
        sp_write_byte( '\r', TRUE );
        sp_write_byte( '\n', TRUE );
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 
        delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); delay_10ms(); 
        
        // transmit test packet	
        while ( 1 ) {
            // change the direction of the DATAIXO pin
            RF_SET_TX_DATA_DIRECTION();
            one_net_write_test_pattern ( data_to_send );
        }
    } // If MODE_SELECT_SW_TOP //
    else
    {
        sp_write_rom_string( "rf test: receiving...\r\n", TRUE );
        delay_10ms();

        turn_on_receiver();
        disable_global_interrupts();
            
        while ( 1 )
        {
            // look for a pattern
            pattern = 0;
            while ( 1 )
            {
                // collect bits looking for a sync pattern
                while(BIT_CLK);
                // wait for DCLK to go high to read the data
                while(!BIT_CLK);

                // read the data
                pattern = (pattern << 1) | RF_RX_DATA;
                if ( pattern == 0x55555533 )
                {
                    break;
                }
            }

            // found pattern, collect the packet
            i = 0;
            while ( 1 )
            {
                for(map = 0x80; map; map >>= 1)
                {
                    // wait for DCLK to go low so we don't read the same bit twice
                    // (and we know we are seeing the first bit after the pattern
                    // irq has gone high)
                    while(BIT_CLK);
                    // wait for DCLK to go high to read the data
                    while(!BIT_CLK);

                    // read the data
                    if(RF_RX_DATA)
                    {
                        buf[i] |= map;
                    }
                    else
                    {
                        buf[i] &= ~map;
                    }
                }
                if ( ++i >= BUF_SIZE)
                {
                    break;
                }
                // i = (i + 1) % BUF_SIZE;
            }
            enable_global_interrupts();
            print_byte( '*' );
            while ( spo_head != spo_tail )
            {
                delay_1ms();
            }
            disable_global_interrupts();
        } // while (1) for packets //
    } // else MODE_SELECT_SW_TOP //
#endif

#if 0
        if(mode_select_value == MASTER)
        {
            UInt8 data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
            UInt8 response[ON_SINGLE_DATA_RAW_PLD_SIZE];

            // master sends the patter
            while(1)
            {
                TURN_ON(TX_LED);

                single_transaction(&slave_addrs[0], data, sizeof(data),
                 response, ON_SINGLE_DATA_RAW_PLD_SIZE);

                TURN_OFF(TX_LED);
        //            delay_100ms();
            }
        }
        else
        {
            while(1)
            {
                TURN_ON(RX_LED);
                if(listen_for_master(500) == SUCCESS)
                {
                    sp_write_string("Found pkt\n", TRUE);
                }
                TURN_OFF(RX_LED);
            }
        }
#endif
#if 0
       void one_net_write_test_pattern ( UInt8 pattern );
        enum
        {
            MSG_BUFFER_SIZE = 16
        };
        
        UInt8 msg_buffer[MSG_BUFFER_SIZE];
        
        while(1)
        {
            if ( mode_select_value == MASTER ) 
            {
                // send the above packect over and over and over again
                one_net_write( msg_buffer, 11 );
            }
            else if ( mode_select_value == SLAVE1 )
            {
                // listen for a packet
                if ( one_net_look_for_pkt( 1000, ONE_NET_ENCODED_ADDR_LEN )
                  == FALSE )
                {
                    sp_write_byte( '.', TRUE );
                }
                else
                {
                    sp_write_byte( 'R', TRUE );
                }
            }
            else if ( mode_select_value == SLAVE2 )
            {
                // transmit 0x55's	
                while ( 1 ) 
                {
                    // change the direction of the DATAIXO pin
                    RF_SET_TX_DATA_DIRECTION();
                    
                    one_net_write_test_pattern ( 0x55 );
                }
           }
           else
           {
                sp_write_rom_string( "rf test: SLAVE3: not implemented.\r\n", TRUE );
           }
        } /* while */
#endif
#if 0
        // xtea timing testing
        sp_write_rom_string( "xtea: timing starting\r\n", TRUE );
        xtea_test();
#endif
} // rf_test //

tick_t one_net_tick(void)
{
    return TickCount;
}


void one_net_sleep(tick_t DURATION)
{
    set_cdt2(DURATION);
    while(get_cdt2());
}

void one_net_set_channel( const UInt8 new_channel )
{
    // TBD: implement for the micrel
} /* one_net_set_channel */

UInt8 one_net_get_channel( void )
{
    // TBD: implement for the micrel
    return( 17 );
} /* one_net_get_channel */

