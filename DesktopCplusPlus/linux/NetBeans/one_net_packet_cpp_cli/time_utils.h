#ifndef TIME_UTILS_H
#define	TIME_UTILS_H

#include <sys/time.h>
#include <stdint.h>


struct timeval add_timeval(struct timeval time1, struct timeval time2);
int timeval_compare(struct timeval time1, struct timeval time2);
uint64_t struct_timeval_to_microseconds(struct timeval timestamp);
struct timeval microseconds_to_struct_timeval(uint64_t time_micro);
uint64_t struct_timeval_to_milliseconds(struct timeval timestamp);
struct timeval milliseconds_to_struct_timeval(uint64_t time_milli);


#endif	/* TIME_UTILS_H */

