#ifndef __COMMON_THREAD_POOL_THREAD__
#define __COMMON_THREAD_POOL_THREAD__

#include <stdint.h>

#include "threadpool.h"

typedef struct s_thread {
    int64_t magic_num;
    void* sp;
    ThreadId tid;
    struct s_thread* next;
    char stack_top[];
} Thread;

void unsuspend_thread(Thread*);
void suspend_thread(Thread*, Thread*);

Thread* get_tcb(void);

#endif
