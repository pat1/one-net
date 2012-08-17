#ifndef _ONE_NET_PORT_CONST_H
#define _ONE_NET_PORT_CONST_H

//! \defgroup ONE-NET_port_const Application Specific ONE-NET constants.
//! \ingroup ONE-NET
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
    \file one_net_port_const.h
    \brief Application specific ONE-NET constants.

    These are constants that are specific to each ONE-NET device.  This file
    should be copied to a project specific location and renamed to
    one_net_port_const.h.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_const_const
//! \ingroup ONE-NET_port_const
//! @{
    
    

#if defined(_QUAD_OUTPUT) || defined(_QUAD_INPUT)
    //  This is a #define so it can be used by the preprocessor
    #define _NUM_IO_UNITS 4             //!< The number of io units.
#elif defined(_DUAL_OUTPUT) // if _QUAD_OUTPUT or _QUAD_INPUT is defined //
    //  This is a #define so it can be used by the preprocessor
    #define _NUM_IO_UNITS 2             //!< The number of io units.
#else // none of _QUAD_OUTPUT, _QUAD_INPUT or _DUAL_OUTPUT are defined //
    #error Need to define _QUAD_OUTPUT, _QUAD_INPUT, or _DUAL_OUTPUT
#endif // else _QUAD_OUTPUT, _QUAD_INPUT and _DUAL_OUTPUT are not defined //


enum
{
    //! The number of different unit types this device supports.  If this value
    //! changes, UNIT_TYPES will also need to be changed.
    ONE_NET_NUM_UNIT_TYPES = 1,

    //! Number of units on this device.  This needs to be the sum of the values
    //! in ONE_NET_DEVICE_UNIT_TYPE
    ONE_NET_NUM_UNITS = _NUM_IO_UNITS
};


enum
{
    SINGLE_DATA_QUEUE_SIZE = 2,
    SINGLE_DATA_QUEUE_PAYLOAD_BUFFER_SIZE = 10
};


//! Timer related constants
enum
{
    //! Time in ticks to spend polling for reception of a packet (the PREAMBLE
    //! & SOF). 10ms
    ONE_NET_WAIT_FOR_SOF_TIME = 10,

    //! Time in ticks a device must wait in between checking if a channel is
    //! clear (5ms)
    ONE_NET_CLR_CHANNEL_TIME = 5,

    //! Time in ticks a device waits for a response (50ms)
    ONE_NET_RESPONSE_TIME_OUT = 50,

    // dje: Added November 10, 2010
    //! Time in ticks a device waits for a transaction to end (ACK or new transaction) (100ms)
    ONE_NET_TRN_END_TIME_OUT = 100,

    //! The base time in ticks for the retransmit delay for low priority
    //! transactions (10ms)
    ONE_NET_RETRANSMIT_LOW_PRIORITY_TIME = 10,

    //! The base time in ticks for the retransmit delay for high prioritty
    //! transactions (2ms)
    ONE_NET_RETRANSMIT_HIGH_PRIORITY_TIME = 2,

    //! Base Fragment delay in ticks for low priority transactions (125ms)
    ONE_NET_FRAGMENT_DELAY_LOW_PRIORITY = 125,

    //! Base Fragment delay in ticks for high priority transactions (25ms)
    ONE_NET_FRAGMENT_DELAY_HIGH_PRIORITY = 25,
};



//! Time in ticks to stay awake after receiving a Single Data ACK Stay
//! Awake Packet (2s)
#define ONE_NET_STAY_AWAKE_TIME 2000


enum
{
    //! Maximum number of recipient for any one message.
    ONE_NET_MAX_RECIPIENTS = 4
};


#ifdef _PEER
enum
{
    //! Size of the peer table
    ONE_NET_MAX_PEER_UNIT = 4,

    // subtract one for the actual recipient and another for the master
    // in case we need to send to it and it isn't already on the list
    ONE_NET_MAX_PEER_PER_TXN = ONE_NET_MAX_RECIPIENTS - 2
};
#endif


//! The time before timeout of the invite process from beginning of the time
//! that the invite starts to be accepted until the time all information has
//! been passed between the master and the new client.  In ms.
#define INVITE_TRANSACTION_TIMEOUT 10000



//! @} ONE-NET_port_const_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_port_const_typedefs
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_port_const_pub_var
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_port_const_pub_func
//! \ingroup ONE-NET_port_const
//! @{

//! @} ONE-NET_port_const_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} ONE-NET_port_specific

#endif // _ONE_NET_PORT_CONST_H //
