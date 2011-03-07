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
#include "debug.h"
#include "master_port_specific.h"
#include "one_net.h"
#include "one_net_master.h"
#include "one_net_client.h"
#include "one_net_status_codes.h"
#include "one_net_prand.h"
#include "debug.h"

#define M_CHAT_SINGLE   0
#define M_CHAT_STREAM   M_CHAT_SINGLE + 1

#define M_CHAT          M_CHAT_SINGLE

//==============================================================================
//								CONSTANTS 

enum
{
    //! The initial value for the CLIENT DIDs.  This value is a copy of
    //! ONE_NET_INITIAL_CLIENT_DID found in one_net_master.c
    INIT_CLIENT_DID = 0x0020,

    //! Max size of the  buffer holding pending stream data.
    MAX_STREAM_DATA_LEN = 256
};

const one_net_raw_did_t TEST_BLOCK_DID = {0x00, 0x20};

//								CONSTANTS END
//==============================================================================


//==============================================================================
//								TYPEDEFS 

/*!
    \brief Structure definition to hold info on the chat streams to each slave

    If both count is 0, the buffer is empty
*/
typedef struct
{
    UInt8 count;                    //!< Number of bytes in the buffer
    UInt8 idx;                      //!< Idx of the next byte to send
    //! The stream data waiting to be sent
    UInt8 data[MAX_STREAM_DATA_LEN];
} chat_stream_t;

//								TYPEDEFS END
//==============================================================================


//==============================================================================
//							PRIVATE VARIABLES

static on_base_param_t base_param;

//! The list of connected CLIENTS, and their information.
static client_t client_list[ONE_NET_MASTER_MAX_CLIENTS];

//! The number of clients currently attached.
static UInt16 client_count = 0;

#if M_CHAT == M_CHAT_STREAM
    static chat_stream_t client_stream[NUM_AUTO_CLIENTS];
#endif

//! The index into the block transaction to be sent
static UInt16 block_idx = 0;

//! Variable to tell if a block transaction is in progress.
static BOOL block_in_progress = FALSE;

//! The index into the stream transaction to be sent
static UInt16 stream_idx = 0;

//! Varialbe to tell if a stream transaction is in progress.
static BOOL stream_in_progress = FALSE;

//							PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//						PRIVATE FUNCTION DECLARATIONS

#if 0
    static BOOL addr_equal(const one_net_raw_did_t * const LHS,
      const one_net_raw_did_t * const RHS);
#endif

static void process_payload(const one_net_raw_did_t * const RX_ADDR,
  const UInt8 * payload_buffer, const UInt16 payload_buffer_len);

static void process_cmd(void);

//						PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//						PUBLIC FUNCTION IMPLEMENTATION 

