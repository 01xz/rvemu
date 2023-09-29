#ifndef RVEMU_INTERP_H_
#define RVEMU_INTERP_H_

#include "reg.h"
#include "types.h"

typedef enum {
  kNone,
  kDirectBranch,
  kIndirectBranch,
  kECall,
  kExitReasonNum,
} ExitReason;

typedef struct {
  u64 xregs[kXRegNum];
  FReg fregs[kFRegNum];
  u64 pc;
  u64 re_enter_pc;
  ExitReason exit_reason;
} State;

void exec_block_interp(State*);

#endif  // RVEMU_INTERP_H_
