.global enter_coroutine
.global switch_coroutine

enter_coroutine:
        /* %rdi is the register where the first argument of
        /* enter_coroutine is given. %rsp is a register that holds the
        address of the stack. Calling mov %rdi,%rsp changes the
        current value of the stack pointer to the one given as input.
        That means that we changed the current context of our code and
        it will access the stack dedicated to the coroutine, in
        whichever position it currently is set. At this position, we
        expect the coroutine to have a set of contextual values
        pushed, so we pop them from that stack to restore the
        registers as they were when we last left the coroutine. */
        mov %rdi,%rsp
        pop %r15
        pop %r14
        pop %r13
        pop %r12
        pop %rbx
        pop %rbp
        /* ret performs a pop and make the instruction pointer jump to
        /* the address it accessed. When we saved the coroutine we
        also saved an instruction pointer on the stack. Now we jump
        back there.*/
        ret

switch_coroutine:
        /* save all registers on the stack */
        push %rbp
        push %rbx
        push %r12
        push %r13
        push %r14
        push %r15
        /* The first argument of switch_coroutine is a pointer to the
         variable holding the coroutine. The mov %rsp,(%rdi) syntax
         tells to modify the variable pointed by the address stored in
         %rdi. We store the current stack pointer there, which means
         we updated the variable that holds the coroutine so that it
         points on the top of the stack now. */
        mov %rsp,(%rdi)
        /* Since we are going to call enter_coroutine, which expects
        /* one argument in %rdi, we have to fill %rdi with the
        /* appropriate value. We set it to the top of the corountine
        /* stack where we want to jump. */
        mov %rsi,%rdi
        jmp enter_coroutine

/* Tell LD we don't need an executable stack here */
.section .note.GNU-stack,"",@progbits
