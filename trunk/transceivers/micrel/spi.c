//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file spi.c
   \brief Hardware specific SPI functions for the Micrel
    
   \author  Roger Meadows
   \date $Date: 2005/04/27 08:37:39 $
   \version $Rev: 51 $
   \note Threshold Corporation

   \see lpw_definitions
   
*/ 

#include "demo.h"
#include "sfr_r81B.h"
#include "spi.h"
#include "one_net_types.h"


#define DEBUG_SPI_TRANSMIT 0

static unsigned char spi_flags;

unsigned int spi_transmit( UInt8 send, UInt8 *receive )
{
    UInt8 rval;
    UInt8 i;
    
    // always send the character provided
    re_sser=0;  //disable receive in BIDE mode 
    te_sser=1;  //enable transmit
    
    
    p3_4=1;  //enable CS (Chip Select)

    if ( receive != ( unsigned char *) 0 )
    {
    while (!tdre_sssr); //wait until transmit buffer is clear
    if (orer_sssr) //if overflow, clear all SPI flags
    {
        sssr=0;
    }
    sstdr=send;
    if (orer_sssr) //if overflow, clear all SPI flags
    {
        sssr=0;
    }

//    while (!tdre_sssr); //wait until transmit buffer is clear
  while(!tend_sssr);        // wait final bit is currenlty being sent
        tend_sssr=0;
    }
    // if the receive pointer is not NULL read and return
    if ( receive != ( unsigned char *) 0 )
    {
//        while(!tend_sssr);        // wait final bit is currenlty being sent
//            tend_sssr=0;
       
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

        
      
        *receive = rval;

        for (i=0;i<100;i++);

    }
        
    for (i=0;i<200;i++);
    p3_4=0; // set chip select to low     //CS disabled
    
    // nothing special to return so far
    return( 0 );
    
} /* spi_transmit */


void SPIPut(BYTE v)
{

	unsigned char dummy;
    unsigned int return_value;
    
    return_value = spi_transmit( v, (BYTE *) 0 );
    
}

BYTE SPIGet(void)
{
	unsigned char rval;
    unsigned return_value;
    
    return_value = spi_transmit( 0x00, &rval);
    
    return( rval );
        
}

/*! \brief Return error flags for SPI interface.

This function returns the error flags for the SPI interface.
Each bit in the byte returned corresponds to an error conditon
that has occurred.
The clear_spi_flags() function must be called to clear these flags.

*/
unsigned char get_spi_flags( void )
{
    unsigned char flags;
    
    disable_global_interrupts();
    flags = spi_flags;
    enable_global_interrupts();
    
    return( flags );
    
} /* get_spi_flags */


/*! \brief Clear error flags for the SPI interface.

This function clears the error flags associated with the SPI
interfacve. Each bit in the
byte returned corresponds to an error conditon that has occurred.
These flags are only cleared when this function is called.

*/
void clear_spi_flags( void ) 
{
    
    disable_global_interrupts();
    spi_flags = 0;
    enable_global_interrupts();
    
} /* clear_spi_flags */


void WriteRegister(UInt8 rnum, UInt8 rval)
{
    int i;
    UInt8 r1;

    re_sser=0;  //disable receive in BIDE mode since in this Fn call only Tx occurs
    te_sser=1;  //enable transmit
    
    
    p3_4=1;  //enable CS (Chip Select)
    p1_4=1;  //debug signal for logic analyzer-- positive pulse for write

    while (!tdre_sssr); //wait until transmit buffer is clear
    if (orer_sssr) //if overflow, clear all SPI flags
        sssr=0;
    sstdr=rnum<<1|0x0;  //write a value to register 0
                        //bit7=0 start bit
                        //bit6=0 write 1=read
                        //bit5/4/3/2/1=address
                        //bit0=1 stop bit


    while (!tdre_sssr); //wait until transmit buffer is clear
    sstdr=rval; //write this value
    
    while(!tend_sssr);
    tend_sssr=0;
    te_sser=0;
    _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");
    
    p1_4=0; // TX external

    for (i=0;i<200;i++);
    p3_4=0; //NSS_CONFIG disabled (p3_4 is Micrel CS line)
    for (i=0;i<100;i++);
    
} /* WriteRegister */

UInt8 ReadRegister(UInt8 rnum)
{
      unsigned char rval;
      int i;
      
      re_sser=0;                    //disable Rx in BIDE mode initially while address Tx occurs
      _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");
      te_sser=1;                    //enable transmit
      _asm("nop");_asm("nop");_asm("nop");_asm("nop");_asm("nop");
        
        
    while(!tdre_sssr);      // wait till tx register set as empty

    
      p3_4=1;  // set chip select line to high ...//enable NSS_CONFIG
  
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
      p3_4=0; // set chip select to low     //NSS_CONFIG disabled
      
        

      for (i=0;i<100;i++);
      return rval;
} /* ReadRegister */

