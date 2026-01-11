#ifndef __COMMON_THREAD_POOL_THREAD__
#define __COMMON_THREAD_POOL_THREAD__

#include <stdint.h>

#include "threadpool.h"

typedef enum {
    UNINITIALIZED = 0,
    NOT_STARTED,
    RUNNING,
    IDLE,
    DEAD
} Status;

typedef struct s_thread {
    int64_t magic_num;
    Status status;
    void* sp;
    void* args;
    Task task;
    struct s_thread* next;
    char stack_top[];
} Thread;

#endif

