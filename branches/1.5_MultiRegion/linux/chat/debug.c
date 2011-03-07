/* debug.c */


/* INCLUDES */
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "debug.h"

#define MAX_PATHNAME_SIZE 128
#define MIN_FILENAME_SIZE 10
#define PROGRAM_NAME_SIZE 128

/* STATIC VARS */
static FILE * the_debug_fp = NULL;
static int the_debug_level = 0;
static char the_debug_path[MAX_PATHNAME_SIZE] = {'\0'};

static char debug_file[MAX_PATHNAME_SIZE] = {'\0'};
static char the_program_name[PROGRAM_NAME_SIZE] = { '\0' };
const char *default_program_name = "<program name not defined>";
#ifndef EMBED
const char *default_debug_path = "/tmp/";
#else
const char *default_debug_path = "/mnt/nandflash/";
#endif
const char *debug_path_name_config_file="/usr/share/soho_set/config/debug_path";
static char *gettimeofday_failed = "ERR:gettimeofday() failed.";

extern int errno;

/* FUNCTIONS */


/*! \defgroup debug_public_functions Debug Public Functions. */
/*! @{ */ 

//========================================================================
//========================================================================

/*! \brief Return the current debug FILE pointer. 

Instead of making the variables global, access to the debug level
and debug FILE pointer are through functions.

\param void
\return FILE pointer to the debug file
*/
FILE * debug_fp( void )
{
    if( the_debug_fp == NULL ) 
    {
        if( strlen(debug_file) == 0 ) 
        {
            the_debug_fp = stdout;
        } else 
        {
            the_debug_fp = fopen( debug_file, "a" );
            if ( the_debug_fp == NULL ) {
                the_debug_fp = stdout;
            }
        } //end of else there is a debug file
	} //end if fp is NULL
    else if( the_debug_fp != stdout )
    {
        //Added to ensure that a new file is created if it has been
        //deleted
        fclose(the_debug_fp);
        the_debug_fp = fopen( debug_file, "a" );
    } //end of else fp is set

    return( the_debug_fp );
} /* debug_fp */


//========================================================================
//========================================================================

/*! \brief Flush the debug file stream.

Flush the debug file stream so that we can see the lastest information 
that was sent to the debug file stream.

\param void
\return void
*/
void debug_flush( void )
{
    fflush( the_debug_fp );

} /* debug_flush */


//========================================================================
//========================================================================

/*! \brief Return the current debug level.

Instead of making the variables global, access to the debug level
and debug FILE pointer are through functions.

\param void
\return The int debug level
*/
int debug_level( void )
{
    return( the_debug_level );
} /* debug_level */


//========================================================================
//========================================================================

/*! \brief Return the program name

Instead of making the variables global, access to the program name
should be through the set_program_name and get_program_name functions.

\param void
\return Pointer to a null terminated string containing the program name (aka
argv[0]).
*/
char * get_program_name( void )
{
    if( (the_program_name == NULL) || (the_program_name[0] == '\0') ) 
    {
        strcpy(the_program_name, default_program_name);
	}
    return( the_program_name );
} /* get_program_name */


//========================================================================
//========================================================================

/*! \brief Return the path to the log file directory

Instead of making the variables global, access to the program name
should be through the set_program_name and get_program_name functions.

\param void
\return Pointer to a null terminated string containing the program name (aka
argv[0]).
*/
char * get_debug_path( void )
{
    FILE *config_fp;

    // see if the_debug_path has already been initialized
    if ( the_debug_path[0] == '\0' ) 
    {
        // no, figure out where we should put debug log files
        config_fp = fopen( debug_path_name_config_file, "r" );
        if ( config_fp == NULL ) 
        {
            // no config file found, use the default path
            strcpy( the_debug_path, default_debug_path );
        } 
        else 
        {
            // read the path from the config file
            if ( fgets( the_debug_path, MAX_PATHNAME_SIZE - MIN_FILENAME_SIZE,
                                                config_fp )  != NULL ) 
            {
                // replace the newline with a null
                if ( the_debug_path[strlen(the_debug_path)-1] == '\n' ) 
                {
                    the_debug_path[strlen(the_debug_path)-1] = '\0';
                // and make sure the path ends with a /
                } 
                else if ( the_debug_path[strlen(the_debug_path)-1] != '/' ) 
                {
                    strcpy(the_debug_path, default_debug_path);
                }
            } 
            else 
            {
                // if there was an error reading, just use the default path
                strcpy( the_debug_path, default_debug_path );
            }
            fclose( config_fp );
        }
    } 

    // return the location we already calculated
    return( the_debug_path );

} /* get_debug_path*/


