#ifndef _ONE_NET_H
#define _ONE_NET_H

#include "config_options.h"
#include "one_net_packet.h"
#include "one_net_message.h"
#include "one_net_data_rate.h"
#include "one_net_channel.h"
#include "one_net_port_const.h"
#include "one_net_features.h"
#include "one_net_application.h"
#include "one_net_acknowledge.h"


//! \defgroup ONE-NET ONE-NET
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
    \file ONE_NET.h
    \brief ONE-NET declarations.

    Basis for ONE-NET.  Everything in this file is application independent
    (including independent from being a MASTER or a CLIENT).  The only places
    these functions should be called from is in one_net_master.c &
    one_net_client.c.
*/

#include "one_net_status_codes.h"
#include "one_net_types.h"
#include "one_net_xtea.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{

//! ONE-NET Version numbers<br>
//! There are two types of ONE-NET vesion numbers. One set of version numbers is used
//! to identify a release of the ONE-NET source code. There are three levels to this
//! source code release version number, major, minor, and revision. These version numbers
//! appear in the one_net.h file as the three #defines ONE_NET_VERSION_MAJOR, ONE_NET_VERSION_MINOR,
//! and ONE_NET_VERSION_REVISION. For example, the source code that was released on CD at Rensas
//! DevCon on 10/13/08 was labeled as ONE_NET_SRC_V1_3_7.zip. This filename was constructed
//! based on the ONE-NET source code release version numbers of 1.3.7 (major=1, minor=3,
//! revision=7). 
//! There is a second type of ONE-NET version number named ON_VERSION. This is a one byte
//! version number which is really a ONE-NET protocol version number. This version number is
//! included in Status Response basic administrative message and is used to verfiy that the
//! master and client are communicating with compatible versions of the ONE-NET protocol.
//! Ideally, we would have only one ONE-NET version number and use that to determine both
//! the source code release version and the ONE-NET protocol compatibility. But ONE-NET 
//! efficiency requires that we use as small a number as possible for the protocol compatibility
//! number so we use one byte. We do not expect this ONE-NET compatibilty version number to 
//! change very often. However, one byte is not sufficient to represent the number of
//! iterations we expect to have for ONE-NET source code. So, we end up with two different
//! ONE-NET version numbers. Most people will be using the source code release version number
//! to identify releases of ONE-NET most of the time. 
//! In order to facilitate final testing of a release candidate, we also include have a 
//! ONE_NET_VERSION_BUILD number. This number will be inremented when a new load is put
//! on any piece of hardware. When a particular load has completed pre-release testing,
//! the build number for the load that was tested becomes the build number for the release.



enum
{
    ONE_NET_VERSION_MAJOR =     2,  //! ONE-NET major version number
    ONE_NET_VERSION_MINOR =     0,  //! ONE-NET minor version number
    ONE_NET_VERSION_REVISION =  2,  //! ONE-NET revision version number
    ONE_NET_VERSION_BUILD =     102 //! ONE-NET build version number
};


enum
{
    //! The ONE-NET version
    ON_VERSION = 0x06,

    //! The version of the base parameter structures
    ON_PARAM_VERSION = 0x04,

    //! The version of the MASTER Invite New CLIENT packet.
    ON_INVITE_PKT_VERSION = 0x01,

    //! The maximum number of times to try a transaction
    ON_MAX_RETRY = 8,

    //! The max nonce
    ON_MAX_NONCE = 63,

    //! The max nonce
    ON_MAX_MSG_ID = 63,

    //! Value to use to mark an invalid nonce
    ON_INVALID_NONCE = 0xFF,

    #ifdef _ONE_NET_MULTI_HOP
    //! Represents an invalid hops value
    ON_INVALID_HOPS = ON_MAX_HOPS_LIMIT + 1,

    //! The number of hops to try when sending the first Multi-Hop packet
    ON_FIRST_MH_MAX_HOPS_COUNT = 2,
    #endif
    
    //! Size in bytes of a XTEA key fragment
    ONE_NET_XTEA_KEY_FRAGMENT_SIZE = 4
};


