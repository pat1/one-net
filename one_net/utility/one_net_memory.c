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
#include <stdlib.h> // for NULL definition


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

//! @} one_net_memory_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

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
    // TODO -- write function
    return NULL;
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
    // TODO -- write functon
} // one_net_free //


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

