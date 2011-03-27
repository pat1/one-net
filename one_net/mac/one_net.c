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

#include "one_net.h"

#include "one_net_crc.h"
#include "one_net_encode.h"
#include "one_net_port_specific.h"
#include "one_net_timer.h"
#include "one_net_prand.h"
#include "one_net_xtea.h"


#ifdef _ONE_NET_DEBUG
    #include "oncli.h"
    #include "oncli_str.h"
    #include "uart.h"
#endif

#ifdef _ONE_NET_DEBUG_STACK
    #include "uart.h"
#endif

#ifdef _ONE_NET_MH_CLIENT_REPEATER
    #ifndef _ONE_NET_MULTI_HOP
        #error "Need to define Multi_Hop if _ONE_NET_MH_CLIENT_REPEATER is defined!"
    #endif // ifndef _ONE_NET_MULTI_HOP //
    #ifdef _ONE_NET_SIMPLE_CLIENT
        #error "A Multi-Hop repeater cannot be a SIMPLE_CLIENT!"
    #endif // ifdef _ONE_NET_SIMPLE_CLIENT //
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_const
//! \ingroup ONE-NET
//! @{

#ifdef _ONE_NET_DEBUG_STACK
    UInt8 one_net_debug_stack_flags = 0xff;
#endif

//! Preamble
static const UInt8 PREAMBLE[] = {0x55, 0x55, 0x55};

//! Start of Frame
static const UInt8 SOF[] = {0x33};

enum
{
    ON_XTEA_8_ROUNDS = 8,           //!< 8 rounds of XTEA
    ON_XTEA_32_ROUNDS = 32          //!< 32 rounds of XTEA
};

#ifdef _ONE_NET_USE_ENCODING
const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0xB4, 0xB4};
#else
const on_encoded_did_t ON_ENCODED_BROADCAST_DID = {0x00, 0x00};
#endif



// Derek_S - adding a raw broadcase did constant
const one_net_raw_did_t ON_RAW_BROADCAST_DID = {0x00, 0x00};

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
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_pri_var
//! \ingroup ONE-NET
//! @{

//! Packet Handlers
#ifndef _ONE_NET_SIMPLE_DEVICE
    static on_pkt_hdlr_set_t pkt_hdlr = {0, 0, 0, 0, 0, 0, 0};
#else
    static on_pkt_hdlr_set_t pkt_hdlr = {0, 0, 0};
#endif // #ifndef _ONE_NET_SIMPLE_DEVICE //

#ifdef _IDLE
    //! Whether the current state is allowed to be changed
    static BOOL allow_set_state = TRUE;
#endif

//! The current state.  This is a "protected" variable.
on_state_t on_state = ON_INIT_STATE;





//! The base parameters for the device
on_base_param_t * on_base_param = 0;

//! Keep track of the number of successful data rate test packets
static UInt8 data_rate_result;

#ifndef _ONE_NET_SIMPLE_DEVICE
    //! Indicates when a block transaction has been completed
    static BOOL block_complete = FALSE;

    //! Points to the stream currently being sent.  This is incase the device is
    //! waiting for a response to the data it sent and has to send a single data
    //! nack, in which case we want it to go back to waiting for the stream
    //! response.
    static on_txn_t * cur_stream = 0;
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //

#ifdef _ONE_NET_MH_CLIENT_REPEATER
    // fill in the preamble in the Multi-Hop packet to be sent.  The rest will
    // be filled in when the received Multi-Hop packet is read in over the rf
    // interface.
    static UInt8 mh_pkt[ONE_NET_MAX_ENCODED_PKT_LEN] = {0x55, 0x55, 0x55, 0x33};

    // Transaction for forwarding on MH packets.  If the number of MH packets
    // is changed, need to update the number of multi-hop packets value in
    // one_net_timer.h.
    static on_txn_t mh_txn = {ONE_NET_LOW_PRIORITY, 0, 0, ON_INVALID_MSG_TYPE,
      TRUE, ONT_MH_TIMER, 0, 0, 0, sizeof(mh_pkt), mh_pkt};
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

// packet reception functions
#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const SRC_DID, UInt8 * data);
#else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const SRC_DID);
#endif // else _ONE_NET_MH_CLIENT_REPEATER not defined //
static one_net_status_t rx_single_resp_pkt(on_txn_t ** txn);
static one_net_status_t rx_single_txn_ack(on_txn_t ** txn);
#ifndef _ONE_NET_SIMPLE_DEVICE
    static one_net_status_t rx_block_resp_pkt(on_txn_t ** txn);
    static one_net_status_t rx_block_txn_ack(on_txn_t ** txn);
    static one_net_status_t rx_stream_resp_pkt(on_txn_t ** txn);
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //
#ifdef _ONE_NET_MULTI_HOP
    static one_net_status_t rx_data_rate(on_txn_t * const txn,
      const BOOL RECEIVER, UInt8 * const hops);
#else // ifdef _ONE_NET_MULTI_HOP //
    static one_net_status_t rx_data_rate(on_txn_t * const txn,
      const BOOL RECEIVER);
#endif // else _ONE_NET_MULTI_HOP is not defined //
static one_net_status_t rx_single_data(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
#ifndef _ONE_NET_SIMPLE_DEVICE
    static one_net_status_t rx_block_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
    static one_net_status_t rx_stream_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn);
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //
static one_net_status_t rx_payload(UInt8 * const raw_pld,
  const UInt8 ENCODED_LEN);
static one_net_status_t rx_nonces(UInt8 * const txn_nonce,
  UInt8 * const next_nonce);
  
#ifdef _ONE_NET_VERSION_2_X
// temporary function while porting to 2.0
/*static one_net_status_t rx_nonces_2_X(UInt8 * const txn_nonce, 
  UInt8 * const next_nonce, on_nack_rsn_t* const nack_reason,
  const one_net_xtea_key_t * const key, const on_data_t type);*/
static one_net_status_t rx_nonces_2_X(UInt8 * const txn_nonce,
  UInt8 * const next_nonce);
#endif
  

#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t repeat_mh_pkt(on_txn_t ** txn);
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_pub_func
//! \ingroup ONE-NET
//! @{

/*!
    \brief Initializes ONE-NET.

    \param[in] PKT_HDLR Contains the necessary callbacks ONE-NET will need
      to make.

    \return void
*/
void one_net_init(const on_pkt_hdlr_set_t * const PKT_HDLR)
{
    one_net_set_channel(on_base_param->channel);

    one_net_memmove(&pkt_hdlr, PKT_HDLR, sizeof(pkt_hdlr));
} // one_net_init //


#ifdef _IDLE
BOOL set_on_state(UInt8 new_on_state)
{
	if(!allow_set_state)
	{
		return FALSE;
	}
	on_state = new_on_state;
	return TRUE;
}


void set_allow_set_state(BOOL allow)
{
	allow_set_state = allow;
}
#endif


/*!
    \brief Compares the NID passed in to the devices own NID

    \param[in] NID The encoded NID to check if it is the devices.

    \return TRUE if the NIDs match.
            FALSE if the NIDs do not match.
*/
BOOL on_is_my_NID(const on_encoded_nid_t * const NID)
{
    UInt16 i;

    if(!NID)
    {
        return FALSE;
    } // if parameter is invalid //

    for(i = 0; i < ON_ENCODED_NID_LEN; i++)
    {
        if((*NID)[i] != on_base_param->sid[i])
        {
            return FALSE;
        } // if the NIDs don't match //
    } // loop through the encoded NID bytes //

    return TRUE;
} // on_is_my_NID //


/*!
    \brief Checks whether a did is a broadcast did / uninitialized
	
	Note that the DID may either be raw or encoded, hence the parameter is UInt8*
	rather than on_encoded_did_t* or one_net_raw_did_t*

    \param[in] did The did to check

    \return TRUE if did is a broadcast did
            FALSE if did is not a broadcast did
*/
BOOL did_is_broadcast(const UInt8* const did)
{
    // sometimes "encoded" dids are stored as uninitialized ({0, 0}).  We want to
	// make sure that these are interpreted as "not taken"/ unitialized / broadcast
	if(!did)
	{
        return FALSE;
	}
	if(did[0] == ON_ENCODED_BROADCAST_DID[0] && did[1] == ON_ENCODED_BROADCAST_DID[1])
	{
        return TRUE;
	}
	if(did[0] == ON_RAW_BROADCAST_DID[0] && did[1] == ON_RAW_BROADCAST_DID[1])
	{
        return TRUE;
	}
	
	return FALSE;
}


/*!
    \brief Compares two encoded dids and sees which is "smaller"

    A broadcast did is considered a "large" id.
	One DID is smaller than the other if the raw did is smaller.
	
    \param[in] enc_did1 First encoded did to compare
    \param[in] enc_did2 Second encoded did to compare
    
    \return negative number if did1 is "smaller" than did2
            postive number if did1 is "larger" than did2
            0 if did1 and did2 have the same values
*/
int enc_did_cmp(const on_encoded_did_t* const enc_did1, const on_encoded_did_t* const enc_did2)
{
	// TODO - check for valid parameters, check for return values
    one_net_raw_did_t raw_did1, raw_did2;
	BOOL did1IsBroadcast, did2IsBroadcast;
	
	if(!enc_did1 || !enc_did2)
	{
        // TODO - Return value of this should be what?  Just return 0 for now
		return 0;
	}
	
	did1IsBroadcast = did_is_broadcast(*enc_did1);
	did2IsBroadcast = did_is_broadcast(*enc_did2);

    if(did1IsBroadcast && did2IsBroadcast)
	{
	    return 0;
	}
	else if(did1IsBroadcast)
	{
		return 1;
	}
	else if(did2IsBroadcast)
	{
		return -1;
	}
		
	// TODO - what if these don't decode?
    on_decode(raw_did1, *enc_did1, ON_ENCODED_DID_LEN);
    on_decode(raw_did2, *enc_did2, ON_ENCODED_DID_LEN);

	if(raw_did1[0] == raw_did2[0])
	{
		return (int) raw_did1[1] - (int) raw_did2[1];
	}

    return (int) raw_did1[0] -  (int) raw_did2[0];
}

/*!
    \brief Compares two encoded Device IDs.

    \param[in] LHS The left hand side of the compare equation.
    \param[in] RHS The right hand side of the compare equation.

    \return TRUE if the DIDs match.
            FALSE if the DIDs do not match.
*/
BOOL on_encoded_did_equal(const on_encoded_did_t * const LHS,
  const on_encoded_did_t * const RHS)
{
	if(!LHS || !RHS)
	{
        return FALSE;
	}
    return (enc_did_cmp(LHS, RHS) == 0);
} // on_encoded_did_equal //



/*!
    \brief Checks the DID to see if it is one that this device listens to.

    \param[in] The encoded DID to check.

    \return ONS_SUCCESS if DID is for this device.
            ONS_BROADCAST_ADDR if DID is the broadcast address
            ONS_DID_FAILED if the DID is unknown
*/
one_net_status_t on_validate_dst_DID(const on_encoded_did_t * const DID)
{
    if(on_encoded_did_equal(DID, (const on_encoded_did_t * const)
      &(on_base_param->sid[ON_ENCODED_NID_LEN])))
    {
        return ONS_SUCCESS;
    } // if DID == my_did //

    if(on_encoded_did_equal(DID, &ON_ENCODED_BROADCAST_DID))
    {
        return ONS_BROADCAST_ADDR;
    } // if DID == broadcast //

    return ONS_DID_FAILED;
} // on_valid_dst_DID //


