#define BUFFER_SIZE 1024

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  while (1) {
    char buffer[BUFFER_SIZE];
    char *result = fgets(buffer, BUFFER_SIZE, stdin);
    if (result == NULL) {
      break;
    }

    for (int i = 0; buffer[i]; i++)
      buffer[i] = toupper(buffer[i]);

    char hostname[BUFFER_SIZE];
    gethostname(hostname, BUFFER_SIZE);
    int pid = getpid();

    printf("Hostname: %s, PID: %d, Message: %s", hostname, pid, buffer);
    fflush(stdout);
  }
  return 0;
}