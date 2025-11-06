#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "coroutines.h"
#include "scheduler.h"
#include "util.h"

#include <errno.h>
#include <unistd.h>

/* size of each stack */
#define STACK_SIZE_FULL 4096

/* number of stacks */
#define STACKS 4

/* number of threads (one is reserved for the scheduler) */
#define THREAD_LIMIT (STACKS - 1)

/* the type of one stack */
typedef char my_stack_t[STACK_SIZE_FULL];

enum status_t { THREAD_EMPTY = 0, THREAD_READY = 1 };

/* a thread is a coroutine with extra information for the scheduler */
struct thread {
  /* thread identifier */
  int id;
  /* pointer where the coroutine context is saved */
  coroutine_t coroutine;
  /* thread state */
  enum status_t status;
};

struct scheduler {
  /* the scheduler sleeps sometimes because we want to interact
   * with the program while it runs and we need to slow it down.
   * we store the delay in microseconds (nb. µs = us).
   */
  int delay_us;
  /* the coroutine associated with the scheduler */
  coroutine_t coroutine;
  /* current thread being executed, this is useful for yield()
   * etc. which are context-dependent. */
  struct thread *current;
  /* preallocate thread structures */
  struct thread threads[THREAD_LIMIT];
};

/* all our available stacks for coroutines */
my_stack_t stacks[STACKS] __attribute__((aligned(4096)));

/* the global scheduler */
static struct scheduler state;

/* initialize coroutine number i */
const coroutine_t init(int i, asyncfn c) {
  /* initialize the associated stack with function c */
  return init_coroutine(stacks[i], STACK_SIZE_FULL, c);
}

/* find an available thread for the coroutine function c and
 * initialize it, returing its id on success and -1 on error. */
int thread_create(asyncfn c) {
  if (c == NULL)
    return -EINVAL;

  for (int i = 0; i < THREAD_LIMIT; i++) {
    struct thread *t = &state.threads[i];
    if (t->status == THREAD_EMPTY) {
      print_str("scheduler: create ");
      print_int(i);
      newline();
      t->id = i;
      t->status = THREAD_READY;
      t->coroutine = init(i, c);
      mprotect(stacks[i], STACK_SIZE_FULL, PROT_NONE);
      return i;
    }
  }
  /* ENOMEM is defined in errno.h and means "no memory"; we use
   * negative error values by convention */
  return -ENOMEM;
}

/* round-robin scheduling of threads */
void scheduler(void) {
  while (1) {
    for (int i = 0; i < THREAD_LIMIT; ++i) {
      struct thread *t = &state.threads[i];
      state.current = t;
      if (t->status == THREAD_READY) {
        coroutine_t c = t->coroutine;
        mprotect(stacks[t->id], STACK_SIZE_FULL,
                 PROT_READ + PROT_WRITE + PROT_EXEC);
        switch_coroutine(&state.coroutine, c);
        /* sleep a little here to avoid having
         * the screen flash with a lot of
         * text */
        if (state.delay_us > 0)
          usleep(state.delay_us);
        mprotect(stacks[t->id], STACK_SIZE_FULL, PROT_NONE);
      }
    }
  }
}

/* give control to the scheduler from a thread */
void yield() {
  /* access the current thread through the global state */
  const struct thread *t = state.current;
  if (t == NULL)
    return;
  /* enter the scheduler coroutine from here */
  switch_coroutine(&t->coroutine, state.coroutine);
}

/* setup the scheduler, the delay in miliseconds is how much time is
 * spent sleeping between each thread. */
void init_scheduler(int delay_ms) {
  state.delay_us = (delay_ms > 0) ? (delay_ms * 1000) : -1;
  state.coroutine =
      init_coroutine(stacks[THREAD_LIMIT], STACK_SIZE_FULL, scheduler);
  state.current = NULL;
}

/* enter the scheduler coroutine */
void start_scheduler() { enter_coroutine(state.coroutine); }

void print_adress() {
  for (int i = 0; i < STACKS; i++) {
    printf("Adress stack %d : %p (adress mod 4096 : %ld)\n", i, stacks[i],
           (__uint64_t)stacks[i] % 4096);
  }
}

void modify_stack() { *stacks[2] = 1; }

int get_current_thread() { return state.current->id; }