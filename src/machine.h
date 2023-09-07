#ifndef RVEMU_MACHINE_H_
#define RVEMU_MACHINE_H_

#include <assert.h>

#include "interp.h"
#include "mmu.h"

#define RVEMU_MACHINE_STACK_SIZE (32 * 1024 * 1024)

typedef struct {
  State state;
  Mmu mmu;
} Machine;

void machine_load_program(Machine*, const char*);

void machine_setup(Machine*, int, char**);

ExitReason machine_step(Machine*);

inline u64 machine_get_regx(Machine* m, int reg) {
  assert(reg > 0 && reg <= kGpRegNum);
  return m->state.gp_regs[reg];
}

inline void machine_set_regx(Machine* m, int reg, u64 reg_val) {
  assert(reg > 0 && reg <= kGpRegNum);
  m->state.gp_regs[reg] = reg_val;
}

#endif  // RVEMU_MACHINE_H_
