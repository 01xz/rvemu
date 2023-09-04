#ifndef RVEMU_MACHINE_H
#define RVEMU_MACHINE_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "mmu.h"
#include "reg.h"
#include "types.h"
#include "utils.h"

typedef enum {
  kNone,
  kDirectBranch,
  kIndirectBranch,
  kECall,
  kExitReasonNum,
} ExitReason;

typedef struct {
  u64 gp_regs[kGpRegNum];
  FpReg fp_regs[kFpRegNum];
  u64 pc;
  u64 re_enter_pc;
  ExitReason exit_reason;
} State;

void exec_block_interp(State*);

typedef struct {
  State state;
  Mmu mmu;
} Machine;

void machine_load_program(Machine*, const char*);

ExitReason machine_step(Machine*);

#endif  // RVEMU_MACHINE_H
