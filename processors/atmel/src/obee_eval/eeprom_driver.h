/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA EEPROM driver header file.
 *
 *      This file contains the function prototypes and enumerator definitions
 *      for various configuration parameters for the XMEGA EEPROM driver.
 *
 *      The driver is not intended for size and/or speed critical code, since
 *      most functions are just a few lines of code, and the function call
 *      overhead would decrease code performance. The driver is intended for
 *      rapid prototyping and documentation purposes for getting started with
 *      the XMEGA EEPROM module.
 *
 *      For size and/or speed critical code, it is recommended to copy the
 *      function contents directly into your application instead of making
 *      a function call.
 *
 * \par Application note:
 *      AVR1315: Accessing the XMEGA EEPROM
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 1569 $
 * $Date: 2008-04-22 13:03:43 +0200 (ti, 22 apr 2008) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include"config_options.h"

#ifdef _ATXMEGA256A3B

#ifndef EEPROM_DRIVER_H
#define EEPROM_DRIVER_H

#include "one_net_types.h"

#include <inttypes.h>
#include <io.h>
#include <interrupt.h>


//#define MAPPED_EEPROM_START 0x1000
#define EEPROM_PAGESIZE 32
#define EEPROM(_pageAddr, _byteAddr) \
	((uint8_t *) MAPPED_EEPROM_START)[_pageAddr*EEPROM_PAGESIZE + _byteAddr]



/* Definitions of macros. */

/*! \brief Enable EEPROM block sleep-when-not-used mode.
 *
 *  This macro enables power reduction mode for EEPROM.
 *  It means that the EEPROM block is disabled when not used.
 *  Note that there will be a penalty of 6 CPU cycles if EEPROM
 *  is accessed.
 */
#define EEPROM_EnablePowerReduction() ( NVM.CTRLB |= NVM_EPRM_bm )

/*! \brief Disable EEPROM block sleep-when-not-used mode.
 *
 *  This macro disables power reduction mode for EEPROM.
 */
#define EEPROM_DisablePowerReduction() ( NVM.CTRLB &= ~NVM_EPRM_bm )

/*! \brief Enable EEPROM mapping into data space.
 *
 *  This macro enables mapping of EEPROM into data space.
 *  EEPROM starts at EEPROM_START in data memory. Read access
 *  can be done similar to ordinary SRAM access.
 *
 *  \note This disables IO-mapped access to EEPROM, although page erase and
 *        write operations still needs to be done through IO register.
 */
#define EEPROM_EnableMapping() ( NVM.CTRLB |= NVM_EEMAPEN_bm )

/*! \brief Disable EEPROM mapping into data space.
 *
 *  This macro disables mapping of EEPROM into data space.
 *  IO mapped access is now enabled.
 */
#define EEPROM_DisableMapping() ( NVM.CTRLB &= ~NVM_EEMAPEN_bm )

/*! \brief Non-Volatile Memory Execute Command
 *
 *  This macro set the CCP register before setting the CMDEX bit in the
 *  NVM.CTRLA register.
 *
 *  \note The CMDEX bit must be set within 4 clock cycles after setting the
 *        protection byte in the CCP register.
 */
#define NVM_EXEC()	asm("push r30"      "\n\t"	\
			    "push r31"      "\n\t"	\
    			    "push r16"      "\n\t"	\
    			    "push r18"      "\n\t"	\
			    "ldi r30, 0xCB" "\n\t"	\
			    "ldi r31, 0x01" "\n\t"	\
			    "ldi r16, 0xD8" "\n\t"	\
			    "ldi r18, 0x01" "\n\t"	\
			    "out 0x34, r16" "\n\t"	\
			    "st Z, r18"	    "\n\t"	\
    			    "pop r18"       "\n\t"	\
			    "pop r16"       "\n\t"	\
			    "pop r31"       "\n\t"	\
			    "pop r30"       "\n\t"	\
			    )

/* Prototyping of functions. */
void EEPROM_WriteByte( UInt8 pageAddr, UInt8 byteAddr, UInt8 value );
//void eeprom_write_byte(uint16_t * address, uint8_t value);

//void eeprom_write_byte(uint16_t address, uint8_t value);
//void eeprom_write_block(uint8_t * src, uint16_t * dst, uint16_t size);
void eeprom_write_byte(UInt16 address, UInt8 value);
void eeprom_write_block(UInt8 * src,UInt16 dst, UInt16 size);




UInt8 EEPROM_ReadByte( UInt8 pageAddr, UInt8 byteAddr );
//uint8_t eeprom_read_byte(uint16_t * address);

//uint8_t eeprom_read_byte(uint16_t address);
//void eeprom_read_block(uint8_t * dst, uint16_t * src, uint16_t size);
UInt8 eeprom_read_byte(UInt16 address);
void eeprom_read_block(UInt8 * dst, UInt16 src, UInt16 size);


void EEPROM_WaitForNVM( void );
void EEPROM_FlushBuffer( void );
void EEPROM_LoadByte( UInt8 byteAddr, UInt8 value );
void EEPROM_LoadPage( const UInt8 * values );
void EEPROM_AtomicWritePage( UInt8 pageAddr );
void EEPROM_ErasePage( UInt8 pageAddress );
void EEPROM_SplitWritePage( UInt8 pageAddr );
void EEPROM_EraseAll( void );

#endif

#endif // #ifdef _ATXMEGA256A3B