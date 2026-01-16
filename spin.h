#ifndef __COMMON_THREAD_POOL_SPIN__
#define __COMMON_THREAD_POOL_SPIN__

#include <stdint.h>

typedef uint_fast8_t SpinLock;

void thpl_spin_init(SpinLock*);
void thpl_spin_enter(SpinLock*);
void thpl_spin_exit(SpinLock*);
int thpl_spin_try(SpinLock*);

#endif
