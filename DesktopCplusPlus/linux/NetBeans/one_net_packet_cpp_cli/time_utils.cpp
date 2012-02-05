#include <stdlib.h>

#include "time_utils.h"


struct timeval add_timeval(struct timeval time1, struct timeval time2)
{
    struct timeval sum;
    sum.tv_sec = time1.tv_sec + time2.tv_sec;
    sum.tv_usec = time1.tv_usec + time2.tv_usec;
    if(sum.tv_usec >= 1000000)
    {
        sum.tv_sec++;
        sum.tv_usec -= 1000000;
    }
    return sum;
}


int timeval_compare(struct timeval time1, struct timeval time2)
{
    if(time1.tv_sec < time2.tv_sec)
    {
        return -1;
    }
    else if(time1.tv_sec > time2.tv_sec)
    {
        return 1;
    }

    if(time1.tv_usec == time2.tv_usec)
    {
        return 0;
    }
    else if(time1.tv_usec < time2.tv_usec)
    {
        return -1;
    }

    return 1;
}


uint64_t struct_timeval_to_microseconds(struct timeval timestamp)
{
    uint64_t time_micro = 1000000 * ((uint64_t) timestamp.tv_sec) +
        timestamp.tv_usec;
    return time_micro;
}


struct timeval microseconds_to_struct_timeval(uint64_t time_micro)
{
    struct timeval timestamp;
    timestamp.tv_usec = time_micro % 1000000;
    timestamp.tv_sec  = time_micro / 1000000;
    return timestamp;
}


uint64_t struct_timeval_to_milliseconds(struct timeval timestamp)
{
    return (struct_timeval_to_microseconds(timestamp) / 1000);
}


struct timeval milliseconds_to_struct_timeval(uint64_t time_milli)
{
    return microseconds_to_struct_timeval(1000 * time_milli);
}
