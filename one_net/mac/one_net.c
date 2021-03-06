//! \addtogroup ONE-NET
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
    \file one_net.c
    \brief Basis for ONE-NET implementation.

    This file is application independent.  The functionality implemented here
    is also independent of the device being a MASTER or CLIENT.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "config_options.h"
#include "one_net_packet.h"
#include "one_net.h"
#include "tal.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"
#include "one_net_xtea.h"
#include "one_net_acknowledge.h"
#ifdef PEER
#include "one_net_peer.h"
#endif
#include "one_net_port_const.h"
#ifdef ONE_NET_CLIENT
#include "one_net_client_port_specific.h"
#include "one_net_client_port_const.h"
#endif
#ifdef ONE_NET_MASTER
#include "one_net_master.h"
#include "one_net_master_port_specific.h"
#include "one_net_master_port_const.h"
#endif
#include "one_net_timer.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Header(Preamble and SOF)
extern const UInt8 HEADER[];


const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0xB4, 0xB4};
const on_raw_did_t ON_RAW_BROADCAST_DID = {0x00, 0x00};
const on_raw_did_t MASTER_RAW_DID = {0x00, 0x10};
const on_encoded_did_t MASTER_ENCODED_DID = {0xB4, 0xBC};

//! @} ONE-NET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_typedefs
//! \ingroup ONE-NET
//! @{

//! @} ONE-NET_typedefs
//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PUBLIC VARIABLES



//! Contiguous block of memory to store parameters that are saved to
//! non-volatile memory.  Parameters will point to locations in the array
UInt8 nv_param[NV_PARAM_SIZE_BYTES];

//! The base parameters for the device
on_base_param_t* const on_base_param = (on_base_param_t* const) nv_param;


//! The set of packet handlers
on_pkt_hdlr_set_t pkt_hdlr;

//! a function to retrieve the sender information
one_net_get_sender_info_func_t get_sender_info;


#ifndef ONE_NET_MULTI_HOP
//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, 0, NULL, NULL, NULL};

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  0, NULL, NULL, NULL};

#ifdef BLOCK_MESSAGES_ENABLED
    //! The current block transaction
    on_txn_t bs_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BS_TIMER, 0, 0, NULL, NULL, NULL};
#endif // if block messages are enabled //
#else
//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, 0, NULL, NULL, NULL, 0, 0};

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  0, NULL, NULL, NULL, 0, 0};

#ifdef BLOCK_MESSAGES_ENABLED
    //! The current block transaction
    on_txn_t bs_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BS_TIMER, 0, 0, NULL, NULL, NULL, 0, 0};
#endif // if block messages are enabled //
#endif


//! true if device is functioning as a master, false otherwise
#ifndef ONE_NET_MASTER
BOOL device_is_master = FALSE;
#else
BOOL device_is_master = TRUE; // if device cvan be master OR client, the
                              // initialization code will need to set this
                              // value
#endif



//! an on_pkt_t structure for data packets
on_pkt_t data_pkt_ptrs;

//! an on_pkt_t structure for response packets
on_pkt_t response_pkt_ptrs;


//! A place to store a single message with payload.
on_single_data_queue_t single_msg;

//! A place to store the single message raw payload.
UInt8 single_data_raw_pld[ONA_MAX_SINGLE_PACKET_PAYLOAD_LEN];

//! Pointer to the current single message being sent.  If none, this will be
//! NULL.  Generally this will point to single_msg.
on_single_data_queue_t* single_msg_ptr = NULL;


//! A place to store the raw packet bytes when encrypting, decrypting, etc.
//! so that it will not have to be declared inside of functions and risk a
//! overflow.
UInt8 raw_payload_bytes[ON_MAX_RAW_PLD_LEN + 1];

#ifdef ONE_NET_CLIENT
extern BOOL client_joined_network; // declared extern in one_net_client.h but
     // we do not want to include one_net_client.h so we declare it here too.
extern on_master_t * const master;
extern on_sending_dev_list_item_t sending_dev_list[];
#endif

#ifndef ONE_NET_MULTI_HOP
//! The current invite transaction
on_txn_t invite_txn = {ON_INVITE, ONE_NET_NO_PRIORITY, 0,
#ifdef ONE_NET_MASTER
  ONT_INVITE_SEND_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL};
#else
  ONT_INVITE_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL};
#endif
#else
//! The current invite transaction
on_txn_t invite_txn = {ON_INVITE, ONE_NET_NO_PRIORITY, 0,
#ifdef ONE_NET_MASTER
  ONT_INVITE_SEND_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL, 0, 0};
#else
  ONT_INVITE_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL,  0, 0};
#endif
#endif

  
#ifdef ONE_NET_CLIENT
extern BOOL client_looking_for_invite;
#endif

//! A buffer containing all encoded bytes for transmitting and receiving
UInt8 encoded_pkt_bytes[ENCODED_BYTES_BUFFER_LEN];

//! The expected source of the next packet.
//! TODO -- more can be done with this variable.  We need to have a way for
//!         the application code to override the default.
on_encoded_did_t expected_src_did; // broadcast

//! Denotes which key was used.  If true, the current key is being used.
BOOL decrypt_using_current_key;


#ifdef NON_VOLATILE_MEMORY
BOOL save = FALSE;
#endif

#ifdef ROUTE
//! variable denoting the start of a route message.
tick_t route_start_time = 0;
#endif

#ifdef BLOCK_MESSAGES_ENABLED
block_stream_msg_t bs_msg;
#endif

#ifdef DATA_RATE_CHANNEL
dr_channel_stage_t dr_channel_stage = ON_DR_CHANNEL_NO_SCHEDULED_CHANGE;
UInt16 dormant_data_rate_time_ms = 0;
UInt8 alternate_data_rate = ONE_NET_DATA_RATE_38_4;
UInt8 alternate_channel;
#endif

//! Boolean value denoting whether a key change should occur in the very
//! near future
BOOL key_change_requested = FALSE;

//! Time of the last key change request
tick_t key_change_request_time = 0;


#ifdef ONE_NET_CLIENT
//! If true and sending a single response, this flag signifies that we
// should instead send our features.
extern BOOL features_override; // defined in one_net_message.c
#endif

#ifdef PID_BLOCK
//! Stores which PIDs are accepted.
pid_block_t pid_block_info = {0xFFFF, PID_ACCEPT, PID_ACCEPT};

//! If true, filter pids.  If false, do not.
BOOL pid_blocking_on = FALSE;
#endif



//                              PUBLIC VARIABLES
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{


//! The current state.
on_state_t on_state = ON_INIT_STATE;


#ifdef ONE_NET_MH_CLIENT_REPEATER
    // Transaction for forwarding on MH packets.
    static on_txn_t mh_txn = {ON_NO_TXN, ONE_NET_NO_PRIORITY, 0,
      ONT_MH_TIMER, 0, 0, encoded_pkt_bytes, NULL, NULL, 0, 0};
#endif


#ifdef RANGE_TESTING
//! Stores the DIDs of the devices which are in range.
static on_encoded_did_t range_test_did_array[RANGE_TESTING_ARRAY_SIZE];

//! If true, range test.  If false, do not.
static BOOL range_testing_on = FALSE;
#endif


//! @} ONE-NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{


#ifdef DATA_RATE_CHANNEL
static void check_dr_channel_change(void);
#endif
static BOOL check_for_clr_channel(void);
static on_message_status_t rx_single_resp_pkt(on_txn_t** const txn,
  on_txn_t** const this_txn, on_pkt_t* const pkt,
  UInt8* const raw_payload_bytes, on_ack_nack_t* const ack_nack);
#ifdef BLOCK_MESSAGES_ENABLED
static on_message_status_t rx_block_resp_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, on_pkt_t* pkt, UInt8* raw_payload_bytes,
  on_ack_nack_t* ack_nack);
static on_message_status_t rx_block_data(on_txn_t* txn, block_stream_msg_t* bs_msg,
  block_pkt_t* block_pkt, on_ack_nack_t* ack_nack);
static void terminate_bs_complete(block_stream_msg_t* bs_msg);
#endif

#ifdef STREAM_MESSAGES_ENABLED
static on_message_status_t rx_stream_resp_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, on_pkt_t* pkt, UInt8* raw_payload_bytes,
  on_ack_nack_t* ack_nack);
static on_message_status_t rx_stream_data(on_txn_t* txn, block_stream_msg_t* bs_msg,
  stream_pkt_t* stream_pkt, on_ack_nack_t* ack_nack);
