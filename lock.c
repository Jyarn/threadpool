#include <assert.h>
#include <stddef.h>

#include "threadpool.h"
#include "thread.h"
#include "lock.h"
#include "spin.h"

void
thpl_lock_init(Lock* lock)
{
    lock->waiters = NULL;
    lock->holder = NULL;
    thpl_spin_init(&lock->_spin);
}

void
thpl_lock_enter(Lock* lock, Thread* self)
{
    assert(get_tcb() == lock->holder);
    if (thpl_spin_try(&lock->_spin))
    {
        assert(lock->holder);
        assert(lock->holder != self);

        suspend_thread(self, lock->holder);
        self->next = lock->waiters;
        lock->waiters = self;
        thpl_yield();
    }

    assert(lock->holder == NULL);
    lock->holder = self;
}

void
thpl_lock_exit(Lock* lock)
{
    assert(get_tcb() == lock->holder);
    thpl_spin_exit(&lock->_spin);

    if (lock->waiters)
    {
        Thread* t = lock->waiters;
        t->next = NULL;

        lock->waiters = lock->waiters->next;
        unsuspend_thread(t);
    }

    lock->holder = NULL;
}

int
thpl_lock_try(Lock* lock, Thread* self)
{
    int r;
    if ((r = thpl_spin_try(&lock->_spin)))
    {
        assert(!lock->holder);
        lock->holder = self;
    }
    return r;
}