extern const on_raw_did_t MASTER_RAW_DID;
extern const on_encoded_did_t MASTER_ENCODED_DID;


    
//! @} ONE-NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_typedefs
//! \ingroup ONE-NET
//! @{
    
    
typedef enum
{
    //! Master and client have not successfully passed any messages
    INVITE_START,

    //! Client has sent its features
    INVITE_FEATURES_TRANSFERRED,

    //! Master has sent the keep alive time and the flags
    INVITE_KEEP_ALIVE_FLAGS_TRANSFERRED,

    #ifdef _BLOCK_MESSAGES_ENABLED
    //! Master has sent the fragment delay values
    INVITE_FRAG_DELAY_TRANSFERRED,
    #endif

    #ifdef _STREAM_MESSAGES_ENABLED
    //! Master has sent the stream key
    INVITE_STREAM_KEY_TRANSFERRED
    #endif
} invite_accept_stage_t;
    
    
//! type of the XTEA key fragment
typedef UInt8 one_net_xtea_key_fragment_t[ONE_NET_XTEA_KEY_FRAGMENT_SIZE];


/*!
    \brief States.

    Changing the states will likely require changing the code were they are
    used. All the WRITE_WAIT states need to be 1 more than the send states.
    All the states after the WRITE_WAIT states need to be 1 more than the
    WRITE_WAIT states.
*/
typedef enum
{
    //! Listen for single, block, or stream (or repeat) data packets
    ON_LISTEN_FOR_DATA,


    //! Sends a packet that does not expect a response back
    ON_SEND_PKT = 10,

    //! Waits for the write to end
    ON_SEND_PKT_WRITE_WAIT,


    //! Sends a Single Data Packet
    ON_SEND_SINGLE_DATA_PKT = 20,

    //! Waits for the write to end
    ON_SEND_SINGLE_DATA_WRITE_WAIT,

    //! Waits for the response to a single data packet
    ON_WAIT_FOR_SINGLE_DATA_RESP,
    

    //! State to send the single data response
    ON_SEND_SINGLE_DATA_RESP = 30,

    //! Waits for the write to end
    ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT,


#ifdef _IDLE	
	//! do nothing
	ON_IDLE,
#endif


    //! State when the device has not yet joined the netwrok.  If the device is
    //! a client, it looks for the invite from the MASTER.  If the device is a
    //! MASTER, it looks for a clear channel to establish its network on.
    ON_JOIN_NETWORK = 100,


    //! The default state when the device starts up (to ensure the proper
    //! initialization routines are called)
    ON_INIT_STATE = 200
} on_state_t;


typedef struct
{
    on_sending_device_t sender;     //!< did, nonces, etc. from sender.
    UInt8 lru;                      //!< least recently used value   
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

    #ifdef _ONE_NET_MULTI_HOP
    //! Number of hops
    UInt8 hops;

    //! Max number of hops
    UInt8 max_hops;
    #endif
} on_txn_t;


//! Data needed to communicate with the MASTER
typedef struct
{
    //! Contains the MASTER did, and nonce expected to receive from the MASTER
    on_sending_device_t device;

    //! Interval at which the device must communicate with the MASTER
    tick_t keep_alive_interval;
    
    //! Bitmap of communication and MASTER settable flags
    //! (sent in the SETTINGS admin message).
    UInt8 flags;
} on_master_t;


#ifdef _ONE_NET_MASTER
//! Parameters specific to the MASTER
typedef struct
{
    //! The next available DID to be handed to a CLIENT that joins the network.
    UInt16 next_client_did;
	
    //! The last key used
    one_net_xtea_key_t old_key;
    
    #ifdef _STREAM_MESSAGES_ENABLED
    //! The last stream key used
    one_net_xtea_key_t old_stream_key;
    #endif
    
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
    
    //! Indicates if using the current stream key, or the old stream key
    BOOL use_current_stream_key;
    
    on_sending_device_t device_send_info;
} on_client_t;


#include "one_net_master_port_const.h" // for ONE_NET_MASTER_MAX_CLIENTS

#endif


