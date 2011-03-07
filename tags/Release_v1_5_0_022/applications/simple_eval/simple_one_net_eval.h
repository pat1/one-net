#ifndef _ONE_NET_EVAL_H
#define _ONE_NET_EVAL_H

//! \defgroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.h
    \brief The ONE-NET evaluation project.

    Declarations for the ONE-NET evaluation project.
*/

#include "one_net.h"
#include "one_net_types.h"


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{

enum
{
    //! The number of CLIENTS in auto mode
    NUM_AUTO_CLIENTS = 3,
    
    //! Number of bits to shift a value to get the proper raw did
    RAW_DID_SHIFT = 4
};

enum
{
    //! The version of the parameters being set.  This is not set to
    //! ON_PARAM_VERSION since if the parameter version changes, we want to
    //! make sure that the application's initialization of the structures
    //! also change, and that would be harder to do if this was set to
    //! ON_PARAM_VERSION since that version number is changed inside ONE-NET
    //! when the structures change, which doesn't mean the app has changed to
    //! handle the new structure version
    EVAL_PARAM_VERSION = 0x0001
};

//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{

//! Mode Select switch values
typedef enum
{
    AUTO_MODE,                      //!< Running auto mode evaluation
    SERIAL_MODE                     //!< Running serial mode evaluation
} mode_select_t;


//! Node Select switch values
typedef enum
{
    SNIFFER_NODE,                   //!< Device is a SNIFFER
    MASTER_NODE,                    //!< Device is a MASTER
    CLIENT_NODE,                    //!< Device is a CLIENT device
    AUTO_CLIENT1_NODE,              //!< Uses first CLIENT addr in auto mode
    AUTO_CLIENT2_NODE,              //!< Uses second CLIENT addr in auto mode
    AUTO_CLIENT3_NODE               //!< Uses third CLIENT addr in auto mode
} node_select_t;


//! structure to keep corresponding raw & encoded sids together.
typedef struct
{
    one_net_raw_sid_t raw;          //!< Raw SID
    on_encoded_sid_t encoded;       //!< Encoded SID.
} eval_sid_t;

//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{

//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{

UInt8 device_type(void);
UInt8 mode_type(void);
BOOL mem_equal(const UInt8 * const LHS, const UInt8 * const RHS,
  const UInt16 LEN);

// functions for retrieving the evaluation network parameters
BOOL get_eval_encoded_nid(on_encoded_nid_t * const nid);
BOOL get_eval_encoded_did(const node_select_t NODE,
  on_encoded_did_t * const did);
BOOL get_eval_key(one_net_xtea_key_t * const key);
BOOL get_eval_stream_key(one_net_xtea_key_t * const stream_key);
UInt8 eval_encryption(const on_data_t ENCRYPT_TYPE);
UInt8 eval_channel(void);
UInt8 eval_data_rate(const node_select_t NODE);
UInt32 eval_keep_alive(void);
UInt32 eval_fragment_delay(const UInt8 PRIORITY);
UInt8 eval_client_features(const node_select_t CLIENT);
UInt8 eval_client_flag(const node_select_t CLIENT);

UInt16 did_to_u16(const one_net_raw_did_t * const DID);

BOOL eval_txn_requested(const UInt8 TYPE, const BOOL SEND,
  const UInt16 DATA_TYPE, const UInt16 DATA_LEN,
  const one_net_raw_did_t * const DID);
const UInt8 * eval_next_payload(const UInt8 TYPE, UInt16 * len,
  const one_net_raw_did_t * const DST);
void eval_block_txn_status(const one_net_status_t STATUS,
  const one_net_raw_did_t * const DID);

void eval_handle_single(const UInt8 * RX_PLD, const UInt16 RX_PLD_LEN,
  const one_net_raw_did_t * const SRC_ADDR);
void print_packet(const char * const TXN_STR, const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR);
void eval_single_txn_status(const one_net_status_t STATUS,
  const UInt8 * const DATA, const one_net_raw_did_t * const DST);

//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE_NET_eval

#endif // _ONE_NET_EVAL_H //

