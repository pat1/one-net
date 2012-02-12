//! \addtogroup ONE-NET_MESSAGE
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
    \file one_net_acknowledge.c
    \brief Implementation of message functions, including the single essage queue

    Implementation of message functions, including the single essage queue.
*/


#include "config_options.h"
#include "one_net_types.h"
#include "one_net_port_const.h"
#include "one_net_message.h"
#include "one_net_acknowledge.h"
#include "one_net_port_specific.h"
#include "tick.h"
#include "one_net.h"


//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_MESSAGE_const
//! \ingroup ONE-NET_MESSAGE
//! @{


const on_encoded_did_t NO_DESTINATION = {0xFF, 0xFF};


//! @} ONE-NET_MESSAGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_MESSAGE_typedefs
//! \ingroup ONE-NET_MESSAGE
//! @{

//! @} ONE-NET_MESSAGE_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{



//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup ONE-NET_TIMER_pri_var
//! \ingroup ONE-NET_TIMER
//! @{



//! @} ONE-NET_MESSAGE_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_MESSAGE_pub_func
//! \ingroup ONE-NET_MESSAGE
//! @{



//! @} ONE-NET_MESSAGE_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_MESSAGE_pri_func
//! \ingroup ONE-NET_MESSAGE
//! @{
    


//! @} ONE-NET_MESSAGE_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_MESSAGE
