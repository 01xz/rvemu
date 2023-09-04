#ifndef RVEMU_RVEMU_H_
#define RVEMU_RVEMU_H_

#include <stdbool.h>

#include "instr.h"
#include "reg.h"
#include "types.h"

typedef struct {
  union {
    RvInstrR rtype;
    RvInstrR4 r4type;
    RvInstrI itype;
    RvInstrS stype;
    RvInstrB btype;
    RvInstrU utype;
    RvInstrJ jtype;
    RvInstrGeneric gtype;
    u32 raw;
  };
  RvInstrType type;
  i32 imm;
  u16 csr;
  bool rvc;
  bool cont;
} RvInstr;

void rv_instr_decode(RvInstr*, u32);


#endif  // RVEMU_RVEMU_H_