/*!
    \brief main body function for chat program

    Main function for the chat program, takes in input from user, sends it
    through ONE-NET packets to destination, as well as displaying information
    received from ONE-NET packets.  It takes a single command line argument,
    specifying if it is a master or client doing the chatting.

    \param type[in] type for this board (Master/Slave1,2,3)
    \return void
*/
void master_chat(void)
{
    UInt16 i;
    fd_set fds_r, fds_w, fds_e;
    struct timeval timeout = {0, 0};

    on_master_param_t master_param;

    #if M_CHAT == M_CHAT_STREAM
        UInt8 bytes_read;
    #endif
   
    //set up program name/version (just in case)
    set_program_name("MASTER");
    set_program_version("");

    for(client_count = 0; client_count < NUM_AUTO_CLIENTS; client_count++)
    {
        client_list[client_count].expected_nonce = 0;
        client_list[client_count].last_nonce = 0;
        client_list[client_count].send_nonce = 0;
        client_list[client_count].data_rate = ONE_NET_DATA_RATE_38_4;
        client_list[client_count].max_data_rate = ONE_NET_MAX_DATA_RATE;
        client_list[client_count].features = 0;
        client_list[client_count].use_current_key = TRUE;
    } // loop to initialize the CLIENT parameters //

    client_list[0].features = 0x74;
    client_list[1].features = 0x34;

    if(mode_value == AUTO)
    {
        memmove(base_param.sid, ENCODED_MASTER_SID, sizeof(base_param.sid));
        base_param.channel = DEFAULT_CHANNEL;
        base_param.data_rate = ONE_NET_DATA_RATE_38_4;
        memmove(base_param.current_key, EVAL_KEY,
          sizeof(base_param.current_key));
        base_param.single_block_encrypt = DEFAULT_SINGLE_BLOCK_ENCRYPT;
        base_param.stream_encrypt = EVAL_STREAM_ENCRYPTION;
        base_param.fragment_delay_low = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY;
        base_param.fragment_delay_high = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY;
        master_param.next_client_did = ONE_NET_INITIAL_CLIENT_DID
          + (NUM_AUTO_CLIENTS << 4);
    } // if auto mode //
    else
    {
        printf("Need to implement serial mode for the MASTER.\n");
        master_param.next_client_did = ONE_NET_INITIAL_CLIENT_DID;
        client_count = 0;
        return;
    } // else serial mode //

    one_net_master_init(&base_param, &master_param, client_list, client_count,
      0);

    printf("chat M> ");
    fflush(NULL);

    // main loop 
    while(1)
    {
		UInt8 payload_buffer[PAYLOAD_SIZE] = {0x00};
        
        // do master loop
        one_net_master();

		// formulate return packet
		payload_buffer[0] = CHAT;

		// get terminal input 
        FD_SET(STDIN, &fds_r);
        select(STDIN + 1, &fds_r, &fds_w, &fds_e, &timeout);      
		if(FD_ISSET(STDIN, &fds_r))
		{
            one_net_raw_did_t dst_did;

            #if M_CHAT == M_CHAT_SINGLE   
                // read 1 less so the string is NULL terminated
                read(STDIN, &payload_buffer[1], MAX_PAYLOAD_CHARS);
            #elif M_CHAT == M_CHAT_STREAM
                bytes_read = read(STDIN, &payload_buffer[0],
                  MAX_PAYLOAD_CHARS + 2);
            #else
                #error Unknown M_CHAT value
            #endif
            if(payload_buffer[1] == EXIT_CHAR)
            {
                if(detach_shm() != 0)
                {
                    printf("ERROR: SHM not detached properly\n");
                    fflush(NULL);
                }
                return;
            } // if exit character //
            else if(payload_buffer[1] == CMD_CHAR)
            {
                process_cmd();
                continue;
            } // if command character //
            else if(mode_value == SERIAL && payload_buffer[1] == '1')
			{
                printf("Need to implement adding device in serial mode\n");
				continue; //don't send this on to clients
			} //if adding client 1 //
            else if(mode_value == SERIAL && payload_buffer[1] == '2')
			{
                printf("Need to implement adding device in serial mode\n");
				continue; //don't send this on to clients
			} //if adding client 2 //
			else if(mode_value == SERIAL && payload_buffer[1] == '3')
			{
                printf("Need to implement adding device in serial mode\n");
				continue; //don't send this on to clients
			} //if adding client 3 //
            
            // send the data to the CLIENTS
            for(i = 0; i < client_count; i++)
            {
                // only the top 12 bits are used for an address
                one_net_int16_to_byte_stream(
                  (i << 4) + ONE_NET_INITIAL_CLIENT_DID, dst_did);

                #if M_CHAT == M_CHAT_SINGLE   
                    one_net_status_t rv;
                    // +1 for the payload type (id, data, & null)
                    while((rv = one_net_master_send_single(payload_buffer, 
                      PAYLOAD_SIZE, ONE_NET_HIGH_PRIORITY,
                      (const one_net_raw_did_t * const)&dst_did,
                      ONE_NET_DEV_UNIT, ONE_NET_DEV_UNIT)) == ONS_RSRC_FULL)
                    {
                        one_net_master();
                    } // while buffer is full //
                #elif M_CHAT == M_CHAT_STREAM
                    // TBD MAY HAVE TO SHIFT THE DATA DOWN SINCE WE ARE NOT
                    // DOING A CIRCULAR BUFFER
                    if(client_stream[i].count + bytes_read
                      > MAX_STREAM_DATA_LEN)
                    {
                        printf("stream for client %u overflowed it's"
                          " buffer.  Clearing the data in the stream",
                          i);

                        client_stream[i].count = 0;
                        client_stream[i].idx = 0;
                    }

                    memmove(&(client_stream[i].data[client_stream[i].count]),
                      payload_buffer, bytes_read);
                    client_stream[i].count += bytes_read;
                #else
                    #error Unknown M_CHAT value
                #endif
			} // for all clients //

            // only send/receive a block from C1
            if(payload_buffer[1] == 's' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 'b')
            {
                if(one_net_master_block_stream_request(ON_BLOCK, TRUE, 0,
                  strlen(BLOCK_TEST_DATA), ONE_NET_LOW_PRIORITY,
                  &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("########## SEND LOW BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
                else
                {
                    printf("!!!!!!!!!! SEND LOW BLOCK CMD FAIL !!!!!!!!!!n");
                } // else queueing the request was not successful //
            } // if MASTER is requesting to send a low priority block txn //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 'b')
            {
                if(one_net_master_block_stream_request(ON_BLOCK, TRUE, 0,
                  strlen(BLOCK_TEST_DATA), ONE_NET_HIGH_PRIORITY,
                  &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("########## SEND HIGH BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
                else
                {
                    printf("!!!!!!!!!! SEND HIGH BLOCK CMD FAIL !!!!!!!!!!n");
                } // else queueing the request was not successful //
            } // else if MASTER requesting to send a high priority block txn //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 'b')
            {
                if(one_net_master_block_stream_request(ON_BLOCK, FALSE, 0,
                  strlen(BLOCK_TEST_DATA), ONE_NET_LOW_PRIORITY,
                  &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("########## RECV LOW BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
                else
                {
                    printf("!!!!!!!!!! RECV LOW BLOCK CMD FAIL !!!!!!!!!!n");
                } // else queueing the request was not successful //
            } // else if MASTER request to receive low priority block //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 'b')
            {
                if(one_net_master_block_stream_request(ON_BLOCK, FALSE, 0,
                  strlen(BLOCK_TEST_DATA), ONE_NET_HIGH_PRIORITY,
                  &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("########## RECV HIGH BLOCK CMD #########\n");
                    block_in_progress = TRUE;
                } // if queueing the request was successful //
                else
                {
                    printf("!!!!!!!!!! RECV HIGH BLOCK CMD FAIL !!!!!!!!!!n");
                } // else queueing the request was not successful //
            } // else if MASTER request to receive high priority block //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 's')
            {
                if(one_net_master_block_stream_request(ON_STREAM, TRUE, 0, 0,
                  ONE_NET_LOW_PRIORITY, &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ SEND LOW STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if MASTER requesting to send a low priority stream txn //
            else if(payload_buffer[1] == 's' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 's')
            {
                if(one_net_master_block_stream_request(ON_STREAM, TRUE, 0, 0,
                  ONE_NET_HIGH_PRIORITY, &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ SEND HIGH STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if MASTER requesting to send a high priority stream txn //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'l'
              && payload_buffer[3] == 's')
            {
                if(one_net_master_block_stream_request(ON_STREAM, FALSE, 0, 0,
                  ONE_NET_LOW_PRIORITY, &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ RECV LOW STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if MASTER requesting to receive a low priority stream //
            else if(payload_buffer[1] == 'r' && payload_buffer[2] == 'h'
              && payload_buffer[3] == 's')
            {
                if(one_net_master_block_stream_request(ON_STREAM, FALSE, 0, 0,
                  ONE_NET_HIGH_PRIORITY, &TEST_BLOCK_DID) == ONS_SUCCESS)
                {
                    printf("$$$$$$$$$$ RECV HIGH STREAM CMD $$$$$$$$$$\n");
                    stream_in_progress = TRUE;
                } // if queueing the request was successful //
            } // else if MASTER requesting to receive a high priority stream //
            else if(stream_in_progress && payload_buffer[1] == 'e'
              && payload_buffer[2] == 's')
            {
                printf("$$$$$$$$$$ END LOW STREAM CMD $$$$$$$$$$\n");
                master_end_stream(&TEST_BLOCK_DID);
                stream_in_progress = FALSE;
            } // else if MASTER is ending the stream //
		} // if typed something // 
    } // infinite loop // 
} // master_chat //


BOOL one_net_master_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    // skip chat portion
    process_payload(SRC_ADDR, &RX_PLD[1], RX_PLD_LEN);

    return TRUE;
} // one_net_master_handle_single_pkt //


void one_net_master_single_txn_status(const one_net_status_t STATUS,
  const UInt8 * const DATA, const one_net_raw_did_t * const DST)
{
} // one_net_master_single_txn_status //


BOOL one_net_master_txn_requested(const UInt8 TYPE, const BOOL SEND,
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
    else if(TYPE == ON_STREAM && !stream_in_progress && DATA_TYPE == 0)
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
    } // if accepting the stream txn //

    return FALSE;
} // one_net_master_txn_requested //


BOOL one_net_master_handle_block_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    BOOL rv = FALSE;

    if(block_in_progress)
    {
        process_payload(0, PLD, LEN);
        rv = TRUE;
    } // if the block is in progress //

    return rv;
} // one_net_master_handle_block_pkt //


void one_net_master_block_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
    printf("############### BLOCK TXN STATUS START ###############\n");
    printf("status of block txn to %02X%02X is %02X\n", (*DID)[0], (*DID)[1],
      STATUS);
    printf("############### BLOCK TXN STATUS END ###############\n");
    block_in_progress = FALSE;
    block_idx = 0;
} // one_net_master_block_txn_status //


BOOL one_net_master_handle_stream_pkt(const UInt8 * PLD, const UInt16 LEN,
  const one_net_raw_did_t * const SRC_ADDR)
{
    BOOL rv = FALSE;

    if(stream_in_progress)
    {
        process_payload(0, PLD, LEN);
        rv = TRUE;
    } // if the stream is in progress //

    return rv;
} // one_net_master_handle_stream_pkt //


void one_net_master_stream_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID)
{
    printf("$$$$$$$$$$$$$$$ STREAM TXN STATUS START $$$$$$$$$$$$$$$\n");
    printf("status of stream txn to %02X%02X is %02X\n", (*DID)[0], (*DID)[1],
      STATUS);
    printf("$$$$$$$$$$$$$$$ STREAM TXN STATUS END $$$$$$$$$$$$$$$\n");
    stream_in_progress = FALSE;
    stream_idx = 0;
} // one_net_master_stream_txn_status //


void one_net_master_invite_result(const one_net_status_t STATUS,
  const one_net_xtea_key_t KEY)
{
    switch(STATUS)
    {
        case ONS_SUCCESS:
        {
            printf("Device successfully added.\n");
            fflush(NULL);
            break;
        } // ONS_SUCCESS //
        
        case ONS_TIME_OUT:
        {
            printf("Timed out trying to add device.\n");
            fflush(NULL);
            break;
        } // ONS_TIME_OUT //
        
        case ONS_CANCELED:
        {
            printf("Canceled adding device to the network.\n");
            fflush(NULL);
            break;
        } // ONS_CANCELED //
        
        default:
        {
            printf("Unknown response in master_handle_add_client\n");
            fflush(NULL);
            break;
        } // default //
    } // switch on result //
} // one_net_master_invite_result //


void one_net_master_data_rate_result(const one_net_raw_did_t * const SENDER,
  const one_net_raw_did_t * const RECEIVER, const UInt8 DATA_RATE,
  const UInt8 ONS_SUCCESS_COUNT, const UInt8 ATTEMPTS)
{
    printf("DATA RATE TEST RESULTS:\n");
    printf("\tSENDER: %02X%02X, RECEIVER %02X%02X\n", (*SENDER)[0],
      (*SENDER)[1], (*RECEIVER)[0], (*RECEIVER)[1]);
    printf("\tdata rate %u, %u successful txns out of %u attempts\n",
      DATA_RATE, ONS_SUCCESS_COUNT, ATTEMPTS);
} // one_net_master_data_rate_result //


void one_net_master_update_result(const mac_update_t UPDATE,
  const one_net_raw_did_t * const DID, const BOOL SUCCEEDED)
{
    const char * STR = 0;

    if(!DID)
    {
        return;
    } // if the parameter is invalid //

    switch(UPDATE)
    {
        case UPDATE_DATA_RATE:
        {
            STR = "data rate";
            break;
        } // UPDATE_DATA_RATE case //

        case UPDATE_PEER_DATA_RATE:
        {
            STR = "peer data rate";
            break;
        } // UPDATE_PEER_DATA_RATE case //

        case UPDATE_ASSIGN_PEER:
        {
            STR = "peer assignment";
            break;
        } // UPDATE ASSIGN PEER case //

        case UPDATE_UNASSIGN_PEER:
        {
            STR = "peer assignment (unassigning)";
            break;
        } // UPDATE_UNASSIGN_PEER case //

        case UPDATE_REPORT_TO_MASTER:
        {
            STR = "report to MASTER flag";
            break;
        } // UPDATE_REPORT_TO_MASTER case //

        case UPDATE_LOW_FRAGMENT_DELAY:
        {
            STR = "low priority fragment delay case";
            break;
        } // UPDATE_LOW_FRAGMENT_DELAY case //

        case UPDATE_HIGH_FRAGMENT_DELAY:
        {
            STR = "high priority fragment delay case";
            break;
        } // UPDATE_HIGH_FRAGMENT_DELAY case //

        case UPDATE_KEEP_ALIVE:
        {
            STR = "keep alive interval";
            break;
        } // UPDATE_KEEP_ALIVE case //

        default:
        {
            return;
            break;
        } // default case //
    } // switch(UPDATE) //

    printf("updating %s on %02X%02X was %s\n", STR, (*DID)[0], (*DID)[1],
      SUCCEEDED ? "successful" : "unsuccessful");
} // one_net_master_update_result //


const UInt8 * one_net_master_next_payload(const UInt8 TYPE, UInt16 * len,
  const one_net_raw_did_t * const DST)
{
    UInt8 i;

    print_debug("one_net_master_next_payload\n");

    if((TYPE != ON_BLOCK && TYPE != ON_STREAM) || !len || !DST)
    {
        return 0;
    } // parameter check

    // not handling block
    if(TYPE == ON_BLOCK)
    {
        const UInt8 * PTR = 0;

        if(block_in_progress)
        {
            *len = strlen(BLOCK_TEST_DATA) - block_idx;

            if(*len > ON_RAW_BLOCK_STREAM_DATA_LEN)
            {
                *len = ON_RAW_BLOCK_STREAM_DATA_LEN;
            } // if more data is left than will fit in a block packet //

            PTR = (UInt8 *)&(BLOCK_TEST_DATA[block_idx]);
            block_idx += *len;
        } // if the block transaction is in progress //
        return PTR;
    } // if block //
    else
    {
        const UInt8 * PTR = 0;

        if(stream_in_progress)
        {
            *len = strlen(BLOCK_TEST_DATA) - block_idx;
            if(*len > ON_RAW_BLOCK_STREAM_DATA_LEN)
            {
                *len = ON_RAW_BLOCK_STREAM_DATA_LEN;
            } // if more data is left than will fit in a stream pkt //
            else if(*len < ON_RAW_BLOCK_STREAM_DATA_LEN)
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
        } // if the stream txn is in progress //

        return PTR;
    } // else a stream txn //

    // stream transaction
    for(i = 0; i < NUM_AUTO_CLIENTS; i++)
    {
        #if 0
        if(addr_equal((const one_net_raw_did_t * const)client_addrs[i],
          DST))
        {
            UInt8 idx;

            if(client_stream[i].idx == client_stream[i].count)
            {
                client_stream[i].idx = 0;
                client_stream[i].count = 0;
                client_stream[i].data[0] = 0;
                *len = 1;
                return client_stream[i].data;
            } // no data to send, so send the NULL byte

            // add 1 for NULL termination
            *len = client_stream[i].count - client_stream[i].idx + 1;
            if(*len > ON_MAX_RAW_PLD_LEN)
            {
                *len = ON_MAX_RAW_PLD_LEN;
            } // verify it will fit in a block
            else
            {
                // don't increment count.  Overwrite the NULL termination
                // next time data is added
                client_stream[i].data[client_stream[i].count] = 0;
            } // NULL terminate data that won't fit in a stream

            idx = client_stream[i].idx;
            client_stream[i].idx += *len;

            if(client_stream[i].idx >= client_stream[i].count)
            {
                client_stream[i].idx = 0;
                client_stream[i].count = 0;
            } // sent all the data we had

            return &(client_stream[i].data[idx]);
        } // found the client to send to
        #endif
    } // find the client to send to

    return 0;
} // one_net_master_next_payload //

//						PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================


//==============================================================================
//						PRIVATE FUNCTION IMPLEMENTATION 

/*!
    \brief Process an incomming message

    Handles an incoming message.  If Master, just add data (if any) to
    str_buffer until it's time to print it out

    \param[in] payload_buffer: buffer holding payload to process
	\param[in] payload_buffer_len: length of buffer
    \return void
*/
static void process_payload(const one_net_raw_did_t * const RX_ADDR,
  const UInt8 * payload_buffer, const UInt16 payload_buffer_len)
{
    char output[payload_buffer_len + 1];

    memmove(output, payload_buffer, payload_buffer_len);
    output[payload_buffer_len] = 0;
    printf("%s", output);
    fflush(NULL);

    #if 0
        // if RX_ADDR is valid, the data should be forwarded on to the other
        // clients (single transactions).  If RX_ADDR == 0, then the data
        // should not be forwarded on (block/stream transactions).
        if(RX_ADDR)
        {
            one_net_raw_did_t raw_did;
            UInt8 i;

            for(i = 0; i < client_count; i++)
            {
                // only the top 12 bits are used for an address
                one_net_int16_to_byte_stream(i << 4, raw_did);

                if(addr_equal((const one_net_raw_did_t * const)&raw_did,
                  RX_ADDR) == FALSE)
                {
                    one_net_master_send_single(payload_buffer,
                      payload_buffer_len, ONE_NET_LOW_PRIORITY,
                      (const one_net_raw_did_t * const)&raw_did);
                }
            } //for all clients //
        } // if the rx_addr was passed in //
    #endif
} // process_payload //


#if 0
    /*!
        \brief compare 2 ONE-NET raw addresses
        
        The RHS MAY have the direction bit set.
        
        \param[in] LHS The left hand side of the compare operation
        \param[in] RHS The right hand side of the compare operation
        \return TRUE if the addresses match (accounting for direction bit in RHS).
                FALSE if the addresses do not match
    */
    static BOOL addr_equal(const one_net_raw_did_t * const LHS,
      const one_net_raw_did_t * const RHS)
    {
        UInt8 i;
        
        for(i = 0; i < ONE_NET_RAW_DID_LEN; i++)
        {
            if((*LHS)[i] != (*RHS)[i])
            {
                return FALSE;
            }
        } // for all clients //
        
        return TRUE;
    } // addr_equal //
#endif


/*!
    \brief Process a command entered in the text window.

    \param void

    \return void
*/
static void process_cmd(void)
{
    const UInt8 CMD_SIZE = 3;

    one_net_status_t status;
    char cmd[CMD_SIZE];

    if(!getlstr(cmd, sizeof(cmd)))
    {
        return;
    } // if getting the string failed //

    if(!strncasecmp(cmd, "AP", CMD_SIZE) || !strncasecmp(cmd, "UP", CMD_SIZE))
    {
        one_net_raw_did_t peer_did, dst_did;

        unsigned int peer_int, peer_unit, dst_int, dst_unit;

        BOOL assign = strncasecmp(cmd, "AP", CMD_SIZE) == 0;

        printf("Enter peer did, peer unit, dst did, dst unit\n");
        scanf("%u %u %u %u", &peer_int, &peer_unit, &dst_int, &dst_unit);

        one_net_int16_to_byte_stream((UInt16)peer_int << 4, peer_did);
        one_net_int16_to_byte_stream((UInt16)dst_int << 4, dst_did);
        if((status = one_net_master_peer_assignment(assign,
          (const one_net_raw_did_t * const)&peer_did, peer_unit,
          (const one_net_raw_did_t * const)&dst_did, dst_unit)) != ONS_SUCCESS)
        {
            printf("Calling master_peer_assignment returned %02X\n", status);
        } // if peer assignment was not successful //
    } // if (un)assign peer cmd //
    else if(!strncasecmp(cmd, "UM", CMD_SIZE))
    {
        one_net_raw_did_t dst_did;
        unsigned int update, dst_int;

        printf("Enter dst did, and 0 to not update the MASTER"
          " or 1 to update the MASTER\n");
        scanf("%u %u", &dst_int, &update);

        one_net_int16_to_byte_stream((UInt16)dst_int << 4, dst_did);

        if((status = one_net_master_set_update_master_flag(update,
          (const one_net_raw_did_t * const)&dst_did)) != ONS_SUCCESS)
        {
            printf(
              "Calling one_net_master_set_update_master_flag returned %02X\n",
              status);
        } // if setting the update MASTER flag failed //
    } // else if update MASTER command //
    else if(!strncasecmp(cmd, "DT", CMD_SIZE))
    {
        one_net_raw_did_t sender, receiver;
        unsigned int sender_int, receiver_int;
        UInt16 data_rate;

        printf("Enter sender and receiver dids,");
        printf(" followed by the data rate idx\n");
        scanf("%u %u %hu", &sender_int, &receiver_int, &data_rate);

        one_net_int16_to_byte_stream((UInt16)receiver_int << 4, receiver);

        if(sender_int == 1)
        {
            status = one_net_master_start_data_rate_test(0,
              (const one_net_raw_did_t * const)&receiver, data_rate);
        } // if the sender is the MASTER //
        else
        {
            one_net_int16_to_byte_stream((UInt16)sender_int << 4, sender);
            status = one_net_master_start_data_rate_test(
              (const one_net_raw_did_t * const)&sender,
              (const one_net_raw_did_t * const)&receiver, data_rate);
        } // else the sender is a peer //

        if(status != ONS_SUCCESS)
        {
            printf("one_net_master_start_data_rate_test returned %02X\n",
              status);
        } // if starting the data rate test was not successful //
    } // else if data rate test cmd //
    else if(!strncasecmp(cmd, "DR", CMD_SIZE))
    {
        one_net_raw_did_t dst;
        unsigned int dst_int;
        UInt16 data_rate;

        printf("Enter destination did and the enumeration for the data rate\n");
        scanf("%u %hu", &dst_int, &data_rate);

        one_net_int16_to_byte_stream((UInt16)dst_int << 4, dst);

        if((status = master_change_client_data_rate(
          (const one_net_raw_did_t * const)&dst, (UInt8)data_rate))
          != ONS_SUCCESS)
        {
            printf("master_change_client_data_rate returned %02X\n", status);
        } // if changing the CLIENT data rate failed //
    } // else if change data rate command //
    else if(!strncasecmp(cmd, "PR", CMD_SIZE))
    {
        one_net_raw_did_t dst, peer;
        UInt16 dst_int, peer_int;
        UInt16 data_rate;

        printf("Enter dst did, peer did, peer data rate\n");
        scanf("%hu %hu %hu", &dst_int, &peer_int, &data_rate);

        one_net_int16_to_byte_stream(dst_int << 4, dst);
        one_net_int16_to_byte_stream(peer_int << 4, peer);

        if((status = master_change_peer_data_rate(
          (const one_net_raw_did_t * const)&dst,
          (const one_net_raw_did_t * const)&peer, data_rate)) != ONS_SUCCESS)
        {
            printf("master_change_peer_data_rate returned %02X\n", status);
        } // if changing the peer data rate failed //
    } // else if change peer data rate command //
    else if(!strncasecmp(cmd, "KA", CMD_SIZE))
    {
        one_net_raw_did_t dst;
        UInt16 dst_int;
        UInt32 keep_alive;

        printf("Enter dst did, keep alive interval\n");
        scanf("%hu %u", &dst_int, &keep_alive);

        one_net_int16_to_byte_stream(dst_int << 4, dst);

        if((status = master_change_client_keep_alive(
          (const one_net_raw_did_t * const)&dst, keep_alive)) != ONS_SUCCESS)
        {
            printf("master_change_client_keep_alive returned %02X\n", status);
        } // if changing the keep alive interval failed //
    } // else if change keep alive command //
    else if(!strncasecmp(cmd, "FD", CMD_SIZE))
    {
        one_net_raw_did_t dst;
        UInt16 dst_int;
        UInt32 delay;

        unsigned int priority;

        printf("enter dst did, fragment delay, priority (0 - low, 1 - high)\n");
        scanf("%hu %u %u", &dst_int, &delay, &priority);

        one_net_int16_to_byte_stream(dst_int << 4, dst);

        if((status = master_change_client_frag_dly(
          (const one_net_raw_did_t * const)&dst,
          priority ? ONE_NET_HIGH_PRIORITY : ONE_NET_LOW_PRIORITY, delay))
          != ONS_SUCCESS)
        {
            printf("master_change_client_frag_dly returned %02X\n", status);
        } // if changing the fragment delay was not successful //
    } // else if change fragment delay command //
    else if(!strncasecmp(cmd, "NK", CMD_SIZE))
    {
        printf("Changing the key\n");
        if((status = master_change_key(one_net_prand(one_net_tick(),
          0xFFFFFFFF))) != ONS_SUCCESS)
        {
            printf("master_change_key returned %02X\n", status);
        } // if creating the new key failed //
    } // else if new key command //
} // process_cmd //

//						PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

