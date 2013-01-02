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

#ifdef NON_VOLATILE_MEMORY


/*!
    \file nv_hal.c
    \brief The ONE-NET Evaluation Project Hardware Abstraction Layer for non-volatile memory.
	
	2012 - By Arie Rechavel at D&H Global Enterprise, LLC., based on the Renesas Evaluation Board Project
*/



#include "nv_hal.h"
#include "one_net_eval.h" // for user_pin[] array and user_pin_t.
#include "one_net_types.h"
#include "dfi.h"
#include "one_net_application.h"
#include "io_port_mapping.h"
#include "tick.h"
#include "one_net_led.h"

#ifdef ONE_NET_CLIENT
#include "one_net_client.h"
#endif
#ifdef ONE_NET_MASTER
#include "one_net_master.h"
#endif
#ifdef PEER
#include "one_net_peer.h"
#endif


#include <eeprom_driver.h>

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
    \brief Erases the data eeprom.

    \param void

    \return TRUE if erasing the entire data eeprom was successful
            FALSE if erasing the entire data eeprom failed
*/
BOOL eval_erase_data_eeprom(void)
{
    UInt8 segment_type_list[] = { DFI_ST_DEVICE_MFG_DATA, DFI_ST_ONE_NET_EEPROM_MANUFACTURING_SAVED };
    UInt8 segment_type_list_size = sizeof(segment_type_list);

    //
    // delete all segments except for the manufacturing data segment type
    // and eeprom manufacturing saved data segment type
    //
    dfi_delete_segments_except_for(segment_type_list, segment_type_list_size);

    return TRUE;
} // eval_erase_data_eeprom //


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
    // using the dfi interface for accessing eeprom data from the eval board.
    //
    // for the master,
    //
    // the DFI_ST_ONE_NET_MASTER_SETTINGS segment will include
    // on_base_param_t, client_list, and on_master_param_t.
    //
    //
    // eeprom data segment type ONE_NET_PEER_SETTINGS will be used to store
    // peer assignment data.
    //

    //
    //
    // for the client,
    //
    // the DFI_ST_ONE_NET_CLIENT_SETTINGS segment will include on_base_param_t
    // and on_master_t.
    //
    //
    // eeprom data segment type ONE_NET_PEER_SETTINGS will be used to store
    // peer assignment data.
    //

    UInt16 * result;
    dfi_segment_type_t settings_segment_type;
    UInt16 nv_param_len;


    // first calculate the crc, length, and type of memory
    #ifdef PEER

    #if !defined(ONE_NET_CLIENT)
    on_base_param->crc = master_nv_crc(NULL, NULL);
    nv_param_len = MAX_MASTER_NV_PARAM_SIZE_BYTES;
    settings_segment_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
    #elif !defined(ONE_NET_MASTER)
    on_base_param->crc = client_nv_crc(NULL, NULL);
    nv_param_len = CLIENT_NV_PARAM_SIZE_BYTES;
    settings_segment_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
    #else
    if(device_is_master)
    {
        on_base_param->crc = master_nv_crc(NULL, NULL);
        nv_param_len = MAX_MASTER_NV_PARAM_SIZE_BYTES;
        settings_segment_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
    }
    else
    {
        on_base_param->crc = client_nv_crc(NULL, NULL);
        nv_param_len = CLIENT_NV_PARAM_SIZE_BYTES;
        settings_segment_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
    }
    #endif


    #else      //  #ifdef PEER


    #if !defined(ONE_NET_CLIENT)
    on_base_param->crc = master_nv_crc(NULL);
    nv_param_len = MAX_MASTER_NV_PARAM_SIZE_BYTES;
    settings_segment_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
    #elif !defined(ONE_NET_MASTER)
    on_base_param->crc = client_nv_crc(NULL);
    nv_param_len = CLIENT_NV_PARAM_SIZE_BYTES;
    settings_segment_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
    #else
    if(device_is_master)
    {
        on_base_param->crc = master_nv_crc(NULL);
        nv_param_len = MAX_MASTER_NV_PARAM_SIZE_BYTES;
        settings_segment_type = DFI_ST_ONE_NET_MASTER_SETTINGS;
    }
    else
    {
        on_base_param->crc = client_nv_crc(NULL);
        nv_param_len = CLIENT_NV_PARAM_SIZE_BYTES;
        settings_segment_type = DFI_ST_ONE_NET_CLIENT_SETTINGS;
    }
    #endif

    #endif   // #ifdef PEER

    //
    // Write the ONE-NET parameters
    //
    result = dfi_write_segment_of_type(settings_segment_type, nv_param,
      nv_param_len);
	  
	if (*result == 0xFFFF)
    {
        return FALSE;
    }

    //
    // Write the pin configuration using DFI_ST_APP_DATA_1 segment type
    //
    result = dfi_write_segment_of_type(DFI_ST_APP_DATA_1, (UInt8*) user_pin,
      sizeof(user_pin));
    if (*result == 0xFFFF)
    {
        return FALSE;
    }

#ifdef PEER
    //
    // write peer data using DFI_ST_ONE_NET_PEER_SETTINGS segment type
    //
    result = dfi_write_segment_of_type(DFI_ST_ONE_NET_PEER_SETTINGS,
      peer_storage, PEER_STORAGE_SIZE_BYTES);
    if (*result ==  0xFFFF)
    {
        return FALSE;
    }
#endif


    #ifdef ONE_NET_CLIENT

    #ifdef ONE_NET_MASTER
    if(device_is_master)
    {        
        // write a 1 to location DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET, 1);

        // write a 1 to location DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET, 1);
    }
    else if(!device_is_master)
    {
        // write a 1 to location DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET, 1);

        // write a 0 to location DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET, 0);
    }
	#endif

    if(!device_is_master)
    {
        // write a 1 to location DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET, 1);

        // write a 0 to location DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET
        eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET, 0);
    }
    #endif

    return TRUE;
} // eval_save //


one_net_status_t one_net_save_mfg_settings(const on_raw_sid_t* raw_sid,
  const one_net_xtea_key_t* invite_key)
{
    UInt8 mfg_data_segment[ON_RAW_SID_LEN + ONE_NET_XTEA_KEY_LEN];
    
    // TODO -- make sure manufacturing data does not already exist.  If so, return
    // ONS_FAIL if there is.
    
    one_net_memmove(mfg_data_segment, *raw_sid, ON_RAW_SID_LEN);
    one_net_memmove(&mfg_data_segment[ON_RAW_SID_LEN], *invite_key,
      ONE_NET_XTEA_KEY_LEN);
    
    if(*(UInt16 *)(dfi_write_segment_of_type(DFI_ST_DEVICE_MFG_DATA, &mfg_data_segment[0],
      sizeof(mfg_data_segment))) == 0xFFFF)
    {
        return ONS_FAIL;
    }

    // write a 1 to location DFI_EEPROM_ONE_NET_EEPROM_MANUFACTURING_SAVED_OFFSET
    eeprom_write_byte ((UInt16)DFI_EEPROM_ONE_NET_EEPROM_MANUFACTURING_SAVED_OFFSET, 1);
    eeprom_manufacturing_saved = TRUE;
    
    return ONS_SUCCESS;
}


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


#endif // #ifdef NON_VOLATILE_MEMORY //
