#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "rvemu.h"
#include "utils.h"

ExitReason machine_step(Machine* m) {
  while (true) {
    exec_block_interp(&m->state);
    if (m->state.exit_reason == kDirectBranch ||
        m->state.exit_reason == kIndirectBranch) {
      continue;  // JIT
    }
    break;
  }
  assert(m->state.exit_reason == kSysCall);
  return kSysCall;
}

void machine_load_program(Machine* m, const char* prog) {
  int fd = open(prog, O_RDONLY);
  if (fd == -1) {
    fatal(strerror(errno));
  }

  mmu_load_elf(&m->mmu, fd);
  close(fd);

  m->state.pc = (u64)m->mmu.entry;
}
