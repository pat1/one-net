#ifndef _CLIENT_PORT_CONST_H
#define _CLIENT_PORT_CONST_H

//! \defgroup CLIENT_port_const ONE-NET CLIENT Specific constants
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
    \file client_port_const.h
    \brief ONE-NET CLIENT specific constants.

    These are constants that are specific to each ONE-NET CLIENT device.  This
    file should be copied to a project specific location and renamed to
    client_port_const.h.

    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/


//=============================================================================
//                                  CONSTANTS



enum
{
    //! The number of remembered devices that communicate with this device.
    ONE_NET_RX_FROM_DEVICE_COUNT = 1
};


//! Time constants
//! Number of ticks to scan each channel when trying to join the network.
//! 1 second
#define ONE_NET_SCAN_CHANNEL_TIME 1000



//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  TYPEDEFS

//                                  TYPEDEFS END
//=============================================================================

//=============================================================================
//                              PUBLIC VARIABLES

//                              PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                      PUBLIC FUNCTION DECLARATIONS

//                      PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @} CLIENT_port_const

#endif // _CLIENT_PORT_CONST_H //
