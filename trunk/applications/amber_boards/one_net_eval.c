//! \addtogroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.c
    \brief The ONE-NET evaluation project.

    This is the application that runs on the ONE-NET evaluation boards.
*/

#include "one_net_eval.h"

#include "oncli.h"
//#include "oncli_port.h"
//#include "oncli_str.h"
#include "one_net_application.h"
#include "one_net_client.h"
#include "one_net_encode.h"
#include "one_net_eval_hal.h"
//#include "one_net_master.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "pal.h"
#include "tal.h"
//#include "uart.h"
//#include "dfi.h"
#include "nprintf.h"
#include "string.h"

#ifdef _R8C_TINY
#pragma section program program_high_rom
#endif

//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{

//! The default keep alive interval (in ticks).
#define DEFAULT_KEEP_ALIVE_INTERVAL 300000

enum
{
    //! The maximum size of a block transaction (in bytes) for the eval.
    EVAL_MAX_BLOCK_LEN = ONCLI_MAX_BLOCK_TXN_LEN,
    
    //! The time in ticks to leave the LEDs on. 50ms
    EVAL_LED_ON_TIME = MS_TO_TICK(50)
};

//! ONE-NET initialization constants
enum
{
    //! The evaluation channel
    EVAL_CHANNEL = 2,

    //! The evaulation data rate.  The MASTER must remain at 38400
    DATA_RATE = ONE_NET_DATA_RATE_38_4,

    //! The encryption method to use during the evaluation for single and
    //! block packets.
    EVAL_SINGLE_BLOCK_ENCRYPTION = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
    
    //! The encryption method to use during the evaluation for stream packets
    EVAL_STREAM_ENCRYPTION = ONE_NET_STREAM_ENCRYPT_XTEA8,
    
    //! The low priority fragment delay to use in the evaluation
    EVAL_LOW_PRIORITY_FRAG_DLY = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY,
    
    //! The high priority fragment delay to use in the evaluation
    EVAL_HIGH_PRIORITY_FRAG_DLY = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY
};

enum
{
    //! Indicates there is no block data transaction
    NO_BLOCK_TXN = 0xFFFF
};


//! The raw CLIENT DIDs for auto mode
static const one_net_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] =
{
    {0x00, 0x20}, {0x00, 0x30}, {0x00, 0x40}
};

//! The key used in the evaluation network ("protected")
const one_net_xtea_key_t EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

//! The key to use for stream transactions in the eval network ("protected")
const one_net_xtea_key_t EVAL_STREAM_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

//! Default invite key to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_INVITE_KEY[] = { '2', '2', '2', '2',   '2', '2', '2', '2',
                                     '2', '2', '2', '2',   '2', '2', '2', '2'};

//! Default SID to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_RAW_SID[] =        { 0x00, 0x00, 0x00, 0x00, 0x10, 0x01 };

//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{

//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_eval_pri_var
//! \ingroup ONE-NET_eval
//! @{

//! The status of the user pins.  These are considered "protected" and are
//! shared with master_eval & client_eval.
user_pin_t user_pin[NUM_USER_PINS];

//! The index of the sid to use in the evaluation network.
static UInt8 eval_sid_idx = 0;

//! The value the mode switch is set to
static UInt8 mode_value;

//! The node type the device is operating at
static UInt8 node_type;

//! The number of bytes in the entire block transaction.  This value should be
//! set to 0 if there is no pending block transaction.
static UInt16 block_data_len = 0;

//! Index to the start of the payload for the next block data transfer
static UInt16 block_data_pos = 0;

//! Buffer to store the data for the entire block transaction
static UInt8 block_data[EVAL_MAX_BLOCK_LEN];

//! did of device that the block transaction is being carried out with
static one_net_raw_did_t block_did;

//! TRUE if sending the block transaction, false if receiving
static BOOL send_block = FALSE;

//! The type of data being transfered in the block transaction
static UInt16 block_data_type = NO_BLOCK_TXN;

//! Pointer to the device dependent (MASTER, CLIENT, SNIFF) function that
//! should be called in the main loop
static void(*node_loop_func)(void) = 0;

//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

// forward declare functions
void init_processor(void);
void init_ports(void);
void tal_init_ports(void);
void init_tick_timer(void);
void init_rf_interrupts(void);
void tal_init_transceiver(void);

void init_auto_client(node_select_t CLIENT);

void client_eval(void);

// "protected" functions
oncli_status_t set_device_type(UInt8 device_type);

void disable_user_pins(void);
BOOL get_user_pin_type(UInt8 * user_pin_type, UInt8 NUM_PINS);

static void print_data_packet(const UInt8 *txn_str, const UInt8 *RX_PLD,
  UInt16 len, const one_net_raw_did_t *src_addr);
static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const one_net_raw_did_t *SRC_ADDR);

//char output[128];

void oncli_send_msg(const char * const FMT, ...);
void oncli_print_admin_msg(const UInt8 MSG_TYPE, const UInt8 TXN_TYPE,
  const UInt8 ADMIN_MSG_TYPE, const UInt8 * ADMIN_MSG_DATA, const UInt16 LEN);

