#ifndef _ONE_NET_MEMORY_H
#define _ONE_NET_MEMORY_H



//! \defgroup one_net_memory Functionality for allocating "heap" memory
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
    \file one_net_memory.h
    \brief memory declarations.

    Declarations for allocation of memory.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_types.h"


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


typedef struct
{
    UInt8 index;
    UInt8 size;
} heap_entry_t;


//! @} one_net_memory_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup one_net_memory_pub_func
//! \ingroup one_net_memory
//! @{
    
    
void* one_net_malloc(UInt8 size);
void* one_net_calloc(UInt8 size, UInt8 value);
void* one_net_realloc(void* ptr, UInt8 size);
void one_net_free(void* ptr);

// temporary debugging --  will be deleted
#ifdef _DEBUGGING_TOOLS
void print_mem(void);
UInt8* get_loc(UInt8 index);
#endif


//! @} one_net_memory_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} one_net_memory


#endif // _ONE_NET_MEMORY //

#endif // _ONE_NET_MEMORY_H //

