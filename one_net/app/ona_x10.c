//! \addtogroup ONE-NET_APP_X10
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
    \file ona_x10.c
    \brief Implementation of x10 msg functions.

    This is the implementation of functions to send an parse x10 msgs.
*/

#include "ona_x10.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_APP_X10_const
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_APP_X10_typedefs
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_APP_X10_pri_var
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_APP_X10_pri_func
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_APP_X10_pub_func
//! \ingroup ONE-NET_APP_X10
//! @{

/*!
    \brief Send a x10 simple msg

    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] HOUSE, x10 house
    \param[in] UNIT, x10 unit
    \param[in] CMD, x10 cmd
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_x10_simple(const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const UInt8 HOUSE, const UInt8 UNIT, const UInt8 CMD,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    UInt8 temp = 0x00;

    one_net_int16_to_byte_stream(ONA_X10_SIMPLE, &payload[ONA_MSG_HDR_IDX]);
 
    // message data format for x10 simple:
    //     payload[2]        payload[3]         payload[4]
    // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
    // | HOUSE |    UNIT   | X10_CMD |       PADDING        | 

    // put house into high 4 bits
    temp = HOUSE;
    temp <<= 4;                     // move house to high nibble
    payload[2] = temp;

    // put unit into low 4 bits & high 1 bit of next byte
    temp = UNIT;
    temp <<= 3;                     // move to high bits
    temp &= 0xF0;                   // take only high 4 bits
    temp >>= 4;                     // move those high 4 bits low
    payload[2] |= temp;

    temp = UNIT;
    temp <<= 7;                     // move low bit of unit to high bit
    payload[3] = temp;

    // put command into next 5 bits of second byte
    temp = CMD;
    temp <<= 2;                     // move command two bits higher
    payload[3] |= temp;

    // TBD: make this random?
    payload[4] = 0x00;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_x10_simple //


/*!
    \brief Send a x10 stream msg

    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] MESSAGE_TYPE, (X10_START_STREAM/X10_STREAM/X10_STOP_STREAM)
    \param[in] HOUSE_1, x10 house 1
    \param[in] UNIT_CMD_1, x10 unit/cmd 1
    \param[in] HOUSE_2, x10 house 2
    \param[in] UNIT_CMD_2, x10 unit/cmd 2
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_x10_stream(const UInt8 SRC_UNIT, const UInt8 DST_UNIT,
  const ona_msg_type_t MESSAGE_TYPE, const UInt8 HOUSE_1,
  const UInt8 UNIT_CMD_1, const UInt8 HOUSE_2, const UInt8 UNIT_CMD_2,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    BOOL proceed = TRUE;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    UInt8 temp = 0x00;
    
    switch(MESSAGE_TYPE)
    {
        case ONA_X10_START_STREAM:                      // fall through
        case ONA_X10_STREAM:                            // fall through
        case ONA_X10_STOP_STREAM:                       // fall through
        {
            one_net_int16_to_byte_stream(MESSAGE_TYPE,
              &payload[ONA_MSG_HDR_IDX]);
            break;
        } // X10_START_STREAM/X10_STREAM/X10_STOP_STREAM //
    
        default:
        {
            proceed = FALSE;
            rv = ONS_BAD_PARAM;
            break;
        } // default //
    
    } // switch on MESSAGE_TYPE
    
    if(proceed)
    {
        // message data format for x10 stream:
        //     payload[2]        payload[3]         payload[4]
        // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
        // |HOUSE 1| UNIT/CMD1 |HOUSE 2| UNIT/CMD2 |  PADDING   | 
     
        // put house 1 into high 4 bits
        temp = HOUSE_1 & 0x0F;      // only 4 bits allowed
        temp <<= 4;                 // move to high nibble
        payload[2] = temp;

        // put unit/cmd 1 into low 4 bits & high 1 bit of next byte
        temp = UNIT_CMD_1 & 0x1F;   // only 5 bits allowed
        temp >>= 1;                 // lose low order bit 
        payload[2] |= temp;

        temp = UNIT_CMD_1 & 0x01;   // just get low bit
        temp <<= 7;                 // move it to high bit
        payload[3] |= temp;

        // put house 2 into position of next byte
        temp = HOUSE_2 & 0x0F;      // only 4 bits allowed
        temp <<= 3;                 // move into position
        payload[3] |= temp;

        // put unit/cmd 2 in position in its 2 bytes
        temp = UNIT_CMD_2 & 0x1C;   // mask off high 3 bits (of 5 bit field)
        temp >>= 2;                 // move to low order bits
        payload[3] |= temp;

        temp = UNIT_CMD_2 & 0x03;   // mask off low 2 bits
        temp <<= 6;                 // move to high 2 bits
        payload[4] = temp; 

        // send payload
        rv = (*send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
          RAW_DST, SRC_UNIT, DST_UNIT);
    } // if proceed

    return rv;
} // ona_send_x10_stream //


/*!
    \brief Send a x10 extended msg

    \param[in] SRC_UNIT, the source unit for switch msg
    \param[in] DST_UNIT, the destination unit for switch msg
    \param[in] HOUSE, x10 extended house
    \param[in] UNIT, x10 extended unit
    \param[in] DATA_BYTE, x10 extended data byte
    \param[in] CMD_BYTE, x10 extended cmd byte
    \param[in] RAW_DST, the destination device id

    \return the status of the send action
*/
one_net_status_t ona_send_x10_extended(const UInt8 SRC_UNIT,
  const UInt8 DST_UNIT, const UInt8 HOUSE, const UInt8 UNIT,
  const UInt8 DATA_BYTE, const UInt8 CMD_BYTE,
  const one_net_raw_did_t * const RAW_DST)
{
    one_net_status_t rv = ONS_SUCCESS;
    UInt8 payload[ONE_NET_RAW_SINGLE_DATA_LEN] = {0x00};
    UInt8 temp = 0x00;

    one_net_int16_to_byte_stream(ONA_X10_EXTENDED, &payload[ONA_MSG_HDR_IDX]);
 
    // message data format for x10 extended:
    //     payload[2]        payload[3]         payload[4]
    // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
    // | HOUSE |  UNIT  |    DATA_BYTE    |     CMD_BYTE    | 

    // put house into high 4 bits
    temp = HOUSE & 0x0F;            // only 4 bits allowed
    temp <<= 4;                     // move house to high nibble
    payload[2] = temp;

    // put unit into low 4 bits
    temp = UNIT & 0x0F;             // only 4 bits allowed
    payload[2] |= temp;

    // get data byte
    payload[3] = DATA_BYTE;

    // get cmd byte
    payload[4] = CMD_BYTE;

    // send payload
    rv = (*one_net_send_single)(payload, sizeof(payload), ONE_NET_LOW_PRIORITY,
      RAW_DST, SRC_UNIT, DST_UNIT);

    return rv;
} // ona_send_x10_extended //


