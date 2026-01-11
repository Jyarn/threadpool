.global swap_tasks
.global swap_tasks_kill

.global start_task
.global start_task_kill

.global kill_task
.global get_sp

.section: .text

// void swap_tasks(void* sp_to, void** sp_from)
swap_tasks:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp, (%rsi)
swap_tasks_kill:
    mov %rdi, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    ret

// void start_task(void* args, Task task, void* sp_to, void** sp_from)
start_task:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp, (%rcx)
start_task_kill:
    mov %rdx, %rsp
    call *%rsi
    call kill_task

get_sp:
    mov %rsp, %rax
    ret
