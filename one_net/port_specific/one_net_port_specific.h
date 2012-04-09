#ifndef _ONE_NET_PORT_SPECIFIC_H
#define _ONE_NET_PORT_SPECIFIC_H


//! \defgroup ONE-NET_port_specific Application Specific ONE-NET functionality
//! \ingroup ONE-NET
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
    \file one_net_port_specific.h
    \brief Application specific ONE-NET declarations.

    The implementer of an application must implement the interfaces in this 
    file for the ONE-NET project to compile (and work).  This file contains 
    interfaces for those functions used by both the MASTER and the CLIENT in
    the ONE-NET project.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


#include "config_options.h"

#include <stdlib.h>
#include "one_net_types.h"
#include "one_net_constants.h"
#include "one_net_status_codes.h"
#include "one_net.h"
#include "one_net_acknowledge.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_specific_const
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_port_specific_typedefs
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_port_specific_pub_var
//! \ingroup ONE-NET_port_specific
//! @{

//! @} ONE-NET_port_specific_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_port_specific_pub_func
//! \ingroup ONE-NET_port_specific
//! @{

/*!
    \brief Copies LEN bytes from SRC to dst

    This function needs to be able to handle copying data when dst & SRC
    overlap.

    \param[out] dst The mem location to receive the bytes
    \param[in] SRC The bytes to copy.
    \param[in] LEN The number of bytes to copy.
    \return void
*/
void * one_net_memmove(void * dst, const void * SRC, size_t len);

/*!
    \brief Fills memory with a certain value.

    This function is just like memset from the string.h library except that
    I've changed the "value" parameter" from an int to a UInt8 to make it
    more clear that the function expects an unsigned character / byte and
    to avoid having to typecast.  I'm not sure why the real memset from
    string.h takes an int instead of an unsigned char, but since this is
    "one_net_memset", not "memset", I'll change it to a UInt8 explicitly.
    It won't hurt my feelings if people decide to change it back.

    \param[out] dst The mem location to receive the bytes
    \param[in] value The value of each byte in the memory
    \param[in] len The number of bytes to set.
    \return dst
*/
void * one_net_memset (void* dst, UInt8 value, size_t len);

/*!
    \brief Fills memory with one or more blocks of a value

    Sort of a hybrid between fread, memmove, and memset.  Allows you to set
    the memory of an array with a structure annd to do it more than once.
    The best way to explain is with an example...
    
    Suppose you had an array of 6 encoded Device IDs and you want to set that
    array so that all elements contained the address {0xB4, 0xBC};
    The following code would accomplish that...
    
    on_encoded_did_t c[6];
    on_encoded_did_t b = {0xB4, 0xBC};
    one_net_memset_block(c, sizeof(b), 6, b);
    
    
    After this code, c will contain
      {{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC},{0xB4, 0xBC}}
    
    
    \param[out] dst The mem location to receive the bytes
    \param[in] size Size of each element
    \param[in] count The number of elements to set
    \param[in] src The element to copy.
    \return dst
*/
void* one_net_memset_block(void* const dst, size_t size, size_t count,
  const void* const src);


/*!
    \brief Compares sequences of chars

    Compares sequences as unsigned chars

    \param[in] vp1  Pointer to the first  sequence
    \param[in] vp2  Pointer to the second sequence
    \param[in] n    The number of bytes to compare.
    \return -1 if the first  is smaller
            +1 if the second is smaller
             0 if they are equal
*/
SInt8 one_net_memcmp(const void *vp1, const void *vp2, size_t n);



/*!
    \brief Convert a byte stream to a 16-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 2 bytes to convert to a 16-bit value, accounting
      for the endianess of the processor.

    \return The 16 bit value contained in the 2 bytes of BYTE_STREAM
*/
UInt16 one_net_byte_stream_to_int16(const UInt8 * const BYTE_STREAM);


/*!
    \brief Convert a 16 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int16_to_byte_stream(const UInt16 VAL, UInt8 * const byte_stream);


/*!
    \brief Convert a byte stream to a 32-bit value, accounting for endianness

    \param[in] BYTE_STREAM The 4 bytes to convert to a 32-bit value, accounting
      for the endianess of the processor.

    \return The 32 bit value contained in the 4 bytes of BYTE_STREAM
*/
UInt32 one_net_byte_stream_to_int32(const UInt8 * const BYTE_STREAM);


