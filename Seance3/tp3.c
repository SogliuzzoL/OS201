#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>  // getchar
#include <stdlib.h> // exit
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

// Pour compiler ce fichier et le linker avec les fonctions assembleur:
// $ gcc -static -Wall tp3-skeleton.c tp3-skeleton.s -o tp3

// Vous pouvez ensuite lancer le programme compilÃ©:
// $ ./tp3

// Sinon "make" devrait suffir à créer l'executable "tp"

// Note: printf in coroutines/threads segfaults on some machines, if that is
// the case on yours, replace the printf's with calls to the handcoded
// print_str and print_int

// print a string char by char using putchar
void print_str(char *str) {
  while (*str) {
    putchar((int)*str);
    ++str;
  }
}

// print a integer in base 10 using putchar
void print_int(int x) {
  if (x < 0) {
    putchar('-');
    x = -x;
  }
  if (x == 0) {
    putchar('0');
    return;
  }
  int pos = 1;
  while (x >= pos) {
    pos *= 10;
  }
  while (pos > 1) {
    pos /= 10;
    putchar('0' + (x / pos) % 10);
  }
}

// The size of our stacks
#define STACK_SIZE_FULL 4096

char stack0[STACK_SIZE_FULL];
char stack1[STACK_SIZE_FULL];
char stack2[STACK_SIZE_FULL];
char stack3[STACK_SIZE_FULL];

typedef void *coroutine_t;

struct thread {
  coroutine_t context;
  bool status;
} typedef thread;

void test0();
void test1();

coroutine_t coroutine0;
coroutine_t coroutine1;
coroutine_t coroutine2;
coroutine_t coroutine3;

coroutine_t current_coroutine;
coroutine_t scheduling_coroutine;
int current_stack = 0;

/* Quitte le contexte courant et charge les registres et la pile de CR. */
void enter_coroutine(coroutine_t cr);

/* Sauvegarde le contexte courant dans p_from, et entre dans TO. */
void switch_coroutine(coroutine_t *p_from, coroutine_t to);

/* Initialise la pile et renvoie une coroutine telle que, lorsqu’on entrera
dedans, elle commencera à s’exécuter à l’adresse initial_pc. */
coroutine_t init_coroutine(void *stack_begin, unsigned int stack_size,
                           void (*initial_pc)(void)) {
  char *stack_end = ((char *)stack_begin) + stack_size;
  void **ptr = (void **)stack_end;
  ptr--;
  *ptr = (void *)initial_pc; // pc
  ptr--;
  *ptr = (void *)0; // rbp
  ptr--;
  *ptr = (void *)0; // rbx
  ptr--;
  *ptr = (void *)0; // r12
  ptr--;
  *ptr = (void *)0; // r13
  ptr--;
  *ptr = (void *)0; // r14
  ptr--;
  *ptr = (void *)0; // r15

  return ptr;
}

void test0() {
  int i = 0;
  while (1) {
    print_str("Coroutine 0: ");
    print_int(i++);
    print_str("\n");
    switch_coroutine(&coroutine0, coroutine1);
  }
}

void test1() {
  int i = 0;
  while (1) {
    print_str("Coroutine 1: ");
    print_int(i++);
    print_str("\n");
    switch_coroutine(&coroutine1, coroutine0);
  }
}

void yield() { switch_coroutine(&current_coroutine, scheduling_coroutine); }

thread thread_create(void (*f)(void)) {
  current_stack++;
  coroutine_t new_coroutine;

  if (current_stack == 1) {
    new_coroutine = init_coroutine(stack1, STACK_SIZE_FULL, f);
  } else if (current_stack == 2) {
    new_coroutine = init_coroutine(stack2, STACK_SIZE_FULL, f);
  } else if (current_stack == 3) {
    new_coroutine = init_coroutine(stack3, STACK_SIZE_FULL, f);
  } else {
    print_str("Max thread number reached\n");
    thread t = {NULL, false};
    return t;
  }

  thread t = {new_coroutine, true};
  return t;
}

void counter() {
  int i = 0;
  while (1) {
    print_str("Counter: ");
    print_int(i++);
    print_str("\n");
    yield();
  }
}

void scheduling() {
  thread t1 = thread_create(counter);
  thread t2 = thread_create(counter);
  thread t3 = thread_create(counter);
  while (1) {
    if (t1.status) {
      current_coroutine = t1.context;
      switch_coroutine(&scheduling_coroutine, t1.context);
      t1.context = current_coroutine;
    }
    if (t2.status) {
      current_coroutine = t2.context;
      switch_coroutine(&scheduling_coroutine, t2.context);
      t2.context = current_coroutine;
    }
    if (t3.status) {
      current_coroutine = t3.context;
      switch_coroutine(&scheduling_coroutine, t3.context);
      t3.context = current_coroutine;
    }
  }
}

int main() {
  //   coroutine0 = init_coroutine(stack0, STACK_SIZE_FULL, test0);
  //   coroutine1 = init_coroutine(stack1, STACK_SIZE_FULL, test1);
  //   enter_coroutine(coroutine0);
  scheduling_coroutine = init_coroutine(stack0, STACK_SIZE_FULL, scheduling);
  enter_coroutine(scheduling_coroutine);
  return 0;
}
