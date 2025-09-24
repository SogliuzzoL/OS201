#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct fs_header {
  u_int8_t rom1fs[8];
  u_int32_t full_size;
  u_int32_t checksum;
  char volume_name[];
};

u_int32_t read32(char ptr[4]) {
  return ((u_int32_t)(unsigned char)ptr[0] << 24) |
         ((u_int32_t)(unsigned char)ptr[1] << 16) |
         ((u_int32_t)(unsigned char)ptr[2] << 8) |
         ((u_int32_t)(unsigned char)ptr[3]);
}

int round16(int number) {
  if (number % 16 == 0)
    return number;
  return number + (16 - (number % 16));
}

struct file_header {
  u_int32_t next_filehdr;
  u_int32_t spec_info;
  u_int32_t size;
  u_int32_t checksum;
  char filename[];
};

void ls(struct file_header *fh, struct fs_header *fs) {
  printf("Type\tFilename\n");
  while (fh != NULL) {
    // lire l’offset du header suivant en big endian
    u_int32_t next = read32((char *)&fh->next_filehdr);
    // supprimer les 4 bits de mode
    u_int32_t type = next & 7;
    printf("%u\t%s\n", type, fh->filename);
    next &= ~0xF;

    if (next == 0) {
      // plus de fichiers
      printf("\n");
      break;
    }

    // avancer au header suivant (offset relatif au début du FS)
    fh = (struct file_header *)((char *)fs + next);
  }
}

struct file_header *find(struct file_header *fh, struct fs_header *fs,
                         const char *name) {
  while (fh != NULL) {
    // lire next et type
    u_int32_t raw_next = read32((char *)&fh->next_filehdr);
    u_int32_t offset = raw_next & ~0xF;
    u_int32_t type = raw_next & 0x7;

    // comparer le nom
    if (strcmp(fh->filename, name) == 0) {
      // trouvé, on retourne le header
      return fh;
    }

    // si c'est un répertoire (type == 1)
    // et pas "." ni ".." alors descente récursive
    if (type == 1 && strcmp(fh->filename, ".") != 0 &&
        strcmp(fh->filename, "..") != 0) {

      // calculer l’adresse du premier file_header de ce sous-répertoire
      // on doit sauter le header et le nom padded à 16
      int len = strlen(fh->filename) + 1;
      int nextOffset = round16(16 + len);
      struct file_header *subdir =
          (struct file_header *)((char *)fh + nextOffset);

      struct file_header *res = find(subdir, fs, name);
      if (res != NULL)
        return res; // trouvé en descendant
    }

    // passer au file header suivant
    if (offset == 0)
      break;
    fh = (struct file_header *)((char *)fs + offset);
  }
  return NULL; // rien trouvé
}

void print_file(struct file_header *fh) {
  // taille du fichier (big-endian)
  u_int32_t size = read32((char *)&fh->size);

  int len = strlen(fh->filename) + 1;
  int offsetData = 16 + round16(len);

  char *data = (char *)fh + offsetData;

  // afficher le contenu (non null-terminated)
  fwrite(data, 1, size, stdout);
  printf("\n");
}

void decode(struct fs_header *p, size_t size) {
  assert(memcmp(p->rom1fs, "-rom1fs-", 8) == 0);
  assert(read32((char *)&p->full_size) <= size);

  int sizeVolumeName = strlen(p->volume_name) + 1;
  int offsetFileHeader = 16 + round16(sizeVolumeName);

  struct file_header *root =
      (struct file_header *)((char *)p + offsetFileHeader);

  ls(root, p);

  struct file_header *res = find(root, p, "message.txt");
  if (res != NULL) {
    printf("=== Contenu de %s ===\n", res->filename);
    print_file(res);
  } else {
    printf("message.txt introuvable\n");
  }
}

int main(void) {
  int fd = open("fs.romfs", O_RDONLY);
  assert(fd != -1);

  off_t fsize = lseek(fd, 0, SEEK_END);

  char *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  assert(addr != MAP_FAILED);

  decode((struct fs_header *)addr, fsize);
  return 0;
}