#include "coroutines.h"
#include <stdio.h>

/* Initialise la pile et renvoie une coroutine telle que, lorsqu’on entrera
   dedans, elle commencera à s’exécuter à l’adresse initial_pc. */
const coroutine_t init_coroutine(const coroutine_t stack_begin,
                                 const size_t stack_size,
                                 const cofn initial_pc) {
  const char *stack_end = ((char *)stack_begin) + stack_size;
  printf("stack: %p --> %p (%zu)\n", stack_begin, stack_end, stack_size);
  void **ptr = (void *)stack_end;
  *(--ptr) = initial_pc;
  *(--ptr) = 0 /* rbp */;
  *(--ptr) = 0 /* rbx */;
  *(--ptr) = 0 /* r12 */;
  *(--ptr) = 0 /* r13 */;
  *(--ptr) = 0 /* r14 */;
  *(--ptr) = 0 /* r15 */;
  return (coroutine_t)ptr;
}