/*!
    \brief Encrypt the data passed in.

    data should be formatted such that the first byte is the location where
    the 8-bit crc is going to go, then the next N bytes are the data that is
    being encrypted, and there should be room for 1 extra byte on the end
    for the encryption type.  In short, data should be the format of the
    payload field for the appropriate data type.

    \param[in] DATA_TYPE Type of data to be sent.  (see on_data_t in one_net.h)
    \param[in/out] data The data to encrypt
    \param[in] KEY The XTEA key used to encrypt the data

    \return The status of the operation
*/
one_net_status_t on_encrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY)
{
    // the length of the payload in bytes and 6-8 bit blocks.
    UInt8 payload_len;

    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data || !KEY)
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    // get the size that the payload is supposed to be
    if(DATA_TYPE == ON_SINGLE)
    {
        payload_len = ON_RAW_SINGLE_PLD_SIZE;
    } // if single //
    else if(DATA_TYPE == ON_INVITE)
    {
        payload_len = ON_RAW_INVITE_SIZE;
    } // if client invite //
    #ifndef _ONE_NET_SIMPLE_DEVICE
        else
        {
            payload_len = ON_RAW_BLOCK_STREAM_PLD_SIZE;
        } // else block/stream //
    #else
        else
        {
            return ONS_BAD_PARAM;
        } // else invalid parameter //
    #endif // ifdef _ONE_NET_SIMPLE_DEVICE //

    // get the number of XTEA rounds
    if(DATA_TYPE != ON_STREAM)
    {
        switch(on_base_param->single_block_encrypt)
        {
            case ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE:
            {
                rounds = 0;
                break;
            } // no encryption //

            case ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32:
            {
                rounds = ON_XTEA_32_ROUNDS;
                break;
            } // xtea with 32 rounds //

            default:
            {
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch on encryption type //

        data[payload_len - 1] = on_base_param->single_block_encrypt;
    } // if not stream //
    #ifndef _ONE_NET_SIMPLE_DEVICE
        else
        {
            switch(on_base_param->stream_encrypt)
            {
                case ONE_NET_STREAM_ENCRYPT_NONE:
                {
                    rounds = 0;
                    break;
                } // no encryption //

                case ONE_NET_STREAM_ENCRYPT_XTEA8:
                {
                    rounds = ON_XTEA_8_ROUNDS;
                    break;
                } // xtea with 8 rounds //

                default:
                {
                    return ONS_INTERNAL_ERR;
                    break;
                } // default //
            } // switch on encryption type //

            data[payload_len - 1] = on_base_param->stream_encrypt;
        } // else stream //
    #endif // if _ONE_NET_SIMPLE_DEVICE is not defined //

#ifdef _ONE_NET_USE_ENCRYPTION
    if(rounds)
    {
        UInt8 i;

        // -1 since we're not enciphering the byte that has the 2 bits for
        // the encryption type used.
        for(i = 0; i < payload_len - 1; i += ONE_NET_XTEA_BLOCK_SIZE)
        {
            one_net_xtea_encipher(rounds, &(data[i]), KEY);
        } // process 8 bytes at a time //
    } // if  rounds //
#endif

    return ONS_SUCCESS;
} // on_encrypt //


/*!
    \brief Decrypt the data passed in.

    The last 2 bits of the data should contain the method used to decrypt the
    packet.  These 2 bits are the high 2 bits of the last byte, as not all of
    the bits in the last byte are used.

    \param[in] DATA_TYPE Type of data to decrypted.  (see on_data_t in
      one_net.h)
    \param[in/out] data The data to decrypt
    \param[in] key The XTEA key used to decrypt the data

    \return The status of the operation
*/
one_net_status_t on_decrypt(const UInt8 DATA_TYPE, UInt8 * const data,
  const one_net_xtea_key_t * const KEY)
{
    // the length of the payload in bytes and 6-8 bit blocks.
    UInt8 payload_len;

    // # of encryption rounds
    UInt8 rounds = 0;

    if(!data)
    {
        return ONS_BAD_PARAM;
    } // if invalid parameter //

    // get the size that the payload is supposed to be
    if(DATA_TYPE == ON_SINGLE)
    {
        payload_len = ON_RAW_SINGLE_PLD_SIZE;
    } // if single //
    else if(DATA_TYPE == ON_INVITE)
    {
        payload_len = ON_RAW_INVITE_SIZE;
    } // if client invite //
    #ifndef _ONE_NET_SIMPLE_DEVICE
        else
        {
            payload_len = ON_RAW_BLOCK_STREAM_PLD_SIZE;
        } // else block/stream //
    #else
        else
        {
            return ONS_BAD_PARAM;
        } // else the parameter is invalid
    #endif // ifdef _ONE_NET_SIMPLE_DEVICE //

    // get the number of XTEA rounds
    if(DATA_TYPE != ON_STREAM)
    {
        switch(data[payload_len - 1])
        {
            case ONE_NET_SINGLE_BLOCK_ENCRYPT_NONE:
            {
                rounds = 0;
                break;
            } // no encryption //

            case ONE_NET_SINGLE_BLOCK_ENCRYPT_XTEA32:
            {
                rounds = ON_XTEA_32_ROUNDS;
                break;
            } // xtea with 32 rounds //

            default:
            {
                return ONS_INTERNAL_ERR;
                break;
            } // default //
        } // switch on encryption type //
    } // if stream //
    #ifndef _ONE_NET_SIMPLE_DEVICE
        else
        {
            switch(data[payload_len - 1])
            {
                case ONE_NET_STREAM_ENCRYPT_NONE:
                {
                    rounds = 0;
                    break;
                } // no encryption //

                case ONE_NET_STREAM_ENCRYPT_XTEA8:
                {
                    rounds = ON_XTEA_8_ROUNDS;
                    break;
                } // xtea with 8 rounds //

                default:
                {
                    return ONS_INTERNAL_ERR;
                    break;
                } // default //
            } // switch on encryption type //
        } // else single/block //
    #endif // ifdef _ONE_NET_SIMPLE_DEVICE //

#ifdef _ONE_NET_USE_ENCRYPTION    
    if(rounds)
    {
        UInt8 i;

        // -1 since we're not enciphering the byte that has the 2 bits for
        // the encryption type used.
        for(i = 0; i < payload_len - 1; i += ONE_NET_XTEA_BLOCK_SIZE)
        {
            one_net_xtea_decipher(rounds, &(data[i]), KEY);
        } // process 8 bytes at a time //
    } // if  rounds //
#endif

    return ONS_SUCCESS;
} // on_decrypt //


/*!
    \brief Receives data packets only.

    This function is to only receive Single, Repeat Single, Block, Repeat Block,
    Stream, and Data Rate Test packets.  All other packets are discarded.

    \param[in] EXPECTED_SRC_DID The encoded DID of the device that this device
      expects to receive a packet from.  If this is set to the broadcast
      address, this device does not expect a packet from anyone in particular.
    \param[in/out] The current transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized properly.
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_READ_ERR If there was an error while reading the packet.
            ONS_BAD_PKT_TYPE If a packet type that was not expected was
              received.
            ONS_INTERNAL_ERR if control reaches the end of the function.
            ONS_INVALID_DATA If data received is not valid.
            For more return codes, see rx_pkt_addr.
*/
one_net_status_t on_rx_data_pkt(const on_encoded_did_t * const EXPECTED_SRC_DID,
  on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 pid;

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!EXPECTED_SRC_DID || !txn)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr(EXPECTED_SRC_DID, &src_did, mh_pkt))
          != ONS_SUCCESS)
        {
            if(status == ONS_DID_FAILED)
            {
                status = repeat_mh_pkt(txn);
            } // if receiving a Multi-Hop packet //

            return status;
        } // if the packet is not for this device //
    #else
        if((status = rx_pkt_addr(EXPECTED_SRC_DID, &src_did)) != ONS_SUCCESS)
        {
            return status;
        } // if the packet is not for this device //
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    switch(pid)
    {
        #ifdef _ONE_NET_MULTI_HOP
            case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
            case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA: // fall through
        #endif // ifdef _ONE_NET_MULTI_HOP //
        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            status = rx_single_data(pid,
              (const on_encoded_did_t * const)&src_did, txn);
            break;
        } // single data case //

        #ifndef _ONE_NET_SIMPLE_DEVICE
            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_BLOCK_DATA:     // fall through
                case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:
            {
                status = rx_block_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);
                break;
            } // block data case //

            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_STREAM_DATA:    // fall through
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_STREAM_DATA:
            {
                status = rx_stream_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);
                break;
            } // stream data case //
        #endif // if _ONE_NET_SIMPLE_DEVICE is not defined //

        default:
        {
            return ONS_BAD_PKT_TYPE;
        } // default //
    } // switch(pid) //

    return status;
} // on_rx_data_pkt //


/*!
    \brief Builds an admin packet.

    \param[out] pkt Returns the ready to send packet.
    \param[in/out] pkt_size On input, contains the size of pkt.
                            On output, contains the size of the packet.
    \param[in] MSG_TYPE The type of message being sent.  This can be either
      ON_ADMIN_MSG or ON_EXTENDED_ADMIN_MSG
    \param[in] MSG_ID The type of admin packet being sent.
    \param[in] ENCODED_DST The encoded DID of the device receiving the packet.
    \param[in] TXN_NONCE The nonce the receiver expects.
    \param[in] RESP_NONCE The nonce expected from the receiver when the
                 response is received.
    \param[in] RAW_DATA The data that goes along with the MSG_ID.
    \param[in] DATA_LEN The length of RAW_DATA in bytes.
    \param[in] KEY The key to use to encrypt the data.
    \param[in] MAX_HOPS This field is only present if the _ONE_NET_MULTI_HOP
      preprocessor definition has been defined.  This is the maximum number of
      hops a multi-hop packet can take.

    \return ONS_SUCCESS If building the packet was successful
            ONS_BAD_PARAM If any of the parameters are invalid.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_admin_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 MSG_ID,
      const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 RESP_NONCE,
      const UInt8 * const RAW_DATA, const UInt8 DATA_LEN,
      const one_net_xtea_key_t * const KEY, const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_admin_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 MSG_ID,
      const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 RESP_NONCE,
      const UInt8 * const RAW_DATA, const UInt8 DATA_LEN,
      const one_net_xtea_key_t * const KEY)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    UInt8 admin_pkt[ONE_NET_RAW_SINGLE_DATA_LEN];

    #ifdef _ONE_NET_MULTI_HOP
        if(!pkt || !pkt_size
          || *pkt_size < ON_ENCODED_SINGLE_DATA_LEN + ON_ENCODED_HOPS_SIZE
          || (MSG_TYPE != ON_ADMIN_MSG && MSG_TYPE != ON_EXTENDED_ADMIN_MSG)
          || !ENCODED_DST || !RAW_DATA || DATA_LEN > ON_MAX_ADMIN_PLD_LEN
          || !KEY || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(!pkt || !pkt_size
          || *pkt_size < ON_ENCODED_SINGLE_DATA_LEN
          || (MSG_TYPE != ON_ADMIN_MSG && MSG_TYPE != ON_EXTENDED_ADMIN_MSG)
          || !ENCODED_DST || !RAW_DATA || DATA_LEN > ON_MAX_ADMIN_PLD_LEN
          || !KEY)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    admin_pkt[ON_ADMIN_MSG_ID_IDX] = MSG_ID;
    one_net_memmove(&(admin_pkt[ON_ADMIN_DATA_IDX]), RAW_DATA, DATA_LEN);

    #ifdef _ONE_NET_MULTI_HOP
        return on_build_data_pkt(pkt, pkt_size, MSG_TYPE,
          MAX_HOPS ? ONE_NET_ENCODED_MH_SINGLE_DATA
          : ONE_NET_ENCODED_SINGLE_DATA, ENCODED_DST, TXN_NONCE, RESP_NONCE,
          admin_pkt, sizeof(admin_pkt), KEY, MAX_HOPS);
    #else // ifdef _ONE_NET_MULTI_HOP //
        return on_build_data_pkt(pkt, pkt_size, MSG_TYPE,
          ONE_NET_ENCODED_SINGLE_DATA, ENCODED_DST, TXN_NONCE, RESP_NONCE,
          admin_pkt, sizeof(admin_pkt), KEY);
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
} // on_build_admin_pkt //


/*!
    \brief Builds a data packet for single, block, or stream transaction.

    \param[out] pkt Pointer to location to store the packet
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] MSG_TYPE The type of message (see msg_type_t)
    \param[in] PID The encoded packet id (must be single, block, or stream,
                     or one of the repeat packets).
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] TXN_NONCE The nonce the receiver expects.
    \param[in] RESP_NONCE The nonce expected from the receiver when the
                 response is received.
    \param[in] RAW_DATA Any packet specific data that must be sent with the pkt.
    \param[in] DATA_LEN The length of RAW_DATA (in bytes).
    \param[in] KEY The key used to encrypt the data.
    \param[in] MAX_HOPS This field is only present if the _ONE_NET_MULTI_HOP
      preprocessor definition has been defined.  This is the maximum number of
      hops a multi-hop packet can take.

    \return See on_build_pkt for return types.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_data_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 PID,
      const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
      const UInt8 RESP_NONCE, const UInt8 * const RAW_DATA,
      const UInt8 DATA_LEN, const one_net_xtea_key_t * const KEY,
      const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_data_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 MSG_TYPE, const UInt8 PID,
      const on_encoded_did_t * const ENCODED_DST, const UInt8 TXN_NONCE,
      const UInt8 RESP_NONCE, const UInt8 * const RAW_DATA,
      const UInt8 DATA_LEN, const one_net_xtea_key_t * const KEY)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    one_net_status_t status;

    // + 1 to include byte needed for 2 bits for encryption method.
    UInt8 raw_pld[ON_MAX_RAW_PLD_LEN + 1];
    UInt8 data_type;                // The type of data being sent    
    UInt8 pld_word_size;            // Size of the payload in 6 or 8 bit words
    UInt8 raw_pld_len = 0;          // size of the raw payload in bytes

    #ifdef _ONE_NET_MULTI_HOP
        if(!pkt || !pkt_size || !ENCODED_DST || !RAW_DATA || !DATA_LEN || !KEY
          || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP
        if(!pkt || !pkt_size || !ENCODED_DST || !RAW_DATA || !DATA_LEN || !KEY)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    switch(PID)
    {
        #ifdef _ONE_NET_MULTI_HOP
            case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
            case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA: // fall through
        #endif // ifdef _ONE_NET_MULTI_HOP //
        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            #ifdef _ONE_NET_MULTI_HOP
                if(*pkt_size < ON_ENCODED_SINGLE_DATA_LEN + ON_ENCODED_HOPS_SIZE
                  || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if(*pkt_size < ON_ENCODED_SINGLE_DATA_LEN
                  || DATA_LEN > ONE_NET_RAW_SINGLE_DATA_LEN)
            #endif // else _ONE_NET_MULTI_HOP was not defined /
            {
                return ONS_BAD_PARAM;
            } // if parameter is invalid //

            data_type = ON_SINGLE;
            raw_pld_len = ONE_NET_RAW_SINGLE_DATA_LEN;
            pld_word_size = ON_ENCODED_SINGLE_PLD_SIZE;
            break;
        } // single data case //

        #ifndef _ONE_NET_SIMPLE_DEVICE
            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_BLOCK_DATA:     // fall through
                case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:
                case ONE_NET_ENCODED_MH_STREAM_DATA:    // fall through
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_BLOCK_DATA:            // fall through
            case ONE_NET_ENCODED_REPEAT_BLOCK_DATA:     // fall through
            case ONE_NET_ENCODED_STREAM_DATA:
            {
                if(*pkt_size < ONE_NET_MAX_ENCODED_PKT_LEN
                  || DATA_LEN > ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
                {
                    return ONS_BAD_PARAM;
                } // if parameter is invalid //

                #ifdef _ONE_NET_MULTI_HOP
                    if(PID == ONE_NET_ENCODED_STREAM_DATA
                      || PID == ONE_NET_ENCODED_MH_STREAM_DATA)
                #else // ifdef _ONE_NET_MULTI_HOP //
                    if(PID == ONE_NET_ENCODED_STREAM_DATA)
                #endif // else _ONE_NET_MULTI_HOP is not defined //
                {
                    data_type = ON_STREAM;
                } // if a stream packet //
                else
                {
                    data_type = ON_BLOCK;
                } // else a block packet //
                raw_pld_len = ONE_NET_RAW_BLOCK_STREAM_DATA_LEN;
                pld_word_size = ON_ENCODED_BLOCK_STREAM_PLD_SIZE;
                break;
            } // single data case //
        #endif // _ONE_NET_SIMPLE_DEVICE //

        default:
        {
            return ONS_BAD_PARAM;
            break;
        } // default case //
    } // switch(PID) //

    // build the packet
    raw_pld[ON_PLD_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    raw_pld[ON_PLD_RESP_NONCE_HIGH_IDX] |= (RESP_NONCE
      >> ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK;
    raw_pld[ON_PLD_RESP_NONCE_LOW_IDX] = (RESP_NONCE << ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_BUILD_LOW_MASK;
    raw_pld[ON_PLD_MSG_TYPE_IDX] |= MSG_TYPE;
    one_net_memmove(&(raw_pld[ON_PLD_DATA_IDX]), RAW_DATA, DATA_LEN);

    // compute the crc
    raw_pld[0] = (UInt8)one_net_compute_crc(&(raw_pld[ON_PLD_TXN_NONCE_IDX]),
      raw_pld_len + ON_RAW_PLD_HDR_SIZE - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC,
      ON_PLD_CRC_ORDER);

    if((status = on_encrypt(data_type, raw_pld, KEY)) == ONS_SUCCESS)
    {
        #ifdef _ONE_NET_MULTI_HOP
            status = on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_pld,
              pld_word_size, MAX_HOPS);
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_pld,
              pld_word_size);
        #endif // else _ONE_NET_MULTI_HOP is not defined //
    } // if encrypting was not successful //

    return status;
} // on_build_data_pkt //


/*!
    \brief Builds a response packet.

    \param[out] pkt Pointer to location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] PID The encoded packet id.
    \param[in] nack_reason The reson for the NACK.  Only applies to NACKs and
      only present if the _ONE_NET_VERSION_2_X preprocessor definition has been defined.
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] TXN_NONCE The transaction nonce associated with this packet.
    \param[in] EXPECTED_NONCE The nonce the receiver should use the next time it
      sends to this device.
    \param[in] KEY The key used to encrypt the data.  This field is only present
	if the _ONE_NET_VERSION_2_X preprocessor definition has been defined.
    \param[in] MAX_HOPS This field is only present if the _ONE_NET_MULTI_HOP
      preprocessor definition has been defined.  This is the maximum number of
      hops a multi-hop packet can take.

    \return ONS_BAD_PARAM if any of the parameters are invalid.
            See on_build_pkt for more return types.
*/
#ifndef _ONE_NET_VERSION_2_X

