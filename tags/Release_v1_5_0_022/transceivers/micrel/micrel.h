//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file micrel.h
   \brief Hardware specific functions for the Micrel
    
   \author  Roger Meadows
   \date $Date: 2005/04/27 08:37:39 $
   \version $Rev: 51 $
   \note Threshold Corporation

   \see lpw_definitions
   
*/ 

#include "one_net_types.h"

enum 
{
   rf_number_of_cfg_regs = 22
};

// SPI Error defines
#define SPI_ERR_OVERFLOW    0x01
#define SPI_ERR_RECEIVE_TO  0x02
#define SPI_ERR_TRANSMIT_TO 0x04


// Evaluation board switch ports
#define MODE_SW                     p1_1
#define MODE_SELECT_SW_TOP          p4_7
#define MODE_SELECT_SW_BOTTOM       p4_6

// Evaluation board IO ports
#define RF_SET_RX_DATA_DIRECTION()  pd1_6=0
#define RF_SET_TX_DATA_DIRECTION()  pd1_6=1
#define RF_RX_DATA  p1_6
#define RF_TX_DATA  p1_6
#define RF_LD       p4_5
#define BIT_CLK     p1_7
#define SPI_CS      p3_4

//==============================================================================
//                       PUBLIC FUNCTION DECLARATIONS

unsigned int spi_transmit( unsigned char send, unsigned char *receive );
void SPIPut(BYTE v);
BYTE SPIGet(void);
unsigned char get_spi_flags( void );
void clear_spi_flags( void );
void init_SPI(void);
void write_DI_micrel(UInt8 rval);
UInt8 read_DI_micrel();
void init_interrupts(void);
void init_io( void );
void set_pattern( UInt8 * data );
BOOL wait_for_ld( UInt16 ms );
BOOL check_xcvr_comm( void );
void list_rf_regs( void );
UInt8 self_test( void );

//                       PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

