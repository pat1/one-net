#ifndef _CB_H 
#define _CB_H 

#include "config_options.h"

//! \defgroup cb Circular Buffer functionality.
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
    \file cb.h

    \brief Contains declarations for functions associated with circular buffer
      routines.

    Circular buffers are used to transfer data between interrupt service
    routines and background threads. The circular buffer header (cb_rec_t)
    contains the information needed to manipulate a circular buffer.

    For the sake of efficiency, updating the circular buffers is done as quickly
    as possible. Only the cb_get function updates the tail member of the
    cb_rec_t structure and only the cb_put function updates the head member
    of the cb_header_t structure. For any particular circular buffer, if cb_get
    is called in an ISR, we expect that only cb_put will be called in the
    background (non-ISR). For any particular curcular buffer, if cb_put is
    called in an ISR, we expect that only cb_get will be called in the
    background (non-ISR). Given these restrictions, we feel it is safe to not
    disable interrupts in either cb_get or cb_put. 
*/

#include "one_net_types.h"


//==============================================================================
//								CONSTANTS
//! \defgroup cb_const 
//! \ingroup cb
//! @{

//! Error/Status flags
enum
{
    CB_FLAG_OVERFLOW = 0x01,        //!< attempt to write when full
    CB_FLAG_RAN_DRY  = 0x02         //!< transmit ISR found cb empty
};


enum
{
    //! index of next available byte
    CB_INIT_HEAD = 0,

    //! index of first used byte
    CB_INIT_TAIL = 0,

    //! used to track status of cb
    CB_INIT_FLAGS = CB_FLAG_RAN_DRY,
};

//! @} cb_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS 
//! \defgroup cb_typedefs
//! \ingroup cb
//! @{

//! Circular Buffer Record    
typedef struct
{
    UInt16 head;                    //!< next position to be written to
    UInt16 tail;                    //!< next position to be read from 
    UInt16 wrap;                    //!< size of circular buffer minus one
    UInt8 flags;                    //!< see CB_FLAG_* constants
    UInt8 * buffer;                 //!< ptr to the data buffer
} cb_rec_t;

//! @} cb_typedefs
//								TYPEDEFS END
//==========================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup cb_pub_var
//! \ingroup cb
//! @{

//! @} cb_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS
//! \defgroup cb_pub_func
//! \ingroup cb
//! @{

UInt16 cb_dequeue(cb_rec_t * const, UInt8 *, const UInt16);
UInt16 cb_enqueue(cb_rec_t * const, const UInt8 *, const UInt16);
UInt16 cb_getqueue(cb_rec_t *, UInt8 *);
UInt16 cb_putqueue(cb_rec_t *, UInt8);
UInt16 cb_size(const cb_rec_t * const);
UInt16 cb_bytes_queued(const cb_rec_t * const);
UInt16 cb_bytes_free(const cb_rec_t * const);

//! @} cb_pub_func
//						PUBLIC FUNCTION DECLARATIONS END
//==========================================================================

//! @} cb

#endif // #ifdef _CB_H //
