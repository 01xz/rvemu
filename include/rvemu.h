#ifndef RVEMU_RVEMU_H_
#define RVEMU_RVEMU_H_

#include "types.h"

typedef struct {
  u64 entry;
  u64 host_alloc;
  u64 alloc;
  u64 base;
} Mmu;

void mmu_load_elf(Mmu*, int);

typedef struct {
  u64 gp_regs[32];
  u64 pc;
} State;

typedef struct {
  State state;
  Mmu mmu;
} Machine;

void machine_load_program(Machine*, const char*);

#endif  // RVEMU_RVEMU_H_
