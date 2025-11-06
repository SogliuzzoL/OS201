#include <linux/seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BEFORE_Q5 0
#define BUFFER_SIZE 1024

#if BEFORE_Q5
int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  read(0, buffer, BUFFER_SIZE);

  // PLUS operation
  if (buffer[0] == '+') {
    int a, b;
    char result[4];
    sscanf(buffer, "+%d,%d", &a, &b);
    sprintf(result, "%d\n\0", a + b);
    write(1, result, 4);
  }

  // MINUS operation
  else if (buffer[0] == '-') {
    int a, b;
    char result[4];
    sscanf(buffer, "-%d,%d", &a, &b);
    sprintf(result, "%d\n\0", a - b);
    write(1, result, 4);
  }

  // System command execution
  else if (buffer[0] == 'e') {
    // prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
    system(buffer + 1);
  }
}

#else

void run_interpreter(int read_fd, int write_fd) {
  prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);

  char buffer[BUFFER_SIZE];
  read(read_fd, buffer, BUFFER_SIZE);

  // PLUS operation
  if (buffer[0] == '+') {
    int a, b;
    char result[4];
    sscanf(buffer, "+%d,%d", &a, &b);
    sprintf(result, "%d\n\0", a + b);
    write(write_fd, result, 4);
  }

  // MINUS operation
  else if (buffer[0] == '-') {
    int a, b;
    char result[4];
    sscanf(buffer, "-%d,%d", &a, &b);
    sprintf(result, "%d\n\0", a - b);
    write(write_fd, result, 4);
  }

  // System command execution
  else if (buffer[0] == 'e') {
    system(buffer + 1);
  }
}

int main() {
  char buffer[BUFFER_SIZE];
  read(0, buffer, BUFFER_SIZE);

  int pipe_to_child[2];
  pipe(pipe_to_child);

  int pipe_from_child[2];
  pipe(pipe_from_child);

  if (fork() == 0) {
    close(pipe_to_child[1]);
    close(pipe_from_child[0]);
    run_interpreter(pipe_to_child[0], pipe_from_child[1]);
    syscall(SYS_exit, 0);
  } else {
    close(pipe_to_child[0]);
    close(pipe_from_child[1]);

    write(pipe_to_child[1], buffer, BUFFER_SIZE);

    char result[BUFFER_SIZE];
    read(pipe_from_child[0], result, BUFFER_SIZE);
    printf("%s", result);
  }
  return 0;
}

#endif