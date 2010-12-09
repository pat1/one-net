//! \defgroup CLIENT_port_specific Implementaton of ONE-NET CLIENT Port
//!   Specific Functionality
//! @{

/*
    Copyright (c) 2010, Threshold Corporation
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
    \file client_port_specific.c
    \brief Common implementation for ONE-NET CLIENT port specific functionality.

    This file contains the common implementation (to the processor) for some of
    the CLIENT port specific ONE-NET functionality.  Interfaces come from
    client_port_specific.h and client_util.h.
*/

#include "one_net_types.h"

#include "flash.h"
#include "one_net_port_specific.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup CLIENT_port_specific_const
//! \ingroup CLIENT_port_specific
//! @{

//! @} CLIENT_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup CLIENT_port_specific_typedef
//! \ingroup CLIENT_port_specific
//! @{

//! Values for representing the type of data stored in non-volatile memory
typedef enum
{
    //! Value that represents start of unused flash data
    INVALID_FLASH_DATA = 0xFF,
    
    //! Value that represents the start of ONE-NET paramets
    ONE_NET_CLIENT_FLASH_DATA = 0x00
} nv_data_t;

//! Header to be saved when parameters are saved.
typedef struct
{
    UInt8 type;                     //!< type of data stored (see nv_data_t)
    UInt16 len;                     //!< Number of bytes that follow
} flash_hdr_t;

//! @} CLIENT_port_specific_typedef
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup CLIENT_port_specific_pri_var
//! \ingroup CLIENT_port_specific
//! @{

//! Location to store next chunk of data written to flash.
static UInt8 * nv_addr = (UInt8 *)DF_BLOCK_A_START;

//! @} CLIENT_port_specific_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup CLIENT_port_specific_pri_fun
//! \ingroup CLIENT_port_specific
//! @{

//! @} CLIENT_port_specific_pri_var
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup CLIENT_port_specific_pub_func
//! \ingroup CLIENT_port_specific
//! @{

/*!
    \brief Returns a pointer to the ONE-NET CLIENT parameters.

    \param[out] len The number of bytes the pointer points to.

    \return 0 if there are no valid parameters or a pointer to the parameters
      if they are valid.
*/
const UInt8 * read_param(UInt16 * const len)
{
#ifdef _DEBUGGER_USES_DATA_FLASH
    *len = 0;
    return NULL;
#else

    const UInt8 * PARAM_PTR = 0;

    if(!len)
    {
        EXIT();
    } // if the parameter is invalid //

    *len = 0;
    for(nv_addr = (UInt8 *)DF_BLOCK_START; (UInt16)nv_addr < DF_BLOCK_END
      && ((const flash_hdr_t *)nv_addr)->type != INVALID_FLASH_DATA;
      nv_addr += sizeof(flash_hdr_t) + ((const flash_hdr_t *)nv_addr)->len)
    {
        if(((const flash_hdr_t *)nv_addr)->type == ONE_NET_CLIENT_FLASH_DATA)
        {
            PARAM_PTR = nv_addr + sizeof(flash_hdr_t);
            *len = ((const flash_hdr_t *)nv_addr)->len;
        } // if the data is valid //
    } // loop to find the most current settings //

    return PARAM_PTR;
#endif
} // read_param //


/*!
    \brief Clears the contents of the data flash.
    
    \param void
    
    \return void
*/
void clr_flash(void)
{
#ifdef _DEBUGGER_USES_DATA_FLASH
    return;
#endif
    erase_data_flash(DF_BLOCK_A_START);
    erase_data_flash(DF_BLOCK_B_START);
} // clr_flash //


/*!
    \brief Checks to see if the data flash should be erased (and erases it if
      it does).

    Checks if the uart rx & tx pins are connected to indicate that the flash
    should be erased.
    
    \param void
    
    \return void
*/
//
// dje: Eliminated superfluous nested loops.  It's valid to check
// one time.
//
void flash_erase_check(void)
{
    UInt8 i;
#ifdef _DEBUGGER_USES_DATA_FLASH
    return;
#else
    FLASH_CHECK_TX_PIN_DIR = OUTPUT;
    FLASH_CHECK_RX_PIN_DIR = INPUT;

    // Note that all pullups were enabled in init_ports(), so, in
    // particular, the FLASH_CHECK_RX_PIN is pulled up.
    //
    // Loop to see if FLASH_CHECK_RX_PIN follows FLASH_CHECK_TX_pin
    //
    for (i = 0; i < 2; i++) {
        FLASH_CHECK_TX_PIN = !FLASH_CHECK_TX_PIN;
        if (FLASH_CHECK_RX_PIN != FLASH_CHECK_TX_PIN) {
            //
            // Pins not connected: Give a quick blink
            // of the Rx LED (the green one) and return
            //
            RX_LED = 1;
            FLASH_CHECK_TX_PIN = 0;
            delay_ms(125);
            RX_LED = 0;
            return;
        } // if the pins aren't connected //
    }
    //
    // Pins are connected: Erase the flash and give something like a
    // two second blink on the Tx LED (the red one).
    //
    TX_LED = 1;
    clr_flash();
    delay_ms(2000);
    TX_LED = 0;
#endif
} // flash_erase_check //


void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
#ifdef _DEBUGGER_USES_DATA_FLASH
    return;
#else
    const UInt16 BYTES_TO_WRITE = sizeof(flash_hdr_t) + PARAM_LEN;
    const UInt16 END_OF_WRITE_PTR = (UInt16)(nv_addr + BYTES_TO_WRITE);

    // if the type of nv_hdr is changed, need to change BYTES_TO_WRITE
    flash_hdr_t nv_hdr = {ONE_NET_CLIENT_FLASH_DATA, PARAM_LEN};

    if(!PARAM || !PARAM_LEN)
    {
        EXIT();
    } // if parameters are invalid //

    if((UInt16)nv_addr < DF_BLOCK_START || END_OF_WRITE_PTR >= DF_BLOCK_END)
    {
        if(!erase_data_flash(DF_BLOCK_A_START)
          || !erase_data_flash(DF_BLOCK_B_START))
        {
            EXIT();
        } // could not erase the blocks //
        
        nv_addr = (UInt8 *)DF_BLOCK_START;
    } // if the address is not in range //

    if(write_data_flash((UInt16)nv_addr, (const UInt8 *)&nv_hdr, sizeof(nv_hdr))
      != sizeof(nv_hdr) || write_data_flash((UInt16)nv_addr + sizeof(nv_hdr),
      PARAM, nv_hdr.len) != nv_hdr.len)
    {
        EXIT();
    } // if saving settings failed //

    nv_addr += sizeof(nv_hdr) + nv_hdr.len;
#endif
} // one_net_client_save_settings //

//! @} CLIENT_port_specific_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup CLIENT_port_specific_pri_func
//! \ingroup CLIENT_port_specific
//! @{

//! @} CLIENT_port_specific_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} CLIENT_port_specific