#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE, const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    UInt8 data[ON_RESP_NONCE_LEN];

    #ifdef _ONE_NET_MULTI_HOP
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              + ON_ENCODED_HOPS_SIZE || TXN_NONCE > ON_MAX_NONCE
              || EXPECTED_NONCE > ON_MAX_NONCE || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              || TXN_NONCE > ON_MAX_NONCE || EXPECTED_NONCE > ON_MAX_NONCE)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //
	

    // build the data portion
    data[ON_RESP_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    data[ON_RESP_RESP_NONCE_HIGH_IDX] |=
      (EXPECTED_NONCE >> ON_RESP_NONCE_HIGH_SHIFT)
      & ON_RESP_NONCE_BUILD_HIGH_MASK;
    data[ON_RESP_RESP_NONCE_LOW_IDX] =
      (EXPECTED_NONCE << ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_BUILD_LOW_MASK;


    #ifdef _ONE_NET_MULTI_HOP
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
              ON_RESP_NONCE_WORD_SIZE, MAX_HOPS);
    #else // ifdef _ONE_NET_MULTI_HOP //
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
            ON_RESP_NONCE_WORD_SIZE); // the NACK reason field requires an exrta byte 
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
} // on_build_response_pkt //

#else // If ONE-NET Version 1.x



// temporarily using this 1.x function for 2.0

#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE, const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_response_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    UInt8 data[ON_RESP_NONCE_LEN];

    #ifdef _ONE_NET_MULTI_HOP
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              + ON_ENCODED_HOPS_SIZE || TXN_NONCE > ON_MAX_NONCE
              || EXPECTED_NONCE > ON_MAX_NONCE || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              || TXN_NONCE > ON_MAX_NONCE || EXPECTED_NONCE > ON_MAX_NONCE)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //
	

    // build the data portion
    data[ON_RESP_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    data[ON_RESP_RESP_NONCE_HIGH_IDX] |=
      (EXPECTED_NONCE >> ON_RESP_NONCE_HIGH_SHIFT)
      & ON_RESP_NONCE_BUILD_HIGH_MASK;
    data[ON_RESP_RESP_NONCE_LOW_IDX] =
      (EXPECTED_NONCE << ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_BUILD_LOW_MASK;


    #ifdef _ONE_NET_MULTI_HOP
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
              ON_RESP_NONCE_WORD_SIZE, MAX_HOPS);
    #else // ifdef _ONE_NET_MULTI_HOP //
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
            ON_RESP_NONCE_WORD_SIZE); // the NACK reason field requires an exrta byte 
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
} // on_build_response_pkt //



// temporary 2.x function (not used yet)
#ifdef _ONE_NET_MULTI_HOP
/*one_net_status_t on_build_response_pkt_2_X(UInt8 * pkt, UInt8 * const pkt_size,
   const UInt8 PID, const on_nack_rsn_t* const nack_reason, const on_encoded_did_t * const ENCODED_DST,
   const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE,
   const one_net_xtea_key_t * const KEY, const UInt8 MAX_HOPS)*/
   
one_net_status_t on_build_response_pkt_2_X(UInt8 * pkt, UInt8 * const pkt_size,
   const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
   const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE, const UInt8 MAX_HOPS)   
#else
/*one_net_status_t on_build_response_pkt_2_X(UInt8 * pkt, UInt8 * const pkt_size,
   const UInt8 PID, const on_nack_rsn_t* const nack_reason, const on_encoded_did_t * const ENCODED_DST,
   const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE,
   const one_net_xtea_key_t * const KEY)*/
   
 one_net_status_t on_build_response_pkt_2_X(UInt8 * pkt, UInt8 * const pkt_size,
   const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
   const UInt8 TXN_NONCE, const UInt8 EXPECTED_NONCE)   
#endif
{
/*    one_net_status_t status;
    UInt8 raw_pld[ON_RAW_ACK_NACK_PLD_LEN + 1]; // + 1 to include byte needed for 2 bits for encryption method.
	
    #ifdef _ONE_NET_USE_RANDOM_PADDING
        UInt8 randomPadStartBitIndex = 8 + (2 * 6); // 8 bits for CRC, 6 bits each for two nonces
        UInt8 randomPadStopBitIndex = ON_RAW_ACK_NACK_PLD_LEN * 8 - 1;
    #endif

    // build the packet
    raw_pld[ON_PLD_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    raw_pld[ON_PLD_RESP_NONCE_HIGH_IDX] |= (EXPECTED_NONCE
      >> ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_BUILD_HIGH_MASK;
    raw_pld[ON_PLD_RESP_NONCE_LOW_IDX] = (EXPECTED_NONCE << 
      ON_RESP_NONCE_LOW_SHIFT) & ON_RESP_NONCE_BUILD_LOW_MASK;
	
	if(nack_reason)
	{
        raw_pld[ON_PLD_NACK_LOW_IDX] |= ((*nack_reason)
          >> ON_NACK_HIGH_SHIFT) & ON_NACK_BUILD_HIGH_MASK;
        raw_pld[ON_PLD_NACK_HIGH_IDX] = ((*nack_reason) << ON_NACK_LOW_SHIFT)
          & ON_NACK_BUILD_LOW_MASK;
		  
        #ifdef _ONE_NET_USE_RANDOM_PADDING
            randomPadStartBitIndex += 6; // add 6 more bits for the NACK reason
            RandomPad(raw_pld, ON_RAW_ACK_NACK_PLD_LEN, randomPadStartBitIndex,
              randomPadStopBitIndex);
        #endif
	}	  

    // compute the crc
    raw_pld[0] = (UInt8)one_net_compute_crc(&(raw_pld[ON_PLD_TXN_NONCE_IDX]),
      ON_RAW_ACK_NACK_PLD_LEN - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC,
      ON_PLD_CRC_ORDER);

    // TODO - ON_SINGLE possibly should not be hard-coded.  Perhaps this needs to
    // be passed to the function or determined from the PID
    if((status = on_encrypt(ON_SINGLE, raw_pld, KEY)) == ONS_SUCCESS)
    {
        #ifdef _ONE_NET_MULTI_HOP
            status = on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_pld,
              ON_RESP_NONCE_WORD_SIZE, MAX_HOPS);
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, raw_pld,
              ON_RESP_NONCE_WORD_SIZE);
        #endif // else _ONE_NET_MULTI_HOP is not defined //
    } // if encrypting was not successful //

    return status;*/



    UInt8 data[ON_RESP_NONCE_LEN];

    #ifdef _ONE_NET_MULTI_HOP
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              + ON_ENCODED_HOPS_SIZE || TXN_NONCE > ON_MAX_NONCE
              || EXPECTED_NONCE > ON_MAX_NONCE || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
            if(!pkt || !pkt_size || *pkt_size < ON_ACK_NACK_LEN
              || TXN_NONCE > ON_MAX_NONCE || EXPECTED_NONCE > ON_MAX_NONCE)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //
	

    // build the data portion
    data[ON_RESP_TXN_NONCE_IDX] = (TXN_NONCE << ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_BUILD_MASK;
    data[ON_RESP_RESP_NONCE_HIGH_IDX] |=
      (EXPECTED_NONCE >> ON_RESP_NONCE_HIGH_SHIFT)
      & ON_RESP_NONCE_BUILD_HIGH_MASK;
    data[ON_RESP_RESP_NONCE_LOW_IDX] =
      (EXPECTED_NONCE << ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_BUILD_LOW_MASK;


    #ifdef _ONE_NET_MULTI_HOP
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
              ON_RESP_NONCE_WORD_SIZE, MAX_HOPS);
    #else // ifdef _ONE_NET_MULTI_HOP //
            return on_build_pkt(pkt, pkt_size, PID, ENCODED_DST, data,
            ON_RESP_NONCE_WORD_SIZE); // the NACK reason field requires an exrta byte 
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
}

#endif // If ONE-NET Version 2.x


/*!
    \brief Builds a data rate packet.

    If Multi-Hop functionality is enabled, then a ONE_NET_ENCODED_DATA_RATE_TEST
    packet will be sent if MAX_HOPS is 0, otherwise a
    ONE_NET_ENCODED_MH_DATA_RATE_TEST packet will be sent.

    \param[out] pkt Pointer to the location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] data_rate The raw data rate.
    \param[in] MAX_HOPS This field is only present if the _ONE_NET_MULTI_HOP
      preprocessor definition has been defined.  This is the maximum number of
      hops a multi-hop packet can take.

    \return ONS_SUCCESS if the packet was successfully built
            ONS_BAD_PARAM If any of the parameters are invalid.
            See on_build_pkt for more possible return values.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_data_rate_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const on_encoded_did_t * const ENCODED_DST, UInt8 data_rate,
      const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_data_rate_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const on_encoded_did_t * const ENCODED_DST, UInt8 data_rate)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    one_net_status_t status;
    UInt8 i;

    #ifdef _ONE_NET_MULTI_HOP
        UInt8 hops = 0;

        if(!pkt || !pkt_size
          || *pkt_size < ON_DATA_RATE_PKT_LEN + ON_ENCODED_HOPS_SIZE
          || !ENCODED_DST || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(!pkt || !pkt_size || *pkt_size < ON_DATA_RATE_PKT_LEN
          || !ENCODED_DST)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    data_rate <<= ON_DATA_RATE_SHIFT;

    #ifdef _ONE_NET_MULTI_HOP
        if((status = on_build_pkt(pkt, pkt_size,
          MAX_HOPS ? ONE_NET_ENCODED_MH_DATA_RATE_TEST
          : ONE_NET_ENCODED_DATA_RATE_TEST, ENCODED_DST, &data_rate,
          sizeof(data_rate), MAX_HOPS)) != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if((status = on_build_pkt(pkt, pkt_size, ONE_NET_ENCODED_DATA_RATE_TEST,
          ENCODED_DST, &data_rate, sizeof(data_rate))) != ONS_SUCCESS)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return status;
    } // if building the packet was not successful //

    #ifdef _ONE_NET_MULTI_HOP
        // If it's a Multi-Hop data rate test, the hops field has already been
        // stuck in the packet, but the test pattern still needs to be inserted.
        // Save the hops field, put the test pattern into the packet starting
        // at where the hops field was, and put the hops field at the end of
        // the test pattern
        if(MAX_HOPS)
        {
            hops = pkt[*pkt_size - 1];
            (*pkt_size)--;
        } // if a Multi-Hop packet //
    #endif // _ONE_NET_MULTI_HOP //

    // The test pattern still needs to be added
    for(i = 0; i < ON_TEST_PATTERN_SIZE; i++)
    {
        pkt[(*pkt_size)++] = ON_TEST_PATTERN;
    } // loop to add the test pattern //

    #ifdef _ONE_NET_MULTI_HOP
        if(MAX_HOPS)
        {
            // if a Multi-Hop packet, the hops field was removed so the test
            // data could be put into the packet, so put the hops field back in
            // now that has been done.
            pkt[(*pkt_size)++] = hops;
        } // if a _MULTI_HOP packet //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    return ONS_SUCCESS;
} // on_build_data_rate_pkt //


/*!
    \brief Builds a complete packet that is ready to be sent out the rf channel.

    If Multi-Hop functionality is enabled, the HOPS field is added to the end of
    the packet if MAX_HOPS is non-zero, otherwise the HOPS field is omitted.

    \param[out] pkt Pointer to the location to store the packet.
    \param[in/out] pkt_size Input: The size (in bytes) of the location pkt
                              points to.
                            Output: The length (in bytes) of the packet.
    \param[in] PID The encoded packet id.
    \param[in] ENCODED_DST The encoded destination for this packet.
    \param[in] RAW_DATA Any packet specific data that must be sent with the
      packet.
    \param[in] DATA_WORD_SIZE The size of RAW_DATA in 6 or 8 bit words.
    \param[in] MAX_HOPS This field is only present if the _ONE_NET_MULTI_HOP
      preprocessor definition has been defined.  This is the maximum number of
      hops a multi-hop packet can take.

    \return ONS_SUCCESS if the packet was successfully built
            ONS_BAD_PARAM if any of the parameters are invalid
            See on_encode for more return types.
*/
#ifdef _ONE_NET_MULTI_HOP
    one_net_status_t on_build_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 * const RAW_DATA, const UInt8 DATA_WORD_SIZE,
      const UInt8 MAX_HOPS)
#else // ifdef _ONE_NET_MULTI_HOP //
    one_net_status_t on_build_pkt(UInt8 * pkt, UInt8 * const pkt_size,
      const UInt8 PID, const on_encoded_did_t * const ENCODED_DST,
      const UInt8 * const RAW_DATA, const UInt8 DATA_WORD_SIZE)
#endif // else _ONE_NET_MULTI_HOP has not been defined // 
{
    enum
    {
        //! size of the header (Preamble, SOF, addresses, PID)
        HDR_SIZE = 15
    };

    one_net_status_t rv = ONS_SUCCESS;

    #ifdef _ONE_NET_MULTI_HOP
        if(!pkt || !pkt_size || !ENCODED_DST
          || *pkt_size < HDR_SIZE + DATA_WORD_SIZE + ON_ENCODED_HOPS_SIZE
          || MAX_HOPS > ON_MAX_HOPS_LIMIT)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(!pkt || !pkt_size || !ENCODED_DST
          || *pkt_size < HDR_SIZE + DATA_WORD_SIZE)
    #endif // else _ONE_NET_MULTI_HOP has not been defined // 
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    // copy the preamble to the packet
    one_net_memmove(pkt, PREAMBLE, sizeof(PREAMBLE));
    pkt += sizeof(PREAMBLE);

    // copy the start of frame to the packet
    one_net_memmove(pkt, SOF, sizeof(SOF));
    pkt += sizeof(SOF);

    // add the destination address
    one_net_memmove(pkt, *ENCODED_DST, ON_ENCODED_DID_LEN);
    pkt += ON_ENCODED_DID_LEN;

    // add the NID and SRC DID (which happens to be this devices SID)
    one_net_memmove(pkt, on_base_param->sid, sizeof(on_base_param->sid));
    pkt += sizeof(on_base_param->sid);

    // add the pid
    *pkt++ = PID;

    if(RAW_DATA)
    {
        rv = on_encode(pkt, RAW_DATA, DATA_WORD_SIZE);
        #ifdef _ONE_NET_DEBUG
        #if 0
            if (PID == ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN)
            {
                one_net_debug(ONE_NET_DEBUG_NACK_WITH_RSN2_TEST, pkt, DATA_WORD_SIZE);
            }
        #endif
        #endif
    } // if there is data associated with this packet //

    *pkt_size = HDR_SIZE + DATA_WORD_SIZE;

    #ifdef _ONE_NET_MULTI_HOP
        pkt += DATA_WORD_SIZE;
        if(MAX_HOPS)
        {
            rv = on_build_hops(pkt, MAX_HOPS, MAX_HOPS);
            pkt++;
            (*pkt_size)++;
        } // if sending a _MULTI_HOP packet //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    return rv;
} // on_build_pkt //
 

