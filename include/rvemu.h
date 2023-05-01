#ifndef RVEMU_RVEMU_H_
#define RVEMU_RVEMU_H_

#include <stdbool.h>

#include "types.h"

typedef enum {
  kAddi,
  kNumInstr,
} RvInstrType;

typedef struct {
  RvInstrType type;
  i8 rs1;
  i8 rs2;
  i8 rd;
  i32 imm;
  bool rvc;
  bool cont;
} RvInstr;

void instr_decode(RvInstr*, u32);

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
  kSysCall,
  kNumExitReason,
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
