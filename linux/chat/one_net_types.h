#ifndef ONE_NET_TYPES_H
#define ONE_NET_TYPES_H

#ifndef _R8C_TINY
#include <stdint.h>
#endif

//! \defgroup Renesas_R8C
//! \ingroup ONE-NET_Types
//! @{

/*!
    \file one_net_types.h
    \brief Types used by ONE-NET

    This file defines known types and sizes.  It is hardware dependent, so it
    will need to be changed when the hardware is changed.
*/

#ifdef _R8C_TINY
typedef unsigned char   UInt8;
typedef signed char     SInt8;
typedef unsigned short  UInt16;
typedef short           SInt16;
typedef unsigned int    UInt32;
typedef int             SInt32;
#else
typedef uint8_t         UInt8;
typedef int8_t          SInt8;
typedef uint16_t        UInt16;
typedef int16_t         SInt16;
typedef uint32_t        UInt32;
typedef int32_t         SInt32;
#endif

typedef float           Float32;
typedef UInt32			tick_t;

typedef enum
{
    FALSE = 0,
    TRUE = 1,
} BOOL;

//! @} Renesas_R8C

#endif