#ifdef _ONE_NET_MULTI_HOP
    /*!
        \brief Builds the encoded hops field for the packet.

        \param[out] hops The hops field to be sent with the pkt.
        \param[in] MAX_HOPS The maximum number of hops the packet can take.
        \param[in] HOPS_LEFT The number of hops remaining that the pkt can take.

        \return ONS_SUCCESS If building the hops field was successful
                ONS_BAD_PARAM If any of the parameters are invalid.
    */
    one_net_status_t on_build_hops(UInt8 * const hops, const UInt8 MAX_HOPS,
      const UInt8 HOPS_LEFT)
    {
        UInt8 raw_hops;

        if(!hops || MAX_HOPS > ON_MAX_HOPS_LIMIT || HOPS_LEFT > MAX_HOPS)
        {
            return ONS_BAD_PARAM;
        } // if any of the parameters are invalid //

        raw_hops = ((MAX_HOPS << ON_MAX_HOPS_SHIFT) & ON_MAX_HOPS_BUILD_MASK)
          | ((HOPS_LEFT << ON_HOPS_LEFT_SHIFT) & ON_HOPS_LEFT_BUILD_MASK);

        on_encode(hops, &raw_hops, ON_ENCODED_HOPS_SIZE);

        return ONS_SUCCESS;
    } // on_build_hops //
#endif // ifdef _ONE_NET_MULTI_HOP //


/*!
    \brief Parses the payload field of a raw data packet.

    This function decodes, decrypts the payload of a data packet.  It will also
    compute and validate the crc, and return the sections of the header.

    \param[out] txn_nonce The transaction nonce.
    \param[out] resp_nonce The nonce to be used in the response packet.
    \param[out] msg_type The type of message contained in the packet.
    \param[out] pld The pld portion of the payload.
    \param[in] DATA_TYPE Type of data being parsed (single, block, or stream)
    \param[in] KEY The xtea key to use to decrypt the packet.

    \return ONS_SUCCESS if parsing the payload was successful
            ONS_CRC_FAIL if the computed crc did not match the received crc.
            ONS_BAD_PARAM If the parameters are invalid.
            See on_decode and decrypt for more possible return values.
*/
one_net_status_t on_parse_pld(UInt8 * const txn_nonce, UInt8 * const resp_nonce,
  UInt8 * const msg_type, UInt8 * const pld, const UInt8 DATA_TYPE,
  const one_net_xtea_key_t * const KEY)
{
    one_net_status_t status;

    UInt8 raw_pld_len;

    if(!txn_nonce || !resp_nonce || !msg_type || !pld || !KEY)
    {
        return ONS_BAD_PARAM;
    } // the parameters are invalid //

    switch(DATA_TYPE)
    {
        case ON_SINGLE:
        {
            // -1 for byte that contains the encryption bits
            raw_pld_len = ON_RAW_SINGLE_PLD_SIZE - 1;
            break;
        } // single case //

        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ON_BLOCK:          // fall through
            case ON_STREAM:
            {
                // -1 for byte that contains the encryption bits
                raw_pld_len = ON_RAW_BLOCK_STREAM_PLD_SIZE - 1;
                break;
            } // block, stream case //
        #endif

        default:
        {
            return ONS_BAD_PARAM;
            break;
        } // default case //
    } // switch(DATA_TYPE)

    if((status = on_decrypt(DATA_TYPE, pld, KEY)) != ONS_SUCCESS)
    {
        return status;
    } // if decrypting the packet is not successful //

    if((UInt8)one_net_compute_crc(&(pld[ON_PLD_CRC_SIZE]),
      raw_pld_len - ON_PLD_CRC_SIZE, ON_PLD_INIT_CRC, ON_PLD_CRC_ORDER)
      != pld[ON_PLD_CRC_IDX])
    {
        return ONS_CRC_FAIL;
    } // if the crc's don't match //

    // get the transaction nonce
    *txn_nonce = (pld[ON_PLD_TXN_NONCE_IDX] >> ON_TXN_NONCE_SHIFT)
      & ON_TXN_NONCE_PARSE_MASK;

    // get the response nonce
    *resp_nonce = (pld[ON_PLD_RESP_NONCE_HIGH_IDX]
      << ON_RESP_NONCE_HIGH_SHIFT) & ON_RESP_NONCE_PARSE_HIGH_MASK;
    *resp_nonce |= (pld[ON_PLD_RESP_NONCE_LOW_IDX] >> ON_RESP_NONCE_LOW_SHIFT)
      & ON_RESP_NONCE_PARSE_LOW_MASK;

    // get the message type
    *msg_type = pld[ON_PLD_MSG_TYPE_IDX] & ON_PLD_MSG_TYPE_MASK;

    return ONS_SUCCESS;
} // on_parse_pld //


#ifdef _ONE_NET_MULTI_HOP
    /*!
        \brief Parses the encoded hops field of the packet.

        This function reads the hops field from the rf interface.

        \param[out] max_hops The maximum number of hops the packet was
          allowed to take.
        \param[out] hops_left The number of hops that the packet was able to
          take before being received by this device.

        \return ONS_SUCCESS If the field was successfully parsed.
                ONS_BAD_PARAM If any of the parameters are invalid.
                ONS_READ_ERR If the data could not be read from the rf
                  interface.
                ONS_INVALID_DATA If the decoded data is invalid
    */
    one_net_status_t on_read_and_parse_hops(UInt8 * const max_hops,
      UInt8 * const hops_left)
    {
        one_net_status_t status;
        UInt8 hops;

        if(!max_hops || !hops_left)
        {
            return ONS_BAD_PARAM;
        } // if any of the parameters are invalid //

        if(one_net_read(&hops, sizeof(hops)) != sizeof(hops))
        {
            return ONS_READ_ERR;
        } // if reading the hops field failed //

        if((status = on_decode(max_hops, &hops, ON_ENCODED_HOPS_SIZE))
          != ONS_SUCCESS)
        {
            return status;
        } // if decoding the hops field failed //

        *hops_left = ((*max_hops >> ON_HOPS_LEFT_SHIFT)
          & ON_HOPS_LEFT_PARSE_MASK);
        *max_hops = ((*max_hops >> ON_MAX_HOPS_SHIFT) & ON_MAX_HOPS_PARSE_MASK);

        if(*max_hops > ON_MAX_HOPS_LIMIT || *hops_left > *max_hops)
        {
            return ONS_INVALID_DATA;
        } // if the data is invalid //

        return ONS_SUCCESS;
    } // on_read_and_parse_hops //


    /*!
        \brief Returns the number of hops the packet took to reach the device.

        This function reads in the hops field portion of the packet from the
        rf interface.

        \return The number of hops the packet took to reach the device.
          ON_INVALID_HOPS will be returned if there was any kind of error.
    */
    UInt8 on_hops_taken(void)
    {
        UInt8 status;
        UInt8 max_hops, hops_left;

        if((status = on_read_and_parse_hops(&max_hops, &hops_left))
          != ONS_SUCCESS)
        {
            return ON_INVALID_HOPS;
        } // if decoding the field was not successful //

        return max_hops - hops_left;
    } // on_hops_taken //
#endif // ifdef _ONE_NET_MULTI_HOP //


#ifndef _ONE_NET_SIMPLE_DEVICE
    /*!
        \brief Updates the time the transaction is supposed to occur at.

        \param[in] txn The transaction to update.

        \return void
    */
    void on_update_next_txn_time(on_txn_t * const txn)
    {
        if(!txn)
        {
            return;
        } // if the parameter is invalid //

        if(txn->priority == ONE_NET_HIGH_PRIORITY)
        {
            ont_set_timer(txn->next_txn_timer,
              on_base_param->fragment_delay_high);
        } // if a high priority transaction //
        else
        {
            ont_set_timer(txn->next_txn_timer,
              on_base_param->fragment_delay_low);
        } // else a low priority transaction //
    } // on_update_next_txn_time //
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //


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
    BOOL rv = FALSE;

#ifdef _ONE_NET_DEBUG_STACK
    if (one_net_debug_stack_flags & 0x01)
    {
        uart_write("\nIn one_net, stack is ", 22);
        uart_write_int8_hex( (((UInt16)(&rv))>>8) & 0xff );
        uart_write_int8_hex( ((UInt16)(&rv)) & 0xff );
        uart_write("\n", 1);
        one_net_debug_stack_flags &= ~0x01;
    }
