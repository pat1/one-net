#ifndef _DFI_H
#define _DFI_H

#include "config_options.h"


#ifdef _NON_VOLATILE_MEMORY

#include "one_net.h"

//! \defgroup dfi Data flash Interface
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

/*!
    \file dfi.h

    \brief Contains declarations for functions associated with data flash interface
      routines.

    The data flash interface (dfi) is a set of functions that can be used when
    implementing ONE-NET port specific functions that access non volatile memory.
    The dfi was designed to allow application code to store data in data flash along
    with ONE-NET configuration information. ONE-NET calls one_net_client_save_settings
    or one_net_master_save_settings when ONE-NET configuration data has changed.
    For example, the processor specific (Renesas) implementation of one_net_client_save_settings
    uses the dfi functions to store the ONE-NET client configuration data to data
    flash so that the client device will remember it's ONE-NET configuration data
    when it is powered up after a power down.

    The details of accessing the processor's data flash is hidden by a hardware
    abstraction layer. The data flash hadware abstraction layer is defined in the
    file flash.h. A Renesas specific data flash hadware abstraction layer is
    implemented in the file flash.c.
*/

#include "one_net_types.h"
#include "one_net_peer.h"


//==============================================================================
//								CONSTANTS
//! \defgroup dfi_const
//! \ingroup dfi
//! @{

#ifdef CUNIT_TESTING
extern UInt8 sim_data_flash[];
#define DFI_UINT16_TO_ADDR(UINT16)     (&sim_data_flash[UINT16-DF_BLOCK_START])
#define DFI_ADDR_TO_UINT16(ADDR)       ((ADDR-(&sim_data_flash[0]))+DF_BLOCK_START)
#else
#define DFI_UINT16_TO_ADDR(UINT16)     ((UInt8 *) UINT16)
#define DFI_ADDR_TO_UINT16(ADDR)       ((UInt16) ADDR)
#endif

//! @} dfi_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup dfi_typedefs
//! \ingroup dfi
//! @{

//! Header to each data flash segment
typedef struct
{
    //!< type of data stored in the dfi segment (see dfi_segment_type_t)
    UInt8 type;

    //!< Number of bytes in the dfi segment
    UInt16 len;
} dfi_segment_hdr_t;


