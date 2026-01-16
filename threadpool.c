#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "context.h"
#include "threadpool.h"
#include "thread.h"
#include "queue.h"


#define STACK_SIZE 1024 * 4
#define TCB_OFF STACK_SIZE - sizeof(Thread)

Queue idle_queue = { 0 };
Thread main_thread = { 0 };
int64_t main_stack_base = 0;
int64_t magic_num;

void swap_thread(Thread*, Thread*);


void
unsuspend_thread(Thread* thread)
{
    assert(thread);
    assert(thpl_self() != thread);
    assert(thread->status == SUSPENDED);
    assert(!thread->next);
    enqueue(&idle_queue, thread);
}

void
suspend_thread(Thread* self, Thread* holder)
{
    assert(self);
    assert(thpl_self() == self);
    assert(self->status == RUNNING);
    assert(!self->next);
    self->status = SUSPENDED;
    swap_thread(self, holder);
}

void
thpl_init(int64_t _magic_num)
{
    assert(!main_stack_base);
    assert(sysconf(_SC_PAGESIZE) == STACK_SIZE);
    assert(sizeof(Thread) % 16 == 0);

    int64_t sp = (int64_t) get_sp();
    main_stack_base = sp & ~(STACK_SIZE - 1);

    main_thread.task = NULL;
    main_thread.args = NULL;
    main_thread.next = NULL;
    main_thread.sp = (void*) sp;
    main_thread.status = RUNNING;
    main_thread.magic_num = _magic_num;
    // it's okay to not set stack_top since we only use that to recycle pages

    magic_num = _magic_num;
}


ThreadPtr
thpl_push(Task task, void* task_args)
{
    assert(((Thread*)thpl_self())->magic_num == magic_num);

    Thread* new_thread = mmap(
        NULL,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0
    ) + TCB_OFF;

    new_thread->magic_num = magic_num;
    new_thread->task = task;
    new_thread->args = task_args;
    new_thread->status = NOT_STARTED;
    new_thread->next = NULL;
    new_thread->sp = new_thread->stack_top - sizeof(Thread);
    assert((int64_t)new_thread->sp % 16 == 0);
    enqueue(&idle_queue, new_thread);
    return new_thread;
}

void
thpl_yield(void)
{
    Thread* from = (Thread*)thpl_self();

    assert(idle_queue.head != from);
    assert(from->status == RUNNING);
    assert(from->magic_num == magic_num);
    Thread* to = NULL;

    if (from == &main_thread) // main calls yield => suspend main til there aren't anymore threads
    {
        if (idle_queue.head == NULL) return;
        to = dequeue(&idle_queue);

        if (to == NULL) return;
    }
    else if (idle_queue.head == NULL) // no more threads to yield to => restore main
    {
        enqueue(&idle_queue, from);
        to = &main_thread;
    }
    else
    {
        to = dequeue(&idle_queue);
        enqueue(&idle_queue, from);
    }

    assert(to->magic_num == magic_num);
    swap_thread(from, to);
}

void
swap_thread(Thread* from, Thread* to)
{
    assert(from->status == RUNNING);
    assert(to->status == NOT_STARTED || to->status == IDLE);

    from->status = IDLE;

    Status status = to->status;
    to->status = RUNNING;

    switch (status) {
        case NOT_STARTED:
           start_task(to->args, to->task, to->sp, &from->sp);
           break;
        case IDLE:
           swap_tasks(to->sp, &from->sp);
           break;
        default:
           assert(6 == 7); // 6 7 !!
    }
}

ThreadPtr
thpl_self(void)
{
    int64_t sp = (int64_t) get_sp();
    int64_t sp_lower = sp & ~(STACK_SIZE - 1);
    if (sp_lower == (int64_t)main_stack_base)
        return &main_thread;
    Thread* r = (void*)(sp_lower + TCB_OFF);
    assert(r->magic_num == magic_num);
    return r;
}

_Noreturn void
kill_task(void)
{
    Thread* next = idle_queue.head ? dequeue(&idle_queue) : &main_thread;

    assert(next);
    assert(next->status == NOT_STARTED || next->status == IDLE);
    Status status = next->status;
    next->status = RUNNING;

    assert(((Thread*)thpl_self())->magic_num == magic_num);
    assert(next->magic_num == magic_num);

    switch (status) {
        case NOT_STARTED:
            start_task_kill(next->args, next->task, next->sp);
            break;
        case IDLE:
            swap_tasks_kill(next->sp);
            break;
        default:
            assert(0);
    }
}
