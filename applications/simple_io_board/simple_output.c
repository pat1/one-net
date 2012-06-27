//! \defgroup SIMPLE_RELAY
//! @{

/*
    Copyright (c) 2007, Threshold Corporation
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
    \file output_example.c
    \brief Quad relay output board.
    
    This file is the main application for a simple ONE-NET quad relay output
    device.

    \version 0.4
*/

#include "client_util.h"
#include "one_net_client.h"
#include "one_net_port_specific.h"
#include "io_port_mapping.h"
#include "hal_adi.h"
#include "pal.h"
#include "tal.h"
#ifdef HAS_LEDS
    #include "one_net_led.h"
#endif
#include "tick.h"

//==============================================================================
//                                  CONSTANTS
//! \defgroup simple_relay_const
//! \ingroup SIMPLE_RELAY
//! @{



//! The unique key for this device.  This key is used when this device is added
//! to the network.
// Changed to be able to use new cli invite command in the master.
// The first two groups are duplicated in the upper two
const one_net_xtea_key_t DEV_KEY = {
    '4', '8', 'u', 't', 
    'p', 'u', 't', '7',
    '4', '8', 'u', 't', 
    'p', 'u', 't', '7'
};

const ona_unit_type_count_t ONE_NET_DEVICE_UNIT_TYPE[ONE_NET_NUM_UNIT_TYPES]
  = {{ONA_OUTLET, ONE_NET_NUM_UNITS}};

//! @} simple_relay_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup simple_relay_typedefs
//! \ingroup SIMPLE_RELAY
//! @{

//! @} simple_relay_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup simple_relay_pri_var
//! \ingroup SIMPLE_RELAY
//! @{


// Dec. 3, 2011
#if 0
//! Timer to turn off the transmit LED
static tick_t tx_led_timer = 0;

//! Timer to turn off the receive LED
static tick_t rx_led_timer = 0;

//! the voltage status reading
static UInt8 voltage_status = ONA_VOLTAGE_GOOD;
#endif

//! @} simple_relay_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS
//! \defgroup simple_relay_pri_func
//! \ingroup SIMPLE_RELAY
//! @{

static BOOL set_output(UInt8 unit, BOOL ON);
static BOOL get_output(UInt8 unit, UInt8* status);

//! @} simple_relay_pri_func
//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup simple_relay_pub_func
//! \ingroup SIMPLE_RELAY
//! @{

void one_net_client_invite_result(const on_raw_did_t * const RAW_DID,
  one_net_status_t status)
{
} // one_net_client_invite_result //


