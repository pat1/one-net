// one_net_data_rate.h
#ifndef _ONE_NET_DATA_RATE
#define _ONE_NET_DATA_RATE

#define DATA_RATE_UNKNOWN 0xFF



//! Data rate type
typedef enum
{
    ONE_NET_DATA_RATE_38_4,         //!< 38400 bps
    ONE_NET_DATA_RATE_76_8,         //!< 76800 bps
    ONE_NET_DATA_RATE_115_2,        //!< 115200 bps
    ONE_NET_DATA_RATE_153_6,        //!< 153600 bps
    ONE_NET_DATA_RATE_192_0,        //!< 192000 bps
    ONE_NET_DATA_RATE_230_4,        //!< 230400 bps

    //! 1 more than the max data rate.  Data rates must be added before this
    //! value
    ONE_NET_DATA_RATE_LIMIT
} on_data_rate_t;


//! Data rate mask
typedef enum
{
    ONE_NET_DATA_RATE_38_4_MASK = 0x01,
    ONE_NET_DATA_RATE_76_8_MASK = 0x02,
    ONE_NET_DATA_RATE_115_2_MASK = 0x04,
    ONE_NET_DATA_RATE_153_6_MASK = 0x08,
    ONE_NET_DATA_RATE_192_0_MASK = 0x10,
    ONE_NET_DATA_RATE_230_4_MASK = 0x20
} on_data_rate_mask_t;


#endif
