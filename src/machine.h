#ifndef RVEMU_MACHINE_H
#define RVEMU_MACHINE_H

#include "interp.h"
#include "mmu.h"

typedef struct {
  State state;
  Mmu mmu;
} Machine;

void machine_load_program(Machine*, const char*);

ExitReason machine_step(Machine*);

#endif  // RVEMU_MACHINE_H
