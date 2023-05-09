#ifndef RVEMU_RVEMU_H_
#define RVEMU_RVEMU_H_

#include <stdbool.h>

#include "instr.h"
#include "types.h"

typedef struct {
  union {
    RvInstrR rtype;
    RvInstrR4 r4type;
    RvInstrI itype;
    RvInstrS stype;
    RvInstrB btype;
    RvInstrU utype;
    RvInstrJ jtype;
    RvInstrGeneric gtype;
    u32 raw;
  };
  RvInstrType type;
  i32 imm;
  u16 csr;
  bool rvc;
  bool cont;
} RvInstr;

void rv_instr_decode(RvInstr*, u32);

typedef struct {
  u64 entry;
  u64 host_alloc;
  u64 alloc;
  u64 base;
} Mmu;

void mmu_load_elf(Mmu*, int);

typedef enum {
  kNone,
  kDirectBranch,
  kIndirectBranch,
  kECall,
  kExitReasonNum,
} ExitReason;

typedef struct {
  u64 gp_regs[32];
  u64 pc;
  ExitReason exit_reason;
} State;

typedef struct {
  State state;
  Mmu mmu;
} Machine;

void machine_load_program(Machine*, const char*);
ExitReason machine_step(Machine*);

void exec_block_interp(State*);

#endif  // RVEMU_RVEMU_H_