//========================================================================
//========================================================================

/*! \brief Return the number of seconds and microseconds since the 
Epoch (see time(2) and gettimeofday(2)). tv is short for timer value.

This function just returns the structure you get when calling the system
funtion gettimeofday(). It is provided so that we have a common interface
for retreiving the current time down to micorseconds.


\param void
\return Returns 0 if the call worked, or -1 if there was an error.
*/
int get_tv( struct timeval *tv )
{
    if( gettimeofday( tv, NULL ) != 0 ) 
    {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
        return ( -1 );
	} 
    else 
    {
        return( 0 );
    }
} /* get_tv */


//========================================================================
//========================================================================

/*! \brief Return the number of seconds and microseconds since the 
Epoch (see time(2) and gettimeofday(2)) formatted as a string.
tv is short for timer value.

This function returns formated time stamp of the form MM/DD/YYYY HH:MM:SS.MMMMMM.


\param[out] tv Pointer to a buffer to hold the formatted time string.
\param[in] length Size of the buffer provided to hold the output string.
\return Returns the pointer to the buffer that was passed in.
*/
char * get_tv_string( char *tv_string, int length )
{
    struct timeval tv;
    struct tm ctm;
    
    assert(tv_string != NULL);
    if( gettimeofday( &tv, NULL ) != 0 ) 
    {
        // supply a special string if the call failed.
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        return ( gettimeofday_failed );
	} 
    else if ( length <= 0 )
    {
        // pick a fixed date/time if the input length is less than 1
        // TBD: this logic is here just for unit testing, we may want 
        // to remove it later.
        tv.tv_sec = 31536000;
        tv.tv_usec = 123456;
        length = TV_STRING_SIZE;
    }
    // now convert the tv.tv_sec and tv.tv_usec into a string
    if ( localtime_r( &tv.tv_sec, &ctm ) == NULL ) 
    {
        strcpy( tv_string, "ERR: ctime_r(...) failed." );
        return( tv_string );
    }
    sprintf( tv_string, "%02d/%02d/%04d %02d:%02d:%02d.%06d:",
        ctm.tm_mon+1, ctm.tm_mday, ctm.tm_year+1900, ctm.tm_hour,
        ctm.tm_min, ctm.tm_sec, (int) tv.tv_usec );
    return( tv_string );
} /* get_tv_string */


//========================================================================
//========================================================================

/*! \brief Set the debug file

Allow caller to set the debug file to be used.  

\param filename IN: a filename string
\return void
*/
void set_debug_file( const char * const filename)
{
    // copy the common direstory name portion of the path name
    strncpy(debug_file, get_debug_path(),
                    MAX_PATHNAME_SIZE - MIN_FILENAME_SIZE );

    // concatenate the log file specific filename portion of the path name
    // allowing room for a null.
    strncat(debug_file, filename, MAX_PATHNAME_SIZE - strlen(debug_file) - 1);
    debug_file[MAX_PATHNAME_SIZE-1] = '\0';

} /* set_debug_file */


//========================================================================
//========================================================================

/*! \brief Set the debug level
 
Allow caller to set the debug level

\param level IN: an integer debug level to set
\return void
*/
void set_debug_level( int level )
{
    the_debug_level = level;
} /* set_debug_level */

//========================================================================
//========================================================================

/*! \brief Set the program name

Allow caller to set the debug file to be used.  

\param filename IN: a filename string
\return void
*/
void set_program_name( char *name )
{
    strncpy(the_program_name, name, PROGRAM_NAME_SIZE);

} /* set_program_name */


//========================================================================
//========================================================================

/*! \brief Set the program version

Allow caller to set the debug file to be used.  

\param[in] NAME: a filename string
\return void
*/
void set_program_version( const char * const NAME )
{
    strncat(the_program_name, NAME,
                    PROGRAM_NAME_SIZE-strlen(the_program_name) );

} /* set_program_version */


/*! @}  end of debug_public_functions group*/

