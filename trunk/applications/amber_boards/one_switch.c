
#include "one_net_timer.h"
#include "ona_voltage_simple.h"
#include "ona_switch.h"
#include "pal.h"
#include "tal.h"

//! The default keep alive interval (in ticks).
#define DEFAULT_KEEP_ALIVE_INTERVAL 300000

enum
{
    //! The time in ticks to leave the LEDs on. 50ms
    EVAL_LED_ON_TIME = MS_TO_TICK(50)
};

//! ONE-NET initialization constants
enum
{
    //! The evaluation channel
    //EVAL_CHANNEL = 2,

    //! The evaulation data rate.  The MASTER must remain at 38400
    //DATA_RATE = ONE_NET_DATA_RATE_38_4,

    //! The encryption method to use during the evaluation for single and
    //! block packets.
    EVAL_SINGLE_BLOCK_ENCRYPTION = ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32,
    
    //! The encryption method to use during the evaluation for stream packets
    EVAL_STREAM_ENCRYPTION = ONE_NET_STREAM_ENCRYPT_XTEA8,
    
    //! The low priority fragment delay to use in the evaluation
    //EVAL_LOW_PRIORITY_FRAG_DLY = ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY,
    
    //! The high priority fragment delay to use in the evaluation
    //EVAL_HIGH_PRIORITY_FRAG_DLY = ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY
};

//! The key used in the evaluation network ("protected")
//const one_net_xtea_key_t EVAL_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
//  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

//! The key to use for stream transactions in the eval network ("protected")
//const one_net_xtea_key_t EVAL_STREAM_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
//  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

//! Default invite key to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
const UInt8 DEFAULT_INVITE_KEY[] = { 'a', 'm', 'b', 'r',   'z', 'I', '2', '3',
                                     'a', 'm', 'b', 'r',   'z', 'I', '2', '3'};
//const UInt8 DEFAULT_INVITE_KEY[] = { 'a', 'm', 'b', 'r',   'z', 'I', '2', '6',
//                                     'a', 'm', 'b', 'r',   'z', 'I', '2', '6'};

const ona_unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES] = {{ONA_SIMPLE_SWITCH, ONE_NET_NUM_UNITS}};

//! the voltage status reading
static UInt8 voltage_status;

//! the switch position reading
static UInt16 g_sw;

void check_switch(void);
BOOL get_sw_state(UInt16 *state);
void turn_on_tx_led(void);
void turn_on_rx_led(void);
void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN);
UInt8 *get_invite_key(void);
UInt8 eval_encryption(on_data_t ENCRYPT_TYPE);
void reset_client(void);

void turn_on_tx_led(void)
{
    TURN_ON(TX_LED);
    ont_set_timer(TX_LED_TIMER, EVAL_LED_ON_TIME);
} // turn_on_tx_led //


void turn_on_rx_led(void)
{
    TURN_ON(RX_LED);
    ont_set_timer(RX_LED_TIMER, EVAL_LED_ON_TIME);
} // turn_on_rx_led //

void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
}

void one_net_client_client_remove_device(void)
{
    reset_client();
} // one_net_client_client_remove_device //

void one_net_client_single_txn_status(const one_net_status_t STATUS, const UInt8 RETRY_COUNT, const UInt8 * const DATA, const one_net_raw_did_t * const DST)
{
} // eval_single_txn_status //

void one_net_client_stream_txn_status(const one_net_status_t STATUS, const one_net_raw_did_t * const DID)
{
} // one_net_client_stream_txn_status //


BOOL one_net_client_handle_stream_pkt(const UInt8 * PLD, const UInt16 LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    return FALSE;
} // one_net_client_handle_stream_pkt //

void one_net_client_joined_network(const one_net_raw_did_t * const RAW_DID, const one_net_raw_did_t * const MASTER_DID)
{
} // one_net_client_joined_network //

void one_net_client_block_txn_status(const one_net_status_t STATUS, const one_net_raw_did_t * const DID)
{
} // one_net_client_block_txn_status //

const UInt8 *one_net_client_next_payload(const UInt8 TYPE, UInt16 * len, const one_net_raw_did_t * const DST)
{
    return 0;
}  // one_net_client_next_payload //

BOOL one_net_client_handle_block_pkt(const UInt8 * PLD, const UInt16 LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    return TRUE;
} // one_net_client_handle_block_pkt //

BOOL one_net_client_txn_requested(const UInt8 TYPE, const BOOL SEND, const UInt16 DATA_TYPE, const UInt16 DATA_LEN, const one_net_raw_did_t * const DID) 
{
    return FALSE;
} // one_net_client_txn_requested //

BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD, const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    ona_msg_type_t msg_type;
    ona_msg_class_t msg_class;
    UInt8 src_unit, dst_unit;
    BOOL rv = FALSE;

    msg_type = get_msg_hdr(RX_PLD);
    msg_class = msg_type & ONA_MSG_CLASS_MASK;
    msg_type &= ~ONA_MSG_CLASS_MASK;

    switch(msg_type)
    {
        case ONA_SWITCH:
        {
            UInt16 status;

            // get src unit and dst unit from payload
            // dst_unit has value 1
            //
            if(ona_parse_switch(RX_PLD, &src_unit, &dst_unit, &status) !=
               ONS_SUCCESS || (dst_unit > ONE_NET_NUM_UNITS) || (dst_unit < 1))
            {
                break;
            } // if parsing the switch payload was not successful //
            switch(msg_class)
            {
                case ONA_QUERY:
                {
                    // units 1
                    if(((rv = get_sw_state(&status)) != FALSE) && ona_send_switch_status(dst_unit, src_unit, status, SRC_ADDR) == ONS_SUCCESS)
                    {
                        // Update the state of the pin that was checked since
                        // the message was successfully queued
                        g_sw = status;
                        rv = TRUE;
                    } // if sending the status was successful //
                    break;
                } // ONA_QUERY case //          
                default:
                {
                    break;
                } // default case //
            } // switch on msg class //
            break;
        } // SWITCH case //
        
        case ONA_UNIT_TYPE_COUNT:
        {
            if(msg_class != ONA_QUERY)
            {
                break;
            } // if an incorrect message class was received //

            if(ona_send_unit_type_count_status(SRC_ADDR) == ONS_SUCCESS)
            {
                rv = TRUE;
            } // if sending was successful //
            break;
        } // UNIT_TYPE_COUNT case //
        
        case ONA_UNIT_TYPE:
        {
            if((msg_class != ONA_QUERY) || (ona_parse_unit_type_query(RX_PLD, &dst_unit) != ONS_SUCCESS) || (ona_send_unit_type_status(dst_unit, SRC_ADDR) != ONS_SUCCESS))
            {
                break;
            } // if an incorrect message class was received || parsing failed //

            rv = TRUE;
            break;
        } // UNIT_TYPE case //
    
        case ONA_VOLTAGE:
        {
            voltage_status = READ_BATTERY_STATUS() >= LOW_BATTERY_THRESHOLD ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
            ona_send_voltage_simple_status(voltage_status, SRC_ADDR);
            rv = TRUE;
            break;
        } // VOLTAGE case //
    
        default:
        {
            break;
        } // default case //
    } // switch(msg_type) //
    return rv;
} // client handle_single_pkt //

BOOL get_sw_state(UInt16 *state)
{
    *state = SW ? ONA_OFF : ONA_ON;
    return TRUE;
}

UInt8 *get_invite_key(void)
{
    return(unsigned char *)(&DEFAULT_INVITE_KEY[0]); 
} // get_invite_key //

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

void reset_client(void)
{
    one_net_xtea_key_t * invite_key;

    // get the unique invite code for this device
    invite_key = (one_net_xtea_key_t *) get_invite_key();
    one_net_client_look_for_invite(invite_key, eval_encryption(ON_SINGLE), eval_encryption(ON_STREAM));
    
} // reset_client //

void check_switch(void)
{
    UInt16 state;

    get_sw_state(&state);
    if (g_sw != state)
    {
        g_sw = state;
        ona_send_switch_command(0, 0, g_sw, 0);
    }
    
}

void main(void)
{
    tick_t tpulse;
    
    INIT_PROCESSOR();
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();
    INIT_TICK();

    tpulse = get_tick_count();
    
    ENABLE_GLOBAL_INTERRUPTS();
    
    get_sw_state(&g_sw);
    voltage_status = READ_BATTERY_STATUS() >= LOW_BATTERY_THRESHOLD ? ONA_VOLTAGE_GOOD : ONA_VOLTAGE_BAD;
    
    reset_client();
    TURN_ON(RX_LED);
    ont_set_timer(RX_LED_TIMER, 1000);
    TURN_ON(TX_LED);
    ont_set_timer(TX_LED_TIMER, 1000);
    
    while(1)
    {
        if (TICK_DIFF(tpulse, get_tick_count()) > 5000)
        {
            tpulse = get_tick_count();
            turn_on_rx_led();
        }
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

        check_switch();
        one_net_client();
    } // main loop //
}