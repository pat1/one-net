#ifndef _ONE_NET_MESSAGE_H
#define _ONE_NET_MESSAGE_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_packet.h"
#include "one_net_port_const.h"
#include "one_net_features.h"


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
    ON_FEATURE_MSG,                 //!< A request for features
    #ifdef _ROUTE
    ON_ROUTE_MSG                    //!< A routing message
    #endif
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
#define ONA_STATUS                 0x000

//!< Status of a unit has changed(i.e. switch has been flipped)
#define ONA_STATUS_CHANGE          0x100

//!< Status of a unit (in the ACK in response to a query)
#define ONA_STATUS_QUERY_RESP      0x200

//!< Status of a unit (in the ACK in response to a fast query)
#define ONA_STATUS_FAST_QUERY_RESP 0x300

//!< Status of a unit (in the ACK in response to a command)
#define ONA_STATUS_COMMAND_RESP    0x400

//!< Command to change status of a unit
#define ONA_COMMAND                0x500

//!< Query status of a unit
#define ONA_QUERY                  0x600

//!< Fast Query / "poll" status of a unit
#define ONA_FAST_QUERY             0x700

// Message classes 0x800 to 0xF00 are currently unused


//!< Used to mask message class bits
#define ONA_MSG_CLASS_MASK         0xF00

//!< Used to shift the message class and message type bits
#define ONA_MSG_CLASS_TYPE_SHIFT 4

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
    
    //! Response from a client when a device has been added.
    ON_ADD_DEV_RESP = 0x04,
    
    //! Response from a client when a device has been removed.
    ON_REMOVE_DEV_RESP = 0x05,
    
    #ifdef _DATA_RATE
    //! Change data rate
    ON_CHANGE_DATA_RATE = 0x06,
    #endif
    
    #ifndef _ONE_NET_SIMPLE_CLIENT
    //! Sent by a client when a new key is needed for whatever reason.
    //! Generally this is sent when a client is about to run out of message
    //! ids or feels there has been some breach of security or some attempted
    //! breach of security.
    ON_REQUEST_KEY_CHANGE = 0x07,
    #endif

    #ifdef _BLOCK_MESSAGES_ENABLED
    //! Change both high and low fragment delays in one message
    ON_CHANGE_FRAGMENT_DELAY = 0x08,
    
    //! Response to changing of fragment delays
    ON_CHANGE_FRAGMENT_DELAY_RESP = 0x09,
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
    
    //! Sent in response to a change settings message.
    ON_CHANGE_SETTINGS_RESP = 0x12,
    
    #ifdef _BLOCK_MESSAGES_ENABLED
    ON_REQUEST_BLOCK_STREAM = 0x13,
    
    ON_REQUEST_REPEATER = 0x14,
    
    ON_TERMINATE_BLOCK_STREAM = 0x15,
    #endif

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
    UInt16 raw_pid; 
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
    UInt16 raw_pid; //! Raw PID of message
    UInt16 msg_id; //! message id of this transaction(0 to 4095)
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



// TODO -- add a regular expression expression matcher.  Will need to be
// careful because it will add a little to the code size and add a few calls
// to the call stack level.
/*!
    \brief A poor-man's limited regular expression to compare messages.
    
    The best way to show is to show a few examples.  For example, to test
    whether a message matches the following criteria...
    
    1.  Test whether a message is a command to turn a switch on or off.  Source
        unit is 3, destination unit is a unit from 0 to 3, inclusive.
        
        For the reg_exp_mask, pick 0xFCFFFFFFFE.  Masks should be chosen
        thoughtfully in order to match all "matching" messages and no
        "non-matching" messages.
        
        reg_exp_mask = 0xFCFFFFFFFE
        
        The corresponding "regular expression" would be (pick any message that
        matches this criteria and do an AND ).  For example, the one below...
        
        message = 0x3200000001 -- src = 3, dst = 2, msg type = 0, data = 1 (on)
        
        Now AND this with 0xFCFFFFFFFE and get 0x3000000000.  Any message that
        "matches" will, when ANDed with 0xFCFFFFFFFE, match 0x3000000000.
        Therefore...
        
        reg_exp = 0x3000000000
        
        len would be 5, since this is 5 bytes.
        
        
        To compare whether a message matches this criteria, Do an AND operation
        on the message and the regular expression mask.
        
        
    2.  As another example, to check whether an admin message is a keep-alive
        response, you can use a mask of 0xFF00000000 and a "regular expression"
        of 0x2100000000 and a length of 5.  Since the last 4 bytes are all 0,
        this can be reduced to a mask of 0xFF, a regular expression of 0x21, and
        a length of 1.
*/
typedef struct
{
    //! The regular expression
    UInt8* reg_exp;
    
    //! The regular expression mask
    UInt8* reg_exp_mask;
    
    // length of the regular expression
    UInt8 len;
} single_reg_exp_t;


