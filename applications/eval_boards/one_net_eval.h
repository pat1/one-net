#ifndef _ONE_NET_EVAL_H
#define _ONE_NET_EVAL_H



//! \defgroup ONE-NET_eval ONE_NET Evaluation
//! @{

/*!
    \file one_net_eval.h
    \brief The ONE-NET evaluation project.

    Declarations for the ONE-NET evaluation project.
*/


#include "one_net_application.h"
#include "one_net_port_const.h"
#include "one_net_acknowledge.h"



//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_eval_const
//! \ingroup ONE-NET_eval
//! @{



#ifdef AUTO_MODE
extern const on_raw_did_t RAW_AUTO_CLIENT_DID[];
extern const on_encoded_did_t ENC_AUTO_CLIENT_DID[];
#endif

#if defined(AUTO_MODE) || defined(ONE_NET_MASTER)
extern const UInt8 DEFAULT_RAW_NID[];
#endif



//! @} ONE-NET_eval_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_eval_typedefs
//! \ingroup ONE-NET_eval
//! @{
    
    
enum
{
    #ifdef AUTO_MODE
    //! The number of clients in AUTO mode.
    NUM_AUTO_CLIENTS = 3,
    #endif
    
    //! The channel for Auto Mode
    DEFAULT_EVAL_CHANNEL = 1,
};    


/*!
    \brief Holds the functionality and state for the user pins.
*/
typedef struct
{
    on_pin_state_t pin_type;        //!< Functionality type
    UInt8 old_state;                //!< The last state of the pin
} user_pin_t;


//! State machine for dealing with the user pins.
enum
{
    CHECK_USER_PIN,               //!< State to check user pins for changes
    SEND_USER_PIN_INPUT           //!< State to send user pin changes to peers
};




//! @} ONE-NET_eval_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_eval_pub_var
//! \ingroup ONE-NET_eval
//! @{

extern user_pin_t user_pin[NUM_USER_PINS];
#ifdef SNIFFER_MODE
extern BOOL in_sniffer_mode;
#endif


extern UInt8 user_pin_state;
extern UInt8 user_pin_src_unit;



//! The key used in the evaluation network
extern const one_net_xtea_key_t EVAL_KEY;

//! Default invite key to use if no manufacturing data (SID and invite key) segment
//! is found in data flash.
extern const UInt8 DEFAULT_INVITE_KEY[];
                                     
#if defined(AUTO_MODE) || defined(ONE_NET_MASTER)
//! Default NID to use if no NID is found in the manufacturing data segment
//! of data flash.
extern const UInt8 DEFAULT_RAW_NID[];

//! Default SID to use if no NID is found in the manufacturing data segment
//! of data flash.
extern const on_raw_sid_t DEFAULT_RAW_SID;
#endif



//! Pointer to the device dependent (MASTER, CLIENT, SNIFF) function that
//! should be called in the main loop
extern void(*node_loop_func)(void);

//! @} ONE-NET_eval_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_eval_pub_func
//! \ingroup ONE-NET_eval
//! @{



#ifdef ONE_NET_MASTER
// these functions are in master_eval.c
void master_eval(void);
#ifdef NON_VOLATILE_MEMORY
void init_serial_master(BOOL load_nv_memory, SInt8 channel);
#else
void init_serial_master(SInt8 channel);
#endif
#endif
#ifdef ONE_NET_CLIENT
void client_eval(void); // in client_eval.c
void init_serial_client(void); // in client_eval.c
#endif
#ifdef SNIFFER_MODE
void sniff_eval(void); // in sniff_eval.c
#endif
#ifdef AUTO_MODE
#ifdef ONE_NET_MASTER
void init_auto_master(void);
#endif
#ifdef ONE_NET_CLIENT
void init_auto_client(UInt8 index);
#endif
#endif


void initialize_default_pin_directions(BOOL is_master);
void check_user_pins(void);
void disable_user_pins(void);
void send_user_pin_input(void);


