#ifndef _ONE_NET_EVAL_H
#define _ONE_NET_EVAL_H

#include "config_options.h"


//! \defgroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.h
    \brief The ONE-NET evaluation project.

    Declarations for the ONE-NET evaluation project.
*/

#include "one_net.h"
#include "one_net_types.h"
/*#ifdef _ONE_NET_VERSION_2_X
    #include "one_net_application.h" // for ona_msg_class_t
#endif*/

//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{

enum
{
#ifdef _AUTO_MODE
    //! The number of CLIENTS in auto mode
    NUM_AUTO_CLIENTS = 3,
#endif
    
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
    EVAL_PARAM_VERSION = 0x0002
};

enum
{
    //! Offset within the data portion of the manufacturing data flash segment
    //! of the SID.
    EVAL_DF_SID_OFFSET =            0,

    //! Size of the SID field within the manufacturing data flash segment.
    EVAL_DF_SID_SIZE =              6,

    //! Offset withing the data portion of the manufacturing data flash segment
    //! of the invite key.
    EVAL_DF_INVITE_KEY_OFFSET =     EVAL_DF_SID_SIZE,

    //! Size of the input invite key. The user enters an 8 byte field
    //! that is doubled to create the real invite key so that the user has
    //! fewer characters to type when issuing the CLI invite command.
    EVAL_DF_INVITE_INPUT_KEY_SIZE = 8,

    //!
};

//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{

// Version
enum
{
    MAJOR_VER = ONE_NET_VERSION_MAJOR,  //!< The major version number
#ifndef _EVAL_0005_NO_REVISION
    MINOR_VER = ONE_NET_VERSION_MINOR,  //!< dje: The minor version number
#else
    MINOR_VER = ONE_NET_VERSION_MINOR*10, //!< dje: The minor version number
#endif
    REVISION_NUM = ONE_NET_VERSION_REVISION,
    BUILD_NUM = ONE_NET_VERSION_BUILD

};


#ifdef _AUTO_MODE
	//! Mode Select switch values
	typedef enum
	{
	    AUTO_MODE,                      //!< Running auto mode evaluation
	    SERIAL_MODE                     //!< Running serial mode evaluation
	} mode_select_t;
#endif


//! Node Select switch values
typedef enum
{
#ifdef _SNIFFER_MODE
    SNIFFER_NODE,                   //!< Device is a SNIFFER
#endif
    MASTER_NODE,                    //!< Device is a MASTER
#ifdef _AUTO_MODE
    CLIENT_NODE,                    //!< Device is a CLIENT device
    AUTO_CLIENT1_NODE,              //!< Uses first CLIENT addr in auto mode
    AUTO_CLIENT2_NODE,              //!< Uses second CLIENT addr in auto mode
    AUTO_CLIENT3_NODE               //!< Uses third CLIENT addr in auto mode
#else
    CLIENT_NODE                    //!< Device is a CLIENT device
#endif
} node_select_t;


//! structure to keep corresponding raw & encoded sids together.
typedef struct
{
    one_net_raw_sid_t raw;          //!< Raw SID
    on_encoded_sid_t encoded;       //!< Encoded SID.
} eval_sid_t;


// copied from switch.h.  Not using switch to save on size, plus the eval board
// has both the MASTER & CLIENT, which the app doesn't handle both at once.
typedef enum
{
    ONA_OFF = 0x00,                 //!< Switch Off
    ONA_ON = 0x01,                  //!< Switch On
    ONA_TOGGLE = 0x02               //!< Switch Toggle (on->off, off->on)
} switch_status_t;


/*!
    \brief Holds the functionality and state for the user pins.
*/
typedef struct
{
    UInt8 pin_type;                 //!< Functionality type (see oncli_pin_t)
    UInt8 old_state;                //!< The last state of the pin
} user_pin_t;

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

BOOL mem_equal(const UInt8 *lhs, const UInt8 *rhs, UInt16 len);

// functions for retrieving the evaluation network parameters
BOOL get_eval_encoded_nid(on_encoded_nid_t *nid);
BOOL get_eval_encoded_did(node_select_t NODE, on_encoded_did_t *did);
BOOL get_raw_master_did(one_net_raw_did_t *did);
BOOL get_eval_key(one_net_xtea_key_t *key);
UInt8 * get_invite_key(void);
one_net_raw_sid_t * get_raw_sid(void);

#ifdef _STREAM_MESSAGES_ENABLED
    BOOL get_eval_stream_key(one_net_xtea_key_t *stream_key);
#endif
UInt8 eval_encryption(on_data_t ENCRYPT_TYPE);
UInt8 eval_channel(void);
UInt8 eval_data_rate(node_select_t NODE);
UInt32 eval_keep_alive(void);
UInt32 eval_fragment_delay(UInt8 PRIORITY);
UInt8 eval_client_features(node_select_t CLIENT);
UInt8 eval_client_flag(node_select_t CLIENT);

UInt16 did_to_u16(const one_net_raw_did_t *DID);

BOOL eval_txn_requested(UInt8 TYPE, BOOL SEND, UInt16 DATA_TYPE, 
                        UInt16 DATA_LEN, const one_net_raw_did_t *DID);

UInt8 *eval_next_payload(UInt8 TYPE, UInt16 * len, 
                         const one_net_raw_did_t *DST);

void eval_block_txn_status(one_net_status_t STATUS,
                           const one_net_raw_did_t *DID);

/*#ifndef _ONE_NET_VERSION_2_X*/
void eval_handle_single(const UInt8 *RX_PLD, UInt16 RX_PLD_LEN,
                        const one_net_raw_did_t *SRC_ADDR);
/*#else
BOOL eval_handle_single(ona_msg_class_t msg_class, ona_msg_type_t msg_type, 
         UInt8 src_unit, UInt8 dst_unit, UInt16* msg_data,
         const one_net_raw_did_t* const SRC_ADDR, BOOL* useDefaultHandling,
		 on_nack_rsn_t* nack_reason);
#endif*/


void print_packet(const UInt8 *TXN_STR, const UInt8 *RX_PLD, UInt16 LEN, 
                  const one_net_raw_did_t *SRC_ADDR);

void eval_single_txn_status(one_net_status_t STATUS, const UInt8 *DATA, 
                            const one_net_raw_did_t *DST);

one_net_status_t send_switch_command(UInt8 SWITCH_STATUS, UInt8 SRC_UNIT, 
                             UInt8 DST_UNIT, const one_net_raw_did_t *DST);

#ifdef _ENABLE_CLIENT_PING_RESPONSE
    one_net_status_t send_simple_text_command(UInt8 *TEXT, UInt8 SRC_UNIT, 
        UInt8 DST_UNIT, const one_net_raw_did_t *DST);
#endif


//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE_NET_eval

#endif // _ONE_NET_EVAL_H //
