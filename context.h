#ifndef __COMMON_THREAD_POOL_CONTEXT__
#define __COMMON_THREAD_POOL_CONTEXT__

#include "threadpool.h"

void swap_tasks(void*, void**);
_Noreturn void swap_tasks_kill(void*);

void start_task(void*, Task, void*, void**);
_Noreturn void start_task_kill(void*, Task, void*);

void* get_sp(void);

extern _Noreturn void kill_task(void);

#endif
