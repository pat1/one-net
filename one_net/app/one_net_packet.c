//! \addtogroup ONE-NET_PACKET
//! @{

/*
    Copyright (c) 2011, Threshold Corporation
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
    \file one_net_packet.c
    \brief Implementation of packet length / type functions.

    Implementation of packet length / type functions.
*/

#include "one_net_packet.h"
#include "one_net_encode.h"
#include "config_options.h"

#ifdef _PID_BLOCK
#include "one_net.h"
#endif




//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PACKET_const
//! \ingroup ONE-NET_PACKET
//! @{



//! @} ONE-NET_PACKET_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_PACKET_typedefs
//! \ingroup ONE-NET_PACKET
//! @{

//! @} ONE-NET_PACKET_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_PACKET_pri_func
//! \ingroup ONE-NET_PACKET
//! @{

//! @} ONE-NET_PACKET_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_PACKET_pub_func
//! \ingroup ONE-NET_PACKET
//! @{


/*!
    \brief Returns the encoded payload len for a PID

    \param[in] raw_pid raw_pid of the packet

    \return the length of the encoded payload if the pid is valid,
           -1 if the pid is not valid.
*/
SInt8 get_encoded_payload_len(UInt16 raw_pid)
{
    SInt8 num_payload_blocks = get_num_payload_blocks(raw_pid);   
    switch(num_payload_blocks)
    {
        case 1: case 2: case 3: return 11 * num_payload_blocks;
        case 4: return 43;
        default: return num_payload_blocks;
    }
}


/*!
    \brief Returns the raw payload len for a PID

    \param[in] raw_pid raw pid of the packet

    \return the length of the raw payload if the pid is valid, -1 if the pid
           is not valid.
*/
SInt8 get_raw_payload_len(UInt16 raw_pid)
{
    // includes 1 byte for the encryption method
    SInt8 num_payload_blocks = get_num_payload_blocks(raw_pid);   
    switch(num_payload_blocks)
    {
        case 1: case 2: case 3: case 4:
            return ONE_NET_XTEA_BLOCK_SIZE * num_payload_blocks + 1;
        default: return num_payload_blocks;
    }
}


/*!
    \brief Returns the number of XTEA blocks in the payload for a PID

    \param[in] raw_pid raw_pid of the packet

    \return the number of XTEA blocks in the payload, -1 if the pid is
           not valid.
*/
SInt8 get_num_payload_blocks(UInt16 raw_pid)
{
    SInt8 num_blocks = (SInt8) ((raw_pid >> 8) & 0x0F);
    if((raw_pid & 0x3F) > ONE_NET_RAW_CLIENT_REQUEST_INVITE)
    {
        return -1;
    }
    
    if(num_blocks >= 1 && num_blocks <= 4)
    {
        return num_blocks;
    }
    
    return -1;
} // get_num_payload_blocks //


/*!
    \brief Returns the packet length for a PID

    \param[in] raw_pid raw pid of the packet
    \param[in] include_header True if the packet lengh should include the
               preamble / header bytes, false otherwise

    \return the length of the packet if the pid is valid, 0 if the pid is
           not valid.
*/
UInt8 get_encoded_packet_len(UInt16 raw_pid, BOOL include_header)
{
    SInt8 pld_len = get_encoded_payload_len(raw_pid);
    
    #ifdef ONE_NET_MULTI_HOP
    UInt8 mh_bytes = packet_is_multihop(raw_pid) ? ON_ENCODED_HOPS_SIZE : 0;
    #endif
    UInt8 header_offset = include_header ? 0 : ON_ENCODED_RPTR_DID_IDX;
    
    if(pld_len < 0)
    {
        // invalid PID
        return 0;
    }

    #ifdef ONE_NET_MULTI_HOP
    return ON_PLD_IDX + pld_len + mh_bytes - header_offset;
    #else
    return ON_PLD_IDX + pld_len - header_offset;
    #endif
} // get_encoded_packet_len //


