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





//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_PACKET_const
//! \ingroup ONE-NET_PACKET
//! @{


//! The number of pairs of stay-awake / non-stay-awake response PIDs.
static enum
{
    #ifndef _EXTENDED_SINGLE
    NUM_STAY_AWAKE_PAIRS = 2
    #elif defined(_ONE_NET_MULTI_HOP)
    NUM_STAY_AWAKE_PAIRS = 12
    #else
    NUM_STAY_AWAKE_PAIRS = 6
    #endif
};


//! The PID pairs of stay-awake / non-stay-awake response PIDs.
static const UInt8 stay_awake_packet_pairs[NUM_STAY_AWAKE_PAIRS][2] =
{
    {ONE_NET_ENCODED_SINGLE_DATA_ACK, ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_SINGLE_DATA_NACK_STAY_AWAKE},
    
    #ifdef _EXTENDED_SINGLE
    {ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK, ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_STAY_AWAKE},
    {ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK, ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE},
    #ifdef _ONE_NET_MULTI_HOP
    {ONE_NET_ENCODED_MH_SINGLE_DATA_ACK, ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_STAY_AWAKE},
    {ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_ACK, ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_NACK_STAY_AWAKE},
    {ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_ACK, ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_ACK_STAY_AWAKE},
    {ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_NACK_RSN, ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_NACK_STAY_AWAKE},
    #endif
    #endif
};



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

    \param[in] pid pid of the packet

    \return the length of the encoded payload if the pid is valid,
           -1 if the pid is not valid.
*/
SInt8 get_encoded_payload_len(UInt8 pid)
{
    SInt8 num_payload_blocks = get_num_payload_blocks(pid);   
    switch(num_payload_blocks)
    {
        case 1: case 2: case 3: return 11 * num_payload_blocks;
        case 4: return 43;
        default: return num_payload_blocks;
    }
}


/*!
    \brief Returns the raw payload len for a PID

    \param[in] pid pid of the packet

    \return the length of the raw payload if the pid is valid, -1 if the pid
           is not valid.
*/
SInt8 get_raw_payload_len(UInt8 pid)
{
    // includes 1 byte for the encryption method
    SInt8 num_payload_blocks = get_num_payload_blocks(pid);   
    switch(num_payload_blocks)
    {
        case 1: case 2: case 3: case 4:
            return ONE_NET_XTEA_BLOCK_SIZE * num_payload_blocks + 1;
        default: return num_payload_blocks;
    }
}


/*!
    \brief Returns the number of XTEA blocks in the payload for a PID

    \param[in] pid pid of the packet

    \return the number of XTEA blocks in the payload, -1 if the pid is
           not valid.
*/
SInt8 get_num_payload_blocks(UInt8 pid)
{
    switch(pid)
    {
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_TXN_ACK:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
        #endif
            return 0;
        #endif
        

        case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
        #endif
            return 3;


        case ONE_NET_ENCODED_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
        #ifdef ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN:
        #endif
            return 1;
            
                    
        #ifdef _EXTENDED_SINGLE
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_RSN:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA_NACK_RSN:
        #endif
            return 2;        
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_RSN:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA_NACK_RSN:
        #endif
            return 3;
        #endif        
        

        case ONE_NET_ENCODED_SINGLE_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA:
        #endif
            return 1;
      

        #ifdef _EXTENDED_SINGLE
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA:
        #endif
            return 2;
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA:
        #endif
            return 3;
        #endif
        

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_DATA:
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_ENCODED_STREAM_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_STREAM_DATA:
        #endif
        #endif
            return 4;
        #endif

        default:
            return -1;
    }
} // get_num_payload_blocks //


/*!
    \brief Returns the packet length for a PID

    \param[in] pid pid of the packet
    \param[in] include_header True if the packet lengh should include the
               preamble / header bytes, false otherwise

    \return the length of the packet if the pid is valid, 0 if the pid is
           not valid.
*/
UInt8 get_encoded_packet_len(UInt8 pid, BOOL include_header)
{
    UInt8 pld_len = get_encoded_payload_len(pid);
    
    #ifdef _ONE_NET_MULTI_HOP
    UInt8 mh_bytes = packet_is_multihop(pid) ? ON_ENCODED_HOPS_SIZE : 0;
    #endif
    UInt8 header_offset = include_header ? 0 : ONE_NET_ENCODED_RPTR_DID_IDX;
    
    if(pld_len == 0)
    {
        // invalid PID
        return 0;
    }

    #ifdef _ONE_NET_MULTI_HOP
    return ON_PLD_IDX + pld_len + mh_bytes - header_offset;
    #else
    return ON_PLD_IDX + pld_len - header_offset;
    #endif
} // get_encoded_packet_len //


/*!
    \brief Determines whether a given PID represents a stay-awake packet.

    Determines whether a given PID represents a stay-awake packet.

    \param[in] encoded_pid The pid to check

    \return True if encoded_pid is a stay-awake packet, false otherwise.
*/
BOOL packet_is_stay_awake(UInt8 encoded_pid)
{
    UInt8 i;
    for(i = 0; i < NUM_STAY_AWAKE_PAIRS; i++)
    {
        if(encoded_pid == stay_awake_packet_pairs[i][1])
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


/*!
    \brief Converts a response PID into its non-stay-awake or stay-awake
      equivalent.

    Converts a response PID into its non-stay-awake or stay-awake
      equivalent.  If a stay-awake PID is given and stay_awake is true,
      no change is made.  Similarly if a non-stay-awake PID is given and
      stay_awake is false, no change is made.

    \param[in/out] encoded_pid The pid to (possibly) be converted.
    \param[in] stay_awake True if the DESIRED OUTGOING PID should be
      a stay_awake PID, False otherwise.
      
    \return True if the pid changed, false otherwise
*/
BOOL set_stay_awake_pid(UInt8* encoded_pid, BOOL stay_awake)
{
    UInt8 i;
    BOOL pid_is_stay_awake = packet_is_stay_awake(*encoded_pid);
    
    if(pid_is_stay_awake == stay_awake)
    {
        return FALSE; // no change
    }
    
    for(i = 0; i < NUM_STAY_AWAKE_PAIRS; i++)
    {
        if(*encoded_pid == stay_awake_packet_pairs[i][pid_is_stay_awake])
        {
            *encoded_pid = stay_awake_packet_pairs[i][!pid_is_stay_awake];
            return TRUE;
        }
    }
    
    return FALSE; // not an ACK or NACK
}


/*!
    \brief Converts a response PID into its ACK or NACK
      equivalent.

    Converts a response PID into its AK or NACK
      equivalent.  If an ACK PID is given and is_ack is true,
      no change is made.  Similarly if a NACK PID is given and
      is_ack is false, no change is made.

    \param[in/out] encoded_pid The pid to (possibly) be converted.
    \param[in] is_ack True if the DESIRED OUTGOING PID should be
      an ACK, False otherwise.
      
    \return True if the pid changed, false otherwise
*/
BOOL set_ack_or_nack_pid(UInt8* encoded_pid, BOOL is_ack)
{
    UInt8 i, j;
    BOOL pid_is_nack = packet_is_nack(*encoded_pid);
    if(pid_is_nack == !is_ack)
    {
        return FALSE;
    }
    
    for(i = 0; i < 2; i++)
    {
        // NACKs are odd, ACKs are even
        for(j = pid_is_nack; j < NUM_STAY_AWAKE_PAIRS; j += 2)
        {
            if(*encoded_pid == stay_awake_packet_pairs[j][i])
            {
                // found it.  We subtract 1 if we're going from a NACK
                // to an ACK.  Add 1 if going from an ACK to a NACK.
                *encoded_pid = pid_is_nack ? stay_awake_packet_pairs[j-1][i] :
                  stay_awake_packet_pairs[j+1][i];
                return TRUE;
            }
        }
    }
    
    return FALSE; // invalid PID
}


#ifdef _ONE_NET_MULTI_HOP
/*!
    \brief Determines whether a given PID represents a multi-hop packet.

    Determines whether a given PID represents a multi-hop packet.

    \param[in] encoded_pid The pid to check

    \return True if encoded_pid is a multi-hop packet, false otherwise.
*/
BOOL packet_is_multihop(UInt8 encoded_pid)
{
    UInt8 raw_pid;
    if(on_decode(&raw_pid, &encoded_pid, 1) != ONS_SUCCESS)
    {
        return FALSE; // invalid encoded pid
    }
    
    // if MSB is 1, then it's multi-hop
    if(raw_pid & 0x80)
    {
        return TRUE;
    }
    
    return FALSE;
}


/*!
    \brief Converts a PID into its non-multi-hop or multi-hop equivalent.

    Converts a PID into its non-multi-hop or multi-hop equivalent.  If a multi-
      hop PID is given and is_multihop is true, no change is made.  Similarly
      if a non-multi-hop PID is given and is_multihop is false, no change is
      made.

    \param[in/out] encoded_pid The pid to (possibly) be converted.
    \param[in] is_multihop True if the DESIRED OUTGOING PID should be multi-hop,
               False otherwise.
    \return True if the pid changed, false otherwise
*/
BOOL set_multihop_pid(UInt8* encoded_pid, BOOL is_multihop)
{
    UInt8 raw_pid;
    
    // note that we shift by 2 since the raw bits are shifted 2.
    const MULTI_HOP_OFFSET = ONE_NET_RAW_PID_MULTI_HOP_OFFSET << 2;
    if(on_decode(&raw_pid, encoded_pid, 1) != ONS_SUCCESS)
    {
        return FALSE; // invalid incoming encoded PID
    }
    
    if(is_multihop)
    {
        if(raw_pid >= MULTI_HOP_OFFSET)
        {
            return FALSE; // want multi-hop, already have multi-hop.
        }
        
        raw_pid += MULTI_HOP_OFFSET;
    }
    else
    {
        if(raw_pid < MULTI_HOP_OFFSET)
        {
            return FALSE; // want non-multi-hop, already have non-multi-hop.
        }
        
        raw_pid -= MULTI_HOP_OFFSET;
    }
    
    on_encode(encoded_pid, &raw_pid, 1);
    return TRUE; // pid has changed.
}
#endif


#ifdef _STREAM_MESSAGES_ENABLED
/*!
    \brief Determines whether a given PID represents a stream packet.

    Determines whether a given PID represents a stream packet.

    \param[in] pid The pid to check

    \return True if pid is a stream packet, false otherwise.
*/
BOOL packet_is_stream(UInt8 pid)
{
    switch(pid)
    {
        case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
        case ONE_NET_ENCODED_STREAM_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
        case ONE_NET_ENCODED_MH_STREAM_DATA:
        #endif
            return TRUE;

        default:
            return FALSE;
    }    
}
#endif


/*!
    \brief Determines whether a given PID represents an invite packet.

    Determines whether a given PID represents an invite packet.

    \param[in] pid The pid to check

    \return True if pid is an invite packet, false otherwise.
*/
BOOL packet_is_invite(UInt8 pid)
{
    switch(pid)
    {
        case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
        #endif
            return TRUE;

        default:
            return FALSE;
    }    
}


/*!
    \brief Determines the "family" type of a packet

    Determines the "family" type of a packet(i.e. single packet, block
      backet, stream packet, etc.

    \param[in] pid The pid to check

    \return the "family" type of the packet.
*/
pkt_group_t get_pkt_family(UInt8 pid)
{
    switch(pid)
    {
        case ONE_NET_ENCODED_MASTER_INVITE_NEW_CLIENT:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_MASTER_INVITE_NEW_CLIENT:
        #endif
            return INVITE_PKT_GRP;
            
            
        case ONE_NET_ENCODED_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
        case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN:
        case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
        #endif
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_DATA_ACK:
        case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:
        case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN:
        #endif
        #endif
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_ENCODED_STREAM_KEEP_ALIVE:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_STREAM_KEEP_ALIVE:
        #endif
        #endif
            return ACK_NACK_PKT_GRP;
      
      
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_TXN_ACK:
        #ifdef _ONE_NET_MULTI_HOP      
        case ONE_NET_ENCODED_MH_BLOCK_TXN_ACK:
        #endif
        #endif
            return TXN_ACK_PKT_GRP;
            
       
        case ONE_NET_ENCODED_SINGLE_DATA:
        #ifdef _EXTENDED_SINGLE
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA:
        #endif
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA:
        #ifdef _EXTENDED_SINGLE
        case ONE_NET_ENCODED_MH_LARGE_SINGLE_DATA:
        case ONE_NET_ENCODED_MH_EXTENDED_SINGLE_DATA:
        #endif
        #endif
            return SINGLE_PKT_GRP;
                        
            
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_DATA:
        #endif  
            return BLOCK_PKT_GRP;
        #endif
        
        
        #ifdef _STREAM_MESSAGES_ENABLED
        case ONE_NET_ENCODED_STREAM_DATA:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_STREAM_DATA:
        #endif
            return STREAM_PKT_GRP;
        #endif
            
            
        default:
            return NUM_PKT_GROUPS; // invalid PID 
    }
}


/*!
    \brief Determines whether a given PID represents a NACK packet.

    Determines whether a given PID represents a NACK packet.

    \param[in] pid The pid to check

    \return True if pid is a NACK packet, false otherwise.
*/
BOOL packet_is_nack(UInt8 pid)
{
    switch(pid)
    {
        case ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA_NACK_RSN:
        #endif

        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_DATA_NACK_RSN:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_DATA_NACK_RSN:
        #endif
        #endif
            return TRUE;
        default:
            return FALSE;
    }
}


/*!
    \brief Determines whether a given PID represents an AACK packet.

    Determines whether a given PID represents an ACK packet.

    \param[in] pid The pid to check

    \return True if pid is an ACK packet, false otherwise.
*/
BOOL packet_is_ack(UInt8 pid)
{
    switch(pid)
    {
        case ONE_NET_ENCODED_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK:
        case ONE_NET_ENCODED_MH_SINGLE_DATA_ACK_STAY_AWAKE:
        #endif
        
        #ifdef _BLOCK_MESSAGES_ENABLED
        case ONE_NET_ENCODED_BLOCK_DATA_ACK:
        #ifdef _ONE_NET_MULTI_HOP
        case ONE_NET_ENCODED_MH_BLOCK_DATA_ACK:
        #endif
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
    given.  For example, a pid of ONE_NET_ENCODED_SINGLE_DATA that should
    ACK and tell the original device to stay awake would return
    ONE_NET_ENCODED_SINGLE_DATA_ACK_STAY_AWAKE.
    

    \param[in] single_pid The pid we are responding to
    \param[in] isACK If true, we should ACK.  If false, we should NACK.
    \param[in] stay_awake True if we want the other device to stay awake,
               false otherwise.

    \return the PID we should use to respond.
*/
UInt8 get_single_response_pid(UInt8 single_pid, BOOL isACK, BOOL stay_awake)
{
    UInt8 resp_pid;
    #ifdef _ONE_NET_MULTI_HOP
    BOOL pid_is_multi = packet_is_multihop(single_pid);
    if(!set_multihop_pid(&single_pid, FALSE))
    {
        return 0; // invalid PID
    }
    #endif
    
    switch(single_pid)
    {
        case ONE_NET_ENCODED_SINGLE_DATA:
          resp_pid = isACK ? ONE_NET_ENCODED_SINGLE_DATA_ACK :
            ONE_NET_ENCODED_SINGLE_DATA_NACK_RSN; break;
        #ifdef _EXTENDED_SINGLE
        case ONE_NET_ENCODED_LARGE_SINGLE_DATA:
          resp_pid = isACK ? ONE_NET_ENCODED_LARGE_SINGLE_DATA_ACK :
            ONE_NET_ENCODED_LARGE_SINGLE_DATA_NACK_RSN; break;             
        case ONE_NET_ENCODED_EXTENDED_SINGLE_DATA:
          resp_pid = isACK ? ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_ACK :
            ONE_NET_ENCODED_EXTENDED_SINGLE_DATA_NACK_RSN; break; 
        #endif
        default:
          return 0; // bad pid
    }
    
    #ifdef _ONE_NET_MULTI_HOP
    // turn it back into multi-hop if it was before
    set_multihop_pid(&resp_pid, pid_is_multi);
    #endif
    
    // now set stay-awake
    set_stay_awake_pid(&resp_pid, stay_awake);

    return resp_pid;
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

