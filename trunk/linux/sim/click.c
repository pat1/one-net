//
// Copyright 2005,2006 Threshold Corporation, All rights reserved.
//
/*!
   \file click.c
   \brief  Implementation file for click program

   Click program
    
   \author  Adam Meadows
   \date $Date: 2006/10/07 08:37:39 $
   \version $Rev: 100 $
   \note Threshold Corporation
   
*/ 

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <sched.h>
#include "one_net_types.h"
#include "rf_shm.h"
#include "th_time.h"

//==============================================================================
//                                  CONSTANTS

enum
{
    STDIN = 1,
    MAX_CMD_LEN = 4
};

//Commands
const char * const GO_CMD = "go";           //!< Start time
const char * const STOP_CMD = "stop";       //!< Stop time
const char * const STEP_CMD = "step";       //!< Increment time
const char * const SEM_CMD = "sem";         //!< Get semaphore info
const char * const EXIT_CMD = "exit";       //!< Exit program

//prompt
const char * const PROMPT = "click> ";      //!< prompt for command entry

//                                  CONSTANTS END
//==============================================================================

//==============================================================================
//                                  TYPEDEFS

//                                  TYPEDEFS END
//==============================================================================


//==============================================================================
//                                  PRIVATE VARIABLES

//flags
static BOOL go = FALSE;                 //!< Flag for if time going
static BOOL step = FALSE;               //!< Flag for stepping 

//tick increment
static UInt32 usecs_per_tick = 1000;    //!< usecs per tick (default of 1000)


//                                  PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                              PRIVATE FUNCTION DECLARATIONS

static void inc_global_tick(void);
static void usage(void);

//                              PRIVATE FUNCTION DECLARATIONS END
//==============================================================================


//==============================================================================
//                              PRIVATE FUNCTION IMPLEMENTATION


/*!
    \breif main function for click program

    \param[in] argc argument count
    \param[in] argv, array of command line arguments

    \return 0 on success, 1 on error
*/
int main(int argc, char **argv) 
{

    //input variables
    fd_set fds_r, fds_w, fds_e;
    struct timeval timeout = {0, 0};

    //no arguments should be given
    if(argc != 1)
    {
        usage();
        return 1;
    } // if arguments given //

    //initialize shared memory
    init_shm();

    //start off at tick 1
    *global_tick_ptr = 1;

    //print initial prompt
    printf("\n%s", PROMPT);
    fflush(NULL);

    //loop until told to exit
    while(1)
    {
        char cmd_buffer[MAX_CMD_LEN+2] = {'\0'};  // 1 for \n, one for \0
        
        //check for input
        FD_SET(STDIN, &fds_r);
        select(STDIN+1, &fds_r, &fds_w, &fds_e, &timeout);
        if(FD_ISSET(STDIN, &fds_r))
        {
            //read in data
            read(STDIN, cmd_buffer, MAX_CMD_LEN+1);
       
            //process command
            if(strncmp(cmd_buffer, GO_CMD, strlen(GO_CMD)) == 0)
            {
                go = TRUE;
                printf(" GO: tick = [%u]\n", *global_tick_ptr);
            } // GO_CMD //
            else if(strncmp(cmd_buffer, STOP_CMD, strlen(STOP_CMD)) == 0)
            {
                printf(" STOP: tick = [%u]\n", *global_tick_ptr);
                go = FALSE;
            } // STOP_CMD //
            else if(strncmp(cmd_buffer, STEP_CMD, strlen(STEP_CMD)) == 0)
            {
                printf(" STEP: tick = [%u]\n", *global_tick_ptr);
                step = TRUE;
            } // STEP_CMD //
            else if(strncmp(cmd_buffer, SEM_CMD, strlen(SEM_CMD)) == 0)
            {
                int semval = -1, semncnt = -1, sempid = -1;
                get_sem_info(RFC_SEM, &semval, &semncnt, &sempid);
                printf("RFC_SEM info:\n\tvalue = [%d]\n\twaiting = [%d]\n"
                  "\tpid = [%d]\n", semval, semncnt, sempid);
            } // STEP_CMD //
            else if(strncmp(cmd_buffer, EXIT_CMD, strlen(EXIT_CMD)) == 0)
            {
                if(detach_shm() != 0)
                {
                    printf("ERROR: could not detach SHM!\n");
                    fflush(NULL);
                } // if detach fails //
                if(free_shm() != 0)
                {
                    printf("ERROR: could not free SHM!\n");
                    fflush(NULL);
                } // if free fails //
                return 0;
            } // EXIT_CMD //
            else
            {
                printf("\nUnknown command: [%s]\n", cmd_buffer);
            } // INVALID CMD //
      
            //print prompt again
            printf("\n%s", PROMPT);
            fflush(NULL);

        } // if text entered //

        //update time if appropriate
        if(go || step)
        {
            step = FALSE;
            inc_global_tick();
        } // if update time //
    } // while 1 //

    return 0;
}// main //


/*!
    \brief Increment global_tick count

    \param void
    
    \return void
*/
static void inc_global_tick(void)
{
    const struct timespec req = {0, usecs_per_tick*1000};
    nanosleep(&req, NULL);

    //TBD: maybe a print_debug here?
    acquire_lock(TICK_SEM);
    (*global_tick_ptr)++;
    release_lock(TICK_SEM);

} // inc_global_tick //


/*!
    \brief Display usage info

    \param void
    
    \return void
*/
static void usage(void)
{
    printf("\nusage:\n"
      "\tclick [DEBUG FILE]\n\n"
      "\t\tDEBUG FILE \t- optional debug filename\n\n");

} // inc_global_tick //


//                              PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================


