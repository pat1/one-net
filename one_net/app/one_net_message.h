#ifndef ONE_NET_MESSAGE_H
#define ONE_NET_MESSAGE_H

#include "config_options.h"
#include "one_net_types.h"
#include "one_net_packet.h"
#include "one_net_port_const.h"
#include "one_net_features.h"
#include "one_net_acknowledge.h"
#include "tick.h"


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


extern const on_encoded_did_t NO_DESTINATION;


//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{
    
    
    
/*!
    \brief Data Packet Message Types
*/
typedef enum
{
    ON_APP_MSG,                     //!< Application message type (normal parsing - units, class, type, data)
    
    // Note -- ON_APP_MSG_TYPE_2, ON_APP_MSG_TYPE_3, and ON_APP_MSG_TYPE_4 are not implemented as of July 23, 2012,
    //         but can still be used by the application code.
    // TODO -- Implement in ONE-NET so these can be used.
    ON_APP_MSG_TYPE_2,              //!< Application message type (no units, includes class, type, data -- units are interpreted as ONE_NET_DEV_UNIT)
    ON_APP_MSG_TYPE_3,              //!< Application message type (Special case ONE-NET-specified format.  Has type, but not class, units, or data based as an integer.  Parsing will be based on type.)
    ON_APP_MSG_TYPE_4,              //!< Application message type (no units, class, or type -- data should be interpreted as an array -- The interpretation of the array may or may not be application-specific)
    ON_ADMIN_MSG,                   //!< Admin message type
    ON_FEATURE_MSG,                 //!< A request for features
    ON_ROUTE_MSG,                   //!< A routing message
    
    ON_RESERVED_MSG_TYPE_1,              //!< Unspecified, but reserved for future use
    ON_RESERVED_MSG_TYPE_2,              //!< Unspecified, but reserved for future use
    ON_RESERVED_MSG_TYPE_3,              //!< Unspecified, but reserved for future use
    
    // add any application-specific parsing techniques other than array data specified as ON_APP_MSG_TYPE_4 below
    ON_APPLICATION_SPECIFIC_MIN_MSG_TYPE,
    ON_APPLICATION_SPECIFIC_MAX_MSG_TYPE = 0x0F,
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


//! Relevant only for non-simple clients.  This structure controls whether a client device can
//! "slide off"(i.e. be replaced by another device) of a client's device list.  If the flag is
//! set to ON_DEVICE_ALLOW_SLIDEOFF, it can be replaced.  If its value is ON_DEVICE_PROHIBIT_SLIDEOFF
//! or ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK, it cannot.
typedef enum
{
    ON_DEVICE_ALLOW_SLIDEOFF, //! Set if the device should be allowed to "slide off" the list.
    ON_DEVICE_PROHIBIT_SLIDEOFF, //! Set if the device should not be allowed to "slide off" the list.
    ON_DEVICE_PROHIBIT_SLIDEOFF_LOCK, //! Set if the device should not be allowed to "slide off" the list and
                                      //! is "locked".  This can be unlocked with a call to on_client_unlock_device_slideoff()
} device_slideoff_t;


/*!
    \brief Info for communicating with a device.

    This structure holds the information needed to receive from a device.
*/
typedef struct
{
    on_encoded_did_t did;           //!< Encoded Device ID of the sender
    on_features_t features;         //!< features of the device.
    #ifdef ONE_NET_MULTI_HOP
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


typedef struct
{
    on_sending_device_t sender;     //!< did, etc. from sender.
    #ifndef ONE_NET_SIMPLE_CLIENT
    UInt8 lru;                      //!< least recently used value
    device_slideoff_t slideoff;    //!< Whether the device can "slide off" the list when the list gets full.
    #endif
} on_sending_dev_list_item_t;


//! Transaction structure
typedef struct
{
    //! The type of transaction (i.e. block, stream, invite, single, response)
    //! See on_data_t for details.
    UInt8 txn_type;
    
    //! The priority of the transaction
    UInt8 priority;

    //! How many times this txn has been tried
    UInt8 retry;

    //! The timer that contains the time the next transaction
    //! is supposed to occur if this is a block or stream transaction.
    UInt8 next_txn_timer;
    
    //! Time in ms before the message is considered timed out.  Note this is
    //! 16 bit value rather than a UInt32 or tick_t because there is no way
    //! we'll ever want to wait more than 65535 milliseconds for a response
    UInt16 response_timeout;

    //! The length of the data the packet contains (in bytes).  This will not
    //! contain the hops field, so if a multihop packet is being sent, the
    //! hop field size will need to be added to this value.  This value can
    //! also be used to find the hops field in the packet since the hops field
    //! is at a different index for each packet type
    UInt8 data_len;

    //! The packet to be sent
    UInt8 * pkt;
    
    //! Device information for the recipient
    on_sending_device_t* device;
    
    //! Key to use for this transaction
    one_net_xtea_key_t* key;

    #ifdef ONE_NET_MULTI_HOP
    //! Number of hops
    UInt8 hops;

    //! Max number of hops
    UInt8 max_hops;
    #endif
} on_txn_t;


//! Data needed to communicate with the MASTER
typedef struct
{
    //! Contains the information needed to send to / receive from the Master
    on_sending_device_t device;

    //! Interval at which the device must communicate with the MASTER(in ms)
    //! If the interval is 0, clients are NOT expected to check in regularly
    //! with the master.  To force a client to check in immediately, you can
    //! set this to 1.  Don't set it to 0.  Setting it to 0 will cause the
    //! device to think it is not supposed to check-in regularly.
    UInt32 keep_alive_interval;
    
    //! Bitmap of communication and MASTER settable flags
    //! (sent in the SETTINGS admin message).
    UInt8 flags;
} on_master_t;


#ifdef ONE_NET_MASTER
//! Parameters specific to the MASTER
typedef struct
{
    #ifdef BLOCK_MESSAGES_ENABLED
    UInt8 block_stream_flags;
    #endif
    
    //! The next available DID to be handed to a CLIENT that joins the network.
    UInt16 next_client_did;
    
    //! The number of CLIENTs currently in the network
    UInt16 client_count;
} on_master_param_t;


//! Structure to keep track of the CLIENT
typedef struct
{
    //! Keeps track of the communication and MASTER setable flags in the CLIENT
    //! (the ones sent in the SETTINGS admin message).
    UInt8 flags;

    //! Indicates if using the current key or the old key.
    BOOL use_current_key;
    
    //! If true, indicates that a device has been added and this client has
    //! not yet been informed.
    BOOL send_add_device_message;
    
    //! If true, indicates that a device has been removed and this client has
    //! not yet been informed.
    BOOL send_remove_device_message;
    
    //! Interval at which the client must communicate with the MASTER(in ms)
    UInt32 keep_alive_interval;
    
    //! The latest time that the client is expected to check in.
    tick_t next_check_in_time;
    
    on_sending_device_t device;
} on_client_t;


#include "one_net_master_port_const.h" // for ONE_NET_MASTER_MAX_CLIENTS

#endif


    
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




//!< Status of a unit (not part of an ACK -- or part of an ACk but not
//!< fit any of the categories below)
#define ONA_STATUS                 0x0

//!< Status of a unit has changed(i.e. switch has been flipped)
#define ONA_STATUS_CHANGE          0x1

//!< Status of a unit (in the ACK in response to a query)
#define ONA_STATUS_QUERY_RESP      0x2

//!< Status of a unit (in the ACK in response to a fast query)
#define ONA_STATUS_FAST_QUERY_RESP 0x3

//!< Status of a unit (in the ACK in response to a command)
#define ONA_STATUS_COMMAND_RESP    0x4

//!< Command to change status of a unit
#define ONA_COMMAND                0x5

//!< Query status of a unit
#define ONA_QUERY                  0x6

//!< Fast Query / "poll" status of a unit
#define ONA_FAST_QUERY             0x7

//!< Used when the message class is not applicable or is unknown
#define ONA_CLASS_UNKNOWN          0x8

// Message classes 0x9 to 0xF are currently unused


//!< Status Message Macro.  This can be used to quickly ascertain whether he message
//!< is a status message if all status messages are to treated the same.
#define ONA_IS_STATUS_MESSAGE(X) (X <= ONA_STATUS_COMMAND_RESP)




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
    ON_ADD_DEV_RESP = 0x03,
    
    //! Response from a client when a device has been removed.
    ON_REMOVE_DEV_RESP = 0x04,
    
    #ifdef DATA_RATE_CHANNEL
    //! Change data rate and channel
    ON_CHANGE_DATA_RATE_CHANNEL = 0x05,
    #endif
    
    #ifndef ONE_NET_SIMPLE_CLIENT
    //! Sent by a client when a new key is needed for whatever reason.
    //! Generally this is sent when a client is about to run out of message
    //! ids or feels there has been some breach of security or some attempted
    //! breach of security.
    ON_REQUEST_KEY_CHANGE = 0x06,
    #endif

    #ifdef BLOCK_MESSAGES_ENABLED
    //! Change both high and low fragment delays in one message
    ON_CHANGE_FRAGMENT_DELAY = 0x07,
    
    //! Response to changing of fragment delays
    ON_CHANGE_FRAGMENT_DELAY_RESP = 0x08,
    #endif
    
    //! Sent to change the keep alive interval
    ON_CHANGE_KEEP_ALIVE = 0x09,

    #ifdef PEER
    //! Sent by the MASTER to assign a peer to the receiving CLIENT.  The CLIENT
    //! can then send directly to the peer.
    ON_ASSIGN_PEER = 0x0A,

    //! Sent by the MASTER to un-assign a peer from the receiving CLIENT.  The
    //! CLIENT must not send directly to that peer anymore.
    ON_UNASSIGN_PEER = 0x0B,
    #endif

    //! Query for the Keep Alive Timeout.  This is the interval at which a
    //! CLIENT must attempt to check in with the MASTER.  Any communication
    //! with the MASTER resets this timer.
    ON_KEEP_ALIVE_QUERY = 0x0C,
    
    //! Sent by a client to check in with the master whenever the keep-alive
    //! timer expires.
    ON_KEEP_ALIVE_RESP = 0x0D,
    
    //! Sent to change a device's settings.  The devices settings should not
    //! be considered changed until a SETTINGS_RESP is received.
    ON_CHANGE_SETTINGS = 0x0E,
    
    //! Sent in response to a change settings message.
    ON_CHANGE_SETTINGS_RESP = 0x0F,
    
    #ifdef BLOCK_MESSAGES_ENABLED
    ON_REQUEST_BLOCK_STREAM = 0x10,
    
    ON_REQUEST_REPEATER = 0x11,
    
    ON_TERMINATE_BLOCK_STREAM = 0x12,
    #endif

    //! Sent by the MASTER when it is adding a device to the network
    ON_ADD_DEV = 0x13,

    //! Sent by the MASTER when it is removing a device from the network
    ON_RM_DEV = 0x14
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
    #ifdef PEER
	BOOL send_to_peer_list;
    UInt8 src_unit;
    #endif
    #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
	tick_t send_time;
    #endif
    #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
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



#ifdef BLOCK_MESSAGES_ENABLED

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
    tick_t start_time;
    tick_t last_response_time;
    UInt32 elapsed_time;
} stream_msg_t;


typedef struct
{
    UInt32 transfer_size;
    SInt32 byte_idx; // TODO -- Why is this unsigned?
    UInt8 chunk_idx;
    UInt8 chunk_size;
    UInt16 chunk_pause;
    UInt8 sent[MAX_CHUNK_SIZE / 8]; // 8 bits per byte.  "complete" is a
           // bitwise "boolean" array, with each bit representing whether a
           // certain packet within a chunk has been received.  0 means FALSE.
           // 1 means TRUE.
} block_msg_t;


typedef union
{
    block_msg_t block;
    stream_msg_t stream;
} bs_msg_union_t;


typedef struct
{
    // note -- repeaters will not be interested in many of these attributes
    UInt8 bs_on_state;
    BOOL transfer_in_progress;
    UInt8 flags;
    UInt16 frag_dly;
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
    #ifdef ONE_NET_MULTI_HOP
    UInt8 num_repeaters;
    on_encoded_did_t repeaters[ON_MAX_HOPS_LIMIT];
    #endif
    bs_msg_union_t bs;
    BOOL response_needed;
    BOOL use_saved_ack_nack;
    UInt8 saved_ack_nack_payload_bytes[5];
    on_ack_nack_t saved_ack_nack;
} block_stream_msg_t;


typedef struct
{
    UInt16 msg_id;
    UInt8* data;
    UInt32 byte_idx;
    UInt8 chunk_idx;
    UInt8 chunk_size;
} block_pkt_t;


// exact same as block_pkt_t except it has time instead of byte_idx.  You
// can typecast easily from one to the other given that the sizes and order of
// the elements are the same.
typedef struct
{
    UInt16 msg_id;
    UInt8* data;
    UInt32 elapsed_time; // in milliseconds, not ticks
    BOOL response_needed;
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
    
    
#ifdef __cplusplus
extern "C"
{
#endif
    
    
one_net_status_t on_parse_response_pkt(UInt8 raw_pid, UInt8* raw_bytes,
  on_ack_nack_t* const ack_nack);

#ifdef ONE_NET_MULTI_HOP
one_net_status_t on_build_hops(on_pkt_t* pkt, UInt8 hops, UInt8 max_hops);
one_net_status_t on_parse_hops(const on_pkt_t* pkt, UInt8* hops,
  UInt8* max_hops);
#endif
BOOL setup_pkt_ptr(UInt16 raw_pid, UInt8* pkt_bytes, UInt16 msg_id,
  on_pkt_t* pkt);

#ifdef ONE_NET_MULTI_HOP
/*!
    \brief Sets the hops for a device
    
    \param[in] raw_dst The raw device ID of the device's hops to change.
    \param[in] hops The desired new number of hops for the device
    
    \return The new number of hops for the device.
    \return -1 if device id could not be decoded or is not in this
               device's sending table.
*/
SInt8 one_net_set_hops(const on_raw_did_t* const raw_did, UInt8 hops);


/*!
    \brief Sets the max_hops for a device
    
    \param[in] raw_dst The raw device ID of the device's max hops to change.
    \param[in] max_hops The desired new number of max_hops for the device.
    
    \return The new maximum number of hops for the device
    \return -1 if device id could not be decoded or is not in this
               device's sending table.
*/
SInt8 one_net_set_max_hops(const on_raw_did_t* const raw_did, UInt8 max_hops);
#endif


#ifndef BLOCK_MESSAGES_ENABLED
one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  on_pkt_t* pkt_ptrs, on_txn_t* txn);
#else
one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  on_pkt_t* pkt_ptrs, on_txn_t* txn, block_stream_msg_t* bs_msg);
#endif

one_net_status_t on_build_response_pkt(on_ack_nack_t* ack_nack,
  on_pkt_t* pkt_ptrs, on_txn_t* txn, BOOL stay_awake);
  
one_net_status_t on_build_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_nid_t* nid, const on_encoded_did_t* repeater_did,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did);
  
