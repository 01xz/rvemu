#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "rvemu.h"
#include "utils.h"

void machine_load_program(Machine* m, const char* prog) {
  int fd = open(prog, O_RDONLY);
  if (fd == -1) {
    FATAL(strerror(errno));
  }

  mmu_load_elf(&m->mmu, fd);
  close(fd);

  m->state.pc = (u64)m->mmu.entry;
}
