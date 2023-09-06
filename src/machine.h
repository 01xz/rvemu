#ifndef RVEMU_MACHINE_H_
#define RVEMU_MACHINE_H_

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

#endif  // RVEMU_MACHINE_H_
