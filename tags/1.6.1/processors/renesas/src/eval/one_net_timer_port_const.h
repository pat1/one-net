#ifndef _ONE_NET_TIMER_PORT_CONST_H
#define _ONE_NET_TIMER_PORT_CONST_H

//! \defgroup ONE-NET_TIMER_PORT_CONST Timer constants defined by the
//!   application.
//! \ingroup ONE-NET_TIMER
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
    \file one_net_timer_port_const.h
    \brief ONE-NET Timer constants defined by the application.

    These are constants that are specific to each ONE-NET device.  This file
    should be copied to a project specific location and renamed to
    one_net_timer_port_const.h.
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


//=============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_TIMER_PORT_CONST_const
//! \ingroup ONE-NET_TIMER_PORT_CONST
//! @{

/*!
    \brief Timers used by the application
    
    If the application wishes to use the one_net_timer module, it must add
    the the timers it uses before NUM_APP_TIMERS, starting with 0, and
    incrementing with each timer used.  These timer enurations are how
    the application will address each timer that it uses.
*/
enum
{
    //! Timer used in auto mode to automatically send packets
    AUTO_MODE_TIMER = 0,
    
    //! Timer used to turn off the transmit led
    TX_LED_TIMER,
    
    //! Timer used to turn off the receive led
    RX_LED_TIMER,

    //! Timer to output prompt if no incoming data for a certain period.
    PROMPT_TIMER,

    //! Timer used when user input has been detected before continuing on.
    USER_INPUT_TIMER,

    //! The number of timers used by the appplication.  This must be defined
    //! last in this enumeration
    ONT_NUM_APP_TIMERS
};

//! @} ONE-NET_TIMER_PORT_CONST_const
//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_TIMER_PORT_CONST_typedefs
//! \ingroup ONE-NET_TIMER_PORT_CONST
//! @{
    
//! @} ONE-NET_TIMER_PORT_CONST_typedefs
//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES
//! \defgroup ONE-NET_TIMER_PORT_CONST_pub_var
//! \ingroup ONE-NET_TIMER_PORT_CONST
//! @{

//! @} ONE-NET_TIMER_PORT_CONST_pub_var
//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS
//! \defgroup ONE-NET_TIMER_PORT_CONST_pub_func
//! \ingroup ONE-NET_TIMER_PORT_CONST
//! @{

//! @} ONE-NET_TIMER_PORT_CONST_pub_func
//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} ONE-NET_TIMER_PORT_CONST

#endif // _ONE_NET_TIMER_PORT_CONST_H //

