#ifndef _TH_TIME_H
#define _TH_TIME_H

#include <stdbool.h>
#include <sys/time.h>

//! \addtogroup th_time_documentation
//! @{

/*!
    \file th_time.h
    \brief General time related functions

    These are functions that go into the library for dealing with time (such as
    adding, subtracting, comparing, etc).

    \author Jay
    \note   Threshold Corporation

*/

//=============================================================================
//                                  CONSTANTS

enum
{
    SEC_TO_MSEC = 1000,             //!< Converts from s to ms
    SEC_TO_USEC = 1000000,          //!< Converts from s to us
    SEC_TO_NSEC = 1000000000,       //!< Converts from s to ns
    MSEC_TO_USEC = 1000,            //!< Converts from ms to us
    MSEC_TO_NSEC = 100000,          //!< Converts from ms to ns
    USEC_TO_NSEC = 1000             //!< Converts from us to ns
};

//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  STRUCTURES

//                                  STRUCTURES END
//=============================================================================

//=============================================================================
//                                  PUBLIC VARIABLES

//                                  PUBLIC VARIABLES END
//=============================================================================

//=============================================================================
//                                  PUBLIC FUNCTION DECLARATIONS

// add to a timeval structure (+=)
bool tv_add(struct timeval * lhs, const struct timeval * const rhs);

// difference between now and then
int tv_diff(const struct timeval * const then,
 const struct timeval * const now);

// compare 2 times
int tv_compare(const struct timeval * const lhs, const struct timeval * const rhs);

//                                  PUBLIC FUNCTION DECLARATIONS END
//=============================================================================

//! @}
#endif // _TH_TIME_H

