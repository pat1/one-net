//! \addtogroup ont_net_eval_hal ONE-NET Evaluation Hardware Abstraction Layer
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
    \file one_net_eval_hal.c
    \brief The ONE-NET evaluation project hardware abstraction layer.
*/

#pragma section program program_high_rom

#include <one_net/port_specific/config_options.h>

#include "one_net_eval_hal.h"

#include <one_net/port_specific/flash.h>
#include <one_net/one_net_client.h>
#include "one_net_eval.h"
#include <one_net/one_net_master.h>
#include <one_net/port_specific/one_net_port_specific.h>
#include <one_net/common/dfi.h>

#include <one_net/one_net_peer.h>


//=============================================================================
//                                  CONSTANTS
//! \defgroup ont_net_eval_hal_const
//! \ingroup ont_net_eval_hal
//! @{

//! @} ont_net_eval_hal_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ont_net_eval_hal_typedefs
//! \ingroup ont_net_eval_hal
//! @{

//! Header to be saved with the parameters that are saved.
typedef struct
{
    UInt8 type;                     //!< type of data stored (see nv_data_t)
    UInt16 len;                     //!< Number of bytes that follow
} flash_hdr_t;

//! @} ont_net_eval_hal_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ont_net_eval_hal_pri_var
//! \ingroup ont_net_eval_hal
//! @{

//! Location to store next chuck of data written to flash
static UInt8 * nv_addr = (UInt8 *)DF_BLOCK_A_START;

//! Points to the ONE-NET parameters to save.
static const UInt8 * ONE_NET_PARAM = 0;

//! The length of the ONE-NET parameters (in bytes)
static UInt16 one_net_param_len = 0;

//! The type of ONE-NET parameter being stored.  This can be either
//! ONE_NET_CLIENT_FLASH_DATA or ONE_NET_MASTER_FLASH_DATA
static UInt8 one_net_param_type = DFI_ST_UNUSED_FLASH_DATA;

//! @} ont_net_eval_hal_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ont_net_eval_hal_pri_func
//! \ingroup ont_net_eval_hal
//! @{

// defined in one_net_master.c, for eval board only
extern void on_master_force_save(void);

// defined in one_net_client.c, for eval board only
extern void on_client_force_save(void);

//! @} ont_net_eval_hal_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC VARAIBLES DECLARATIONS
//! \defgroup ont_net_eval_hal_pub_data
//! \ingroup ont_net_eval_hal
//! @{

//! Only the dfi_segment_type_t's in the following table will be copied
//! to the new data flash block when the current data flash block is full.
const UInt8 dfi_segment_types_used[] =
{
    DFI_ST_ONE_NET_MASTER_SETTINGS,
    DFI_ST_ONE_NET_CLIENT_SETTINGS,
    DFI_ST_APP_DATA_1,
    DFI_ST_APP_DATA_2,
    DFI_ST_APP_DATA_3 
};

//! the number of entries in dfi_segment_types_used
const UInt8 dfi_segment_types_used_count = sizeof(dfi_segment_types_used);

//! @} ont_net_eval_hal_pub_data
//                      PUBLIC VARAIBLES DECLARATIONS END
//=============================================================================


//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ont_net_eval_hal_pub_func
//! \ingroup ont_net_eval_hal
//! @{

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
    
    FLASH_CHECK_TX_PIN_DIR = OUTPUT;
    FLASH_CHECK_RX_PIN_DIR = INPUT;

    // Note that all pullups were enabled in init_ports(), so, in
    // particular, the FLASH_CHECK_RX_PIN is pulled up.
    
    FLASH_CHECK_TX_PIN = 0;

    //
    // Loop to see if FLASH_CHECK_RX_PIN follows FLASH_CHECK_TX_pin
    //
    for (i = 0; i < 2; i++) {
        FLASH_CHECK_TX_PIN = !FLASH_CHECK_TX_PIN;
		delay_ms(2); // dje: give the rx time to get through RS-232
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
    eval_erase_data_flash();
    delay_ms(2000);
    TX_LED = 0;

} // flash_erase_check //


/*!
 *     \brief Return a pointer to the ONE-NET parameter block.
 *
 *     If this looks like cheating, then you are observant. It is cheating.
 *     Within the eval CLI, we would like to print some ONE-NET information for 
 *     which there is no public interface to obtain the information. So,
 *     we save a pointer to the data structures ONE-NET uses so that we
 *     have access to data such as the client list. We realize that this
 *     is dependent on the internal implementation details of ONE-NET 
 *     which may change without changing the public interface to ONE-NET.
 *     Whatever you do, don't tell Jay that we did this.
 *
 *     \param 
 *     \return 
 */
UInt8 * oncli_get_param(void)
{

    return(ONE_NET_PARAM);

} // oncli_get_param //

void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
    //
    // NOTE: since this is an eval board, we do not want to save the ONE-NET
    // settings whenever ONE-NET requests that we save them. if the device is
    // powered down before the CLI "save" command is issued, we do not want to
    // remember any changes that have been made. instead, we will save the ONE-NET
    // settings when the CLI "save" command is issued (see the eval_save function).
    //
    ONE_NET_PARAM = PARAM;
    one_net_param_len = PARAM_LEN;
    one_net_param_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
} // one_net_client_save_settings //