/*!
    \brief Info for communicating with a device.

    This structure holds the information needed to receive from a device.
*/
typedef struct
{
    on_encoded_did_t did;           //!< Encoded Device ID of the sender
    on_features_t features;         //!< features of the device.
    #ifdef _ONE_NET_MULTI_HOP
    UInt8 max_hops;                 //!< May be different from max_hops in features and may change and may vary
                                    //!< between devices depending on distance, noise, past experience, etc.
                                    //!< This is the CURRENT maximum number of hops these two devices use
                                    //!< if / when they use multi-hop
    UInt8 hops;                     //!< The expected "best guess" of the current number of hops between the
                                    //!< two devices.  This may or may not change often. If conditions, distances,
                                    //!< and packet lengths tend to remain the same,this value will likely remain
                                    //!< the same.
    #endif
    UInt8 data_rate;                //!< The current data rate the device is using
    UInt16 msg_id;                  //!< The message id of the current or next transaction with this device(0 - 4095).
    tick_t verify_time;             //!< The last time the message id was verified for this device
} on_sending_device_t; 


#ifdef _BLOCK_MESSAGES_ENABLED

// we need to cram 3 parameters into 1 byte for the message.
// hops will be the 3 least significant bbits(0 - 2).
// priority will be the middle two bits(3 - 4)
// Block / Stream flag will be bit 5(byte 0 defined as least significant).
// 2 most significant bytes are unused in the actual message so we'll use them
// as follows.  Byte 7(MSB) will be true if this device is the source.  Byte
// 6 will be true if this device is the destination.  If neither are true,
// then be default we must be functioning strictly as a repeater between the
// source and the destination.  Or if we are the master, we are not involved
// in the transaction at all.

typedef enum
{
    ON_BLK_TRANSFER,
    ON_STREAM_TRANSFER
} on_bs_transfer_type_t;



#define BLOCK_STREAM_SETUP_DEVICE_IS_SRC_MASK  0x01
#define BLOCK_STREAM_SETUP_DEVICE_IS_SRC_SHIFT 7
#define BLOCK_STREAM_SETUP_DEVICE_IS_DST_MASK  0x01
#define BLOCK_STREAM_SETUP_DEVICE_IS_DST_SHIFT 6
#define BLOCK_STREAM_SETUP_TYPE_MASK           0x01
#define BLOCK_STREAM_SETUP_TYPE_SHIFT          5
#define BLOCK_STREAM_SETUP_PRIORITY_MASK       0x03
#define BLOCK_STREAM_SETUP_PRIORITY_SHIFT      3

// relevant only for multi-hop, but should be filled in as 0 for non-multi-hop
#define BLOCK_STREAM_SETUP_HOPS_MASK           0x07
#define BLOCK_STREAM_SETUP_HOPS_SHIFT          0

#define MAX_CHUNK_SIZE 40


enum
{
    BLOCK_STREAM_SETUP_FLAGS_IDX = 1,
    BLOCK_STREAM_SETUP_PRIORITY_IDX = 1,
    BLOCK_STREAM_SETUP_HOPS_IDX = 1,
    BLOCK_STREAM_SETUP_TRANSFER_SIZE_IDX = 2,
    BLOCK_STREAM_SETUP_CHUNK_SIZE_IDX = BLOCK_STREAM_SETUP_TRANSFER_SIZE_IDX +
        sizeof(UInt32),
    BLOCK_STREAM_SETUP_FRAG_DLY_IDX = BLOCK_STREAM_SETUP_CHUNK_SIZE_IDX +
        sizeof(UInt8),
    BLOCK_STREAM_SETUP_CHUNK_PAUSE_IDX = BLOCK_STREAM_SETUP_FRAG_DLY_IDX +
        sizeof(UInt16),
    BLOCK_STREAM_SETUP_CHANNEL_IDX = BLOCK_STREAM_SETUP_CHUNK_PAUSE_IDX +
        sizeof(UInt16),
    BLOCK_STREAM_SETUP_DATA_RATE_IDX = BLOCK_STREAM_SETUP_CHANNEL_IDX +
        sizeof(UInt8),
    BLOCK_STREAM_SETUP_TIMEOUT_IDX = BLOCK_STREAM_SETUP_DATA_RATE_IDX +
        sizeof(UInt8),
    BLOCK_STREAM_SETUP_DST_IDX = BLOCK_STREAM_SETUP_TIMEOUT_IDX +
        sizeof(UInt16),
    BLOCK_STREAM_SETUP_ESTIMATED_TIME_IDX = BLOCK_STREAM_SETUP_DST_IDX +
        ON_ENCODED_DID_LEN,
};


