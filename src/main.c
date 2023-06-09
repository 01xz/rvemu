#include <assert.h>
#include <stdio.h>

#include "rvemu.h"

int main(int argc, char* argv[]) {
  assert(argc > 1);

  Machine m = {0};
  machine_load_program(&m, argv[1]);

  while (true) {
    ExitReason reason = machine_step(&m);
    assert(reason == kECall);
  }

  return 0;
}
