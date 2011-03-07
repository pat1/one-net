#ifndef _ONE_NET_CLIENT_PORT_SPECIFIC_H
#define _ONE_NET_CLIENT_PORT_SPECIFIC_H

//! \defgroup ONE_NET_CLIENT_port_specific CLIENT Specific ONE-NET functionality
//! \ingroup ONE-NET_port_specific 
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
    \file one_net_client_port_specific.h
    \brief CLIENT specific ONE-NET declarations.

    These are interfaces for application and hardware specific functions that
    must be implemented for the ONE-NET project to compile (and work).
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "one_net_port_specific.h"

#include "one_net_client_port_const.h"
#include "one_net_client.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE_NET_CLIENT_port_specific_const
//! \ingroup ONE_NET_CLIENT_port_specific
//! @{

//! @} ONE_NET_CLIENT_port_specific_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE_NET_CLIENT_port_specific_typedefs
//! \ingroup ONE_NET_CLIENT_port_specific
//! @{
    
//! @} ONE_NET_CLIENT_port_specific_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE_NET_CLIENT_port_specific_pub_var
//! \ingroup ONE_NET_CLIENT_port_specific
//! @{

//! @} ONE_NET_CLIENT_port_specific_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE_NET_CLIENT_port_specific_pub_func
//! \ingroup ONE_NET_CLIENT_port_specific
//! @{

/*!
    \brief Callback when the CLIENT has successfully joined a network.
    
    This function is called by ONE-NET after a CLIENT has successfully joined
    a network in which it had been invited (received the invite packet).

    \param[in] RAW_DID The did assigned to the CLIENT.
    \param[in] MASTER_DID The did of the MASTER

    \return void
*/
void one_net_client_joined_network(const one_net_raw_did_t * const RAW_DID,
  const one_net_raw_did_t * const MASTER_DID);


/*!
    \brief Callback for the application to handle the received packet.

    This function is application dependent.  It is called by ONE-NET when a
    CLIENT receives a packet.  If the packet causes the CLIENT to send a
    transaction back to the sender, the CLIENT must remember the address of the
    sender and pass those back in when it sends the response.

    \param[in] RX_PLD The decoded, decrypted payload that was received.
    \param[in] RX_PLD_LEN The number of bytes received
    \param[in] SRC_ADDR The raw address of the sender.

    \return TRUE If it was a valid packet (and it will be handled)
            FALSE If it was not a valid packet and is being ignored.
*/
BOOL one_net_client_handle_single_pkt(const UInt8 * RX_PLD,
  const UInt16 RX_PLD_LEN, const one_net_raw_did_t * const SRC_ADDR);


/*!
    \brief The status of a single transaction.

    Callback to report the status of sending an application single data packet.
    A SINGLE_END will be sent for every single transaction initiated by the
    CLIENT whether the data was sent to a specific device, or the peer list.
    ONS_SINGLE_END will be returned after the status for the last device is
    returned.  The did returned for the ONS_SINGLE_END is the DID of the
    destination for the last single transaction that was sent.

    \param[in] STATUS The status of the transaction.  As long as the status is
      not ONS_SINGLE_END, the data should be updated with NEXT_DST_UNIT.
      ONS_SUCCESS if the single transaction was successfully been queued (dst is
        0), or the single transaction to DST was successful.
      ONS_SINGLE_FAIL if the transaction to DST failed
      ONS_SINGLE_END If the transaction has been sent to all peers
        and the MASTER (if required).
    \param[in] RETRY_COUNT The number of times that the packet had to be resent.
      0 will be passed in when this function is called to alert the application
      that the single transaction is complete (done sending to all the peers in
      the peer list).
    \param[in/out] DATA The data that was sent by the application (what was
      passed to client_send_single).  This value may be 0 if the data
      could not be reliably returned to the app.
    \param[in] DST The raw did of where the packet was sent.  This can be 0 if
      an internal error occured.  If such an event occurs, then the destination
      DID was not decoded properly.
    \param[in] NEXT_DST_UNIT The destination unit in the next peer that the
      packet is going to be sent to.

    \return void
*/
void one_net_client_single_txn_status(const one_net_status_t STATUS,
  const UInt8 RETRY_COUNT, const UInt8 * const DATA,
  const one_net_raw_did_t * const DST);


