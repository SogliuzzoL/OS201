#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define PARTIE2

#ifdef PARTIE1
char heap[16 * 1024];
size_t heap_size = sizeof(heap);

void *memalloc(int size) {
  if (size <= 0 || size > heap_size) {
    return NULL;
  }
  void *ptr = &heap[16 * 1024 - heap_size];
  heap_size -= size;
  return ptr;
}

void memfree(void *) { return; }

int main() {
  char *test = memalloc(10);
  if (test == NULL) {
    printf("Allocation échouée !\n");
    return 1;
  }
  test[0] = 'a';
  test[1] = '\0';
  printf("%s\n", test);

  char *test2 = memalloc(10);
  if (test2 == NULL) {
    printf("Allocation échouée !\n");
    return 1;
  }
  test2[0] = 'b';
  test2[1] = '\0';
  printf("%s\n", test2);

  return 0;
}
#endif
#ifdef PARTIE2
#define HEAP_SIZE (16 * 1024)

char heap[HEAP_SIZE];

typedef struct block {
  size_t size;
  struct block *next;
  bool is_free;
} block;

block *free_list = NULL;

void meminit() {
  free_list = (block *)heap;
  free_list->size = HEAP_SIZE - sizeof(block);
  free_list->next = NULL;
  free_list->is_free = true;
}

void *memalloc(size_t size) {
  if (size <= 0) {
    return NULL;
  }

  block *prev = NULL;
  block *curr = free_list;

  while (curr != NULL) {
    printf("Current pointer : %p, size: %u\n", curr, curr->size);
    if (curr->size >= size) {
      if (curr->size > size + sizeof(block)) {
        block *new_block = (block *)((char *)curr + sizeof(block) + size);
        new_block->size = curr->size - size - sizeof(block);
        new_block->next = curr->next;
        curr->size = size;

        if (prev == NULL) {
          free_list = new_block;
        } else {
          prev->next = new_block;
        }
      } else {
        if (prev == NULL) {
          free_list = curr->next;
        } else {
          prev->next = curr->next;
        }
      }

      curr->is_free = false;
      return (void *)((char *)curr + sizeof(block));
    }
    prev = curr;
    curr = curr->next;
  }

  return NULL;
}

void memfree(void *ptr) {
  if (ptr == NULL)
    return;
  block *block_ptr = (block *)((char *)ptr - sizeof(block));
  block_ptr->is_free = true;
  free_list->next = block_ptr;
  block *curr;
  while (curr->next != NULL) {
    curr = curr->next;
    printf("%p\n", curr);
  }
  curr = block_ptr;
  return;
}

int main() {
  meminit();
  char *test = memalloc(10);
  if (test == NULL) {
    printf("Allocation échouée !\n");
    return 1;
  }
  test[0] = 'a';
  test[1] = '\0';
  printf("%s\n", test);

  char *test2 = memalloc(10);
  if (test2 == NULL) {
    printf("Allocation échouée !\n");
    return 1;
  }
  test2[0] = 'b';
  test2[1] = '\0';
  printf("%s\n", test2);

  memfree(test);
  memfree(test2);

  return 0;
}
#endif