/*!
    \brief Handles the received single packet.
	
    \param[in] raw_pld the raw single payload.  Note that ack_nack.ayload below
      also points to this parameter, so the memory can be changed.  Therefore,
      if the application code needs this payload to NOT change after sending
      a response, it must copy it.
    \param[in/out] msg_hdr of packet / response -- only the pid portion of
      this parameter should be changed.  The message type is irrelevant for
      a response message (the equivalent of a "message type" can be specified
      in the "handle" portion of the "ack_nack" parameter below.  The message
      id should not be changed.  The pid should be changed to the type of ACK
      or NACK that should be sent.
	\param[in] src_did the raw address of the source
	\param[in] repeater_did the raw address of the repeater, if any.  This will
               always equal the source id if 1) this is not a multi-hop message
               and 2) this is not a "forwarded" message by either the peer
               manager or the application code.  Most applications will ignore
               this parameter.
	\param[out] ack_nack: Contains three parts...
                  nack_reason... if nacking, the reason for the NACK.  Irrelevant for ACKs
	            handle... if including a payload, how the payload should be parsed.
                  For example, if this is a ffast query or command response that contains
                  a status message, the handle should be set to ON_ACK_STATUS.
                  If the message contains a time, this should be set to
                  ON_ACK_TIME_MS or ON_NACK_MS.  If the message contains an
                  application-specific value, this should be set to ON_ACK_VALUE
                  or ON_NACK_VALUE.  If simply sending an ACK or a NACK without
                  any payload, this parameter can be ignored.
                payload...
                  The "payload" of the NACK or ACK.  Irrelevant if the handle
                  is not set.  Note that this also points to raw_pld to save
                  space.
    \param[in] hops the number of hops it took to get here.  Can be ignored by
                    most applications.
    \param[in/out] max_hops in --> the maximum number of hops that was set for the message
	                        out --> the maximum number of hops to use for a response.  Can
                                    generally be ignored if you want the maximum number of
                                    hops to remain unchanged.
                 
    \return ON_MSG_RESPOND if an ACK or a NACK should be sent back.
            ON_MSG_IGNORE if no reponse should occur.
*/
#ifndef ONE_NET_MULTI_HOP
on_message_status_t eval_handle_single(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack);
#else
on_message_status_t eval_handle_single(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack,
  UInt8 hops, UInt8* const max_hops);
#endif


