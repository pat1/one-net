//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file spi.h
   \brief Hardware specific SPI declarations for the Micrel
    
   \author  Roger Meadows
   \date $Date: 2005/04/27 08:37:39 $
   \version $Rev: 51 $
   \note Threshold Corporation

   \see lpw_definitions
   
*/ 

// SPI Error defines
#define SPI_ERR_OVERFLOW    0x01
#define SPI_ERR_RECEIVE_TO  0x02
#define SPI_ERR_TRANSMIT_TO 0x04


//==============================================================================
//                       PUBLIC FUNCTION DECLARATIONS

unsigned int spi_transmit( unsigned char send, unsigned char *receive );
void SPIPut(BYTE v);
BYTE SPIGet(void);
unsigned char get_spi_flags( void );
void clear_spi_flags( void );
void init_SPI(void);


//                       PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