/*!
    \brief Convert a 32 bit value to a MSB first stream.

    \param[in] VAL The value to convert to a byte stream
    \param[out] byte_stream The location to return the byte stream

    \return void
*/
void one_net_int32_to_byte_stream(const UInt32 VAL, UInt8 * const byte_stream);


/*!
    \brief Converts a raw did to a U16 value.
    
    \param[in] DID The device id to convert
    
    \return The UInt16 value corresponding to the DID.
*/
UInt16 did_to_u16(const on_raw_did_t *DID);


/*!
    \brief converts a U16 value to a raw DID
    
    \param[in] raw_did_int -- the UInt16 representation of the raw DID
      (0 - 4015 range)
    \param[out] raw_did -- the converted raw DID
    
    \return True if the conversion was successful, false otherwise
*/
BOOL u16_to_did(UInt16 raw_did_int, on_raw_did_t* raw_did);



/*!
    \brief Converts string to long integer.
    
    \param[in] str String to convert
    \param[in] endptr Reference to an object of type char*, whose value is set
        by the function to the next character in str after the numerical
        value.  This parameter can also be a null pointer, in which case it is
        not used.
    \param[in] base String to convert
    
    \return The "base" of the string representation.
*/
long int one_net_strtol(const char * str, char ** endptr, int base);


#ifdef _ONE_NET_MULTI_HOP
/*!
    \brief Application level code to change the number of hops and/or
           max hops for a device.
           
    This function should be used when the application code wants to
    change the TRANSACTION-SPECIFIC number of hops or the TRANSACTION-
    SPECIFIC maximum number of hops.  If the default behavior is to be used,
    this function should not change the behavior and should return
    ON_MSG_DEFAULT_BHVR.  To abort the transaction, ON_MSG_ABORT should be
    returned.  To override the hops and/or max_hops, the application
    code should change one or both of them and return ON_MSG_CONTINUE.
    
    This function is called when the previous number of hops FAILED and
    ONE-NET will try again with a new "hops" value.  It is also called
    BEFORE the first attempt is made.
    
    Again, this is a TRANSACTION-SPECIFIC function.  To change the number
    of hops for a device and have the change remain for future transactions,
    the application code should call either the one_net_set_hops() or the
    one_net_set_max_hops() functions.  This should be done when the
    application code has knowledge of how many hops it believes the
    transaction SHOULD take or the maximum possible number of hops the
    transaction SHOULD take.  If this is a multi-hop-capable device, but
    hops should NOT be taken, then max_hops should be set to 0.  If no
    inside knowledge of hops is known and no fine-tuning is desired, this
    should be an empty function and should return ON_MSG_DEFAULT_BHVR will
    use the default behavior.
    
    This function is particularly useful when the number of hops between
    devices varies either from data rate changes, distance changes,
    network changes, or atmospheric / enviroment changes.
    
    \param[in] raw_dst The raw device ID of the recipient.
    \param[in/out] max_hops The maximum number of hops to use
    
    \return ON_MSG_CONTINUE if changing the number of hops or max_hops.
    \return ON_MSG_DEFAULT_BHVR if no changes are to be made and default
            behavior should continue.
    \return ON_MSG_ABORT if the transaction should be aborted.
*/
on_message_status_t one_net_adjust_hops(const on_raw_did_t* const raw_dst,
  UInt8* const max_hops);
#endif



#ifndef _ONE_NET_SIMPLE_CLIENT
/*!
    \brief Allows the user to adjust the recipient list for a message

    This function is called after a single message has been popped
    from the queue and ready to send.  ONE-NET has set up a list of
    destination dids and destination units that the message will be sent to.
    The destination units are relevant only if the message type is ON_APP_MSG.
    
    The application code can do one of four things.
    
    1) Do nothing.  In this instance the list remains unchanged and this will
       be the list that is sent.
    2) Cancel the message.  If this is desired, *recipient_list should be set
       to NULL.
    3) A new list can replace the old list.  In this case the appliation code
       should change *recipient_list to point to the list it wants to have
       sent.
    4) The existing list can be used, but the applicaiton code can add to it,
       remove from it, or reorder it.
   

    Lists can be emptied by setting the "num_recipients" field to 0.  Elements
    can be added using the add_recipient_to_recipient_list function.  Elements
    can be removed using the "remove_recipient_from_recipient_list" function.

    
    \param[in] msg The message that is to be sent.
    \param[in/out] A pointer to a pointer to a list of recipients.  The list
                   itself can be changed by changing the pointer.  Change the
                   pointer to NULL to cancel the message.  See the main
                   description for how to adjust lists.
*/
void one_net_adjust_recipient_list(const on_single_data_queue_t* const msg,
  on_recipient_list_t** recipient_send_list);
