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



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{


//! Preamble
static const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
static const UInt8 SOF[] = {0x33};

//! Header(Preamble and SOF)
static const UInt8 HEADER[] = {0x55, 0x55, 0x55, 0x33};


enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};


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


//! location to store the encoded data for an ack/nack packet
UInt8 response_pkt[ON_ACK_NACK_ENCODED_PKT_SIZE];

//! Used to send a response
on_txn_t response_txn = {ON_RESPONSE, ONE_NET_NO_PRIORITY, 0,
  ONT_RESPONSE_TIMER, 0, sizeof(response_pkt), response_pkt,
  NULL, NULL};

//! location to store the encoded data for the single transaction
UInt8 single_pkt[ON_SINGLE_ENCODED_PKT_SIZE];

//! Used to send a single message
on_txn_t single_txn = {ON_SINGLE, ONE_NET_NO_PRIORITY, 0, ONT_SINGLE_TIMER, 0,
  sizeof(single_pkt), single_pkt, NULL, NULL};

#ifdef _BLOCK_MESSAGES_ENABLED
    //! location to store the encoded data for a block transaction.
    UInt8 block_pkt[ON_BLOCK_ENCODED_PKT_SIZE];
    
    //! The current block transaction
    on_txn_t block_txn = {ON_BLOCK, ONE_NET_NO_PRIORITY, 0,
      ONT_BLOCK_TIMER, 0, sizeof(block_pkt), block_pkt, NULL, NULL};

    #ifdef _STREAM_MESSAGES_ENABLED
    //! location to store the encoded data for a stream transaction.
    UInt8 stream_pkt[ON_STREAM_ENCODED_PKT_SIZE];
    
    //! The current stream transaction
    on_txn_t stream_txn = {ON_STREAM, ONE_NET_NO_PRIORITY, 0,
      ONT_STREAM_TIMER, 0, sizeof(stream_pkt), stream_pkt, NULL, NULL};    
    #endif
#endif // if block messages are not enabled //



//! true if device is functioning as a master, false otherwise
#ifndef _ONE_NET_MASTER
BOOL device_is_master = FALSE;
#else
BOOL device_is_master = TRUE; // if device cvan be master OR client, the
                              // initialization code will need to set this
                              // value
#endif


one_net_startup_status_t startup_status = ON_STARTUP_IN_PROGRESS;


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




