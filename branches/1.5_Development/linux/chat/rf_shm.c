//
// Copyright 2006, Threshold Corporation, All rights reserved.
//

/*!
    \file rf_shm.c
    \brief initialization of shared memory for RF simulation on FC5

    This file contains the functions that deal with the shared
    memory for the FC5 simulation of an RF channel for use by ONE-NET devices
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "one_net.h"            //for MAX_ENCODED_PKT_SIZE
#include "one_net_types.h"
#include "th_time.h"
#include "rf_shm.h"

//==============================================================================
//                              CONSTANTS

//! Enumeration of various constants for shared memory
enum
{
    PROJ_ID = 12345,
    SHM_CREAT_FLG = IPC_CREAT | IPC_EXCL | 0666,
    SHM_OPEN_FLG = 0666,
    SHM_BUFFER_SIZE = ONE_NET_MAX_ENCODED_PKT_LEN + 2 * sizeof(time_t)
      + 2 * sizeof(UInt8) + sizeof(on_encoded_did_t)
};

static const char * const FTOK_FILENAME = "/tmp/one_net_fc5";

static struct sembuf SHM_LOCK_RFC_SEMBUF = { RFC_SEM, -1, 0 };
static struct sembuf SHM_UNLOCK_RFC_SEMBUF = { RFC_SEM, 1, 0 };
static struct sembuf SHM_LOCK_TICK_SEMBUF = { TICK_SEM, -1, 0 };
static struct sembuf SHM_UNLOCK_TICK_SEMBUF = { TICK_SEM, 1, 0 };

//                              CONSTANTS END
//==============================================================================


//==============================================================================
//                              TYPEDEFS

//                              TYPEDEFS END
//==============================================================================


//==============================================================================
//                              PRIVATE VARIABLES

static void * shm_ptr = NULL;
static int shm_id = 0;

// Not really "Private", externed in header
tick_t * global_tick_ptr = NULL;    //!< Ptr to shared tick count
UInt8 * channel_num_ptr = NULL;     //!< Ptr to the channel number
UInt8 * rf_channel_ptr = NULL;      //!< Ptr to RF Channel (RFC) buffer
UInt8 * data_rate_ptr = NULL;       //!< The data rate the packet was sent at
UInt8 * rf_xmtr_ptr = NULL;         //!< did of device that actually xmted pkt
tick_t * timestamp_ptr = NULL;      //!< Ptr to RFC timestamp 
int sem_id = 0;                     //!< ID of shared semaphore
    

//                              PRIVATE VARIABLES END
//==============================================================================


//==============================================================================
//                          PRIVATE FUNCTION DECLARATIONS

//                          PRIVATE FUNCTION DECLARATIONS END
//==============================================================================



//==============================================================================
//                          PUBLIC FUNCTION IMPLEMENTATION

/*!
    \brief Initialize shared memory buffer

    Set up shared memory buffer to hold global tick count, simulated RF channel
    and timestamp for data written to that channel

    \param void
    
    \return 0 on success, -1 on failure
*/
int init_shm(void)
{
    FILE * ftok_file_fp = NULL;
    key_t ftok_key = 0;
    BOOL clear_buff = FALSE;

    //make sure ftok file exists
    ftok_file_fp = fopen(FTOK_FILENAME, "w+");
    fclose(ftok_file_fp);

    //get key
    ftok_key = ftok(FTOK_FILENAME, PROJ_ID);

    //get semaphore id for shared semaphore
    sem_id = semget(ftok_key, NUM_SEMS, IPC_CREAT | 0666);

    //get shared memory (first try create)
    if((shm_id = shmget(ftok_key, SHM_BUFFER_SIZE, SHM_CREAT_FLG)) != -1)
    {
        UInt8 i = 0;
        //if creating, initialize semaphore and buffer
        for(i = 0; i < NUM_SEMS; i ++)
        {
            semctl(sem_id, i, SETVAL, 1);
        } // init all sems //
        clear_buff = TRUE;
   
    } // if ceate worked //
    else if((shm_id = shmget(ftok_key, SHM_BUFFER_SIZE, SHM_OPEN_FLG)) != -1)
    {
        //nothing to initialize
    } // else if open worked //
    else
    {
        printf("ERROR: couldn't get shared memory\n");
        fflush(NULL);
        return -1;
    } // else shmget failed //

    //attach shared memory
    shm_ptr = shmat(shm_id, NULL, 0);
    if((int)shm_ptr == -1)
    {
        printf("ERROR: couldn't attach shared memory\n");
        fflush(NULL);
        return -1;
    } // if attach failed //
    
    //clear out buffer
    if(clear_buff)
    {
        bzero(shm_ptr, SHM_BUFFER_SIZE);
    }

    //initialize tick, timestamp & rfc pointers
    global_tick_ptr = ((tick_t *)shm_ptr);
    timestamp_ptr = global_tick_ptr + sizeof(tick_t);
    channel_num_ptr = ((UInt8 * )timestamp_ptr) + sizeof(tick_t);
    data_rate_ptr = channel_num_ptr + sizeof(UInt8);
    rf_xmtr_ptr = data_rate_ptr + sizeof(UInt8);
    rf_channel_ptr = rf_xmtr_ptr + sizeof(on_encoded_did_t);

    return 0;
} // init_shm //


