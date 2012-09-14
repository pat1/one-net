//! \addtogroup dfi
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
    \file dfi.c
    \brief Contains functions for accessing circular buffers.

    Contains the implementation of functions for accessing circular buffers.
*/


#include "config_options.h"

#ifdef NON_VOLATILE_MEMORY

#include "dfi.h"
#include "one_net_port_specific.h"


#include <eeprom_driver.h>



//==============================================================================
//								CONSTANTS
//! \defgroup dfi_const
//! \ingroup dfi
//! @{


//! Only the dfi_segment_type_t's in the following table will be copied
//! to the new data flash block when the current data flash block is full.
const UInt8 dfi_segment_types_used[] =
{
    DFI_ST_DEVICE_MFG_DATA, /* TODO -- should this be on the list? */
    DFI_ST_ONE_NET_MASTER_SETTINGS,
    DFI_ST_ONE_NET_CLIENT_SETTINGS,
    #ifdef PEER
    DFI_ST_ONE_NET_PEER_SETTINGS,
    #endif
    DFI_ST_APP_DATA_1,
    DFI_ST_APP_DATA_2,
    DFI_ST_APP_DATA_3,
    DFI_ST_APP_DATA_4
    #ifdef _ATXMEGA256A3B
    ,DFI_ST_ONE_NET_EEPROM_MANUFATURING_SAVED,
    DFI_ST_ONE_NET_EEPROM_MASTER_SAVED,
    DFI_ST_ONE_NET_EEPROM_CLIENT_SAVED,
    DFI_ST_ONE_NET_EEPROM_MODE_SAVED
    #endif
};

//! the number of entries in dfi_segment_types_used
const UInt8 dfi_segment_types_used_count = sizeof(dfi_segment_types_used);


//! @} dfi_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup dfi_typedefs
//! \ingroup dfi
//! @{

//! @} dfi_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup dfi_pri_var
//! \ingroup dfi
//! @{

//! @} dfi_pri_var
//							PRIVATE VARIABLES END
//==============================================================================


BOOL eeprom_manufacturing_saved;
BOOL eeprom_master_saved;
BOOL eeprom_client_saved;


UInt8 manufacturing_settings[DFI_EEPROM_DEVICE_MFG_DATA_SIZE];
UInt8 client_settings[DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE];
UInt8 master_settings[DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE];
#ifdef PEER
UInt8 peer_settings[DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE];
#endif
UInt8 user_pins_settings[DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE];



//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup dfi_pri_func
//! \ingroup dfi
//! @{



//! this function returns the start address
UInt16 * dfi_find_last_segment_of_type(dfi_segment_type_t segment_type)
{
    UInt16 * ptr_start_address = 0;
    UInt16 start_address = 0xFFFF;

    ptr_start_address = &start_address;

    //
    // setting up  variables for start address and size of the egment type passed
    //

    switch(segment_type)
    {
      case DFI_ST_DEVICE_MFG_DATA:      //! 1
         start_address = DFI_EEPROM_DEVICE_MFG_DATA_OFFSET;
         break;

      case DFI_ST_ONE_NET_MASTER_SETTINGS:      //! 2
         start_address = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET;
         break;

      case DFI_ST_ONE_NET_CLIENT_SETTINGS:      //! 3
         start_address = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET;
         break;

#ifdef PEER
      case DFI_ST_ONE_NET_PEER_SETTINGS:        //! 4
         start_address = DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET;
         break;
#endif

      case DFI_ST_APP_DATA_1:   //! 5
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET;
         break;

      case DFI_ST_APP_DATA_2:   //! 6
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_OFFSET;
         break;

      case DFI_ST_APP_DATA_3:   //! 7
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_OFFSET;
         break;

      case DFI_ST_APP_DATA_4:   //! 8
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_OFFSET;
         break;

      case DFI_ST_ONE_NET_EEPROM_MANUFATURING_SAVED: ///! 9
      {
         start_address = DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET;
        break;
      }

      case DFI_ST_ONE_NET_EEPROM_MASTER_SAVED: ///! 10
      {
         start_address = DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET;
        break;
      }

      case DFI_ST_ONE_NET_EEPROM_CLIENT_SAVED: ///! 11
      {
         start_address = DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET;
        break;
      }

      default:
         start_address = 0xFFFF;
         break;
    }

    return (ptr_start_address);
} // dfi_find_last_segment_of_type //


