#include <assert.h>
#include <stdbool.h>

#include "interp.h"
#include "machine.h"

int main(int argc, char* argv[]) {
  assert(argc > 1);

  Machine m = {0};
  machine_load_program(&m, argv[1]);
  machine_setup(&m, argc, argv);

  while (true) {
    ExitReason reason = machine_step(&m);
    assert(reason == kECall);
  }

  return 0;
}
