//
// Copyright 2006, Threshold Corporation, All rights reserved.
//

/*!
    \file rf_shm.h
    \brief Header file for shm_rf function

*/

#ifndef __RF_SHM_H__
#define __RF_SHM_H__

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#include "one_net.h"
#include "one_net_types.h"

//==============================================================================
//								CONSTANTS 

enum
{
    RFC_SEM,
    TICK_SEM,
    NUM_SEMS        //must remain at the bottom
};

//								CONSTANTS END
//==============================================================================


//==============================================================================
//								TYPEDEFS 

//								TYPEDEFS END
//==============================================================================


//==============================================================================
//							PUBLIC VARIABLES 

extern tick_t * global_tick_ptr;
extern tick_t * timestamp_ptr;
extern UInt8 * channel_num_ptr;
extern UInt8 * data_rate_ptr;
extern UInt16 * rf_xmtr_ptr;
extern UInt8 * rf_channel_ptr;
extern int sem_id;

//							PUBLIC VARIABLES END
//==============================================================================


//==============================================================================
//						PUBLIC FUNCTION DECLARATIONS 

//shm functions
int init_shm(void);
void clear_rfc(void);
int detach_shm(void);
int free_shm(void);

//sem functions
void acquire_lock(const int SEM_NUM);
BOOL try_lock(const int SEM_NUM);
void release_lock(const int SEM_NUM);
void get_sem_info(const int SEM_NUM, int * semval, int * semncnt, int * sempid);

//						PUBLIC FUNCTION DECLARATIONS END
//==============================================================================

#endif