void oncli_send_msg(const char * const FMT, ...)
{
/*
    va_list ap;
    int output_len;

    if(!FMT)
    {
        return;
    } // if the parameter is invalid //

    va_start(ap, FMT);    
    output_len = vsnprintf(output, sizeof(output), FMT, ap);
    uart_write((UInt8 const *)output, output_len);
    va_end(ap);
*/
} // oncli_send_msg //

void oncli_print_admin_msg(const UInt8 MSG_TYPE, const UInt8 TXN_TYPE,
  const UInt8 ADMIN_MSG_TYPE, const UInt8 * ADMIN_MSG_DATA, const UInt16 LEN)
{
  return;
}

void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
  return;
}

//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{

/*!
    \brief Returns the node type of the device.
    
    \param void
    
    \return The node type of the device (see node_select_t)
*/
UInt8 device_type(void)
{
    return node_type;
} // device_type //


/*!
    \brief Returns the mode (auto or serial)
    
    \param void
    
    \return The mode the device is operating at (see mode_select_t)
*/
UInt8 mode_type(void)
{
    return mode_value;
} // mode_type //


/*!
    \brief Turns on the transmit LED
    
    \param void
    
    \return void
*/
void turn_on_tx_led(void)
{
    TURN_ON(TX_LED);
    ont_set_timer(TX_LED_TIMER, EVAL_LED_ON_TIME);
} // turn_on_tx_led //


/*!
    \brief Turns on the receive LED
    
    \param void
    
    \return void
*/
void turn_on_rx_led(void)
{
    TURN_ON(RX_LED);
    ont_set_timer(RX_LED_TIMER, EVAL_LED_ON_TIME);
} // turn_on_rx_led //


/*!
    \brief Compares LEN bytes of memory
    
    \param[in] LHS The left hand side of the comparison
    \param[in] RHS The right hand side of the comparison
    \param[in] LEN The number of bytes to compare
    
    \return TRUE if the dids match
            FALSE otherwise
*/
BOOL mem_equal(const UInt8 *LHS, const UInt8 *RHS, UInt16 LEN)
{
    UInt16 i;
    
    for(i = 0;  i < LEN; i++)
    {
        if(LHS[i] != RHS[i])
        {
            return FALSE;
        } // if the bytes are not equal //
    } // loop to compare bytes //

    return TRUE;
} // mem_equal //


/*!
    \brief Returns the encoded NID to use for the evaluation network.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[out] nid Returns the encoded NID to use in the evaluation network.
    
    \return TRUE if the operation was successful.
            FALSE if the nid was not returned.
*/
BOOL get_eval_encoded_nid(on_encoded_nid_t *nid)
{
    one_net_raw_sid_t * ptr_raw_sid;
    on_encoded_nid_t encoded_nid;

    if(!nid)
    {
        return FALSE;
    } // if the parameter is invalid //

    // get the current raw sid
    if (mode_value == SERIAL_MODE)
    {
        //
        // for serial mode, get a board specific NID
        //
        ptr_raw_sid = get_raw_sid();
    }
    else
    {
        //
        // for auto mode, get a fixed NID
        //
        ptr_raw_sid = (one_net_raw_sid_t *) &DEFAULT_RAW_SID[0];
    }

    // encode the raw sid to obtain an encoded sid
    if (on_encode(&encoded_nid[0], (UInt8 *)ptr_raw_sid, sizeof(on_encoded_nid_t)) != ONS_SUCCESS)
    {
        return FALSE;
    }

    // copy the encoded sid to the buffer provided by the caller
    one_net_memmove(nid, &encoded_nid[0], sizeof(on_encoded_nid_t));

    return TRUE;
} // get_eval_encoded_nid //


/*!
    \brief Returns the encoded did for the given device in the evaluation
      network.

    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[in] NODE The device to get the evaluation did for.
    
    \return TRUE if the the did was returned.
            FALSE if an error occurred and the did was not returned.
*/
BOOL get_eval_encoded_did(node_select_t node, on_encoded_did_t *did)
{
    one_net_raw_sid_t * ptr_raw_sid;
    on_encoded_sid_t encoded_sid;

    if((node != MASTER_NODE && node != AUTO_CLIENT1_NODE
      && node != AUTO_CLIENT2_NODE && node != AUTO_CLIENT3_NODE) || !did)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    if(node == MASTER_NODE)
    {
        // get the current raw sid
        ptr_raw_sid = get_raw_sid();

        // encode the raw sid to obtain an encoded sid
        if (on_encode(&encoded_sid[0], (UInt8 *) ptr_raw_sid, sizeof(on_encoded_sid_t)) != ONS_SUCCESS)
        {
            return FALSE;
        }

        one_net_memmove(*did, &encoded_sid[ON_ENCODED_NID_LEN], sizeof(on_encoded_did_t));

    } // if the MASTER device //
    else
    {
        on_encode(*did, RAW_AUTO_CLIENT_DID[node - AUTO_CLIENT1_NODE],
          sizeof(on_encoded_did_t));
    } // else it's a CLIENT device //
    
    return TRUE;
} // get_eval_encoded_did //


/*!
    \brief Returns the key to use in the evaluation network.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[out] key Returns the key to use in the evaluation network.
    
    \return TRUE if the operation was successful
            FALSE if the key was not returned.
*/
BOOL get_eval_key(one_net_xtea_key_t *key)
{
    if(!key)
    {
        return FALSE;
    } // if the parameter is invalid //
    
    one_net_memmove(*key, EVAL_KEY, sizeof(one_net_xtea_key_t));
    
    return TRUE;
} // get_eval_key //


