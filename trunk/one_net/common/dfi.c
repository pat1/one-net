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


#include <one_net/port_specific/config_options.h>


#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //

#include <one_net/common/dfi.h>
#include <one_net/port_specific/flash.h>

#include <one_net/port_specific/one_net_port_specific.h>

#if 0 // for debugging
#include <one_net/port_specific/uart.h>
#endif


//==============================================================================
//								CONSTANTS
//! \defgroup dfi_const
//! \ingroup dfi
//! @{

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

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup dfi_pri_func
//! \ingroup dfi
//! @{

/*!
      \brief Return an index to the data flash block that is currently being used.

      At anytime in between calls to dfi write functions, we need to be able to 
      figure out which data flash block is current just by looking at the data in
      the two data blocks. If one of the two data blocks has at the first byte 
      something other than 0xFF and the other block has 0xFF in the first block, the
      current block is the block that does not start with 0xxFF. If both blocks
      start with 0xFF, we consider the first block (block 0) to be current. If 
      both blocks start with something other than 0xFF, we have a problem.
      
      \param 
      \return 0 for the first data flash block, 1 for the second data flash block,
      and -1 if neither block looks like it is current.
*/
int dfi_current_block(void)
{
    dfi_segment_hdr_t * ptr_first_segment_hdr;
    dfi_segment_hdr_t * ptr_second_segment_hdr;
    UInt8 * ptr_block0;
    UInt8 * ptr_block1;
    UInt16 free_in_block0;
    UInt16 free_in_block1;

    ptr_first_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_A_START);
    ptr_second_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_B_START);

    if ((ptr_first_segment_hdr->type != DFI_ST_UNUSED_FLASH_DATA)
      && (ptr_second_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA))
    {
        return(0);
    }
    else if ((ptr_first_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA)
      && (ptr_second_segment_hdr->type != DFI_ST_UNUSED_FLASH_DATA))
    {
        return(1);
    }
    else if ((ptr_first_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA)
      && (ptr_second_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA))
    {
        return(0);
    }
    else
    {
        //
        // neither block starts with a free (unsed) byte, this probably means
        // we are in the process of switching blocks, so we should be able
        // to detect the current block by counting the number of continguous
        // free (unused) bytes starting at the end of the block. the block 
        // with the fewer free bytes should be the current block.
        //
        free_in_block0 = 0;
        free_in_block1 = 0;

        // start at the end of the first block
        ptr_block0 = (UInt8 *) DFI_UINT16_TO_ADDR(DF_BLOCK_B_START-1);
        
        // start at the end of the second block
        ptr_block1 = (UInt8 *) DFI_UINT16_TO_ADDR(DF_BLOCK_END-1);

        // count free bytes in the first block
        while(*ptr_block0-- == DFI_ST_UNUSED_FLASH_DATA)
        {
            free_in_block0++;
        }

        // count free bytes in the second block
        while(*ptr_block1-- == DFI_ST_UNUSED_FLASH_DATA)
        {
            free_in_block1++;
        }

        if (free_in_block0 < free_in_block1)
        {
            return(0);
        }
        else
        {
            return(1);
        }
    }
} // dfi_current_block //
 

/*!
      \brief Find free space in the current data flash block.

      Scan the current block to see how much free space is available. Return
      a pointer to the beginning of the free space and how many bytes are
      available.
    
      \param[out] available Pointer to a UInt16 to hold the number bytes of free space found.
      \return Pointer to the first byte of space. 
*/
static UInt8 * dfi_find_free_space(UInt16 *available)
{
    int current_segment;
    dfi_segment_hdr_t * ptr_segment_hdr;
    dfi_segment_hdr_t * ptr_end_segment;
    UInt8 * result;
    UInt8 * flash_ptr;

    //
    // starting at the beginning of the current block find the first
    // free (unused) byte.
    //
    current_segment = dfi_current_block();
    if (current_segment == 0)
    {
        ptr_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_A_START);
        ptr_end_segment = (dfi_segment_hdr_t *) (DFI_UINT16_TO_ADDR(DF_BLOCK_A_START+DF_BLOCK_SIZE));
    }
    else
    {
        ptr_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_B_START);
        ptr_end_segment = (dfi_segment_hdr_t *) (DFI_UINT16_TO_ADDR(DF_BLOCK_B_START+DF_BLOCK_SIZE));
    }

    result = (UInt8 *) 0;
    while (ptr_segment_hdr < ptr_end_segment)
    {
#if 0 // for debugging
        uart_write("dfi: ff: sadr=", 14);
        uart_write_int8_hex((UInt8) (((UInt16)ptr_segment_hdr>>8) & 0xff));
        uart_write_int8_hex((UInt8) (((UInt16)ptr_segment_hdr) & 0xff));
        uart_write(" eadr=", 6);
        delay_ms(100);
        uart_write_int8_hex((UInt8) (((UInt16)ptr_end_segment>>8) & 0xff));
        uart_write_int8_hex((UInt8) (((UInt16)ptr_end_segment) & 0xff));
        uart_write(" type=", 6);
        uart_write_int8_hex(ptr_segment_hdr->type);
        uart_write("\n", 1);
        delay_ms(100);
#endif
        if (ptr_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA)
        {
            result = (UInt8 *) ptr_segment_hdr;
            break;
        }
        flash_ptr = (UInt8 *) ptr_segment_hdr;
        flash_ptr += ptr_segment_hdr->len + sizeof(dfi_segment_hdr_t);
        ptr_segment_hdr = (dfi_segment_hdr_t *) flash_ptr;
    }

    //
    // calculate how many bytes are free (unused) in the current block
    //
    if (ptr_segment_hdr >= ptr_end_segment)
    {
        *available = 0;
    }
    else
    {
        *available = ((UInt8 *)ptr_end_segment - (UInt8 *)ptr_segment_hdr); 
        *available -= sizeof(dfi_segment_hdr_t);
    }

