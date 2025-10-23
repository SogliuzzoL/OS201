#pragma once

#include <stddef.h>

/* coroutines are pointers to the top address of a stack, on top of
   which some context was saved. But here we just say that is is a
   pointer of indeterminate type. */
typedef void *coroutine_t;

/* a coroutine function (cofn) is a function that accepts no argument
   and produces no output */
typedef void (*cofn)(void);

/* load context from stack and resume execution in coroutine */
void enter_coroutine(coroutine_t cr);

/* p_from points to a variable "from" that holds the address of a
   coroutine. switch_coroutine saves the instruction pointer and
   registers on to of the "from" stack and also update the "from"
   variable so that it nows point to a new position in the stack.
   then, it enters coroutine "to" */
void switch_coroutine(const coroutine_t *p_from, const coroutine_t to);

/* initialize a coroutine */
const coroutine_t init_coroutine(const coroutine_t stack_begin,
                                 const size_t stack_size,
                                 const cofn initial_pc);
