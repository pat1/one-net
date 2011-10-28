#ifndef _ONE_NET_PORT_CONST_H
#define _ONE_NET_PORT_CONST_H



//! \defgroup ONE-NET_port_const Application Specific ONE-NET constants.
//! \ingroup ONE-NET
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
    \file one_net_port_const.h
    \brief Application specific ONE-NET constants.

    These are constants that are specific to each ONE-NET device
*/



//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_port_const_const
//! \ingroup ONE-NET_port_const
//! @{



// uart buffer size
enum
{
    UART_RX_BUF_SIZE = 50,   //!< Size of the uart receive buffer
    UART_TX_BUF_SIZE = 50    //!< Size of the uart transmit buffer
};



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
