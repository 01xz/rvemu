#include "machine.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "utils.h"

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

static inline void mmu_write(u64 addr, u8* data, size_t len) {
  memcpy((void*)TO_HOST(addr), (void*)data, len);
}

void machine_setup(Machine* m, int argc, char** argv) {
  size_t stack_size = RVEMU_MACHINE_STACK_SIZE;
  u64 stack = mmu_alloc(&m->mmu, stack_size);
  m->state.gp_regs[kSp] = stack + stack_size;

  m->state.gp_regs[kSp] -= 8;  // auxv
  m->state.gp_regs[kSp] -= 8;  // envp
  m->state.gp_regs[kSp] -= 8;  // argv end

  u64 guest_argc = argc - 1;
  for (u64 i = guest_argc; i > 0; i--) {
    size_t arg_len = strlen(argv[i]);
    u64 addr = mmu_alloc(&m->mmu, arg_len + 1);
    mmu_write(addr, (u8*)argv[i], arg_len);
    m->state.gp_regs[kSp] -= 8;
    mmu_write(m->state.gp_regs[kSp], (u8*)&addr, sizeof(u64));
  }

  m->state.gp_regs[kSp] -= 8;  // argc
  mmu_write(m->state.gp_regs[kSp], (u8*)&guest_argc, sizeof(u64));
}
