//! \addtogroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file simple_client__eval.c
    \brief The ONE-NET (simple) evaluation project.

    This is the application that runs on the simple ONE-NET evaluation boards.
    These boards contain small processors (16k) and only contain simple CLIENT
    functionality.
*/

#include "simple_one_net_eval.h"

#include "one_net.h"
#include "one_net_client.h"
#include "one_net_client_net.h"
#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "pal.h"
#include "tal.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{

//! The default keep alive interval (in ticks).
#define DEFAULT_KEEP_ALIVE_INTERVAL MS_TO_TICK(300000)


// Version
enum
{
    MAJOR_VER = 0,                  //!< The major version number
    MINOR_VER = 1                   //!< The minor version number
};

enum
{
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
};


//! The raw CLIENT DIDs for auto mode
static const one_net_raw_did_t RAW_AUTO_CLIENT_DID[NUM_AUTO_CLIENTS] =
{
    {0x00, 0x20}, {0x00, 0x30}, {0x00, 0x40}
};

//! The key used in the evaluation network ("protected")
const one_net_xtea_key_t EVAL_KEY = {0x02, 0x99, 0x33, 0x13, 0x97, 0x22, 0xAA,
  0xA4, 0xDD, 0xBE, 0xEF, 0x79, 0x00, 0x11, 0x3A, 0x20};

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

//! The index of the sid to use in the evaluation network.
static UInt8 eval_sid_idx = 0;

//! The node type the device is operating at
static UInt8 node_type;

//! @} ONE-NET_eval_pri_var
//                              PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

static void init_auto_client(const node_select_t CLIENT);

void client_eval(void);

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
BOOL mem_equal(const UInt8 * const LHS, const UInt8 * const RHS,
  const UInt16 LEN)
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
BOOL get_eval_encoded_nid(on_encoded_nid_t * const nid)
{
    static const on_encoded_nid_t eval_nid = {0xB4, 0xB4, 0xB4, 0xB4, 0xB4,
      0xB4};

    if(!nid)
    {
        return FALSE;
    } // if the parameter is invalid //

    one_net_memmove(nid, eval_nid, sizeof(on_encoded_nid_t));

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
BOOL get_eval_encoded_did(const node_select_t NODE,
  on_encoded_did_t * const did)
{
    if((NODE != AUTO_CLIENT1_NODE && NODE != AUTO_CLIENT2_NODE
      && NODE != AUTO_CLIENT3_NODE) || !did)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    on_encode(*did, RAW_AUTO_CLIENT_DID[NODE - AUTO_CLIENT1_NODE],
      sizeof(on_encoded_did_t));
    
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
BOOL get_eval_key(one_net_xtea_key_t * const key)
{
    if(!key)
    {
        return FALSE;
    } // if the parameter is invalid //
    
    one_net_memmove(*key, EVAL_KEY, sizeof(one_net_xtea_key_t));
    
    return TRUE;
} // get_eval_key //


/*!
    \brief Returns the encryption method to use for the given transfer type.
    
    This is a "protected" function.  This should be called when the evaluation
    network is first set up.
    
    \param[in] ENCRYPT_TYPE The type of transfer to return the encryption
      method for (single/block or stream).

    \return The encryption method to use for the given transfer type.
      ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE is returned if there was an error.
*/
UInt8 eval_encryption(const on_data_t ENCRYPT_TYPE)
{
    if(ENCRYPT_TYPE == ON_SINGLE || ENCRYPT_TYPE == ON_BLOCK)
    {
        return EVAL_SINGLE_BLOCK_ENCRYPTION;
    } // if single or block transfer //
    
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
UInt8 eval_data_rate(const node_select_t NODE)
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


void one_net_client_joined_network(const one_net_raw_did_t * const RAW_DID,
  const one_net_raw_did_t * const MASTER_DID)
{
} // one_net_client_joined_network //


BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR)
{
    return TRUE;
} // client handle_single_pkt //


void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 RETRY_COUNT, const UInt8 * const DATA,
  const one_net_raw_did_t * const DST)
{
} // one_net_client_single_txn_status //


void one_net_client_client_remove_device(void)
{
} // one_net_client_client_remove_device //


void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN)
{
} // one_net_client_save_settings //


//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_eval_pri_func
//! \ingroup ONE-NET_eval
//! @{

