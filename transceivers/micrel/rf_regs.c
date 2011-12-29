//
// Copyright 2005, 2006, Threshold Corporation, All rights reserved.
//
/*!
   \file rf_regs.c
   \brief Configuration register values for the Micrel transceiver.
    
    This file contains the structure definitions for transeiver 
    configuration registers. These structures are used by some 
    hardware independent code to initialize and check the configuration
    of a transciever.
 
   \author  $Author: Roger $
   \date $Date: 2005/04/27 08:37:39 $
   \version $Rev: 121 $
   \note Threshold Corporation

   \see lpw_definitions
   
*/ 

#include "rf_regs.h"
#include "micrel.h"


UInt8 rf_regs_current_r0 = 0x7f;    // current value of register 0
#if 1   // values from Adam's calcs for channel 17
        // with Modulation Type set to "FSK applied using two sets of dividers"
const RF_REGS rf_regs_cfg[rf_number_of_cfg_regs] = 
{
    // address, initial_value, compare_mask, initialze
    {   0x00,   0x7f,           0xfe,       TRUE },
    {   0x01,   0x87 ,          0xff,       TRUE }, // using two sets of dividers
    {   0x02,   0x00,           0xff,       TRUE },
    {   0x03,   0xc6,           0xff,       TRUE },
    {   0x04,   0x87,           0xff,       TRUE },
    {   0x05,   0x11,           0xff,       TRUE },
    {   0x06,   0x5c,           0xff,       TRUE },
    {   0x07,   0x8d,           0xff,       TRUE },
    {   0x08,   0xc1,           0xff,       TRUE },
    {   0x09,   0x30,           0xff,       TRUE },
    {   0x0a,   0x01,           0xff,       TRUE }, // A0
    {   0x0b,   0x00,           0xff,       TRUE }, // N0 high
    {   0x0c,   0x93,           0xff,       TRUE }, // N0 low
    {   0x0d,   0x00,           0xff,       TRUE }, // M0 high
    {   0x0e,   0x29,           0xff,       TRUE }, // M0 low
    {   0x0f,   0x06,           0xff,       TRUE }, // A1
    {   0x10,   0x00,           0xff,       TRUE }, // N1 high
    {   0x11,   0x88,           0xff,       TRUE }, // N1 low
    {   0x12,   0x00,           0xff,       TRUE }, // M1 high
    {   0x13,   0x26,           0xff,       TRUE }, // M1 low
    {   0x14,   0xb5,           0xff,       TRUE },
    {   0x15,   0x00,           0xff,       TRUE }
};
#endif

#if 0   // values from Adam's calcs for channel 17
        // but setting Modulation Type to "Closed loop modulation
        // using modulator" and setting transmit values to 
        // those for the receiver.
const RF_REGS rf_regs_cfg[rf_number_of_cfg_regs] = 
{
    // address, initial_value, compare_mask, initialze
    {   0x00,   0x7f,           0xfe,       TRUE },
    {   0x01,   0x07 ,          0xff,       TRUE }, // Closed loop modulation
    {   0x02,   0x00,           0xff,       TRUE },
    {   0x03,   0xc6,           0xff,       TRUE },
    {   0x04,   0x87,           0xff,       TRUE },
    {   0x05,   0x11,           0xff,       TRUE },
    {   0x06,   0x5c,           0xff,       TRUE },
    {   0x07,   0x8d,           0xff,       TRUE },
    {   0x08,   0xc1,           0xff,       TRUE },
    {   0x09,   0x30,           0xff,       TRUE },
    {   0x0a,   0x0c,           0xff,       TRUE }, // A0
    {   0x0b,   0x00,           0xff,       TRUE }, // N0 high
    {   0x0c,   0x84,           0xff,       TRUE }, // N0 low
    {   0x0d,   0x00,           0xff,       TRUE }, // M0 high
    {   0x0e,   0x25,           0xff,       TRUE }, // M0 low
    {   0x0f,   0x0c,           0xff,       TRUE }, // A1
    {   0x10,   0x00,           0xff,       TRUE }, // N1 high
    {   0x11,   0x84,           0xff,       TRUE }, // N1 low
    {   0x12,   0x00,           0xff,       TRUE }, // M1 high
    {   0x13,   0x25,           0xff,       TRUE }, // M1 low
    {   0x14,   0xb5,           0xff,       TRUE },
    {   0x15,   0x00,           0xff,       TRUE }
};
#endif

#if 0
void set_rf_reg( UInt8 rnum, UInt8 rval )
{
    UInt8 i;

    for ( i=0; i<rf_number_of_cfg_regs; i++ )
    {
        if ( rf_regs_cfg[i].address == rnum )
        {
            rf_regs_current[i] = rval;
            return;
        }
    }
} /* set_rf_reg */
#endif

