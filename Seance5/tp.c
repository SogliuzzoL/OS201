#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "scheduler.h"
#include "util.h"

struct {
  char last_char;
  enum { QUEUE_EMPTY = 0, QUEUE_FULL } queue_state;
} task;

void clear() {
  while (getchar() != EOF) {
  }
}

void producer(void) {
  for (;;) {
    yield();
    if (task.queue_state == QUEUE_FULL) {
      clear();
      continue;
    }
    int c = getchar();
    clear();
    if (c == EOF) {
      continue;
    }
    task.queue_state = QUEUE_FULL;
    task.last_char = (char)c;
    println("%%% QUEUE FULL");
  }
}

void consumer(const char n) {
  for (;;) {
    yield();
    if (task.queue_state == QUEUE_EMPTY)
      continue;
    char c = task.last_char;
    if (c > 20) {
      c = 20;
    }
    task.queue_state = QUEUE_EMPTY;
    println("%%% QUEUE EMPTY");

    /* this is exactly the same as "(c--) > 0", but
       written in a way that looks like an arrow */
    while (c-- > 0) {
      putchar(n);
      putchar(' ');
      print_int((int)c);
      putchar('\n');
      yield();
    }
  }
}

void consumer_a(void) { consumer('A'); }
void consumer_b(void) { consumer('B'); }

void check(int ret) {
  if (ret < 0) {
    println("ERROR: ");
    print_str(strerror(-ret));
    newline();
    exit(-1);
  }
}

void init_terminal(int echo) {
  println("setup terminal");
  struct termios settings;
  fcntl(0, F_SETFL, O_NONBLOCK);
  tcgetattr(0, &settings);                 /* grab old terminal i/o settings */
  settings.c_lflag &= ~ICANON;             /* disable buffered i/o */
  settings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &settings);
}

int main() {
  print_adress();
  init_terminal(0);
  println("- INIT");
  init_scheduler(150);
  println("- PRODUCER");
  check(thread_create(producer));
  println("- CONSUMERS");
  check(thread_create(consumer_a));
  check(thread_create(consumer_b));
  println("- RUN");
  start_scheduler();
}
