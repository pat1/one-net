#ifndef _ONE_NET_MESSAGE_H
#define _ONE_NET_MESSAGE_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_packet.h"
#include "one_net_port_const.h"


//! \defgroup ONE-NET_MESSAGE ONE-NET Message Definitions
//! \ingroup ONE-NET
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
    \file one_net_message.h
    \brief Declarations for ONE-NET messages

    This is global ONE-NET message information.
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MESSAGE_const
//! \ingroup ONE-NET_MESSAGE
//! @{


/*!
    \brief Data Packet Message Types
*/
typedef enum
{
    ON_APP_MSG,                     //!< Application message type
    ON_ADMIN_MSG,                   //!< Admin message type
    ON_FEATURE_MSG                  //!< A request for features
} on_msg_type_t;


//! Data type being sent/received
typedef enum
{
    ON_NO_TXN,                      //!< No data being sent or received
    ON_INVITE,                      //!< Invitation packet.
    ON_SINGLE,                      //!< Sending a single data packet
    ON_BLOCK,                       //!< Sending a block data packet
    ON_STREAM,                      //!< Sending a stream data packet
    ON_RESPONSE                     //!< Response packet
} on_data_t;


/*!
    \brief Priority Levels

    \Note Changing these may cause the implementation of the Fragment delays
      admin messages to be changed.
*/
typedef enum
{
    ONE_NET_NO_PRIORITY,            //!< No current transaction
    ONE_NET_LOW_PRIORITY,           //!< Low priority transaction
    ONE_NET_HIGH_PRIORITY,          //!< High priority transaction
    ONE_NET_SEND_SINGLE_PRIORITY = ONE_NET_HIGH_PRIORITY
} on_priority_t;


extern const on_encoded_did_t NO_DESTINATION;


//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{


#ifndef _ONA_MSG_CLASS_T
#define _ONA_MSG_CLASS_T
    
// note : ON_STATUS_QUERY_RESP and ON_STATUS_FAST_QUERY_RESP are subsets of status messages.
// They just add a little more information, which the application is free to ignore.  In
// particular, for applications using peer lists where the status of one peer
// directly results in the changing of the state of another device, generally if, say,
// a light switch is flipped, an ONA_STATUS_CHANGE message will be sent and any
// recipient of that message through the peer list will interpret the
// ONA_STATUS_CHANGE (or possibly all status messages) as a command.  Whether to do
// that should be considered application specific.  Generally an ONA_STATUS_CHANGE message
// will be sent when an internal event like a switch flip occurs, ONA_STATUS will be
// sent when issuing regular time-based status updates, and the "status responses" will
// be issued when ACKing or NACKing a command or a query of some type.

// Thus if an application wants to treat all status updates the same, it should not check
// to see whether the message class is ONA_STATUS.  It should instead use the
// ONA_IS_STATUS_MESSAGE(X) macro as a bit-mask.


typedef UInt16 ona_msg_class_t;


//!< Status of a unit (not part of an ACK -- or part of an ACk but not
//!< fit any of the categories below)
#define ONA_STATUS                 0x0000

//!< Status of a unit has changed(i.e. switch has been flipped)
#define ONA_STATUS_CHANGE          0x1000

//!< Status of a unit (in the ACK in response to a query)
#define ONA_STATUS_QUERY_RESP      0x2000

//!< Status of a unit (in the ACK in response to a fast query)
#define ONA_STATUS_FAST_QUERY_RESP 0x3000

//!< Status of a unit (in the ACK in response to a command)
#define ONA_STATUS_COMMAND_RESP    0x4000

//!< Command to change status of a unit
#define ONA_COMMAND                0x5000

//!< Query status of a unit
#define ONA_QUERY                  0x6000

//!< Fast Query / "poll" status of a unit
#define ONA_FAST_QUERY             0x7000

// TODO - looks like we have a spare bit here.  Range is 0 to 7, but we have
// 4 bits reserved.  Shall we reserve only 3 bits instead, leave 8 to 15 for
// future use, or allow 8 to 15 to be user defined?


//!< Used to mask message class bits
#define ONA_MSG_CLASS_MASK         0xF000

//!< Status Message Macro.  This can be used to quickly ascertain whether he message
//!< is a status message if all status messages are to treated the same.
#define ONA_IS_STATUS_MESSAGE(X) ((X & ONA_MSG_CLASS_MASK) <= ONA_STATUS_COMMAND_RESP)


#endif // _ONA_MSG_CLASS_T


