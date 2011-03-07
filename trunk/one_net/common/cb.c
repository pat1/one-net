//! \addtogroup cb
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
    \file cb.c
    \brief Contains functions for accessing circular buffers.

    Contains the implementation of functions for accessing circular buffers.
*/

#include <one_net/port_specific/config_options.h>


#ifdef _ONE_NET_EVAL
    #pragma section program program_high_rom
#endif // ifdef _R8C_TINY //


#include <one_net/common/cb.h>

#include <one_net/port_specific/one_net_port_specific.h>


//==============================================================================
//								CONSTANTS
//! \defgroup cb_const
//! \ingroup cb
//! @{

//! @} cb_const
//								CONSTANTS END
//==============================================================================

//==============================================================================
//								TYPEDEFS
//! \defgroup cb_typedefs
//! \ingroup cb
//! @{

//! @} cb_typedefs
//								TYPEDEFS END
//==============================================================================

//==============================================================================
//							PRIVATE VARIABLES
//! \defgroup cb_pri_var
//! \ingroup cb
//! @{

//! @} cb_pri_var
//							PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS
//! \defgroup cb_pri_func
//! \ingroup cb
//! @{

//! @} cb_pri_func
//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 
//! \defgroup cb_pub_func
//! \ingroup cb
//! @{

/*!
    \brief Dequeues bytes from the buffer

    \param[in/out] cb Pointer to the circular buffer record (cb_rec_t) to 
      be accessed.
    \param[out] buffer Pointer to a buffer for storing the bytes obtained from
      the circular buffer.
    \param[in] LEN The number of bytes to retrieve (sizeof buffer)

    \return The number of bytes read from the buffer.
*/
#ifdef _NEED_CB_DEQUEUE
UInt16 cb_dequeue(cb_rec_t * cb, UInt8 * buffer, const UInt16 LEN)
{
    UInt16 bytes_read = 0;

    if(!cb || !buffer || !cb->buffer || !LEN)
    {
        return 0;
    } // if any of the parameters are invalid //

    // see if there is data in the circular buffer
    if(cb->head == cb->tail)
    {
        return 0;
    } // if no bytes in the buffer //

    // check if the buffer wraps
    if(cb->head < cb->tail)
    {
        // The buffer wraps, so read the lesser of LEN, or to the end of the
        // buffer
        bytes_read = LEN <= cb->wrap - cb->tail ? LEN
          : cb->wrap - cb->tail + 1;

        one_net_memmove((void *)buffer, (void *)&(cb->buffer[cb->tail]),
          bytes_read);
        buffer += bytes_read;
        cb->tail += bytes_read;
        if(cb->tail > cb->wrap)
        {
            cb->tail = 0;
        } // if tail has gone beyond the wrap point //
    } // if the buffer wraps //

    if(bytes_read < LEN)
    {
        UInt16 bytes_to_read;

        bytes_to_read = LEN - bytes_read <= cb->head - cb->tail
          ? LEN - bytes_read : cb->head - cb->tail;

        one_net_memmove(buffer, &(cb->buffer[cb->tail]), bytes_to_read);
        cb->tail += bytes_to_read;
        bytes_read += bytes_to_read;
    } // if more bytes to read //

    return bytes_read;
} // cb_dequeue //
#else
//
// Get one byte from the queue
// If it's empty, return zero, else return 1;
//
UInt16 cb_getqueue(cb_rec_t *cb, UInt8 *data)
{
    // bad parameters or buffer empty: return 0
    if (!cb || !data || (cb->head == cb->tail)) {
        return 0;
    }
    *data = cb->buffer[cb->tail];
    if (++cb->tail > cb->wrap) {
        cb->tail = 0;
    }
    return 1;
}
#endif


/*!
    \brief Enqueues data into the circular buffer

    \param[in/out] cb Pointer to the circular buffer header (cb_header_t) to
      be accessed 
    \param[in] DATA The data to be put into the circular buffer.
    \param[in] LEN The number of bytes to write to the buffer.

    \return TRUE if the byte was added to the circular
            FALSE if the byte was not added
 */