on_message_status_t one_net_client_handle_single_pkt(const UInt8* const raw_pld,
  on_msg_hdr_t* const msg_hdr, const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, on_ack_nack_t* const ack_nack)
{
    // Rough skeleton -- this will change!
    
    
    UInt8 src_unit, dst_unit, msg_type, msg_data, original_state;
    ona_msg_class_t msg_class;
    UInt32 tmp;
    #ifdef PEER
    ona_msg_class_t original_class;
    BOOL forward_to_peer = TRUE;
    #endif
    
    ack_nack->nack_reason = ON_NACK_RSN_NO_ERROR;
    ack_nack->handle = ON_ACK;
    
    if(msg_hdr->msg_type != ON_APP_MSG)
    {
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        return ON_MSG_CONTINUE;
    }
    
    on_parse_app_pld(raw_pld, &src_unit, &dst_unit, &msg_class, &msg_type,
      &tmp);
    msg_data = (UInt8) tmp;
    
    #ifdef PEER  
    original_class = msg_class;
    #endif
      
    if(!get_output(dst_unit, &original_state))
    {
        // no need to change handle.  ON_ACK and ON_NACK are the same.
        ack_nack->nack_reason = ON_NACK_RSN_INVALID_UNIT_ERR;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_class == ONA_COMMAND)
    {
        msg_class = ONA_STATUS_COMMAND_RESP;
        if(msg_data == ONA_TOGGLE)
        {
            msg_data = !original_state;
            #ifdef PEER
            forward_to_peer = FALSE;
            #endif
        }
    }
    else if(msg_class == ONA_STATUS_CHANGE)
    {
        // we'll treat this as a command
        msg_class = ONA_STATUS_COMMAND_RESP;
    }
    else if(ONA_IS_STATUS_MESSAGE(msg_class))
    {
        // we aren't interested in this message, but we'll ACK it anyway.
        return ON_MSG_CONTINUE;
    }
    else if(msg_class == ONA_QUERY)
    {
        msg_data = original_state;
        msg_class = ONA_STATUS_QUERY_RESP;
    }
    else if(msg_class == ONA_FAST_QUERY)
    {
        msg_data = original_state;
        msg_class = ONA_STATUS_FAST_QUERY_RESP;
    }
    else
    {
        // NACK it.  We don't handle it
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        return ON_MSG_CONTINUE;
    }
    
    if(msg_data != ONA_ON && msg_data != ONA_OFF)
    {
        // NACK it.  We don't handle it
        ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
        return ON_MSG_CONTINUE;
    }
    
    // source and destination are reversed
    put_src_unit(dst_unit, ack_nack->payload->status_resp);
    put_dst_unit(src_unit, ack_nack->payload->status_resp);
    put_msg_data(msg_data, ack_nack->payload->status_resp);
    put_msg_type(ONA_SWITCH, ack_nack->payload->status_resp);
    
    if(msg_class == ONA_STATUS_COMMAND_RESP)
    {
        if(msg_type != ONA_SWITCH)
        {
            // NACK it.  We don't handle it
            ack_nack->nack_reason = ON_NACK_RSN_DEVICE_FUNCTION_ERR;
            return ON_MSG_CONTINUE;
        }
        
        set_output(dst_unit, msg_data);

        #ifdef PEER
        // we may forward this message to our peer list
        if(forward_to_peer)
        {
            put_msg_class(original_class, ack_nack->payload->status_resp);

            if(one_net_send_single(ONE_NET_RAW_SINGLE_DATA, ON_APP_MSG,
              ack_nack->payload->status_resp, ONA_SINGLE_PACKET_PAYLOAD_LEN,
              ONE_NET_HIGH_PRIORITY, NULL, NULL, TRUE, dst_unit))
            {
            }
        }
        #endif
    }

    ack_nack->handle = ON_ACK_STATUS;
    put_msg_class(msg_class, ack_nack->payload->status_resp); 
    return ON_MSG_CONTINUE;
} // one_net_client_handle_single_pkt //


void one_net_client_single_txn_status(on_message_status_t status,
  UInt8 retry_count, on_msg_hdr_t msg_hdr, const UInt8* data,
  const on_raw_did_t *dst, on_ack_nack_t* ack_nack)
{
} // one_net_client_single_txn_status //


void one_net_client_client_remove_device(void)
{
    clr_flash();
    one_net_client_look_for_invite(&DEV_KEY);
} // one_net_client_client_remove_device //


one_net_status_t one_net_client_reset_client(one_net_xtea_key_t* invite_key)
{
    return one_net_client_look_for_invite(invite_key);
}


/*!
    \brief Returns a pointer to the invite key to use in for joining a network.
    
    \return A pointer to the invite key to use.
*/
one_net_xtea_key_t* one_net_client_get_invite_key(void)
{
    return &DEV_KEY;
}


void one_net_client_client_removed(const on_raw_did_t * const raw_did,
    BOOL this_device_removed)
{
}


void one_net_client_client_added(const on_raw_did_t * const raw_did)
{
}


on_message_status_t one_net_client_handle_ack_nack_response(
  UInt8* const raw_pld, on_msg_hdr_t* const msg_hdr,
  const on_msg_hdr_t* const resp_msg_hdr,
  on_ack_nack_t* const resp_ack_nack,
  const on_raw_did_t* const src_did,
  const on_raw_did_t* const repeater_did, UInt8* const retries)
{
    return ON_MSG_CONTINUE;
}



//! @} simple_relay_pub_func
//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION
//! \defgroup simple_relay_pri_func
//! \ingroup SIMPLE_RELAY
//! @{

