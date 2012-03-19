//! \addtogroup ONE-NET_FEATURES
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
    \file one_net_features.c
    \brief Implementation of features-releated code.

    Implementation of features-releated code.
*/


#include "config_options.h"
#include "one_net_features.h"
#include "one_net_port_specific.h"





//==============================================================================
//                                  CONSTANTS
//! \defgroup ONE-NET_FEATURES_const
//! \ingroup ONE-NET_FEATURES
//! @{

const on_features_t THIS_DEVICE_FEATURES =
{
    THIS_DEVICE_FEATURE_BITS, THIS_DEVICE_DATA_RATES,
    THIS_DEVICE_QUEUE_BITS, THIS_DEVICE_PEERS_HOPS
};


const on_features_t FEATURES_UNKNOWN =
{
    0xFF, 0xFF, 0xFF, 0xFF
};




//! @} ONE-NET_FEATURES_const
//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS
//! \defgroup ONE-NET_FEATURES_typedefs
//! \ingroup ONE-NET_FEATURES
//! @{

//! @} ONE-NET_FEATURES_typedefs
//                                  TYPEDEFS END
//==============================================================================



//==============================================================================
//                      PRIVATE FUNCTION DECLARATIONS
//! \defgroup ONE-NET_FEATURES_pri_func
//! \ingroup ONE-NET_FEATURES
//! @{

//! @} ONE-NET_FEATURES_pri_func
//                      PRIVATE FUNCTION DECLARATIONS END
//==============================================================================

//==============================================================================
//                      PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup ONE-NET_FEATURES_pub_func
//! \ingroup ONE-NET_FEATURES
//! @{


BOOL features_known(on_features_t features)
{
    return !(one_net_memcmp(&features, &FEATURES_UNKNOWN,
      sizeof(on_features_t)) == 0);
}


BOOL features_device_sleeps(on_features_t features)
{
    return ((features.feature_flags & ON_DEVICE_NEVER_SLEEPS_MASK) == 0);
}


#ifndef _ONE_NET_SIMPLE_CLIENT
UInt8 features_max_hops(on_features_t features)
{
    return features.peers_hops & ON_FEATURES_MAX_HOPS_MASK;
}


UInt8 features_max_peers(on_features_t features)
{
    return (features.peers_hops & ON_FEATURES_PEER_MASK) >> ON_FEATURES_PEER_SHIFT;   
}


BOOL features_data_rate_capable(on_features_t features, UInt8 data_rate)
{
    if(data_rate >= ONE_NET_DATA_RATE_LIMIT)
    {
        return ONS_BAD_PARAM;
    }
    return (((features.data_rates >> data_rate) & 0x01) != 0);
}


// TODO -- add #ifdef check for _DATA_RATE?
UInt8 features_highest_data_rate(on_features_t features)
{
    UInt8 dr;
    for(dr = ONE_NET_DATA_RATE_LIMIT; dr > ONE_NET_DATA_RATE_38_4; dr--)
    {
        if(features_data_rate_capable(features, dr))
        {
            return dr;
        }
    }
    
    return ONE_NET_DATA_RATE_38_4;
}


UInt8 features_highest_matching_data_rate(on_features_t dev1_features,
  on_features_t dev2_features)
{
    UInt8 dr;
    for(dr = ONE_NET_DATA_RATE_LIMIT; dr > ONE_NET_DATA_RATE_38_4; dr--)
    {
        if(features_data_rate_capable(dev1_features, dr) &&
          features_data_rate_capable(dev2_features, dr))
        {
            return dr;
        }
    }
    
    return ONE_NET_DATA_RATE_38_4;
}


BOOL features_peer_capable(on_features_t features)
{
    return ((features.feature_flags & ON_PEER_FEATURE_MASK) != 0);
}


BOOL features_mh_capable(on_features_t features)
{
    return ((features.feature_flags & ON_MH_FEATURE_MASK) != 0);
}


BOOL features_mh_repeat_capable(on_features_t features)
{
    return ((features.feature_flags & ON_MH_REPEATER_FEATURE_MASK) != 0);
}


BOOL features_block_capable(on_features_t features)
{
    return ((features.feature_flags & ON_BLOCK_FEATURE_MASK) != 0);
}


BOOL features_stream_capable(on_features_t features)
{
    return ((features.feature_flags & ON_STREAM_FEATURE_MASK) != 0);
}


BOOL features_simple_client(on_features_t features)
{
    return ((features.feature_flags & ON_NON_SIMPLE_CLIENT_MASK) == 0);
}
#endif


//! @} ONE-NET_FEATURES_pub_func
//                      PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                      PRIVATE FUNCTION IMPLEMENTATION
//! \addtogroup ONE-NET_FEATURES_pri_func
//! \ingroup ONE-NET_FEATURES
//! @{

//! @} ONE-NET_FEATURES_pri_func
//                      PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

//! @} ONE-NET_FEATURES