/*!
    \brief Returns a pointer to the invite key to use in for joining a network.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \return A pointer to the invite key to use.
*/
UInt8 * get_invite_key(void)
{
    return(unsigned char *)(&DEFAULT_INVITE_KEY[0]); 
} // get_invite_key //


/*!
    \brief Returns a pointer to the SID of this board.
    
    \return A pointer to the invite key to use.
*/
one_net_raw_sid_t * get_raw_sid(void)
{
        return((one_net_raw_sid_t *) &DEFAULT_RAW_SID[0]); 
} // get_raw_sid //


/*!
    \brief Returns the key to use for stream transactions.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[out] stream_key Returns the key to use for stream transactions
    
    \return TRUE if the operation was successful
            FALSE if the key was not returned.
*/
BOOL get_eval_stream_key(one_net_xtea_key_t *stream_key)
{
    if(!stream_key)
    {
        return FALSE;
    } // if the parameter is invalid //
    
    one_net_memmove(*stream_key, EVAL_STREAM_KEY, sizeof(one_net_xtea_key_t));
    
    return TRUE;
} // get_eval_stream_key //


/*!
    \brief Returns the encryption method to use for the given transfer type.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[in] ENCRYPT_TYPE The type of transfer to return the encryption
      method for (single/block or stream).

    \return The encryption method to use for the given transfer type.
      ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE is returned if there was an error.
*/
UInt8 eval_encryption(on_data_t ENCRYPT_TYPE)
{
    if(ENCRYPT_TYPE == ON_SINGLE || ENCRYPT_TYPE == ON_BLOCK)
    {
        return EVAL_SINGLE_BLOCK_ENCRYPTION;
    } // if single or block transfer //
    else if(ENCRYPT_TYPE == ON_STREAM)
    {
        return EVAL_STREAM_ENCRYPTION;
    } // else if stream transaction //
    
    return ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE;
} // eval_encryption //


/*!
    \brief Returns the channel the evaluation network should use.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param void
    
    \return The channel the evaluation network uses.
*/
UInt8 eval_channel(void)
{
    return EVAL_CHANNEL;
} // eval_channel //


/*!
    \brief Returns the default data rate the given device receives at in the
      evaluation network.

    This is a "protected" function.  This should be called when the evaluation
    network is first set up.

    \param[in] NODE The device to get the default data rate for.
    
    \return The default data rate for the given device.
*/
UInt8 eval_data_rate(node_select_t node)
{
    return DATA_RATE;
} // eval_data_rate //


/*!
    \brief Returns the default keep alive interval CLIENTS should use in the
      evaluation network.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param void
    
    \return The default keep alive interval
*/
UInt32 eval_keep_alive(void)
{
    return DEFAULT_KEEP_ALIVE_INTERVAL;
} // eval_keep_alive //


/*!
    \brief Returns the default fragment delay at the given priority
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.

    \param[in] PRIORITY The priority of the requested fragment delay
    
    \return The fragment delay for the given priority
*/
UInt32 eval_fragment_delay(UInt8 PRIORITY)
{
    if(PRIORITY == ONE_NET_HIGH_PRIORITY)
    {
        return EVAL_HIGH_PRIORITY_FRAG_DLY;
    } // if high priority //
    
    return EVAL_LOW_PRIORITY_FRAG_DLY;
} // eval_fragment_delay //


/*!
    \brief Returns the features field for the given CLIENT in the evaluation
      network.
      
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.

    \param[in] CLIENT The evaluation client to get the features for.
    
    \return The features for the given CLIENT.
*/
UInt8 eval_client_features(node_select_t CLIENT)
{
    if(CLIENT == AUTO_CLIENT1_NODE || CLIENT == AUTO_CLIENT2_NODE
      || CLIENT == AUTO_CLIENT3_NODE)
    {
        return ON_MH_CAPABLE | ON_MAC_FEATURES;
    } // if CLIENT 1, 2, or 3 //
    
    return 0x00;
} // eval_client_features //


/*!
    \brief Returns the flags field for the given CLIENT in the evaluation
      network.

    This is a "protected" function.  This should be called when the evaluation
    network is first set up.

    \param[in] The evaluation client to get the flags for.
    
    \return The flags for the given CLIENT.
*/
UInt8 eval_client_flag(node_select_t CLIENT)
{
    if(CLIENT == AUTO_CLIENT1_NODE)
    {
        return ON_JOINED | ON_SEND_TO_MASTER;
    } // if CLIENT 1 //
    else if(CLIENT == AUTO_CLIENT2_NODE || CLIENT == AUTO_CLIENT3_NODE)
    {
        return ON_JOINED;
    } // if CLIENT 2 or 3 //
    
    return 0x00;
} // eval_client_flag //


/*!
    \brief Converts a raw did to a U16 value.
    
    \param[in] DID The device id to convert
    
    \return The U16 value corresponding to the DID.
*/
UInt16 did_to_u16(const one_net_raw_did_t *DID)
{
    if(!DID)
    {
        return 0;
    } // if the parameter is invalid //
    
    return one_net_byte_stream_to_int16(*DID) >> RAW_DID_SHIFT;
} // did_to_u16 //


