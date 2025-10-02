#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Décommenter pour activer la partie 1 (allocation triviale)
#define PARTIE2

#ifdef PARTIE1
// --- Partie 1 : Allocation mémoire triviale ---
char heap[16 * 1024];
size_t heap_size = sizeof(heap);

void *memalloc(int size) {
  if (size <= 0 || size > heap_size) {
    return NULL;
  }
  void *ptr = &heap[sizeof(heap) - heap_size];
  heap_size -= size;
  return ptr;
}

void memfree(void *ptr) {
  // Ne fait rien dans cette version
  return;
}
#else
// --- Partie 2 : Allocation mémoire avec liste chaînée ---
#define HEAP_SIZE (16 * 1024)
char heap[HEAP_SIZE];

typedef struct block {
  size_t size;
  struct block *next;
  int is_free; // 1 si libre, 0 si alloué
} block;

block *free_list = NULL;

void meminit() {
  free_list = (block *)heap;
  free_list->size = HEAP_SIZE - sizeof(block);
  free_list->next = NULL;
  free_list->is_free = 1;
}

void *memalloc(size_t size) {
  if (size <= 0) {
    return NULL;
  }

  block *prev = NULL;
  block *curr = free_list;

  while (curr != NULL) {
    if (curr->is_free && curr->size >= size) {
      // Scinder le bloc si nécessaire
      if (curr->size > size + sizeof(block)) {
        block *new_block = (block *)((char *)curr + sizeof(block) + size);
        new_block->size = curr->size - size - sizeof(block);
        new_block->next = curr->next;
        new_block->is_free = 1;
        curr->size = size;
        curr->is_free = 0;
        if (prev == NULL) {
          free_list = new_block;
        } else {
          prev->next = new_block;
        }
      } else {
        curr->is_free = 0; // Marquer comme alloué
        if (prev == NULL) {
          free_list = curr->next;
        } else {
          prev->next = curr->next;
        }
      }
      return (void *)((char *)curr + sizeof(block));
    }
    prev = curr;
    curr = curr->next;
  }

  printf("Erreur : pas assez de mémoire libre.\n");
  return NULL;
}

void memfree(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  block *block_ptr = (block *)((char *)ptr - sizeof(block));
  block_ptr->is_free = 1;

  // Coalescence : fusionner avec les blocs libres adjacents
  block *curr = free_list;
  block *prev = NULL;

  // Vérifier si le bloc est valide
  if (block_ptr < (block *)heap ||
      block_ptr > (block *)((char *)heap + HEAP_SIZE)) {
    printf("Erreur : pointeur invalide.\n");
    return;
  }

  // Fusionner avec le bloc suivant si libre
  block *next_block =
      (block *)((char *)block_ptr + sizeof(block) + block_ptr->size);
  if (next_block < (block *)((char *)heap + HEAP_SIZE) && next_block->is_free) {
    block_ptr->size += sizeof(block) + next_block->size;
    block_ptr->next = next_block->next;
  }

  // Fusionner avec le bloc précédent si libre
  block *prev_block = NULL;
  curr = free_list;
  prev = NULL;
  while (curr != NULL && curr < block_ptr) {
    prev = curr;
    curr = curr->next;
  }

  if (prev != NULL && prev->is_free &&
      (char *)prev + sizeof(block) + prev->size == (char *)block_ptr) {
    prev->size += sizeof(block) + block_ptr->size;
    prev->next = block_ptr->next;
  } else {
    // Réinsérer le bloc libéré dans la free list
    block_ptr->next = free_list;
    free_list = block_ptr;
  }
}
#endif

// --- Programme principal ---
int main() {
#ifdef PARTIE1
  // Test de la partie 1
  void *ptr1 = memalloc(100);
  void *ptr2 = memalloc(200);
  if (ptr1 != NULL && ptr2 != NULL) {
    printf("Allocations réussies en partie 1.\n");
  }
  memfree(ptr1);
#else
  // Test de la partie 2
  meminit();

  // Allouer et libérer des blocs
  char *test1 = memalloc(100);
  char *test2 = memalloc(200);
  char *test3 = memalloc(50);

  if (test1 != NULL) {
    test1[0] = 'a';
    test1[1] = '\0';
    printf("%s\n", test1);
  }

  if (test2 != NULL) {
    test2[0] = 'b';
    test2[1] = '\0';
    printf("%s\n", test2);
  }

  if (test3 != NULL) {
    test3[0] = 'c';
    test3[1] = '\0';
    printf("%s\n", test3);
  }

  memfree(test1);
  memfree(test3);

  char *test4 = memalloc(150);
  if (test4 != NULL) {
    test4[0] = 'd';
    test4[1] = '\0';
    printf("%s\n", test4);
  }
#endif

  return 0;
}