/*!
    \brief Converts a response PID into its ACK or NACK
      equivalent.

    Converts a response PID into its ACK or NACK
      equivalent.  If an ACK PID is given and is_ack is true,
      no change is made.  Similarly if a NACK PID is given and
      is_ack is false, no change is made.

    \param[in/out] raw_pid The pid to (possibly) be converted.
    \param[in] is_ack True if the DESIRED OUTGOING PID should be
      an ACK, False otherwise.
      
    \return True if the pid changed, false otherwise
*/
BOOL set_ack_or_nack_pid(UInt16* raw_pid, BOOL is_ack)
{
    BOOL pid_is_nack = packet_is_nack(*raw_pid);
    BOOL pid_is_ack  = packet_is_ack(*raw_pid);
    if(!pid_is_ack && !pid_is_nack)
    {
        return FALSE; // raw pid is not an ACK or a NACK.
    }
    
    if(pid_is_ack == is_ack)
    {
        return FALSE; // No conversion required.
    }
    
    (*raw_pid)++;
    if(is_ack)
    {
        (*raw_pid) -= 2;
    }

    
    return FALSE; // not an ACK or a NACK. encoded_pid unchanged
}


#ifdef ONE_NET_MULTI_HOP
/*!
    \brief Converts a PID into its non-multi-hop or multi-hop equivalent.

    Converts a PID into its non-multi-hop or multi-hop equivalent.  If a multi-
      hop PID is given and is_multihop is true, no change is made.  Similarly
      if a non-multi-hop PID is given and is_multihop is false, no change is
      made.

    \param[in/out] raw_pid The pid to (possibly) be converted.
    \param[in] is_multihop True if the DESIRED OUTGOING PID should be multi-hop,
               False otherwise.
    \return True if the pid changed, false otherwise
*/
BOOL set_multihop_pid(UInt16* raw_pid, BOOL is_multihop)
{
    UInt16 orig_raw_pid = *raw_pid;
    
    (*raw_pid) &= ~ONE_NET_RAW_PID_MH_MASK;
    if(is_multihop)
    {
        (*raw_pid) |= ONE_NET_RAW_PID_MH_MASK;
    }
    
    return (orig_raw_pid != *raw_pid);
}
#endif


/*!
    \brief Determines whether a given PID represents a data packet.

    Determines whether a given PID represents a data packet.

    \param[in] raw_pid The pid to check

    \return True if pid is a data packet, false otherwise.
*/
BOOL packet_is_data(UInt16 raw_pid)
{
    raw_pid &= 0x3F;

    switch(raw_pid)
    {
        case ONE_NET_RAW_SINGLE_DATA:
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_RAW_BLOCK_DATA:
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_RAW_STREAM_DATA:
        #endif
        #ifdef ROUTE
        case ONE_NET_RAW_ROUTE:
        #endif
            return TRUE;
        default:
            return FALSE;
    }
}


/*!
    \brief Determines whether a given PID represents a NACK packet.

    Determines whether a given PID represents a NACK packet.

    \param[in] raw_pid The pid to check

    \return True if pid is a NACK packet, false otherwise.
*/
BOOL packet_is_nack(UInt16 raw_pid)
{
    return packet_is_ack(raw_pid - 1);
}


/*!
    \brief Determines whether a given PID represents an ACK packet.

    Determines whether a given PID represents an ACK packet.

    \param[in] raw_pid The pid to check

    \return True if pid is an ACK packet, false otherwise.
*/
BOOL packet_is_ack(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    
    switch(raw_pid)
    {
        case ONE_NET_RAW_SINGLE_DATA_ACK:
        #ifdef ROUTE
        case ONE_NET_RAW_ROUTE_ACK:
        #endif
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_RAW_BLOCK_DATA_ACK:
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_RAW_STREAM_DATA_ACK:
        #endif
            return TRUE;
        default:
            return FALSE;
    }
}


