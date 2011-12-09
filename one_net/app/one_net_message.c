//! \addtogroup ONE-NET_MESSAGE
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
    \file one_net_acknowledge.c
    \brief Implementation of message functions, including the single essage queue

    Implementation of message functions, including the single essage queue.
*/


#include "config_options.h"
#include "one_net_types.h"
#include "one_net_port_const.h"
#include "one_net_message.h"
#include "one_net_acknowledge.h"
#include "one_net_port_specific.h"
#include "tick.h"
#include "one_net.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MESSAGE_const
//! \ingroup ONE-NET_MESSAGE
//! @{


const on_encoded_did_t NO_DESTINATION = {0xFF, 0xFF};


//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{

//! @} ONE-NET_MESSAGE_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{

#if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static void delete_expired_queue_elements(void);
#endif

//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_TIMER_pri_var
//! \ingroup ONE-NET_TIMER
//! @{


#if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
static UInt8 payload_buffer[SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE];
static on_single_data_queue_t single_data_queue[SINGLE_DATA_QUEUE_SIZE];
static UInt16 pld_buffer_tail_idx = 0;
#endif

UInt8 single_data_queue_size = 0;

#ifdef _ONE_NET_CLIENT
extern BOOL device_is_master;
extern BOOL client_joined_network;
extern on_master_t * const master;
#endif


//! @} ONE-NET_MESSAGE_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MESSAGE_pub_func
//! \ingroup ONE-NET_MESSAGE
//! @{



/*
    \brief Remove all messages from the queue
    
    \return void
*/
void empty_queue(void)
{
    single_data_queue_size = 0;
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    pld_buffer_tail_idx = 0;
    #endif
}


