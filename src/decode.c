#include "rvemu.h"
#include "utils.h"

void instr_decode(RvInstr* instr, u32 instr_raw) {
  u32 quadrant = QUADRANT(instr_raw);
  switch (quadrant) {
    case 0x0:
      fatal("TODO");
      break;
    case 0x1:
      fatal("TODO");
      break;
    case 0x2:
      fatal("TODO");
      break;
    case 0x3:
      fatal("TODO");
      break;
    default:
      unreachable();
      break;
  }
}