/*!
    \brief Callback to check if device should grant the requested transaction
    
    \param[in] TYPE The type of transaction being requested.  Currently only
      handles ON_BLOCK
    \param[in] SEND TRUE if this device is the sender of the data.  Currently
      only handles FALSE.
    \param[in] DATA_TYPE The type of data being requested
    \param[in] DATA_LEN The number of bytes to be transferred.
    \param[in] DID The other end of the transaction.
    
    \return TRUE if the transaction is to be handled.
            FALSE otherwise
*/
BOOL eval_txn_requested(UInt8 TYPE, BOOL SEND, UInt16 DATA_TYPE, 
                        UInt16 DATA_LEN, const one_net_raw_did_t *DID)
{
    // block_data_len is non-0 if there is an outstanding transaction
    if(TYPE != ON_BLOCK || SEND || !DID || block_data_len)
    {
        return FALSE;
    } // if the transaction should not be handled //

    one_net_memmove(block_did, *DID, sizeof(block_did));
    block_data_len = DATA_LEN;
    send_block = FALSE;
    block_data_type = DATA_TYPE & ONA_MSG_TYPE_MASK;
    
    return TRUE;
} // eval_txn_requested //


/*!
    \brief Returns the next payload to send for the given transaction.

    \param[in] TYPE The type of transaction.  Must be ON_BLOCK.
    \param[out] len The number of bytes to be sent
      (must be <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
    \param[in] DST The device receiving the next payload.

    \return Pointer to the data to be sent.
*/
UInt8 *eval_next_payload(UInt8 TYPE, UInt16 *len,
                                const one_net_raw_did_t *DST)
{
    UInt16 position = block_data_pos;

    if(TYPE != ON_BLOCK || !len || !DST || !block_data_len
      || !send_block || !mem_equal(*DST, block_did, ONE_NET_RAW_DID_LEN))
    {
        return 0;
    } // if the parameters are invalid //
    
    *len = block_data_len - block_data_pos < ONE_NET_RAW_BLOCK_STREAM_DATA_LEN
      ? block_data_len - block_data_pos : ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;

    block_data_pos += *len;
    
    return &(block_data[position]);
} // eval_next_payload //


/*!
    \brief Reports the status of the block transaction
    
    This will be called in the case of an extended admin transaction as well
    (meaning the app doesn't know about the transaction).
    
    \param[in] STATUS The results of the block transaction
    \param[in] DID The device at the other end of the block transaction.

    \return void
*/
void eval_block_txn_status(one_net_status_t STATUS, 
                           const one_net_raw_did_t *DID)
{
    if(!DID)
    {
        return;
    } // if the parameters are invalid //

//    oncli_send_msg(ONCLI_BLOCK_RESULT_FMT, did_to_u16(DID),
//      oncli_status_str(STATUS));
//    oncli_print_prompt();

    if(mem_equal(*DID, block_did, ONE_NET_RAW_DID_LEN))
    {
        block_data_len = 0;
        block_data_type = NO_BLOCK_TXN;
    } // if there was an outstanding transaction //
} // eval_block_txn_status //


/*!
    \brief Handles the received single transaction.
    
    \param[in] RX_PLD The data that was received.
    \param[in] RX_PLD_LEN The length of the data that was received
    \param[in] SRC_ADDR The sender of the data

    \return void
*/
/* dje: Like original from jay: handles on or off; doesn't handle toggle. */
void eval_handle_single(const UInt8 *RX_PLD, UInt16 RX_PLD_LEN,
                        const one_net_raw_did_t *SRC_ADDR)
{
    ona_msg_type_t msg_type;
    ona_msg_class_t msg_class;
    UInt8  dst_unit;
    UInt16 msg_data;

    if(!RX_PLD || !SRC_ADDR)
    {
//        oncli_send_msg(ONCLI_GENERAL_ERROR_FMT, ONCLI_GEN_ERR_BAD_PARAMETER, eval_handle_single);
        // EXIT();
        return;
    } // if any of the parameters are invalid //
    dst_unit = get_dst_unit(RX_PLD);
    msg_data = get_msg_data(RX_PLD);

    msg_type  =  (ona_msg_type_t)get_msg_hdr(RX_PLD);
    msg_class =  msg_type & ONA_MSG_CLASS_MASK; // uses lower bits
    msg_type  &= ~ONA_MSG_CLASS_MASK;           // uses upper bits
    

    if(msg_type == ONA_SWITCH && msg_class == ONA_COMMAND)
    {
        if ((user_pin[0].pin_type == ONCLI_OUTPUT_PIN) && (dst_unit == 0))
        {
            USER_PIN0_OUT = (msg_data == ONA_ON) ? 1 : 0;
        } // if pin1 is an output and the message is for unit 1 //
        else if ((user_pin[1].pin_type == ONCLI_OUTPUT_PIN) && (dst_unit == 1))
        {
            USER_PIN1_OUT = (msg_data == ONA_ON) ? 1 : 0;
        } // if pin2 is an output and the message is for pin 1 //
        else if ((user_pin[2].pin_type == ONCLI_OUTPUT_PIN) && (dst_unit == 2))
        {
            USER_PIN2_OUT = (msg_data == ONA_ON) ? 1 : 0;
        } // if pin3 is an output and the message is for pin 1 //
        else if ((user_pin[3].pin_type == ONCLI_OUTPUT_PIN) && (dst_unit == 3))
        {
            USER_PIN3_OUT = (get_msg_data(RX_PLD) == ONA_ON) ? 1 : 0;
        } // if pin4 is an output and the message is for pin 1 //
    } // if a switch command message //

//    print_packet(ONCLI_SINGLE_TXN_STR, RX_PLD, RX_PLD_LEN, SRC_ADDR);
} // eval_handle_single //