//! Identifies the type of data contained in a data eeprom segment
typedef enum
{
    //! Used to indicate no segment type
    DFI_ST_NO_SEGMENT_TYPE                    =    0x00,

    //! Value that represents start of unused flash data
//    DFI_ST_UNUSED_EEPROM_DATA =          0xFF,

    //! MANUFACTURING DATA
    //! Device manufacturing data type
    DFI_ST_DEVICE_MFG_DATA                    =     0x01,

    //! Start EEPROM address of Device manufacturing data
    DFI_EEPROM_DEVICE_MFG_DATA_OFFSET         =     0x00,

    //! Device manufacturing data size (in bytes)
    DFI_EEPROM_DEVICE_MFG_DATA_SIZE           =     0x16,


    //! MASTER SEETINGS
    //! Master settings data type
    DFI_ST_ONE_NET_MASTER_SETTINGS            =      0x02,

    //! Start EEPROM address of one net master settings data
    DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET =   (DFI_EEPROM_DEVICE_MFG_DATA_OFFSET + DFI_EEPROM_DEVICE_MFG_DATA_SIZE),    // 22

    //! Device one net master settings data size (in bytes)
    DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE   =  sizeof(on_base_param_t) + sizeof(on_master_param_t) +
                                                 ONE_NET_MASTER_MAX_CLIENTS * sizeof(on_client_t),

    //! CLIENT SEETINGS
    //! Client settings data type
    DFI_ST_ONE_NET_CLIENT_SETTINGS            =    0x03,

    //! Start EEPROM address of one net client settings data
    DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET =    (DFI_EEPROM_ONE_NET_MASTER_SETTINGS_OFFSET + DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE),

    //! Device one net client settings data size (in bytes)
    DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE   =  sizeof(on_base_param_t) + sizeof(on_master_t),


    //! PEER SEETINGS
    //! Peer settings data type
    DFI_ST_ONE_NET_PEER_SETTINGS              =    0x04,

    //! Start EEPROM address of one net peer settings data
    DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET   =  DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_OFFSET + DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE,

    #ifdef _PEER
    //! Device one net peer settings data size (in bytes)
    DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE     =  (ONE_NET_MAX_PEER_UNIT * sizeof(on_peer_unit_t)),
    #endif


    //! APPLICATION DATA 1
    //! General application data of user defined type 5
    DFI_ST_APP_DATA_1                            =    0x05,

    #ifdef _PEER
    //! Start EEPROM address of one net appilication 1 data
    DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET =  (DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET + DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE),
    #else
    //! Start EEPROM address of one net appilication 1 data
    DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET =  DFI_EEPROM_ONE_NET_PEER_SETTINGS_OFFSET,                                                //!
    #endif
	
    //! Device one net application 1 data size (in bytes)
    DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE   =    12, // TODO --  Where does the 12 come from?


    //! APPLICATION DATA 2
    //! General application data of user defined type 6
    DFI_ST_APP_DATA_2                             = 0x06,

    //! Start EEPROM address of one net appilication 2 data
    DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_OFFSET  =  (DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_OFFSET + DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE),

    //! Device one net application 2 data size (in bytes)
    DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_SIZE   =    0,



    //! APPLICATION DATA 3
    //! General application data of user defined type 7
    DFI_ST_APP_DATA_3                            =  0x07,

    //! Start EEPROM address of one net appilication 3 data
    DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_OFFSET =  (DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_OFFSET + DFI_EEPROM_ONE_NET_APPLICATION_2_DATA_SIZE),

    //! Device one net application 3 data size (in bytes)
    DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_SIZE   =    0,


    //! APPLICATION DATA 4
    //! General application data of user defined type 8
    DFI_ST_APP_DATA_4                            =  0x08,

    //! Start EEPROM address of one net appilication 4 data
    DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_OFFSET =  (DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_OFFSET + DFI_EEPROM_ONE_NET_APPLICATION_3_DATA_SIZE),

    //! Device one net application 4 data size (in bytes)
    DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_SIZE   =    0,

	
    //! EEPROM MANUFATURING SAVED
    //! On startup this byte is read, when the EEPROM is new and this address was never written, the vaue of this
    //! address is 0xFF
    //! when the program saves manufaturing data to the EEPROM it also writes a value of 0x01 to this address.

    //! eeprom manufactiring data of user defined type 9
    DFI_ST_ONE_NET_EEPROM_MANUFATURING_SAVED               =    0x09,

    //! Start EEPROM address of eeprom saved byte
    DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET =  (DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_OFFSET + DFI_EEPROM_ONE_NET_APPLICATION_4_DATA_SIZE),

    //! Device eeprom saved size (in bytes)
    DFI_EEPROM_ONE_NET_MANUFATURING_SAVED_EEPROM_SIZE      =  1,


    //! EEPROM MASTER SETTINGS SAVED
    //! On startup this byte is read, when the EEPROM is new and this address was never written, the vaue of this
    //! address is 0xFF
    //! when the program saves master settings data to the EEPROM it also writes a value of 0x01 to this address.
    //! When the program erasses the EEPROM it also writes the value of oxFF to this address


    //! General application data of user defined type 10
    DFI_ST_ONE_NET_EEPROM_MASTER_SAVED               =    10,

    //! Start EEPROM address of eeprom saved byte
    DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET =  (DFI_EEPROM_ONE_NET_EEPROM_MANUFATURING_SAVED_OFFSET +
                                                      DFI_EEPROM_ONE_NET_MANUFATURING_SAVED_EEPROM_SIZE),

    //! Device eeprom saved size (in bytes)
    DFI_EEPROM_ONE_NET_MASTER_SAVED_EEPROM_SIZE      =  1,



    //! EEPROM CLIENT SETTINGS SAVED
    //! On startup this byte is read, when the EEPROM is new and this address was never written, the vaue of this
    //! address is 0xFF
    //! when the program saves client settings data to the EEPROM it also writes a value of 0x01 to this address.
    //! When the program erasses the EEPROM it also writes the value of oxFF to this address


    //! General application data of user defined type 11
    DFI_ST_ONE_NET_EEPROM_CLIENT_SAVED               =    11,

    //! Start EEPROM address of eeprom saved byte
    DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET =  (DFI_EEPROM_ONE_NET_EEPROM_MASTER_SAVED_OFFSET +
                                                      DFI_EEPROM_ONE_NET_MASTER_SAVED_EEPROM_SIZE),

    //! Device eeprom saved size (in bytes)
    DFI_EEPROM_ONE_NET_CLIENT_SAVED_EEPROM_SIZE      =  1,


    //! EEPROM MODE SAVED
    //! On startup this byte is read, when the EEPROM is new and this address was never written, the vaue of this
    //! address is 0xFF.
    //! when the program saves operation mode to the EEPROM it also writes a value of
    //!  0x01 to this address, when device_is_master = 1
    //!  0x00 to this address. if device_is_master = 0 (it is a client)
    //! When the program erases the EEPROM it also writes the value of oxFF to this address


    //! General application data of user defined type 12
    DFI_ST_ONE_NET_EEPROM_MODE_SAVED               =    12,

    //! Start EEPROM address of eeprom saved byte
    DFI_EEPROM_ONE_NET_EEPROM_MODE_SAVED_OFFSET =  (DFI_EEPROM_ONE_NET_EEPROM_CLIENT_SAVED_OFFSET +
                                                      DFI_EEPROM_ONE_NET_CLIENT_SAVED_EEPROM_SIZE),

    //! Device eeprom saved size (in bytes)
    DFI_EEPROM_ONE_NET_MODE_SAVED_EEPROM_SIZE      =  1
} dfi_segment_type_t;
//! @} dfi_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup dfi_pub_var
//! \ingroup dfi
//! @{