void one_net_master_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
    //
    // NOTE: since this is an eval board, we do not want to save the ONE-NET
    // settings whenever ONE-NET requests that we save them. if the device is
    // powered down before the CLI "save" command is issued, we do not want to
    // remember any changes that have been made. instead, we will save the ONE-NET
    // settings when the CLI "save" command is issued (see the eval_save function).
    //
    ONE_NET_PARAM = PARAM;
    one_net_param_len = PARAM_LEN;
    one_net_param_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
} // one_net_master_save_settings //


/*!
    \brief Erases the data flash.
    
    \param void
    
    \return TRUE if erasing the entire data flash was successful
            FALSE if erasing the entire data flash failed
*/
BOOL eval_erase_data_flash(void)
{
    UInt8 segment_type_list[] = { DFI_ST_DEVICE_MFG_DATA };
    UInt8 segment_type_list_size = sizeof(segment_type_list);
    
    //
    // delete all segments except for the manufacturing data segment type
    //
    dfi_delete_segments_except_for(segment_type_list, segment_type_list_size);

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
    UInt8 * one_net_param = 0;
    dfi_segment_hdr_t * ptr_segment_header;

    if(!len || !DATA)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    *DATA = 0;
    *len = 0;
    
    // find the segment type specified
    one_net_param = dfi_find_last_segment_of_type(NV_DATA_TYPE);
    if (one_net_param != (UInt8 *) 0)
    {
        // the segment was found, return a pointer to it and its length
        *DATA = one_net_param + sizeof(dfi_segment_hdr_t);
        ptr_segment_header = (dfi_segment_hdr_t *) one_net_param;
        *len = ptr_segment_header->len;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
} // eval_load //


#ifdef _ONE_NET_EVAL
/*!
    \brief Saves the data to non-volatile memory.
    
    If this function fails, it is possible the data in the flash is now corrupt.
    
    \param void

    \return TRUE if the data was saved successfully
            FALSE if the data was not saved successfully
*/
BOOL eval_save(void)
{
    // 
    // using the dfi interface for accessing data flash from the eval board.
    //
    // for the master,
    //
    // the DFI_ST_ONE_NET_MASTER_SETTINGS segment will include on_base_param_t 
    // and on_master_param_t.
    //
    // data flash segment type DFI_ST_APP_DATA_1 will be used to store the user pin
    // configuration information.
    //
    // data flash segment type DFI_ST_APP_DATA_2 will be used to store peer assignment
    // date (see extra_device_* variables below).
    //

    //
    //
    // for the client,
    //
    // the DFI_ST_ONE_NET_CLIENT_SETTINGS segment will include on_base_param_t,
    // on_master_t, and on_peer_t.
    //
    // data flash segment type DFI_ST_APP_DATA_1 will be used to store the user pin
    // configuration information.
    //
    
    // dje: The following wasn't initialized: failed when saving client data
    UInt8 * result;
#ifdef _PEER
    UInt8 * extra_device_data = NULL;
    UInt16 extra_device_data_len;
#endif
    dfi_segment_type_t settings_segment_type;
    UInt8 pin_type[NUM_USER_PINS];
	

    //
    // force the save since the data may contain nonces which will cause the crc
    // to be different, so we want it recomputed
    //
    if(device_type() == MASTER_NODE)
    {
#ifdef _PEER
        if(!master_get_peer_assignment_to_save(&extra_device_data,
          &extra_device_data_len))
        {
            return FALSE;
        } // if getting the master peer assignments failed //
#endif
        on_master_force_save();
        settings_segment_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
    } // if a MASTER //
    else if(device_type() == CLIENT_NODE)
    {
        on_client_force_save();
        settings_segment_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
    } // else if a CLIENT device //
    else
    {
        return FALSE;
    } // else don't save data for this device type //
    
    //
    // Write the ONE-NET parameters
    //
    if(ONE_NET_PARAM)
    {
        result = dfi_write_segment_of_type(settings_segment_type, ONE_NET_PARAM,
          one_net_param_len);
        if (result == (UInt8 *) 0)
        {
            return FALSE;
        }
    } 
    
    //
    // Write the user pin settings using DFI_ST_APP_DATA_1 segment type
    //
    get_user_pin_type(pin_type, sizeof(pin_type));
    result = dfi_write_segment_of_type(DFI_ST_APP_DATA_1, pin_type, sizeof(pin_type));
    if (result == (UInt8 *) 0)
    {
        return FALSE;
    }
        
    
#ifdef _PEER
    //
    // write extra device data using DFI_ST_APP_DATA_2 segment type
    //
    if(extra_device_data)
    {
        result = dfi_write_segment_of_type(DFI_ST_APP_DATA_2, extra_device_data,
          extra_device_data_len);
        if (result == (UInt8 *) 0)
        {
            return FALSE;
        }
    }
#endif
	   
    return TRUE;
} // eval_save //
#endif



//! @} ont_net_eval_hal_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ont_net_eval_hal_pri_func
//! \ingroup ont_net_eval_hal
//! @{

//! @} ont_net_eval_hal_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ont_net_eval_hal

