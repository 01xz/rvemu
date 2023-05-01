#include "reg.h"
#include "rvemu.h"
#include "utils.h"

static void (*handler[kNumInstr])(State*, RvInstr*) = {};

void exec_block_interp(State* state) {
  static RvInstr instr = {0};
  while (true) {
    u32 instr_raw = *(u32*)TO_HOST(state->pc);
    instr_decode(&instr, instr_raw);
    handler[instr.type](state, &instr);

    state->gp_regs[kZero] = 0;

    if (instr.cont) break;

    state->pc += instr.rvc ? 2 : 4;
  }
}
