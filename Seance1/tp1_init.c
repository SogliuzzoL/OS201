#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct fs_header {
  uint8_t rom1fs[8];
  uint32_t full_size;
  uint32_t checksum;
  char volume_name[];
};

struct file_header {
  uint32_t next_filehdr;
  uint32_t spec_info;
  uint32_t size;
  uint32_t checksum;
  char filename[];
};

uint32_t read32(char ptr[4]) {
  return ((uint32_t)(unsigned char)ptr[0] << 24) |
         ((uint32_t)(unsigned char)ptr[1] << 16) |
         ((uint32_t)(unsigned char)ptr[2] << 8) |
         ((uint32_t)(unsigned char)ptr[3]);
}

uint32_t multiple_of_16(uint32_t number) {
  if (number % 16 == 0)
    return number;
  return number + (16 - (number % 16));
}

void decode(struct fs_header *p, size_t size) {
  printf("rom1fs: %.8s\n", p->rom1fs);
  printf("full_size: %u\n", read32((char *)&p->full_size));
  printf("checksum: %u\n", read32((char *)&p->checksum));
  printf("volume_name: %s\n", p->volume_name);

  uint8_t file_header_offset = multiple_of_16(16 + strlen(p->volume_name));
  printf("fileHeaderOffset: %u\n", file_header_offset);
}

int main(void) {
  int fd = open("tp1fs.romfs", O_RDONLY);
  assert(fd != -1);
  off_t fsize = lseek(fd, 0, SEEK_END);

  printf("size is %jd\n", (intmax_t)fsize);

  void *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  assert(addr != MAP_FAILED);

  decode(addr, fsize);

  return 0;
}