#endif



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{


/*!
    \brief Initializes ONE-NET.

    \return void
*/
void one_net_init(void)
{
    one_net_set_channel(on_base_param->channel);
    #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
    empty_queue();
    #endif
    single_msg_ptr = NULL;
    single_txn.priority = ONE_NET_NO_PRIORITY;
    #ifdef RANGE_TESTING
    reset_range_test_did_array();
    #endif
    #ifdef BLOCK_MESSAGES_ENABLED
    bs_msg.transfer_in_progress = FALSE;
    bs_msg.saved_ack_nack.payload = (ack_nack_payload_t*)
      bs_msg.saved_ack_nack_payload_bytes;
    bs_msg.use_saved_ack_nack = FALSE;
    #endif
} // one_net_init //
    


/*!
    \brief The main ONE-NET function.

    This is the main state machine for ONE-NET.  It handles sending/receiving
    the various transactions.

    \param[in/out] txn The transaction currently being carried out

    \return TRUE if the current transaction is completed.  This should coincide
              with the priority for the transaction being set to
              ONE_NET_NO_PRIORITY unless another transaction resulted from the
              one that just finished.
            FALSE if the current transaction is not complete.
*/
void one_net(on_txn_t ** txn)
{
    one_net_status_t status;
    on_txn_t* this_txn;
    on_pkt_t* this_pkt_ptrs;
    #ifndef ONE_NET_SIMPLE_CLIENT
    static BOOL at_least_one_response = FALSE;
    #endif
    
    on_ack_nack_t ack_nack;
    ack_nack_payload_t ack_nack_payload;
    ack_nack.payload = &ack_nack_payload;

    #ifdef BLOCK_MESSAGES_ENABLED
    if(on_state <= ON_BS_COMMENCE || on_state >= ON_BS_CHUNK_PAUSE)
    {
        if(*txn == &bs_txn)
        {
            *txn = NULL; //clear if not already clearedso we do not get
                         // stuck.
        }
    }
    if(bs_msg.transfer_in_progress && bs_msg.src &&
      ont_inactive_or_expired(ONT_BS_TIMEOUT_TIMER))
    {
        if(on_state >= ON_BS_TERMINATE || bs_msg.bs_on_state >=
          ON_BS_TERMINATE)
        {
            terminate_bs_complete(&bs_msg);
            return;
        }
        terminate_bs_msg(&bs_msg, NULL, ON_MSG_TIMEOUT, NULL);
        return;
    }
    #endif
    
    switch(on_state)
    {
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_FIND_ROUTE:
        case ON_BS_CONFIRM_ROUTE:
        #ifdef DATA_RATE_CHANNEL
        case ON_BS_CHANGE_DR_CHANNEL:
        case ON_BS_CHANGE_MY_DR_CHANNEL:
        #endif
        case ON_BS_DEVICE_PERMISSION:
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_MASTER_DEVICE_PERMISSION:
        #endif
        #ifdef ONE_NET_MULTI_HOP
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_MASTER_REPEATER_PERMISSION_START:
        case ON_BS_MASTER_REPEATER_PERMISSION:
        case ON_BS_MASTER_REPEATER_PERMISSION_END:
        #endif
        case ON_BS_REPEATER_PERMISSION_START:
        case ON_BS_REPEATER_PERMISSION:
        case ON_BS_REPEATER_PERMISSION_END:
        #endif
        case ON_BS_COMMENCE:
        case ON_BS_CHUNK_PAUSE:
        case ON_BS_TERMINATE:
        #endif
        case ON_LISTEN_FOR_DATA:
        {
            #ifdef ONE_NET_MULTI_HOP
            on_raw_did_t raw_did;
            #endif
            
            
            // TODO -- more work is needed to allow single messages to
            // go through DURING block and stream transfers, especially
            // from devices which are NOT the block / stream source device.
            
            // we are listening for data.  Make sure we have nothing
            // pending
            #ifndef BLOCK_MESSAGES_ENABLED
            if(*txn == NULL && single_txn.priority == ONE_NET_NO_PRIORITY)
            #else
            if((*txn == NULL || ((*txn == &bs_txn &&
              bs_msg.transfer_in_progress) && (on_state == ON_BS_CHUNK_PAUSE ||
              on_state == ON_BS_TERMINATE || get_bs_priority(bs_msg.flags) <
              ONE_NET_HIGH_PRIORITY))) && single_txn.priority ==
              ONE_NET_NO_PRIORITY)
            #endif
            {
                on_sending_device_t* device;
                #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                tick_t next_pop_time = 0;
                #endif
                
                // first see if we're in the middle of a message.
                if(!load_next_recipient(&single_msg, recipient_send_list_ptr))
                {
                    // we are not in the middle of a message.  We might have
                    // something ready to pop though.

                    #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
                    int index = single_data_queue_ready_to_send(
                        &next_pop_time);
                    #else
                    int index = single_data_queue_ready_to_send();
                    #endif
                    if(index >= 0)
                    {
                        #if SINGLE_QUEUE_LEVEL > NO_SINGLE_QUEUE_LEVEL
                        if(pop_queue_element(&single_msg, single_data_raw_pld,
                          (UInt8) index))
                        #else
                        if(pop_queue_element())
                        #endif
                        {
                            on_did_unit_t first_recipient;


                            // we have just popped a message.  Set things up
                            // with the recipient list, then next time the
                            // load_next_recipient will take it from there.
                            recipient_send_list_ptr = &recipient_send_list;
                            

                            one_net_memmove(first_recipient.did, single_msg.dst_did,
                              ON_ENCODED_DID_LEN);
                              
                            // this will only be relevant for ON_APP_MSG
                            // message types.  Fill it in anyway.  If it's
                            // irrelevant, it will be ignroed later on it the
                            // process.
                            first_recipient.unit = get_dst_unit(single_msg.payload);

                            // first clear the list and set up the direct
                            // recipient                            
                            recipient_send_list_ptr->num_recipients = 0;
                            
                            add_recipient_to_recipient_list(
                              recipient_send_list_ptr, &first_recipient);
                            
                            #ifdef PEER
                            // now add any peers, if they are relevant
                            add_peers_to_recipient_list(&single_msg,
                              recipient_send_list_ptr, peer);
                            #endif
                            
                            #ifdef ONE_NET_CLIENT
                            // now add the master if needed
                            if(must_send_to_master(&single_msg))
                            {
                                on_did_unit_t master_unit;
                                one_net_memmove(master_unit.did,
                                  &MASTER_ENCODED_DID, ON_ENCODED_DID_LEN);
                                master_unit.unit = ONE_NET_DEV_UNIT;
                                add_recipient_to_recipient_list(
                                  recipient_send_list_ptr, &master_unit);
                            }
                            #endif
                            
                            #ifndef ONE_NET_SIMPLE_CLIENT
                            #if defined(BLOCK_MESSAGES_ENABLED) && defined(DATA_RATE_CHANNEL)
                            if(bs_msg.transfer_in_progress &&
                              single_msg.msg_type == ON_ADMIN_MSG &&
                              single_msg.payload[0] ==
                              ON_CHANGE_DATA_RATE_CHANNEL)
                            {
                                #ifdef ONE_NET_MULTI_HOP
                                // add any repeaters.
                                UInt8 i;
                                on_did_unit_t did_unit;
                                
                                did_unit.unit = ONE_NET_DEV_UNIT;
                                for(i = 0; i < bs_msg.num_repeaters; i++)
                                {
                                    one_net_memmove(did_unit.did,
                                      bs_msg.repeaters[i], ON_ENCODED_DID_LEN);
                                    add_recipient_to_recipient_list(
                                      recipient_send_list_ptr, &did_unit);
                                }
                                #endif
                            }
                            else
                            #endif
                            {
                                (*pkt_hdlr.adj_recip_list_hdlr)(&single_msg,
                                  &recipient_send_list_ptr);
                            }
                            #endif

                            if(recipient_send_list_ptr)
                            {
                                #ifndef ONE_NET_SIMPLE_CLIENT
                                at_least_one_response = FALSE;
                                #endif
                                recipient_send_list_ptr->recipient_index = -1;
                                one_net(txn);
                                return; // we have a message. Send it through
                                        // one_net() again.
                            }
                        }
                    }
                    
                    // we are either done or we never had anything to send in
                    // the first place.  Clear the single message pointer.
                    single_msg_ptr = NULL;
                    
                    #ifdef BLOCK_MESSAGES_ENABLED
                    if(bs_msg.transfer_in_progress && !bs_msg.src)
                    {
                        UInt8 transfer_type =
                          get_bs_transfer_type(bs_msg.flags);
                        BOOL long_transfer;
                        #ifdef STREAM_MESSAGES_ENABLED
                        if(transfer_type == ON_BLK_TRANSFER)
                        #endif
                        {
                            long_transfer =
                              (bs_msg.bs.block.transfer_size > 2000);
                        }
                        #ifdef STREAM_MESSAGES_ENABLED
                        else
                        {
                            long_transfer = (bs_msg.time > 2000);
                        }
                        #endif
                        
                        if(!ont_get_timer(ONT_BS_TIMER) &&
                          single_data_queue_size == 0)
                        {
                            static UInt8 rptr_idx;
                            on_raw_did_t raw_did;
                            BOOL master_involved = (device_is_master ||
                              is_master_did((const on_encoded_did_t*)
                              &(bs_msg.dst->did)));
                            
                            on_decode(raw_did, bs_msg.dst->did,
                              ON_ENCODED_DID_LEN);
                                                        
                            // we are through waiting for things, so it's
                            // time to revert back to or start our block /
                            // stream transaction.  Let's see where we are.
                            
                            switch(bs_msg.bs_on_state)
                            {
                                case ON_LISTEN_FOR_DATA:
                                    bs_msg.bs_on_state = ON_BS_FIND_ROUTE;
                                case ON_BS_FIND_ROUTE:
                                    #ifdef ONE_NET_MULTI_HOP
                                    bs_msg.num_repeaters = 0;
                                    #endif
                                case ON_BS_CONFIRM_ROUTE:
                                    send_route_msg((const on_raw_did_t*)
                                      &raw_did);
                                    break;
                                #ifdef DATA_RATE_CHANNEL
                                case ON_BS_CHANGE_DR_CHANNEL:
                                    if(bs_msg.channel == on_base_param->channel
                                      && bs_msg.data_rate ==
                                      on_base_param->data_rate)
                                    {
                                        // no need to change anything
                                        bs_msg.bs_on_state = ON_BS_CONFIRM_ROUTE;
                                    }
                                    else
                                    {
                                        // add short pause of 125 ms to allow for any
                                        // missed ACKs or NACKs.  3000 ms should be
                                        // plenty long enough to get things done elsewhere
                                        // if need be.
                                        on_change_dr_channel(
                                          (const on_encoded_did_t*)
                                          &(bs_msg.dst->did), 125, 3000,
                                          bs_msg.channel, bs_msg.data_rate);
                                    }
                                    break;
                                case ON_BS_CHANGE_MY_DR_CHANNEL:
                                    // everyone else's data rate and channel has
                                    // been changed, so change ours now and give a
                                    // 125 ms pause not before the change, but before sending
                                    // any messages.
                                    on_change_dr_channel(NULL, 0, 3000,
                                      bs_msg.channel, bs_msg.data_rate);
                                    ont_set_timer(ONT_BS_TIMER, MS_TO_TICK(125));
                                    bs_msg.bs_on_state = ON_BS_CONFIRM_ROUTE;
                                    break;
                                #endif
                                
                                
                                case ON_BS_DEVICE_PERMISSION:
                                    #ifdef STREAM_MESSAGES_ENABLED
                                    if(transfer_type == ON_BLK_TRANSFER)
                                    #endif
                                    {
                                        // estimated completion time
                                        bs_msg.time =
                                          get_tick_count() + MS_TO_TICK(
                                          estimate_block_transfer_time(
                                          &bs_msg));
                                    }                              
                                    send_bs_setup_msg(&bs_msg,
                                      (const on_encoded_did_t*)
                                      &bs_msg.dst->did);
                                    break;
                                    
                                #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
                                case ON_BS_MASTER_DEVICE_PERMISSION:
                                    if(master_involved || !long_transfer)
                                    {
                                        // Master is involved, so we either ARE
                                        // the master or the master is the
                                        // device we are sending to, so if we
                                        // are the master, we don't need our
                                        // permission.  If the master is the
                                        // destination, we'll get the master's
                                        // permission when we ask the
                                        // destination device's permission.
                                        
                                        // On the other hand, if this is a short transfer,
                                        // we won't bother to ask for the master's permission
                                        // either.
                                        bs_msg.bs_on_state += 4;
                                    }
                                    else
                                    {
                                        #ifdef DATA_RATE_CHANNEL
                                        // ask the master for permission.
                                        // We'll need to drop back down
                                        // to the base channel and data rate
                                        one_net_set_channel(
                                          on_base_param->channel);
                                        one_net_set_data_rate(
                                          ONE_NET_DATA_RATE_38_4);
                                        #endif
                                        send_bs_setup_msg(&bs_msg,
                                          &MASTER_ENCODED_DID);
                                    }
                                    break;
                                #endif
                                    
                                #if defined(BLOCK_STREAM_REQUEST_MASTER_PERMISSION) && defined(ONE_NET_MULTI_HOP)
                                case ON_BS_MASTER_REPEATER_PERMISSION_START:
                                    if(device_is_master || bs_msg.num_repeaters
                                      == 0 || !long_transfer)
                                    {
                                        // master is involved or it is a short
                                        // transfer of less than 2000 bytes, so
                                        // don't bother asking the master.
                                        bs_msg.bs_on_state += 6;
                                        break;
                                    }

                                    rptr_idx = 0;
                                    bs_msg.bs_on_state++;
                                    break;
                                case ON_BS_MASTER_REPEATER_PERMISSION:
                                    #ifdef DATA_RATE_CHANNEL
                                    // The master is at the main data rate
                                    // and channel, so change to them.
                                    one_net_set_data_rate(
                                      ONE_NET_DATA_RATE_38_4);
                                    one_net_set_channel(
                                      on_base_param->channel);
                                    #endif
                                    request_reserve_repeater(&bs_msg,
                                      (const on_encoded_did_t*)
                                      &(bs_msg.repeaters[rptr_idx]));
                                    break;                                
                                case ON_BS_MASTER_REPEATER_PERMISSION_END:
                                    rptr_idx++;
                                    if(rptr_idx < bs_msg.num_repeaters)
                                    {
                                        bs_msg.bs_on_state -= 4;
                                        break;
                                    }
                                    bs_msg.bs_on_state++;
                                    break;
                                #endif
                                
                                #ifdef ONE_NET_MULTI_HOP
                                case ON_BS_REPEATER_PERMISSION_START:
                                    if(bs_msg.num_repeaters == 0)
                                    {
                                        bs_msg.bs_on_state += 6;
                                        break;
                                    }

                                    rptr_idx = 0;
                                    bs_msg.bs_on_state++;
                                    break;
                                case ON_BS_REPEATER_PERMISSION:
                                    #ifdef DATA_RATE_CHANNEL
                                    one_net_set_data_rate(bs_msg.data_rate);
                                    one_net_set_channel(bs_msg.channel);
                                    #endif
                                    send_bs_setup_msg(&bs_msg,
                                      (const on_encoded_did_t*)
                                      &(bs_msg.repeaters[rptr_idx]));
                                    break;
                                case ON_BS_REPEATER_PERMISSION_END:
                                    rptr_idx++;
                                    if(rptr_idx < bs_msg.num_repeaters)
                                    {
                                        bs_msg.bs_on_state -= 4;
                                        break;
                                    }
                                    bs_msg.bs_on_state++;
                                    break;
                                #endif
                                
                                
                                case ON_BS_COMMENCE:
                                {
                                    #ifdef ONE_NET_MULTI_HOP
                                    UInt16 data_pid = ONE_NET_RAW_BLOCK_DATA;
                                    #endif
                                    UInt16 response_pid =
                                      ONE_NET_RAW_SINGLE_DATA_ACK;
                                    #ifdef ONE_NET_MULTI_HOP
                                    data_pid |= (get_default_num_blocks(
                                      data_pid) << 8);
                                    #endif
                                    response_pid |= (get_default_num_blocks(
                                      response_pid) << 8);
                                      
                                    bs_msg.dst->msg_id++;
                                    bs_txn.key = (one_net_xtea_key_t*)
                                      on_base_param->current_key;
                                    bs_msg.bs.block.byte_idx = 0;
                                    bs_msg.bs.block.chunk_idx = 0;
                                    bs_msg.bs_on_state =
                                      ON_BS_PREPARE_DATA_PACKET;
                                    ont_set_timer(ONT_BS_TIMER, 0);
                                    ont_set_timer(ONT_BS_TIMEOUT_TIMER,
                                      MS_TO_TICK(bs_msg.timeout));
                                    one_net_memset(bs_msg.bs.block.sent, 0,
                                      sizeof(bs_msg.bs.block.sent));
                                    
                                    // we'll make fairly long process times.
                                    // Things will get corrected soon enough
                                    // anyway.  Just make the processing time
                                    // 20 ms for end devices and 10 ms for
                                    // repeaters.
                                    
                                    // TODO -- perhaps do more with the time
                                    // estimate?
                                    #ifdef STREAM_MESSAGES_ENABLED
                                    if(transfer_type == ON_BLK_TRANSFER)
                                    #endif
                                    {
                                        // TODO -- delete this?
                                        #ifdef ONE_NET_MULTI_HOP
                                        bs_msg.time = estimate_response_time(
                                          get_encoded_packet_len(data_pid, TRUE),
                                          get_encoded_packet_len(response_pid, TRUE),
                                          get_bs_hops(bs_msg.flags), 20, 10,
                                          bs_msg.data_rate);
                                        #else
                                        bs_msg.time = estimate_response_time(
                                          get_encoded_packet_len( response_pid,
                                          TRUE), 20, bs_msg.data_rate);
                                        #endif
                                    }
                                    #ifdef STREAM_MESSAGES_ENABLED
                                    else
                                    {
                                        bs_msg.bs.stream.last_response_time = 0;
                                        bs_msg.bs.stream.start_time = get_tick_count();
                                    }
                                    #endif
                                    break;
                                }
                                
                                case ON_BS_CHUNK_PAUSE:
                                {
                                    if(ont_inactive_or_expired(ONT_BS_TIMER))
                                    {
                                        bs_msg.bs_on_state =
                                          ON_BS_PREPARE_DATA_PACKET;
                                    }
                                    break;
                                }
                                default:
                                {
                                    // abort for now.
                                    terminate_bs_msg(&bs_msg, NULL, ON_MSG_FAIL,
                                      NULL);
                                    return;
                                }
                            }

                            on_state = bs_msg.bs_on_state;
                        }
                    }
                    #endif
                    
                    #ifdef ONE_NET_CLIENT
                    #ifdef ONE_NET_MASTER
                    if(device_is_master || client_joined_network)
                    #else
                    if(client_joined_network)
                    #endif
                    #endif
                    {
                        // accept a packet from anyone.
                        one_net_memmove(expected_src_did,
                          ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
                    }

                    // nothing popped, so look for a packet.  Any packet we
                    // get should be a single packet or possibly, if we are
                    // a repeater and the packet wasn't for us, a repeat
                    // packet.  What we should NOT get is a response, block,
                    // or stream packet.
                    response_txn.pkt =
                      &encoded_pkt_bytes[ON_MAX_ENCODED_DATA_PKT_SIZE];
                    single_txn.pkt = encoded_pkt_bytes;
                    this_txn = &single_txn;
                    #ifdef BLOCK_MESSAGES_ENABLED
                    bs_txn.pkt = encoded_pkt_bytes;
                    if(bs_msg.transfer_in_progress && !bs_msg.dst)
                    {
                        this_txn = &bs_txn;
                    }
                    #endif

                    *txn = NULL;
                    this_pkt_ptrs = &data_pkt_ptrs;

                    #if defined(BLOCK_MESSAGES_ENABLED) || defined(ONE_NET_MH_CLIENT_REPEATER)
                    status = on_rx_packet((const on_txn_t* const) *txn,
                      &this_txn, &this_pkt_ptrs, raw_payload_bytes);
                    #else
                    status = on_rx_packet(&this_txn, &this_pkt_ptrs,
                      raw_payload_bytes);
                    #endif
            
                    if(status == ONS_PKT_RCVD)
                    {
                        #ifdef ONE_NET_MH_CLIENT_REPEATER
                        if(this_txn == &mh_txn)
                        {
                            *txn = &mh_txn;
                            mh_txn.priority = ONE_NET_LOW_PRIORITY;
            
                            // copy the preamble / header just in case it isn't there already
                            one_net_memmove(mh_txn.pkt, HEADER, ONE_NET_PREAMBLE_HEADER_LEN);

                            // set the timer to send right away.
                            ont_set_timer(mh_txn.next_txn_timer, 0);
                    
                            on_state = ON_SEND_PKT;
                            return;
                        }
                        #endif
                        
                        if(this_txn == &single_txn)
                        {
                            on_message_status_t msg_status;
                            
                            UInt8 msg_type =
                              get_payload_msg_type(raw_payload_bytes);
                              
                            ack_nack.payload = (ack_nack_payload_t*)
                              &raw_payload_bytes[ON_PLD_DATA_IDX];
                              
                            // first check for message ids, features,
                            // etc.  We'll take care of that level of
                            // processing in rx_single_data().  Anything
                            // beyond that will take place in the single data
                            // handler function.
                            msg_status = rx_single_data(&this_txn,
                              this_pkt_ptrs, raw_payload_bytes, &ack_nack);

                            if(msg_status == ON_MSG_CONTINUE)
                            {
                                (*pkt_hdlr.single_data_hdlr)(&this_txn,
                                  this_pkt_ptrs, raw_payload_bytes, &msg_type,
                                  &ack_nack);
                            }

                            if(this_txn == &response_txn)
                            {
                                // we'll send back a reply.
                                *txn = &response_txn;
                                on_state = ON_SEND_SINGLE_DATA_RESP;
                                response_txn.priority = ONE_NET_HIGH_PRIORITY;
                                
                                // send the response immediately.
                                ont_set_timer((*txn)->next_txn_timer, 0);
                            }
                            else
                            {
                                *txn = 0;
                            }
                        }
                        
                        #ifdef BLOCK_MESSAGES_ENABLED
                        if(this_txn == &bs_txn)
                        {
                            BOOL respond;
                            on_message_status_t msg_status = ON_MSG_IGNORE;
                            block_stream_pkt_t bs_pkt;
                            
                            // TODO -- check for / reject invalid message id.
                            
                            #ifdef STREAM_MESSAGES_ENABLED
                            BOOL is_stream_txn = (get_bs_transfer_type(
                              bs_msg.flags) == ON_STREAM_TRANSFER);
                            #endif
                            *txn = 0;

                            #ifdef STREAM_MESSAGES_ENABLED
                            if(is_stream_txn)
                            {
                                if(!on_parse_stream_pld(raw_payload_bytes,
                                  &bs_pkt.stream_pkt))
                                {
                                    break;
                                }
                                
                                msg_status = rx_stream_data(&bs_txn, &bs_msg,
                                  &bs_pkt.stream_pkt, &ack_nack);
                                  
                                if(msg_status == ON_MSG_RESPOND)
                                {
                                    // Now reset some timers so we don't timeout.
                                    ont_set_timer(ONT_BS_TIMEOUT_TIMER,
                                      MS_TO_TICK(bs_msg.timeout));
                                    #ifdef DATA_RATE_CHANNEL
                                    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER,
                                      MS_TO_TICK(bs_msg.timeout));
                                    #endif
                                }
                                                                  
                                respond = (bs_pkt.stream_pkt.response_needed &&
                                  msg_status == ON_MSG_RESPOND);
                            }
                            else
                            #endif
                            {
                                if(!on_parse_block_pld(raw_payload_bytes,
                                  &bs_pkt.block_pkt))
                                {
                                    break;
                                } 
                                                                
                                msg_status = rx_block_data(&bs_txn, &bs_msg,
                                  &bs_pkt.block_pkt, &ack_nack);
                                  
                                if(msg_status == ON_MSG_RESPOND)
                                {
                                    // Now reset some timers so we don't timeout.
                                    ont_set_timer(ONT_BS_TIMEOUT_TIMER,
                                      MS_TO_TICK(bs_msg.timeout));
                                    #ifdef DATA_RATE_CHANNEL
                                    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER,
                                      MS_TO_TICK(bs_msg.timeout));
                                    #endif
                                }  
                                  
                                respond = (bs_pkt.block_pkt.chunk_idx + 1 ==
                                  bs_pkt.block_pkt.chunk_size) &&
                                  (msg_status == ON_MSG_RESPOND);
                            }
                            
                            if(respond)
                            {
                                // we'll send back a reply.
                                UInt16 response_pid;
                                
                                if(!bs_msg.use_saved_ack_nack)
                                {
                                    bs_msg.saved_ack_nack.nack_reason =
                                      ack_nack.nack_reason;
                                    bs_msg.saved_ack_nack.handle =
                                      ack_nack.handle;
                                    one_net_memmove(
                                      bs_msg.saved_ack_nack.payload,
                                      ack_nack.payload, 5);
                                }
                                
                                response_pid = (
                                  (bs_msg.saved_ack_nack.nack_reason ==
                                  ON_NACK_RSN_NO_ERROR) ?
                                  ONE_NET_RAW_SINGLE_DATA_ACK :
                                  ONE_NET_RAW_SINGLE_DATA_NACK_RSN);
                                  
                                #ifdef ONE_NET_MULTI_HOP
                                response_txn.hops = get_bs_hops(bs_msg.flags);
                                response_txn.max_hops = response_txn.hops;
                                set_multihop_pid(&response_pid,
                                  response_txn.max_hops);
                                #endif
                                  
                                if(!setup_pkt_ptr(response_pid,
                                  response_txn.pkt, bs_pkt.block_pkt.msg_id,
                                  &response_pkt_ptrs))
                                {
                                    break;
                                }
    
                                if(on_build_my_pkt_addresses(&response_pkt_ptrs,
                                  (const on_encoded_did_t*)&(bs_msg.src->did),
                                  NULL) != ONS_SUCCESS)
                                {
                                    break;
                                }

                                response_txn.key = bs_txn.key;
                                response_txn.priority = ONE_NET_HIGH_PRIORITY;
                                
                                if(on_build_response_pkt(&bs_msg.saved_ack_nack,
                                  &response_pkt_ptrs, &response_txn, FALSE) !=
                                  ONS_SUCCESS)
                                {
                                    break;
                                }
                                if(on_complete_pkt_build(&response_pkt_ptrs,
                                  response_pid) != ONS_SUCCESS)
                                {
                                    break;
                                }
                                
                                // send the response immediately.
                                *txn = &response_txn;
                                ont_set_timer((*txn)->next_txn_timer, 0);
                                on_state = ON_SEND_SINGLE_DATA_RESP;
                                bs_msg.use_saved_ack_nack = FALSE;
                            }
                        }
                        #endif
                    }
                    
                    #ifdef DATA_RATE_CHANNEL
                    if(*txn == 0)
                    {
                        // see if we need to change data rates.
                        check_dr_channel_change();
                    }
                    #endif
                    break;
                }

                // we have a message.  Let's create the packet.
                single_txn.pkt =
                  &encoded_pkt_bytes[ON_MAX_ENCODED_DATA_PKT_SIZE];
                response_txn.pkt = encoded_pkt_bytes;
                
                // first get the sending device info.
                device = (*get_sender_info)((const on_encoded_did_t* const)
                  single_msg.dst_did);
                if(device == NULL)
                {
                    // an error occurred somewhere.  Abort.
                    return; // no pending transaction since we're
                                 // aborting
                }
                
                // signifies the time we verified this message with this
                // client to prevent replay attacks.
                device->verify_time = 0;
                

                // TODO -- look at rollover code.
                // pick a message id.  Increment the last one.  If it
                // rolls over, make it 0.
                (device->msg_id)++;
                if(device->msg_id > ON_MAX_MSG_ID)
                {
                    device->msg_id = 0;
                }
                
                single_txn.priority = single_msg.priority;
                *txn = &single_txn;
                (*txn)->retry = 0;
                (*txn)->response_timeout = one_net_response_time_out;
                (*txn)->device = device;
                
                // we'll need to fill in the key.  We're dealing with
                // a single transaction here.  Fill in the key.
                #ifdef ONE_NET_CLIENT
                single_txn.key =
                  (one_net_xtea_key_t*) on_base_param->current_key;
                #endif
                #ifdef ONE_NET_MASTER
                if(device_is_master)
                {
                    // we're the master.  We may or may not be dealing
                    // with a client using the old key.
                    single_txn.key = master_get_encryption_key(
                      (const on_encoded_did_t* const) single_msg.dst_did);
                }
                #endif
                
                #ifndef ONE_NET_SIMPLE_CLIENT
                // give the application code a chance to change whatever it
                // wants or abort.
                one_net_single_msg_loaded(txn, &single_msg);
                if(*txn == NULL)
                {
                    // aborted by the application code
                    return;
                }
                #endif
                
                if(!setup_pkt_ptr(single_msg.raw_pid, single_txn.pkt, device->msg_id,
                  &data_pkt_ptrs))
                {
                    // an error of some sort occurred.  We likely have
                    // a bad pid.  Unrecoverable.  Just abort.
                    return; // no outstanding transaction
                }
                
                #ifdef ONE_NET_MULTI_HOP
                // TODO -- we already gave the applciation code a chance to
                // change things above.  The code below seems redundant at best
                // and conterproductive at worst in that changes previously made
                // in the application code might be accidentally undone here?
                on_decode(raw_did, device->did, ON_ENCODED_DID_LEN);
                single_txn.hops = 0;
                single_txn.max_hops = device->hops;

                // give the application code a chance to override if it
                // wants to.
                switch(one_net_adjust_hops((const on_raw_did_t* const) &raw_did,
                  &(single_txn.max_hops)))
                {
                    case ON_MSG_ABORT: return; // aborting
                    #ifdef COMPILE_WO_WARNINGS
                    // add default case that does nothing for clean compile
                    default:
                        break;
                    #endif
                }
                
                data_pkt_ptrs.hops = single_txn.hops;
                data_pkt_ptrs.max_hops = single_txn.max_hops;
                #endif                                
                
                // now fill in the packet
                
                // fill in the addresses
                if(on_build_my_pkt_addresses(&data_pkt_ptrs,
                  (const on_encoded_did_t*) single_msg.dst_did,
                  (const on_encoded_did_t*) single_msg.src_did) != ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return; // no outstanding transaction
                }
                
                // It's a data packet.  Fill in the data portion
                #ifdef ONE_NET_CLIENT
                features_override = features_override ||
                  !features_known(device->features);
                #endif
                #ifndef BLOCK_MESSAGES_ENABLED
                if(on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn) !=
                  ONS_SUCCESS)
                #else
                if(on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn, NULL) !=
                  ONS_SUCCESS)
                #endif
                {
                    // An error of some sort occurred.  Abort.
                    return; // no outstanding transaction
                }

                // now finish building the packet.
                if(on_complete_pkt_build(&data_pkt_ptrs, single_msg.raw_pid) !=
                  ONS_SUCCESS)
                {
                    // An error of some sort occurred.  Abort.
                    return; // no outstanding transaction                            
                }

                // packet was built successfully.  Set the transaction,
                // state, etc.
                single_txn.data_len =
                  get_encoded_packet_len(single_msg.raw_pid, TRUE);
                one_net_memmove(expected_src_did,
                  &(data_pkt_ptrs.packet_bytes[ON_ENCODED_DST_DID_IDX]),
                  ON_ENCODED_DID_LEN);
                
                #ifndef BLOCK_MESSAGES_ENABLED
                on_state = ON_SEND_SINGLE_DATA_PKT;
                #else
                if(on_state > ON_BS_COMMENCE && on_state < ON_BS_TERMINATE)
                {
                    bs_msg.bs_on_state = ON_BS_CHUNK_PAUSE;
                    // make the pause half the timeout
                    pause_bs_msg(&bs_msg, MS_TO_TICK(bs_msg.timeout / 2));
                    on_state = ON_LISTEN_FOR_DATA;
                }
                
                if(on_state == ON_LISTEN_FOR_DATA)
                {
                    on_state = ON_SEND_SINGLE_DATA_PKT;
                }
                else
                {
                    on_state++;
                }
                #endif
                
                // set the timer to send immediately
                ont_set_timer((*txn)->next_txn_timer, 0);
                single_msg_ptr = &single_msg;
                return;
            }

            break;
        } // case ON_LISTEN_FOR_DATA //
        
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_PREPARE_DATA_PACKET:
        {
            tick_t tick_now = get_tick_count();
            #ifdef STREAM_MESSAGES_ENABLED
            UInt8 transfer_type = get_bs_transfer_type(bs_msg.flags);
            UInt16 raw_pid = ((transfer_type == ON_BLK_TRANSFER) ?
              ONE_NET_RAW_BLOCK_DATA : ONE_NET_RAW_STREAM_DATA);
            #else
            UInt16 raw_pid = ONE_NET_RAW_BLOCK_DATA;
            #endif            
            one_net_status_t status;
            on_message_status_t msg_status;
            // TODO -- I think there's a global buffer somewhere so we don't
            // need to set our own buffer.  However, the stack size shouldn't
            // be very high here so it's no big deal.
            UInt8 buffer[ON_BS_DATA_PLD_SIZE];
            ack_nack.nack_reason = ON_NACK_RSN_NO_ERROR;
            ack_nack.handle = ON_ACK;
            
            if(!ont_inactive_or_expired(ONT_BS_TIMER))
            {
                if(get_bs_priority(bs_msg.flags) == ONE_NET_LOW_PRIORITY)
                {
                    tick_t next_pop_time;
                    if(single_data_queue_ready_to_send(&next_pop_time) != -1)
                    {
                        on_state = ON_BS_CHUNK_PAUSE;
                        bs_msg.bs_on_state = ON_BS_PREPARE_DATA_PACKET;
                    }
                }
                break;
            }
            
            bs_txn.pkt = encoded_pkt_bytes;
            if(!setup_pkt_ptr(raw_pid, bs_txn.pkt, bs_msg.dst->msg_id,
              &data_pkt_ptrs))
            {
                break; // should never get here?
            }
            
            *txn = &bs_txn;
            #ifdef STREAM_MESSAGES_ENABLED
            if(transfer_type == ON_BLK_TRANSFER)
            #endif
            {
                msg_status = one_net_block_get_next_payload(&bs_msg, buffer,
                  &ack_nack);
            }
            #ifdef STREAM_MESSAGES_ENABLED
            else
            {
                // We need a response we haven't gotten one in the last 5
                // seconds.
                bs_msg.response_needed = (tick_now -
                  bs_msg.bs.stream.last_response_time > STREAM_RESPONSE_INTERVAL);
                bs_msg.bs.stream.elapsed_time = TICK_TO_MS(tick_now -
                  bs_msg.bs.stream.start_time);
                msg_status = one_net_stream_get_next_payload(&bs_msg, buffer,
                  &ack_nack);
            }
            #endif
                  
            if(msg_status == ON_MSG_CONTINUE)
            {
                // fill in the addresses
                if(on_build_my_pkt_addresses(&data_pkt_ptrs,
                  (const on_encoded_did_t*) &(bs_msg.dst->did), NULL) !=
                  ONS_SUCCESS)
                {
                    break;
                }
                
                #ifdef STREAM_MESSAGES_ENABLED
                if(transfer_type == ON_BLK_TRANSFER)
                #endif
                {
                    // We need a response if any of the following is true so
                    // we may temporarily change the chunk size when building
                    // the packet, then immediately change it back.  If we
                    // are in the first or last 1000 bytes in the transfer,
                    // we'll change the chunk sizeto 1.
                    UInt8 original_chunk_size = bs_msg.bs.block.chunk_size;
            
                    // possible quick change.  We'll change back immediately
                    // after the function call.
                    bs_msg.bs.block.chunk_size =
                      get_current_bs_chunk_size(&bs_msg);
                    
                    // If we are sending anything but the last packet, we'll
                    // reset the timeout timer.  Otherwise we expect a
                    // response, so we'll reset it when / if we get one.
                    bs_msg.response_needed = (bs_msg.bs.block.chunk_idx + 1 ==
                      bs_msg.bs.block.chunk_size);

                    status = on_build_data_pkt(buffer, ON_APP_MSG,
                      &data_pkt_ptrs, &bs_txn, &bs_msg);
                    // change back if it was changed before.
                    bs_msg.bs.block.chunk_size = (UInt8) original_chunk_size;
                }
                #ifdef STREAM_MESSAGES_ENABLED
                else
                {
                    if(bs_msg.bs.stream.elapsed_time >= bs_msg.time)
                    {
                        terminate_bs_msg(&bs_msg, NULL, ON_MSG_SUCCESS, NULL);
                        return;
                    }
                    status = on_build_data_pkt(buffer, ON_APP_MSG,
                      &data_pkt_ptrs, &bs_txn, &bs_msg);
                }
                #endif

                if(status != ONS_SUCCESS)
                {
                    break;
                }
                    
                // now finish building the packet.
                if(on_complete_pkt_build(&data_pkt_ptrs, raw_pid) !=
                  ONS_SUCCESS)
                {
                    break;                        
                }

                if(!bs_msg.response_needed)
                {
                    ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(
                      bs_msg.timeout));
                }

                bs_txn.data_len = get_encoded_packet_len(raw_pid, TRUE);
                #ifdef DATA_RATE_CHANNEL
                one_net_set_channel(bs_msg.channel);
                one_net_set_data_rate(bs_msg.data_rate);
                #endif
                on_state++;
            }
            else if(msg_status == ON_MSG_PAUSE)
            {
                UInt32 pause_time = bs_msg.bs.block.chunk_pause;
                if(ack_nack.handle == ON_ACK_PAUSE_TIME_MS)
                {
                    pause_time = ack_nack.payload->ack_time_ms;
                }
                pause_bs_msg(&bs_msg, (UInt16) pause_time);
            }
            else
            {
                terminate_bs_msg(&bs_msg, NULL, msg_status, &ack_nack);
            }
            break;
        }
        #endif
        
        case ON_SEND_PKT:
        #ifdef ONE_NET_MASTER
        case ON_SEND_INVITE_PKT:
        #endif
        case ON_SEND_SINGLE_DATA_PKT:
        case ON_SEND_SINGLE_DATA_RESP:
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_SEND_FIND_ROUTE:
        case ON_BS_SEND_CONFIRM_ROUTE:
        #ifdef DATA_RATE_CHANNEL
        case ON_BS_SEND_CHANGE_DR_CHANNEL:
        #endif
        case ON_BS_SEND_DEVICE_PERMISSION:
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_SEND_MASTER_DEVICE_PERMISSION:
        #endif
        #ifdef ONE_NET_MULTI_HOP
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_SEND_MASTER_REPEATER_PERMISSION:
        #endif
        case ON_BS_SEND_REPEATER_PERMISSION:
        #endif
        case ON_BS_SEND_DATA_PKT:
        case ON_BS_SEND_TERMINATE_PACKET:
        #endif
        {
            #ifndef BLOCK_MESSAGES_ENABLED
            if(ont_inactive_or_expired((*txn)->next_txn_timer)
              && check_for_clr_channel())
            #else
            // We don't do "retries" with block / stream data packets.  We
            // do "re-sends" and we timeout based on criteria other than having
            // too many retries, so the timer test is counterproductive.  We
            // handle it elsewhere.  If it got to this state and the channel
            // is clear, it's ready to go.  We checked timers elsewhere.
            if((on_state == ON_BS_SEND_DATA_PKT || ont_inactive_or_expired(
              (*txn)->next_txn_timer)) && check_for_clr_channel())
            #endif
            {
                UInt16 raw_pid;
                if(get_raw_pid(&((*txn)->pkt[ON_ENCODED_PID_IDX]), &raw_pid))
                {
                    one_net_write((*txn)->pkt, get_encoded_packet_len(raw_pid,
                      TRUE));
                    on_state++;
                }
                #ifdef BLOCK_MESSAGES_ENABLED
                else if(on_state == ON_BS_SEND_DATA_PKT)
                {
                    // TODO -- why are we getting here?
                    ont_set_timer(ONT_BS_TIMER, 0);
                    on_state--;  // something happened.  Not sure what.  Re-prepare
                                 // packet
                }
                #endif
            } // if the channel is clear //
            
            break;
        } // case ON_SEND_SINGLE_DATA_PKT //
        
        case ON_SEND_PKT_WRITE_WAIT:
        #ifdef ONE_NET_MASTER
        case ON_SEND_INVITE_PKT_WRITE_WAIT:
        #endif
        case ON_SEND_SINGLE_DATA_WRITE_WAIT:
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_SEND_FIND_ROUTE_WRITE_WAIT:
        case ON_BS_SEND_CONFIRM_ROUTE_WRITE_WAIT:
        #ifdef DATA_RATE_CHANNEL
        case ON_BS_SEND_CHANGE_DR_CHANNEL_WRITE_WAIT:
        #endif
        case ON_BS_SEND_DEVICE_PERMISSION_WRITE_WAIT:
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_SEND_MASTER_DEVICE_PERMISSION_WRITE_WAIT:
        #endif
        #ifdef ONE_NET_MULTI_HOP
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_SEND_MASTER_REPEATER_PERMISSION_WRITE_WAIT:
        #endif
        case ON_BS_SEND_REPEATER_PERMISSION_WRITE_WAIT:
        #endif
        case ON_BS_SEND_DATA_WRITE_WAIT:
        case ON_BS_SEND_TERMINATE_WRITE_WAIT:
        #endif
        {
            // TODO -- add handling for block / stream
            
            if(one_net_write_done())
            {
                UInt32 new_timeout_ms;
                
                #ifdef ROUTE
                UInt16 raw_pid;
                if(get_raw_pid(&((*txn)->pkt[ON_ENCODED_PID_IDX]), &raw_pid) &&
                  ((raw_pid & 0x3F) == ONE_NET_RAW_ROUTE))
                {
                    route_start_time = get_tick_count();
                }                                
                #endif
                
                #ifdef BLOCK_MESSAGES_ENABLED
                if(on_state == ON_BS_SEND_DATA_WRITE_WAIT)
                {
                    new_timeout_ms = MS_TO_TICK(bs_msg.frag_dly);
                    
                    // save the state so we can get back to it if need be
                    bs_msg.bs_on_state = ON_BS_PREPARE_DATA_PACKET;
                    #ifdef STREAM_MESSAGES_ENABLED
                    if(get_bs_transfer_type(bs_msg.flags) == ON_STREAM_TRANSFER)
                    {
                        
                    }
                    else
                    #endif
                    {
                        // Chunk sizes are always 1 in the first and last 1000
                        // bytes of a message.  The get_current_bs_chunk_size()
                        // function will determine the current chunk size.
                        // If the current chunk index is 1 less than that value,
                        // we want a response.  If all packets within a chunk
                        // have been sent, we need a response.
                        UInt8 current_chunk_size = get_current_bs_chunk_size(
                          &bs_msg);
                        SInt8 new_chunk_idx;
                        
                        block_set_index_sent(bs_msg.bs.block.chunk_idx, TRUE,
                          bs_msg.bs.block.sent);
                          
                        new_chunk_idx = block_get_lowest_unsent_index(
                          bs_msg.bs.block.sent, current_chunk_size);
                          
                        bs_msg.response_needed = FALSE;  
                        if(new_chunk_idx == -1)
                        {
                            bs_msg.response_needed = (bs_msg.bs.block.chunk_idx
                              == current_chunk_size - 1);
                            new_chunk_idx = current_chunk_size - 1;
                        }

                        bs_msg.bs.block.chunk_idx = new_chunk_idx;
                    }
                    
                    if(bs_msg.response_needed)
                    {
                        UInt16 response_pid = (1 << 8) +
                          ONE_NET_RAW_SINGLE_DATA_ACK;
                        
                        // Set timer for triple the expected time?
                        #ifdef ONE_NET_MULTI_HOP
                        UInt16 data_pid = (4 << 8) + ONE_NET_RAW_BLOCK_DATA;
                        ont_set_timer(ONT_RESPONSE_TIMER,
                          MS_TO_TICK(3 * estimate_response_time(
                            get_encoded_packet_len(data_pid, TRUE),
                            get_encoded_packet_len(response_pid, TRUE),
                            get_bs_hops(bs_msg.flags), 20, 10,
                            bs_msg.data_rate)));
                        #else
                        ont_set_timer(ONT_RESPONSE_TIMER,
                          MS_TO_TICK(3 * estimate_response_time(
                            get_encoded_packet_len(response_pid, TRUE), 20,
                            bs_msg.data_rate)));
                        #endif
                        on_state = ON_BS_WAIT_FOR_DATA_RESP;
                        break;
                    }
                    else
                    {
                        // this is for high priority.
                        // TODO -- low priority.
                        // No response needed, so use the fragment
                        // delay as the pause.
                        ont_set_timer(ONT_BS_TIMER,
                          MS_TO_TICK(bs_msg.frag_dly));
                        on_state = ON_BS_PREPARE_DATA_PACKET;
                        break;
                    }
                }
                #endif
                
                #ifdef DATA_RATE_CHANNEL
                if(dr_channel_stage == ON_DR_CHANNEL_CHANGE_DONE)
                {
                    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER,
                        MS_TO_TICK(dormant_data_rate_time_ms));
                }
                #endif                
                
                #ifdef ONE_NET_MULTI_HOP
                new_timeout_ms = (*txn)->max_hops * ONE_NET_MH_LATENCY
                  + (1 + (*txn)->max_hops) * (*txn)->response_timeout;
                
                #ifdef ONE_NET_MASTER
                if(on_state == ON_SEND_INVITE_PKT_WRITE_WAIT)
                {
                    // invite packets don't lengthen timeout with
                    // multi-hop
                    new_timeout_ms = invite_txn.response_timeout;
                }
                #endif
                
                #else
                new_timeout_ms = (*txn)->response_timeout;
                #endif
                
                if(on_state == ON_SEND_PKT_WRITE_WAIT || on_state ==
                  ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT)
                {
                    // no timers are needed, no response is needed, we're just
                    // doing a quick write and that's it.  There is no
                    // follow-up, so we are done.  Reset the transaction and
                    // return the state to ON_LISTEN_FOR_DATA.
                    (*txn)->priority = ONE_NET_NO_PRIORITY;
                    *txn = NULL;
                    ont_stop_timer((*txn)->next_txn_timer);
                    on_state = ON_LISTEN_FOR_DATA;
                    return;
                }
                
                #ifdef ONE_NET_MASTER  
                if(on_state == ON_SEND_INVITE_PKT_WRITE_WAIT)
                {
                    on_state = ON_LISTEN_FOR_DATA;
                    ont_set_timer(invite_txn.next_txn_timer, MS_TO_TICK(
                      new_timeout_ms));
                    *txn = 0;
                }
                else
                #endif
                {
                    ont_set_timer(ONT_RESPONSE_TIMER, MS_TO_TICK(new_timeout_ms));
                    on_state++;
                }
            } // if write is complete //
            break;
        } // send single data write wait case //
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_WAIT_FOR_FIND_ROUTE_RESP:
        case ON_BS_WAIT_FOR_CONFIRM_ROUTE_RESP:
        #ifdef DATA_RATE_CHANNEL
        case ON_BS_WAIT_FOR_CHANGE_DR_CHANNEL_RESP:
        #endif
        case ON_BS_WAIT_FOR_DEVICE_PERMISSION_RESP:
        #ifdef BLOCK_STREAM_REQUEST_MASTER_PERMISSION
        case ON_BS_WAIT_FOR_MASTER_DEVICE_PERMISSION_RESP:
        #ifdef ONE_NET_MULTI_HOP
        case ON_BS_WAIT_FOR_MASTER_REPEATER_PERMISSION_RESP:
        #endif
        #endif
        #ifdef ONE_NET_MULTI_HOP
        case ON_BS_WAIT_FOR_REPEATER_PERMISSION_RESP:
        #endif
        case ON_BS_WAIT_FOR_DATA_RESP:
        case ON_BS_WAIT_FOR_TERMINATE_RESP:
        #endif
        case ON_WAIT_FOR_SINGLE_DATA_RESP:
        {
            // TODO -- add block /stream response handling
            
            on_message_status_t msg_status;
            BOOL terminate_txn = FALSE;
            BOOL response_msg_or_timeout = FALSE; // will be set to true if
                 // the response timer timed out or there was a response
            
            #ifndef ONE_NET_SIMPLE_CLIENT
            // send right away unless overruled later.
            UInt32 next_send_pause_time = 0;
            #endif
            
            if(ont_inactive_or_expired(ONT_RESPONSE_TIMER))
            {
                #ifndef ONE_NET_SIMPLE_CLIENT
                // we timed out without a response.  Notify application code
                // it's unlikely they'll decide to change the response time-
                // out, but they might.

                ack_nack_payload.nack_time_ms = (*txn)->response_timeout;
                ack_nack.handle = ON_NACK_TIMEOUT_MS;
                ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE;
                #endif
                
                #ifdef BLOCK_MESSAGES_ENABLED
                if(on_state == ON_BS_WAIT_FOR_DATA_RESP)
                {
                    if(ont_inactive_or_expired(ONT_BS_TIMEOUT_TIMER))
                    {
                        terminate_bs_msg(&bs_msg, NULL, ON_MSG_TIMEOUT, NULL);
                    }
                    else
                    {
                        // no response, so prepare the next packet, which may
                        // may or may not be this one.
                        on_state = ON_BS_PREPARE_DATA_PACKET;
                    }
                    
                    break;
                }
                #endif
                
                response_msg_or_timeout = TRUE;
                (*txn)->retry++;
                
                #ifndef ONE_NET_SIMPLE_CLIENT
                msg_status = (*pkt_hdlr.single_ack_nack_hdlr)(&single_txn,
                  &data_pkt_ptrs, single_msg_ptr->payload,
                  &(single_msg_ptr->msg_type), &ack_nack);
                  
                // we may have been given a pause by the application code,
                // so check if it has changed the nack handle to
                // ON_NACK_PAUSE_TIME_MS.  If so, we'll adjust the
                // next_send_pause_time variable.
                if(ack_nack.handle == ON_NACK_PAUSE_TIME_MS)
                {
                    next_send_pause_time = ack_nack_payload.nack_time_ms;
                }
                  
                // things may or may not have been changed.  One thing
                // that would change would be if the message was terminated
                // or failed in one way or another, so check msg_status
                // first.  The ACK-NACK handler will set this return value
                // if we have timed out too many times.
                switch(msg_status)
                {
                    case ON_MSG_DEFAULT_BHVR: case ON_MSG_CONTINUE:
                    {
                        // we continue.  The response timeout may or may not
                        // have been changed.  We don't care here.
                        msg_status = ON_MSG_TIMEOUT;
                        terminate_txn = ((*txn)->retry >= ON_MAX_RETRY);
                        break;
                    }
                    default:
                    {
                        // transaction has been terminated either by ONE-NET
                        // or by the application code.
                        terminate_txn = TRUE;
                    }
                }
                #else
                msg_status = ON_MSG_TIMEOUT;
                terminate_txn = ((*txn)->retry >= ON_MAX_RETRY);
                #endif
            }
            else
            {
                this_txn = &response_txn;
                this_pkt_ptrs = &response_pkt_ptrs;
                
                
                #if defined(BLOCK_MESSAGES_ENABLED) || defined(ONE_NET_MH_CLIENT_REPEATER)
                status = on_rx_packet(&single_txn, &this_txn, &this_pkt_ptrs,
                  raw_payload_bytes);
                #else
                status = on_rx_packet(&this_txn, &this_pkt_ptrs,
                  raw_payload_bytes);
                #endif
            
                if(status == ONS_PKT_RCVD)
                {                    
                    #ifdef BLOCK_MESSAGES_ENABLED
                    if(on_state == ON_BS_WAIT_FOR_DATA_RESP)
                    {
                        #ifdef STREAM_MESSAGES_ENABLED
                        if(get_bs_transfer_type(bs_msg.flags) ==
                          ON_STREAM_TRANSFER)
                        {
                            msg_status = rx_stream_resp_pkt(*txn, &bs_msg,
                              this_pkt_ptrs, raw_payload_bytes, &ack_nack);
                        }
                        else
                        #endif
                        {
                            msg_status = rx_block_resp_pkt(*txn, &bs_msg,
                              this_pkt_ptrs, raw_payload_bytes, &ack_nack);
                        }
                        
                        if(msg_status == ON_MSG_CONTINUE)
                        {
                            // At least one packet was not received, so
                            // send again immediately.
                            on_state = ON_BS_PREPARE_DATA_PACKET;
                            ont_set_timer(ONT_BS_TIMER, 0);
                        }
                        return;
                    }
                    #endif
                    
                    response_msg_or_timeout = TRUE;
                    msg_status = rx_single_resp_pkt(txn, &this_txn,
                      this_pkt_ptrs, raw_payload_bytes, &ack_nack);
                      
                    switch(msg_status)
                    {
                        case ON_MSG_IGNORE:
                            response_msg_or_timeout = FALSE;
                            break;
                        case ON_BS_MSG_SETUP_CHANGE:
                            terminate_txn = TRUE;
                            break;
                        default:
                            terminate_txn = ((*txn)->retry >= ON_MAX_RETRY ||
                              this_txn == 0);
                            #ifndef ONE_NET_SIMPLE_CLIENT
                            at_least_one_response = TRUE;
                            #endif
                    }
                }
                else
                {
                    break;
                }
            }
            
            if(nack_reason_is_fatal(ack_nack.nack_reason))
            {
                // if nothing else set this terminate flag, something
                // somewhere set a fatal NACK, which means this transaction
                // will never work, so we'll terminate.  Why bother going
                // through a bunch of retries that will never work?
                terminate_txn = TRUE;
            }

            if(!terminate_txn && response_msg_or_timeout)
            {
                // rebuild the packet                
                if(setup_pkt_ptr(single_msg.raw_pid, single_txn.pkt,
                  (*txn)->device->msg_id,
                  &data_pkt_ptrs) &&
                  #ifndef BLOCK_MESSAGES_ENABLED
                  on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn) ==
                  ONS_SUCCESS &&
                  #else
                  on_build_data_pkt(single_msg.payload,
                  single_msg.msg_type, &data_pkt_ptrs, &single_txn, NULL) ==
                  ONS_SUCCESS &&
                  #endif
                  on_complete_pkt_build(
                  &data_pkt_ptrs, single_msg.raw_pid) == ONS_SUCCESS)
                {
                    // do nothing. everything worked.
                }
                else
                {
                    // an error of some sort occurred.  Abort.
                    terminate_txn = TRUE;
                    msg_status = ON_MSG_INTERNAL_ERR;
                }
                
                if(!terminate_txn)
                {
                    #ifndef ONE_NET_SIMPLE_CLIENT
                    ont_set_timer((*txn)->next_txn_timer,
                      MS_TO_TICK(next_send_pause_time));
                    #else
                    // send right away.
                    ont_set_timer((*txn)->next_txn_timer, 0);
                    #endif

                    on_state -= 2;
                }
            }
            
            if(terminate_txn)
            {
                // transaction has been terminated either by ONE_NET
                // or by the application code.
                
                // first check whether we are close to the end of our message
                // IDs for the other device.  If so, request a key change for
                // the network.  We'll define "close to" as within 300 just to
                // pick a somewhat random number.
                if(one_net_reject_bad_msg_id((*txn)->device) &&
                  (*txn)->device->msg_id > ON_MAX_MSG_ID - 300)
                {
                    // to avoid duplicate requests, only request a key change
                    // at most every 60 seconds.
                    // TODO -- what if this is within 60 seconds of 
                    if(key_change_request_time + MS_TO_TICK(60000) <
                      get_tick_count())
                    {
                        key_change_requested = TRUE;
                    }
                }
                
                
                if((msg_status == ON_MSG_DEFAULT_BHVR || msg_status ==
                  ON_MSG_CONTINUE) && ack_nack.nack_reason !=
                    ON_NACK_RSN_NO_ERROR)
                {
                      msg_status = ON_MSG_FAIL;
                }

                #ifndef ONE_NET_SIMPLE_CLIENT
                // if we get no reponse this last try and we NEVER got any
                // reponse, we'll make the nack reason
                // ON_NACK_RSN_NO_RESPONSE_TXN.
                if(ack_nack.nack_reason == ON_NACK_RSN_NO_RESPONSE &&
                  !at_least_one_response)
                {
                    // if we got no response ever from the other device.
                    ack_nack.nack_reason = ON_NACK_RSN_NO_RESPONSE_TXN;
                }
                #endif
                
                #if  SINGLE_QUEUE_LEVEL == NO_SINGLE_QUEUE_LEVEL
                if(!recipient_send_list_ptr ||
                  recipient_send_list_ptr->num_recipients -
                  recipient_send_list_ptr->recipient_index <= 1)
                {
                    // for devices with no queue, we want to clear the queue
                    // if possible BEFORE sending to the transaction handler
                    // so that the transaction handler can send a message.
                    single_msg_ptr = NULL;
                    (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                      single_msg.payload, &(single_msg.msg_type),
                      msg_status, &ack_nack);                    
                }
                #else
                
                (*pkt_hdlr.single_txn_hdlr)(txn, &data_pkt_ptrs,
                  single_msg_ptr->payload, &(single_msg_ptr->msg_type),
                  msg_status, &ack_nack);
                single_msg_ptr = NULL;
                #endif
                
                #ifdef ONE_NET_MULTI_HOP
                // TODO -- this seems like the wrong place to put this.
                // What about the application code?
                (*txn)->device->hops = (*txn)->max_hops;
                #endif
                  
                // clear the transaction.
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                *txn = 0;
                
                #ifdef BLOCK_MESSAGES_ENABLED
                if(bs_msg.transfer_in_progress && !bs_msg.src && on_state
                  >= ON_BS_WAIT_FOR_FIND_ROUTE_RESP)
                {
                    bs_msg.bs_on_state = on_state + 1;
                    on_state -= 3;
                    if(on_state == ON_BS_TERMINATE)
                    {
                        on_state = ON_BS_TERMINATE_COMPLETE;
                    }
                    else if(msg_status == ON_BS_MSG_SETUP_CHANGE)
                    {
                        // try again with the new parameters.
                        on_state = ON_BS_FIND_ROUTE;
                        bs_msg.bs_on_state = ON_BS_FIND_ROUTE;
                    }
                    else if(msg_status != ON_MSG_SUCCESS)
                    {
                        terminate_bs_msg(&bs_msg, NULL, msg_status, &ack_nack);
                    }
                }
                else
                #endif
                {
                    on_state = ON_LISTEN_FOR_DATA;
                }
                one_net(txn); // we may have another message to send.
                              // send it before going back to the master
                              // or client code.
            }
            
            break;
        } // case ON_WAIT_FOR_SINGLE_DATA_RESP
        
        #ifdef BLOCK_MESSAGES_ENABLED
        case ON_BS_TERMINATE_COMPLETE:
           terminate_bs_complete(&bs_msg);
        #endif
        
        #ifdef COMPILE_WO_WARNINGS
        // add default case that does nothing for clean compile
        default:
            break;
        #endif
    }
} // one_net //


