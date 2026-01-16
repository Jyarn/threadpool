.global thpl_spin_enter
.global thpl_spin_exit

.section: .text

thpl_spin_try:
    xor %rax, %rax
    xchg %rax, (%rdi)
    ret

thpl_spin_enter:
    xor %r12, %r12
    xchg %r12, (%rdi)
    cmp $0, %r12
    je thpl_spin_enter
    ret

thpl_spin_exit:
    movq $1, (%rdi)
    ret