#ifdef _NEED_CB_ENQUEUE
UInt16 cb_enqueue(cb_rec_t * const cb, const UInt8 * DATA, const UInt16 LEN)
{
    UInt16 bytes_written = 0;
    UInt16 last_free_position;      // The last free position if the buffer
                                    // will be filled with this write

    if(!cb || !cb->buffer || !LEN)
    {
        return FALSE;
    } // if any of the parameters are invalid //

    last_free_position = cb->tail ? cb->tail - 1 : cb->wrap;

    if(last_free_position < cb->head)
    {
        // +1 to include bound
        bytes_written = LEN <= cb->wrap - cb->head + 1 ? LEN
          : cb->wrap - cb->head + 1;
        one_net_memmove(&(cb->buffer[cb->head]), DATA, bytes_written);
        DATA += bytes_written;
        cb->head += bytes_written;
        if(cb->head > cb->wrap)
        {
            cb->head = 0;
        } // if head has gone beyond the wrap point //
    } // if we may need to wrap writing //

    if(bytes_written < LEN)
    {
        UInt16 bytes_to_write;
        
        bytes_to_write = LEN - bytes_written <= last_free_position - cb->head
          ? LEN - bytes_written : last_free_position - cb->head;

        one_net_memmove(&(cb->buffer[cb->head]), DATA, bytes_to_write);
        cb->head += bytes_to_write;
        bytes_written += bytes_to_write;
    } // if more data to be written //

    // see if the circular buffer is full.
    if(bytes_written != LEN)
    {
        // overflow has occurred
        cb->flags |= CB_FLAG_OVERFLOW;
    } // if an overflow occured //

    return bytes_written;
} // cb_enqueue //

//
// Try to put a single char into the queue. If there is room,
// put it into the buffer and return 1. 
//
// If the queue is already full, set overflow flag and return 0.
//
UInt16 cb_putqueue(cb_rec_t *cb , UInt8 data)
{
    cb_enqueue(cb, &data, 1);
}
#else
UInt16 cb_putqueue(cb_rec_t *cb , UInt8 data)
{
    UInt8  bytes_written = 0;
    UInt16 next_head_position; // The next free position if after
                               // this write

    // Trap illegal values
    if(!cb || !cb->buffer)
    {
        return 0;
    }
    // The "head" points to the next position to be written;
    //
    // We calculate what the new value will be after this byte
    // goes into ghe buffer.
    //
    // Note that tail==head if and only if it's empty, so we
    // never let head get equal to tail when writing. 
    // In other words, if the next position of the head would
    // be equal to the tail, it's a no-go.
    //
    next_head_position = cb->head + 1;
    if (next_head_position > cb->wrap) {
        next_head_position = 0;
    }

    if(next_head_position != cb->tail) {// can put a byte
        cb->buffer[cb->head] = data;
        cb->head = next_head_position;
        ++bytes_written;
    }
    if(bytes_written == 0) { // couldn't write: It's an overflow
        cb->flags |= CB_FLAG_OVERFLOW;
    }
    return bytes_written;
}
#endif

/*!
    \brief Return the capacity of the referenced circular buffer.

    The circular buffer record structure (cb_rec_t) contains information
    about the circular buffer. This function returns the maximum number
    of characters that can be stored in the circular buffer, which is not
    necessarily the size of the buffer.
    
    Because head == tail indicates the buffer is empty, the maximum number of
    bytes that can actually be stored in the buffer is 1 less than the sizeof
    the buffer.
      
    \param[in] CB Pointer to the circular buffer record (cb_header_t) to 
      be accessed.

    \return The total number of bytes that can be stored in this buffer.
*/
UInt16 cb_size(const cb_rec_t * const CB)
{
    if(!CB || !CB->buffer)
    {
        return 0;
    } // if any of the parameters are invalid //
    
    return CB->wrap;
} // cb_size //


/*!
    \brief Return the bytes used in the referenced circular buffer.

    The circular buffer record structure (cb_rec_t) contains information
    about the circular buffer. This function returns the current number
    of characters that are stored in the circular buffer. 
      
    \param[in] CB Pointer to the circular buffer record (cb_rec_t) to 
      be accessed .
    \return The total number of bytes currently in the circular buffer.
*/
UInt16 cb_bytes_queued(const cb_rec_t * const CB)
{
    if(!CB || !CB->buffer)
    {
        return 0;
    } // if any of the parameters are invalid //

    if(CB->head >= CB->tail)
    {
        // head has not wrapped to the start of the buffer
        return CB->head - CB->tail;
    } // if the head has not wrapped //

    // add 1 to include the bounds
    return (CB->wrap - CB->tail) + CB->head + 1;
} // cb_bytes_queued //


/*!
    \brief Return the number of bytes available in the referenced circular
      buffer.

    The circular buffer record (cb_rec_t) struture contains information
    about the circular buffer. This function returns the current number
    of characters that are free in the circular buffer. 
      
    \param[in] CB Pointer to the circular buffer header (cb_header_t) to 
      be accessed.

    \return The total number of bytes free in the circular buffer.
*/
UInt16 cb_bytes_free(const cb_rec_t * const CB)
{
    if(!CB || !CB->buffer)
    {
        return 0;
    } // if the parameter is invalid //

    return CB->wrap - cb_bytes_queued(CB);
} // cb_bytes_free //

//! @} cb_pub_func
//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 
//! \addtogroup cb_pri_func
//! \ingroup cb
//! @{

//! @} cb_pri_func
//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} cb