#if 0 // for debugging
        uart_write("dfi: ff: free=", 14);
        uart_write_int8_hex((UInt8) ((*available>>8) & 0xff));
        uart_write_int8_hex((UInt8) ((*available) & 0xff));
        uart_write("\n", 1);
        delay_ms(100);
#endif
    return(result);
} // dfi_find_free_space //


/*!
      \brief Switch to the other data flash block, copying segments from the current block or erasing all segments except the ones specified.

      The other segment should be empty (contains all free space). The erase_flag
      indicates whether to erase segments or to copy segments during the switch.
n
      For the case where an erase is specified (erase_flag is TRUE), copy the
      the segment types supplied from the current block to the other block.
      All other segment types will not be copied, which means they will be erased.
      
      For the case where a copy is specified (erase_flag is FALSE), copy the last instance 
      of each segment type from the current block into the other block, except we want 
      to skip the type_being_added type since that is the type of segment that caused
      the block switch and a new version of that segment type will be added immediately
      after the block switch is complete. Then erase the current block so that the other
      block becomes the current block.

      \param[in] erase_flag If TRUE, then the erase all segments except for the list provided, 
      otherwise copy all segments except the one being added.
      \param[in] segment_type_list Pointer to an array of segment types.
      \param[in] segment_type_list_count Number of segment types in the segment_type_list.
      \param[in] type_being_added A segment type to not copy when doing a switch.
      \return 
*/
static void dfi_switch_blocks(BOOL erase_flag,
  const UInt8 * segment_type_list,
  UInt8 segment_type_list_count,
  dfi_segment_type_t type_being_added)
{
    int i;
    int current_block;
    dfi_segment_hdr_t * ptr_segment_hdr;
    UInt8 *ptr_other_block;
    UInt8 *ptr_current_block;

    current_block = dfi_current_block();
#if 0 // for debugging
    uart_write("dfi: sb: cb= ", 13);
    uart_write_int8_hex((UInt8) current_block);
    uart_write("\n", 1);
    delay_ms(100);
#endif
    if (current_block == 0)
    {
        ptr_other_block = DFI_UINT16_TO_ADDR(DF_BLOCK_B_START);
        ptr_current_block = DFI_UINT16_TO_ADDR(DF_BLOCK_A_START);
    }
    else
    {
        ptr_other_block = DFI_UINT16_TO_ADDR(DF_BLOCK_A_START);
        ptr_current_block = DFI_UINT16_TO_ADDR(DF_BLOCK_B_START);
    }

    // 
    // iterate over the segment_type_list array.
    //

    for (i=0; i<segment_type_list_count; i++)
    {
        //
        // copy the last instance of each segment type to the other block.
        // however, if this call is not an erase, then don't copy the
        // type_being_added segment type since we will be adding that type
        // of segment after the block switch is complete.
        //
        if ((erase_flag == FALSE) && (segment_type_list[i] == type_being_added))
        {
            continue;
        }
        else
        {
            //
            // find the last instance of segment_type_list[i]
            // and copy it to the other block.
            //
            ptr_segment_hdr = 
              (dfi_segment_hdr_t *) dfi_find_last_segment_of_type(
                (dfi_segment_type_t) segment_type_list[i]);
            if (ptr_segment_hdr != (dfi_segment_hdr_t *) 0)
            {
                // copy this segment to the first free locationin the
                // other data flash block.
                write_data_flash((UInt16)DFI_ADDR_TO_UINT16(ptr_other_block),
                  (UInt8 *) ptr_segment_hdr, ptr_segment_hdr->len + sizeof(dfi_segment_hdr_t));

                // update the pointer to the other block so it points to the
                // next free (unused) data.
                ptr_other_block += ptr_segment_hdr->len + sizeof(dfi_segment_hdr_t);
            }
        }
    } // for each segment type in the segment_type_list //

    //
    // erase the current block so that the other block becomes the current block
    //
    erase_data_flash(DFI_ADDR_TO_UINT16(ptr_current_block));

} // dfi_switch_blocks //
 

