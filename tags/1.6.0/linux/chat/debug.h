/* debug.h */

#ifndef _DEBUG_H_
#define _DEBUG_H_

/*
 * INCLUDES
 */
#include <stdio.h>
#include <sys/time.h>

// TBD: it looks like print_debug1 and print_debug_noheader were added by
//      two different people (roger and suneetha) but they seem to do the same
//      thing. One should be replaced by the other?

/*
 * DEFINES
 */
#define print_debug(args...)			\
    { char buffer[TV_STRING_SIZE];      \
    if ( debug_level() >= 1 ) {		\
        fprintf( debug_fp(), "%s [%u] %s: ", \
        get_tv_string( buffer, TV_STRING_SIZE ),\
        one_net_tick(), get_program_name() );	\
        fprintf( debug_fp(), args );	\
        fflush( debug_fp() );			\
    } }

#define print_debug1(args...)			\
    {                                   \
    if ( debug_level() >= 1 ) { 		\
        fprintf( debug_fp(), args );	\
        fflush( debug_fp() );			\
    } }

#define print_debug_noheader(args...)			\
    if ( debug_level() >= 1 ) {		\
        fprintf( debug_fp(), args );	\
        fflush( debug_fp() );			\
    } 

#define TV_STRING_SIZE         32

/*
 * PROTOTYPES
 */
FILE * debug_fp( void );
int debug_level( void );
char * get_program_name( void );
void set_debug_file( const char * const);
void set_debug_level( int );
void set_program_name( char * );
void set_program_version( const char * const );
int get_timer_value( struct timeval *tv );
char * get_tv_string( char *buffer, int length );
void debug_flush( void );

#endif /*_DEBUG_H_*/