enum
{
#ifdef _ONE_NET_CLIENT
    CLIENT_NV_PARAM_SIZE_BYTES = sizeof(on_base_param_t) + sizeof(on_master_t),
#else
    CLIENT_NV_PARAM_SIZE_BYTES = 0,
#endif
#ifdef _ONE_NET_MASTER
    MIN_MASTER_NV_PARAM_SIZE_BYTES = sizeof(on_base_param_t) +
      sizeof(on_master_param_t),
    MAX_MASTER_NV_PARAM_SIZE_BYTES = MIN_MASTER_NV_PARAM_SIZE_BYTES +
      ONE_NET_MASTER_MAX_CLIENTS * sizeof(on_client_t),
#else
    MASTER_NV_PARAM_SIZE_BYTES = 0,
#endif
    NV_PARAM_SIZE_BYTES =
      CLIENT_NV_PARAM_SIZE_BYTES > MAX_MASTER_NV_PARAM_SIZE_BYTES ?
      CLIENT_NV_PARAM_SIZE_BYTES : MAX_MASTER_NV_PARAM_SIZE_BYTES
};


#ifdef _ENHANCED_INVITE
//! Reason why an invitation has been cancelled.
//! TO DO : We may want to make this part of the application code since
//! different applications may have specific codes that shouldn't be part
//! be part of the official ONE-NET package.
typedef enum
{
    CANCEL_INVITE_TIMEOUT, /* look for invitation time has expired */
    CANCEL_INVITE_CANCELLED_BY_USER, /* look for invitation time has expired */
    CANCEL_INVITE_CANCELLED_INTERNAL_ERROR, /* Some error occurred somewhere */
    CANCEL_INVITE_OTHER_REASON /* If none of the reasons above fit. */	
} cancel_invite_reason_t;
#endif

//! Function to retrieve sending device information
typedef on_sending_device_t* (*one_net_get_sender_info_func_t)
  (const on_encoded_did_t * const DID);;


//! Packet Handling Function for data packets
typedef on_message_status_t (*on_pkt_hdlr_t)(on_txn_t** txn,
  on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type);

//! Packet Handling Function for responses
typedef on_message_status_t (*on_ack_nack_hdlr_t)(on_txn_t** txn,
  on_pkt_t* const pkt, UInt8* raw_pld, UInt8* msg_type,
  on_ack_nack_t* ack_nack);

//! Transaction handler
typedef on_message_status_t (*on_txn_hdlr_t)(on_txn_t ** txn,
  on_pkt_t* const pkt,  UInt8* raw_pld, UInt8* msg_type,
  const on_message_status_t status, on_ack_nack_t* ack_nack);
  
//! The set of needed packet handlers
typedef struct
{
    //! Single Data Packet Handler
    on_pkt_hdlr_t single_data_hdlr;
	
	//! Single Data ACK/NACK Handler
	on_ack_nack_hdlr_t single_ack_nack_hdlr;

    //! Single transaction handler
    on_txn_hdlr_t single_txn_hdlr;

    #ifdef _BLOCK_MESSAGES_ENABLED
    //! Single Data Packet Handler
    on_pkt_hdlr_t block_data_hdlr;
	
	//! Single Data ACK/NACK Handler
	on_ack_nack_hdlr_t block_ack_nack_hdlr;

    //! Single transaction handler
    on_txn_hdlr_t block_txn_hdlr;
    #endif

    #ifdef _STREAM_MESSAGES_ENABLED
    //! Stream Data Packet Handler
    on_pkt_hdlr_t stream_data_hdlr;
	
	//! Stream Data ACK/NACK Handler
	on_ack_nack_hdlr_t stream_ack_nack_hdlr;

    //! Stream transaction handler
    on_txn_hdlr_t stream_txn_hdlr;
    #endif
} on_pkt_hdlr_set_t;



//! @} ONE-NET_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_pub_var
//! \ingroup ONE-NET
//! @{


#ifdef _ONE_NET_MULTI_HOP
//! flag that indicates if any Multi-Hop Repeaters have joined the network
extern BOOL mh_repeater_available;
#endif

//! The encoded broadcast did.
extern const on_encoded_did_t ON_ENCODED_BROADCAST_DID;

//! The raw broadcast did.
extern const on_raw_did_t ON_RAW_BROADCAST_DID;

extern UInt8 nv_param[];
extern on_base_param_t* const on_base_param;


extern on_txn_t response_txn;
extern on_txn_t single_txn;
#ifdef _BLOCK_MESSAGES_ENABLED
extern on_txn_t block_txn;
#endif
#ifdef _STREAM_MESSAGES_ENABLED
extern on_txn_t stream_txn;
#endif


//! An array that contains the number of of units of each type that this
//! device supports.  If values are changed here, see ONE_NET_NUM_UNIT_TYPES &
//! ONE_NET_NUM_UNITS
extern const ona_unit_type_count_t
  ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES];

