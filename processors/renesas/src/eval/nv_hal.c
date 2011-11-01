//! \addtogroup nv_hal The ONE-NET Evaluation Project Hardware Abstraction Layer for non-volatile memory.
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
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


#include "config_options.h"

#ifdef _NON_VOLATILE_MEMORY


/*!
    \file nv_hal.c
    \brief The ONE-NET Evaluation Project Hardware Abstraction Layer for non-volatile memory.
*/



#include "nv_hal.h"
#include "one_net_types.h"
#include "dfi.h"
#include "one_net_application.h"
#include "io_port_mapping.h"
#include "tick.h"
#include "one_net_led.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup nv_hal_const
//! \ingroup nv_hal
//! @{


//! @} nv_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup nv_hal_typedefs
//! \ingroup nv_hal
//! @{

//! Header to be saved with the parameters that are saved.
typedef struct
{
    UInt8 type;                     //!< type of data stored
    UInt16 len;                     //!< Number of bytes that follow
} flash_hdr_t;

//! @} nv_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup nv_hal_pri_var
//! \ingroup nv_hal
//! @{


//! @} nv_hal_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup nv_hal_pri_func
//! \ingroup nv_hal
//! @{



//! @} nv_hal_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC VARAIBLES DECLARATIONS
//! \defgroup nv_hal_pub_data
//! \ingroup nv_hal
//! @{



//! @} nv_hal_pub_data
//                      PUBLIC VARAIBLES DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup nv_hal_pub_func
//! \ingroup nv_hal
//! @{

/*!
    \brief Checks to see if the data flash should be erased (and erases it if
      it should -- i.e. erase if a "shorting device" is attached to the uart port).

    Checks if the uart rx & tx pins are connected to indicate that the flash
    should be erased (i.e. a person has physically attached a shorting plus
    that does this)
    
    \param void
    
    \return void
*/
void flash_erase_check(void)
{
    UInt8 i;
    
    FLASH_CHECK_TX_PIN_DIR = OUTPUT;
    FLASH_CHECK_RX_PIN_DIR = INPUT;

    // Note that all pullups were enabled in init_ports(), so, in
    // particular, the FLASH_CHECK_RX_PIN is pulled up.
    
    FLASH_CHECK_TX_PIN = 0;

    //
    // Loop to see if FLASH_CHECK_RX_PIN follows FLASH_CHECK_TX_pin
    //
    for (i = 0; i < 2; i++)
    {
        FLASH_CHECK_TX_PIN = !FLASH_CHECK_TX_PIN;
		delay_ms(2); // dje: give the rx time to get through RS-232
        if (FLASH_CHECK_RX_PIN != FLASH_CHECK_TX_PIN) 
        {
            //
            // Pins not connected: Give a quick blink
            // of the Rx LED (the green one) and return
            // since the shorting plug is not connected.  If it was, the two
            // pins would be physically connected and would therefore share
            // the same state and we would not get here
            FLASH_CHECK_TX_PIN = 0;
            rx_led_blink(125, 1);
            return;
        } // if the pins aren't connected //
    }
    //
    // Pins are connected: Erase the flash and give something like a
    // two second blink on the Tx LED (the red one).
    //
    eval_erase_data_flash();
    tx_led_blink(2000, 1);
} // flash_erase_check //


/*!
    \brief Erases the data flash.
    
    \param void
    
    \return TRUE if erasing the entire data flash was successful
            FALSE if erasing the entire data flash failed
*/
BOOL eval_erase_data_flash(void)
{
/*    UInt8 segment_type_list[] = { DFI_ST_DEVICE_MFG_DATA };
    UInt8 segment_type_list_size = sizeof(segment_type_list);
    
    //
    // delete all segments except for the manufacturing data segment type
    //
    dfi_delete_segments_except_for(segment_type_list, segment_type_list_size);
*/
    return TRUE;
} // eval_erase_data_flash //


/*!
    \brief Finds pointer to the data stored in non-volatile memory.

    This will be used to copy the data into nv_ram

    If looking for CLIENT data, but a newer set of MASTER parameters was found,
    FALSE is returned.  FALSE is also returned if looking for MASTER parameters,
    but a newer version of CLIENT data was found.

    \param[in]  NV_DATA_TYPE The type of data to restore.  See dfi_segment_type_t.
    \param[out] len The length (in bytes) of the data being restored.
    \param[out] DATA Pointer to retrieve the location of the data.

    \return TRUE if the data was loaded successfully
            FALSE if the data was not loaded successfully
*/
BOOL eval_load(const UInt8 NV_DATA_TYPE, UInt16 * const len,
  const UInt8 ** const DATA)
{
    // TODO -- write
    return FALSE;
} // eval_load //


/*!
    \brief Saves the data to non-volatile memory.

    If this function fails, it is possible the data in the flash is now corrupt.

    \param void

    \return TRUE if the data was saved successfully
            FALSE if the data was not saved successfully
*/
BOOL eval_save(void)
{
    // TODO -- write
    return TRUE;
} // eval_save //



//! @} nv_hal_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup nv_hal_pri_func
//! \ingroup nv_hal
//! @{

//! @} nv_hal_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} nv_hal


#endif // #ifdef _NON_VOLATILE_MEMORY //