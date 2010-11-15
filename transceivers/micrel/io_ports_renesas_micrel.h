/*! \defgroup lpw_io_ports_dw_th_sensor IO Port Definitions */ 
/*!@{*/ 

/*!
   \file io_ports_renesas_micrel.h
   \brief IO Port definitions for the ONE-NET Renesas/Semtech
   Evaluation Module.
    
    This file contains the IO Port definitions for the ONE-NET Renesas/Micrel
   Evaluation Module. The header file io_ports.h will 
    include this file if the symbol ONE_NET_EVAL_MOD_RENESAS_SEMTECH is set to
    1.
    
   \author Roger Meadows 
   \date 05/09/06
   \version $Rev: 43 $
   \note Threshold Corporation

   \see lpw_definitions
   
*/ 


/*******************************************************************
** I/O Ports Definitions                                          **
*******************************************************************/
#define RX_LED         p1_2    // R8:output LED2:green    
#define TX_LED         p1_3    // R8:output LED1:red 
#define NSS_DATA       p3_3    // R8:output SPI select data
#define IRQ1           p4_5    // R8:input  FIFO FULL or PATTERN

#define IRQ0           p1_7    // R8:input  /FIFO EMPTY or PATTERN
#define FIFONOTEMPTY   IRQ0    // R8:input true when data is in FIFO
#define FIFOFULL       IRQ1    // R8:input  FIFO FULL or PATTERN
#define TXSTOPPED      IRQ1    // R8:input  Transmission has stopped
#define NSS_CONFIG     p3_4    // R8:output SPI select config
#define SCLK           p3_5    // R8:output SPI clock
#define MISO           p1_0    // R8:input  SPI Master In Slave Out
#define MOSI           p3_7    // R8:output SPI Master Out Slave In    

#define SW1            NotUsed    // R8:output TX/RX/SBY/SLP mode select
#define SW0            NotUsed    // R8:output TX/RX/SBY/SLP mode select

/*!@}*/ 