/*!
    \brief parse an x10 simple msg into fields

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] house, house from msg
    \param[out] unit, unit from msg
    \param[out] cmd, command from msg

    \return the status of the send action
*/
one_net_status_t ona_parse_x10_simple(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * house, UInt8 * unit, UInt8 * cmd)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    }

    if(proceed)
    {
        // message data format for x10 simple:
        //    MSG_DATA[0]       MSG_DATA[1]       MSG_DATA[2]
        // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
        // | HOUSE |    UNIT   | X10_CMD |       PADDING        | 
        
        // get house
        UInt8 temp = MSG_DATA[0];
        temp &= 0xF0;
        temp >>= 4;
        *house = temp;

        // get unit
        temp = MSG_DATA[1] & 0x80;  // mask away all but high bit
        temp >>= 7;                 // put high bit in low bit
        *unit = temp;
        temp = MSG_DATA[0] & 0x0F;  // mask low nibble (high 4 of unit)
        temp <<= 1;                 // move them 1 higher
        *unit |= temp;              // add high 4 bits of unit

        // get x10_command
        temp = MSG_DATA[1] & 0x7C;  // mask only x10 cmd bits
        temp >>= 2;                 // shift it to low order 5 bits
        *cmd = temp;

    } // if proceed //
    
    return rv;
} // ona_parse_x10_simple //