#endif

    // Only need to check one of the handlers since it's all or nothing //
    if(!txn || !(*txn) || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE
      || !pkt_hdlr.single_data_hdlr)
    {
        return FALSE;
    } // if parameter is invalid //

    switch(on_state)
    {
        case ON_SEND_PKT:
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                on_state = ON_SEND_PKT_WRITE_WAIT;
            } // if channel is clear //
            break;
        } // send packet on_state //

        case ON_SEND_PKT_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // if write is complete //
            break;
        } // send packet write wait on_state //

        case ON_SEND_SINGLE_DATA_PKT:                   // fall through
        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ON_SEND_BLOCK_DATA_PKT:                // fall through
            case ON_SEND_STREAM_DATA_PKT:
        #endif // ifndef _ONE_NET_SIMPLE_DEVICE //
        {
            if(ont_inactive_or_expired(ONT_GENERAL_TIMER)
              && check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                // general timer is now the response timer
                #ifdef _ONE_NET_MULTI_HOP
                    ont_set_timer(ONT_GENERAL_TIMER, ((*txn)->max_hops + 1)
                      * ONE_NET_RESPONSE_TIME_OUT);
                #else // ifdef _ONE_NET_MULTI_HOP //
                    ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_RESPONSE_TIME_OUT);
                #endif // else _ONE_NET_MULTI_HOP is not defined //
                on_state++;
            } // if the channel is clear //
            break;
        } // send single data on_state //

        case ON_SEND_SINGLE_DATA_WRITE_WAIT:            // fall through
        case ON_SEND_SINGLE_DATA_RESP_WRITE_WAIT:       // fall through
        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ON_SEND_BLOCK_DATA_WRITE_WAIT:         // fall through
            case ON_SEND_STREAM_DATA_WRITE_WAIT:        // fall through
        #endif // ifndef _ONE_NET_SIMPLE_DEVICE //
        case ON_SEND_DATA_RATE_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                #ifndef _ONE_NET_SIMPLE_DEVICE
                    if(cur_stream)
                    {
                        *txn = cur_stream;
                        cur_stream = 0;
                        on_state = ON_WAIT_FOR_STREAM_DATA_RESP;
                    } // if cur_stream //
                    else
                #endif // ifndef _ONE_NET_SIMPLE_DEVICE //
                {
                    on_state++;
                } // else not waiting for a stream response //
            } // if write is complete //
            break;
        } // send single data write wait case //

        case ON_WAIT_FOR_SINGLE_DATA_RESP:
        {
            // if the packet was not received, check the response timeout.
            // if timer expired, check retry, and either send the data again,
            // or fail the transaction.
            if(rx_single_resp_pkt(txn) == ONS_SINGLE_FAIL)
            {
                rv = TRUE;
                break;
            } // if the transaction failed //

            if(on_state == ON_LISTEN_FOR_DATA)
            {
                rv = TRUE;
            } // if the on_state changed to ON_LISTEN_FOR_DATA //
            else if(on_state == ON_WAIT_FOR_SINGLE_DATA_RESP
              && ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                // on_state has not changed, and the packet was not received
                // in time
                (*txn)->retry++;

                if((*txn)->retry >= ON_MAX_RETRY)
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_SINGLE_FAIL, 0);
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
                          ONS_SINGLE_FAIL);
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                } // if the transaction has been tried too many times //
                else
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        if((*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                          == ONE_NET_ENCODED_SINGLE_DATA)
                        {
                            (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                              = ONE_NET_ENCODED_REPEAT_SINGLE_DATA;
                        } // if sending a single data pkt //
                        else if((*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                          == ONE_NET_ENCODED_MH_SINGLE_DATA)
                        {
                            (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                              = ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA;
                        } // else if sending a mh single data pkt //
                        // else must already be a repeat packet
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        // make sure a repeat packet is sent
                        (*txn)->pkt[ONE_NET_ENCODED_PID_IDX]
                          = ONE_NET_ENCODED_REPEAT_SINGLE_DATA;
                    #endif // else _ONE_NET_MULTI_HOP is not defined //

                    if((*txn)->priority == ONE_NET_HIGH_PRIORITY)
                    {
                        ont_set_timer(ONT_GENERAL_TIMER,
                          one_net_prand(one_net_tick(),
                          ONE_NET_RETRANSMIT_HIGH_PRIORITY_TIME));
                    } // if it's a high priority transaction //
                    else
                    {
                        ont_set_timer(ONT_GENERAL_TIMER,
                          one_net_prand(one_net_tick(),
                          ONE_NET_RETRANSMIT_LOW_PRIORITY_TIME));
                    } // else it's a low priority transaction //
                    on_state = ON_SEND_SINGLE_DATA_PKT;
                }
            } // else if on_state hasn't changed & timed out waiting for resp //
            break;
        } // wait for single data response case //

        case ON_SEND_SINGLE_DATA_RESP:                  // fall through
        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ON_SEND_BLOCK_DATA_RESP:               // fall through
            case ON_SEND_STREAM_DATA_RESP:
        #endif // ifndef _ONE_NET_SIMPLE_DEVICE //
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);

                #ifndef _ONE_NET_SIMPLE_DEVICE
                    // don't update the timer if waiting for a stream data resp
                    if(!cur_stream)
                #endif // ifdef _ONE_NET_SIMPLE_DEVICE //
                {
                    // general timer is now the response timer

                    // TODO: RWM: maybe we should only set the general timer to 
                    // ONE_NET_TRN_END_TIME_OUT for the ON_SEND_SINGLE_DATA_RESP case
                    // and use ONE_NET_RESPONSE_TIME_OUT for the other cases as it was
                    // before 3/11/09 when we changed to ONE_NET_TRN_END_TIME_OUT?
                    
                    #ifdef _ONE_NET_MULTI_HOP
                        ont_set_timer(ONT_GENERAL_TIMER, ((*txn)->max_hops + 1)
                          * ONE_NET_TRN_END_TIME_OUT);
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        ont_set_timer(ONT_GENERAL_TIMER,
                          ONE_NET_TRN_END_TIME_OUT);
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                } // else not waiting for a stream response //

                on_state++;
            } // if the channel is clear //
            break;
        } // send single/block data response case //

        case ON_WAIT_FOR_SINGLE_DATA_END:
        {
            rx_single_txn_ack(txn);

            // if the on_state has not changed, then the packet was not received
            // in time
            if(on_state == ON_WAIT_FOR_SINGLE_DATA_END
              && ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // if on_state has not changed & timed out waiting for response //
            break;
        } // wait for the end of the single data transaction case //

        #ifndef _ONE_NET_SIMPLE_DEVICE
            case ON_WAIT_FOR_BLOCK_DATA_RESP:
            {
                if(rx_block_resp_pkt(txn) == ONS_BLOCK_FAIL)
                {
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                    break;
                } // if the transaction failed //

                if(on_state == ON_WAIT_FOR_BLOCK_DATA_RESP
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    // the packet was not received in time
                    (*txn)->retry++;

                    if((*txn)->retry >= ON_MAX_RETRY)
                    {
                        #ifdef _ONE_NET_MULTI_HOP
                            pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_BLOCK_FAIL, 0);
                        #else // ifdef _ONE_NET_MULTI_HOP //
                            pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_BLOCK_FAIL);
                        #endif // else _ONE_NET_MULTI_HOP is not defined //
                        rv = TRUE;
                    } // if the transaction has been tried too many times //
                    else
                    {
                        #ifdef _ONE_NET_MULTI_HOP
                            pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_TIME_OUT, 0);
                        #else // ifdef _ONE_NET_MULTI_HOP //
                            pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_TIME_OUT);
                        #endif // else _ONE_NET_MULTI_HOP is not defined //
                    } // else try some more //

                    on_state = ON_LISTEN_FOR_DATA;
                } // if timed out waiting for response //
                else if(block_complete)
                {
                    block_complete = FALSE;
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                } // if the transaction is compolete //
                break;
            } // wait for block data response case //

            case ON_SEND_BLOCK_DATA_RESP_WRITE_WAIT:
            {
                if(one_net_write_done())
                {
                    if(block_complete)
                    {
                        block_complete = FALSE;
                        #ifdef _ONE_NET_MULTI_HOP
                            ont_set_timer(ONT_GENERAL_TIMER,
                              ((*txn)->max_hops + 1)
                              * ONE_NET_RESPONSE_TIME_OUT);
                        #else // ifdef _ONE_NET_MULTI_HOP //
                            ont_set_timer(ONT_GENERAL_TIMER,
                              ONE_NET_RESPONSE_TIME_OUT);
                        #endif // else _ONE_NET_MULTI_HOP is not defined //
                        on_state = ON_WAIT_FOR_BLOCK_DATA_END;
                    } // if the block transaction is complete //
                    else
                    {
                        rv = TRUE;  // done sending response
                        on_state = ON_LISTEN_FOR_DATA;
                    } // else the block transaction is not complete //
                } // if write is complete //
                break;
            } // SEND_BLOCK_DATA_RESP_WRITE_WAIT case //

            case ON_WAIT_FOR_BLOCK_DATA_END:
            {
                if(rx_block_txn_ack(txn) == ONS_SUCCESS)
                {
                    rv = TRUE;
                } // if the txn ack has been received //

                // if the state has not changed, then the packet was not
                // received in time
                if(on_state == ON_WAIT_FOR_BLOCK_DATA_END
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    on_state = ON_LISTEN_FOR_DATA;
                } // if timed out waiting for response //
                break;
            } // WAIT_FOR_BLOCK_DATA_END case //

            case ON_WAIT_FOR_STREAM_DATA_RESP:
            {
                if(rx_stream_resp_pkt(txn) == ONS_STREAM_FAIL)
                {
                    cur_stream = 0;
                    rv = TRUE;
                    on_state = ON_LISTEN_FOR_DATA;
                    break;
                } // if the transaction failed //

                if(on_state == ON_WAIT_FOR_STREAM_DATA_RESP
                  && ont_inactive_or_expired(ONT_GENERAL_TIMER))
                {
                    // the packet was not received in time
                    (*txn)->retry++;

                    if((*txn)->retry >= ON_MAX_RETRY)
                    {
                        cur_stream = 0;
                        #ifdef _ONE_NET_MULTI_HOP
                            pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_STREAM_FAIL, 0);
                        #else // ifdef _ONE_NET_MULTI_HOP //
                            pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_STREAM_FAIL);
                        #endif // else _ONE_NET_MULTI_HOP not defined //
                        rv = TRUE;
                    } // if the transaction has been tried too many times //
                    else
                    {
                        #ifdef _ONE_NET_MULTI_HOP
                            pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_TIME_OUT, 0);
                        #else // ifdef _ONE_NET_MULTI_HOP //
                            pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                              ONS_TIME_OUT);
                        #endif // else _ONE_NET_MULTI_HOP not defined //
                    } // else try some more //

                    on_state = ON_LISTEN_FOR_DATA;
                } // else if timed out waiting for response //
                break;
            } // wait for block data response case //

            case ON_SEND_STREAM_DATA_RESP_WRITE_WAIT:
            {
                if(one_net_write_done())
                {
                    on_state = ON_LISTEN_FOR_DATA;
                } // if write is complete //
                break;
            } // SEND_STREAM_DATA_RESP_WRITE_WAIT case //
        #endif // ifndef _ONE_NET_SIMPLE_DEVICE //

        case ON_INIT_SEND_DATA_RATE:
        {
            UInt8 data_rate;

            if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
              ON_DATA_RATE_WORD_SIZE) != ONS_SUCCESS
              || data_rate >= ONE_NET_DATA_RATE_LIMIT)
            {
                // drop the transaction since the data rate is invalid
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
                break;
            } // if the data rate is invalid //

            one_net_set_data_rate(data_rate);
            (*txn)->retry = 0;
            data_rate_result = 0;
            on_state = ON_SEND_DATA_RATE;
            break;
        } // init sending data rate test case //

        case ON_SEND_DATA_RATE:
        {
            if((*txn)->retry < ON_MAX_DATA_RATE_TRIES)
            {
                if(check_for_clr_channel())
                {
                    one_net_write((*txn)->pkt, (*txn)->data_len);

                    // general timer is now the response timer
                    #ifdef _ONE_NET_MULTI_HOP
                        ont_set_timer(ONT_GENERAL_TIMER, ((*txn)->max_hops + 1)
                          * ONE_NET_RESPONSE_TIME_OUT);
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        ont_set_timer(ONT_GENERAL_TIMER,
                          ONE_NET_RESPONSE_TIME_OUT);
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                    on_state = ON_SEND_DATA_RATE_WRITE_WAIT;
                } // if the channel is clear //
            } // if not finished //
            else
            {
                UInt8 data_rate;

                one_net_set_data_rate(on_base_param->data_rate);
                (*txn)->priority = ONE_NET_NO_PRIORITY;

                if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
                  ON_DATA_RATE_WORD_SIZE) == ONS_SUCCESS)
                {
                    #ifdef _ONE_NET_MULTI_HOP
                        pkt_hdlr.data_rate_hdlr(data_rate,
                          (const on_encoded_did_t * const)
                          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                          data_rate_result);
                    #else // ifdef _ONE_NET_MULTI_HOP //
                        pkt_hdlr.data_rate_hdlr(data_rate,
                          (const on_encoded_did_t * const)
                          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
                          data_rate_result);
                    #endif // else _ONE_NET_MULTI_HOP is not defined //
                } // if decoding was successful //

                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
            } // else the data rate test is complete //
            break;
        } // send data rate case //

        case ON_RX_DATA_RATE_RESP:
        {
            #ifdef _ONE_NET_MULTI_HOP
                UInt8 hops;

                if(rx_data_rate(*txn, FALSE, &hops) == ONS_SUCCESS)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if(rx_data_rate(*txn, FALSE) == ONS_SUCCESS)
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            {
                data_rate_result++;
                (*txn)->retry++;
                on_state = ON_SEND_DATA_RATE;
            } // if receiving the response was successful //
            else if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                (*txn)->retry++;
                on_state = ON_SEND_DATA_RATE;
            } // if the device timed out waiting for the response //
            break;
        } // receive data rate response case //

        case ON_INIT_RX_DATA_RATE:
        {
            UInt8 data_rate;

            if(on_decode(&data_rate, &((*txn)->pkt[ON_DATA_RATE_IDX]),
              ON_DATA_RATE_WORD_SIZE) != ONS_SUCCESS
              || data_rate >= ONE_NET_DATA_RATE_LIMIT)
            {
                // drop the transaction since the data rate is invalid
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;
                break;
            } // if the data rate is invalid //

            one_net_set_data_rate(data_rate);
            (*txn)->retry = 0;
            ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_STAY_AWAKE_TIME);
            on_state = ON_RX_DATA_RATE;
            break;
        } // init receiving data rate test case //

        case ON_RX_DATA_RATE:
        {
            #ifdef _ONE_NET_MULTI_HOP
                UInt8 on_hops_taken;
            #endif // ifdef _ONE_NET_MULTI_HOP //

            if((*txn)->retry >= ON_MAX_RETRY ||
              ont_inactive_or_expired(ONT_GENERAL_TIMER))
            {
                one_net_set_data_rate(on_base_param->data_rate);
                rv = TRUE;
                on_state = ON_LISTEN_FOR_DATA;

                // dje: 23 September, 2008
                //
                // The following two statements were added to make 
                // sure there are no further transmissions since
                // this is the receiving node of the data rate test
                //
                (*txn)->priority = ONE_NET_NO_PRIORITY;
                (*txn)->pkt[ONE_NET_ENCODED_PID_IDX] = 
                    ONE_NET_ENCODED_SINGLE_DATA;
            } // if we had enough successes or timed out //
            #ifdef _ONE_NET_MULTI_HOP
                else if(rx_data_rate(*txn, TRUE, &on_hops_taken)
                  == ONS_SUCCESS)
            #else // ifdef _ONE_NET_MULTI_HOP //
                else if(rx_data_rate(*txn, TRUE) == ONS_SUCCESS)
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            {
                (*txn)->retry++;
                // try to send response write away
                ont_set_timer(ONT_GENERAL_TIMER, 0);
                on_state = ON_SEND_DATA_RATE_RESP;
            } // if receiving the response was successful //
            break;
        } // receive data rate case //

        case ON_SEND_DATA_RATE_RESP:
        {
            if(check_for_clr_channel())
            {
                one_net_write((*txn)->pkt, (*txn)->data_len);
                on_state = ON_SEND_DATA_RATE_RESP_WRITE_WAIT;
            } // if the channel is clear //
            break;
        } // send data rate response case //

        case ON_SEND_DATA_RATE_RESP_WRITE_WAIT:
        {
            if(one_net_write_done())
            {
                // update timer since a pkt was successfully received (and used
                // the timer when the response was sent)
                ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_STAY_AWAKE_TIME);
                on_state = ON_RX_DATA_RATE;
            } // if write is complete //
            break;
        } // send data rate response write wait case //

        case ON_INIT_STATE:
        {
            // if the code gets here, it means the device was not initialized
            // properly.  Stay in this state until the device is initialized.
            break;
        } // initial state //

        default:
        {
            on_state = ON_LISTEN_FOR_DATA;
            break;
        } // default //
    } // switch(on_state) //

    if(on_state == ON_LISTEN_FOR_DATA)
    {
        one_net_set_data_rate(on_base_param->data_rate);
        //rv = TRUE;
    } // if going to the ON_LISTEN_FOR_DATA state //

    return rv;
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
    if(ont_inactive_or_expired(ONT_GENERAL_TIMER))
    {
        ont_set_timer(ONT_GENERAL_TIMER, ONE_NET_CLR_CHANNEL_TIME);
        return one_net_channel_is_clear();
    } // if it is time to check the channel //

    return FALSE;
} // check_for_clr_channel //


/*!
    \brief Receives a packet.

    This function only receives the address portion of the packet.  It verifies
    that the dst is correct, and if this device is expecting a packet from a
    specific sender, it verifies that the sender is correct.  If the address
    checks are all correct, this function calls the function pointed to by
    rx_specific_pkts to finish receiving the packets the device is allowed to
    receive based on the state it is in.

    \param[in] EXPECTED_SRC_DID Encoded address that the device expects to
      receive a packet from.  Set to the encoded form of the broadcast address
      if the device is willing to receive a packet from anybody.
    \param[in] src_did The device ID of the sender.
    \param[out] pkt Only valid if this device is a Multi-Hop repeater.  If this
      field is not NULL and if this device is not the intended destination, the
      received address fields (DST DID, NID, SRC DID) will be returned if the
      sending device is on the same network as this device (NIDs equal).  pkt
      must be at least the size of the longest packet, and represents the
      Multi-Hop packet that will be sent on.

    \return ONS_SUCCESS If the addresses were read correctly, and were what was
              expected.
            ONS_BAD_PARAM If the parameters are invalid.
            ONS_READ_ERR If there was a problem reading in the packet.
            ONS_NID_FAIL If the received NID does not match the NID for this
              device.
            ONS_INCORRECT_ADDR If this device is expecting a packet from
              someone, and the received packet is from someone other than the
              expected device.
            For more return values, see one_net_look_for_pkt, and
              on_validate_dst_DID
*/
#ifdef _ONE_NET_MH_CLIENT_REPEATER
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const src_did, UInt8 * pkt)
#else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
    static one_net_status_t rx_pkt_addr(
      const on_encoded_did_t * const EXPECTED_SRC_DID,
      on_encoded_did_t * const src_did)
#endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
{
    on_encoded_nid_t rx_nid;
    on_encoded_did_t rx_dst;
    one_net_status_t status;

    if(!EXPECTED_SRC_DID || !src_did)
    {
        return ONS_BAD_PARAM;
    } // if parameters are invalid //

    if((status = one_net_look_for_pkt(ONE_NET_WAIT_FOR_SOF_TIME))
      != ONS_SUCCESS)
    {
        return status;
    } // if SOF was not received //

    // get the destination DID
    if(one_net_read(rx_dst, sizeof(rx_dst)) != sizeof(rx_dst))
    {
        return ONS_READ_ERR;
    } // if reading the destination DID failed //

    if((status = on_validate_dst_DID((const on_encoded_did_t * const)&rx_dst))
      != ONS_SUCCESS && status != ONS_BROADCAST_ADDR)
    {
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if(!pkt)
            {
                return status;
            } // if not repeating Multi-Hop packets.

            one_net_memmove(&(pkt[ONE_NET_ENCODED_DST_DID_IDX]), rx_dst,
              sizeof(rx_dst));
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            return status;
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    } // if the packet is not for this device //

    // get the NID
    if(one_net_read(rx_nid, sizeof(rx_nid)) != sizeof(rx_nid))
    {
        return ONS_READ_ERR;
    } // if reading the NID failed //

    if(!on_is_my_NID((const on_encoded_nid_t * const)&rx_nid))
    {
        return ONS_NID_FAILED;
    } // if NID does not match the NID for this device //

    // read the source address
    if(one_net_read(*src_did, sizeof(on_encoded_did_t))
      != sizeof(on_encoded_did_t))
    {
        return ONS_READ_ERR;
    } // if reading the source DID failed //

    // check to see if the device expects something from a specific source
    if(!on_encoded_did_equal(EXPECTED_SRC_DID, &ON_ENCODED_BROADCAST_DID))
    {
        if(!on_encoded_did_equal(EXPECTED_SRC_DID,
          (const on_encoded_did_t * const)src_did))
        {
            // don't bother with the Multi-Hop since the device is looking for
            // a specefic packet
            return ONS_INCORRECT_ADDR;
        } // if the source is not who the device is expecting //
    } // if this device is expecting a packet //

    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if(status == ONS_DID_FAILED)
        {
            one_net_memmove(&(pkt[ON_ENCODED_NID_IDX]), rx_nid, sizeof(rx_nid));
            one_net_memmove(&(pkt[ON_ENCODED_SRC_DID_IDX]), *src_did,
              sizeof(*src_did));
        } // if the this device is not the destination for the pkt //
    #endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

    return status;
} // rx_pkt_addr //


