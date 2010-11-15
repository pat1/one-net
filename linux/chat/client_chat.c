//
// Copyright 2006, Threshold Corporation, All rights reserved.
//

/*!
    \file chat.c
    \brief Chat program for demo of ONE-NET communications

    This file contains the implementation of the chat program, used for
    ONE-NET masters and clients to communicate with one another
*/

#include <string.h>

#include "chat.h"
#include "client_port_specific.h"
#include "debug.h"

//==============================================================================
//                              CONSTANTS 

//                              CONSTANTS END
//==============================================================================


//==============================================================================
//                              TYPEDEFS 

//                              TYPEDEFS END
//==============================================================================

//==============================================================================
//                          PUBLIC VARIABLES

//                          PUBLIC VARIABLES END
//==============================================================================


//==============================================================================
//                          PRIVATE VARIABLES

//! The index into the block transaction to be sent
static UInt16 block_idx = 0;

//! Variable to tell if a block transaction is in progress.
static BOOL block_in_progress = FALSE;

//! The index into the stream transaction to be sent
static UInt16 stream_idx = 0;

//! Variable that indicates if a stream transaction is in progress.
static BOOL stream_in_progress = FALSE;

//! The unit currently used to send data
static UInt8 sending_unit = 0;

//                          PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS 

static void process_payload(const UInt8 * payload_buffer, 
  const UInt16 payload_buffer_len);

static void process_cmd(void);

//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION 

