#include <stdio.h>

// print a string char by char using putchar
void print_str(const char *str) {
  while (*str) {
    putchar((int)*str);
    ++str;
  }
}

// print a newline
void newline() { putchar('\n'); }

// print a string followed by a newline
void println(const char *str) {
  print_str(str);
  newline();
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