//                              PUBLIC VARIABLES
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{


//! The current state.
on_state_t on_state = ON_INIT_STATE;


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    // fill in the preamble in the Multi-Hop packet to be sent.  The rest will
    // be filled in when the received Multi-Hop packet is read in over the rf
    // interface.
    static UInt8 mh_pkt[ON_MAX_ENCODED_PKT_SIZE] = {0x55, 0x55, 0x55, 0x33};

    // Transaction for forwarding on MH packets.
    static on_txn_t mh_txn = {ON_NO_TXN, ONE_NET_LOW_PRIORITY, 0,
      ONT_MH_TIMER, 0, sizeof(mh_pkt), mh_pkt};
#endif



//! @} ONE-NET_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{



static BOOL check_for_clr_channel(void);



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{
    
    

/*!
    \brief Sets the pointers of an on_pkt_t structure.

    \param[in] pid pid of the packet
    \param[in] pkt_bytes The array holding the packet bytes
    \param[out] pkt The on_pkt_t structure to fill

    \return TRUE if the on_pkt structure was set up successfully.
            FALSE upon error.
*/
BOOL setup_pkt_ptr(UInt8 pid, UInt8* pkt_bytes, on_pkt_t* pkt)
{
    SInt8 len = get_encoded_payload_len(pid);
    if(len < 0)
    {
        return FALSE; // bad pid
    }
    
    if(!pkt_bytes || !pkt)
    {
        return FALSE;
    }
    
    pkt->packet_header    = &pkt_bytes[0];
    pkt->pid              = &pkt_bytes[ONE_NET_ENCODED_PID_IDX];
    pkt->enc_msg_id       = &pkt_bytes[ONE_NET_ENCODED_MSG_ID_IDX];
    pkt->enc_msg_crc      = &pkt_bytes[ONE_NET_ENCODED_MSG_CRC_IDX];
    pkt->enc_src_did      = (on_encoded_did_t*) &pkt_bytes[ON_ENCODED_SRC_DID_IDX];
    pkt->enc_dst_did      = (on_encoded_did_t*) &pkt_bytes[ONE_NET_ENCODED_DST_DID_IDX];
    pkt->enc_repeater_did = (on_encoded_did_t*) &pkt_bytes[ONE_NET_ENCODED_RPTR_DID_IDX];
    pkt->enc_nid          = (on_encoded_nid_t*) &pkt_bytes[ON_ENCODED_NID_IDX];
    pkt->payload          = &pkt_bytes[ON_PLD_IDX];
    pkt->payload_len      = (UInt8) len;
    
    #ifdef _ONE_NET_MULTI_HOP
    pkt->enc_hops_field = pkt->payload + pkt->payload_len;
    #endif
    
    return TRUE;
}

    

/*!
    \brief Initializes ONE-NET.

    \return void
*/
void one_net_init(void)
{
    one_net_set_channel(on_base_param->channel);
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
BOOL one_net(on_txn_t ** txn)
{
    switch(on_state)
    {
        case ON_LISTEN_FOR_DATA:
        {
            // we are listinging for data.  Make sure we have nothing
            // pending
            if(*txn == NULL && single_txn.priority == ONE_NET_NO_PRIORITY
              && single_msg_ptr == NULL)
            {
                tick_t next_pop_time; // we don't sleep, so we won't use
                                       // this.  But the function requires it
                                       // as a parameter.
                
                // nothing is pending.  See if we have anything ready to go
                int index = single_data_queue_ready_to_send(&next_pop_time);
                if(index >= 0)
                {
                    if(pop_queue_element(&single_msg, single_data_raw_pld, index))
                    {
                        // we have a message ready to send and we've popped it.
                        // Now let's get things ready to send.
                        
                        
                        if(!setup_pkt_ptr(single_msg.pid, single_pkt,
                          &data_pkt_ptrs))
                        {
                            // an error of some sort occurred.  We likely have
                            // a bad pid.  Unrecoverable.  Just abort.
                            return TRUE; // no outstanding transaction
                        }
                        

                        // we'll fill in what we can here.
                        single_txn.priority = single_msg.priority;
                        single_txn.retry = 0;
                        single_txn.next_txn_timer = ONT_SINGLE_TIMER;
                        single_txn.data_len = get_encoded_payload_len(single_msg.pid);
                        single_msg_ptr = &single_msg;
                        *txn = &single_txn;
                        
                        // now fill in as much of the packet as we can.
                        
                        // the preamble is always the same
                        one_net_memmove(data_pkt_ptrs.packet_header,
                          HEADER, sizeof(HEADER));
                          
                        // the repeater did is always us
                        one_net_memmove(data_pkt_ptrs.enc_repeater_did,
                          &on_base_param->sid[ON_ENCODED_NID_LEN],
                          ON_ENCODED_DID_LEN);
                          
                        // fill in the destination
                        one_net_memmove(*(data_pkt_ptrs.enc_dst_did),
                          single_msg.dst_did, ON_ENCODED_DID_LEN);
                          
                        // fill in the NID
                        one_net_memmove(*(data_pkt_ptrs.enc_nid),
                          on_base_param->sid, ON_ENCODED_NID_LEN);
                        
                        // fill in the PID
                        *(data_pkt_ptrs.pid) = single_msg.pid;
                        
                        // that's about all we can fill in.  one_net_client or
                        // one_net_master will have to complete.  Set on_state
                        // to ON_SEND_PKT, but keep the priority as
                        // ONE_NET_NO_PRIORITY so that one_net_client or
                        // one_net_master know that 1) they need to send a
                        // single packet, but 2) more needs to be done before
                        // the packet can be sent.
                        on_state = ON_SEND_SINGLE_DATA_PKT;
                        return FALSE; // transaction is not complete
                    }
                }
            }
        } // case ON_LISTEN_FOR_DATA //
    }
    return TRUE;
} // one_net //

//! @} ONE-NET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_pri_func
//! \ingroup ONE-NET
//! @{



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



//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET

