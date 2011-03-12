#ifndef _ONE_NET_MASTER_PORT_CONST_H
#define _ONE_NET_MASTER_PORT_CONST_H

//! \defgroup on_master_port_const ONE-NET MASTER port specific constants
//! \ingroup ONE-NET_port_specific
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
    \file on_net_master_port_const.h
    \brief ONE-NET MASTER specific constants.

    These are constants that are specific to each ONE-NET MASTER device.  This
    file should be copied to a project specific location and renamed to
    master_port_const.h.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include "tick.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup on_master_port_const_const
//! \ingroup on_master_port_const
//! @{

enum
{
    //! The number of CLIENTS the MASTER keeps track of.
    ONE_NET_MASTER_MAX_CLIENTS = 5,

    //! Frequency to send new CLIENT Invite
    ONE_NET_MASTER_INVITE_SEND_TIME = MS_TO_TICK(250),

    //! Timeout period (in ticks)to wait for keep alive in response to a
    //! CHANGE KEY before moving on to try the next device.
    ONE_NET_MASTER_CHANGE_KEY_TIMEOUT = MS_TO_TICK(10000),
    
    //! The number of ticks to poll a given channel to see if it is clear when a
    //! network is created for the first time.
    ONE_NET_MASTER_NETWORK_CHANNEL_CLR_TIME = MS_TO_TICK(5000),
    
    //! The number of ticks to wait before deciding all channels viewed so far
    //! are busy and to lower the channel clear time to try and find the least
    //! busy channel.
    ONE_NET_MASTER_CHANNEL_SCAN_TIME = MS_TO_TICK(10000)
};

//! The default keep alive interval in ticks to assign to new clients.  This is
//! not an enum since a 16-bit value may not be big enough.
#define ONE_NET_MASTER_DEFAULT_KEEP_ALIVE MS_TO_TICK(1800000)

//! Duration the MASTER sends the new CLIENT invite.  This is not an enum since
//! a 16-bit value is not big enough.
#define ONE_NET_MASTER_INVITE_DURATION MS_TO_TICK(600000)


enum
{
    //! The number of transactions the MASTER can queue to send
    ONE_NET_MASTER_MAX_SEND_TXN = 6,

    //! The minimum number of transactions that must be kept for single
    //! transactions
    ONE_NET_MASTER_MIN_SINGLE_TXN = 2,

    //! The maximum number of transactions that are kept track of.  Note that
    //! block/stream receive transactions do not need packet data
    //! associated with them.
    ONE_NET_MASTER_MAX_TXN = 10,

    //! The maximum number (+1) of transactions that can be queued and
    //! considered a low load
    ONE_NET_MASTER_LOW_LOAD_LIMIT = 3 + 1
};

//! @} on_master_port_const_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup on_master_port_const_typedefs
//! \ingroup on_master_port_const
//! @{

//! @} on_master_port_const_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PUBLIC VARIABLES
//! \defgroup on_master_port_const_pub_var
//! \ingroup on_master_port_const
//! @{

//! @} on_master_port_const_pub_var
//                              PUBLIC VARIABLES END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup on_master_port_const_pub_func
//! \ingroup on_master_port_const
//! @{

//! @} on_master_port_const_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

//! @} on_master_port_const

#endif // _ONE_NET_MASTER_PORT_CONST_H //