/*!
    \brief Initializes the device as a CLIENT in auto mode
    
    This is not static since it will be called from eval, but we don't want it
    called from anywhere else.
    
    \param CLIENT The CLIENT_NODE this device is supposed to operate as.

    \return void
*/
void init_auto_client(const node_select_t CLIENT)
{
    on_base_param_t * base_param;
    on_master_t * master;
    on_peer_t * peer;

    // The parameters used to initialize an existing network as if they were
    // stored in a continguous block of memory
    UInt8 param[sizeof(on_base_param_t) + sizeof(on_master_t)
      + sizeof(on_peer_t)];
    UInt8 i, j;

    // check the parameters
    if(CLIENT < AUTO_CLIENT1_NODE || CLIENT > AUTO_CLIENT3_NODE)
    {
        EXIT();
    } // if any of the parameters are invalid //
    
    // set the pointers
    base_param = (on_base_param_t *)param;
    master = (on_master_t *)(((UInt8 *)base_param) + sizeof(on_base_param_t));
    peer = (on_peer_t *)(((UInt8 *)master) + sizeof(on_master_t));

    // set up the base parameters
    base_param->version = EVAL_PARAM_VERSION;
    get_eval_encoded_nid((on_encoded_nid_t *)&(base_param->sid));
    get_eval_encoded_did(CLIENT,
      (on_encoded_did_t *)&(base_param->sid[ON_ENCODED_NID_LEN]));
    base_param->channel = eval_channel();
    base_param->data_rate = eval_data_rate(CLIENT);
    get_eval_key(&(base_param->current_key));
    base_param->single_block_encrypt = eval_encryption(ON_SINGLE);

    // set up the on_master_t parameters
    get_eval_encoded_did(MASTER_NODE, &(master->device.did));
    master->device.expected_nonce = 0;
    master->device.last_nonce = 0;
    master->device.send_nonce = 0;
    master->keep_alive_interval = eval_keep_alive();
    master->settings.master_data_rate = ONE_NET_DATA_RATE_38_4;
    master->settings.flags = ON_JOINED;
    
    // set up the peer parameters
    for(i = 0; i < ONE_NET_PEER_PER_UNIT * ONE_NET_NUM_UNITS; i++)
    {
        one_net_memmove(peer->dev[i].did, ON_ENCODED_BROADCAST_DID,
          sizeof(peer->dev[i].did));
    } // loop to initialize peer devices //

    for(i = 0; i < ONE_NET_NUM_UNITS; i++)
    {
        for(j = 0; j < ONE_NET_PEER_PER_UNIT; j++)
        {
            peer->unit[i][j].peer_dev_idx = 0xFFFF;
            peer->unit[i][j].peer_unit = ONE_NET_DEV_UNIT;
        } // inner loop to initialize peer units //
    } // outer loop to initialize peer units //
    
    base_param->crc = one_net_compute_crc((UInt8 *)base_param
      + sizeof(base_param->crc), sizeof(param) - sizeof(base_param->crc),
      ON_PARAM_INIT_CRC, ON_PARAM_CRC_ORDER);

    // initialize the ONE-NET CLIENT
    one_net_client_init(param, sizeof(param));
} // init_auto_client //


void main(void)
{
    INIT_PORTS();
    TAL_INIT_TRANSCEIVER();

    // initialize the processor speed
    INIT_PROCESSOR();

    INIT_PORTS_LEDS();
    INIT_TICK();

    ENABLE_GLOBAL_INTERRUPTS();

    // Get the node type
    if((SW_ADDR_SELECT1 == 1) && (SW_ADDR_SELECT2 == 0))
    {
        node_type = AUTO_CLIENT1_NODE;
        init_auto_client(node_type);
    } // if CLIENT1 //
    else if((SW_ADDR_SELECT1 == 0) && (SW_ADDR_SELECT2 == 1))
    {
        node_type = AUTO_CLIENT2_NODE;
        init_auto_client(node_type);
    } // else if CLIENT2 //
    else
    {
        node_type = AUTO_CLIENT3_NODE;
        init_auto_client(node_type);
    } // else CLIENT3 //

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

        one_net_client();
    } // main loop //
} // main //

//! @} ONE-NET_eval_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================

//! @} ONE_NET_eval