/*!
    \brief Clears out RF channel buffer (including timestamp)

    This function zero's out the RF channel buffer as well as the
    timestamp filed for that buffer.

    \param void

    \return void
*/
void clear_rfc(void)
{
    //first clear timestamp
    bzero((void *) timestamp_ptr, sizeof(tick_t));

    //then clear channel
    bzero((void *) rf_channel_ptr, ONE_NET_MAX_ENCODED_PKT_LEN);
} // clear_rfc //


/*!
    \brief Detach shared memory segment

    \param void

    \return 0 on success, -1 on failure
*/
int detach_shm(void)
{
    return shmdt(shm_ptr); 
} // detach_shm //


/*!
    \brief free shared memory

    \param void

    \return 0 on success, -1 on failure
*/
int free_shm(void)
{
    //first free semaphore set
    semctl(sem_id, 0, IPC_RMID);

    //then free memory
    return shmctl(shm_id, IPC_RMID, NULL);
} // free_shm //


/*!
    \brief Release lock on shared semaphore

    \param[in] SEM_NUM: number of semaphore to release 

    \return void
*/
void release_lock(const int SEM_NUM)
{
    struct sembuf *sem_buf_ptr;
    switch(SEM_NUM)
    {
        case RFC_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_UNLOCK_RFC_SEMBUF;
            break;
        } // RFC_SEM //

        case TICK_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_UNLOCK_TICK_SEMBUF;
            break;
        } // TICK_SEM //
    } // switch SEM_NUM //

    //release it
    if(semop(sem_id, sem_buf_ptr, 1) == -1)
    {
        printf("Couldn't release lock\n");
        fflush(NULL);
        exit(1);
    } // if release lock fails //
} // release_lock //


/*!
    \brief Release lock on shared semaphore

    \param[in] SEM_NUM: number of semaphore to acquire

    \return void
*/
void acquire_lock(const int SEM_NUM)
{
    struct sembuf *sem_buf_ptr;
    switch(SEM_NUM)
    {
        case RFC_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_LOCK_RFC_SEMBUF;
            break;
        } // RFC_SEM //

        case TICK_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_LOCK_TICK_SEMBUF;
            break;
        } // TICK_SEM //
    } // switch SEM_NUM //

    sem_buf_ptr->sem_flg &= ~IPC_NOWAIT;
    if(semop(sem_id, sem_buf_ptr, 1) == -1)
    {
        printf("Couldn't acquire lock\n");
        fflush(NULL);
        exit(1);
    } // if acquire lock fails //
} // acquire_lock //


/*!
    \brief Tries to acquire the lock.

    Does not block if it can not aquire the lock.

    \param[in] SEM_NUM: number of semaphore to acquire

    \return TRUE If the lock was acquired.
            FALSE If the semaphore was locked by another process.
*/
BOOL try_lock(const int SEM_NUM)
{
    struct sembuf * sem_buf_ptr;

    switch(SEM_NUM)
    {
        case RFC_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_LOCK_RFC_SEMBUF;
            break;
        } // RFC_SEM //

        case TICK_SEM:
        {
            sem_buf_ptr = (struct sembuf *)&SHM_LOCK_TICK_SEMBUF;
            break;
        } // TICK_SEM //
    } // switch SEM_NUM //

    sem_buf_ptr->sem_flg |= IPC_NOWAIT;
    return (BOOL)(semop(sem_id, sem_buf_ptr, 1) == 0);
} // try_lock //


/*!
    \brief Get info of shared semaphore

    \param[in] SEM_NUM: number of semaphore to get info for
    \param[out] semval: value of the semaphore
    \param[out] semncnt: # of processes waiting for increase of semval
    \param[out] sempid: PID of last process to use semaphore

    \return void
*/
void get_sem_info(const int SEM_NUM, int * semval, int * semncnt, int * sempid)
{
    // make sure nothing is null
    assert(semval != NULL);
    assert(semncnt != NULL);
    assert(sempid != NULL);

    //get semval
    *semval = semctl(sem_id, SEM_NUM, GETVAL);

    //get semncnt
    *semncnt = semctl(sem_id, SEM_NUM, GETNCNT);

    //get sempid
    *sempid = semctl(sem_id, SEM_NUM, GETPID);

} // get_sem_info //

//                          PUBLIC FUNCTION IMPLEMENTATION END
//==============================================================================

//==============================================================================
//                          PRIVATE FUNCTION IMPLEMENTATION

//                          PRIVATE FUNCTION IMPLEMENTATION END
//==============================================================================

