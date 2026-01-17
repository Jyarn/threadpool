#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "context.h"
#include "threadpool.h"
#include "thread.h"


#define STACK_SIZE 1024 * 4
#define TCB_OFF STACK_SIZE - sizeof(Thread)

struct {
    Thread* next;
    Thread* last;
} idle_queue;

typedef struct s_args {
    void* args;
    Task task;
    ThreadId tid;
    struct s_args* next;
} Args;

struct {
    Args* next;
    Args* last;
} uninit_queue;


Thread main_thread = { 0 };
int64_t main_stack_base = 0;

int64_t magic_num;
ThreadId next_tid = 0;



Thread* init_thread(Args*);
void swap_threads(Thread*, Thread*);


void
unsuspend_thread(Thread* thread)
{
    assert(thread);
    assert(get_tcb() != thread);
    assert(!thread->next);

    if (!idle_queue.next)
        idle_queue.next = thread;
    else
        idle_queue.last->next = thread;

    idle_queue.last = thread;
}

void
suspend_thread(Thread* self, Thread* holder)
{
    assert(self);
    assert(get_tcb() == self);
    assert(self->next);

    swap_tasks(holder->sp, &self->sp);
}

void
thpl_init(int64_t _magic_num)
{
    assert(!main_stack_base);
    assert(sysconf(_SC_PAGESIZE) == STACK_SIZE);
    assert(sizeof(Thread) % 16 == 0);

    int64_t sp = (int64_t) get_sp();
    main_stack_base = sp & ~(STACK_SIZE - 1);

    main_thread.next = NULL;
    main_thread.sp = (void*) sp;
    main_thread.magic_num = _magic_num;
    // it's okay to not set stack_top since we only use that to recycle pages

    magic_num = _magic_num;
}

ThreadId
thpl_push(Task task, void* task_args)
{
    assert(get_tcb()->magic_num == magic_num);
    Args* args = malloc(sizeof(Args));
    args->args = task_args;
    args->task = task;
    args->tid = next_tid++;

    // Save memory by pushing the bare minimum to the queue
    if (!uninit_queue.next)
        uninit_queue.next = args;
    else
        uninit_queue.last->next = args;

    uninit_queue.last = args;
    return args->tid;
}

Thread*
init_thread(Args* args)
{
    assert(args != NULL);
    Thread* new_thread = mmap(
            NULL,
            STACK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS | MAP_STACK,
            -1,
            0
            ) + TCB_OFF;

    new_thread->magic_num = magic_num;
    new_thread->sp = new_thread->stack_top - sizeof(Thread);
    new_thread->tid = args->tid;
    new_thread->next = NULL;
    return new_thread;
}

void
thpl_yield(void)
{
    Thread* from = get_tcb();
    Thread* to = &main_thread;
    assert(from->next == NULL);

    if (idle_queue.next)
    {
        to = idle_queue.next;
        idle_queue.next = idle_queue.next->next;
    }
    else if (uninit_queue.next)
    {
        Args* args = uninit_queue.next;
        uninit_queue.next = uninit_queue.next->next;

        void* task_args = args->args;
        Task task = args->task;
        Thread* new_thread = init_thread(args);
        free(args);

        assert(new_thread->magic_num == magic_num);
        start_task(task_args, task, new_thread->sp, &from->sp);
        return;
    }

    if (to == from) return;

    if (from == &main_thread)
    {
        if (!idle_queue.next)
            idle_queue.next = from;
        else
            idle_queue.last->next = from;

        idle_queue.last = from;
    }

_skip_enqueue:
    assert(to);
    assert(to->next == NULL);
    assert(to->magic_num == magic_num);
    to->next = NULL;
    swap_tasks(to->sp, &from->sp);
}

Thread*
get_tcb(void)
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
    Thread* to = &main_thread;
    if (idle_queue.next)
    {
        to = idle_queue.next;
        idle_queue.next = idle_queue.next;
    }
    else if (uninit_queue.next)
    {
        Args* args = uninit_queue.next;
        uninit_queue.next = uninit_queue.next->next;

        to = init_thread(args);

        void* task_args = args->args;
        Task task = args->task;

        assert(to->magic_num == magic_num);
        start_task_kill(task_args, task, to->sp);
    }

    assert(to);
    assert(to->magic_num == magic_num);
    swap_tasks_kill(to->sp);
}
