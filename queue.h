#ifndef __COMMON_THREAD_POOL_QUEUE__
#define __COMMON_THREAD_POOL_QUEUE__

#include "thread.h"

typedef struct {
    Thread* head;
    Thread* last;
} Queue;

void enqueue(Queue*, Thread*);
Thread* dequeue(Queue*);

#endif