/*!
    \brief Finishes reception of a single data pkt

    \param[in/out] txn The single transaction
    \param[out] this_txn If set to NULL in this function, the  message is cancelled / terminated
    \param[in] pkt The packet structure being responded to
    \param[out] raw_payload_bytes The raw payload of the message that is being responded to
    \param[out] ack_nack The ACK or NACK attached to the response, if any

    \return ON_BS_MSG_SETUP_CHANGE Applicable only for block / stream setup messages.  Terminates the current block / stream message
            ON_MSG_IGNORE If the packet should be be ignored
            All other return values result in continuing of the message in the default fashion.
*/
static on_message_status_t rx_single_resp_pkt(on_txn_t** const txn,
  on_txn_t** const this_txn, on_pkt_t* const pkt,
  UInt8* const raw_payload_bytes, on_ack_nack_t* const ack_nack)
{
    on_message_status_t msg_status;
    BOOL verify_needed;
    BOOL message_ignore = TRUE;
    const tick_t VERIFY_TIMEOUT = MS_TO_TICK(2000); // 2 seconds
    tick_t time_now = get_tick_count();
    
    if(on_parse_response_pkt(pkt->raw_pid, raw_payload_bytes, ack_nack) !=
      ONS_SUCCESS)
    {
        return ON_MSG_IGNORE; // undecipherable
    }
    
    // Replay attacks are FAR more worriesome on the receiving side.
    // We are the sender, but both sides should always check

    // first check  the message ID against the sending device info.  We may
    // or may not need to verify anything.
    verify_needed = TRUE;

    if((*txn)->device->verify_time != 0 && time_now >=
      (*txn)->device->verify_time && time_now - (*txn)->device->verify_time
      < VERIFY_TIMEOUT)
    {
        verify_needed = FALSE;
    }
    else
    {
        // regardless of whether we may have verified in the past, we'll
        // want to re-sync / re-verify after getting this problem message.
        (*txn)->device->verify_time = 0;
    }

    #ifdef ONE_NET_CLIENT
    features_override = FALSE; // if this flag's been set, clear it.  If we
                              // need to reset it, we'll do that below.
    
    // first check to make sure we have the other device's features and they
    // have ours
    if(ack_nack->nack_reason == ON_NACK_RSN_NEED_FEATURES)
    {
        // they need our features and have given us theirs.
        (*txn)->device->features = ack_nack->payload->features;
        features_override = TRUE; // we have theirs, but they need ours.
        return ON_MSG_CONTINUE;
    }
    else if(!features_known((*txn)->device->features))
    {
        // we need their features.  They may or may not have given them
        // to us.
        if(ack_nack->nack_reason == ON_NACK_RSN_FEATURES)
        {
            // they gave it to us.
            (*txn)->device->features = ack_nack->payload->features;
            #ifdef ONE_NET_MULTI_HOP
            (*txn)->device->max_hops =
              features_max_hops((*txn)->device->features);
            #endif
            // OK, we both have each otehrs' features.  Next time we'll
            // send the real message.
            return ON_MSG_CONTINUE;
        }
        else
        {
            // can't do anything without the features and they didn't give
            // it to us, so ignore.  We need their features, so set the
            // features_override flag again.
            features_override = TRUE;
            return ON_MSG_CONTINUE;
        }
    }
    #endif

    // March 2, 2012 - changing rollover message ID protocol.

    // Now check the message IDs.  We should be getting a response to the one
    // we sent.  If we don't get a response to the one we sent, if we
    // have gotten back a NACK saying that the receiver did not like OUR
    // message ID and giving us one to use, we'll use it.  Otherwise, we'll
    // just call it a bad packet and otherwise ignore it.
    
    // TODO -- The protocol for avoiding replay attacks with message ids was
    // originally to always INCREASE the message id.  If we were given a lower
    // message id than we expected, we should not accept it.  This needs to be
    // revisited.  
    
    // The message_ignore flag is set to true.  If the message ID matches
    // what we gave the other device, we may set it false.  If they didn't
    // like our message ID and gave us one to use back, we'll use it if it's
    // not "less than" the one we gave them.  IF our current message ID is
    // within 2 of the maximum, we'll accept a 0 or 1 as well.
    if(pkt->msg_id == (*txn)->device->msg_id)
    {
        if(ack_nack->nack_reason == ON_NACK_RSN_INVALID_MSG_ID)
        {
            if(ack_nack->payload->nack_value >= (*txn)->device->msg_id ||
              ((*txn)->device->msg_id + 2 >= ON_MAX_MSG_ID &&
              ack_nack->payload->nack_value < 2))
            {
                // they gave us back a valid message id to use instead of the
                // one we were using.
                (*txn)->device->msg_id = ack_nack->payload->nack_value;
                pkt->msg_id = ack_nack->payload->nack_value;
                (*txn)->device->verify_time = 0;
                return ON_MSG_CONTINUE;
            }
        }
        else
        {
            message_ignore = FALSE;
        }
    }

    if(message_ignore)
    {
        return ON_MSG_IGNORE; // replay attack?  Out of sync?  Who knows?
                              // Ignore it.
    }
    
    if(verify_needed)
    {
        // If they liked the message 
        (*txn)->device->verify_time = time_now;
    }
    

	// now we'll give the application code a chance to do whatever it wants to do
	// with the ACK/NACK, including handle it itself.  If it wants to handle everything
	// itself and cancel this transaction, it should return false.  It may decide
    // it wants to abort, in which case it should return ON_MSG_ABORT.  If
    // there was a problem with the message ID, we're simply calling the
    // application code to notify it.  It should NOT change anything, but it may
    // want to be aware of failures for its own needs.
    msg_status = (*pkt_hdlr.single_ack_nack_hdlr)(&single_txn,
      &data_pkt_ptrs, single_msg_ptr->payload,
      &(single_msg_ptr->msg_type), ack_nack);
    
    if(msg_status == ON_BS_MSG_SETUP_CHANGE)
    {
        return ON_BS_MSG_SETUP_CHANGE;
    }
    
    #if defined(ONE_NET_CLIENT) && defined(DEVICE_SLEEPS)
    if(!device_is_master && (pkt->raw_pid & ONE_NET_RAW_PID_STAY_AWAKE_MASK))
    {
        // TODO -- should 3 seconds be hard-coded?
        // we received a stay-awake, so set the stay-awake timer
        // for 3 seconds
        ont_set_timer(ONT_STAY_AWAKE_TIMER,
          MS_TO_TICK(DEVICE_SLEEP_STAY_AWAKE_TIME));
    }
    #endif

    // the application code may or may not have changed this from an ACK to a
    // NACK or vice versa, changed the number of retries, added a delay,
    // changed the response timeout, or whateer else.  We don't particularly
    // care here.
    if(ack_nack->nack_reason != ON_NACK_RSN_NO_ERROR)
    {
        ((*txn)->retry)++;
    }
    else
    {
        *this_txn = 0;
        return ON_MSG_SUCCESS;
    }

    return ON_MSG_CONTINUE;
}