one_net_status_t on_build_my_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did);

one_net_status_t on_complete_pkt_build(on_pkt_t* pkt_ptrs, UInt8 pid); 

UInt8 calculate_msg_crc(const on_pkt_t* pkt_ptrs);
BOOL verify_msg_crc(const on_pkt_t* pkt_ptrs);
BOOL verify_payload_crc(UInt16 raw_pid, const UInt8* decrypted);
#ifndef _R8C_TINY
// This is a helper function not used by ONE-NET. Just added it as a helper function for anyone
// who wants it.  #defining it out to spare about 60 bytes of compiled code.
// Note: Some compilers will automatically discard unused functions, but Renesas does not, so
// adding the #ifndef guard.
BOOL calculate_payload_crc(UInt8* crc_calc, UInt16 raw_pid, const UInt8* decrypted);
#endif
  
// encrypting / decrypting
#ifdef STREAM_MESSAGES_ENABLED
one_net_status_t on_encrypt(BOOL is_stream_pkt, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len);
one_net_status_t on_decrypt(BOOL is_stream_pkt, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len);
#else
one_net_status_t on_encrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len);
one_net_status_t on_decrypt(UInt8 * const data,
  const one_net_xtea_key_t * const KEY, UInt8 payload_len);
#endif


#ifdef DEBUGGING_TOOLS
void get_queue_memory(UInt8** pld_buffer, on_single_data_queue_t** queue,
         UInt8* queue_size, UInt16* tail_idx);