/*!
    \brief Callback for the application to handle an ack or a nack.

    This function is application dependent.  It is called by ONE-NET when a
    device receives an ACK or a NACK.  For many applications, no ACK/ NACK
    handling is needed and the application waits for the entire transaction
    to be completed.  In this case (which is very common), the function should
    simply be an empty function that returns ON_MSG_CONTINUE, which tells
    ONE-NET to use its default handling.
    
    For NON-default handling, this function allows the application code to
    change a nack reason, change the number of retries, change the message it
    is sending, change the number of hops, or abort the transaction.  Even if
    nothing is changed, it informs the application code of what is going on
    with each individual response.  The application may be keeping tracking
    of how many ACKs versus NACKs it gets for whatever reason.
    
    
    "Fast query" messages and "command" messages or any other message which
    involves response messages containing status messages, may wish to call
    the one_net_master_handle_single_pkt and pass it the ACK payload as
    the payload.  This is generally how this is handled.
    one_net_master_handle_single_pkt should look at the message class and,
    if it is ONA_STATUS_FAST_QUERY_RESP or ONA_STATUS_COMMAND_RESP, this
    tells the function that the message was a response to a command or a fast
    query.
	

    \param[in/out] raw_pld Raw payload of the ORIGINAL message so that the
       application code can see what is being ACK'd or NACK'd.  Changable, but
       generally is NOT changed.
    \param[in/out] msg_hdr PID, Message ID, and Message Type(i.e. admin,
      app, etc.) of the ORIGINAL message.  This is changable, but like the
      raw payload, is generally NOT changed.  In particular, the message id
      is assigned by ONE-NET, so it should only be changed by ONE-NET.  If the
      application code needs to change the message id to a new one, it should
      call the function get_new_message_id() and use its return value.  Again,
      use caution when changing the message id.  There is usually no need to
      change it.
    \param[in] resp_msg_hdr The PID and Message ID of the response. Generally
      the message TYPE is irrelevant in a response.  The "handle" of the ACK
      or the NACK should be looked at instead for how to parse.
    \param[in] resp_msg_hdr The PID and Message ID of the response. Generally
      the message TYPE is irrelevant in a response.  The "handle" of the ACK
      or the NACK should be looked at instead for how to parse.  The message
      ID in the response should match the message id in the original message.
      If it does not, the original raw payload should be considered invalid.
      ONE-NET will consider this an invalid response if the message
      IDs do not match.  However, this function is still called to notify
      the application code.  If the message IDs DO NOT match, however, any
      changes made in this function will be ignored by ONE-NET.
    \param[in/out] resp_ack_nack The response (ACK or NACK), possibly with a
      payload and a NACK reason.  The "handle" will describe how the payload
      should be interpreted.  The nack reason can be changed and possibly
      a time in the payload if the nack reason involves a time that ONE-NET
      understands (i.e. length of time to pause).
    \param[in] resp_ack_nack The response (ACK or NACK), possibly with a
      payload and a NACK reason.  The "handle" will describe how the payload
      should be interpreted.  See the description of ack_nack in the
      one_net_master_handle_single_pkt for more details of how this should
      be interpreted.  This function can change an ACK to a NACK or vice versa
      and can also change the NACK reason.
	\param[in] src_did the raw address of the source
	\param[in] repeater_did the raw address of the repeater, if any.  This will
               always equal the source id if 1) this is not a multi-hop message
               and 2) this is not a "forwarded" message by either the peer
               manager or the application code.  Most applications will ignore
               this parameter.
    \param[in/out] retries The number of times this message has been sent.
      Generally this is not changed, but it can be.
    \param[in] hops the number of hops it took to get here.  Can be ignored by
                    most applications.
    \param[in/out] max_hops in --> the maximum number of hops that was set for
                                   the message
	                       out --> the maximum number of hops to use.  Can
                                   generally be ignored if you want the maximum
                                   number of hops to remain unchanged.
	
    \return ON_MSG_CONTINUED If ONE-NET should proceed with any further handling
              of the transaction
            ON_MSG_IGNORE to ignore this response and treat it at as if it had
              never been received
            ON_MSG_SUCCESS, ON_MSG_FAIL, ON_MSG_ABORT, or an
              application-specific code if this transaction should be
              terminated prematurely.  The nack reason (if it is to be
              changed), should be set here because this return code will
              result in a call to a callback function with both the NACK
              reason and this return code passed as parameters.
*/
#ifndef ONE_NET_MULTI_HOP
on_message_status_t eval_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries);
#else
on_message_status_t eval_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries,
  UInt8 hops, UInt8* const max_hops);
#endif



/*!
    \brief The status of a single transaction.

    Callback to report the status of sending an application single data packet.

    \param[in] status The status of the transaction.
    \param[in] retry_count The number of times that the packet had to be sent.
    \param[in] msg_hdr message id, message type, and pid of the message.
    \param[in] data The data that was sent.
    \param[in] dst The raw did of where the packet was sent.
    \param[in] ack_nack The reason for failure, if relevant.  A response
               payload, if relevant.
    \param[in] hops Number of hops it took to deliver the message, if
               known and relevant.  Negative number implies unknown or
               not relevant / reliable.
    \return void
*/
#ifndef ONE_NET_MULTI_HOP
void eval_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack);
#else
void eval_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack, SInt8 hops);
#endif



#ifdef AUTO_MODE
one_net_status_t send_simple_text_command(const char* text, UInt8 src_unit, 
  UInt8 dst_unit, const on_encoded_did_t* const enc_dst);
#endif
one_net_status_t send_switch_status_change_msg(UInt8 src_unit, 
  UInt8 status, UInt8 dst_unit, const on_encoded_did_t* const enc_dst);



//! @} ONE-NET_eval_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE_NET_eval

#endif // _ONE_NET_EVAL_H //