/*!
    \brief Determines the appropriate acknowledgement PID for responding to a
      single data packet with certain criteria.

    For a given PID and given criteria, the response PID that should be
    given.
    

    \param[in] raw_single_pid The pid we are responding to or an ACK or a NACK
    \param[in] isACK If true, we should ACK.  If false, we should NACK.
    \param[in] stay_awake True if we want the other device to stay awake,
               false otherwise.

    \return the PID we should use to respond.  -1 if invalid
*/
SInt16 get_single_response_pid(UInt16 raw_single_pid, BOOL isACK,
  BOOL stay_awake)
{
    UInt16 raw_resp_pid;

    switch(raw_single_pid & 0x3F)
    {
        case ONE_NET_RAW_SINGLE_DATA:
          raw_resp_pid = ONE_NET_RAW_SINGLE_DATA_ACK; break;
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_RAW_BLOCK_DATA:
          raw_resp_pid = ONE_NET_RAW_BLOCK_DATA_ACK; break;
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_RAW_STREAM_DATA:
          raw_resp_pid = ONE_NET_RAW_STREAM_DATA_ACK; break;
        #endif
        #ifdef ROUTE
        case ONE_NET_RAW_ROUTE:
          raw_resp_pid = ONE_NET_RAW_ROUTE_ACK; break;
        #endif

        default:
          if(packet_is_ack(raw_single_pid))
          {
              raw_resp_pid = raw_single_pid & 0x3F;
          }
          else if(packet_is_nack(raw_single_pid))
          {
              raw_resp_pid = (raw_single_pid & 0x3F) - 1; // make it an ACK
          }
          else
          {
              return -1; // invalid
          }
    }
    
    // adjust it if it is not an ACK.
    if(!isACK)
    {
        raw_resp_pid++; // change from ACK to NACK.
    }
    
    // set the multi-hop pid
    raw_resp_pid |= (raw_single_pid & ONE_NET_RAW_PID_MH_MASK);
    
    // set the length
    raw_resp_pid |= (raw_single_pid & 0xF00);
    
    // set the stay-awake from the parameter.
    if(stay_awake)
    {
        raw_resp_pid |= ONE_NET_RAW_PID_STAY_AWAKE_MASK;
    }
    
    return raw_resp_pid;
}


BOOL packet_length_is_valid(UInt16 raw_pid)
{
    UInt8 num_blocks = (UInt8)((raw_pid & 0xF00) >> 8);
    UInt8 base_pid = (UInt8)(raw_pid & 0x3F);
    if(num_blocks == 0 || num_blocks > 3) // TODO --  make this 4
    {
        return FALSE;
    }
    if(base_pid > ONE_NET_RAW_CLIENT_REQUEST_INVITE)
    {
        return FALSE;
    }
    
    #ifndef ROUTE
    if(packet_is_invite(raw_pid))
    #else
    if(packet_is_invite(raw_pid) || packet_is_route(raw_pid))
    #endif
    {
        return (num_blocks == 3);
    }
    
    #ifndef EXTENDED_SINGLE
    return (num_blocks == 1);
    #endif
    
    return TRUE;
}


// return -1 if invalid
SInt8 get_default_num_blocks(UInt16 raw_pid)
{
    raw_pid &= 0x3F;
    if(raw_pid > ONE_NET_RAW_CLIENT_REQUEST_INVITE)
    {
        return -1;
    }
    
    #ifndef ROUTE
    if(packet_is_invite(raw_pid))
    #else
    if(packet_is_invite(raw_pid) || packet_is_route(raw_pid))
    #endif
    {
        return 3;
    }
    
    switch(raw_pid)
    {
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_RAW_BLOCK_DATA:
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_RAW_STREAM_DATA:
        #endif
            return 4;
        default:
            return 1;
    }
}



//! @} ONE-NET_PACKET_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_PACKET_pri_func
//! \ingroup ONE-NET_PACKET
//! @{

//! @} ONE-NET_PACKET_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_PACKET