/*!
    \brief Prints the contents of the received data packet.

    This function needs to handle two types of message payloads
    (single data and block data) and for each message type, two
    different paylaod types (text and binary data). We have to 
    look inside the payload in each case to determine whether
    the payload contains text or binary data.

    NOTE: The static variables block_data_type and block_data_length
    are set in the function eval_txn_requested which is called when
    we receive the single data basic admin message that sets up
    the block data message.

    NOTE: This function uses the "==" operator to compare two strings
    rather than using strcmp() because it relies on the fact that
    pointers will only be assigned values from limited set of pointers
    to strings defined in oncli_strs.[ch]. 
    
    \param[in] TXN_STR String representing the transaction type that was
      received.  This can only be ONCLI_SINGLE_TXN_STR, or ONCLI_BLOCK_TXN_STR.
    \param[in] TX_PLD The payload that was received.
    \param[in] RX_PLD_LEN The number of bytes that was received.
    \param[in] SRC_ADDR The sender of the data packet

    \return void
*/
/*
void print_packet(const UInt8 *txn_str, const UInt8 *rx_pld,
                  UInt16 len, const one_net_raw_did_t *src_addr)
{

    if(!txn_str || !rx_pld || !src_addr ||
       (txn_str != ONCLI_SINGLE_TXN_STR && txn_str != ONCLI_BLOCK_TXN_STR))
    {
        oncli_send_msg(ONCLI_GENERAL_ERROR_FMT, ONCLI_GEN_ERR_BAD_PARAMETER, print_packet);
        return;
    } // if any of the parameters are invalid //
    
    if(txn_str == ONCLI_SINGLE_TXN_STR)
    {
        //
        // handle the single data message case by first
        // extracting the message class and type
        //
        UInt16 class_and_data_type;
        
        class_and_data_type = get_msg_hdr(rx_pld);
        
        if((class_and_data_type & ONA_MSG_CLASS_MASK) == ONA_COMMAND
          && (class_and_data_type & ONA_MSG_TYPE_MASK) == ONA_SIMPLE_TEXT)
        {
            //
            // handle single data text payload
            //
            print_text_packet(txn_str, &(rx_pld[ONA_MSG_DATA_IDX]), ONA_MSG_DATA_LEN, src_addr);
        } // if text command //
        else
        {
            //
            // handle single data binary payload
            //
            print_data_packet(txn_str, rx_pld, len, src_addr);
        } // else not a text command packet //
    } // if a single data packet //
    else 
    {
        UInt16 msg_type;
        UInt16 blk_length;

        //
        // handle the block data message case by first extracting the 
        // message type from the payload
        //
        get_block_data_payload_hdr(&msg_type, &blk_length, NULL, NULL, (unsigned char *)rx_pld);

        if (block_data_type != msg_type)
        {
            oncli_send_msg(ONCLI_GENERAL_ERROR_FMT, ONCLI_GEN_ERR_WRONG_BLOCK_DATA_TYPE, print_packet);
        }

        if(msg_type == ONA_BLOCK_TEXT)
        {
            print_text_packet(txn_str, &(rx_pld[ONA_BLK_DATA_HDR_DATA_IDX]),
              blk_length-ONA_BLK_DATA_HDR_DATA_LEN, src_addr);
        } // if text command //
        else
        {
            print_data_packet(txn_str, rx_pld, blk_length, src_addr);
        } // else not a text command packet //
    
    } // else if a block data packet //

    oncli_print_prompt();
} // print_packet //
*/

/*!
    \brief Outputs the results of a single transaction
    
    \param[in] STATUS The results of the transaction.
    \param[in] DATA The data sent in the transaction
    \param[in] DST The destination device for the transaction
    
    \return void
*/

void eval_single_txn_status(one_net_status_t STATUS, const UInt8 *DATA, 
                            const one_net_raw_did_t *DST)
{
/*
    if(!DATA && !DST)
    {
        return;
    } // if the parameters are invalid //
    
    oncli_send_msg(ONCLI_SINGLE_RESULT_FMT, did_to_u16(DST),
      oncli_status_str(STATUS));
    oncli_print_prompt();
*/
  return;
} // eval_single_txn_status //


one_net_send_single_func_t oncli_get_send_single_txn_func(void)
{
    if(node_type >= CLIENT_NODE && node_type <= AUTO_CLIENT3_NODE)
    {
        return &one_net_client_send_single;
    } // else if a CLIENT device //
    
    return 0;
} // oncli_get_send_single_func //


