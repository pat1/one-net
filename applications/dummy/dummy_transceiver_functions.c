#include "config_options.h"
#include "one_net_data_rate.h"
#include "one_net_channel.h"
#include "one_net_types.h"
#include "one_net_features.h"
#include "one_net_status_codes.h"



/*
These are "dummy" functions.  Copy / paste / change / use them as a skeleton 
for your application code.
June 3, 2012 -- Currently these functions have not been thoroughly tested.
*/



//! The current ONE-NET channel
UInt8 current_channel = 0;

//! The current data rate
UInt8 current_data_rate = ONE_NET_DATA_RATE_38_4;





one_net_status_t tal_set_channel(const UInt8 channel)
{
    if(channel < ONE_NET_NUM_CHANNELS)
    {
        // see config_options.h.  Make sure _CHANNEL_OVERRIDE and
        // CHANNEL_OVERRIDE_CHANNEL are defined if overriding the channel
        // parameter here.  Make sure _CHANNEL_OVERRIDE is NOT defined if
        // NOT overriding the channel.
        #ifdef _CHANNEL_OVERRIDE
        current_channel = CHANNEL_OVERRIDE_CHANNEL
        #else
        current_channel = channel;
        #endif
        return ONS_SUCCESS;
    } // if the parameter is valid //
    
    return ONS_BAD_PARAM;
} // tal_set_channel //


one_net_status_t tal_set_data_rate(UInt8 data_rate)
{
    one_net_status_t status;
    
    #ifndef DATA_RATE_CHANNEL
    if(data_rate != ONE_NET_DATA_RATE_38_4)
    {
        return ONS_DEVICE_NOT_CAPABLE;
    }
    #else
    
    if(!features_data_rate_capable(THIS_DEVICE_FEATURES, data_rate))
    {
        return ONS_DEVICE_NOT_CAPABLE;
    }
    #endif
    
    current_data_rate = data_rate;    
    return status;
}


BOOL tal_channel_is_clear(void)
{
    return TRUE;
} // tal_channel_is_clear //


UInt8 tal_write_packet(const UInt8 * data, const UInt8 len)
{
    if(!data)
    {
        return 0;
    }
    return len;
}


BOOL tal_write_packet_done()
{
    return TRUE;
}


UInt8 tal_read_bytes(UInt8 * data, const UInt8 len)
{
    if(!data)
    {
        return 0;
    }
    return len;
}


one_net_status_t tal_look_for_packet(tick_t duration)
{
    if(duration == 0)
    {
        return ONS_TIME_OUT;
    }
    return ONS_SUCCESS;
}