/*!
    \brief main body function for client_chat program

    Main function for the chat program, takes in input from user, sends it
    through ONE-NET packets to destination, as well as displaying information
    received from ONE-NET packets. 

    \param void
    \return void
*/
void client_chat(void)
{
    fd_set fds_r, fds_w, fds_e;
    struct timeval timeout = {0, 0};

    tick_t sleep_time = 0;

    on_base_param_t base_param;
    on_master_t master;
    on_peer_t peer = 
    {
        #ifdef _ONE_NET_MULTI_HOP
            {
                {{0xB4, 0xB4}, 0, 0, 0, FALSE}, {{0xB4, 0xB4}, 0, 0, 0, FALSE},
                  {{0xB4, 0xB4}, 0, 0, 0, FALSE},
                  {{0xB4, 0xB4}, 0, 0, 0, FALSE},
            },
        #else // ifdef _ONE_NET_MULTI_HOP //
            {
                {{0xB4, 0xB4}, 0, 0}, {{0xB4, 0xB4}, 0, 0},
                  {{0xB4, 0xB4}, 0, 0}, {{0xB4, 0xB4}, 0, 0},
                  {{0xB4, 0xB4}, 0, 0}, {{0xB4, 0xB4}, 0, 0},
                  {{0xB4, 0xB4}, 0, 0}, {{0xB4, 0xB4}, 0, 0}
            },
        #endif // else _ONE_NET_MULTI_HOP is not defined //

        {
            {{0xFFFF, ONE_NET_DEV_UNIT}, {0xFFFF, ONE_NET_DEV_UNIT},
              {0xFFFF, ONE_NET_DEV_UNIT}, {0xFFFF, ONE_NET_DEV_UNIT}},
        }
    };

    //set up program name/version (just in case)
    set_program_name("CLIENT");

    if(mode_value == AUTO)
    {
        // initialize the MASTER parameters
        one_net_memmove(&(base_param.sid), ENCODED_MASTER_SID,
          ON_ENCODED_NID_LEN);
        on_encode(&(base_param.sid[ON_ENCODED_NID_LEN]),
          RAW_AUTO_CLIENT_DID[node_select_val - CLIENT1], ON_ENCODED_DID_LEN);
        base_param.channel = DEFAULT_CHANNEL;
        base_param.data_rate = DATA_RATE;
        one_net_memmove(&(base_param.current_key), EVAL_KEY,
          sizeof(base_param.current_key));
        base_param.single_block_encrypt = EVAL_SINGLE_BLOCK_ENCRYPTION;
        base_param.stream_encrypt = EVAL_STREAM_ENCRYPTION;
        base_param.fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
        base_param.fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;

        one_net_memmove(master.device.did,
          &(ENCODED_MASTER_SID[ON_ENCODED_NID_LEN]), ON_ENCODED_DID_LEN);
        master.device.expected_nonce = 0;
        master.device.last_nonce = 0;
        master.keep_alive_interval = 300000;
        master.device.send_nonce = 0;
        master.settings.master_data_rate = DATA_RATE;
        master.settings.flags = 0x80;

        if(node_select_val == CLIENT1)
        {
            one_net_memmove(peer.dev[0].did, ON_ENCODED_BROADCAST_DID,
              sizeof(peer.dev[0].did));
        } // if CLIENT1 //
        else if(node_select_val == CLIENT2)
        {
            one_net_memmove(peer.dev[0].did, ON_ENCODED_BROADCAST_DID,
              sizeof(peer.dev[0].did));
        } // else if CLIENT2 //
        else
        {
            on_encode(peer.dev[0].did, RAW_AUTO_CLIENT_DID[CLIENT2 - CLIENT1],
              ON_ENCODED_DID_LEN);
        } // else CLIENT3 //

        peer.dev[0].nonce = 0;
        peer.dev[0].data_rate = DATA_RATE;
    } // if AUTO mode //

    one_net_client_init(&base_param, &master, &peer);

    // initial prompt (depends on mode)
    switch(node_select_val)
    {
        case CLIENT1:
        {
            printf("chat C1> ");
            set_program_version("(1)");
            break;
        } // CLIENT1 //
        
        case CLIENT2:
        {
            printf("chat C2> ");
            set_program_version("(2)");
            break;
        } // CLIENT2 //
        
        default:
        {
            printf("chat C3> ");
            set_program_version("(3)");
            break;
        } // default/CLIENT3
        
    } // switch mode_select_value
    fflush(NULL);

    // main loop 
    while(1)
    {   
        UInt8 payload_buffer[PAYLOAD_SIZE] = {0x00};
        
        payload_buffer[0] = CHAT;
        
        //check for serial input
        
        //if we have something to send, send it, else wait
        FD_SET(STDIN, &fds_r);
        select(STDIN+1, &fds_r, &fds_w, &fds_e, &timeout);
        if(sleep_time > 10 && FD_ISSET(STDIN, &fds_r))
        {
            read(STDIN, &payload_buffer[1], MAX_PAYLOAD_CHARS);

            if(payload_buffer[1] == EXIT_CHAR)
            {
                if(detach_shm() != 0)
                {
                    printf("ERROR: detach_shm failed!\n");
                    fflush(NULL);
                } // if detach_shm failed //
                return;
            } // if exit char //
            else if(payload_buffer[1] == CMD_CHAR)
            {
                process_cmd();
                continue;
            } // if command character //

            one_net_client_send_singleS(payload_buffer, PAYLOAD_SIZE,
              ONE_NET_HIGH_PRIORITY, 0, sending_unit, ONE_NET_DEV_UNIT);

            if(payload_buffer[1] == 's' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 'b')
            {
                if(!block_in_progress && one_net_client_block_stream_request(
                  ON_BLOCK, TRUE, 0, strlen(BLOCK_TEST_DATA),
                  ONE_NET_LOW_PRIORITY, 0, 0) == ONS_SUCCESS)
                {
                    printf("########## SEND LOW BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
            } // if CLIENT request to send a low priority block transaction //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 'b')
            {
                if(!block_in_progress && one_net_client_block_stream_request(
                  ON_BLOCK, TRUE, 0, strlen(BLOCK_TEST_DATA),
                  ONE_NET_HIGH_PRIORITY, 0, sending_unit) == ONS_SUCCESS)
                {
                    printf("########## SEND HIGH BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to send a high priority block txn //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 'b')
            {
                if(!block_in_progress && one_net_client_block_stream_request(
                  ON_BLOCK, FALSE, 0, strlen(BLOCK_TEST_DATA),
                  ONE_NET_LOW_PRIORITY, 0, sending_unit) == ONS_SUCCESS)
                {
                    printf("########## RECV LOW BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to receive a low priority block //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 'b')
            {
                if(!block_in_progress && one_net_client_block_stream_request(
                  ON_BLOCK, FALSE, 0, strlen(BLOCK_TEST_DATA),
                  ONE_NET_HIGH_PRIORITY, 0, sending_unit) == ONS_SUCCESS)
                {
                    printf("########## RECV HIGH BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to receive a high priority block //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 's')
            {
                if(!stream_in_progress && one_net_client_block_stream_request(
                  ON_STREAM, TRUE, 0, 0, ONE_NET_LOW_PRIORITY, 0, sending_unit)
                  == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ SEND LOW STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to send a low priority stream //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 's')
            {
                if(!stream_in_progress && one_net_client_block_stream_request(
                  ON_STREAM, TRUE, 0, 0, ONE_NET_HIGH_PRIORITY, 0, sending_unit)
                  == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ SEND LOW STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to send a high priority stream //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 's')
            {
                if(!stream_in_progress && one_net_client_block_stream_request(
                  ON_STREAM, FALSE, 0, 0, ONE_NET_LOW_PRIORITY, 0, sending_unit)
                  == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ RECV LOW STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to receive a low priority stream //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 's')
            {
                if(!stream_in_progress && one_net_client_block_stream_request(
                  ON_STREAM, FALSE, 0, 0, ONE_NET_HIGH_PRIORITY, 0,
                  sending_unit) == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ RECV HIGH STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if CLIENT request to receive a high priority stream //
            else if(stream_in_progress && payload_buffer[1] == 'e'
              && payload_buffer[2] == 's')
            {
                printf("$$$$$$$$$$ END STREAM CMD $$$$$$$$$$\n");
                client_end_stream();
                stream_in_progress = FALSE;
            } // if ending the stream //
        } //if we have data to send

        sleep_time = one_net_client();
    } // infinite loop //
} // client_chat //


BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    // handle incoming message, skip CHAT part of the payload
    process_payload(&(RX_PLD[1]), RX_PLD_LEN);

    return TRUE;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 * const DATA, const one_net_raw_did_t * const DST)
{
} // one_net_client_single_txn_status //


BOOL one_net_client_txn_requested(const UInt8 TYPE, const BOOL SEND,
  const UInt16 DATA_TYPE, const UInt16 DATA_LEN,
  const one_net_raw_did_t * const DID)
{
    if(TYPE == ON_BLOCK && !block_in_progress && DATA_TYPE == 0)
    {
        block_idx = 0;
        block_in_progress = TRUE;

        if(SEND)
        {
            printf("############### SEND BLOCK REQ RECEIVED ##############\n");
        } // if request to send the block transaction //
        else
        {
            printf("############### RECV BLOCK REQ RECEIVED ##############\n");
        } // else request to receive the block transaction //

        return TRUE;
    } // if accepting the block txn //

    if(TYPE == ON_STREAM && !stream_in_progress && DATA_TYPE == 0)
    {
        stream_idx = 0;
        stream_in_progress = TRUE;

        if(SEND)
        {
            printf("$$$$$$$$$$$$$$$ SEND STREAM REQ RXED $$$$$$$$$$$$$$$\n");
        } // if request to send the stream transaction //
        else
        {
            printf("$$$$$$$$$$$$$$$ RECV STREAM REQ RXED $$$$$$$$$$$$$$$\n");
        } // else request to receive the stream transaction //

        return TRUE;
    } // if accepting the stream transaction //

    return FALSE;
} // one_net_client_txn_requested //


BOOL one_net_client_handle_block_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    BOOL rv = FALSE;

    if(block_in_progress)
    {
        process_payload(PLD, LEN);
        rv = TRUE;
    } // if the block is in progress //

    return rv;
} // client_handle_block //


void one_net_client_block_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
    printf("############### BLOCK TXN STATUS START ###############\n");
    printf("status of block txn to %02X%02X is %02X\n", (*DID)[0], (*DID)[1],
      STATUS);
    printf("############### BLOCK TXN STATUS END ###############\n");
    block_in_progress = FALSE;
    block_idx = 0;
} // one_net_client_block_txn_status //


BOOL one_net_client_handle_stream_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    BOOL rv = FALSE;

    if(stream_in_progress)
    {
        process_payload(PLD, LEN);
        rv = TRUE;
    } // if the stream is in progress //

    return rv;
} // one_net_client_handle_stream_pkt //


void one_net_client_stream_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
    printf("$$$$$$$$$$$$$$$ STREAM TXN STATUS START $$$$$$$$$$$$$$$\n");
    printf("status of stream txn to %02X%02X is %02X\n", (*DID)[0], (*DID)[1],
      STATUS);
    printf("$$$$$$$$$$$$$$$ STREAM TXN STATUS END $$$$$$$$$$$$$$$\n");
    stream_in_progress = FALSE;
    stream_idx = 0;
} // one_net_client_stream_txn_status //


const UInt8 * one_net_client_next_payload(const UInt8 TYPE, UInt16 * len,
  const one_net_raw_did_t * const DST)
{
    const UInt8 * PTR = 0;

    print_debug("one_net_client_next_payload\n");

    if((TYPE != ON_BLOCK && TYPE != ON_STREAM) || !len || !DST)
    {
        return 0;
    } // parameter check //

    // not handling block
    if(TYPE == ON_BLOCK)
    {
        if(block_in_progress)
        {
            *len = strlen(BLOCK_TEST_DATA) - block_idx;

            if(*len > ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
            {
                *len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
            } // if more data is left than will fit in a block packet //

            PTR = (UInt8 *)&(BLOCK_TEST_DATA[block_idx]);
            block_idx += *len;
        } // if the block transaction is in progress //
    } // block //
    else if(TYPE == ON_STREAM)
    {
        if(stream_in_progress)
        {
            *len = strlen(BLOCK_TEST_DATA) - stream_idx;
            if(*len > ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
            {
                *len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
            } // if more data is left than will fit in a block packet //
            else if(*len < ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
            {
                // Include the NULL termination that should be implicitly
                // stored with the string
                (*len)++;
            } // else if less data is being sent //

            PTR = (UInt8 *)&(BLOCK_TEST_DATA[stream_idx]);
            stream_idx += *len;

            if(stream_idx >= strlen(BLOCK_TEST_DATA))
            {
                // wrap around and start at the beginning next time
                stream_idx = 0;
            } // if we've sent all the test data //
        } // if the stream transaction is in progress //
    } // stream //

    return PTR;
} // one_net_client_next_payload //


void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
} // one_net_client_save_settings //

//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================


//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION 

/*!
    \brief Process an incomming message

    Handles an incoming message.  If Master, just add data (if any) to
    str_buffer until it's time to print it out

    \param[in] payload_buffer: buffer holding payload to process
    \param[in] payload_buffer_len: length of buffer
    \return void
*/
static void process_payload(const UInt8 * payload_buffer, 
  const UInt16 payload_buffer_len)
{
    char output[payload_buffer_len + 1];

    one_net_memmove(output, payload_buffer, payload_buffer_len);
    output[payload_buffer_len] = 0;
    printf("%s", output);
    fflush(NULL);
} // process_payload //


/*!
    \brief Process a command entered in the text window.

    \param void

    \return void
*/
static void process_cmd(void)
{
    const UInt8 CMD_SIZE = 3;

    char cmd[CMD_SIZE];

    if(!getlstr(cmd, sizeof(cmd)))
    {
        return;
    } // if getting the string failed //

    if(!strncasecmp(cmd, "SU", CMD_SIZE))
    {
        unsigned int unit;

        printf("Enter sending unit\n");
        scanf("%u", &unit);

        if(unit >= ONE_NET_NUM_UNITS)
        {
            printf("The maximum number of units is %u (range 0 - %u)\n",
              ONE_NET_NUM_UNITS, ONE_NET_NUM_UNITS - 1);
        } // if the unit entered is not valid //
        else
        {
            sending_unit = (UInt8)unit;
        } // else the unit entered is valid //
    } // if (un)assign peer cmd //
} // process_cmd //

//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