#ifndef _ONE_NET_SIMPLE_CLIENT
    /*!
        \brief Called when a block or stream request is received.

        This allows the app to decide if it is willing and able to send or
        receive the requested block or stream transaction.

        \param[in] TYPE The type of transaction to be performed (ON_BLOCK or
          ON_STREAM only).
        \param[in] SEND TRUE if this device is to send the transaction.
                        FALSE if this device is to receive the transaction.
        \param[in] DATA_TYPE The type of data to be sent or received.
        \param[in] DATA_LEN 0 if a ON_STREAM transaction, else the total number
          of bytes to be sent/received for a block transaction.
        \param[in] DID The device ID of the device making the request.

        \return TRUE if the request is to be granted.
                FALSE if the request is to be ignored.
    */
    BOOL one_net_client_txn_requested(const UInt8 TYPE, const BOOL SEND,
      const UInt16 DATA_TYPE, const UInt16 DATA_LEN,
      const one_net_raw_did_t * const DID);


    /*!
        \brief Callback for the application to handle the received block packet.

        \param[in] PLD The decoded, decrypted payload that was received.
        \param[in] LEN The length of PLD in bytes.
        \param[in] SRC_ADDR The raw address of the sender

        \return TRUE if it was a valid packet (and it will be handled).
                FALSE If it was not a valid packet and is being ignored
    */
    BOOL one_net_client_handle_block_pkt(const UInt8 * PLD, const UInt16 LEN,
      const one_net_raw_did_t * const SRC_ADDR);


    /*!
        \brief Callback to report the status of a block transaction.

        \param[in] STATUS The status of the transaction.
        \param[in] DID The device ID of the other device involved in the txn.

        \return void
    */
    void one_net_client_block_txn_status(const one_net_status_t STATUS,
      const one_net_raw_did_t * const DID);


    /*!
        \brief Callback to report the status of a stream transaction.

        \param[in] STATUS The status of the transaction.
        \param[in] DID The device ID of the other device involved in the txn.

        \return void
    */
    void one_net_client_stream_txn_status(const one_net_status_t STATUS,
      const one_net_raw_did_t * const DID);


    /*!
        \brief Callback for the application to handle the received stream
          packet.

        \param[in] PLD The decoded, decrypted payload that was received.
        \param[in] LEN The length of PLD in bytes.
        \param[in] SRC_ADDR The raw address of the sender

        \return TRUE if it was a valid packet (and it will be handled).
                FALSE If it was not a valid packet and is being ignored
    */
    BOOL one_net_client_handle_stream_pkt(const UInt8 * PLD, const UInt16 LEN,
      const one_net_raw_did_t * const SRC_ADDR);


    /*!
        \brief Callback to retrieve the next payload for a block or stream 
         transaction
        
        The application needs to keep track of where in the block or stream
        transaction it is.  The ONE-NET code will keep track of the last block
        for retransmissions.  This function will be called when the ONE-NET
        code wants to retrieve the next block or stream payload for
        transmission.  The implementer must set len to the number of bytes
        being sent, and return a pointer to the data to be sent.
        
        \param[in] TYPE The type of transaction.  Must be either ON_BLOCK or
          ON_STREAM
        \param[out] len The number of bytes to be sent
          (must be <= ONE_NET_RAW_BLOCK_STREAM_DATA_LEN)
        \param[in] DST The device receiving the next payload.

        \return Pointer to the data to be sent.
    */
    const UInt8 * one_net_client_next_payload(const UInt8 TYPE, UInt16 * len,
      const one_net_raw_did_t * const DST);
#endif // ifndef _ONE_NET_SIMPLE_CLIENT //


/*!
    \brief Called when settings have been changed that need to be saved.

    This data should be saved into non-volatile memory and passed in to
    initialize ONE-NET when the device starts up.

    \param[in] PARAM The parameters to save in non-volatile memory.
    \param[in] PARAM_LEN The number of bytes to save.

    \return void
*/
void one_net_client_save_settings(const UInt8 * PARAM, const UInt16 PARAM_LEN);


/*!
    \brief Called when a CLIENT has been removed from the network.
    
    If this is called, it means that the device has received a Remove Device
    admin message and has been removed from the network.

    \param void

    \return void
*/
void one_net_client_client_remove_device(void);

//! @} ONE_NET_CLIENT_port_specific_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE_NET_CLIENT_port_specific

#endif // _ONE_NET_CLIENT_PORT_SPECIFIC_H //

