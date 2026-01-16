#ifndef __COMMON_THREAD_POOL_LOCK__
#define __COMMON_THREAD_POOL_LOCK__

#include "thread.h"
#include "spin.h"

typedef struct {
    SpinLock _spin;
    Thread* waiters;
    Thread* holder;
} Lock;

void thpl_lock_init(Lock*);
void thpl_lock_enter(Lock*, Thread*);
void thpl_lock_exit(Lock*);
int thpl_lock_try(Lock*, Thread*);

#endif
