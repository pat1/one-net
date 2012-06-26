//! \addtogroup ONE-NET_ACKNOWLEDGE
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
    \brief Implementation of ack / nack functions.

    Implementation of ack / nack functions.
*/


#include "config_options.h"
#include "one_net_acknowledge.h"
#include "one_net_port_specific.h"





//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_ACKNOWLEDGE_const
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

//! @} ONE-NET_ACKNOWLEDGE_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_ACKNOWLEDGE_typedefs
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

//! @} ONE-NET_ACKNOWLEDGE_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_ACKNOWLEDGE_pri_func
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

//! @} ONE-NET_ACKNOWLEDGE_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_ACKNOWLEDGE_pub_func
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{


BOOL nack_reason_is_fatal(const on_nack_rsn_t nack_reason)
{
    BOOL is_fatal = FALSE;
	if(nack_reason >= ON_NACK_RSN_MIN_FATAL)
	{
		is_fatal = TRUE;
	}
	
    #ifndef ONE_NET_SIMPLE_CLIENT
    // call application code to see if it wants to change.
	one_net_adjust_fatal_nack(nack_reason, &is_fatal);
    #endif
    return is_fatal;
}


//! @} ONE-NET_ACKNOWLEDGE_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_ACKNOWLEDGE_pri_func
//! \ingroup ONE-NET_ACKNOWLEDGE
//! @{

//! @} ONE-NET_ACKNOWLEDGE_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_ACKNOWLEDGE

