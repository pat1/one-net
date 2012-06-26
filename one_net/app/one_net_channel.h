// one_net_channel.h

#ifndef _ONE_NET_CHANNEL
#define _ONE_NET_CHANNEL


#include "config_options.h"


//! The frequencies supported by ONE-NET (USA & European)  These need to be 0
//! based without any gaps.  Whole groups (US & European) need to be included.
typedef enum
{
#ifdef US_CHANNELS
    // US frequencies
    ONE_NET_MIN_US_CHANNEL,                             //!< Min US frequency
    ONE_NET_US_CHANNEL_1 = ONE_NET_MIN_US_CHANNEL,      //!< 903.0Mhz
    ONE_NET_US_CHANNEL_2,                               //!< 904.0Mhz
    ONE_NET_US_CHANNEL_3,                               //!< 905.0Mhz
    ONE_NET_US_CHANNEL_4,                               //!< 906.0Mhz
    ONE_NET_US_CHANNEL_5,                               //!< 907.0Mhz
    ONE_NET_US_CHANNEL_6,                               //!< 908.0Mhz
    ONE_NET_US_CHANNEL_7,                               //!< 909.0Mhz
    ONE_NET_US_CHANNEL_8,                               //!< 910.0Mhz
    ONE_NET_US_CHANNEL_9,                               //!< 911.0Mhz
    ONE_NET_US_CHANNEL_10,                              //!< 912.0Mhz
    ONE_NET_US_CHANNEL_11,                              //!< 913.0Mhz
    ONE_NET_US_CHANNEL_12,                              //!< 914.0Mhz
    ONE_NET_US_CHANNEL_13,                              //!< 915.0Mhz
    ONE_NET_US_CHANNEL_14,                              //!< 916.0Mhz
    ONE_NET_US_CHANNEL_15,                              //!< 917.0Mhz
    ONE_NET_US_CHANNEL_16,                              //!< 918.0Mhz
    ONE_NET_US_CHANNEL_17,                              //!< 919.0Mhz
    ONE_NET_US_CHANNEL_18,                              //!< 920.0Mhz
    ONE_NET_US_CHANNEL_19,                              //!< 921.0Mhz
    ONE_NET_US_CHANNEL_20,                              //!< 922.0Mhz
    ONE_NET_US_CHANNEL_21,                              //!< 923.0Mhz
    ONE_NET_US_CHANNEL_22,                              //!< 924.0Mhz
    ONE_NET_US_CHANNEL_23,                              //!< 925.0Mhz
    ONE_NET_US_CHANNEL_24,                              //!< 926.0Mhz
    ONE_NET_US_CHANNEL_25,                              //!< 927.0Mhz
    ONE_NET_MAX_US_CHANNEL = ONE_NET_US_CHANNEL_25,     //!< Max US frequency
#endif
#ifdef EUROPE_CHANNELS    
    // European frequencies
    ONE_NET_MIN_EUR_CHANNEL,                            //!< Min European freq.
    ONE_NET_EUR_CHANNEL_1 = ONE_NET_MIN_EUR_CHANNEL,    //!< 865.8Mhz
    ONE_NET_EUR_CHANNEL_2,                              //!< 866.5Mhz
    ONE_NET_EUR_CHANNEL_3,                              //!< 867.2Mhz
    ONE_NET_MAX_EUR_CHANNEL = ONE_NET_EUR_CHANNEL_3,    //!< Max European freq.
#endif
    ONE_NET_NUM_CHANNELS,                               //!< Number of channels
    ONE_NET_MAX_CHANNEL = ONE_NET_NUM_CHANNELS - 1      //!< Max ONE-NET channel
} one_net_channel_t;


#endif