typedef struct
{
    // note -- repeaters will not be interested in many of these attributes
    UInt8 bs_on_state;
    BOOL transfer_in_progress;
    UInt8 flags;
    UInt32 transfer_size; // also used for the time in a stream transaction
                          // TODO -- make it a union or a different variable
                          // instead?
    UInt8 chunk_size;
    UInt16 frag_dly;
    UInt16 chunk_pause;
    UInt8 channel;
    UInt8 data_rate;
    UInt16 timeout;
    on_sending_device_t* src; // originator of block message
    on_sending_device_t* dst; // recipient of block message
    UInt32 time; // this value can represent a variety of things.  Generally
                 // for a repeater, it will represent the estimated time of
                 // completion.  For the sender or the recipient of a stream
                 // message, it will represent the start time of the stream
                 // transfer.  For block transfers, it will also generally
                 // represent the estimated completion tim eof the transfer.
    
    #ifdef _ONE_NET_MULTI_HOP
    UInt8 num_repeaters;
    on_encoded_did_t repeaters[ON_MAX_HOPS_LIMIT];
    #endif
    SInt32 byte_idx;
    SInt8 chunk_idx;
    UInt8 sent[MAX_CHUNK_SIZE / 8]; // 8 bits per byte.  "complete" is a
           // bitwise "boolean" array, with each bit representing whether a
           // certain packet within a chunk has been received.  0 means FALSE.
           // 1 means TRUE.
} block_stream_msg_t;


typedef struct
{
    UInt16 msg_id;
    UInt8 chunk_idx;
    UInt8 chunk_size;
    UInt32 byte_idx;
    UInt8* data;
} block_pkt_t;


// exact same as block_pkt_t except it has time instead of byte_idx.  You
// can typecast easily from one to the other given that the sizes and order of
// the elements are the same.
typedef struct
{
    UInt16 msg_id;
    UInt8 chunk_idx; // not really used much for stream packets
    UInt8 chunk_size; // not really used much for stream packets
    UInt32 time; // in milliseconds, not ticks
    UInt8* data;
} stream_pkt_t;


typedef union
{
    block_pkt_t block_pkt;
    stream_pkt_t stream_pkt;
} block_stream_pkt_t;
#endif



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