/*!
    \brief Sets the output for the given unit.
    
    \param[in] unit The unit to set.
    \param[in] ON   TRUE if setting the output pin
                    FALSE if clearing the output pin

    \return TRUE if the operation was successful
            FALSE if the operation was not successful (such as an invalid unt)
*/
static BOOL set_output(UInt8 unit, BOOL ON)
{
    switch(unit)
    {
        #if _NUM_IO_UNITS > 0
            case 0:
            {
                OUTPUT1 = ON;
                break;
            } // unit 0 case //
        #endif // #if _NUM_IO_UNITS > 0 //

        #if _NUM_IO_UNITS > 1
            case 1:
            {
                OUTPUT2 = ON;
                break;
            } // unit 1 case //
        #endif // #if _NUM_IO_UNITS > 1 //

        #if _NUM_IO_UNITS > 2
            case 2:
            {
                OUTPUT3 = ON;
                break;
            } // unit 3 case //
        #endif // #if _NUM_IO_UNITS > 2 //

        #if _NUM_IO_UNITS > 3
            case 3:
            {
                OUTPUT4 = ON;
                break;
            } // unit 4 case //
        #endif // #if _NUM_IO_UNITS > 3 //

        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    
    return TRUE;
} // set_output //


/*!
    \brief Returns the current status for one of the units
    
    \param[in] unit The unit to get the status for.
    \param[out] status The status of the unit.  Either ONA_ON or ONA_OFF.
    
    \return TRUE if returning the status was successful
            FALSE if returning the status was not successful (invalid params or
              and invalid unit).
*/
static BOOL get_output(UInt8 unit, UInt8* status)
{
    UInt8 stat8; /* local variable for get_output() */

    if(!status)
    {
        return FALSE;
    } // if any of the parameters are invalid //
    
    switch(unit)
    {
        #if _NUM_IO_UNITS > 0
            case 0:
            {
                stat8 = OUTPUT1;
                break;
            } // unit 0 case //
        #endif // #if _NUM_IO_UNITS > 0 //

        #if _NUM_IO_UNITS > 1
            case 1:
            {
                stat8 = OUTPUT2;
                break;
            } // unit 1 case //
        #endif // #if _NUM_IO_UNITS > 1 //

        #if _NUM_IO_UNITS > 2
            case 2:
            {
                stat8 = OUTPUT3;
                break;
            } // unit 2 case //
        #endif // #if _NUM_IO_UNITS > 2 //

        #if _NUM_IO_UNITS > 3
            case 3:
            {
                stat8 = OUTPUT4;
                break;
            } // unit 3 case //
        #endif // #if _NUM_IO_UNITS > 3 //
        
        default:
        {
            return FALSE;
            break;
        } // default case //
    } // switch(unit) //
    *status = stat8;
    return TRUE;
} // get_output //


/*!
    \brief The entry point into the application

    \param void

    \return void
*/
void main(void)
{
    const UInt8 * PARAM_PTR = NULL;
    UInt16 param_len = 0;
    #ifdef PEER
    const UInt8* peer_param_ptr = NULL;
    UInt16 peer_param_len = 0;
    #endif
    one_net_status_t status = ONS_NOT_JOINED;

    INIT_PORTS();
    
    // set up the transceiver first since the CLKOUT from the transceiver will
    // be used as the main clock for the processor
    TAL_INIT_TRANSCEIVER();
    INIT_PROCESSOR(TRUE);
    
    #ifdef HAS_LEDS
        initialize_leds();
    #endif    
    
    INIT_TICK();
    ENABLE_GLOBAL_INTERRUPTS();
    FLASH_ERASE_CHECK();

    #ifdef PEER
    if(((PARAM_PTR = read_param(ONE_NET_CLIENT_FLASH_NV_DATA, &param_len))
      != NULL) && ((peer_param_ptr = read_param(ONE_NET_CLIENT_FLASH_PEER_DATA,
      &peer_param_len)) != NULL))
    #else
    if((PARAM_PTR = read_param(&param_len)) != NULL)
    #endif
    {
        #ifdef PEER
        status = one_net_client_init(PARAM_PTR, param_len, peer_param_ptr,
          peer_param_len);
        #else
        status = one_net_client_init(PARAM_PTR, param_len);
        #endif
    } // if the settings were successfully returned //
    
    if(status != ONS_SUCCESS)
    {
        one_net_client_look_for_invite(&DEV_KEY);
    } // else join a network //

    while(1)
    {
        // Dec. 3, 2011 -- commenting out
        #if 0
        if(tx_led_timer && tx_led_timer < one_net_tick())
        {
            TURN_OFF(TX_LED);
            tx_led_timer = 0;
        } // if time to turn off the tx led //
        
        if(rx_led_timer && rx_led_timer < one_net_tick())
        {
            TURN_OFF(RX_LED);
            rx_led_timer = 0;
        } // if time to turn off the rx led //
        #endif

        one_net_client();
    } // main loop //
} // main //

//! @} simple_relay_pri_func
//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} SIMPLE_RELAY