/*!
    \brief Add a single message to the queue.
    
    The message is either sent to the peer list or only to the specific device
    that is passed in.
    
    \param[in] pid The pid of the message.
    \param[in] msg_type The message type of the message(admin, application, etc.)
    \param[in] data The data to send.
    \param[in] data_len The length of DATA (in bytes).
    \param[in] priority The priority of the transaction.
    \param[in] src_did The source of the message (if NULL, the source will be
      assumed to be this device).
    \param[in] enc_dst The device the message is destined for.  This can be
      NULL if the message is to be sent to the peer list.
    \param[in] send_to_peer_list If true, the message will be sent to.
    \param[in] src_unit The unit that the message originated from.  Relevant
      only if sending to the peer list.
	\param[in] send_time_from_now Time to pause before sending.  NULL is interpreted as "send immediately"
	\param[in] expire_time_from_now If after this time, don't bother sending.  NULL is interpreted as "no expiration"
    
    \return pointer to the queue element if the queue add was successful
            NULL if error or no room in queue.
*/
on_single_data_queue_t* push_queue_element(UInt8 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t* send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t* expire_time_from_now
  #endif
  )
{
    on_single_data_queue_t* element = NULL;
    
    if(!raw_data)
    {
        return NULL; // invalid parameter
    }
    
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    if(single_data_queue_size >= SINGLE_DATA_QUEUE_SIZE)
    {
        return NULL; // no room in queue
    }
    if((data_len + pld_buffer_tail_idx) >
      SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE)
    {
        return NULL; // no room in queue
    }

    element = &single_data_queue[single_data_queue_size];
    #else
    if(single_msg_ptr != NULL)
    {
        return NULL;  // no room
    }
    single_msg_ptr = &single_msg;
    element= single_msg_ptr;
    #endif
    
    element->pid = pid;
    element->priority = priority;
    element->msg_type = msg_type;
    element->payload_size = data_len;
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    element->payload = &payload_buffer[pld_buffer_tail_idx];
    pld_buffer_tail_idx += data_len;
    #else
    element->payload = single_data_raw_pld;
    #endif

    one_net_memmove(element->payload, raw_data, data_len);
    
    if(src_did != NULL)
    {
        one_net_memmove(element->src_did, *src_did, ON_ENCODED_DID_LEN);
    }
    else
    {
        one_net_memmove(element->src_did,
          &(on_base_param->sid[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
    }
    
    if(enc_dst)
    {
        one_net_memmove(element->dst_did, *enc_dst, ON_ENCODED_DID_LEN);
    }
    else
    {
        one_net_memmove(element->dst_did, NO_DESTINATION, ON_ENCODED_DID_LEN);
    }
    
    #ifdef _PEER
	element->send_to_peer_list = send_to_peer_list;
    element->src_unit = src_unit;
    #endif
    
    #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
    element->send_time = 0;
    if(send_time_from_now)
    {
        element->send_time = *send_time_from_now;
    }
    #endif
    #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
    element->expire_time = 0;
    if(expire_time_from_now && *expire_time_from_now > 0)
    {
	    element->expire_time = get_tick_count() + *expire_time_from_now;
    }
    #endif
    
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    single_data_queue_size++;
    #endif
    return element;
}


#if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
// return true if an element was popped, false otherwise.
BOOL pop_queue_element(on_single_data_queue_t* const element,
    UInt8* const buffer, UInt8 index)
#else
BOOL pop_queue_element(void)
#endif
{
    #if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    UInt8 i;

    if(index >= single_data_queue_size)
    {
        // index out of range.
        return FALSE;
    }
    
    // we have a legitimate index.  Pop the element if the caller to the
    // function provided somewhere to store it.
    if(element != NULL && buffer != NULL)
    {
        one_net_memmove(element, &single_data_queue[index],
            sizeof(on_single_data_queue_t));
        element->payload = buffer;
        one_net_memmove(element->payload, single_data_queue[index].payload,
            single_data_queue[index].payload_size);
    }
    
    // now delete the element
    
    // first delete the payload memory it takes in the buffer.
    // TODO - more range checking.  Unnecessaray if the queue is doing what
    // it's supposed to
    if(index < single_data_queue_size - 1)
    {
        // we aren't popping the last element
        UInt16 this_msg_buffer_start_idx = single_data_queue[index].payload -
            &payload_buffer[0];
        UInt16 next_msg_buffer_start_idx = this_msg_buffer_start_idx +
            single_data_queue[index].payload_size;
        UInt16 bytes_to_move = pld_buffer_tail_idx - next_msg_buffer_start_idx;
        one_net_memmove(&payload_buffer[this_msg_buffer_start_idx],
            &payload_buffer[next_msg_buffer_start_idx], bytes_to_move);
            
        // payloads have been moved.  Now adjust the payload pointers
        for(i = index + 1; i < single_data_queue_size; i++)
        {
            single_data_queue[i].payload -= single_data_queue[index].payload_size;
        }
    }
    
    // adjust the tail index.
    pld_buffer_tail_idx -= single_data_queue[index].payload_size;
    
    // payload(s) have been adjusted.  Now move the queue elements themselves
    if(index < single_data_queue_size - 1)
    {
        one_net_memmove(&single_data_queue[index], &single_data_queue[index + 1],
            (single_data_queue_size - index - 1) * sizeof(on_single_data_queue_t));
    }
    
    // finally adjust the queue size
    single_data_queue_size--;
    return TRUE;
    
    #else
    {
        // there's nothing to copy since everything is already loaded if
        // it exists.
        BOOL ret_value = (single_data_queue_size == 0);
        single_data_queue_size == 0;
        return ret_value;
    }
    #endif
}


/*!
    \ brief Checks the the single message queue for messages ready to pop
	
	If there is a single data transaction in the queue and it's ready to
	be placed into a transaction, it will be placed into the single transaction spot
	
	Any unsent packets that have expired will be deleted.
	
	\param[out] next_sleep_time The earliest time the queue might have something to pop.
                If the value is 0 and the function returns -1, then the queue is empty and
                next_pop_time should be disregarded.  If the function returns a non-negative
                value, next_pop_time should be disregarded.
	
	\return index of an element ready to be popped.  If none, -1 is returned.
*/
#if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
int single_data_queue_ready_to_send(tick_t* const next_pop_time)
{
	int i, j;
    int* index;
    tick_t sleep_time;
	UInt8 priority = ONE_NET_HIGH_PRIORITY;
	tick_t cur_tick = get_tick_count();
	*next_pop_time = 0;
        
    #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
    delete_expired_queue_elements();
    #endif
    
    if(single_data_queue_size == 0)
    {
        return -1; // empty queue
    }
	
	// note that send_time equals 0 means send immediately

	
	// Go through the list and see if any packets are ready.  Try high priority, then low priority.
	// Note that priority started initialized as HIGH_PRIORITY.  Go through the list twice, once for
	// each priority.  We'll change priority at the bottom of the outer loop.  If anything is ready
    // to pop, pop it
	for(j = 0; j < 2; j++)
	{
	    for(i = 0; i < single_data_queue_size; i++)
	    {
            if(single_data_queue[i].priority != priority)
            {
                continue; // wrong priority
            }
            
		    if(single_data_queue[i].send_time <= cur_tick)
		    {
                // we're ready to pop this element.
                return i;
		    }
            
            sleep_time = single_data_queue[i].send_time - cur_tick;
            if(*next_pop_time == 0 || sleep_time < *next_pop_time)
            {
                *next_pop_time = sleep_time;
            }
		}
		 
		priority = ONE_NET_LOW_PRIORITY;
	}
	
	return -1; // nothing ready to pop.
}
#else
int single_data_queue_ready_to_send(void)
{
    if(single_data_queue_size == 0)
    {
        return -1;
    }
    
    #if _SINGLE_QUEUE_LEVEL == MIN_SINGLE_QUEUE_LEVEL
    if(single_data_queue[0].priority != ONE_NET_HIGH_PRIORITY)
    {
        UInt8 i;
        
        // go throught the list.  If we have a HIGH priority message, return the index
        for(i = 1; i < single_data_queue_size; i++)
        {
            if(single_data_queue[i].priority == ONE_NET_HIGH_PRIORITY)
            {
                return i;
            }
        }
    }
    #endif
    
    return 0;
}
#endif


#ifdef _ONE_NET_CLIENT
/*!
    \ brief Determiners whether a message must have an added message to the
            master tacked on to the end.
    
    The message will need to have this extra message sent if and only if...
    
    1) Device is a client.
    2) Device has joined the network.
    3) Device has the _SEND_TO_MASTER flag set.
    4) The message is a single message (as opposed to a block or stream)
    5) The message type is an ON_APP_MSG (as opposed to an admin message or
           a user-defined message message type.
    6) The message class is a "status-type" message.
    7) The message is not already being sent to the master as a recipient,
       either because the master is the recipient or the master is on the
       peer list (if relevant).
       
    Note that for devices where _PEER is enabled, the second part of
        condition #7 will be checked elsewhere rather than from this function.
	
	\param[in] element An on_single_data_queue_t object containing the message
                       to be sent.

	\return TRUE if an extra _SEND_TO_MASTER message needs to be sent.
*/
BOOL must_send_to_master(const on_single_data_queue_t* const element)
{
    return (!device_is_master && client_joined_network
      && (master->flags & ON_SEND_TO_MASTER)
      && element->msg_type == ON_APP_MSG && 
      ONA_IS_STATUS_MESSAGE(get_msg_class(element->payload)) &&
      !is_master_did(&(element->dst_did)));
}
#endif


//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{
    
    
#if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL
static void delete_expired_queue_elements(void)
{
    int i;
    tick_t cur_tick = get_tick_count();
    for(i = single_data_queue_size - 1; i >= 0 ; i--)
    {
        if(single_data_queue[i].expire_time > 0 &&
          single_data_queue[i].expire_time < cur_tick)
        {
            pop_queue_element(NULL, NULL, i);
        }
    }
}
#endif    


//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_MESSAGE
