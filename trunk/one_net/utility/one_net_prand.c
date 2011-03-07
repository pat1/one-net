//! \addtogroup one_net_prand
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
    \file one_net_prand.c
    \brief Implementation of Pseudo Random Number Generator
    
    \note See one_net.h for the version of the ONE-NET source as a whole.  If
      any one file is modified, the version number in one_net.h will need to be
      updated.
*/

#include <one_net/one_net_types.h>


//==============================================================================
//                                  CONSTANTS
//! \defgroup one_net_prand_const
//! \ingroup one_net_prand
//! @{

//! @} one_net_prand_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup one_net_prand_typedefs
//! \ingroup one_net_prand
//! @{

//! @} one_net_prand_typedefs
//                                  TYPEDEFS END
//==============================================================================

//==============================================================================
//                              PRIVATE VARIABLES
//! \defgroup one_net_prand_pri_var
//! \ingroup one_net_prand
//! @{

//! @} one_net_prand_pri_var
//                              PRIVATE VARIABLES END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup one_net_prand_pri_func
//! \ingroup one_net_prand
//! @{

//! @} one_net_prand_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup one_net_prand_pub_func
//! \ingroup one_net_prand
//! @{

/*
    \brief Returns a pseudo random value.

    This is a Linear Congruential Generator (V(j + 1) = (A * V(j) + B) % M).

    \param[in] MODIFIER Value to modify the random number to be output by.
    \param[in] MAX_RAND The maximum random value to be output.
   
    \return A pseudo random number.
*/
UInt32 one_net_prand(const UInt32 MODIFIER, const UInt32 MAX_RAND)
{
    // The pseudo random value that is the basis for the returned value
    static UInt32 pseudo_rand = 2;

    pseudo_rand = 1664525 * pseudo_rand + 1013904233;

    // add since it is faster than a multiply or divide
    return (pseudo_rand + MODIFIER) % (MAX_RAND + 1);
} // one_net_prand //

//! @} one_net_prand_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup one_net_prand_pri_func
//! \ingroup one_net_prand
//! @{

//! @} one_net_prand_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} one_net_prand

