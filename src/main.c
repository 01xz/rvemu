#include "interp.h"
#include "machine.h"
#include "reg.h"
#include "syscall.h"

int main(int argc, char* argv[]) {
  assert(argc > 1);

  Machine m = {0};
  machine_load_program(&m, argv[1]);
  machine_setup(&m, argc, argv);

  while (true) {
    ExitReason reason = machine_step(&m);
    assert(reason == kECall);

    u64 syscall = machine_get_xreg(&m, XREG_A7);
    u64 ret = do_syscall(&m, syscall);
    machine_set_xreg(&m, XREG_A0, ret);
  }

  return 0;
}