oncli_status_t oncli_q_block_request(BOOL SEND, UInt16 DATA_TYPE,
              UInt8 PRIORITY, const one_net_raw_did_t *DID,
              UInt8 SRC_UNIT, UInt8 DST_UNIT, const UInt8 *DATA, UInt16 LEN)
{
    // The result of queuing the block request
    one_net_status_t request_status;

    if(!DID || !DATA || LEN > sizeof(block_data))
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //
    
    // check to see if a transaction is already queued
    if(block_data_len)
    {
        return ONCLI_ALREADY_IN_PROGRESS;
    } // if a block transaction is already in progress //
    
    // make the appropriate request for the device
    if(oncli_is_client())
    {
        request_status = one_net_client_block_stream_request(ON_BLOCK, SEND,
          DATA_TYPE, LEN, PRIORITY, DID, SRC_UNIT);
    } // else if CLIENT //
    else
    {
        return ONCLI_INVALID_CMD_FOR_NODE;
    } // else command invalid for the node type //
    
    // parse the return value from the request
    switch(request_status)
    {
        case ONS_SUCCESS:
        {
            one_net_memmove(block_data, DATA, LEN);
            block_data_len = LEN;
            send_block = TRUE;
            block_data_pos = 0;
            one_net_memmove(block_did, *DID, sizeof(block_did));
            return ONCLI_SUCCESS;
//            break;
        } // success case //

        case ONS_BAD_PARAM:
        {
            return ONCLI_PARSE_ERR;
//            break;
        } // bad parameter case //

        case ONS_RSRC_FULL:
        {
            return ONCLI_RSRC_UNAVAILABLE;
//            break;
        } // resource full case //
        
        case ONS_BAD_ADDR:
        {
            return ONCLI_INVALID_DST;
//            break;
        } // invalid destination case //
        
        case ONS_ALREADY_IN_PROGRESS:
        {
            return ONCLI_ALREADY_IN_PROGRESS;
//            break;
        } // already in progress case //

        case ONS_NOT_JOINED:
        {
            return ONCLI_NOT_JOINED;
//            break;
        } // device needs to join a network first //

        default:
        {
            break;
        } // default case //
    } // switch(request_status) //
    
    // all return types should have been handled
    return ONCLI_INTERNAL_ERR;
} // oncli_q_block_request //


oncli_status_t oncli_set_user_pin_type(UInt8 PIN, UInt8 PIN_TYPE)
{
    if(PIN_TYPE > ONCLI_DISABLE_PIN)
    {
        return ONCLI_BAD_PARAM;
    } // if any of the parameters are invalid //

    switch(PIN)
    {
        case 0:
        {
            user_pin[0].old_state = USER_PIN0_IN;
            USER_PIN0_DIR = PIN_TYPE == ONCLI_DISABLE_PIN ? ONCLI_INPUT_PIN
              : PIN_TYPE;
            break;
        } // pin 1 case //

        case 1:
        {
            user_pin[1].old_state = USER_PIN1_IN;
            USER_PIN1_DIR = PIN_TYPE == ONCLI_DISABLE_PIN ? ONCLI_INPUT_PIN
              : PIN_TYPE;
            break;
        } // pin 2 case //

        case 2:
        {
            user_pin[2].old_state = USER_PIN2_IN;
//            prc2 = 1;
            USER_PIN2_DIR = PIN_TYPE == ONCLI_DISABLE_PIN ? ONCLI_INPUT_PIN
              : PIN_TYPE;
//            prc2 = 0;
            break;
        } // pin 3 case //

        case 3:
        {
            user_pin[3].old_state = USER_PIN3_IN;
//            prc2 = 1;
            USER_PIN3_DIR = PIN_TYPE == ONCLI_DISABLE_PIN ? ONCLI_INPUT_PIN
              : PIN_TYPE;
//            prc2 = 0;
            break;
        } // pin 4 case //

        default:
        {
            return ONCLI_RSRC_UNAVAILABLE;
//            break;
        } // default case //
    } // switch(PIN) //

    user_pin[PIN].pin_type = PIN_TYPE;
    return ONCLI_SUCCESS;
} // oncli_set_user_pin_type //


const one_net_raw_sid_t * oncli_get_sid(void)
{

    return get_raw_sid();

} // oncli_get_sid //

/*
const char *oncli_node_type_str(void)
{
    switch(node_type)
    {
        case MASTER_NODE:
        {
            return ONCLI_MASTER_STR;
//            break;
        } // MASTER case //
        
        case SNIFFER_NODE:
        {
            return ONCLI_SNIFFER_STR;
//            break;
        } // sniffer case //
        
        default:
        {
            // must be a CLIENT
            break;
        } // default case //
    } // switch(node_type) //
    
    return ONCLI_CLIENT_STR;
} // oncli_node_type_str //
*/
/*
const char * oncli_mode_type_str(void)
{
    switch(mode_value)
    {
        case AUTO_MODE:
        {
            return ONCLI_AUTO_MODE_STR;
//            break;
        } // auto_mode case //

        case SERIAL_MODE:
        {
            return ONCLI_SERIAL_MODE_STR;
//            break;
        } // serial mode case //

        default:
        {
            break;
        } // default case //
    } // switch(mode_value) //
    
    return 0;
} // oncli_mode_type_str //
*/

BOOL oncli_is_master(void)
{
    return (BOOL)(node_type == MASTER_NODE);
} // oncli_is_master //


BOOL oncli_is_client(void)
{
    return (BOOL)(node_type >= CLIENT_NODE && node_type <= AUTO_CLIENT3_NODE);
} // oncli_is_client //