/*!
    \brief Receive the possible responses to a single data packet.

    This function is called from the ON_WAIT_FOR_SINGLE_DATA_RESP state.
    It only receives Single Data ACK, Single Data NACK, or Single
    Data ACK Stay Awake Packets.  These are expected in response to a Single
    Data Packet this device sent.<p>

    This function may change on_state:<br>
    If the nonce check fails and the maximum retries has been exeeded
    on_state is set to ON_LISTEN_FOR_DATA.<br>
    For ONE_NET_ENCODED_SINGLE_DATA_NACK and ONE_NET_ENCODED_MH_SINGLE_DATA_NACK 
    packet types, on_state is set to ON_SEND_SINGLE_DATA_PKT.

    \param[in] txn The transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized<br>
            ONS_BAD_PARAM If the parameter is not valid<br>
            ONS_BAD_PKT_TYPE If a packet type was received that this device is
              not looking for.<br>
            ONS_SINGLE_FAIL If the transaction has been tried too many times.<br>
            ONS_INCORRECT_NONCE If the received transaction nonce is not what is
              expected.<br>
            ONS_INVALID_DATA If received data was not correct<br>
            For more response values, see rx_pkt_addr, on_decode,
              single_resp_hdlr.
*/
static one_net_status_t rx_single_resp_pkt(on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 pid;
    UInt8 txn_nonce, next_nonce;

	#ifdef _ONE_NET_VERSION_2_X
		on_nack_rsn_t nack_reason;
        const one_net_xtea_key_t* key = 0;
	#endif
	
    #ifdef _ONE_NET_MULTI_HOP
        UInt8 hops = 0;
        BOOL mh = FALSE;
    #endif // ifdef _ONE_NET_MULTI_HOP //

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!txn || !*txn || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    // pkt in txn contains the single data packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from.
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
          != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    #ifdef _ONE_NET_MULTI_HOP
        if(pid != ONE_NET_ENCODED_SINGLE_DATA_ACK
          && pid != ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
          && pid != ONE_NET_ENCODED_SINGLE_DATA_NACK
          && pid != ONE_NET_ENCODED_MH_SINGLE_DATA_ACK
          && pid != ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE
          && pid != ONE_NET_ENCODED_MH_SINGLE_DATA_NACK)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(pid != ONE_NET_ENCODED_SINGLE_DATA_ACK
          && pid != ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
          && pid != ONE_NET_ENCODED_SINGLE_DATA_NACK)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
    #ifdef _ONE_NET_DEBUG
        one_net_debug(ONE_NET_DEBUG_ONS_BAD_PKT_TYPE, &pid, 1);
    #endif
        return ONS_BAD_PKT_TYPE;
    } // if the packet is not what the device is expecting //

    // read the nonces
	#ifndef _ONE_NET_VERSION_2_X	
        if((status = rx_nonces(&txn_nonce, &next_nonce)) != ONS_SUCCESS)
	#else
	    // temporarily making ONE_NET_ENCODED_SINGLE_DATA_ACK calls
		// function rx_nonces_2_X for nonces
        if(pid == ONE_NET_ENCODED_SINGLE_DATA_ACK ||
		   pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE)
        {
			status = rx_nonces_2_X(&txn_nonce, &next_nonce);
		}
		else
		{
			status = rx_nonces(&txn_nonce, &next_nonce);
		}
        if(status != ONS_SUCCESS)
	#endif
    {
        return status;
    } // if reading the nonces failed //

    #ifdef _ONE_NET_MULTI_HOP
        if(pid == ONE_NET_ENCODED_MH_SINGLE_DATA_ACK
          || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE
          || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_NACK)
        {
            mh = TRUE;

            if((hops = on_hops_taken()) > ON_MAX_HOPS_LIMIT)
            {
                return ONS_INVALID_DATA;
            } // if parsing the hops field was not successful //
        } // if a multi-hop packet was received //

        if(txn_nonce != (*txn)->expected_nonce
          || pid == ONE_NET_ENCODED_SINGLE_DATA_NACK
          || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_NACK)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(txn_nonce != (*txn)->expected_nonce
          || pid == ONE_NET_ENCODED_SINGLE_DATA_NACK)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        (*txn)->retry++;

        if((*txn)->retry >= ON_MAX_RETRY)
        {
            #ifdef _ONE_NET_MULTI_HOP
                pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1, ONS_SINGLE_FAIL,
                  hops);
            #else // ifdef _ONE_NET_MULTI_HOP //
                pkt_hdlr.single_txn_hdlr(txn, ON_MAX_NONCE + 1,
                  ONS_SINGLE_FAIL);
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            on_state = ON_LISTEN_FOR_DATA;
            return ONS_SINGLE_FAIL;
        } // if the transaction has been tried too many times //

        if(txn_nonce != (*txn)->expected_nonce)
        {
            return ONS_INCORRECT_NONCE;
        } // if the nonce did not match //
    } // if the nonce did not match or it was a NACK //

    // If _ONE_NET_MULTI_HOP is defined, the hops field should be read here, but
    // that field is being ignored in processing these packet types since the
    // benefits of processing the data are not enough to outweigh the overhead
    // of processing the data.

    #ifdef _ONE_NET_MULTI_HOP
        if(pid == ONE_NET_ENCODED_SINGLE_DATA_NACK
          || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_NACK)
        {
            status = pkt_hdlr.single_txn_hdlr(txn, next_nonce, ONS_RX_NACK,
              hops);
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(pid == ONE_NET_ENCODED_SINGLE_DATA_NACK)
        {
            status = pkt_hdlr.single_txn_hdlr(txn, next_nonce, ONS_RX_NACK);
    #endif // else _ONE_NET_MULTI_HOP is not defined //

        // try to send right away
        ont_set_timer(ONT_GENERAL_TIMER, 0);
        on_state = ON_SEND_SINGLE_DATA_PKT;
    } // if a single data nack was received //
    else
    {
        #ifdef _ONE_NET_MULTI_HOP
            status = pkt_hdlr.single_txn_hdlr(txn, next_nonce,
              (pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
              || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE)
              ? ONS_RX_STAY_AWAKE : ONS_SUCCESS, hops);

            if((pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
              || pid == ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE)
              || status == ONS_SUCCESS)
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = pkt_hdlr.single_txn_hdlr(txn, next_nonce,
              pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
              ? ONS_RX_STAY_AWAKE : ONS_SUCCESS);

            if(pid == ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE
              || status == ONS_SUCCESS)
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        {
            // Received a stay awake, or there is not another transaction being
            // sent to that device, so end the transaction.
            (*txn)->data_len = (*txn)->pkt_size;
            
            // try to send right away
            ont_set_timer(ONT_GENERAL_TIMER, 0);

            #ifdef _ONE_NET_MULTI_HOP
                if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
                  mh ? ONE_NET_ENCODED_MH_SINGLE_TXN_ACK
                  : ONE_NET_ENCODED_SINGLE_TXN_ACK,
                  (const on_encoded_did_t * const)&src_did, 0, 0,
                  mh ? hops : 0)) != ONS_SUCCESS)
            #else // ifdef _ONE_NET_MULTI_HOP //
                if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
                  ONE_NET_ENCODED_SINGLE_TXN_ACK,
                  (const on_encoded_did_t * const)&src_did, 0, 0))
                  != ONS_SUCCESS)
            #endif // else _ONE_NET_MULTI_HOP is not defined //
            {
                on_state = ON_LISTEN_FOR_DATA;
                return status;
            } // if building the transaction ack failed //
            on_state = ON_SEND_PKT;
        }
        else if(status == ONS_TXN_QUEUED)
        {
            // There is another single transaction to send to the receiving
            // device. Try to send right away since not sending a transaction
            // ACK (the default state is ON_LISTEN_FOR_DATA, so that is where
            // the next packet should be dequeued from.
            ont_set_timer(ONT_GENERAL_TIMER, 0);
            on_state = ON_LISTEN_FOR_DATA;
        } // else if another transaction is queued //
        // else the transaction was an error, so time out and retry //
    } // if a single data ack or single data ack stay awake was received //

    return status;
} // rx_single_resp_pkt //


/*!
    \brief Receives the single data transaction ack.

    This is called by the receiving device of a single data transaction.  It
    will also receive a single data (for a new transaction) or repeat single
    data packet.

    \param[in] txn The transaction being carried out.

    \return ONS_NOT_INIT If the device was not initialized properly.
            ONS_BAD_PARAM If the parameter is invalid.
            ONS_READ_ERR If there was an error while reading the packet.
            ONS_BAD_PKT_TYPE If a packet type that was not expected was
              received.
            For more return codes, see rx_pkt_addr.
*/
static one_net_status_t rx_single_txn_ack(on_txn_t ** txn)
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 pid;

    // only need to check 1 handler since it is all or nothing
    if(!pkt_hdlr.single_data_hdlr)
    {
        return ONS_NOT_INIT;
    } // if this device was not initialized //

    if(!txn || !(*txn) || !(*txn)->pkt
      || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    //
    // pkt in txn contains the ack packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from. rx_pkt_addr will filter on this 
    // address.
    //
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
          != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    switch(pid)
    {
        #ifdef _ONE_NET_MULTI_HOP
            case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
        #endif // ifdef _ONE_NET_MULTI_HOP //
        case ONE_NET_ENCODED_SINGLE_TXN_ACK:
        {
            (*txn)->priority = ONE_NET_NO_PRIORITY;
            on_state = ON_LISTEN_FOR_DATA;
            break;
        } // single transaction ack case //

        #ifdef _ONE_NET_MULTI_HOP
            case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
            case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA: // fall through
        #endif // ifdef _ONE_NET_MULTI_HOP //
        case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
        case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
        {
            status = rx_single_data(pid,
              (const on_encoded_did_t * const)&src_did, txn);
            break;
        } // single data case //

        default:
        {
            return ONS_BAD_PKT_TYPE;
        } // default //
    } // switch(pid) //

    // received the end of the transaction (or start of a new one), so clear
    // the general timer as it is not needed
    ont_stop_timer(ONT_GENERAL_TIMER);
    return status;
} // rx_single_txn_ack //