#endif

void empty_queue(void);


#if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
// return true if an element was popped, false otherwise.
BOOL pop_queue_element(on_single_data_queue_t* const element,
    UInt8* const buffer, UInt8 index);
#else
BOOL pop_queue_element(void);
#endif

#if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
int single_data_queue_ready_to_send(tick_t* const queue_sleep_time);
#else
int single_data_queue_ready_to_send(void);
#endif

on_single_data_queue_t* push_queue_element(UInt16 pid,
  UInt8 msg_type, UInt8* raw_data, UInt8 data_len, UInt8 priority,
  const on_encoded_did_t* const src_did,
  const on_encoded_did_t* const enc_dst
  #ifdef PEER
      , BOOL send_to_peer_list,
      UInt8 src_unit
  #endif
  #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , tick_t send_time_from_now
  #endif
  #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , tick_t expire_time_from_now
  #endif
  );


#ifdef ONE_NET_CLIENT
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


#ifdef BLOCK_MESSAGES_ENABLED
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


BOOL block_get_index_sent(UInt8 index, const UInt8 array[5]);
void block_set_index_sent(UInt8 index, BOOL rcvd, UInt8 array[5]);
SInt8 block_get_lowest_unsent_index(const UInt8 array[5], UInt8 chunk_size);
UInt32 block_get_bytes_remaining(UInt32 transfer_size, UInt32 byte_index,
  UInt8 chunk_index);


// returns the chunk size to be used. For the first and last 40 packets,
// the chunk size is 1.  Otherwise it is whatever is stored in the message
UInt8 get_current_bs_chunk_size(const block_stream_msg_t* bs_msg);
#endif



#ifdef __cplusplus
}
#endif





//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_MESSAGE

#endif // ONE_NET_MESSAGE_H //