/*!
    \brief Finishes reception of a single data pkt

    \param[in/out] txn The single transaction
    \param[in] sing_pkt_ptr A pointer to elements of the parsed packet
    \param[in] raw_payload The decoded, decrypted payload
    \param[out] ack_nack The ACK or NACK reason, handle, and payload

    \return ON_MSG_CONTINUE If the packet should be processed further
            ON_MSG_RESPONSE If the packet does not need further processing
                            and should be responded to.
            ON_MSG_IGNORE If the packet should be be ignored
            See on_message_status_t & single_data_hdlr for more options.
*/
on_message_status_t rx_single_data(on_txn_t** txn, on_pkt_t* sing_pkt_ptr,
  UInt8* raw_payload, on_ack_nack_t* ack_nack)
{
    UInt8 msg_type;
    BOOL src_features_known, use_current_key;
    
    if(!txn || !(*txn) || !raw_payload || !ack_nack)
    {
        return ONS_BAD_PARAM;
    }

    ack_nack->payload = (ack_nack_payload_t*) &raw_payload[ON_PLD_DATA_IDX];
    
    // March 2, 2012 -- Removing all nonce code!
    
    // we'll do a little prep work here before actually sending the message
    // to the single data handler.  Anything we can handle here where the
    // application code does not need to be alerted will be handled here.
    // We'll set some flags and nack reasons that will signal follow-up
    // functions that we've already processed the message.
    
    // Possibilities include...
    //
    // 1) Making sure that both ends have each others' features.
    // 2) Verifying message ids
    // 3) Figuring out what messages can definitely be aborted.
    
    
    // Message ids and nonces serve three purposes...
    // 1) Protecting against replay attacks.
    // 2) Making sure devices are in sync.
    // 3) Making sure that single messages which have been completed are
    //    not acted upon again.  We sent and earlier ACK that must have been
    //    lost.  We will ACK again.
    //
    //    Note : The protocol is not complete on this.  Simply ACKing won't
    //           do the job because very often the message will require the
    //           current status and only the application code can provide
    //           that.  Some messages should never be acted upon twice
    //           (i.e. changing a key fragment) and some don't matter.  For
    //           now if we've ACK'd before, we will skip the verification.
    //           Acting upon a message twice, however, opens up the
    //           possibility of replay attacks, so this needs to be worked
    //           out much more thoroughly in the protocol.  Ideally we would
    //           like to ACKNOWLEDGE a valid message that has already been
    //           ACK'd, but not ACT UPON it, particularly if it is a command
    //           to turn a relay on or off or a command to set some other
    //           setting to a certain setting.
    //
    //           For now we will risk the replay attack in order to reduce
    //           lost messages, but we will require the nonce to be correct.
    //           Upon verification, we will CHANGE the nonce ONCE, then not
    //           again.  Verification will only occur if the message matches
    //           the EXPECTED nonce, not the expected or the last nonce, again
    //           to help prevent replay attacks.
    //                   
    
    

    (*txn)->device = (*get_sender_info) ((const on_encoded_did_t* const)
       &((*txn)->pkt[ON_ENCODED_SRC_DID_IDX]));

    if((*txn)->device == NULL)
    {
        // how the heck did this happen?
        *txn = 0;
        return ON_MSG_INTERNAL_ERR;
    }
    
    #ifdef ONE_NET_MULTI_HOP
    {
        (*txn)->device->hops = sing_pkt_ptr->hops;
        (*txn)->hops = (*txn)->device->hops;
        
        // assume it will take as many hops to get back as it took to get
        // there.  Application code will change if it likes.
        (*txn)->max_hops = (*txn)->device->hops;
    }
    #endif    
    

    src_features_known = features_known((*txn)->device->features);
    msg_type = get_payload_msg_type(raw_payload);
    use_current_key = (one_net_memcmp(on_base_param->current_key, (*txn)->key,
      ONE_NET_XTEA_KEY_LEN) == 0);
    
    // If the source features are not known and it is NOT an admin message
    // telling us what those features are, we'll NACK the message.  We'll
    // also NACK the message if the message type is ON_FEATURE_MSG.  This
    // generally means that the device wants to send something to us, but
    // doesn't have features.  It is giving us its features and wants ours
    // in return.  It also wants to be NACK'd, not ACK'd for easier handling
    // the other end.  If it wanted an ACK, it would have sent a features
    // request admin message.
    
    
    // first we'll check the features and set the handle and payload as
    // such.  If this level passes, we'll reset them to something else.
    ack_nack->handle = ON_NACK_FEATURES;
    

    if(msg_type == ON_FEATURE_MSG)
    {
        one_net_memmove(&((*txn)->device->features),
          &raw_payload[ON_PLD_DATA_IDX], sizeof(on_features_t));
        ack_nack->payload->features = THIS_DEVICE_FEATURES;
        // we'll NACK it and give the nack reason as ON_NACK_RSN_FEATURES,
        // but the NACK handle will be ON_NACK_FEATURES.  We are making the
        // nack reason ON_NACK_RSN_FEATURES rather than
        // ON_NACK_RSN_NEED_FEATURES because if we made the nack reason that,
        // the sending device would think that the problem was that WE didn't
        // have ITS features when in fact the opposite is true.
        ack_nack->nack_reason = ON_NACK_RSN_FEATURES;
        (*txn)->device->verify_time = 0;
        return ON_MSG_CONTINUE;
    }
    
    if(!src_features_known)
    {
        // this may be an admin message where this device is giving us its
        // features, so check and copy them if it is.  NACK regardless just in
        // case that device needs OUT features.
        // TODO -- does this make sense to NACK?  Seems a bit strange and
        // possibly redundant given that we have a message type called
        // ON_FEATURE_MSG.
        if(msg_type == ON_ADMIN_MSG && raw_payload[ON_PLD_ADMIN_TYPE_IDX] ==
          ON_PLD_ADMIN_TYPE_IDX)
        {
            one_net_memmove(&((*txn)->device->features),
              &raw_payload[ON_PLD_ADMIN_DATA_IDX], sizeof(on_features_t));
        }
        
        // this time WE need the other device's features, so we'll set
        // the nack reason to ON_NACK_FEATURES.
        ack_nack->nack_reason = ON_NACK_RSN_NEED_FEATURES;
        ack_nack->payload->features = THIS_DEVICE_FEATURES;
        (*txn)->device->verify_time = 0;
        return ON_MSG_CONTINUE;
    }
    
    
    // we've gotten this far, so we have each other's features.  Now we
    // need to check the message id.
    
    // If the message ID is the same as the one we have on file, we'll
    // assume this is a repeat packet.  If we have not ACK'd it yet,
    // everything is good.  If we HAVE ACK'd it, we'll have a verify time.
    // If that verify time is close to the current time, we'll assume that
    // the other side simply missed our ACK and count it as good.  If the
    // verify time is NOT close to the current time, we'll assume this is
    // possibly a replay attack or that something is out of sync.  We will
    // NACK it and send back the NACK along with a message id that we WILL
    // accept.
    
    // If the message is LESS THAN the message we have on file, there might be
    // a problem.  If we care about replay attacks, we'll NACK it and tell the
    // sender to use a message id greater than what we have on file.
    
    // If the message ID is GREATER than the one we have on file, we will
    // assume that this is a new message and accept it.
    
    // Accept the message ID if the old key is used.  We're in the middle of a
    // key change.  The message IDs will be re-synced once we're both using the
    // current key.  If we're not using it, we won't bother checking the message
    // ID.  We'll NACK with a bad key nack reason instead.
    if(use_current_key)
    {
        tick_t time_now = get_tick_count();
        const tick_t VERIFY_TIMEOUT = MS_TO_TICK(2000); // 2 seconds
        
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_MSG_ID;

        if(!one_net_reject_bad_msg_id((*txn)->device) ||
          sing_pkt_ptr->msg_id > (*txn)->device->msg_id)
        {
            // We either do not care about invalid message ids or we received a
            // valid message id for a new message we haven't seen before.
            (*txn)->device->msg_id = sing_pkt_ptr->msg_id;
            (*txn)->device->verify_time = 0;
            ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
        }
        else if(sing_pkt_ptr->msg_id == (*txn)->device->msg_id)
        {
            if((*txn)->device->verify_time == 0)
            {
                ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
            }
            else if(time_now >= (*txn)->device->verify_time &&
              time_now - (*txn)->device->verify_time < VERIFY_TIMEOUT)
            {
                // verified and ACK'd recently.  The ACK may have been missed.
                ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
            }
        }

        if(ack_nack->nack_reason == ON_NACK_RSN_INVALID_MSG_ID)
        {
            ack_nack->handle = ON_NACK_VALUE;
            // this is a message id we will accept. 
            ack_nack->payload->nack_value = 1 + (*txn)->device->msg_id;
            (*txn)->device->verify_time = 0; 
            return ON_MSG_CONTINUE;        
        }
    }
    
    ack_nack->handle = ON_ACK;
    
    // make sure the key used is the current key.  If it isn't, we may send back
    // a NACK with a nack reason of ON_NACK_RSN_BAD_KEY.  This will cause the
    // other device to check in with the master and get the new key.  If we ARE
    // the master and the device IS checking in, then we'll let the message go a
    // little farther because a main part of the check-in is to make sure the
    // client has the right key.
    if(!use_current_key)
    {
        ack_nack->nack_reason = ON_NACK_RSN_BAD_KEY;
        // fill in the new key fragment in the NACK reason.
        ack_nack->handle = ON_NACK_KEY;
        one_net_memmove(ack_nack->payload->key_frag,
          &(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
          ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
        #ifdef DEVICE_SLEEPS
        // we'll stay awake in case there is a follow-up.
        ont_set_timer(ONT_STAY_AWAKE_TIMER,
          MS_TO_TICK(DEVICE_SLEEP_STAY_AWAKE_TIME));
        #endif
        (*txn)->device->verify_time = 0; 
    }
    
    return ON_MSG_CONTINUE;
} // rx_single_data //


#ifdef BLOCK_MESSAGES_ENABLED
static on_message_status_t rx_block_resp_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, on_pkt_t* pkt, UInt8* raw_payload_bytes,
  on_ack_nack_t* ack_nack)
{
    on_message_status_t status;
    on_encoded_did_t* terminating_did = &bs_msg->dst->did;

    if(on_parse_response_pkt(pkt->raw_pid, raw_payload_bytes, ack_nack) !=
      ONS_SUCCESS)
    {
        return ON_MSG_IGNORE; // undecipherable
    }
    
    if(!nack_reason_is_fatal(ack_nack->nack_reason))
    {
        terminating_did = NULL;
    }
    
    // send it up to the application code.
    status = (*pkt_hdlr.block_ack_nack_hdlr)(txn, bs_msg, pkt,
      raw_payload_bytes, ack_nack);
      
    if(terminating_did)
    {
        status = ON_MSG_ABORT;
    }
      
    switch(status)
    {
        case ON_MSG_ABORT: case ON_MSG_TIMEOUT: case ON_MSG_TERMINATE:
          terminate_bs_msg(bs_msg, (const on_encoded_did_t*) terminating_did,
          status, ack_nack);
          return ON_MSG_TERMINATE;
        case ON_MSG_IGNORE:
          return ON_MSG_IGNORE;
        #ifdef COMPILE_WO_WARNINGS
        // add default case that does nothing for clean compile
        default:
            break;
        #endif
    }
    
    
    // we got a response, so reset the data rate change timer to the
    // timeout
    #ifdef DATA_RATE_CHANNEL
    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, MS_TO_TICK(bs_msg->timeout));
    #endif
    ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(bs_msg->timeout));
    switch(ack_nack->handle)
    {
        case ON_ACK_SLOW_DOWN_TIME_MS:
          bs_msg->frag_dly += ack_nack->payload->nack_time_ms;
          break;
        case ON_ACK_SPEED_UP_TIME_MS:
          bs_msg->frag_dly += ack_nack->payload->nack_time_ms;
          break;
        case ON_ACK_PAUSE_TIME_MS:
          pause_bs_msg(bs_msg, ack_nack->payload->nack_time_ms);
          break;
        case ON_ACK_BLK_PKTS_RCVD:
          one_net_memmove(bs_msg->bs.block.sent, ack_nack->payload->ack_payload,
            sizeof(bs_msg->bs.block.sent));
          return ON_MSG_CONTINUE;
        default:
        {
            switch(ack_nack->nack_reason)
            {
                case ON_NACK_RSN_INVALID_CHUNK_DELAY:
                    bs_msg->bs.block.chunk_pause =
                      ack_nack->payload->nack_time_ms;
                    break;
                case ON_NACK_RSN_INVALID_BYTE_INDEX:
                    bs_msg->bs.block.byte_idx = ack_nack->payload->nack_value;
                    
                    // TODO -- why is one of these unsigned?
                    #ifndef COMPILE_WO_WARNINGS
                    if(bs_msg->bs.block.byte_idx * ON_BS_DATA_PLD_SIZE >=
                      bs_msg->bs.block.transfer_size)
                    #else
                    if((UInt32) (bs_msg->bs.block.byte_idx * ON_BS_DATA_PLD_SIZE) >=
                      bs_msg->bs.block.transfer_size)
                    #endif
                    {
                        terminate_bs_msg(bs_msg, NULL, ON_MSG_SUCCESS,
                          NULL);
                        return ON_MSG_IGNORE;
                    }
                    
                    one_net_memset(bs_msg->bs.block.sent, 0,
                      sizeof(bs_msg->bs.block.sent));
                    pause_bs_msg(bs_msg, bs_msg->bs.block.chunk_pause);
                    break;
                case ON_NACK_RSN_INVALID_FRAG_DELAY:
                    bs_msg->frag_dly = ack_nack->payload->nack_time_ms;
                    break;
                case ON_NACK_RSN_INVALID_PRIORITY:
                    set_bs_priority(&bs_msg->flags,
                      ack_nack->payload->nack_value);
                    break;
                #ifdef COMPILE_WO_WARNINGS
                // add default case that does nothing for clean compile
                default:
                    break;
                #endif
            }
        }
    }

    return ON_MSG_IGNORE;
}
#endif