// contains port specific list of segment types used
extern const UInt8 dfi_segment_types_used[];
extern const UInt8 dfi_segment_types_used_count;


extern BOOL eeprom_manufacturing_saved;
extern BOOL eeprom_master_saved;
extern BOOL eeprom_client_saved;

extern UInt8 manufacturing_settings[DFI_EEPROM_DEVICE_MFG_DATA_SIZE];
extern UInt8 client_settings[DFI_EEPROM_ONE_NET_CLIENT_SETTINGS_SIZE];
extern UInt8 master_settings[DFI_EEPROM_ONE_NET_MASTER_SETTINGS_SIZE];
#ifdef _PEER
extern UInt8 peer_settings[DFI_EEPROM_ONE_NET_PEER_SETTINGS_SIZE];
#endif
extern UInt8 user_pins_settings[DFI_EEPROM_ONE_NET_APPLICATION_1_DATA_SIZE];


//! @} dfi_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup dfi_pub_func
//! \ingroup dfi
//! @{

/*!
      \brief Find the last instance of a data flash segment of the specified type.

      Data flash segments start with a segment header (dfi_segment_hdr_t) that
      identifies the type of data in the segment. This function scans data flash
      and returned a pointer to the data portion of the last segment of the type
      specified. The pointer returned is only valid until the next dfi function is
      called that writes data to data flash. Data segments may be copied to other
      data flash segments when new data is written to data flash.

      The pointer returned by this function is only valid until the next
      call to a dfi interface function because segments may be moved from
      one block to another block when segments are added or erased.

      This interface is based on a data flash architecture where data flash
      memory is divided into two (or more) eraseable blocks. For example,
      the 2,048 bytes of Renesas R8C data flash is divided into two 1,048
      blocks where the finest granularity for erasing is a block at a time.

      \param segment_type The type of segment to find.

      \return Address of the segment header of the last segment of the type
      segment_type, or 0 if no segments of the type are found or some other error was
      encountered.
*/
#ifndef _ATXMEGA256A3B
UInt8 * dfi_find_last_segment_of_type(dfi_segment_type_t segment_type);
#else
UInt16 * dfi_find_last_segment_of_type(dfi_segment_type_t segment_type);
#endif

/*!
      \brief Write a data flash segment of the specified type to data flash

      Data flash segments start with a segment header (dfi_segment_hdr_t) that
      identifies the type of data in the segment. This function scans data flash
      and returned a pointer to the data portion of the last segment of the type
      specified. The pointer returned is only valid until the next dfi function is
      called that writes data to data flash. Data segments may be copied to other
      data flash segments when new data is written to data flash.

      The pointer returned by this function is only valid until the next
      call to a dfi interface function because segments may be moved from
      one block to another block when segments are added or erased.

      This interface is based on a data flash architecture where data flash
      memory is divided into two (or more) eraseable blocks. For example,
      the 2,048 bytes of Renesas R8C data flash is divided into two 1,048
      blocks where the finest granularity for erasing is a block at a time.

      \param segment_type The type of segment that is being written.
      \param data Pointer to the data to be written to data flash.
      \param length The number of data bytes in the segment.

      \return Address of the data portion of the segment just stored in data flash,
      , or 0 if the segment could not be written to data flash.
      encountered.
*/
#ifndef _ATXMEGA256A3B
UInt8 * dfi_write_segment_of_type(
  dfi_segment_type_t segment_type,
  UInt8 * data,
  UInt16 length);
#else
UInt16 * dfi_write_segment_of_type(
  dfi_segment_type_t segment_type,
  UInt8 * data,
  UInt16 length);
#endif

/*!
      \brief Erase all segments except the ones specified.

      Only the segment types in the list supplied will be copied to the
      free (empty) block and then the original block will be erased. This
      process will result in a switch from one data block to the next
      free data block.

      This interface is based on a data flash architecture where data flash
      memory is divided into two (or more) eraseable blocks. For example,
      the 2,048 bytes of Renesas R8C data flash is divided into two 1,048
      blocks where the finest granularity for erasing is a block at a time.

      \param[in] segment_type_list Pointer to an array of segment types.
      \param[in] segment_type_list_count Number of segment types in the segment_type_list.
      \return
*/
void dfi_delete_segments_except_for(
  const UInt8 * segment_type_list,
  UInt8 segment_type_list_count);



//! @} dfi_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} dfi


#endif // ifdef _NON_VOLATILE_MEMORY //
#endif // #ifdef _DFI_H //
