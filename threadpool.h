#ifndef __COMMON_THREAD_POOL__
#define __COMMON_THREAD_POOL__

#include <stdint.h>

typedef void* ThreadPtr;
typedef void(*Task)(void*);
typedef uint64_t ThreadId;

ThreadId thpl_push(Task, void*);

void thpl_yield(void);
void thpl_init(int64_t);


#endif
