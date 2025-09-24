#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void decode(struct fs_header *p, size_t size) {
  // Décodage à implémenter
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