/*!
    \brief parse an x10 stream msg into fields

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] house_1, house_1 from msg
    \param[out] unit_cmd_1, unit/cmd 1 from msg
    \param[out] house_2, house_2 from msg
    \param[out] unit_cmd_2, unit/cmd 2 from msg

    \return the status of the send action
*/
one_net_status_t ona_parse_x10_stream(const UInt8 * const MSG_DATA,
  const UInt8 LEN, UInt8 * house_1, UInt8 * unit_cmd_1,
  UInt8 * house_2, UInt8 * unit_cmd_2)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(proceed)
    {
        // message data format for x10 stream:
        //     MSG_DATA[0]       MSG_DATA[1]       MSG_DATA[2]
        // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
        // |HOUSE 1| UNIT/CMD1 |HOUSE 2| UNIT/CMD2 |  PADDING   | 
        
        // get house
        UInt8 temp = MSG_DATA[0];
        temp &= 0xF0;
        temp >>= 4;
        *house_1 = temp;

        // get unit/cmd 1
        temp = MSG_DATA[1] & 0x80;  // mask away all but high bit
        temp >>= 7;                 // put high bit in low bit
        *unit_cmd_1 = temp;
        temp = MSG_DATA[0] & 0x0F;  // mask low nibble (high 4 of unit)
        temp <<= 1;                 // move them 1 higher
        *unit_cmd_1 |= temp;        // add high 4 bits of unit

        // get house_2
        temp = MSG_DATA[1] & 0x78;  // mask only x10 cmd bits
        temp >>= 3;                 // shift it to low order 5 bits
        *house_2 = temp;

        // get unit/cmd 2
        temp = MSG_DATA[2] & 0xC0;  // get high 2 bits
        temp >>= 6;                 // move to low 2 bits
        *unit_cmd_2 = temp;
        temp = MSG_DATA[1] & 0x07;  // get low 3 bits
        temp <<= 2;                 // shift for 2 already in there
        *unit_cmd_2 |= temp;
    } // if proceed //
    
    return rv;
} // ona_parse_x10_stream //


/*!
    \brief parse an x10 extended msg into fields

    \param[in] MSG_DATA, messgae data of the received payload
    \param[in] LEN, the length of the msg data
    \param[out] house, from extended msg
    \param[out] unit, from extended msg
    \param[out] data_byte, data byte from msg
    \param[out] cmd_byte, cmd byte from msg

    \return the status of the send action
*/
one_net_status_t ona_parse_x10_extended(const UInt8 * const MSG_DATA, 
  const UInt8 LEN, UInt8 * house, UInt8 * unit, UInt8 * data_byte,
  UInt8 * cmd_byte)
{
    BOOL proceed = TRUE;
    one_net_status_t rv = ONS_SUCCESS;

    // error checking
    if(LEN != ONA_MSG_DATA_LEN)
    {
        proceed = FALSE;
        rv = ONS_BAD_PARAM;
    } // if any of the parameters are invalid //

    if(proceed)
    {
        // message data format for x10 extended:
        //     MSG_DATA[0]       MSG_DATA[1]       MSG_DATA[2]
        // |- - - - - - - - | - - - - - - - - | - - - - - - - - |
        // | HOUSE |  UNIT  |    DATA_BYTE    |     CMD_BYTE    | 
        
        // get house
        UInt8 temp = MSG_DATA[0] & 0xF0;
        temp >>= 4;
        *house = temp;

        // get unit
        temp = MSG_DATA[0] & 0x0F;
        *unit = temp;        

        // get data byte
        *data_byte = MSG_DATA[1];
        
        // get cmd byte
        *cmd_byte = MSG_DATA[2];

    } // if proceed //
    
    return rv;
} // ona_parse_x10_extended //

//! @} ONE-NET_APP_X10_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_APP_X10_pri_func
//! \ingroup ONE-NET_APP_X10
//! @{

//! @} ONE-NET_APP_X10_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_APP_X10