void oncli_print_prompt(void)
{
    // None of the strings are declared as constants since this should be the
    // only place they are used, and by not declaring them, we are ensuring
    // that this is the only placed they are used.
    switch(device_type())
    {
        case SNIFFER_NODE:
        {
            oncli_send_msg("ocm-s> ");
            break;
        } // sniffer case //
        
        case MASTER_NODE:
        {
            oncli_send_msg("ocm-m> ");
            break;
        } // master case //
        
        case CLIENT_NODE:
        {
            oncli_send_msg("ocm-c> ");
            break;
        } // client case //
        
        case AUTO_CLIENT1_NODE:
        {
            oncli_send_msg("ocm-c1> ");
            break;
        } // auto client1 case //
        
        case AUTO_CLIENT2_NODE:
        {
            oncli_send_msg("ocm-c2> ");
            break;
        } // auto client2 case //
        
        case AUTO_CLIENT3_NODE:
        {
            oncli_send_msg("ocm-c3> ");
            break;
        } // auto client3 case //

        default:
        {
            oncli_send_msg("ocm> ");
            break;
        } // default case //
    } // switch(device_type()) //
} // oncli_print_prompt //


/*!
    \brief Sends the switch status messages for the given source unit.
    
    \param[in] SWITCH_STATUS The status of the switch being sent to DST.
    \param[in] SRC_UNIT The unit in this device the message pertains to.
    \param[in] DST_UNIT The unit in the receiving device that is to receive
      this message.
    \param[in] DST The device that is to receive this message.
    
    \return ONS_SUCCESS if the message was successfully queued.
            ONS_INTERNAL_ERR If the send single function could not be retrieved.
            See one_net_client_send_single & one_net_master_send_single for
            more return values.
*/

/* dje: Changed for new message data stuff */
one_net_status_t send_switch_command(UInt8 SWITCH_STATUS, UInt8 SRC_UNIT, 
        UInt8 DST_UNIT, const one_net_raw_did_t *DST)
{
    one_net_send_single_func_t send_single_func;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0}; // Initialize to all zeros

    put_msg_hdr(ONA_COMMAND|ONA_SWITCH, payload);
    put_src_unit(SRC_UNIT, payload);
    put_dst_unit(DST_UNIT, payload);
    put_msg_data(SWITCH_STATUS, payload);
    
    if(!(send_single_func = oncli_get_send_single_txn_func()))
    {
        return ONS_INTERNAL_ERR;
    } // if getting the single function failed //

    // third argument < second means that it will go through peer list
    return send_single_func(payload, sizeof(payload),
                            3, ONE_NET_SEND_SINGLE_PRIORITY, DST, SRC_UNIT);
} // send_switch_command //

//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

/*!
    \brief Sets the node type this device operates at.
    
    This is a "protected" function.
    
    \param[in] device_type The node type to operate at (see node_select_t)
    
    \return ONCLI_SUCCESS if the device has been set to operate as device_type
            ONCLI_BAD_PARAM if the parameter is invalid
            ONCLI_INVALID_CMD_FOR_MODE if the current mode does not allow changing
              the device type.
*/
oncli_status_t set_device_type(UInt8 device_type)
{
    if(device_type > AUTO_CLIENT3_NODE)
    {
        return ONCLI_BAD_PARAM;
    } // if the parameter is invalid //

    if(mode_value == AUTO_MODE)
    {
        return ONCLI_INVALID_CMD_FOR_MODE;
    } // if not setting the node type to device_type //
    
    node_type = device_type;
    
    node_loop_func = &client_eval;
    
    return ONCLI_SUCCESS;
} // set_device_type //


/*!
    \brief Prints the contents of the received data packet.
    
    \param[in] TXN_STR String representing the transaction type that was
      received.  This can only be ONCLI_SINGLE_TXN_STR, or ONCLI_BLOCK_TXN_STR.
    \param[in] TX_PLD The payload that was received.
    \param[in] RX_PLD_LEN The number of bytes that was received.
    \param[in] SRC_ADDR The sender of the data packet

    \return void
*/
/*
static void print_data_packet(const UInt8 *txn_str, const UInt8 *RX_PLD,
  UInt16 len, const one_net_raw_did_t *src_addr)
{
    UInt16 i;

#ifdef _ONE_NET_DEBUG_STACK 
    uart_write("\nIn print_data_packet, stack is ", 32);
    uart_write_int8_hex( (((UInt16)(&i))>>8) & 0xff );
    uart_write_int8_hex( ((UInt16)(&i)) & 0xff );
    uart_write("\n", 1);
#endif

    oncli_send_msg(ONCLI_RX_DATA_FMT, txn_str, did_to_u16(src_addr), len);

    for(i = 0; i < len; i++)
    {
        oncli_send_msg("%02X ", RX_PLD[i]);
        if((i % 16) == 15)
        {
            oncli_send_msg("\n");
        } // if the line has been filled //
    } // loop to output the bytes //
    
    if((i % 16) != 15)
    {
        oncli_send_msg("\n");
    } // if need to end the line //
} // print_data_packet //
*/