/*!
      \brief Find enough space for a new segment, write the segment
      header, and return a pointer to where the segment data should be 
      written.

      When data flash is erased each byte is set to 0xFF. This function
      seraches the current data flash block to find a contiguous block 
      of at least size bytes.  If there is not enough
      space in the current block, then we need to move to the other block.
      First we must copy the last instance of any segments in the current
      block (other than the segment type supplied) to the new block.
      Next, we write the segment header for the segment beign added,
      then we erase the current block making this new block the current
      block.

      \param type The segment type for which we are trying to find space.
      \param size The number of bytes of data that will be in the new segment.
      \return A pointer to where the data portion of the new segment can be
      stored, or 0 if there is an error.
*/
 
static UInt8 * dfi_add_segment(dfi_segment_type_t type, UInt16 size)
{
    UInt16 free_space;
    dfi_segment_hdr_t segment_hdr;
    UInt8 * result;
    UInt16 bytes_written;

    //
    // see if there is room for this segment in the current block
    //
    result = dfi_find_free_space(&free_space);
#if 0 // for debugging
    uart_write("dfi: as: type=", 14);
    uart_write_int8_hex((UInt8) type);
    uart_write(" size=", 6);
    uart_write_int8_hex((UInt8) size);
    uart_write(" free=", 6);
    uart_write_int8_hex((UInt8) ((free_space>>8) & 0xff));
    uart_write_int8_hex((UInt8) ((free_space) & 0xff));
    uart_write(" addr=", 6);
    uart_write_int8_hex((UInt8) (((UInt16)result>>8) & 0xff));
    uart_write_int8_hex((UInt8) (((UInt16)result) & 0xff));
    uart_write("\n", 1);
    delay_ms(100);
#endif

    if (free_space < size)
    {
        // no room for the new segment, we need to switch blocks
        dfi_switch_blocks(FALSE, dfi_segment_types_used, dfi_segment_types_used_count, type);
        result = dfi_find_free_space(&free_space);
        if (free_space < size)
        {
            // no room after switching blocks, something is wrong
            return((UInt8 *) 0);
        }
    }

    //
    // write the new segment header to the current block, and return a 
    // pointer to the data portion of the segment
    //
    segment_hdr.type = type;
    segment_hdr.len = size;
    bytes_written = write_data_flash(DFI_ADDR_TO_UINT16(result), (UInt8 *) &segment_hdr,
      sizeof(segment_hdr));
    if (bytes_written != sizeof(segment_hdr))
    {
        return((UInt8 *) 0);
    }

    result = result + sizeof(dfi_segment_hdr_t);

    return(result);

} // dfi_add_segment //


//! @} dfi_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup dfi_pub_func
//! \ingroup dfi
//! @{

UInt8 * dfi_find_last_segment_of_type(dfi_segment_type_t segment_type)
{
    int current_segment;
    dfi_segment_hdr_t * ptr_segment_hdr;
    dfi_segment_hdr_t * ptr_end_segment;
    UInt8 * flash_ptr;
    UInt8 * result;

    //
    // starting at the beginning of the current block find all instances
    // of the segment type given.
    //
    current_segment = dfi_current_block();
    if (current_segment == 0)
    {
        ptr_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_A_START);
        ptr_end_segment = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_A_START+DF_BLOCK_SIZE);
    }
    else
    {
        ptr_segment_hdr = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_B_START);
        ptr_end_segment = (dfi_segment_hdr_t *) DFI_UINT16_TO_ADDR(DF_BLOCK_B_START+DF_BLOCK_SIZE);
    }

    result = (UInt8 *) 0;
    while (ptr_segment_hdr < ptr_end_segment)
    {
        if (ptr_segment_hdr->type == segment_type)
        {
            result = (UInt8 *) ptr_segment_hdr;
        }
        else if (ptr_segment_hdr->type == DFI_ST_UNUSED_FLASH_DATA)
        {
            // stop looking when we reach a free segment
            break;
        }
        flash_ptr = (UInt8 *) ptr_segment_hdr;
        flash_ptr += ptr_segment_hdr->len + sizeof(dfi_segment_hdr_t);
        ptr_segment_hdr = (dfi_segment_hdr_t *) flash_ptr;
    }

    return(result);

} // dfi_find_last_segment_of_type //


UInt8 * dfi_write_segment_of_type(dfi_segment_type_t segment_type, UInt8 * data, UInt16 length)
{
    UInt8 *ptr_segment_data;
    UInt16 bytes_written;

    ptr_segment_data = dfi_add_segment(segment_type, length);
    if (ptr_segment_data != (UInt8 *) 0)
    {
        //
        // copy the data portion
        //
        bytes_written = write_data_flash(DFI_ADDR_TO_UINT16(ptr_segment_data), data, length);
        if (bytes_written != length)
        {
            return((UInt8 *) 0);
        }
    }

    return(ptr_segment_data);

} // dfi_write_segment_of_type //

void dfi_delete_segments_except_for(
  const UInt8 *segment_type_list,
  UInt8 segment_type_list_count)
{

    dfi_switch_blocks(TRUE, segment_type_list, segment_type_list_count, DFI_ST_NO_SEGMENT_TYPE);

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
