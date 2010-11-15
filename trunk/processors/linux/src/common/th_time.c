#include <stdlib.h>

#include "th_time.h"

//! \addtogroup th_time_documentation
//! @{

//=============================================================================
//                                  CONSTANTS

//                                  CONSTANTS END
//=============================================================================

//=============================================================================
//                                  STRUCTURES

//                                  STRUCTURES END
//=============================================================================

//=============================================================================
//                                  PRIVATE VARIABLES

//                                  PRIVATE VARIABLES END
//=============================================================================

//=============================================================================
//                                  PRIVATE FUNCTION DECLARATIONS

//                                  PRIVATE FUNCTION DECLARATIONS END
//=============================================================================

//=============================================================================
//                                  PUBLIC FUNCTION IMPLEMENTATION
//! \defgroup th_time_public_functions
//! @{

/*!
    \brief Add time to the lhs structure.

    Add rhs to lhs and store in lhs (like +=)

    \param[in/out] lhs timeval struct to add time to
    \param[in] rhs Time to add
    \return true if successful
*/
bool tv_add(struct timeval * lhs, const struct timeval * const rhs)
{
    if(lhs == NULL || rhs == NULL)
    {
        return false;
    }

    lhs->tv_usec += rhs->tv_usec;
    // add usec overflow to sec
    lhs->tv_sec += rhs->tv_sec + lhs->tv_usec / SEC_TO_USEC;
    lhs->tv_usec %= SEC_TO_USEC;

    return true;
}

/*!
    \brief Utility function for calculating the time difference

    Returns the time difference in microseconds between then and now

    \param[in] then the earlier time to subtract
    \param[in] now the later time to subtract from
    \return the difference between now and then
*/
int tv_diff(const struct timeval * const then,
 const struct timeval * const now)
{
    if(then == NULL || now == NULL || then->tv_sec > now->tv_sec)
    {
        return 0;
    }

    if(now->tv_sec > then->tv_sec)
    {
        return ((now->tv_sec - then->tv_sec) * 1000000) +
         (now->tv_usec - then->tv_usec);
    }
 
    return now->tv_usec - then->tv_usec;
} // tv_diff //

/*!
    \brief Utility function for comparing time

    Calculates if time1 < time2, time1 == time2, time1 > time2

    \param[in] lhs What would be the left hand side of the comparison equations
    \param[in] rhs What would be the right hand side of the comparison equations
    \return int -1 if lhs < rhs
    0 if lhs == rhs
    1 if lhs > rhs
*/
int tv_compare(const struct timeval * const lhs, const struct timeval * const rhs)
{
    if(lhs == NULL || rhs == NULL || lhs == rhs)
    {
        return 0;
    }

    if(lhs->tv_sec < rhs->tv_sec)
    {
        return -1;
    }

    if(lhs->tv_sec > rhs->tv_sec)
    {
        return 1;
    }

    if(lhs->tv_usec < rhs->tv_usec)
    {
        return -1;
    }

    if(lhs->tv_usec > rhs->tv_usec)
    {
        return 1;
    }

    // they are ==
    return 0;
} // tv_compare //

//! @} th_time_public_function
//                                  PUBLIC FUNCTION IMPLEMENTATION END
//=============================================================================

//=============================================================================
//                                  PRIVATE FUNCTION IMPLEMENTATION

//                                  PRIVATE FUNCTION IMPLEMENTATION END
//=============================================================================
//! @} th_time_documentation