#ifndef _ONE_NET_SIMPLE_DEVICE
    /*!
        \brief Receive the possible responses to a block data packet.

        This function only receives Block Data ACK, or Block Data NACK Packets.
        These are expected in response to a Block Data Packet this device sent.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized
                ONS_BAD_PARAM If the parameter is not valid
                ONS_BAD_PKT_TYPE If a packet type was received that this device
                  is not looking for.
                ONS_BLOCK_FAIL If the transaction has been tried too many times.
                ONS_INCORRECT_NONCE If the received transaction nonce is not
                  what is expected.
                For more response values, see rx_pkt_addr, on_decode.
    */
    static one_net_status_t rx_block_resp_pkt(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;
        UInt8 txn_nonce, next_nonce;

        #ifdef _ONE_NET_MULTI_HOP
            UInt8 hops = 0;
            BOOL mh = FALSE;
        #endif // ifdef _ONE_NET_MULTI_HOP //
		
	    #ifdef _ONE_NET_VERSION_2_X
		    on_nack_rsn_t nack_reason;
            const one_net_xtea_key_t* key = 0;
	    #endif

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.block_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !*txn || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the block data packet that was sent which
        // contains the encoded address (the destination did in the packet)
        // that the response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr((const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        #ifdef _ONE_NET_MULTI_HOP
            if(pid != ONE_NET_ENCODED_BLOCK_DATA_ACK
              && pid != ONE_NET_ENCODED_BLOCK_DATA_NACK
              && pid != ONE_NET_ENCODED_MH_BLOCK_DATA_ACK
              && pid != ONE_NET_ENCODED_MH_BLOCK_DATA_NACK)
        #else // ifdef _ONE_NET_MULTI_HOP //
            if(pid != ONE_NET_ENCODED_BLOCK_DATA_ACK
              && pid != ONE_NET_ENCODED_BLOCK_DATA_NACK)
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        {
            return ONS_BAD_PKT_TYPE;
        } // if the packet is not what the device is expecting //

        // read the nonces
        #ifndef _ONE_NET_VERSION_2_X	
            if((status = rx_nonces(&txn_nonce, &next_nonce)) != ONS_SUCCESS)
	    #else
            if((status = rx_nonces(&txn_nonce, &next_nonce)) != ONS_SUCCESS)
	    #endif
        {
            return status;
        } // if reading the nonces failed //

        #ifdef _ONE_NET_MULTI_HOP
            if(pid == ONE_NET_ENCODED_MH_BLOCK_DATA_ACK)
            {
                mh = TRUE;

                if((hops = on_hops_taken()) > ON_MAX_HOPS_LIMIT)
                {
                    return ONS_INVALID_DATA;
                } // if parsing the hops field was not successful //
            } // if a multi-hop packet was received //

            if(txn_nonce != (*txn)->expected_nonce
              || pid == ONE_NET_ENCODED_BLOCK_DATA_NACK
              || pid == ONE_NET_ENCODED_MH_BLOCK_DATA_NACK)
        #else // ifdef _ONE_NET_MULTI_HOP //
            if(txn_nonce != (*txn)->expected_nonce
              || pid == ONE_NET_ENCODED_BLOCK_DATA_NACK)
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        {
            (*txn)->retry++;

            if((*txn)->retry >= ON_MAX_RETRY)
            {
                #ifdef _ONE_NET_MULTI_HOP
                    pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                      ONS_BLOCK_FAIL, hops);
                #else // ifdef _ONE_NET_MULTI_HOP //
                    pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1,
                      ONS_BLOCK_FAIL);
                #endif // else _ONE_NET_MULTI_HOP is not defined //
                return ONS_BLOCK_FAIL;
            } // if the transaction has been tried too many times //

            if(txn_nonce != (*txn)->expected_nonce)
            {
                return ONS_INCORRECT_NONCE;
            } // if the nonce did not match //
        } // if the nonce did not match or it was a NACK //

        on_state = ON_LISTEN_FOR_DATA;
        #ifdef _ONE_NET_MULTI_HOP
            if(pid == ONE_NET_ENCODED_BLOCK_DATA_NACK
              || pid == ONE_NET_ENCODED_MH_BLOCK_DATA_NACK)
            {
                if((status = pkt_hdlr.block_txn_hdlr(txn, next_nonce,
                  ONS_RX_NACK, hops)) == ONS_SUCCESS)
                {
                    status = ONS_RX_NACK;
                } // if txn_hdlr successful //
            } // if a block data nack was received //
        #else // ifdef _ONE_NET_MULTI_HOP //
            if(pid == ONE_NET_ENCODED_BLOCK_DATA_NACK)
            {
                if((status = pkt_hdlr.block_txn_hdlr(txn, next_nonce,
                  ONS_RX_NACK)) == ONS_SUCCESS)
                {
                    status = ONS_RX_NACK;
                } // if txn_hdlr successful //
            } // if a block data nack was received //
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        else
        {
            #ifdef _ONE_NET_MULTI_HOP
                status = pkt_hdlr.block_txn_hdlr(txn, next_nonce, ONS_SUCCESS,
                  hops);
            #else // ifdef _ONE_NET_MULTI_HOP //
                status = pkt_hdlr.block_txn_hdlr(txn, next_nonce, ONS_SUCCESS);
            #endif // else _ONE_NET_MULTI_HOP is not defined //

            if(status == ONS_BLOCK_END)
            {
                // The block is completed
                (*txn)->data_len = (*txn)->pkt_size;

                // Try to send right away
                ont_set_timer(ONT_GENERAL_TIMER, 0);

                #ifdef _ONE_NET_MULTI_HOP
                    if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
                      mh ? ONE_NET_ENCODED_MH_BLOCK_TXN_ACK
                      : ONE_NET_ENCODED_BLOCK_TXN_ACK,
                      (const on_encoded_did_t * const)&src_did, 0, 0,
                      mh ? hops : 0)) != ONS_SUCCESS)
                #else // ifdef _ONE_NET_MULTI_HOP //
                    if((status = on_build_pkt((*txn)->pkt, &((*txn)->data_len),
                      ONE_NET_ENCODED_BLOCK_TXN_ACK,
                      (const on_encoded_did_t * const)&src_did, 0, 0))
                      != ONS_SUCCESS)
                #endif // else _ONE_NET_MULTI_HOP is not defined //
                {
                    block_complete = TRUE;
                    return status;
                } // if building the transaction ack failed //

                on_state = ON_SEND_PKT;
            } // if the block transaction is complete //
            else
            {
                ont_stop_timer(ONT_GENERAL_TIMER);
            } // else the block transaction is not yet complete //
        } // else a block data ack was received //

        return status;
    } // rx_block_resp_pkt //


    /*!
        \brief Receives the block data transaction ack.

        This is called by the receiving device of a block transaction when the
        transaction is complete.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized properly.
                ONS_BAD_PARAM If the parameter is invalid.
                ONS_READ_ERR If there was an error while reading the packet.
                ONS_BAD_PKT_TYPE If an unexpected packet type was received.
                For more return codes, see rx_pkt_addr.
    */
    static one_net_status_t rx_block_txn_ack(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.block_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !(*txn) || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the ack packet that was sent which contains
        // the encoded address (the destination did in the packet) that the
        // response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        #ifdef _ONE_NET_MULTI_HOP
            if(pid == ONE_NET_ENCODED_REPEAT_BLOCK_DATA
              || pid == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA)
        #else // ifdef _ONE_NET_MULTI_HOP //
            if(pid == ONE_NET_ENCODED_REPEAT_BLOCK_DATA)
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        {
            // don't bother reading the data since to be here, this device must
            // have already received the last block data packet, therefore the
            // sending device must not have received the response, so resend the
            // response (which happens to still be txn).
            on_state = ON_SEND_BLOCK_DATA_RESP;
        } // if block or repeat block packet //
        #ifdef _ONE_NET_MULTI_HOP
            else if(pid == ONE_NET_ENCODED_BLOCK_TXN_ACK
              || pid == ONE_NET_ENCODED_MH_BLOCK_TXN_ACK)
            {
                // received the end of the transaction, so clear the general
                // timer as it is not needed.  Also, don't worry about reading
                // the hops field since nothing is sent back.
                pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1, ONS_BLOCK_END,
                  0);
        #else // ifdef _ONE_NET_MULTI_HOP //
            else if(pid == ONE_NET_ENCODED_BLOCK_TXN_ACK)
            {
                // received the end of the transaction, so clear the general
                // timer as it is not needed
                pkt_hdlr.block_txn_hdlr(txn, ON_MAX_NONCE + 1, ONS_BLOCK_END);
        #endif // else _ONE_NET_MULTI_HOP is not defined //
            ont_stop_timer(ONT_GENERAL_TIMER);
            (*txn)->priority = ONE_NET_NO_PRIORITY;
            on_state = ON_LISTEN_FOR_DATA;
        } // else if a block txn ack //
        else
        {
            return ONS_BAD_PKT_TYPE;
        } // if an unexpected packet was received //

        return status;
    } // rx_block_txn_ack //


    /*!
        \brief Receive the response to a stream data packet.

        This function only receives Stream Keep Alive Packets.  Thise are
        expected in response to a Stream Data Packet this device sent.

        \param[in] txn The transaction being carried out.

        \return ONS_NOT_INIT If the device was not initialized
                ONS_BAD_PARAM If the parameter is not valid
                ONS_BAD_PKT_TYPE If a packet type was received that this device
                  is not looking for.
                ONS_STREAM_FAIL If the transaction has been tried too many
                  times.
                ONS_INCORRECT_NONCE If the received transaction nonce is not
                  what is expected.
                For more response values, see rx_pkt_addr, on_decode.
    */
    static one_net_status_t rx_stream_resp_pkt(on_txn_t ** txn)
    {
        one_net_status_t status;
        on_encoded_did_t src_did;
        UInt8 pid;
        UInt8 txn_nonce, next_nonce;

        #ifdef _ONE_NET_MULTI_HOP
            UInt8 hops = 0;
        #endif // ifdef _ONE_NET_MULTI_HOP //

	    #ifdef _ONE_NET_VERSION_2_X
		    on_nack_rsn_t nack_reason;
            const one_net_xtea_key_t* key = 0;
	    #endif

        // only need to check 1 handler since it is all or nothing
        if(!pkt_hdlr.stream_txn_hdlr)
        {
            return ONS_NOT_INIT;
        } // if this device was not initialized //

        if(!txn || !*txn || !(*txn)->pkt
          || (*txn)->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
        {
            return ONS_BAD_PARAM;
        } // if the parameter is invalid //

        // pkt in txn contains the stream data packet that was sent which
        // contains the encoded address (the destination did in the packet)
        // that the response is expected from.
        #ifdef _ONE_NET_MH_CLIENT_REPEATER
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
              != ONS_SUCCESS)
        #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
            if((status = rx_pkt_addr(
              (const on_encoded_did_t * const)
              &((*txn)->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did))
              != ONS_SUCCESS)
        #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
        {
            return status;
        } // if the packet is not for this device //

        // read the pid
        if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        switch(pid)
        {
            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:  // fall through
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
            {
                // handled below
                break;
            } // ONE_NET_ENCODED_STREAM_KEEP_ALIVE case //

            #ifdef _ONE_NET_MULTI_HOP
                case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
                case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
            #endif // ifdef _ONE_NET_MULTI_HOP //
            case ONE_NET_ENCODED_SINGLE_DATA:               // fall through
            case ONE_NET_ENCODED_REPEAT_SINGLE_DATA:
            {
                // remember the stream in case we need to come back to it.
                cur_stream = *txn;

                status = rx_single_data(pid,
                  (const on_encoded_did_t * const)&src_did, txn);

                if(*txn != cur_stream)
                {
                    if(status == ONS_STREAM_END)
                    {
                        // the stream ended so it does not need to be saved
                        cur_stream = 0;
                    } // if the stream has ended //

                    on_state = ON_SEND_SINGLE_DATA_RESP; 
                } // if a response is being sent back //

                return status;
                break;
            } // single data case //

            default:
            {
                return ONS_BAD_PKT_TYPE;
                break;
            } // default case //
        } // switch(pid) //

        // read the nonces
        #ifndef _ONE_NET_VERSION_2_X	
            if((status = rx_nonces(&txn_nonce, &next_nonce)) != ONS_SUCCESS)
	    #else
            if((status = rx_nonces(&txn_nonce, &next_nonce)) != ONS_SUCCESS)
	    #endif

        if(txn_nonce != (*txn)->expected_nonce)
        {
            (*txn)->retry++;

            if((*txn)->retry >= ON_MAX_RETRY)
            {
                cur_stream = 0;
                #ifdef _ONE_NET_MULTI_HOP
                    pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                      ONS_STREAM_FAIL, 0);
                #else // ifdef _ONE_NET_MULTI_HOP //
                    pkt_hdlr.stream_txn_hdlr(txn, ON_MAX_NONCE + 1,
                      ONS_STREAM_FAIL);
                #endif // else _ONE_NET_MULTI_HOP has not been defined //
                return ONS_STREAM_FAIL;
            } // if the transaction has been tried too many times //

            return ONS_INCORRECT_NONCE;
        } // if the nonce did not match //

        on_state = ON_LISTEN_FOR_DATA;
        #ifdef _ONE_NET_MULTI_HOP
            if(pid == ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE
              && (hops = on_hops_taken()) > ON_MAX_HOPS_LIMIT)
            {
                return ONS_INVALID_DATA;
            } // if parsing the hops field was not successful //

            status = pkt_hdlr.stream_txn_hdlr(txn, next_nonce, ONS_SUCCESS,
              hops);
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = pkt_hdlr.stream_txn_hdlr(txn, next_nonce, ONS_SUCCESS);
        #endif // else _ONE_NET_MULTI_HOP is not defined //
        ont_stop_timer(ONT_GENERAL_TIMER);

        return status;
    } // rx_stream_resp_pkt //
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //


/*!
    \brief Receives a data rate packet.

    Verifies the contents of the packet to make sure there were no bit errors.

    \param[in/out] txn If sending, contains the address the response data rate
      packet should be coming from.  If receiving the transaction, returns the
      data rate packet to send back (if the received packet was correct).
    \param[in] RECEIVER TRUE if this device is the receiver in the transaction.
                      FALSE if this device is the sender in the transaction.
    \param[out] hops If _ONE_NET_MULTI_HOP is enabled, the number of hops taken
      if it was a MULTI_HOP packet.  If it was not a MULTI_HOP data rate test, 0
      is returned.

    \return ONS_SUCCESS if the received packet was correct.
            ONS_BAD_PARAM if at least one parameter is invalid.
            ONS_BAD_PKT_TYPE if the packet type is not what is expected.
            ONS_INVALID_DATA if the received data was not correct.
            See rx_pkt_addr for more possible return codes.
*/
#ifdef _ONE_NET_MULTI_HOP
    static one_net_status_t rx_data_rate(on_txn_t * const txn,
      const BOOL RECEIVER, UInt8 * const hops)
#else // ifdef _ONE_NET_MULTI_HOP //
    static one_net_status_t rx_data_rate(on_txn_t * const txn,
      const BOOL RECEIVER)
#endif // else _ONE_NET_MULTI_HOP is not defined //
{
    one_net_status_t status;
    on_encoded_did_t src_did;
    UInt8 i;
    UInt8 pid, data_rate;
    UInt8 test_pattern[ON_TEST_PATTERN_SIZE];

    if(!txn || !(txn->pkt) || txn->pkt_size < ON_MIN_ENCODED_PKT_SIZE)
    {
        return ONS_BAD_PARAM;
    } // if the parameter is invalid //

    if(RECEIVER)
    {
        // the receiver isn't expecting the packet from anyone in particular.
        one_net_memmove(&(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]),
          ON_ENCODED_BROADCAST_DID, sizeof(ON_ENCODED_BROADCAST_DID));
    } // if the device is the receiver for the transaction //

    // pkt in txn contains the ack packet that was sent which contains
    // the encoded address (the destination did in the packet) that the
    // response is expected from.
    #ifdef _ONE_NET_MH_CLIENT_REPEATER
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did, 0))
          != ONS_SUCCESS)
    #else // ifdef _ONE_NET_MH_CLIENT_REPEATER //
        if((status = rx_pkt_addr((const on_encoded_did_t * const)
          &(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), &src_did)) != ONS_SUCCESS)
    #endif // else _ONE_NET_MH_CLIENT_REPEATER is not defined //
    {
        return status;
    } // if the packet is not for this device //

    // read the pid
    if(one_net_read(&pid, sizeof(pid)) != sizeof(pid))
    {
        return ONS_READ_ERR;
    } // if reading the pid failed //

    #ifdef _ONE_NET_MULTI_HOP
        if(pid != ONE_NET_ENCODED_DATA_RATE_TEST
          && pid != ONE_NET_ENCODED_MH_DATA_RATE_TEST)
    #else // ifdef _ONE_NET_MULTI_HOP //
        if(pid != ONE_NET_ENCODED_DATA_RATE_TEST)
    #endif // else _ONE_NET_MULTI_HOP is not defined //
    {
        return ONS_BAD_PKT_TYPE;
    } // not the expected pid //

    // read the data rate
    if(one_net_read(&data_rate, sizeof(data_rate)) != sizeof(data_rate))
    {
        return ONS_READ_ERR;
    } // if reading the data rate failed //

    if(data_rate != txn->pkt[ON_DATA_RATE_IDX])
    {
        return ONS_INVALID_DATA;
    } // if the received data rate is not as expected //

    // read the test pattern
    if(one_net_read(test_pattern, sizeof(test_pattern)) != sizeof(test_pattern))
    {
        return ONS_READ_ERR;
    } // if reading the test pattern failed //

    for(i = 0; i < ON_TEST_PATTERN_SIZE; i++)
    {
        if(test_pattern[i] != ON_TEST_PATTERN)
        {
            return ONS_INVALID_DATA;
        } // if the test pattern does not match //
    } // loop to verify the test pattern //

    #ifdef _ONE_NET_MULTI_HOP
        if(pid == ONE_NET_ENCODED_MH_DATA_RATE_TEST)
        {
            if((*hops = on_hops_taken()) > ON_MAX_HOPS_LIMIT)
            {
                return ONS_INVALID_DATA;
            } // if problem reading hops field //
        } // if it was a multi-hop packet //
    #endif // ifdef _ONE_NET_MULTI_HOP //

    if(RECEIVER)
    {
        // add the senders did as the destination did to complete the
        // response
        one_net_memmove(&(txn->pkt[ONE_NET_ENCODED_DST_DID_IDX]), src_did,
          sizeof(src_did));
        #ifdef _ONE_NET_MULTI_HOP
            if(pid == ONE_NET_ENCODED_MH_DATA_RATE_TEST)
            {
                status = on_build_hops(&(txn->pkt[txn->pkt_size - 1]),
                  ON_MAX_HOPS_LIMIT, ON_MAX_HOPS_LIMIT);
            } // if it was a multi-hop packet //
        #endif // ifdef _ONE_NET_MULTI_HOP //
    } // if this device is receiving during the test //

    return status;
} // rx_data_rate //


