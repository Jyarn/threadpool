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
} TaskArgs;

struct {
    TaskArgs* next;
    TaskArgs* last;
} uninit_queue;


Thread main_thread = { 0 };
int64_t main_stack_base = 0;

int64_t magic_num;
ThreadId next_tid = 0;


Thread* init_thread(ThreadId, Thread*);
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
    TaskArgs* args = malloc(sizeof(TaskArgs));
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
talloc(void)
{
    char* page = mmap(NULL,
		STACK_SIZE,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS | MAP_STACK,
		-1,
		0
		);
    page += TCB_OFF;
    return (Thread*) page;
}

Thread*
init_thread(ThreadId tid, Thread* new_thread)
{
    new_thread->magic_num = magic_num;
    new_thread->sp = new_thread->stack_top - sizeof(Thread);
    new_thread->tid = tid;
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
        TaskArgs* args = uninit_queue.next;
        uninit_queue.next = uninit_queue.next->next;

        void* task_args = args->args;
        Task task = args->task;

        Thread* new_thread = init_thread(args->tid, talloc());
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
    Thread* from = get_tcb();
    assert(from->next == NULL);

    Thread* to = &main_thread;
    if (idle_queue.next)
    {
        to = idle_queue.next;
        idle_queue.next = idle_queue.next;
    }
    else if (uninit_queue.next)
    {
        TaskArgs* args = uninit_queue.next;
        uninit_queue.next = uninit_queue.next->next;

        // recycle tcb
        to = init_thread(args->tid, from);
        void* task_args = args->args;
        Task task = args->task;

        assert(to->magic_num == magic_num);
        start_task_kill(task_args, task, to->sp);
    }

    assert(to);
    assert(to->magic_num == magic_num);

    // call munmap on the current stack page
    // need to inline because when we free the thread we can't write to the stack
    asm volatile inline (
        "mov %[free], %%rdi\n\t"
        "mov $1, %%rsi\n\t"
        "mov $11, %%rax\n\t"
        "syscall\n\t"
        "cmp $0, %%rax\n\t"
        "jne panic\n\t"
        "mov %[new_stack], %%rdi\n\t"
        "jmp swap_tasks_kill"
        :
        : [free] "r" (from->stack_top - STACK_SIZE), [new_stack] "r" (to->sp)
        : "r11", "rcx", "rax", "rdi", "rsi", "memory"
    );
}

_Noreturn
void
panic(void)
{
    char err_msg[] = "munmap failed :D :D :D\n";
    write(STDOUT_FILENO, err_msg, sizeof(err_msg));
    exit(-1);
}
