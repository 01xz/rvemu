#ifndef RVEMU_DECODE_H_
#define RVEMU_DECODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "instr.h"
#include "types.h"

typedef struct {
  RvInstrType type;
  u8 rs1;
  u8 rs2;
  u8 rs3;
  u8 rd;
  i32 imm;
  u16 csr;
  bool rvc;
} RvInstr;

void rv_instr_decode(RvInstr*, u32);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // RVEMU_DECODE_H_
