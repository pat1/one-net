#ifndef __TRC102_IO_H__
#define __TRC102_IO_H__

/*!
   \file trc102_io.h
   \brief Header file for trc102_io.c, interface code for TRC102 transceiver chip.
   
   \date Sept 8 2006  
   \author Sean Machin
   \note Threshold Corporation

*/

#include <sfr_r81B.h>
#include "one_net_types.h"

/* Define some symbolic names for the transceiver I/O lines etc. */
#define SDI p3_7		// TRC102 SPI data in
#define SCK p3_5		// TRC102 SPI data clock
#define CS p3_4			// TRC102 chip select
#define SDO p1_6		// TRC102 SPI data output
#define GREEN_LED p1_2	// green RX LED
#define RED_LED p1_3	// RED TX LED
#define TRC_DATA p3_3	// TRC102 data pin
#define FINT p1_7		// CR pin from TRC102, can indicate RX FIFO is full

void write_reg(const _U16 reg);
_U16 read_status_reg();
void enable_tx(BOOL enabled);
void enable_rx(BOOL enabled);
void tx_byte(BYTE value);
#pragma INTERRUPT int0_isr
void int0_isr(void);
void init_board_interrupts(void);

#endif