/*!
    \brief Raw Admin Message Type
*/
typedef enum
{
    //! Queries the fetures of the device and sends its own features
    ON_FEATURES_QUERY = 0x00,

    //! Response to a features query.  It can also be sent autonomously
    //! without a request for features whenever a device feels that
    //! another device needs to know its features.
    ON_FEATURES_RESP = 0x01,
    
    //! Replacing the first four bytes of the key and adding these four
    //! bytes at the end
    ON_NEW_KEY_FRAGMENT = 0x02,
    
    #ifdef _EXTENDED_SINGLE
    //! Replacing all 16 bytes of the key
    ON_NEW_KEY = 0x03,
    #endif
    
    #ifdef _STREAM_MESSAGES_ENABLED
    //! Replacing the first four bytes of the stream key and adding these four
    //! bytes at the end
    ON_NEW_STREAM_KEY_FRAGMENT = 0x04,
    
    #ifdef _EXTENDED_SINGLE
    //! Replacing all 16 bytes of the stream key
    ON_NEW_STREAM_KEY = 0x05,
    #endif
    #endif

    #ifdef _BLOCK_MESSAGES_ENABLED
    //! Change both high and low fragment delays in one message
    ON_CHANGE_FRAGMENT_DELAY = 0x08,
    #endif
    
    ON_KEY_CHANGE_CONFIRM = 0x09,
    
    #ifdef _STREAM_MESSAGES_ENABLED
    ON_STREAM_KEY_CHANGE_CONFIRM = 0x0A,
    #endif
    
    //! Sent to change the keep alive interval
    ON_CHANGE_KEEP_ALIVE = 0x0B,

    #ifdef _PEER
    //! Sent by the MASTER to assign a peer to the receiving CLIENT.  The CLIENT
    //! can then send directly to the peer.
    ON_ASSIGN_PEER = 0x0C,

    //! Sent by the MASTER to un-assign a peer from the receiving CLIENT.  The
    //! CLIENT must not send directly to that peer anymore.
    ON_UNASSIGN_PEER = 0x0D,
    #endif

    //! Query for the Keep Alive Timeout.  This is the interval at which a
    //! CLIENT must attempt to check in with the MASTER.  Any communication
    //! with the MASTER resets this timer.
    ON_KEEP_ALIVE_QUERY = 0x0E,
    
    //! Sent by a client to check in with the master whenever the keep-alive
    //! timer expires.
    ON_KEEP_ALIVE_RESP = 0x0F,
    
    //! Sent to change a device's settings.  The devices settings should not
    //! be considered changed until a SETTINGS_RESP is received.
    ON_CHANGE_SETTINGS = 0x10,
    
    //! Query settings
    ON_QUERY_SETTINGS = 0x11,

    //! Sent by the MASTER when it is adding a device to the network
    ON_ADD_DEV = 0x21,

    //! Sent by the MASTER when it is removing a device from the network
    ON_RM_DEV = 0x22
} on_admin_msg_t;



  

// TODO - is this the correct spot for on_single queue constants and functions?
// Should it have its own files?


//!< Single Message Data Queue Structure.
typedef struct
{
    UInt8 pid; 
    UInt8 priority;   
	on_encoded_did_t dst_did;
    UInt8 msg_type;
    UInt8* payload;
    UInt8 payload_size;
	on_encoded_did_t src_did;
    #ifdef _PEER
	BOOL send_to_peer_list;
    UInt8 src_unit;
    #endif
    #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
	tick_t send_time;
    #endif
    #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	tick_t expire_time;
    #endif
} on_single_data_queue_t;


//! Combining three common elements of a message to save stack space when
//! calling functions.
typedef struct
{
    UInt8 pid; //! PID of message
    UInt8 msg_id; //! message id of this transaction
    UInt8 msg_type; //! message type (i.e. admin, app, etc.)
} on_msg_hdr_t;


/*!
    \brief A combination of a did and a unit
*/
typedef struct
{
    //! did of the of the device
    on_encoded_did_t did;
	
    //! The unit of the device.
    UInt8 unit;
} on_did_unit_t;


/*!
    \brief The list of recipients to send for THIS message.  ONE-NET can fill
           this based on the peer list, if relevant.  The application code
           will also be given a chance to adjust this list.
*/
typedef struct
{
    //! List of recipients to send to for THIS message
    on_did_unit_t recipient_list[ONE_NET_MAX_RECIPIENTS];

    //! The number of recipients for THIS transaction
    UInt8 num_recipients;
   
    //! Index into the list.  Negative number signifies that sending to the
    //! recipient list either has not started, has finished, or is not
    //! relevant
    SInt8 recipient_index;
} on_recipient_list_t;





//! @} ONE-NET_MESSAGE_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_MESSAGE_pub_var
//! \ingroup ONE-NET_MESSAGE
//! @{

extern UInt8 single_data_queue_size;


//! The list of recipients to send to for THIS message
extern on_recipient_list_t recipient_send_list;

//! Pointer to the list of recipients to send to for THIS message.  Generally
//! will point either to NULL or recipient_send_list.  However, the user is
//! allowed to provide their own recipient lists to override this list
extern on_recipient_list_t* recipient_send_list_ptr;


//! @} ONE-NET_MESSAGE_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MESSAGE_pub_func
//! \ingroup ONE-NET_MESSAGE
//! @{


#ifdef _DEBUGGING_TOOLS
void get_queue_memory(UInt8** pld_buffer, on_single_data_queue_t** queue,
         UInt8* queue_size, UInt16* tail_idx);
#endif

void empty_queue(void);


#if _SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
// return true if an element was popped, false otherwise.
BOOL pop_queue_element(on_single_data_queue_t* const element,
    UInt8* const buffer, UInt8 index);
#else
BOOL pop_queue_element(void);
#endif

#if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
int single_data_queue_ready_to_send(tick_t* const queue_sleep_time);
#else
int single_data_queue_ready_to_send(void);
#endif

on_single_data_queue_t* push_queue_element(UInt8 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef _PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if _SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t send_time_from_now
  #endif
  #if _SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t expire_time_from_now
  #endif
  );
  

#ifdef _ONE_NET_CLIENT
BOOL must_send_to_master(const on_single_data_queue_t* const element);
#endif



void clear_recipient_list(on_recipient_list_t* rec_list);
BOOL did_and_unit_equal(const on_did_unit_t* const dev1,
  const on_did_unit_t* const dev2);
BOOL remove_recipient_from_recipient_list(on_recipient_list_t* rec_list,
  const on_did_unit_t* const recipient_to_remove);
BOOL add_recipient_to_recipient_list(on_recipient_list_t* rec_list,
  const on_did_unit_t* const recipient_to_add);
on_single_data_queue_t* load_next_recipient(on_single_data_queue_t* msg,
  on_recipient_list_t* recipient_list);
  
BOOL device_should_stay_awake(const on_encoded_did_t* const did);



//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MESSAGE

#endif // _ONE_NET_MESSAGE_H //