#endif


#ifdef _DATA_RATE_CHANNEL
/*!
    \brief Called by ONE-NET to inform the application-level code that the
           data rate and/or channel has been changed.

    \param[in] new_channel The channel that ONE-NET has changed to.
    \param[in] new_data_rate The data rate that ONE-NET has changed to.

*/
void one_net_data_rate_channel_changed(UInt8 new_channel, UInt8 new_data_rate);
#endif


#ifndef _ONE_NET_SIMPLE_CLIENT
/*!
    \brief Allows the application code to override whether a nack reason is fatal

    If desired, the application code can change the is_fatal parameter.

    
    \param[in] nack_reason
    \param[in/out] is_fatal Whether ONE-NET has determined a NACK Reason to be fatal.  To override
                   ONE-NET's decision, change the is_fatal parameter.  Otherwise, do nothing
*/
void one_net_adjust_fatal_nack(on_nack_rsn_t nack_reason, BOOL* is_fatal);


/*!
    \brief Informs the application code when a single message has been loaded.

    Informs the application code when a single message has been loaded.
    The message can be changed, aborted, etc.

    
    \param[in/out] txn The transaction that is about to occur
    \param[in/out] msg The message that has just been loaded.
*/
void one_net_single_msg_loaded(on_txn_t** txn, on_single_data_queue_t* msg);
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
/*!
    \brief Informs the application code what a block or stream transfer has
           been requested and allows it to accept, modify, or reject the
           request.

           Informs the application code what a block or stream transfer has
           been requested and allows it to accept, modify, or reject the
           request.
    
    \param[in] bs_msg The parameters of the requested block or stream
                     transfer.
    \param[out] ack_nack An option to NACK the proposed transfer with a
                reason and a payload.
*/
void one_net_block_stream_transfer_requested(const block_stream_msg_t* const
  bs_msg, on_ack_nack_t* ack_nack);
  

/*!
    \brief Retrieves the next block payload to send
    
    \param[in] bs_msg The parameters of the requested block transfer,
               which includes the packet index we want.
    \param[out] buffer Buffer containing the bytes to send in the next packet
    \param[out] ack_nack The ack / nack associated with the termination, if any.
                
    \return ON_MSG_PAUSE to pause the block/stream transfer.  If no time is specified, the chunk pause will be used.  Otherwise, the
                the ack_nack parameter should be set to ON_ACK_PAUSE_TIME_MS and ack_nack->payload->ack_time_ms should
                be set to the time in milliseconds to pause.  Returning ON_MSG_PAUSE will cause the state to go to ON_BS_CHUNK_PAUSE.
            ON_MSG_CONTINUE if the buffer to send is acceptable and has been loaded.
            Any other value will terminate the message. This return value will be sent to any devices receiving this termination message.
*/
on_message_status_t one_net_block_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack);
#endif


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Retrieves the next stream payload to send
    
    \param[in] bs_msg The parameters of the requested block or stream transfer,
    \param[out] buffer Buffer containing the bytes to send in the next packet
    \param[out] ack_nack The ack / nack associated with the termination, if any.
                
    \return ON_MSG_PAUSE to pause the block/stream transfer.  If no time is specified, the chunk pause will be used.  Otherwise, the
                the ack_nack parameter should be set to ON_ACK_PAUSE_TIME_MS and ack_nack->payload->ack_time_ms should
                be set to the time in milliseconds to pause.  Returning ON_MSG_PAUSE will cause the state to go to ON_BS_CHUNK_PAUSE.
            ON_MSG_CONTINUE if the buffer to send is acceptable and has been loaded.
            Any other value will terminate the message. This return value will be sent to any devices receiving this termination message.
*/
on_message_status_t one_net_stream_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack);
#endif


#ifdef _DATA_RATE_CHANNEL
/*!
    \brief Finds an alternate channel to use.  This is an application-level
           function.

    \return The alternate channel to use.If negative, then no alternate channel
            could be found.
*/
SInt8 one_net_get_alternate_channel(void);
#endif




//! @} ONE-NET_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_port_specific

#endif // _ONE_NET_PORT_SPECIFIC_H //
