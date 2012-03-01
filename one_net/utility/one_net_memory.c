//! \addtogroup one_net_memory
//! @{

/*
    Copyright (c) 2012, Threshold Corporation
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
    \file one_net_memory.c
    \brief Basis for memory allocation.

    This file is application independent.  The functionality implemented here
    is also independent of the device being a MASTER or CLIENT.  Some of the
    functions do make calls that are dependent on the device being a MASTER
    or CLIENT, or some other hardware dependent calls, but the implementation
    for those calls are elsewhere.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_port_const.h"
#include "one_net_port_specific.h"
#include "one_net_memory.h"


#ifdef _ONE_NET_MEMORY



//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_memory_const
//! \ingroup one_net_memory
//! @{

//! @} one_net_memory_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_memory_typedefs
//! \ingroup one_net_memory
//! @{

//! @} one_net_memory_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup one_net_memory_pri_var
//! \ingroup one_net_memory
//! @{


static UInt8 heap_buffer[ONE_NET_HEAP_SIZE];
static heap_entry_t heap_entry[ONE_NET_HEAP_NUM_ENTRIES] = {0};


//! @} one_net_memory_pub_var
//                              PRIVATE VARIABLES END
//==============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup one_net_memory_pri_func
//! \ingroup one_net_memory
//! @{

//! @} one_net_memory_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup one_net_memory_pub_func
//! \ingroup one_net_memory
//! @{

/*
    \brief Allocates memory

    Allocates a block of size bytes of memory, returning a pointer to the
    beginning of the block.  The content of the newly allocated block of
    memory is not initialized,remaining with indeterminate values.


    \param[in] size Size of the requested memory block, in bytes.
   
    \return On success, a pointer to the memory block allocated by the function.
            The type of this pointer is always void*, which can be cast to the
            desired type of data pointer in order to be dereferenceable.  If the
            function failed to allocate the requested block of memory, a null
            pointer is returned.

*/
void* one_net_malloc(UInt8 size)
{
    SInt8 i;

    if(heap_entry[ONE_NET_HEAP_NUM_ENTRIES - 1].size != 0)
    {
        return NULL; // all slots taken
    }
    
    if(size == 0 || size > ONE_NET_HEAP_SIZE)
    {
        return NULL; // requested size is 0 or bigger than buffer.
    }
    
    // see if everything is empty
    if(heap_entry[0].size == 0)
    {
        // use the first spot
        heap_entry[0].index = 0;
        heap_entry[0].size = size;
        return &heap_buffer[0];
    }
    
    // see if the last slot is free
    for(i = 1; i < ONE_NET_HEAP_NUM_ENTRIES; i++)
    {
        if(heap_entry[i].size == 0)
        {
            if(size <= ONE_NET_HEAP_SIZE - heap_entry[i-1].index -
              heap_entry[i-1].size)
            {
                heap_entry[i].index = heap_entry[i-1].index +
                  heap_entry[i-1].size;
                heap_entry[i].size = size;
                return &heap_buffer[heap_entry[i].index];
            }
            break; // last slot is not free
        }
    }
    
    // now see if there are any gaps.  First check the first index.
    if(size <= heap_entry[0].index)
    {
        // move everything up 1
        one_net_memmove(&heap_entry[1], &heap_entry[0],
          (ONE_NET_HEAP_NUM_ENTRIES - 1) * sizeof(heap_entry_t));
        heap_entry[0].index = 0;
        heap_entry[0].size = size;
        return &heap_buffer[0];
    }
    
    // now look elsewhere
    for(i = 1; i < ONE_NET_HEAP_NUM_ENTRIES; i++)
    {
        if(heap_entry[i].size == 0)
        {
            return NULL; // no gaps
        }
        
        if(size <= heap_entry[i].index - heap_entry[i-1].index -
          heap_entry[i-1].size)
        {
            // move everything up 1
            one_net_memmove(&heap_entry[i+1], &heap_entry[i],
              (ONE_NET_HEAP_NUM_ENTRIES - 1) * sizeof(heap_entry_t));
            heap_entry[i].index = heap_entry[i-1].index + heap_entry[i-1].size;
            heap_entry[i].size = size;
            return &heap_buffer[heap_entry[i].index];
        }
    }
    
    // should never get here?
    return NULL;
}


/*
    \brief Allocates memory and sets it to a certain value

    This one is a little dangerous as it DOES NOT act like calloc from stdlib.h.
    It's embedded programming and we previously had a one_net_memmove function
    that failed when the memory overlapped in the wrong way, so it should have
    been called memcpy(note : this bug has since been fixed).  Same here.  This
    function takes ONE "size" variable, not TWO like calloc from stdlib.h, and
    the second parameter is the value to set the memory to.
    
    // TODO -- People will expect "one_net_calloc" to behave like it does in
    // stdlib.h.  Find another name for this
    


    \param[in] size Size of the requested memory block, in bytes.
    \param[in] value Byte value to set the memory to.
   
    \return On success, a pointer to the memory block allocated by the function.
            The type of this pointer is always void*, which can be cast to the
            desired type of data pointer in order to be dereferenceable.  If the
            function failed to allocate the requested block of memory, a null
            pointer is returned.

*/
void* one_net_calloc(UInt8 size, UInt8 value)
{
    void* buf = one_net_malloc(size);
    if(buf == NULL)
    {
        return NULL;
    }
    
    one_net_memset(buf, value, size);
    return buf;
}


/*
    \brief Deallocate space in memory

    A block of memory previously allocated using a call to one_net_malloc,
    one_net_calloc or one_net_realloc is deallocated, making it available
    again for further allocations.  Notice that this function leaves the value
    of ptr unchanged, hence it still points to the same (now invalid) location,
    and not to the null pointer.


    \param[in] ptr Pointer to a memory block previously allocated with
                   one_net_malloc, one_net_calloc or one_net_realloc to
                   be deallocated.  If a null pointer is passed as argument,
                   no action occurs.
   
    \return none
*/
void  one_net_free(void* ptr)
{
    UInt8 i;
    if(!ptr)
    {
        return;
    }
    for(i = 0; i < ONE_NET_HEAP_NUM_ENTRIES; i++)
    {
        if(heap_entry[i].size > 0)
        {
            if(ptr == &heap_buffer[heap_entry[i].index])
            {
                if(i < ONE_NET_HEAP_NUM_ENTRIES - 1)
                {
                    one_net_memmove(&heap_entry[i], &heap_entry[i+1],
                      (ONE_NET_HEAP_NUM_ENTRIES - i - 1) *
                      sizeof(heap_entry_t));
                }
                heap_entry[ONE_NET_HEAP_NUM_ENTRIES - 1].size = 0;
            }
        }
    }
} // one_net_free //





// temporary debugging
#ifdef _DEBUGGING_TOOLS
#include "oncli.h"
void print_mem(void)
{
    oncli_send_msg("Heap Entry\n");
    xdump((UInt8*) &heap_entry[0], sizeof(heap_entry));
    oncli_send_msg("Heap Buffer\n");
    xdump((UInt8*) &heap_buffer[0], ONE_NET_HEAP_SIZE);
}
UInt8* get_loc(UInt8 index)
{
    return &heap_buffer[index];
}
#endif



// TODO -- write one_net_calloc and one_net_realloc functions


//! @} one_net_memory_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup one_net_memory_pri_func
//! \ingroup one_net_memory
//! @{

//! @} one_net_memory_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


#endif // _ONE_NET_MEMORY //

//! @} one_net_memory

