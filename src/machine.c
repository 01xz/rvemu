#include "machine.h"

ExitReason machine_step(Machine* m) {
  while (true) {
    m->state.exit_reason = kNone;
    exec_block_interp(&m->state);

    assert(m->state.exit_reason != kNone);
    if (m->state.exit_reason == kDirectBranch ||
        m->state.exit_reason == kIndirectBranch) {
      m->state.pc = m->state.re_enter_pc;
      continue;  // JIT
    }
    break;
  }

  m->state.pc = m->state.re_enter_pc;
  assert(m->state.exit_reason == kECall);
  return kECall;
}

void machine_load_program(Machine* m, const char* prog) {
  int fd = open(prog, O_RDONLY);
  if (fd == -1) {
    FATAL(strerror(errno));
  }

  mmu_load_elf(&m->mmu, fd);
  close(fd);

  m->state.pc = (u64)m->mmu.entry;
}
