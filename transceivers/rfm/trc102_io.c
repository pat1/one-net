/*!
   \file trc102_io.c
   \brief I/O functions specific to TRC102 transceiver chip.
   
   \date Sept 8 2006  
   \author Sean Machin
   \note Threshold Corporation

*/
#include "trc102_io.h"

/* Define total number of config. registers and index of some specific registers in the array below. */
#define NUM_CONFIG_REGS 16
#define PWR_MGMT_REG 15
#define TX_DATA 3 

/*
	Default config. register values sent to the TRC102, see RFM datasheet p.12.
	These registers are defined in the order we want to write them to the chip.
	Do not re-order these entries without reviewing the constants such as PWR_MGMT_REG
	defined above or understanding the order they need to be in.
*/
static _U16 RegistersCfg[] = {
	0b1000000000111000, // config; TX data reg. DISABLED (using manual modulation), RX FIFO DISABLED, 915.5MHz
    0b1100010000110000,	// AFA; mode off
    0b1001100011110000,	// TX config.; +/-240 KHz deviation, max. power output, +ve polarity
    0b1011100010101010, // TX data; send 0xaa (same as on chip preamble byte)
    0b1010100000010011, // freq. setting; 915.5MHz
    0b1001010000100000, // recv. control; pin 16 is valid data out, fast valid data detect, 400KHz baseband BW, 0dB LNA gain, RSSI -103dB
    0b1100001010111100, // baseband; auto. clock recovery, slow clock recovery mode, digital filter, data detect threshold 4
    0b1011000000000000, // FIFO read; (lower 8 bits are the data read)
    0b1100101010001111, // FIFO/reset config; FIFO size 8, FIFO fill cont., FIFO fill, disable reset mode
    0b1100111000110011, // synch. char; (lower 8 bits)
    0b1100011000000100, // data rate setup; approx. 38.4Kbps
    0b1110000000000000, // wake up period; 
    0b1100100000000000, // duty cycle; 
    0b1100000000000001, // batt. detect & clk out; clk out 1MHz
    0b1100110001010111,	// PLL config.; clock buf. slew < 2.5MHz, crystal startup 1ms, phase detector delay enabled, dithering disabled, PLL BW reduced
    0b1000001010011010, // power management, RX enabled, TX disabled, synthesizer enabled, oscillator enabled, low batt. detector disabled, wake up timer disabled, clock output enabled,
};

/*
	Reads the single 16 bit status register on the TRC102 using manual
	bit banging of the I/O lines.
*/
_U16 read_status_reg() {

	_U16 ret = 0;
	int i;
	
	CS = 1;		
	SCK = 0;
	SDI = 1;
	
	CS = 0;	// start of read
	
	SDI = 0;
	for (i = 0;i < 16;i++) {
		SCK = 1;
		ret |= (SDO & 0x01);
		ret <<= 1;
		delay_10us();
		SCK = 0;
		delay_10us();
	}
	
	CS = 1;	// end of read
	
	return ret;	
}

/*
	Writes a 16 bit register value to the TRC102 SPI interface using manual
	bit banging of the I/O lines.
	The address of the register is implicit in the upper bits of its 16 bit value
	(i.e. this function has no separate address argument).
*/
void write_reg(const _U16 reg) {
	int i;
	BOOL bit;
	_U16 mask = 0b1000000000000000;	// bitmask for writing one bit of the reg. at a time
	
	CS = 1;		// CS is p3_4	
	SCK = 0;
	
	CS = 0; // start of write, CS is p3_4
	delay_10us();

	for (i = 0;i < 16;i++) {
		// data read by TRC102 from SDI as SCK transitions 0->1
		SDI = (reg & mask) ? 1 : 0;	// SDI is p3_7
		mask >>= 1;
		SCK = 1;	// SCK is p3_5
		delay_10us();
		SCK = 0;	// SCK is p3_5
		delay_10us();
	}
	
	CS = 1;	// end of write, CS is p3_4
}

/*
	Enable or disable the TRC102 transmitter by setting TXEN bit
	in the power management register;
*/
void enable_tx(BOOL enabled) {
	
	_U16 reg = RegistersCfg[PWR_MGMT_REG];
	if (enabled) {
		reg |= 0b0000000000100000;	// turn on TXEN bit
	}
	else {
		reg &= 0b1111111111011111;	// turn off TXEN bit
	}
	write_reg(reg);
}

/*
	Enable or disable the TRC102 receiver by setting RXEN bit
	in the power management register;
*/
void enable_rx(BOOL enabled) {
	
	_U16 reg = RegistersCfg[PWR_MGMT_REG];
	if (enabled) {
		reg |= 0b0000000010000000;	// turn on RXEN bit
	}
	else {
		reg &= 0b1111111101111111;	// turn off TXEN bit
	}
	write_reg(reg);
}

/*
	Transmit the byte value given.
*/
void tx_byte(BYTE value) {
	
	const _U16 new_tx = (RegistersCfg[TX_DATA] & 0xff00) | value;

	write_reg(new_tx);	
	enable_tx(TRUE);
	
	// TODO : this not finished
	while (SDO == 0) {
		;
	}
	enable_tx(FALSE);
}

/*
	Interrupt handler for INT0 from TRC102.
	This interrupt is multipurpose, an active low interrupt is generated
	when the TX reg. is ready for next byte or RX FIFO has filled 
	to its threshold or FIFO overflow.
*/
void int0_isr(void) { 

	// if the FINT input is high, then the RX FIFO has filled to its threshold
	if (FINT) {
		// try to read from FIFO RX register while FINT stays high
		while (FINT) {
			
		}
	}
}

/*
	Initialize any board specific interrupts.
	This is called before init_transceiver().
	
	For the TRC102 we enable interrupt 0, the TRC102 drives INT0 active low
	when the TX reg. is ready for the next byte or the RX FIFO has received a
	certain number of bytes.
*/
void init_board_interrupts(void) {
	
	int0ic |= 7; // set int0 to highest priority
	int0pl = 0; // select falling edge polarity
	// don't know what to write if anything to ir_int0ic
	int0en = 1;	// enable int0
}

void init_transceiver(void)
{
}
