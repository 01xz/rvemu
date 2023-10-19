#ifndef RVEMU_INTERP_H_
#define RVEMU_INTERP_H_

#include <stdbool.h>

#include "csr.h"
#include "reg.h"
#include "types.h"

#define PAGE_SIZE 4096

typedef enum {
  kNone,
  kDirectBranch,
  kIndirectBranch,
  kECall,
  kExitReasonNum,
} ExitReason;

typedef enum {
  kUser = 0x0,
  kSupervisor = 0x1,
  kMachine = 0x3,
  kDebug,
} Mode;

typedef struct {
  u64 xregs[kXRegNum];
  FReg fregs[kFRegNum];
  u64 csrs[kCsrNum];
  u64 pc;
  u64 re_enter_pc;
  ExitReason exit_reason;
  Mode mode;
  bool enable_paging;
  u64 page_table;
} State;

void exec_block_interp(State*);

#endif  // RVEMU_INTERP_H_
