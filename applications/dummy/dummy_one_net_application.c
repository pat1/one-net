#include "config_options.h"
#include "one_net.h"
#include "one_net_prand.h"
#include "tick.h"
#include "one_net_port_specific.h"

/*
These are "dummy" functions.  Copy / paste / change / use them as a skeleton 
for your application code.
June 3, 2012 -- Currently these functions have not been thoroughly tested.
*/


#ifndef _ONE_NET_SIMPLE_CLIENT
/*!
    \brief Allows the application code to override whether a nack reason is fatal

    If desired, the application code can change the is_fatal parameter.

    
    \param[in] nack_reason
    \param[in/out] is_fatal Whether ONE-NET has determined a NACK Reason to be fatal.  To override
                   ONE-NET's decision, change the is_fatal parameter.  Otherwise, do nothing
*/
void one_net_adjust_fatal_nack(on_nack_rsn_t nack_reason, BOOL* is_fatal)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(!(*is_fatal) && nack_reason == ON_NACK_RSN_MIN_FATAL)
    {
        *is_fatal = TRUE;
    }

    #endif
}


void one_net_single_msg_loaded(on_txn_t** txn, on_single_data_queue_t* msg)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(txn && ((UInt8*) txn == (UInt8*) msg))
    {
        // will never get here, so it doesn't matter what we do.
        *txn = NULL;
    }
    #endif
}
#endif


#ifdef _ONE_NET_MULTI_HOP
on_message_status_t one_net_adjust_hops(const on_raw_did_t* const raw_dst,
  UInt8* const max_hops)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(raw_dst && ((UInt8*) raw_dst == (UInt8*) max_hops))
    {
        // will never get here, so it doesn't matter what we do.
        return ON_MSG_INTERNAL_ERR;
    }
    #endif
    return ON_MSG_DEFAULT_BHVR;
}
#endif


#ifdef _DATA_RATE_CHANNEL
void one_net_data_rate_channel_changed(UInt8 new_channel, UInt8 new_data_rate)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(new_channel == 0xFF && new_data_rate == 0xFF)
    {
        // will never get here, so it doesn't matter what we do.
        one_net_init();
    }
    #endif
}
#endif


#ifndef _ONE_NET_SIMPLE_CLIENT
void one_net_adjust_recipient_list(const on_single_data_queue_t* const msg,
  on_recipient_list_t** recipient_send_list)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(msg && ((UInt8*) msg == (UInt8*) recipient_send_list))
    {
        // will never get here, so it doesn't matter what we do.
        one_net_init();
    }
    #endif
}
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
on_message_status_t one_net_block_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(!bs_msg && !buffer && !ack_nack)
    {
        return ON_NACK_RSN_INTERNAL_ERR;
    }
    #endif

    return ON_MSG_CONTINUE;
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
on_message_status_t one_net_stream_get_next_payload(block_stream_msg_t* bs_msg,
  UInt8* buffer, on_ack_nack_t* ack_nack)
{
    #ifdef _COMPILE_WO_WARNINGS
    // mess around with the variables doing trivial things to avoid unused
    // variable warnings.
    if(!bs_msg && !buffer && !ack_nack)
    {
        return ON_NACK_RSN_INTERNAL_ERR;
    }
    #endif

    return ON_MSG_CONTINUE;
}
#endif


#ifdef _DATA_RATE_CHANNEL
SInt8 one_net_get_alternate_channel(void)
{
    while(1)
    {
        SInt8 new_channel = (SInt8) one_net_prand(get_tick_count(),
          ONE_NET_MAX_CHANNEL);
        if((UInt8) new_channel != on_base_param->channel)
        {
            return new_channel;
        }
    }
}
#endif


#ifdef _BLOCK_MESSAGES_ENABLED
void one_net_block_stream_transfer_requested(const block_stream_msg_t* const
  bs_msg, on_ack_nack_t* ack_nack)
{
    #ifdef _STREAM_MESSAGES_ENABLED
    if(get_bs_transfer_type(bs_msg->flags) == ON_STREAM_TRANSFER)
    {
        return;
    }
    #endif
    if(!bs_msg->dst)
    {
        if(bs_msg->bs.block.chunk_size > DEFAULT_BS_CHUNK_SIZE)
        {
            // make sure we can handle the chunk size.
            ack_nack->nack_reason = ON_NACK_RSN_INVALID_CHUNK_SIZE;
            ack_nack->handle = ON_NACK_VALUE;
            ack_nack->payload->nack_value = DEFAULT_BS_CHUNK_SIZE;
            return;
        }
    }
}
#endif
