#ifndef _ONE_NET_MASTER_PORT_CONST_H
#define _ONE_NET_MASTER_PORT_CONST_H



//! \defgroup on_master_port_const ONE-NET MASTER port specific constants
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
    \file one_net_master_port_const.h
    \brief ONE-NET MASTER specific constants.

    These are constants that are specific to each ONE-NET MASTER device.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


//==============================================================================
//                                  CONSTANTS
//! \defgroup on_master_port_const_const
//! \ingroup on_master_port_const
//! @{



enum
{
    //! The number of CLIENTS the MASTER keeps track of.
    ONE_NET_MASTER_MAX_CLIENTS = 5
};


// timers
//! Frequency in ms to send new CLIENT Invite
#define ONE_NET_MASTER_INVITE_SEND_TIME 250

//! The number of ms to poll a given channel to see if it is clear when a
//! network is created for the first time.
#define ONE_NET_MASTER_NETWORK_CHANNEL_CLR_TIME 5000

//! The number of ticks to wait before deciding all channels viewed so far
//! are busy and to lower the channel clear time to try and find the least
//! busy channel.
#define ONE_NET_MASTER_CHANNEL_SCAN_TIME 10000

//! The default keep alive interval in ms to assign to new clients.  30 minutes
//! If clients are not expected to check in regularly, change this value to 0.
#define ONE_NET_MASTER_DEFAULT_KEEP_ALIVE 1800000

//! Duration the MASTER sends the new CLIENT invite, in ms.  10 minutes
#define ONE_NET_MASTER_INVITE_DURATION 600000

//! Default of whether the master wants the client to inform it when
//! status is changed.  Should be TRUE or FALSE.  An example of this might
//! be a motion sensor that is expected to inform the master whenever triggered.
//! Note that one can also choose to set this value as FALSE and still inform
//! the master when this happens via the application code.  Setting this value
//! to TRUE will inform the master to tell all clients to send it updates of any
//! status message.  Note that the master can choose to have some clients have
//! this flag set, but not others and can change it at any time.  This value
//! simply sets the INITIAL flag.
#define ONE_NET_MASTER_SEND_TO_MASTER TRUE

//! Default of whether the master wants the client to reject invalid message
//! ids.  Define this option TRUE only if you are worried about replay attacks.
//! If this does not apply, define as FALSE.  Note that the master can choose
//! to have some clients have this flag set, but not others and can change it at
//! any time.  This value simply sets the INITIAL flag.
#define ONE_NET_MASTER_REJECT_INVALID_MSG_ID TRUE


//! Default of whether the master allows client to engage in sending block /
//! stream transfers of long duration under any circumstances.  Should be
//! TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.
#define ONE_NET_MASTER_CLIENT_ALLOW_LONG_BLOCK_STREAM

// The constants below pertain to default CLIENT-TO-CLIENT transfers.  In
// other words, transfers where the master is not involved at all.


//! Default of whether the master wants the client to change data rates when
//! sending block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.
#define ONE_NET_MASTER_CLIENT_BLOCK_STREAM_ELEVATE_DATA_RATE TRUE

//! Default of whether the master wants the client to change channels when
//! sending block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.  It is also only relevant for devices that can change data rates.
#define ONE_NET_MASTER_CLIENT_BLOCK_STREAM_CHANGE_CHANNEL TRUE

//! Default of whether the master wants the client to use high priority when
//! sending block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.
#define ONE_NET_MASTER_CLIENT_BLOCK_STREAM_HIGH_PRIORITY TRUE


// The four constants below pertain to default long MASTER-TO-CLIENT and
// CLIENT-TO-MASTER transfers.

//! Default of whether long block / stream transfers involving the master at
//! all
#define ONE_NET_MASTER_MASTER_ALLOW_LONG_BLOCK_STREAM TRUE


//! Default of whether the master wants to change data rates when involved in
//! block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.
#define ONE_NET_MASTER_MASTER_BLOCK_STREAM_ELEVATE_DATA_RATE TRUE

//! Default of whether the master wants to change channels when involved in
//! block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.  It is also only relevant for devices that can change data rates.
#define ONE_NET_MASTER_MASTER_BLOCK_STREAM_CHANGE_CHANNEL TRUE

//! Default of whether the master wants to use high priority when involved in
//! block / stream transfers of long duration.  Should be TRUE or FALSE.
//! Note that the master can choose to have some clients have this flag set, but
//! not others and can change it at any time.  This value simply sets the
//! INITIAL flag.  Note that this flag is only relevant for block / stream
//! transfers.
#define ONE_NET_MASTER_MASTER_BLOCK_STREAM_HIGH_PRIORITY TRUE








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