/*!
    \brief Finishes reception of a single data pkt

    \param[in] PID The PID that was received.  Should be
      ONE_NET_ENCODED_SINGLE_DATA, ONE_NET_ENCODED_REPEAT_SINGLE_DATA,
      ONE_NET_ENCODED_MH_SINGLE_DATA, or ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA.
    \param[in] SRC_DID Pointer to the did of the sender.
    \param[in/out] txn The transaction that is being carried out.

    \return ONS_READ_ERR If the hops field could not be read.
            ONS_INTERNAL_ERR If something unexpexted happened.
            See rx_payload & single_data_hdlr for more options.
*/
static one_net_status_t rx_single_data(const UInt8 PID,
  const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
{
    one_net_status_t status = ONS_INTERNAL_ERR;
    UInt8 raw_pld[ON_RAW_SINGLE_PLD_SIZE];

    #ifdef _ONE_NET_MULTI_HOP
        UInt8 hops = 0;
    #endif // ifdef _ONE_NET_MULTI_HOP //

    if((status = rx_payload(raw_pld, ON_ENCODED_SINGLE_PLD_SIZE))
      != ONS_SUCCESS)
    {
        return status;
    } // if receiving the packet was not successful //

    #ifdef _ONE_NET_MULTI_HOP
        if(PID == ONE_NET_ENCODED_MH_SINGLE_DATA
          || PID == ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA)
        {
            hops = on_hops_taken();
        } // if a multi-hop packet //

        status = (*pkt_hdlr.single_data_hdlr)(PID, SRC_DID, raw_pld, txn, hops);
    #else // ifdef _ONE_NET_MULTI_HOP //
        status = (*pkt_hdlr.single_data_hdlr)(PID, SRC_DID, raw_pld, txn);
    #endif // else _ONE_NET_MULTI_HOP is not defined //

    if(*txn)
    {
        on_state = ON_SEND_SINGLE_DATA_RESP;
    } // if a response is being sent back //
    else
    {
        on_state = ON_LISTEN_FOR_DATA;
    } // else if the device does not want to send a response back //

    return status;
} // rx_single_data //


#ifndef _ONE_NET_SIMPLE_DEVICE
    /*!
        \brief Finishes reception of a block data pkt

        \param[in] PID The PID that was received.  Should be
          ONE_NET_ENCODED_BLOCK_DATA, ONE_NET_ENCODED_REPEAT_BLOCK_DATA,
          ONE_NET_ENCODED_MH_BLOCK_DATA, or ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA
        \param[in] SRC_DID Pointer to the did of the sender.
        \param[in/out] txn The transaction that is being carried out.

        \return ONS_READ_ERR If the hops field could not be read.
                ONS_INTERNAL_ERR If something unexpexted happened.
                See rx_payload & block_data_hdlr for more options.
    */
    static one_net_status_t rx_block_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
    {
        one_net_status_t status = ONS_INTERNAL_ERR;
        UInt8 raw_pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];
        #ifdef _ONE_NET_MULTI_HOP
            UInt8 hops = 0;
        #endif // ifdef _ONE_NET_MULTI_HOP //

        if((status = rx_payload(raw_pld, ON_ENCODED_BLOCK_STREAM_PLD_SIZE))
          != ONS_SUCCESS)
        {
            return status;
        } // if receiving the packet was not successful //

        #ifdef _ONE_NET_MULTI_HOP
            if(PID == ONE_NET_ENCODED_MH_BLOCK_DATA
              || PID == ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA)
            {
                hops = on_hops_taken();
            } // if a multi-hop packet //

            status = (*pkt_hdlr.block_data_hdlr)(PID, SRC_DID, raw_pld, txn,
              hops);
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = (*pkt_hdlr.block_data_hdlr)(PID, SRC_DID, raw_pld, txn);
        #endif // else _ONE_NET_MULTI_HOP is not defined //

        if(status == ONS_BLOCK_END)
        {
            block_complete = TRUE;
        } // if the block transaction has ended //

        if((status == ONS_SUCCESS || status == ONS_BLOCK_END) && *txn)
        {
            on_state = ON_SEND_BLOCK_DATA_RESP;
        } // if a response is being sent back //

        return status;
    } // rx_block_data //


    /*!
        \brief Finishes reception of a stream data pkt

        \param[in] PID The PID that was received.  Should be
          ONE_NET_ENCODED_STREAM_DATA or ONE_NET_ENCODED_MH_STREAM_DATA.
        \param[in] SRC_DID Pointer to the did of the sender.
        \param[in/out] txn The transaction that is being carried out.

        \return ONS_READ_ERR If the hops field could not be read.
                ONS_INTERNAL_ERR If something unexpexted happened.
                See rx_payload & stream_data_hdlr for more options.
    */
    static one_net_status_t rx_stream_data(const UInt8 PID,
      const on_encoded_did_t * const SRC_DID, on_txn_t ** txn)
    {
        one_net_status_t status = ONS_INTERNAL_ERR;
        UInt8 raw_pld[ON_RAW_BLOCK_STREAM_PLD_SIZE];
        #ifdef _ONE_NET_MULTI_HOP
            UInt8 hops = 0;
        #endif // ifdef _ONE_NET_MULTI_HOP //

        if((status = rx_payload(raw_pld, ON_ENCODED_BLOCK_STREAM_PLD_SIZE))
          != ONS_SUCCESS)
        {
            return status;
        } // if receiving the packet was not successful //

        #ifdef _ONE_NET_MULTI_HOP
            if(PID == ONE_NET_ENCODED_MH_STREAM_DATA)
            {
                hops = on_hops_taken();
            } // if a multi-hop packet //

            status = (*pkt_hdlr.stream_data_hdlr)(PID, SRC_DID, raw_pld, txn,
              hops);
        #else // ifdef _ONE_NET_MULTI_HOP //
            status = (*pkt_hdlr.stream_data_hdlr)(PID, SRC_DID, raw_pld, txn);
        #endif // else _ONE_NET_MULTI_HOP is not defined //

        if(*txn)
        {
            if(status == ONS_SUCCESS)
            {
                on_state = ON_SEND_STREAM_DATA_RESP;
            } // if successful //
            else if(status == ONS_STREAM_END)
            {
                cur_stream = 0;
                on_state = ON_SEND_SINGLE_DATA_PKT;
            } // else if ending the stream //
        } // if a response is being sent back //
 
        return status;
    } // rx_stream_data //
#endif // ifndef _ONE_NET_SIMPLE_DEVICE //


/*!
    \brief Reads in and decodes the payload portion of data packets.

    \param[out] raw_pld The raw payload of the packet that was received.  The
      size of raw_pld must be ENCODED_LEN * 6 / 8.
    \param[in] ENCODED_LEN The number of bytes to read from the rf interface.

    \return ONS_BAD_PARAM If a parameter passed in is not valid
            ONS_READ_ERR If reading the encoded payload failed.
            For more return values, see on_decode.
*/
static one_net_status_t rx_payload(UInt8 * const raw_pld,
  const UInt8 ENCODED_LEN)
{
    UInt8 rx_encoded_pld[ON_ENCODED_BLOCK_STREAM_PLD_SIZE];

    if(!raw_pld || ENCODED_LEN > sizeof(rx_encoded_pld))
    {
        return ONS_BAD_PARAM;
    } // if parameter is invalid //

    // read the encoded payload
    if(one_net_read(rx_encoded_pld, ENCODED_LEN) != ENCODED_LEN)
    {
        return ONS_READ_ERR;
    } // if reading the encoded payload failed //

    // decode the payload
    return on_decode(raw_pld, rx_encoded_pld, ENCODED_LEN);
} // rx_payload //


/*!
    \brief Receives the nonces from the rf interface.

    Receives the txn_nonce and resp_nonce from the rf_interface.  The decoded
    and parsed nonces are returned.

    \param[out] txn_nonce The decoded & parsed transaction nonce.
    \param[out] next_nonce The decoded & parsed next nonce.

    \return ONS_SUCCESS If the nonces were successfully received and parsed.
            ONS_BAD_PARAM If any of the parameters are invalid.
            ONS_READ_ERR If either nonce could not be read from the rf
              interface.
*/
static one_net_status_t rx_nonces(UInt8 * const txn_nonce,
  UInt8 * const next_nonce)
{
    one_net_status_t status;
    UInt8 encoded_nonce;

    if(!txn_nonce || !next_nonce)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // txn nonce
    if(one_net_read(&encoded_nonce, sizeof(encoded_nonce))
      != sizeof(encoded_nonce))
    {
        return ONS_READ_ERR;
    } // if reading the transaction nonce failed //

    if((status = on_decode(txn_nonce, &encoded_nonce, sizeof(encoded_nonce)))
      != ONS_SUCCESS)
    {
        return status;
    } // if decoding the transaction nonce failed //

    // next nonce
    if(one_net_read(&encoded_nonce, sizeof(encoded_nonce))
      != sizeof(encoded_nonce))
    {
        return ONS_READ_ERR;
    } // if reading the transaction nonce failed //

    if((status = on_decode(next_nonce, &encoded_nonce, sizeof(encoded_nonce)))
      != ONS_SUCCESS)
    {
        return status;
    } // if decoding the transaction nonce failed //

    *txn_nonce >>= ON_TXN_NONCE_SHIFT;
    *txn_nonce &= ON_TXN_NONCE_PARSE_MASK;

    *next_nonce >>= ON_TXN_NONCE_SHIFT;
    *next_nonce &= ON_TXN_NONCE_PARSE_MASK;

    return ONS_SUCCESS;
} // rx_nonces //


#ifdef _ONE_NET_VERSION_2_X
// temporary function while porting to 2.0.  Right now it's the same as the
// 1.6 version.
/*static one_net_status_t rx_nonces_2_X(UInt8 * const txn_nonce, 
  UInt8 * const next_nonce, on_nack_rsn_t* const nack_reason,
  const one_net_xtea_key_t * const key, const on_data_t type)*/
static one_net_status_t rx_nonces_2_X(UInt8 * const txn_nonce,
  UInt8 * const next_nonce)
{
    one_net_status_t status;
    UInt8 encoded_nonce;

    if(!txn_nonce || !next_nonce)
    {
        return ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    // txn nonce
    if(one_net_read(&encoded_nonce, sizeof(encoded_nonce))
      != sizeof(encoded_nonce))
    {
        return ONS_READ_ERR;
    } // if reading the transaction nonce failed //

    if((status = on_decode(txn_nonce, &encoded_nonce, sizeof(encoded_nonce)))
      != ONS_SUCCESS)
    {
        return status;
    } // if decoding the transaction nonce failed //

    // next nonce
    if(one_net_read(&encoded_nonce, sizeof(encoded_nonce))
      != sizeof(encoded_nonce))
    {
        return ONS_READ_ERR;
    } // if reading the transaction nonce failed //

    if((status = on_decode(next_nonce, &encoded_nonce, sizeof(encoded_nonce)))
      != ONS_SUCCESS)
    {
        return status;
    } // if decoding the transaction nonce failed //

    *txn_nonce >>= ON_TXN_NONCE_SHIFT;
    *txn_nonce &= ON_TXN_NONCE_PARSE_MASK;

    *next_nonce >>= ON_TXN_NONCE_SHIFT;
    *next_nonce &= ON_TXN_NONCE_PARSE_MASK;

    return ONS_SUCCESS;
} // rx_nonces_2_X
#endif


#ifdef _ONE_NET_MH_CLIENT_REPEATER
    /*!
        \brief Reads in the rest of a Multi-Hop packet (after the address),
          changes the hops field, and sends the packet back out.

        \param txn[out] Returns the pointer to the mh_hop txn to be sent.

        \return ONS_SUCCESS If the packet was successfully handled either by
                  sending the packet on, or droping the packet because hops left
                  reached 0.
                ONS_READ_ERR if the data from the rf interface could not be
                  read.
                ONS_UNHANDLED_PKT If it was not a Multi-Hop packet.
    */
    static one_net_status_t repeat_mh_pkt(on_txn_t ** txn)
    {
        one_net_status_t status;
        UInt8 max_hops, hops_left;

        // read the pid
        if(one_net_read(&(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX]),
          ON_ENCODED_PID_SIZE) != ON_ENCODED_PID_SIZE)
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        switch(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX])
        {
            case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
            {
                mh_txn.data_len = ON_ENCODED_INVITE_PKT_LEN;
                break;
            } // ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT case //

            case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:    // fall through
            case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
            case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK:   // fall through
            case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:     // fall through
            case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK:    // fall through
            case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
            {
                mh_txn.data_len = ON_ACK_NACK_LEN;
                break;
            } // ACK/NACK cases //

            case ONE_NET_ENCODED_MH_SINGLE_TXN_ACK:     // fall through
            case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
            {
                mh_txn.data_len = ON_TXN_ACK_LEN;
                break;
            } // multi-hop single/block transaction ack case //

            case ONE_NET_ENCODED_MH_SINGLE_DATA:        // fall through
            case ONE_NET_ENCODED_MH_REPEAT_SINGLE_DATA:
            {
                mh_txn.data_len = ON_ENCODED_SINGLE_DATA_LEN;
                break;
            } // (repeat)single data case //

            case ONE_NET_ENCODED_MH_BLOCK_DATA:         // fall through
            case ONE_NET_ENCODED_MH_REPEAT_BLOCK_DATA:  // fall through
            case ONE_NET_ENCODED_MH_STREAM_DATA:
            {
                // don't read in the hops field (which is included in the
                // length of this packet).
                mh_txn.data_len = ONE_NET_MAX_ENCODED_PKT_LEN
                  - ON_ENCODED_HOPS_SIZE;
                break;
            } // block and stream data case //

            case ONE_NET_ENCODED_MH_DATA_RATE_TEST:
            {
                mh_txn.data_len = ON_DATA_RATE_PKT_LEN;
                break;
            } // ONE_NET_ENCODED_MH_DATA_RATE_TEST case //

            default:
            {
                return ONS_UNHANDLED_PKT;
                break;
            } // default case //
        } // switch(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX]) //

        // read the rest of the packet (except for the hops field).  +1 since
        // 1 more byte than current index has been read.
        if(one_net_read(&(mh_txn.pkt[ONE_NET_ENCODED_PID_IDX
          + ON_ENCODED_PID_SIZE]), mh_txn.data_len
          - (ONE_NET_ENCODED_PID_IDX + 1)) != mh_txn.data_len
          - (ONE_NET_ENCODED_PID_IDX + 1))
        {
            return ONS_READ_ERR;
        } // if reading the pid failed //

        if((status = on_read_and_parse_hops(&max_hops, &hops_left))
          != ONS_SUCCESS)
        {
            return status;
        } // if reading and parsing the hops field failed //

        if(hops_left)
        {
            hops_left--;
            if((status = on_build_hops(&(mh_txn.pkt[mh_txn.data_len++]),
              max_hops, hops_left)) == ONS_SUCCESS)
            {
                // successfully received and rebuilt the hops field of a
                // Multi-Hop packet, so now send it back out.
                *txn = &mh_txn;
                on_state = ON_SEND_PKT;
            } // if on_build_hops was successful //
        } // if hops remain //

        return status;
    } // repeat_mh_pkt //
#endif // ifdef _ONE_NET_MH_CLIENT_REPEATER //

#ifdef _ONE_NET_DEBUG
    void one_net_debug(UInt8 debug_type, UInt8 * data, UInt16 length)
    {
        tick_t ticks;
        UInt16 low_tick_count;
        UInt8 i;

        ticks = get_tick_count();
        low_tick_count = (UInt16) ticks;

        // print the debug message prefix
        oncli_send_msg(ONCLI_DEBUG_PREFIX_STR, low_tick_count, debug_type);
        oncli_send_msg(ONCLI_DEBUG_LENGTH_STR, length);

        //
        // display data from data as needed
        //
#if 1   
        if (length < 10)
        {
            //
            // it must not be a full message, just print the debug data
            //
            for (i=0; i<length; i++)
            {
                oncli_send_msg(ONCLI_DEBUG_BYTE_STR, data[i]);
            }
        }
        else
        {
            //
            // this must be a message, print the interesting parts to 
            // save time
            //
            for (i=4; i<=17; i++)
            {
                if (i < length)
                {
                    oncli_send_msg(ONCLI_DEBUG_BYTE_STR, data[i]);
                }
                else
                {
                    break;
                }
            }
        }
#endif

        // print the debug message suffix
        oncli_send_msg(ONCLI_DEBUG_SUFFIX_STR);

        // UART output seems to interfere with RF output
        while(uart_tx_bytes_free() < uart_tx_buffer_size());

    } // one_net_debug //
#endif


//! @} ONE-NET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE_NET