//! true if device is functioning as a master, false otherwise 
extern BOOL device_is_master;

//! current status of startup
extern one_net_startup_status_t startup_status;

//! The current state.
extern on_state_t on_state;


//! The set of packet handlers
extern on_pkt_hdlr_set_t pkt_hdlr;


//! an on_pkt_t structure for data packets
extern on_pkt_t data_pkt_ptrs;

//! an on_pkt_t structure for response packets
extern on_pkt_t response_pkt_ptrs;

//! a function to retrieve the sender information
extern one_net_get_sender_info_func_t get_sender_info;


//! A place to store a single message with payload.
extern on_single_data_queue_t single_msg;

//! A place to store the single message raw payload.
extern UInt8 single_data_raw_pld[];

//! Pointer to the current single message being sent.  If none, this will be
//! NULL.  Generally this will point to single_msg.
extern on_single_data_queue_t* single_msg_ptr;


//! A place to store the raw packet bytes when encrypting, decrypting, etc.
//! so that it will not have to be declared inside of functions and risk a
//! overflow.
extern UInt8 raw_payload_bytes[];

extern UInt8 invite_pkt[];

//! Unique key of the device being invited into the network
extern one_net_xtea_key_t invite_key;


//! @} ONE-NET_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{



#ifdef _ONE_NET_MULTI_HOP
one_net_status_t on_build_hops(UInt8 * enc_hops_field, UInt8 hops,
  UInt8 max_hops);
one_net_status_t on_parse_hops(UInt8 enc_hops_field, UInt8* hops,
  UInt8* max_hops);
#endif
BOOL setup_pkt_ptr(UInt8 pid, UInt8* pkt_bytes, on_pkt_t* pkt);

one_net_status_t on_build_data_pkt(const UInt8* raw_pld, UInt8 msg_type,
  const on_pkt_t* pkt_ptrs, on_txn_t* txn, on_sending_device_t* device);
  
one_net_status_t on_build_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_nid_t* nid, const on_encoded_did_t* repeater_did,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did);
  
one_net_status_t on_build_my_pkt_addresses(const on_pkt_t* pkt_ptrs,
  const on_encoded_did_t* dst_did, const on_encoded_did_t* src_did);

one_net_status_t on_complete_pkt_build(on_pkt_t* pkt_ptrs,
  UInt8 msg_id, UInt8 pid);
  
UInt8 calculate_msg_crc(const on_pkt_t* pkt_ptrs);
BOOL verify_msg_crc(const on_pkt_t* pkt_ptrs);
BOOL verify_payload_crc(UInt8 pid, const UInt8* decrypted);
  


// encrypting / decrypting
one_net_status_t on_encrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len);
one_net_status_t on_decrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY, const UInt8 payload_len);



// initialization
void one_net_init(void);


// this is a master-specific function, but we're declaring it in one_net.h
// rather than one_net_master.h because we want to avoid if possible having
// one_net.h include one_net_master.h.  The implementation is in
// one_net_master.c
#ifdef _ONE_NET_MASTER
#ifdef _STREAM_MESSAGES_ENABLED
one_net_xtea_key_t* master_get_encryption_key(on_data_t type,
  const on_encoded_did_t* const did);
#else
one_net_xtea_key_t* master_get_encryption_key(
  const on_encoded_did_t* const did);
#endif
#endif


//! the main function
BOOL one_net(on_txn_t ** txn);



#ifdef _ONE_NET_MULTI_HOP
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


one_net_status_t rx_single_data(on_txn_t** txn);
#ifdef _BLOCK_MESSAGES_ENABLED
one_net_status_t rx_block_data(on_txn_t** txn);
#endif
#ifdef _STREAM_MESSAGES_ENABLED
one_net_status_t rx_stream_data(on_txn_t** txn);
#endif
one_net_status_t on_rx_data_pkt(const on_encoded_did_t * const EXPECTED_SRC_DID,
  on_txn_t ** txn);
  

#ifdef _RANGE_TESTING
void enable_range_testing(BOOL on);
BOOL adjust_range_test_did_array(on_encoded_did_t* const did, BOOL add);
void reset_range_test_did_array(void);
BOOL device_in_range(on_encoded_did_t* did);
#endif




//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET

#endif // _ONE_NET_H //
 