UInt16 * dfi_write_segment_of_type(dfi_segment_type_t segment_type, UInt8 * data, UInt16 length)
{
    UInt16 * ptr_start_address = 0;
    UInt16 start_address = 0xFFFF;
    UInt16 segment_size = 0;

    ptr_start_address = &start_address;

    //
    // setting up  variables for start address
    //

    switch(segment_type)
    {
      case DFI_ST_DEVICE_MFG_DATA:      //! 1
         start_address = DFI_EEPROM_DEVICE_MFG_DATA_OFFSET;
         segment_size = DFI_EEPROM_DEVICE_MFG_DATA_SIZE;
         break;

      case DFI_ST_ONE_NET_MASTER_SETTINGS:      //! 2
         start_address = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE;
         break;

      case DFI_ST_ONE_NET_CLIENT_SETTINGS:      //! 3
         start_address = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE;
         break;

      #ifdef PEER
      case DFI_ST_ONE_NET_PEER_SETTINGS:        //! 4
         start_address = DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE;
         break;
      #endif

      case DFI_ST_APP_DATA_1:   //! 5
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE;
         break;

      case DFI_ST_APP_DATA_2:   //! 6
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_SIZE;
         break;

      case DFI_ST_APP_DATA_3:   //! 7
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_SIZE;
         break;

      case DFI_ST_APP_DATA_4:   //! 8
         start_address = DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_SIZE;
         break;

      case DFI_ST_ONE_NET_EEPROM_MANUFATURING_SAVED: ///! 9
         start_address = DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_MANUFATURING_SAVED_EEPROM_SIZE;
        break;


      case DFI_ST_ONE_NET_EEPROM_MASTER_SAVED: ///! 10
         start_address = DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_MASTER_SAVED_EEPROM_SIZE;
        break;

      case DFI_ST_ONE_NET_EEPROM_CLIENT_SAVED: ///! 11
         start_address = DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET;
         segment_size = DFI_EEPROM_ONE_NET_CLIENT_SAVED_EEPROM_SIZE;
        break;

      default:
         start_address = 0xFFFF;
         segment_size = 0;
         break;
    }

   // Check for data to write size
   if(length > segment_size)
   {
     start_address= 0xFFFF;
     // data size too big
     return(ptr_start_address);
   }

    if (start_address != 0xFFFF)
    {
        //
        // write to EEPROM the data portion
        //
        // write block EEPROM
        eeprom_write_block((UInt8 *)data, (UInt16)start_address, length);
    }

    return (ptr_start_address);
} // dfi_write_segment_of_type //


void dfi_delete_segments_except_for(
  const UInt8 *segment_type_list,
  UInt8 segment_type_list_count)
{
    UInt16 segment_size;
    UInt16 start_address = 0xFFFF;

    int j = 0;
    int i = 0;
    BOOL erase_segment = FALSE;

    // a block of data to write to eeprom
    /*
    UInt8 erase_block[DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE];   // which is the maximum length of all segments
    // initialize the erase_block to the values of 0xFF
    for (i=0; i<DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE; i++)
    {
       erase_block[i] = 0xFF;
    }
    */

    // iterate through the dfi_segment_types_list_used array
    // and check if the segment is included in the segment_type_list array
    // if it does set the erase segments array item to FALSE, which means (do not erase)
    for (j = 0; j < dfi_segment_types_used_count; j++)
    {
        // set the erase_segment flag to TRUE
        erase_segment = TRUE;

        for (i = 0; i < segment_type_list_count; i++)
        {
            // if the segment in the used segments type list is not equal to the segment in the passed segment typelist
            if(dfi_segment_types_used[j] == segment_type_list[i])
            {
                // do not erase the segment
                erase_segment = FALSE;
            }
        }

        // erase the segment if erase_segment is set to FALSE
        if(erase_segment)
        {
            // set the segment start address and segment size
            switch(dfi_segment_types_used[j])
            {
              case DFI_ST_DEVICE_MFG_DATA:      //! 1
                 start_address = DFI_EEPROM_DEVICE_MFG_DATA_OFFSET;
                 segment_size =  DFI_EEPROM_DEVICE_MFG_DATA_SIZE;
                 break;

              case DFI_ST_ONE_NET_MASTER_SETTINGS:      //! 2
                 start_address = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE;
                 break;

              case DFI_ST_ONE_NET_CLIENT_SETTINGS:      //! 3
                 start_address = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE;
                 break;

              #ifdef PEER
              case DFI_ST_ONE_NET_PEER_SETTINGS:        //! 4
                 start_address = DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE;
                 break;
              #endif

              case DFI_ST_APP_DATA_1:   //! 5
                 start_address = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE;
                 break;

              case DFI_ST_APP_DATA_2:   //! 6
                 start_address = DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_SIZE;
                 break;

              case DFI_ST_APP_DATA_3:   //! 7
                 start_address = DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_SIZE;
                 break;

              case DFI_ST_APP_DATA_4:   //! 8
                 start_address = DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_SIZE;
                 break;

              case DFI_ST_ONE_NET_EEPROM_MANUFATURING_SAVED:   //! 9
                 start_address = DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_MANUFATURING_SAVED_EEPROM_SIZE;
                 break;

              case DFI_ST_ONE_NET_EEPROM_MASTER_SAVED:   //! 10
                 start_address = DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_MASTER_SAVED_EEPROM_SIZE;
                 break;

              case DFI_ST_ONE_NET_EEPROM_CLIENT_SAVED:   //! 11
                 start_address = DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_CLIENT_SAVED_EEPROM_SIZE;
                 break;

              case DFI_ST_ONE_NET_EEPROM_MODE_SAVED:   //! 12
                 start_address = DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET;
                 segment_size = DFI_EEPROM_ONE_NET_MODE_SAVED_EEPROM_SIZE;
                 break;

              default:
                 start_address = 0xFFFF;
                 segment_size = 0;
                 break;
            }

            if( (start_address != 0xFFFF) && (segment_size > 0) )
            {
               // iterate through the segment size and write one byte at a time
               for(i = 0; i < segment_size; i++)
               {
                   eeprom_write_byte ((UInt16)(start_address+i), 0xFF);
               }
            }
       }
    }
} // dfi_delete_segments_except_for //



//! @} dfi_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup dfi_pri_func
//! \ingroup cb
//! @{

//! @} dfi_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} cb



#endif // ifdef NON_VOLATILE_MEMORY //
