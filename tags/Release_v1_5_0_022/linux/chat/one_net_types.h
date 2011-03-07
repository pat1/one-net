#ifndef ONE_NET_TYPES_H
#define ONE_NET_TYPES_H

//! \defgroup Renesas_R8C
//! \ingroup ONE-NET_Types
//! @{

/*!
    \file one_net_types.h
    \brief Types used by ONE-NET

    This file defines known types and sizes.  It is hardware dependent, so it
    will need to be changed when the hardware is changed.
*/

typedef unsigned char   UInt8;
typedef signed char     SInt8;
typedef unsigned short    UInt16;
typedef short             SInt16;
typedef unsigned int   UInt32;
typedef int            SInt32;

typedef float           Float32;
typedef UInt32			tick_t;

typedef enum
{
    FALSE = 0,
    TRUE = 1,
} BOOL;

//! @} Renesas_R8C

#endif