on_single_data_queue_t* push_queue_element(UInt16 pid,
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


#ifdef _BLOCK_MESSAGES_ENABLED
on_single_data_queue_t* send_bs_setup_msg(const block_stream_msg_t* bs_msg,
  const on_encoded_did_t* dst);
void admin_msg_to_block_stream_msg_t(const UInt8* msg, block_stream_msg_t*
  bs_msg, const on_encoded_did_t* src_did);
void block_stream_msg_t_to_admin_msg(UInt8* msg, const block_stream_msg_t*
  bs_msg);


ONE_NET_INLINE void set_bs_transfer_type(UInt8* flags,
  on_bs_transfer_type_t type)
{
    (*flags) &= ~(BLOCK_STREAM_SETUP_TYPE_MASK <<
      BLOCK_STREAM_SETUP_TYPE_SHIFT);
    (*flags) |= (type << BLOCK_STREAM_SETUP_TYPE_SHIFT);
}


ONE_NET_INLINE UInt8 get_bs_transfer_type(UInt8 flags)
{
    return ((flags >> BLOCK_STREAM_SETUP_TYPE_SHIFT) &
      BLOCK_STREAM_SETUP_TYPE_MASK);
}


ONE_NET_INLINE void set_bs_priority(UInt8* flags, UInt8 priority)
{
    (*flags) &= ~(BLOCK_STREAM_SETUP_PRIORITY_MASK <<
      BLOCK_STREAM_SETUP_PRIORITY_SHIFT);
    (*flags) |= (priority << BLOCK_STREAM_SETUP_PRIORITY_SHIFT);
}


ONE_NET_INLINE UInt8 get_bs_priority(UInt8 flags)
{
    return ((flags >> BLOCK_STREAM_SETUP_PRIORITY_SHIFT) &
      BLOCK_STREAM_SETUP_PRIORITY_MASK);
}


ONE_NET_INLINE void set_bs_hops(UInt8* flags, UInt8 hops)
{
    (*flags) &= ~(BLOCK_STREAM_SETUP_HOPS_MASK <<
      BLOCK_STREAM_SETUP_HOPS_SHIFT);
    (*flags) |= (hops << BLOCK_STREAM_SETUP_HOPS_SHIFT);
}


ONE_NET_INLINE UInt8 get_bs_hops(UInt8 flags)
{
    return ((flags >> BLOCK_STREAM_SETUP_HOPS_SHIFT) &
      BLOCK_STREAM_SETUP_HOPS_MASK);
}


ONE_NET_INLINE void set_bs_device_is_src(UInt8* flags, BOOL is_src)
{
    (*flags) &= ~(BLOCK_STREAM_SETUP_DEVICE_IS_SRC_MASK <<
      BLOCK_STREAM_SETUP_DEVICE_IS_SRC_SHIFT);
    (*flags) |= (is_src << BLOCK_STREAM_SETUP_DEVICE_IS_SRC_SHIFT);
}


ONE_NET_INLINE BOOL get_bs_device_is_src(UInt8 flags)
{
    return ((flags >> BLOCK_STREAM_SETUP_DEVICE_IS_SRC_SHIFT) &
      BLOCK_STREAM_SETUP_DEVICE_IS_SRC_MASK);
}


ONE_NET_INLINE void set_bs_device_is_dst(UInt8* flags, BOOL is_dst)
{
    (*flags) &= ~(BLOCK_STREAM_SETUP_DEVICE_IS_DST_MASK <<
      BLOCK_STREAM_SETUP_DEVICE_IS_DST_SHIFT);
    (*flags) |= (is_dst << BLOCK_STREAM_SETUP_DEVICE_IS_DST_SHIFT);
}


ONE_NET_INLINE BOOL get_bs_device_is_dst(UInt8 flags)
{
    return ((flags >> BLOCK_STREAM_SETUP_DEVICE_IS_DST_SHIFT) &
      BLOCK_STREAM_SETUP_DEVICE_IS_DST_MASK);
}


ONE_NET_INLINE BOOL block_get_index_sent(UInt8 index, const UInt8 array[5])
{
    UInt8 mask = (0x80 >> (index % 8));
    if(index >= MAX_CHUNK_SIZE)
    {
        return FALSE;
    }
    
    if(array[index / 8] & mask)
    {
        return TRUE;
    }
    return FALSE;
}


ONE_NET_INLINE void block_set_index_sent(UInt8 index, BOOL rcvd, UInt8 array[5])
{
    UInt8 array_index;
    UInt8 mask = (0x80 >> (index % 8));
    if(index >= MAX_CHUNK_SIZE)
    {
        return;
    }
    
    array_index = index / 8;
    
    array[array_index] &= ~mask;
    if(!rcvd)
    {
        return;
    }
    array[array_index] |= mask;
}


// returns -1 if all are sent
ONE_NET_INLINE SInt8 block_get_lowest_unsent_index(const UInt8 array[5],
  UInt8 chunk_size)
{
    UInt8 i;
    for(i = 0; i < chunk_size; i++)
    {
        if(!block_get_index_sent(i, array))
        {
            return i;
        }
    }
    return -1;
}


// returns the chunk size to be used. For the first and last 40 packets,
// the chunk size is 1.  Otherwise it is whatever is stored in the message
ONE_NET_INLINE UInt8 get_bs_chunk_size_to_send(const block_stream_msg_t* bs_msg)
{
    UInt32 num_packets_total = bs_msg->transfer_size / ON_BS_DATA_PLD_SIZE;
    UInt32 num_packets_left;
    
    if(bs_msg->byte_idx < 40)
    {
        return 1;
    }
    
    num_packets_left = num_packets_total - bs_msg->byte_idx;
    
    if(num_packets_left <= 40)
    {
        return 1;
    }
    
    if(num_packets_left >= 40 + bs_msg->chunk_size)
    {
        return bs_msg->chunk_size;
    }
    
    return num_packets_left - 40;
}
#endif



//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MESSAGE

#endif // _ONE_NET_MESSAGE_H //
