#include <assert.h>
#include <stdio.h>

#include "rvemu.h"

int main(int argc, char* argv[]) {
  assert(argc > 1);

  Machine m = {0};
  machine_load_program(&m, argv[1]);

  printf("entry: %lx\n", m.mmu.entry);
  return 0;
}