/*!
    \brief Receives packets.

    This function receives all packets.

    \param[in] The current transaction being carried out.
    \param[out] this_txn The packet received
    \param[out] this_pkt_ptrs Filled in pointers to the packet received
    \param[out] raw_payload_bytes The decoded / decrypted bytes of this packet

    \return ONS_PKT_RCVD if a valid packet was received
            For more return values one_net_status_codes.h
*/
#if defined(BLOCK_MESSAGES_ENABLED) || defined(ONE_NET_MH_CLIENT_REPEATER)
one_net_status_t on_rx_packet(const on_txn_t* const txn, on_txn_t** this_txn,
  on_pkt_t** this_pkt_ptrs, UInt8* raw_payload_bytes)
#else
one_net_status_t on_rx_packet(on_txn_t** this_txn, on_pkt_t** this_pkt_ptrs,
  UInt8* raw_payload_bytes)
#endif
{
    one_net_status_t status;
    one_net_xtea_key_t* key = NULL;
    UInt16 raw_pid;
    BOOL dst_is_broadcast, dst_is_me, src_match;
    #ifdef ONE_NET_MULTI_HOP
    BOOL packet_is_mh;
    #endif
    #ifdef ONE_NET_MH_CLIENT_REPEATER
    BOOL repeat_this_packet = FALSE;
    #ifdef ROUTE
    BOOL repeat_route_packet = FALSE;
    #endif
    #endif
    on_data_t type = ON_NO_TXN;
    UInt8* pkt_bytes;
    BOOL src_is_master;
    
    #ifdef BLOCK_MESSAGES_ENABLED
    BOOL src_is_bs_endpoint;
    #ifdef ONE_NET_MH_CLIENT_REPEATER
    BOOL dst_is_master, dst_is_bs_endpoint;
    BOOL high_priority_bs = (bs_msg.transfer_in_progress && get_bs_priority(
      bs_msg.flags) > ONE_NET_LOW_PRIORITY);
    #endif
    #endif


    if(one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME) != ONS_SUCCESS)
    {
        return ONS_READ_ERR;
    }
    
    pkt_bytes = (*this_txn)->pkt;
    
    if(one_net_read(&pkt_bytes[ONE_NET_PREAMBLE_HEADER_LEN],
      ON_ENCODED_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN) !=
      ON_ENCODED_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN)
    {
        return ONS_READ_ERR;
    }
    
    // check some addresses
    #ifndef ONE_NET_CLIENT
    if(!is_my_nid((on_encoded_nid_t*) (&pkt_bytes[ON_ENCODED_NID_IDX])))
    {
        return ONS_NID_FAILED; // not our network
    }
    #else
    if(*this_txn != &invite_txn && !is_my_nid((const on_encoded_nid_t*)
      (&pkt_bytes[ON_ENCODED_NID_IDX])))
    {
        return ONS_NID_FAILED; // not our network
    }
    #endif    
    
    #ifdef ONE_NET_MULTI_HOP
    // first check the source.  If it was us originally, then we probably
    // got back our own repeated packet.
    if(is_my_did((const on_encoded_did_t* const)
      &pkt_bytes[ON_ENCODED_SRC_DID_IDX]))
    {
        return ONS_DID_FAILED; // We SENT this packet, so no sense RECEIVING
                               // it.
    }
    #endif

    #ifdef RANGE_TESTING
    if(!device_in_range((on_encoded_did_t*)
      &(pkt_bytes[ON_ENCODED_RPTR_DID_IDX])))
    {
        // we'll pretend that this device was out of range and we couldn't
        // read it.
        return ONS_READ_ERR;
    }
    #endif
    
    if(!get_raw_pid(&pkt_bytes[ON_ENCODED_PID_IDX], &raw_pid))
    {
        return ONS_BAD_PKT_TYPE;
    }
    
    #ifdef PID_BLOCK
    // only care about the least significant 8 bits
    if(pid_is_blocked((UInt8) raw_pid))
    {
        // We are filtering this PID out, so we'll pretend we did not hear it.
        return ONS_READ_ERR;
    }
    #endif

    dst_is_broadcast = is_broadcast_did((const on_encoded_did_t*)
      (&pkt_bytes[ON_ENCODED_DST_DID_IDX]));
    dst_is_me = is_my_did((const on_encoded_did_t* const)
      (&pkt_bytes[ON_ENCODED_DST_DID_IDX]));
    src_match = is_broadcast_did((const on_encoded_did_t* const)
      &expected_src_did) || on_encoded_did_equal((const on_encoded_did_t* const)
      &expected_src_did,(const on_encoded_did_t* const)
      &pkt_bytes[ON_ENCODED_SRC_DID_IDX]);
      
    // TODO -- should master messages ever be discarded?
    src_is_master = is_master_did(
      (const on_encoded_did_t*) &pkt_bytes[ON_ENCODED_SRC_DID_IDX]);
      
    #ifdef BLOCK_MESSAGES_ENABLED
    {
        // note that these values might be garbage if we are not in the
        // middle of a block / stream transfer.  It's OK if they are because
        // the values will only be used if we are in a block / stream transfer.
        on_encoded_did_t* bs_src_did = get_encoded_did_from_sending_device(
          bs_msg.src);
        on_encoded_did_t* bs_dst_did = get_encoded_did_from_sending_device(
          bs_msg.dst);
        
        src_is_bs_endpoint = on_encoded_did_equal(
          (const on_encoded_did_t* const)bs_src_did,
          (const on_encoded_did_t* const) &pkt_bytes[ON_ENCODED_SRC_DID_IDX]) ||
          on_encoded_did_equal((const on_encoded_did_t* const) bs_dst_did,
          (const on_encoded_did_t*const) &pkt_bytes[ON_ENCODED_SRC_DID_IDX]);
        #ifdef ONE_NET_MH_CLIENT_REPEATER
        dst_is_bs_endpoint = on_encoded_did_equal(
          (const on_encoded_did_t* const)bs_src_did,
          (const on_encoded_did_t* const) &pkt_bytes[ON_ENCODED_DST_DID_IDX]) ||
          on_encoded_did_equal((const on_encoded_did_t* const) bs_dst_did,
          (const on_encoded_did_t* const) &pkt_bytes[ON_ENCODED_DST_DID_IDX]);
        dst_is_master = is_master_did((const on_encoded_did_t* const)
          (const on_encoded_did_t* const) &pkt_bytes[ON_ENCODED_DST_DID_IDX]);
        #endif
    }
    #endif
    
    #ifdef ONE_NET_MULTI_HOP
    packet_is_mh = packet_is_multihop(raw_pid);
    #endif
    
    #ifndef ONE_NET_MH_CLIENT_REPEATER
    if(!src_match || (!dst_is_me && !dst_is_broadcast))
    {
        return ONS_BAD_ADDR;
    }
    #else
    if(!src_match || (!dst_is_me && !dst_is_broadcast) || ((raw_pid & 0x3F) ==
      ONE_NET_RAW_MASTER_INVITE_NEW_CLIENT && client_joined_network))
    {
        // not to us, but maybe we'll repeat it if we're not the master,
        // not in the middle of our own transaction, it's a multi-hop
        // packet, and it wasn't to us.
        if(dst_is_me || device_is_master || txn || !packet_is_mh)
        {
            return ONS_BAD_PARAM;
        }
        
        #ifdef BLOCK_MESSAGES_ENABLED
        // if we are a repeater in the middle of a high-priority block / stream
        // message and neither the source or the destination is an endpoint or
        // the master, we won't repeat the packet.
        if(high_priority_bs)
        {
            if(!bs_msg.src || !bs_msg.dst)
            {
                return ONS_BUSY; // we don't repeat packets if we are an
                                 // endpoint
            }
            
            if(!src_is_master && !src_is_bs_endpoint && !dst_is_master &&
              !dst_is_bs_endpoint)
            {
                return ONS_BUSY; // only repeat packets that might be part of
                                 // the block / stream transaction.
            }
        }
        #endif
        
        // we'll repeat it if there are any hops left.
        repeat_this_packet = TRUE;
        #ifdef ROUTE
        // if it's a NACK, we'll just forward it as we do all other packets.
        // If it's the original message or an ACK, we'll need to decrypt it,
        // add ourself, then encrypt it and send it along, so we'll set a
        // repeat_route_packet flag to true for these PIDs.
        repeat_route_packet = packet_is_route(raw_pid) &&
          !packet_is_nack(raw_pid);
        #endif
    }
    #endif
    
    if(packet_is_data(raw_pid))
    {
        if(packet_is_single(raw_pid))
        {
            type = ON_SINGLE;
        }
        #ifdef BLOCK_MESSAGES_ENABLED
        else if(packet_is_block(raw_pid))
        {
            type = ON_BLOCK;
        }
        #endif
        #ifdef STREAM_MESSAGES_ENABLED
        else if(packet_is_stream(raw_pid))
        {
            type = ON_STREAM;
        }
        #endif
    }
    else if(packet_is_ack(raw_pid) || packet_is_nack(raw_pid))
    {
        type = ON_RESPONSE;
    }
    #ifdef ONE_NET_CLIENT
    else if(packet_is_invite(raw_pid))
    {
        type = ON_INVITE;
    }
    #endif
    else
    {
        #ifdef ONE_NET_MH_CLIENT_REPEATER
        if(!repeat_this_packet)
        #endif
        {
            return ONS_BAD_PKT_TYPE;
        }
    }

    #ifdef ONE_NET_MH_CLIENT_REPEATER
    if(repeat_this_packet)
    {
        #ifdef  BLOCK_MESSAGES_ENABLED
        if(bs_msg.transfer_in_progress)
        {
            ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(bs_msg.timeout));
            #ifdef DATA_RATE_CHANNEL
            ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER,
              MS_TO_TICK(bs_msg.timeout));
            #endif
        }
        #endif
        one_net_memmove(&(mh_txn.pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
          &((*this_txn)->pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
          ON_ENCODED_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN);
        *this_txn = &mh_txn;
    }
    else
    #endif
    {
        // not repeating.  Let's verify the packets we are receiving
        // are valid for our current state.
        switch(type)
        {
            case ON_INVITE:
              #ifndef ONE_NET_CLIENT
              return ONS_UNHANDLED_PKT;;
              #else
              if(*this_txn != &invite_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              #endif
              break;
            case ON_SINGLE:
              #ifdef BLOCK_MESSAGES_ENABLED
              if(*this_txn != &single_txn && *this_txn != &bs_txn)
              #else
              if(*this_txn != &single_txn)
              #endif
              {
                  return ONS_UNHANDLED_PKT;
              }
              #ifdef BLOCK_MESSAGES_ENABLED
              else
              {
                  one_net_memmove(&(single_txn.pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
                    &((*this_txn)->pkt[ONE_NET_PREAMBLE_HEADER_LEN]),
                    ON_ENCODED_PLD_IDX - ONE_NET_PREAMBLE_HEADER_LEN);                  
                  *this_txn = &single_txn;
              }
              #endif
              break;
            #ifdef BLOCK_MESSAGES_ENABLED
            case ON_BLOCK:
            #ifdef STREAM_MESSAGES_ENABLED
            case ON_STREAM:
            #endif
              if(*this_txn != &bs_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              break;
            #endif
            case ON_RESPONSE:
              if(*this_txn != &response_txn)
              {
                  return ONS_UNHANDLED_PKT;
              }
              break;
            default:
              return ONS_UNHANDLED_PKT;
        }
    }

    if(!setup_pkt_ptr(raw_pid, (*this_txn)->pkt, 0, *this_pkt_ptrs))
    {
        return ONS_INTERNAL_ERR;
    }

    if(one_net_read(&((*this_txn)->pkt[ON_ENCODED_PLD_IDX]), (*this_pkt_ptrs)->payload_len)
      != (*this_pkt_ptrs)->payload_len)
    {
        return ONS_READ_ERR;
    }
    
    #ifndef ONE_NET_MH_CLIENT_REPEATER
    if(!verify_msg_crc(*this_pkt_ptrs))
    #else
    // don't bother verifying if we are just going to repeat.
    if(!repeat_this_packet && !verify_msg_crc(*this_pkt_ptrs))
    #endif
    {
        return ONS_CRC_FAIL;
    }
    
    #ifdef ONE_NET_MULTI_HOP
    // overwritten below if multi-hop
    (*this_pkt_ptrs)->hops = 0;
    (*this_pkt_ptrs)->max_hops = 0;
    if(packet_is_mh)
    {
        UInt8 raw_hops_field;

        if(one_net_read(&((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_PLD_IDX]) +
          (*this_pkt_ptrs)->payload_len, ON_ENCODED_HOPS_SIZE) !=
          ON_ENCODED_HOPS_SIZE)
        {
            return ONS_READ_ERR;
        }

        if(on_decode(&raw_hops_field,
          &((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_PLD_IDX]) +
          (*this_pkt_ptrs)->payload_len, ON_ENCODED_HOPS_SIZE) != ONS_SUCCESS)
        {
            return ONS_BAD_ENCODING;
        }
        
        // TODO -- use defined masks and shifts
        raw_hops_field >>= 2;
        (*this_pkt_ptrs)->max_hops = raw_hops_field & 0x07;
        (*this_pkt_ptrs)->hops = (raw_hops_field >> 3);
        
        #ifdef ONE_NET_MH_CLIENT_REPEATER
        if(repeat_this_packet)
        {
            if((*this_pkt_ptrs)->hops >= (*this_pkt_ptrs)->max_hops)
            {
                // too many hops.  Don't repeat.
                return ONS_UNHANDLED_PKT;
            }
            
            // we have everything.  Increment the hops, add ourself as the
            // repeater, pause a very short time, and send the message.
            ((*this_pkt_ptrs)->hops)++;
            
            raw_hops_field = ((((*this_pkt_ptrs)->hops) << 3) +
              (*this_pkt_ptrs)->max_hops) << 2;
            on_encode(&((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_PLD_IDX]) +
              (*this_pkt_ptrs)->payload_len, &raw_hops_field,
              ON_ENCODED_HOPS_SIZE);
            one_net_memmove(
              &((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_RPTR_DID_IDX]),
              &(on_base_param->sid[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
            ont_set_timer(mh_txn.next_txn_timer, MS_TO_TICK(
              ONE_NET_MH_LATENCY));
            
            #ifdef ROUTE
            if(!repeat_route_packet)
            {
                // more needs to be done for route packets.  This is NOT a
                // route packet or it is a route packet NACK, so we're done.
                return ONS_PKT_RCVD; // TODO -- what's with this if-statement.  Seems pointless.
            }
            #else  
            return ONS_PKT_RCVD;
            #endif
        }
        #endif
    }
    #endif
    
    key = &(on_base_param->current_key);
    
    #ifdef ONE_NET_CLIENT
    if(!device_is_master && *this_txn == &invite_txn)
    {
        key = (one_net_xtea_key_t*) one_net_client_get_invite_key();
    }
    #endif

    decrypt_using_current_key = TRUE;
    while(1)
    {        
        if((status = on_decode(raw_payload_bytes,
          &((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_PLD_IDX]),
          (*this_pkt_ptrs)->payload_len)) != ONS_SUCCESS)
        {
            return status;
        }
    
        #ifdef STREAM_MESSAGES_ENABLED
        if((status = on_decrypt(type == ON_STREAM, raw_payload_bytes,
          (const one_net_xtea_key_t * const) key, get_raw_payload_len(raw_pid)))
          != ONS_SUCCESS)
        #else
        if((status = on_decrypt(raw_payload_bytes,
          (const one_net_xtea_key_t * const) key, get_raw_payload_len(raw_pid)))
          != ONS_SUCCESS)
        #endif
        {
            return status;
        }   

        // check the payload CRC.
        if(verify_payload_crc(raw_pid, raw_payload_bytes))
        {
            break;
        }
        
        // we may be in the middle of a key change.  We may try the other
        // key
        #ifdef ONE_NET_CLIENT
        if(!device_is_master && *this_txn == &invite_txn)
        {
            // only have one invite key and it didn't work.
            return ONS_CRC_FAIL;;
        }
        #endif
        
        if(!decrypt_using_current_key)
        {
            return ONS_CRC_FAIL;; // Tried two keys.  Neither key worked.
        }
        
        decrypt_using_current_key = FALSE;
        
        // try the old key
        key = (one_net_xtea_key_t*) &(on_base_param->old_key);
    }

    // set the key
    (*this_txn)->key = key;
    
    // message id is irrelevant for invite packets, but fill it in regardless.
    (*this_pkt_ptrs)->msg_id = get_payload_msg_id(raw_payload_bytes);
    
    #if defined(ONE_NET_MH_CLIENT_REPEATER) && defined(ROUTE)
    if(repeat_route_packet)
    {
        on_raw_did_t my_raw_did;
        on_raw_did_t raw_src_did;
        SInt8 src_idx;
        UInt8 msg_crc;
        
        // We need to do two things.  One, we need to check whether we are
        // already in this route.  If we are, we will not forward since that
        // would make a circular route.  Two, we need to append ourself to the
        // end of this route.  We'll do am quick check to make sure there is
        // room to do that without overflowing the message.  Assuming we're not
        // on the route and there's room in the message, we'll add ourselves,
        // then re-encrypt, re-encode, and re-calculate the CRC values and send
        // the message on its way.
        if(on_decode(my_raw_did, &(on_base_param->sid[ON_ENCODED_NID_LEN]),
          ON_ENCODED_DID_LEN) != ONS_SUCCESS)
        {
            return ONS_INTERNAL_ERR;
        }
        if(on_decode(raw_src_did,
          &((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_SRC_DID_IDX]),
          ON_ENCODED_DID_LEN) != ONS_SUCCESS)
        {
            return ONS_BAD_ENCODING;
        }
        
        src_idx = find_raw_did_in_route(&raw_payload_bytes[ON_PLD_DATA_IDX],
          (const on_raw_did_t* const) raw_src_did, 0);
          
        // see if we are already in this route
        if(find_raw_did_in_route(&raw_payload_bytes[ON_PLD_DATA_IDX],
          (const on_raw_did_t* const) my_raw_did, src_idx + 1) != -1)
        {
            // already here.  Don't add yourself again.
            return ONS_INCORRECT_ADDR;
        }
        
        // If we have room to append our raw did, we'll repeat.
        if(append_raw_did_to_route(&raw_payload_bytes[ON_PLD_DATA_IDX],
          (const on_raw_did_t* const) my_raw_did) == -1)
        {
            // no room.  Don't repeat.
            return ONS_RSRC_FULL;
        }
        
        // looks like we'll repeat.  TODO -- use named constant length for 23.
        // Change the payload CRC, re-encrypt, re-encode, re-calculate the
        // message CRC.
        raw_payload_bytes[0] = one_net_compute_crc(&raw_payload_bytes[1], 23,
            ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER);
        #ifndef STREAM_MESSAGES_ENABLED
        if((status = on_encrypt(raw_payload_bytes,
          (const one_net_xtea_key_t* const) key, get_raw_payload_len(raw_pid)))
          != ONS_SUCCESS)
        #else
        if((status = on_encrypt(FALSE, raw_payload_bytes,
         (const one_net_xtea_key_t * const) key, get_raw_payload_len(raw_pid)))
         != ONS_SUCCESS)
        #endif
        {
            return status;
        }
        if((status = on_encode(&((*this_pkt_ptrs)->packet_bytes[ON_ENCODED_PLD_IDX]),
          raw_payload_bytes, (*this_pkt_ptrs)->payload_len)) != ONS_SUCCESS)
        {
            return status;
        }
        msg_crc = calculate_msg_crc(*this_pkt_ptrs);
        (*this_pkt_ptrs)->packet_bytes[ON_ENCODED_MSG_CRC_IDX] =
          decoded_to_encoded_byte(msg_crc, TRUE);
    }
    #endif

    return ONS_PKT_RCVD;
} // on_rx_packet //


#ifdef PID_BLOCK
void set_pid_block(UInt8 raw_pid, BOOL accept)
{
    UInt16 mask = 1;
    
    if(raw_pid == 0xFF)
    {
        if(accept)
        {
            pid_block_info.block_pid_list = 0xFFFF;
        }
        else
        {
            pid_block_info.block_pid_list = 0;
        }
        return;
    }
    else if(raw_pid > 0x0F)
    {
        return;
    }
    
    mask <<= raw_pid;
    pid_block_info.block_pid_list &= ~mask;
    if(accept)
    {
        pid_block_info.block_pid_list |= mask;
    }
}


void set_pid_block_sa(pid_block_criteria_t sa_block)
{
    pid_block_info.sa_block = sa_block;
}


void set_pid_block_mh(pid_block_criteria_t mh_block)
{
    pid_block_info.mh_block = mh_block;
}


BOOL pid_is_blocked(UInt8 raw_pid)
{
    BOOL sa = packet_is_stay_awake(raw_pid);
    BOOL mh = packet_is_multihop(raw_pid);
    UInt16 mask = 1;
    
    if(!pid_blocking_on)
    {
        return FALSE;
    }
    
    if(pid_block_info.sa_block != PID_ACCEPT)
    {
        if(sa && (pid_block_info.sa_block == PID_REJECT_IF_PRESENT))
        {
            return TRUE;
        }
        if(!sa && (pid_block_info.sa_block != PID_REJECT_IF_PRESENT))
        {
            return TRUE;
        }
    }
    
    if(pid_block_info.mh_block != PID_ACCEPT)
    {
        if(mh && (pid_block_info.mh_block == PID_REJECT_IF_PRESENT))
        {
            return TRUE;
        }
        if(!mh && (pid_block_info.mh_block != PID_REJECT_IF_PRESENT))
        {
            return TRUE;
        }
    }
    
    raw_pid &= 0x3F;
    if(raw_pid > 0x0F)
    {
        // TODO -- Bad PID as of 2.3.0.  Might be good in the future.  Really reject here?
        return TRUE;
    }
    
    mask <<= raw_pid;
    return ((pid_block_info.block_pid_list & mask) == 0);
}
#endif


#ifdef RANGE_TESTING
/*!
    \brief Turns range testing either on or off.

    \param on If false, range testing should be turned off. If true, range
              testing should be turned on.
    \return void
*/
void enable_range_testing(BOOL on)
{
    range_testing_on = on;
}


/*!
    \brief Returns an array of the encoded dids which are within range

    \param[out] enc_dids Array of encoded dids to store the in-range devices
    \param[in/out] num_in_range in --> size of array passed.  out --> number
                   of devices within range.
    \param[out] on True if range testing is on, false otherwise.
    
    \return TRUE if no errors and devices could be retrieved.
            FALSE is bad paraemter, device is not in a network, or not enough
                  room exists in the passed array.
*/
BOOL devices_within_range(on_encoded_did_t* enc_dids, UInt8* num_in_range,
    BOOL* on)
{
    UInt8 i = 0;
    UInt8 arr_size;

    
    if(!enc_dids || !num_in_range || !on)
    {
        return FALSE; // bad parameter.
    }

    arr_size = *num_in_range;
    *num_in_range = 0;
    
    while(i < RANGE_TESTING_ARRAY_SIZE &&
      one_net_memcmp(range_test_did_array[i], ON_ENCODED_BROADCAST_DID,
      ON_ENCODED_DID_LEN) != 0)
    {
        i++;
        if(i >= arr_size)
        {
            return FALSE; // no room.
        }
    }
    
    *num_in_range = i;
    one_net_memmove(*enc_dids, range_test_did_array, ((*num_in_range) *
      sizeof(on_encoded_did_t)));
    *on = range_testing_on;
    return TRUE;
}


/*!
    \brief Places a device either in range or out of range.

    \param did encoded did of the device
    \param add if true, make this device in range.  If false, then the device
               is out of range.
    \return TRUE If the list was changed
            FALSE If the list was not changed.
*/
BOOL adjust_range_test_did_array(on_encoded_did_t* const did, BOOL add)
{
    SInt8 i;
    SInt8 index = -1;
    SInt8 empty_index = -1;
    
    for(i = RANGE_TESTING_ARRAY_SIZE - 1; i >= 0 ; i--)
    {
        if(one_net_memcmp(range_test_did_array[i], ON_ENCODED_BROADCAST_DID,
          ON_ENCODED_DID_LEN) == 0)
        {
            empty_index = i;
        }
        else if(one_net_memcmp(range_test_did_array[i], *did,
          ON_ENCODED_DID_LEN) == 0)
        {
            index = i;
        }
    }
    
    if(!add && index == -1)
    {
        return TRUE; // nothing to do
    }
    else if(add && index != -1)
    {
        return TRUE; // nothing to do.
    }
    else if(add && empty_index == -1)
    {
        return FALSE; // no room.
    }
    else if(add)
    {
        one_net_memmove(range_test_did_array[empty_index], *did,
          ON_ENCODED_DID_LEN);
        return TRUE; // added
    }
    else
    {
        one_net_memmove(range_test_did_array[index],
          ON_ENCODED_BROADCAST_DID, ON_ENCODED_DID_LEN);
        return TRUE; // added        
    }
}


/*!
    \brief Resets the range testing array to make everything out of range

    \return void
*/
void reset_range_test_did_array(void)
{
    one_net_memset_block(&range_test_did_array[0], ON_ENCODED_DID_LEN,
      RANGE_TESTING_ARRAY_SIZE, ON_ENCODED_BROADCAST_DID);
}


/*!
    \brief Checks to see whether a device is within range of this device

    \param did encoded did of the device

    \return TRUE If the device is in range
            FALSE If the device not in range
*/
BOOL device_in_range(on_encoded_did_t* did)
{
    SInt8 i;
    
    if(!range_testing_on)
    {
        return TRUE;
    }
    
    for(i = RANGE_TESTING_ARRAY_SIZE - 1; i >= 0 ; i--)
    {
        if(one_net_memcmp(range_test_did_array[i], *did,
          ON_ENCODED_DID_LEN) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}
#endif


#ifdef ROUTE
one_net_status_t send_route_msg(const on_raw_did_t* raw_did)
{
    UInt8 raw_pld[ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN];
    on_encoded_did_t enc_dst;
    one_net_status_t status;
    on_raw_did_t my_raw_did;
    
    
    if((status = on_encode(enc_dst, *raw_did, ON_ENCODED_DID_LEN)) !=
      ONS_SUCCESS)
    {
        return status;
    }
    if((status = on_decode(my_raw_did, &on_base_param->sid[ON_ENCODED_NID_LEN],
      ON_ENCODED_DID_LEN)) != ONS_SUCCESS)
    {
        return status;
    }    
    
    one_net_memset(raw_pld, 0, ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN);
    append_raw_did_to_route(raw_pld, (const on_raw_did_t* const)
      my_raw_did);
    
    if(one_net_send_single(ONE_NET_RAW_ROUTE, ON_ROUTE_MSG, raw_pld,
      ONA_EXTENDED_SINGLE_PACKET_PAYLOAD_LEN, ONE_NET_LOW_PRIORITY,
      NULL, (const on_encoded_did_t* const) enc_dst
      #ifdef PEER
        , FALSE,
        ONE_NET_DEV_UNIT
      #endif
      #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
        , 0
      #endif
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	    , 0
      #endif
      ) == NULL)
    {
        return ONS_RSRC_FULL;
    }
    
    return ONS_SUCCESS;
}


UInt16 extract_raw_did_from_route(const UInt8* route, UInt8 index)
{
    const int MAX_NUM_DIDS = 14;
    UInt16 raw_did_int;
    
    if(index >= MAX_NUM_DIDS)
    {
        return 0;
    }
    
    route += ((3 * index) / 2);
    if(index % 2)
    {
        raw_did_int = ((route[0] & 0x0F) << 8) + route[1];
    }
    else
    {
        raw_did_int = (route[0] << 4) + (route[1] >> 4);
    }
    
    return raw_did_int;
}


SInt8 find_raw_did_in_route(const UInt8* route,
  const on_raw_did_t* const raw_did, SInt8 minimum_index)
{
    UInt8 route_index;
    UInt16 raw_did_int;
    const int MAX_NUM_DIDS = 14;
    
    if(!route || !raw_did)
    {
        return -1;
    }
    raw_did_int = did_to_u16(raw_did);
    
    for(route_index = minimum_index < 0 ? 0 : (UInt8) minimum_index;
      route_index < MAX_NUM_DIDS; route_index++)
    {
        if(extract_raw_did_from_route(route, route_index) == raw_did_int)
        {
            return route_index;
        }
    }
    
    return -1;
}


// return -1 for bad parameter or no room left.  Buffer must be at least 21
// bytes long.
SInt8 append_raw_did_to_route(UInt8* route, const on_raw_did_t* const raw_did)
{
    on_raw_did_t vacant_raw_did = {0, 0};
    SInt8 route_index = find_raw_did_in_route(route,
      (const on_raw_did_t* const) vacant_raw_did, 0);
    route += ((3 * route_index) / 2);  
    
    if(route_index != -1)
    {
        // found a spot
        if(route_index % 2)
        {
            route[0] |=  (((*raw_did)[0] & 0xF0) >> 4);
            route[1] = (((*raw_did)[0] & 0x0F) << 4) +
              (((*raw_did)[1] & 0xF0) >> 4);
        }
        else
        {
            route[0] = (*raw_did)[0];
            route[1] = ((*raw_did)[1] & 0xF0);
        }
    }
    
    return route_index;
}


/*!
    \brief Takes a route message and determines the number of hops involved in
      the way to and back, as well as the repeaters involved.

    \param[in] dst The destination device
    \param[in/out] route The route taken
    \param[out] hops The number of hops between source and destination
    \param[out] return_hops The number of hops in the ACK or NACK
    \param[out] num_repeaters The number of devices which are functioning as
               repeaters in this route.
    \param[out] repeaters A list of the repeaters in the route.

    \return TRUE if the route is valid.
            FALSE otherwise
*/
BOOL extract_repeaters_and_hops_from_route(const on_encoded_did_t* const
  dst, UInt8* route, UInt8* hops, UInt8* return_hops,
  UInt8* num_repeaters, on_encoded_did_t* repeaters)
{
    // One of the main uses of this function is to change data rates and
    // channels.  Not only do the repeaters have to be known, but the ORDER
    // can be quite crucial.  The majority of the time, the route TO a
    // device and the route BACK will be the same, but one cannot assume.
     
    // TODO -- do some research on the statement above.  CAN we assume that the
    // route TO a device and FROM a device will be the same?  I imagine one
    // can think of some situations where the positioning and direction of
    // devices and antennas, plus the power of various transceivers as far as
    // transmission and reception range would cause the return path to deviate
    // from the original path.  For now, definitely do NOT make that assumption.
    // However, we can save some code space and complexity and RAM if we only
    // need to store a ONE-WAY route and can ignore the "oddball" situations
    // where the routes are different (again, how "oddball" is this in real life
    // and what are the pros and cons of assuming the same route?).
    
    // We'll likely be changing the data rates, so we'll want to change the
    // farthest devices first, then go towards us because the close repeaters
    // need to be available for repeating the distant repeaters, but not vice-
    // versa, then work backwards one by one.  The route TO the device and the
    // route BACK may be the same or may be different(see above).  Routes where
    // the route back is different are currently not thoroughly tested, so it
    // might not all work well at the moment.  More testing is needed.  In
    // addition, a 38,400 baud route might not be achievable at higher routes,
    // so more needs to be done with this.
    // TODO -- address the issues above.
    
    // An example might be the following.  The source is 002, the
    // destination is 006, and the route is
    //
    // 002-003-004-005-006-007-005-003-002.
    //
    // 006 is passed to us as the destiantion.  002 denotes the source.  The
    // repeaters are therefore all the devices in the route other than 002 and
    // 006.  We need to get them in the right order from far to near.  That
    // order is 007,005,004,003.  The only compilcated one is 005 versus 007.
    // has to be BEFORE 005 because it needs 005 to get an ACK back to 002.
    // Therefore if we accidentally raise 005's data rate BEFORE 007, 007 will
    // be unreachable because 005 will have raised its data rate already.  So
    // we need to do some checking to make sure this does not happen.
    //
    // We need to always make sure that a device is not already in the list
    // before adding it.  Invalid routes include...
    //
    // 1. Source does not appear exactly twice.
    // 2. Destination does not appear exactly once.
    // 3. A device appears AFTER the second time the source appears.
    // 4. Any device appears more than once either BEFORE the destination or
    //    AFTER the destination.
    // 5. Any repeaters that mutually need each other to complete the route.
    
    // Algorith for the 002-003-004-005-006-007-005-003-002 example.
    
    // 1. Find the index of the recipient(006), which is 4.
    // 2. The next possible additions will be indexes 3 or 5.  We'll test
    //    and pick one of them.
    // 3. Are the devices at index 3 and index 5 the same?
    // 3a. If so, there is no conflict, add one of them.
    // 3b. If they are different (as they are here, 005 and 007, we test to
    //     see if one needs the other.
    // 3c. If both show up exactly once in the route, neither needs the
    //     other, so we can add both and it does not matter which order we
    //     add.
    // 3d. If both need each other, there is a problem with the route.
    // 3e. If one of them needs the other, add the one that needs the other
    //     first.
    // 4.  In this case, 007 needs 005.  We decide that by noticing that 005
    //     comes AFTER 007 on the return route.  Note that 007 does not come
    //     BEFORE 005 on the source route.
    // 5. Therefore add 007, then increment the return index from 5 to 6.
    //    Index 6 is 005, so repeat steps 3 to 5 with indexes 3 and 6.
    // 6. Eventually the route is all added.    

    
    
    UInt8 i;
    SInt8 idx, to_idx, return_idx, last_idx;
    UInt16 raw_did_int, to_raw_did_int, return_raw_did_int;
    on_raw_did_t raw_did;
    
    if(!dst || !route || !hops || !return_hops || !num_repeaters ||
      !repeaters)
    {
        return FALSE;
    }

    raw_did_int = extract_raw_did_from_route(route, 0);
    u16_to_did(raw_did_int, (on_raw_did_t*) raw_did);
    
    while((last_idx = find_raw_did_in_route(route, (const on_raw_did_t* const)
      raw_did, 1)) < 0)
    {
        if(append_raw_did_to_route(route, (const on_raw_did_t* const) raw_did)
          < 0)
        {
            return FALSE;
        }
    }
    
    if(last_idx < 2)
    {
        return FALSE;
    }
    
    if(on_decode(raw_did, *dst, ON_ENCODED_DID_LEN) != ONS_SUCCESS)
    {
        return FALSE;
    }
    
    idx = find_raw_did_in_route(route, (const on_raw_did_t* const) raw_did, 1);
    if(idx < 0 || idx >= last_idx)
    {
        return FALSE;
    }
    
    return_idx = find_raw_did_in_route(route, (const on_raw_did_t* const)
      raw_did, idx + 1);
      
    if(return_idx >= 0)
    {
        return FALSE; // bad. Shows up twice.
    }
    
    *hops = idx - 1;
    *return_hops = last_idx - idx - 1;
    return_idx = idx + 1;
    to_idx = idx - 1;
    *num_repeaters = 0;
    
    
    while(to_idx > 0 || return_idx < last_idx)
    {
        on_encoded_did_t to_enc_did ;
        on_encoded_did_t return_enc_did;
        on_encoded_did_t* did_to_add = NULL;
        BOOL found = FALSE;
        
        if(to_idx > 0)
        {
            to_raw_did_int = extract_raw_did_from_route(route, to_idx);
            u16_to_did(to_raw_did_int, (on_raw_did_t*) raw_did);
            on_encode(to_enc_did, raw_did, ON_ENCODED_DID_LEN);
        }
        if(return_idx < last_idx)
        {
            return_raw_did_int = extract_raw_did_from_route(route,
              return_idx);
            u16_to_did(return_raw_did_int, (on_raw_did_t*) raw_did);
            on_encode(return_enc_did, raw_did, ON_ENCODED_DID_LEN);
        }
        
        if(return_idx >= last_idx)
        {
            did_to_add = &to_enc_did;
            to_idx--;
        }
        else if(to_idx == 0)
        {
            did_to_add = &return_enc_did;
            return_idx++;
        }
        else if(on_encoded_did_equal((const on_encoded_did_t* const) to_enc_did,
          (const on_encoded_did_t* const) return_enc_did))
        {
            // they are the same.  Just add one of them.
            did_to_add = &to_enc_did;
            to_idx--;
            return_idx++;
        }
        else
        {
            // here is a case where the dids are different.  We need to pick
            // one of them.
            u16_to_did(to_raw_did_int, (on_raw_did_t*) raw_did);
            if(find_raw_did_in_route(route, (const on_raw_did_t* const)
              raw_did, return_idx + 1) < 0)
            {
                // the ACK route does not need the message device as the
                // did so add the message device first
                did_to_add = &to_enc_did;
                to_idx--;
            }
            else
            {
                // the ACK route does not need the message route as the
                // did so add it.
                did_to_add = &return_enc_did;
                return_idx++;
            }
        }
        
        // go through and make sure it is not already on the list.
        for(i = 0; i < *num_repeaters; i++)
        {
            if(on_encoded_did_equal((const on_encoded_did_t* const) did_to_add,
              (const on_encoded_did_t* const) &repeaters[i]))
            {
                found = TRUE;
            }
        }
        
        if(!found)
        {
            one_net_memmove(repeaters[*num_repeaters], *did_to_add,
              ON_ENCODED_DID_LEN);
            (*num_repeaters)++;
        }
    }    

    return TRUE;
}
#endif


#ifdef DATA_RATE_CHANNEL
/*!
    \brief Sets up a data rate and channel change to occur in the future
    
    This function is called either to order other devices to change data rates
    and channels or to change a device's own channel.
           
    \param[in] enc_did The device whose data rate should change (possibly this device)
               If NULL, then it is this device.
    \param[in] pause_time_ms Time to pause BEFORE changing data rates, in ms
    \param[in] dormant_time_ms Amount of time in milliseconds, to spend waiting
               for a relevant message, before reverting back to the original data
               rate and channel.  Every time a relevant message is received, this
               wait time starts again.
    \param[in] new_channel The channel to change to.
    \param[in] new_data_rate The data rate to change to.

    \return Upon success and agreement to change according to the parameters,
            ON_NACK_RSN_NO_ERROR.
            If unable or unwilling to schedule the change, a NACK reason is
            returned.
*/
on_nack_rsn_t on_change_dr_channel(const on_encoded_did_t* enc_did,
  UInt16 pause_time_ms, UInt16 dormant_time_ms, UInt8 new_channel,
  UInt8 new_data_rate)
{
    UInt8 pld[7];
    on_sending_device_t* device;
    
    if(!enc_did || is_my_did(enc_did))
    {
        if(!features_data_rate_capable(THIS_DEVICE_FEATURES, new_data_rate))
        {
            return ON_NACK_RSN_INVALID_DATA_RATE;
        }
        
        alternate_data_rate = new_data_rate;
        alternate_channel = new_channel;
        dormant_data_rate_time_ms = dormant_time_ms;
        dr_channel_stage = ON_DR_CHANNEL_CHANGE_SCHEDULED;
        ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, MS_TO_TICK(pause_time_ms));
        return ON_NACK_RSN_NO_ERROR;
    }
    
    device = (*get_sender_info)(enc_did);
    
    #ifdef ONE_NET_MASTER
    if(device_is_master && !device)
    {
        return ON_NACK_RSN_DEVICE_NOT_IN_NETWORK;
    }
    #endif
    if(device && !features_data_rate_capable(device->features, new_data_rate))
    {
        return ON_NACK_RSN_INVALID_DATA_RATE;
    }
    
    
    pld[0] = ON_CHANGE_DATA_RATE_CHANNEL;
    pld[1] = new_channel;
    pld[2] = new_data_rate;
    one_net_uint16_to_byte_stream(pause_time_ms, &pld[3]);
    one_net_uint16_to_byte_stream(dormant_time_ms, &pld[5]);
    
    return (one_net_send_single(ONE_NET_RAW_SINGLE_DATA, ON_ADMIN_MSG,
      pld, 7, ONE_NET_LOW_PRIORITY, NULL, enc_did
      #ifdef PEER
      , FALSE, ONE_NET_DEV_UNIT
      #endif
      #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
      , 0
      #endif
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
	  , 0
      #endif
      ) == NULL ? ON_NACK_RSN_RSRC_UNAVAIL_ERR : ON_NACK_RSN_NO_ERROR);
}
#endif


/*!
    \brief Resets all message IDs to low values.

    Resets all message IDs to low values.
*/
void reset_msg_ids(void)
{
    UInt16 i;
    // set them randomly low.  Just make it a random number from 0 to 50.
    #if !defined(ONE_NET_MASTER)
    master->device.msg_id = one_net_prand(get_tick_count(), 50);
    for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
    {
        sending_dev_list[i].sender.msg_id = one_net_prand(get_tick_count(), 50);
    }
    #elif !defined(ONE_NET_CLIENT)
    for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
    {
        client_list[i].device.msg_id = one_net_prand(get_tick_count(), 50);
    }
    #else
    if(device_is_master)
    {
        for(i = 0; i < ONE_NET_MASTER_MAX_CLIENTS; i++)
        {
            client_list[i].device.msg_id = one_net_prand(get_tick_count(), 50);
        }
    }
    else
    {
        master->device.msg_id = one_net_prand(get_tick_count(), 50);
        for(i = 0; i < ONE_NET_RX_FROM_DEVICE_COUNT; i++)
        {
            sending_dev_list[i].sender.msg_id = one_net_prand(get_tick_count(),
                50);
        }        
    }
    #endif
}


/*!
    \brief Checks to see whether a fragment is in memory.  If not, moves the
           fragment into memory.
           
    \param[in] fragment The new key fragment.
    \param[in] copy_key If true, copy the new key into memory if the key is new

    \return TRUE If memory was changed
            FALSE if memory was not changed
*/
BOOL new_key_fragment(const one_net_xtea_key_fragment_t* const fragment,
  BOOL copy_key)
{
    UInt8 i;
    const one_net_xtea_key_fragment_t* param_frag =
      (const one_net_xtea_key_fragment_t*) &(on_base_param->old_key);
    
    if(!fragment)
    {
        return FALSE;
    }
    
    // check whether the fragment matches anything we have already (Need to
    // check the old key fragment and 4 from the new one.  1 + 4 = 5.
    for(i = 0; i < 5; i++)
    {
        if(one_net_memcmp(*param_frag, *fragment,
          ONE_NET_XTEA_KEY_FRAGMENT_SIZE) == 0)
        {
            return FALSE; // match
        }
        param_frag++;
    }
    
    if(copy_key)
    {
        one_net_memmove(on_base_param->old_key, on_base_param->current_key,
          ONE_NET_XTEA_KEY_LEN);
        one_net_memmove(&(on_base_param->current_key[3 * ONE_NET_XTEA_KEY_FRAGMENT_SIZE]),
          fragment, ONE_NET_XTEA_KEY_FRAGMENT_SIZE);
        reset_msg_ids();
    }
    
    return TRUE;
}


/*!
    \brief Determines whether an invalid message ID should be rejected by a device.
    
    Generally, devices which are NOT at risk of a replay attack should not care
    about bad message IDs.  A very large number of devices would likely not
    have security concerns and in particular, not be concerned about relay attacks.
    
    There are always exceptions, but generally these devices would not be thetarget
    of replay attacks.
    
    1. Devices that control whether a television or stereo or lights is on or off.
    2. Motion sensors that determine when someone walks into a store.
    3. Temperature / humidity sensors.
    
    
    The following devices likely WOULD be subject to replay attacks and should reject
    messages with invalid message ids.
    
    1. Garage door openers or any device that opens and closes something that has a lock.
    2. Any sort of device that is part of an anti-trespassing or other intruder-detection
       or prevention system.
    3. Any other device that is part of a security system.
    

    If a device should reject an invalid message ID, it should have the
    ON_REJECT_INVALID_MSG_ID bit of its flags set.  Otherwise it should not.
    
    
    Currently the master controls whether this is true and the default setting
    is determined by whether the ONE_NET_MASTER_REJECT_INVALID_MSG_ID in
    one_net_mster_port_const.h true or false.
    
    
    TODO -- More fine-tuning of this flag needs to be done.


    \param[in] device -- The device that is being checked.

    \return TRUE if the device should reject invalid message IDs
            FALSE otherwise.
*/
BOOL one_net_reject_bad_msg_id(const on_sending_device_t* device)
{
    #ifdef ONE_NET_CLIENT
    if(!device_is_master)
    {
        return ((master->flags & ON_JOINED) && (master->flags &
          ON_REJECT_INVALID_MSG_ID));
    }
    #endif
    #ifdef ONE_NET_MASTER
    if(device_is_master)
    {
        on_client_t* client = client_info((const on_encoded_did_t*)
          device->did);
        if(client == NULL)
        {
            return FALSE;;
        }
        return ((client->flags & ON_JOINED) && (client->flags &
          ON_REJECT_INVALID_MSG_ID));
    }
    #endif
    
    return FALSE; // should never get here?
}


#ifdef BLOCK_MESSAGES_ENABLED
// TODO -- Do we really want to require block messages for this function?


/*!
    \brief Calculates the estimated time, in milliseconds that it should take
    for a response after a message is sent.
    
    Calculates the estimated time, in milliseconds that it should take
    for a response after a message is sent.  This can be a very rough estimate.
    The clock starts when the source device has sent the last bit of the data packet.
    The clock stops when the last repeater sends the last bit of the response.

    \param[in] data_len The length, in bytes, of the encoded data packet
    \param[in] response_len The length, in bytes, of the encoded response packet
    \param[in] dst_process_time The amount of the time the destination is expected
               to take processing the packet (i.e. encrypting, decrypting, etc.,
               including any time spent looking for a clear channel.  Does not include
               the time that it takes to receive from / write to the transceiver.
    \param[in] dst_process_time The amount of the time a repeater is expected
               to take processing the packet (generally there is less work for a
               repeater to do than the recipient because repeaters generally will
               not need to decrypt or encrypt the packet), including any time spent
               looking for a clear channel.  Does not include the time that it takes
               to receive from / write to the transceiver.
    \param[in] The data rate that the devices are receiving / transmitting at.

    \return The estimated time in milliseconds between when the source sends the data
            packet and receives the response packet.
*/
#ifdef ONE_NET_MULTI_HOP
UInt16 estimate_response_time(UInt8 data_len, UInt8 response_len,
  UInt8 hops, UInt16 dst_process_time, UInt16 repeater_process_time,
  UInt8 data_rate)
#else
UInt16 estimate_response_time(UInt8 response_len, UInt8 dst_process_time,
  UInt8 data_rate)
#endif 
{
    // The time between when a message is sent and when the response
    // should be received is based on these factors...
    // 
    // 1. Amount of time devices are actually sending a message to
    //    the transceiver.
    // 2. The amount of time spent processing the message between receiving it
    //    and sending it.
    double dst_write_time = (1000 * response_len * 8) /
      (38400 * (data_rate + 1));
    double dst_time = dst_write_time + dst_process_time;
    #ifndef ONE_NET_MULTI_HOP
    return (UInt16) dst_time;
    #else
    double rptr_data_write_time = hops * (1000 * data_len * 8) /
      (38400 * (data_rate + 1));
    double rptr_response_write_time = hops * (1000 * response_len * 8) /
      (38400 * (data_rate + 1));
    double rptr_process_time = 2 * hops * repeater_process_time;
    return (UInt16) (dst_time + rptr_data_write_time +
      rptr_response_write_time + rptr_process_time);
    #endif
    
}


/*!
    \brief Adjusts the priority in an in-progress block / stream transfer
    
    This function can be called by the application code of either the sender
    or recipient of a block / stream transfer already in progress in order to
    change the priority of the transfer.

    \param[in/out] msg The block / stream message in progress
    \param[in] priority The new priority
*/
void adjust_bs_priority(block_stream_msg_t* msg, UInt8 priority)
{
    set_bs_priority(&msg->flags, priority);
    msg->use_saved_ack_nack = TRUE;
    msg->saved_ack_nack.nack_reason = ON_NACK_RSN_INVALID_PRIORITY;
    msg->saved_ack_nack.handle = ON_NACK_VALUE;
    msg->saved_ack_nack.payload->nack_value = priority;
}


/*!
    \brief Adjusts the chunk_pause in an in-progress block transfer
    
    This function can be called by the application code of either the sender
    or recipient of a block transfer already in progress in order to
    change the chunk pause (time in milliseconds between data "chunks")

    \param[in/out] msg The block / stream message in progress
    \param[in] chunk_pause The new chunk pause (in milliseconds)
*/
void adjust_bs_chunk_pause(block_stream_msg_t* msg, UInt16 chunk_pause)
{
    msg->bs.block.chunk_pause = chunk_pause;
    msg->use_saved_ack_nack = TRUE;
    msg->saved_ack_nack.nack_reason = ON_NACK_RSN_INVALID_CHUNK_DELAY;
    msg->saved_ack_nack.handle = ON_NACK_VALUE;
    msg->saved_ack_nack.payload->nack_value = chunk_pause;
}


/*!
    \brief Pauses a block or stream transfer that is already in progress
    
    This function can be called by the application code of either the sender
    or recipient of a block / stream transfer already in progress in order to
    pause it.

    \param[in/out] msg The block / stream message in progress
    \param[in] pause_ms The time to pause (in milliseconds)
*/
void pause_bs_msg(block_stream_msg_t* msg, UInt16 pause_ms)
{
    msg->use_saved_ack_nack = TRUE;
    msg->saved_ack_nack.nack_reason = ON_ACK_PAUSE_TIME_MS;
    msg->saved_ack_nack.handle = ON_NACK_PAUSE_TIME_MS;
    msg->saved_ack_nack.payload->nack_value = pause_ms;
    ont_set_timer(ONT_BS_TIMER, MS_TO_TICK(
      msg->saved_ack_nack.payload->nack_time_ms));
    ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(
      msg->saved_ack_nack.payload->nack_time_ms + msg->timeout));
    #ifdef DATA_RATE_CHANNEL
    // Adjust the data rate timer too.
    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, MS_TO_TICK(
      msg->saved_ack_nack.payload->nack_time_ms + msg->timeout));
    #endif
    
    if(!msg->src)
    {
        // we are the source
        msg->bs_on_state = ON_BS_CHUNK_PAUSE;
        on_state = ON_LISTEN_FOR_DATA;
    }
}


/*!
    \brief Adjusts the fragment delay in an in-progress block / stream transfer
    
    This function can be called by the application code of either the sender
    or recipient of a block / stream transfer already in progress in order to
    change the fragment delay (time in milliseconds between data packets)

    \param[in/out] msg The block / stream message in progress
    \param[in] frag_delay The new fragment delay (in milliseconds)
*/
void adjust_bs_frag_delay(block_stream_msg_t* msg, UInt16 frag_delay)
{
    msg->frag_dly = frag_delay;
    msg->use_saved_ack_nack = TRUE;
    msg->saved_ack_nack.nack_reason = ON_NACK_RSN_INVALID_FRAG_DELAY;
    msg->saved_ack_nack.handle = ON_NACK_VALUE;
    msg->saved_ack_nack.payload->nack_value = frag_delay;
}


/*!
    \brief Calculates the estimated time for a block transfer to complete
    
    Calculates the estimated time for a block transfer to complete.  This can
    be a very rough estimate.  Many of these parameters might change DURING
    the transfer to improve reliablity, releive clogging, etc.  This function
    returns a very optimistic time estimate assuming no collisons, delays, no
    packet loss, and all packets are accpeted by each side as valid.

    \param[in] bs_msg The block / stream parameters.

    \return The estimated time in milliseconds of the transfer given no
            collisions or extra delays and 0% packet loss
*/
UInt32 estimate_block_transfer_time(const block_stream_msg_t* bs_msg)
{
    // TODO -- this function can definitely be improved.  This is a REALLY
    // rough estimate!  It also hasn't really been tested.

    // TODO -- where is this function called?

    
    const UInt32 num_data_packets = bs_msg->bs.block.transfer_size /
      25; // 25 payload bytes in packet
    const UInt32 num_chunks = num_data_packets / bs_msg->bs.block.chunk_size;
    const UInt8 data_packet_len = 63; // TODO -- use constants.
    const UInt8 ack_packet_len  = 30; // TODO -- use constants.
    const double bytes_per_sec = (bs_msg->data_rate + 1) * 38400 / 8; // make it a
                                            // double to avoid integer division
    
    // rounding off is OK.  This is all just an estimate.
    double time_between_chunks = (((data_packet_len + ack_packet_len) *
      (get_bs_hops(bs_msg->flags) + 1)) / bytes_per_sec) * 1000.0;
    double time_per_data_packet = bs_msg->frag_dly + (data_packet_len /
      bytes_per_sec) * 1000;
    double time_per_chunk;
    
    if(time_between_chunks < bs_msg->bs.block.chunk_pause)
    {
        time_between_chunks = bs_msg->bs.block.chunk_pause;
    }
    
    time_per_chunk = (bs_msg->bs.block.chunk_size - 1) * time_per_data_packet +
      time_between_chunks;
      
    return (UInt32)(num_chunks * time_per_chunk);
}


#if defined(ONE_NET_MULTI_HOP) && defined(BLOCK_STREAM_REQUEST_MASTER_PERMISSION)
/*!
    \brief Called by a client initiating a block or stream message.  Requests the master's
           permission to reserve a multi-hop repeater for use as a repeater in
           its block / stream message.
    
    \param[in] bs_msg The block / stream message that the repeater is needed for.
    \param[in] repeater The repeater that is requested.
    
    \return void
*/
on_single_data_queue_t* request_reserve_repeater(
  const block_stream_msg_t* bs_msg, const on_encoded_did_t* repeater)
{
    UInt8 pld[14];
    UInt32 est_transfer_time = estimate_block_transfer_time(bs_msg);
      
    pld[0] = ON_REQUEST_REPEATER;
    one_net_memmove(&pld[1], *repeater, ON_ENCODED_DID_LEN);
    one_net_memmove(&pld[3], &on_base_param->sid[ON_ENCODED_NID_LEN],
      ON_ENCODED_DID_LEN);
    one_net_memmove(&pld[5], bs_msg->dst->did, ON_ENCODED_DID_LEN);
    
    one_net_uint32_to_byte_stream(est_transfer_time, &pld[7]);
    pld[11] = bs_msg->channel;
    pld[12] = bs_msg->data_rate;
    pld[13] = get_bs_priority(bs_msg->flags);
 
    return one_net_send_single(ONE_NET_RAW_SINGLE_DATA, ON_ADMIN_MSG, pld,
      14, ONE_NET_HIGH_PRIORITY, NULL, &MASTER_ENCODED_DID
      #ifdef PEER
          , FALSE,
          ONE_NET_DEV_UNIT
      #endif
      #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
          , 0
      #endif
      #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
    	  , 0
      #endif
      );
}
#endif


/*!
    \brief Called when a block or stream message is terminated
    
    This function may be called either by ONE-NET code or by the application code.
    It can be called either when this device decides to terminate the block or
    stream message or when another device had done so.
    
    The application level block or stream transaction handler can change the
    behavior of this function.  It can choose whether to inform other devices
    to terminate or it can terminate immediately without telling anyone.  Generally
    it is best to inform the other devices.  ONE-NET will determine whether and
    how to do so.  Otherwise the other devices may attempt to continue this
    transaction and fail with no idea why.
    
    To allow this default termination behavior to occur, the application-level
    block / stream transaction handler should return ON_MSG_RESPOND.  To abort
    immediately, it should return anything OTHER THAN ON_MSG_RESPOND.
    
    \param[in] bs_msg The block / stream message that is terminating.
    \param[in] terminating_did The DID of the device terminating the block / stream message.
                               If NULL, then this device is the device terminating.
    \param[in] status The reason for the termination (i.e. cancelled, successful, timeout)
    \param[in/out] ack_nack The reason and payload attached to the termination, if any.
    
    \return void
*/
void terminate_bs_msg(block_stream_msg_t* bs_msg,
  const on_encoded_did_t* terminating_did, on_message_status_t status,
  on_ack_nack_t* ack_nack)
{
    #ifdef STREAM_MESSAGES_ENABLED
    on_bs_txn_hdlr_t txn_hdlr = (get_bs_transfer_type(bs_msg->flags) ==
      ON_BLK_TRANSFER) ? pkt_hdlr.block_txn_hdlr : pkt_hdlr.stream_txn_hdlr;
    #else
    on_bs_txn_hdlr_t txn_hdlr = pkt_hdlr.block_txn_hdlr;
    #endif
      
    if(on_state >= ON_BS_TERMINATE || bs_msg->bs_on_state>= ON_BS_TERMINATE)
    {
        return; // already in the process of terminating.
    }

    if(ack_nack)
    {
        bs_msg->saved_ack_nack.nack_reason = ack_nack->nack_reason;
        bs_msg->saved_ack_nack.handle = ack_nack->handle;
        if(ack_nack->payload)
        {
            one_net_memmove(bs_msg->saved_ack_nack.payload, ack_nack->payload,
              5);
        }
    }
    else
    {
        bs_msg->saved_ack_nack.handle = ON_ACK;
        if(status == ON_MSG_ABORT)
        {
            bs_msg->saved_ack_nack.nack_reason = ON_NACK_RSN_ABORT;
        }
        else
        {
            bs_msg->saved_ack_nack.nack_reason = ON_NACK_RSN_NO_ERROR;
        }
    }
    
    // First inform the application code and give it a chance to change.
    if((*txn_hdlr)(bs_msg, terminating_did, &status, &bs_msg->saved_ack_nack) !=
      ON_MSG_RESPOND)
    {
        terminate_bs_complete(bs_msg);
        return;
    }

    if(status == ON_MSG_TIMEOUT)
    {
        // if this was a timeout, then presumably communication has been lost
        // so abort immediately.
        terminate_bs_complete(bs_msg);
        return;
    }
    
    bs_msg->bs_on_state = ON_BS_TERMINATE;
    on_state = ON_BS_TERMINATE;
    ont_set_timer(ONT_BS_TIMER, MS_TO_TICK(bs_msg->timeout));
    
    if(!bs_msg->src)
    {
        // We are the source.  Push a single message onto the queue.
        // We'll use the bs_txn.pkt buffer as temporary storage for the termination
        // message.  We'll change states to ON_BS_TERMINATE and make sure that
        // memory is not accidentally overwritten.
        on_ack_nack_t* response_ack_nack = (on_ack_nack_t*)
          &bs_txn.pkt[ON_ENCODED_DID_LEN+2];
        response_ack_nack->payload = (ack_nack_payload_t*)
          &bs_txn.pkt[ON_ENCODED_DID_LEN+4];        

        bs_txn.pkt[0] = ON_TERMINATE_BLOCK_STREAM;
        one_net_memmove(&bs_txn.pkt[1], &on_base_param->sid[ON_ENCODED_NID_LEN],
          ON_ENCODED_NID_LEN);
        if(terminating_did)
        {
            one_net_memmove(&bs_txn.pkt[1], *terminating_did,
              ON_ENCODED_DID_LEN);
        }
        bs_txn.pkt[ON_ENCODED_DID_LEN+1] = status;
        response_ack_nack->nack_reason = bs_msg->saved_ack_nack.nack_reason;
        response_ack_nack->handle = bs_msg->saved_ack_nack.handle;
        one_net_memmove(response_ack_nack->payload,
          bs_msg->saved_ack_nack.payload, 5);
        
        // TODO -- The length should be 10, not 11?  1 byte for status, 2 for terminating DID, 7 for the ACK/NACK with payload.
        //         That adds up to 10, and the receiving end seems to only use 10 bytes.
        push_queue_element(ONE_NET_RAW_SINGLE_DATA, ON_ADMIN_MSG,
          bs_txn.pkt, 11, ONE_NET_HIGH_PRIORITY, NULL,
          (const on_encoded_did_t* const) &bs_msg->dst->did
          #ifdef PEER
              , FALSE,
              ONE_NET_DEV_UNIT
          #endif
          #if SINGLE_QUEUE_LEVEL > MIN_SINGLE_QUEUE_LEVEL
              , 0
          #endif
          #if SINGLE_QUEUE_LEVEL > MED_SINGLE_QUEUE_LEVEL   
        	  , 0
          #endif
          );
    }
    else if(!bs_msg->dst)
    {
        // we are the destination.
        bs_msg->use_saved_ack_nack = TRUE;
        if(terminating_did)
        {
            // someone else initiated this termination, so wait a VERY
            // short time to see if any ACKs might be missed, then terminate
            ont_set_timer(ONT_BS_TIMER, MS_TO_TICK(500));
        }
    }
}
#endif


//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{



#ifdef DATA_RATE_CHANNEL
static void check_dr_channel_change(void)
{
    // see if we need to possible change data rates.
    if(ont_inactive_or_expired(ONT_DATA_RATE_CHANNEL_TIMER))
    {
        UInt8 new_data_rate = ONE_NET_DATA_RATE_38_4;
        UInt8 new_channel = on_base_param->channel;
        switch(dr_channel_stage)
        {
            case ON_DR_CHANNEL_CHANGE_SCHEDULED:
                // Change to the alternate channel and data rate
                new_data_rate = alternate_data_rate;
                new_channel = alternate_channel;
                ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, MS_TO_TICK(
                  dormant_data_rate_time_ms));
                // intentional fall-through           
            case ON_DR_CHANNEL_CHANGE_DONE:
                one_net_set_channel(new_channel);
                one_net_set_data_rate(new_data_rate);
                dr_channel_stage++;
                one_net_data_rate_channel_changed(new_channel, new_data_rate);
                // intentional fall-through
            default:
                break;
        }
    }
}
#endif


/*!
    \brief Checks if the channel is clear.

    If the channel is not clear, this function will set the clear channel time
    and will not check the channel again until the timer has expired.

    \param void

    \return TRUE If the channel is clear
            FALSE If the channel is not clear.
*/
static BOOL check_for_clr_channel(void)
{
    if(ont_inactive_or_expired(ONT_CLR_CHANNEL_TIMER))
    {
        ont_set_timer(ONT_CLR_CHANNEL_TIMER,
          MS_TO_TICK(ONE_NET_CLR_CHANNEL_TIME));
        return one_net_channel_is_clear();
    } // if it is time to check the channel //

    return FALSE;
} // check_for_clr_channel //


#ifdef BLOCK_MESSAGES_ENABLED
/*!
    \brief Receives a block data packet.

    This function is called when this device is the destination in a block
    transfer.  This function will either reject or accept a packet, pass it to
    the application code if it looks like it should be accepted, and send a
    response to the source device if needed.

    \param[in] txn The transactionbeing carried out.
    \param[in/out] bs_msg The block message associated with this packet.
    \param[in] block_pkt The block data packet received fom the source, including indexes and the data itself.
    ]param[out] ack_nack Filled in by this function. This is the response, if any, to the source device.

    \return ONS_RESPOND if the device should send a response.  No response will be sent otherwise.
            Note that ONS_RESPOND does not GUARANTEE a response will be sent.  More processing is done
            outside of this function that may cause the device to NOT respond.  However if ONS_RESPOND is
            NOT returned, no response will be sent.
*/
static on_message_status_t rx_block_data(on_txn_t* txn, block_stream_msg_t* bs_msg,
  block_pkt_t* block_pkt, on_ack_nack_t* ack_nack)
{
    on_message_status_t msg_status;
      
    if(bs_msg->bs_on_state >= ON_BS_TERMINATE)
    {
        goto bs_build_terminate_ack;
    }
      
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_NACK_VALUE;
    
    // TODO -- why is one of these unsigned?
    #ifndef COMPILE_WO_WARNINGS
    if(block_pkt->byte_idx != bs_msg->bs.block.byte_idx)
    #else
    if(block_pkt->byte_idx != (UInt32)bs_msg->bs.block.byte_idx)
    #endif
    {
        ack_nack->payload->nack_value = bs_msg->bs.block.byte_idx;
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_BYTE_INDEX;
        return ON_MSG_RESPOND;
    }
    
    if(block_pkt->chunk_size > bs_msg->bs.block.chunk_size)
    {
        ack_nack->payload->nack_value = bs_msg->bs.block.chunk_size;
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_CHUNK_SIZE;
        return ON_MSG_RESPOND;
    }
    
    ack_nack->handle = ON_ACK_BLK_PKTS_RCVD;
    if(!block_get_index_sent(block_pkt->chunk_idx, bs_msg->bs.block.sent))
    {
        msg_status = (*pkt_hdlr.block_data_hdlr)(txn, bs_msg, block_pkt,
          ack_nack);
        switch(msg_status)
        {
            case ON_MSG_TERMINATE: case ON_MSG_ABORT:
                terminate_bs_msg(bs_msg, NULL, msg_status, ack_nack);
                goto bs_build_terminate_ack;
            case ON_MSG_ACCEPT_PACKET:
                block_set_index_sent(block_pkt->chunk_idx, TRUE,
                  bs_msg->bs.block.sent);
                // reset the timeout timer since we received a packet.
                ont_set_timer(ONT_BS_TIMER, MS_TO_TICK(bs_msg->timeout));
            case ON_MSG_RESPOND:
                break;
            default:
                return ON_MSG_IGNORE;
        }
    }

    if(block_get_lowest_unsent_index(bs_msg->bs.block.sent,
      block_pkt->chunk_size) == -1)
    {
        // chunk has been received.
        #if !defined(ONE_NET_MASTER)
        msg_status = one_net_client_block_chunk_received(bs_msg,
          bs_msg->bs.block.byte_idx, block_pkt->chunk_size, ack_nack);
        #elif !defined(ONE_NET_CLIENT)
        msg_status = one_net_master_block_chunk_received(bs_msg,
          bs_msg->bs.block.byte_idx, block_pkt->chunk_size, ack_nack);
        #else
        if(device_is_master)
        {
            msg_status = one_net_master_block_chunk_received(bs_msg,
              bs_msg->bs.block.byte_idx, block_pkt->chunk_size, ack_nack);
        }
        else
        {
            msg_status = one_net_client_block_chunk_received(bs_msg,
              bs_msg->bs.block.byte_idx, block_pkt->chunk_size, ack_nack);
        }
        #endif
        
        switch(msg_status)
        {
            case ON_MSG_ACCEPT_CHUNK:
                bs_msg->bs.block.byte_idx += block_pkt->chunk_size;
                if(ack_nack->handle == ON_ACK_BLK_PKTS_RCVD)
                {
                    // not really a NACK, but that's OK.  The sending device will now
                    // sync.
                    ack_nack->handle = ON_NACK_VALUE;
                    ack_nack->payload->nack_value = bs_msg->bs.block.byte_idx;
                    ack_nack->nack_reason = ON_NACK_RSN_INVALID_BYTE_INDEX;
                }
            case ON_MSG_REJECT_CHUNK:
                one_net_memset(bs_msg->bs.block.sent, 0,
                  sizeof(bs_msg->bs.block.sent));
            #ifdef COMPILE_WO_WARNINGS
            // add default case that does nothing for clean compile
            default:
                break;
            #endif
        }
    }
    
    if(ack_nack->handle == ON_ACK_BLK_PKTS_RCVD)
    {
        one_net_memmove(ack_nack->payload->ack_payload, bs_msg->bs.block.sent,
          sizeof(bs_msg->bs.block.sent));
    }
    
    return ON_MSG_RESPOND;
    
bs_build_terminate_ack:
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK_ADMIN_MSG;
    one_net_memmove(ack_nack->payload->admin_msg, bs_txn.pkt, 11);
    return ON_MSG_RESPOND;
}


static void terminate_bs_complete(block_stream_msg_t* bs_msg)
{
    // abort immediately
    on_state = ON_LISTEN_FOR_DATA;
    bs_msg->bs_on_state = ON_LISTEN_FOR_DATA;
    bs_msg->transfer_in_progress = FALSE;
    bs_msg->use_saved_ack_nack = FALSE;
    #ifdef DATA_RATE_CHANNEL
    one_net_set_data_rate(ONE_NET_DATA_RATE_38_4);
    one_net_set_channel(on_base_param->channel);
    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, 0);
    #endif
}
#endif


#ifdef STREAM_MESSAGES_ENABLED
static on_message_status_t rx_stream_resp_pkt(on_txn_t* txn,
  block_stream_msg_t* bs_msg, on_pkt_t* pkt, UInt8* raw_payload_bytes,
  on_ack_nack_t* ack_nack)
{
    on_message_status_t status;
    on_encoded_did_t* terminating_did = &bs_msg->dst->did;
        
    if(on_parse_response_pkt(pkt->raw_pid, raw_payload_bytes, ack_nack) !=
      ONS_SUCCESS)
    {
        return ON_MSG_IGNORE; // undecipherable
    }
    
    if(!nack_reason_is_fatal(ack_nack->nack_reason))
    {
        terminating_did = NULL;
    }
    
    // send it up to the application code.
    status = (*pkt_hdlr.stream_ack_nack_hdlr)(txn, bs_msg, pkt,
      raw_payload_bytes, ack_nack);
      
    if(terminating_did)
    {
        status = ON_MSG_ABORT;
    }

    switch(status)
    {
        case ON_MSG_ABORT: case ON_MSG_TIMEOUT: case ON_MSG_TERMINATE:
          terminate_bs_msg(bs_msg, (const on_encoded_did_t*)terminating_did,
          status, ack_nack);
          return ON_MSG_TERMINATE;
        case ON_MSG_IGNORE:
          return ON_MSG_IGNORE;
        #ifdef COMPILE_WO_WARNINGS
        // add default case that does nothing for clean compile
        default:
            break;
        #endif
    }

    // we got a response, so reset the data rate change timer to the
    // timeout
    #ifdef DATA_RATE_CHANNEL
    ont_set_timer(ONT_DATA_RATE_CHANNEL_TIMER, MS_TO_TICK(bs_msg->timeout));
    #endif
    ont_set_timer(ONT_BS_TIMEOUT_TIMER, MS_TO_TICK(bs_msg->timeout));
    switch(ack_nack->handle)
    {
        case ON_ACK_SLOW_DOWN_TIME_MS:
          bs_msg->frag_dly += ack_nack->payload->nack_time_ms;
          break;
        case ON_ACK_SPEED_UP_TIME_MS:
          bs_msg->frag_dly += ack_nack->payload->nack_time_ms;
          break;
        case ON_ACK_PAUSE_TIME_MS:
          pause_bs_msg(bs_msg, ack_nack->payload->nack_time_ms);
          break;
        default:
        {
            switch(ack_nack->nack_reason)
            {
                case ON_NACK_RSN_INVALID_FRAG_DELAY:
                    bs_msg->frag_dly = ack_nack->payload->nack_time_ms;
                    break;
                case ON_NACK_RSN_INVALID_PRIORITY:
                    set_bs_priority(&bs_msg->flags,
                      ack_nack->payload->nack_value);
                    break;
                #ifdef COMPILE_WO_WARNINGS
                // add default case that does nothing for clean compile
                default:
                    break;
                #endif
            }
        }
    }

    if(status == ON_MSG_CONTINUE || status == ON_MSG_ACCEPT_PACKET)
    {
        bs_msg->bs.stream.last_response_time = get_tick_count();
        return ON_MSG_CONTINUE;
    }
    return ON_MSG_IGNORE;
}


/*!
    \brief Receives a stream data packet.

    This function is called when this device is the destination in a stream
    transfer.  This function will either reject or accept a packet, pass it to
    the application code if it looks like it should be accepted, and send a
    response to the source device if needed.

    \param[in] txn The transactionbeing carried out.
    \param[in/out] bs_msg The stream message associated with this packet.
    \param[in] block_pkt The stream data packet received fom the source.
    ]param[out] ack_nack Filled in by this function. This is the response, if any, to the source device.

    \return ONS_RESPOND if the device should send a response.  No response will be sent otherwise.
            Note that ONS_RESPOND does not GUARANTEE a response will be sent.  More processing is done
            outside of this function that may cause the device to NOT respond.  However if ONS_RESPOND is
            NOT returned, no response will be sent.
*/
static on_message_status_t rx_stream_data(on_txn_t* txn, block_stream_msg_t* bs_msg,
  stream_pkt_t* stream_pkt, on_ack_nack_t* ack_nack)
{
    on_message_status_t msg_status;
      
    if(bs_msg->bs_on_state >= ON_BS_TERMINATE)
    {
        goto bs_build_terminate_ack;
    }
      
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_NACK_VALUE;
    msg_status = (*pkt_hdlr.stream_data_hdlr)(txn, bs_msg, stream_pkt,
      ack_nack);
    
    switch(msg_status)
    {
        case ON_MSG_TERMINATE: case ON_MSG_ABORT:
            terminate_bs_msg(bs_msg, NULL, msg_status, ack_nack);
            goto bs_build_terminate_ack;
        case ON_MSG_RESPOND: case ON_MSG_ACCEPT_PACKET:
            bs_msg->bs.stream.last_response_time = get_tick_count();
            break;
        default:
            return ON_MSG_IGNORE;
    }
    return ON_MSG_RESPOND;
    
bs_build_terminate_ack:
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK_ADMIN_MSG;
    one_net_memmove(ack_nack->payload->admin_msg, bs_txn.pkt, 11);
    return ON_MSG_RESPOND;
}
#endif



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET
