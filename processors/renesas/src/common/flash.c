//! \addtogroup FLASH
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice,
          this list of conditions, and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of Threshold Corporation (trustee of ONE-NET) nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHEWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!
    \file flash.c
    \brief Contains implementation for interacting with the data flash on the
      R8C family of processors.
*/

#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //


#include "flash.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup FLASH_const
//! \ingroup FLASH
//! @{

//! Commands for accessing the data flash
enum
{
    //! Command written in the first bus cycle to start reading the data flash
    //! at the given address.
    READ_ARRAY = 0xFF,
    
    //! Command written in the first bus cycle to read the status register
    READ_STATUS_REG = 0x70,

    //! Command written in the first bus cycle to clear the status register
    CLR_STATUS_REG = 0x50,

    //! Command written in the first bus cycle to program data.  This needs to
    //! be the same address that is written to in the second bus cycle when the
    //! data is written to that address.
    PROGRAM = 0x40,

    //! Command written in the first bus cycle to erase a block.
    ERASE_CYCLE1 = 0x20,

    //! Command written in the second bus cycle to erase a block.
    ERASE_CYCLE2 = 0xD0
};

//! @} FLASH_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup FLASH_typedefs
//! \ingroup FLASH
//! @{

//! @} FLASH_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup FLASH_pri_var
//! \ingroup FLASH
//! @{

//! @} FLASH_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup FLASH_pri_func
//! \ingroup FLASH
//! @{

void enable_df_write(void);
void disable_df_write(void);


/*!
    \brief Returns TRUE when writing the byte or erasing the block is complete

    \param void

    \return TRUE if the operation is complete
            FALSE if operation is still in progress.
*/
#define COMPLETE() ((BOOL)(fmr00 == 1))


/*!
    \brief Returns TRUE if the write was successful

    \param void

    \return TRUE if the write was successful
            FALSE if the write was not successful
*/
#define WRITE_SUCCESS() ((BOOL)(fmr06 == 0))


/*!
    \brief Returns TRUE if the erase was successful

    \param void

    \return TRUE if the erase was successful
            FALSE if the erase was not successful
*/
#define ERASE_SUCCESS() ((BOOL)(fmr07 == 0))


void clr_status_reg(UInt8 * addr);

//! @} FLASH_pri_func
//!                     PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup FLASH_pub_func
//! \ingroup FLASH
//! @{

/*!
    \brief Writes data to the data flash block

    No data is written if the data to be written will cross data flash block
    boundries.    

    \param[in] ADDR The data flash address to start the write from
    \param[in] DATA The data to be written
    \param[in] LEN The number of bytes to write

    \return The number of bytes written
*/
UInt16 write_data_flash(const UInt16 ADDR, const UInt8 * DATA, const UInt8 LEN)
{
    UInt16 written = 0;
    
    if(!DATA || ADDR < DF_BLOCK_START || ADDR >= DF_BLOCK_END)
    {
        return 0;
    } // if the parameters are invalid //
    
    if(ADDR + LEN >= DF_BLOCK_END)
    {
        return 0;
    } // if not enough room //

    enable_df_write();

    while(written < LEN)
    {
        ((UInt8 *)ADDR)[written] = PROGRAM;
        ((UInt8 *)ADDR)[written++] = *DATA++;
        while(!COMPLETE());
        if(!WRITE_SUCCESS())
        {
            clr_status_reg(&(((UInt8 *)ADDR)[written]));
            written--;              // since it failed
            break;
        } // if the write was not successful //
    } // loop to read the data flash //

    disable_df_write();

    return written;
} // write_data_flash //


/*!
    \brief Erase a data flash block.

    \param[in] ADDR Address in the block to erase.

    \return TRUE if erase was successful
            FALSE if erase was not successful
*/
BOOL erase_data_flash(const UInt16 ADDR)
{
    BOOL rv = TRUE;

    if(ADDR < DF_BLOCK_START || ADDR >= DF_BLOCK_END)
    {
        return 0;
    } // if the parameters are invalid //

    enable_df_write();

    *((UInt8 *)ADDR) = ERASE_CYCLE1;
    *((UInt8 *)ADDR) = ERASE_CYCLE2;
    while(!COMPLETE());
    if(!ERASE_SUCCESS())
    {
        clr_status_reg((UInt8 *)ADDR);
        rv = FALSE;
    } // if the write was not successful //

    disable_df_write();

    return rv;
} // erase_data_flash //

//! @} FLASH_pub_func
//!                     PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup FLASH_pri_func
//! \ingroup FLASH
//! @{

/*!
    \brief Gets the processor ready for writing to the flash.

    This function also disables global interrupts.

    \param void

    \return void
*/
void enable_df_write(void)
{
    DISABLE_GLOBAL_INTERRUPTS();
    fmr01 = 0;
    fmr01 = 1;
    fmr02 = 0;
    fmr02 = 1;
    fmr11 = 0;
    fmr11 = 1;
} // enable_df_write //


/*!
    \brief Disables processor rewrite mode

    This function also enables global interrupts.

    \param void

    \return void
*/
void disable_df_write(void)
{
    fmr01 = 0;
    ENABLE_GLOBAL_INTERRUPTS();
} // disable_df_write //


/*!
    \brief Clears the status register

    \param addr[in/out] Pointer to the location to use to clear the status
      register

    \return void
*/
void clr_status_reg(UInt8 * addr)
{
    if((UInt16)addr >= DF_BLOCK_START && (UInt16)addr < DF_BLOCK_END)
    {
        *addr = CLR_STATUS_REG;
    } // if the address if valid //
} // clr_status_reg //

//! @} FLASH_pri_func
//!                     PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} FLASH