/*!
    \brief Prints the contents of the received text packet.
    
    \param[in] TXN_STR String representing the transaction type that was
      received.  This can only be ONCLI_SINGLE_TXN_STR, or ONCLI_BLOCK_TXN_STR.
    \param[in] TXT The text that was received.
    \param[in] TXT_LEN The number of characters received
    \param[in] SRC_ADDR The sender of the data packet

    \return void
*/
/*
static void print_text_packet(const UInt8 *txn_str, const UInt8 *TXT,
  UInt16 TXT_LEN, const one_net_raw_did_t *SRC_ADDR)
{
#ifdef _ONE_NET_DEBUG_STACK 
    UInt8 tmp;
    uart_write("\nIn print_text_packet, stack is ", 32);
    uart_write_int8_hex( (((UInt16)(&tmp))>>8) & 0xff );
    uart_write_int8_hex( ((UInt16)(&tmp)) & 0xff );
    uart_write("\n", 1);
#endif

    oncli_send_msg(ONCLI_RX_TXT_FMT, did_to_u16(SRC_ADDR), TXT_LEN, TXT);

} // print_text_packet //
*/

/*!
    \brief disables the user pins

    \param void

    \return void
*/
void disable_user_pins(void)
{
    UInt8 i;
    
    for(i = 0; i < NUM_USER_PINS; i++)
    {
        user_pin[i].pin_type = ONCLI_DISABLE_PIN;
    } // loop to clear user pins //
} // disable_user_pins //


/*!
    \brief Returns the settings for the user pins.
    
    \param[out] user_pin_type Array containing the user pin settings
    \param[in] NUM_PINS The number of user pins in user_pin_type
    
    \return TRUE if returning the user pin settings was successful
            FALSE if returning the user pin settings was not successful
*/
BOOL get_user_pin_type(UInt8 * user_pin_type, UInt8 NUM_PINS)
{
    UInt8 i;

    if(!user_pin_type || NUM_PINS != NUM_USER_PINS)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    for(i = 0; i < NUM_PINS; i++)
    {
        user_pin_type[i] = user_pin[i].pin_type;
    } // loop to get the settings //
    
    return TRUE;
} // get_user_pin_type //

/*
void oncli_print_user_pin_cfg(void)
{
    UInt8 i;
    UInt8 state;
    const UInt8 * type_string;

    oncli_send_msg("User pins:\n");
    for(i = 0; i < NUM_USER_PINS; i++)
    {
        //
        // collect the state of the user pin, so we can pront it
        //
        switch (i)
        {
            case 0:
            {
                state = USER_PIN0_IN;
                break;
            }

            case 1:
            {
                state = USER_PIN1_IN;
                break;
            }

            case 2:
            {
                state = USER_PIN2_IN;
                break;
            }

            case 3:
            {
                state = USER_PIN3_IN;
                break;
            }

            default:
            {
                state = 3;
            }
        }
        type_string = ONCLI_DISABLE_STR;
        if (user_pin[i].pin_type == ONCLI_INPUT_PIN)
        {
            type_string = ONCLI_INPUT_STR;
            oncli_send_msg("  %d %s state: %d\n", i, type_string, state); 
        }
        else if (user_pin[i].pin_type == ONCLI_OUTPUT_PIN)
        {
            type_string = ONCLI_OUTPUT_STR;
            oncli_send_msg("  %d %s state: %d\n", i, type_string, state); 
        }
        else
        {
            oncli_send_msg("  %d %s\n", i, type_string); 
        }
        delay_ms(100);
    }
} // oncli_print_user_pin_cfg //
*/

void main(void)
{
#ifdef _ONE_NET_DEBUG_STACK 
    UInt8 tmp; // should be close to the top of the normal stack
#endif
    // initialize the processor speed
    INIT_PROCESSOR();
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();

    INIT_PORTS_LEDS();
    INIT_TICK();
//    FLASH_ERASE_CHECK();

//    uart_init(BAUD_9600, DATA_BITS_8, STOP_BITS_1, PARITY_NONE);
    disable_user_pins();
    ENABLE_GLOBAL_INTERRUPTS();

//    oncli_send_msg("\n");
//    oncli_send_msg(ONCLI_STARTUP_FMT, MAJOR_VER, MINOR_VER);
//    oncli_send_msg(ONCLI_STARTUP_REV_FMT, REVISION_NUM, BUILD_NUM);

#ifdef _ONE_NET_DEBUG_STACK 
    uart_write("\nIn main, stack is ", 19);
    uart_write_int8_hex( (((UInt16)(&tmp))>>8) & 0xff );
    uart_write_int8_hex( ((UInt16)(&tmp)) & 0xff );
    uart_write("\n", 1);
#endif

    mode_value = AUTO_MODE;
//    node_type = AUTO_CLIENT1_NODE;
    node_type = AUTO_CLIENT2_NODE;
//    node_type = AUTO_CLIENT3_NODE;
    node_loop_func = &client_eval;
    init_auto_client(node_type);

    while(1)
    {
        if(ont_active(TX_LED_TIMER) && ont_expired(TX_LED_TIMER))
        {
            TURN_OFF(TX_LED);
            ont_stop_timer(TX_LED_TIMER);
        } // if time to turn off the tx led //
        
        if(ont_active(RX_LED_TIMER) && ont_expired(RX_LED_TIMER))
        {
            TURN_OFF(RX_LED);
            ont_stop_timer(RX_LED_TIMER);
        } // if time to turn off the rx led //

        (*node_loop_func)();

    } // main loop //
} // main //

//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
