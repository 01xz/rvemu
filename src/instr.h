#ifndef RVEMU_INSTR_H_
#define RVEMU_INSTR_H_

#include "types.h"

typedef enum {
  kLb, kLh, kLw, kLd, kLbu, kLhu, kLwu,
  kFence, kFenceI,
  kAddi, kSlli, kSlti, kSltiu, kXori, kSrli, kSrai, kOri, kAndi,
  kAuipc,
  kAddiw, kSlliw, kSrliw, kSraiw,
  kSb, kSh, kSw, kSd,
  kAdd, kSub, kSll, kSlt, kSltu, kXor, kSrl, kSra, kOr, kAnd,
  kMul, kMulh, kMulhsu, kMulhu, kDiv, kDivu, kRem, kRemu,
  kLui,
  kAddw, kSubw, kSllw, kSrlw, kSraw, kMulw, kDivw, kDivuw, kRemw, kRemuw,
  kBeq, kBne, kBlt, kBge, kBltu, kBgeu,
  kJalr, kJal,
  kEcall,
  kCsrrw, kCsrrs, kCsrrc, kCsrrwi, kCsrrsi, kCsrrci,
  kFlw,
  kFsw,
  kFmaddS, kFmsubS, kFnmsubS, kFnmaddS, kFaddS, kFsubS, kFmulS, kFdivS,
  kFsgnjS, kFsgnjnS, kFsgnjxS,
  kFminS, kFmaxS,
  kFsqrtS,
  kFleS, kFltS, kFeqS,
  kFcvtWS, kFcvtWuS, kFcvtLS, kFcvtLuS,
  kFcvtSW, kFcvtSWu, kFcvtSL, kFcvtSLu,
  kFmvXW, kFclassS,
  kFmvWX,
  kFld,
  kFsd,
  kFmaddD, kFmsubD, kFnmsubD, kFnmaddD, kFaddD, kFsubD, kFmulD, kFdivD,
  kFsgnjD, kFsgnjnD, kFsgnjxD,
  kFminD, kFmaxD,
  kFcvtSD, kFcvtDS,
  kFsqrtD,
  kFleD, kFltD, kFeqD,
  kFcvtWD, kFcvtWuD, kFcvtLD, kFcvtLuD,
  kFcvtDW, kFcvtDWu, kFcvtDL, kFcvtDLu,
  kFmvXD, kFclassD,
  kFmvDX,
  kRvInstrNum,
} RvInstrType;

typedef struct {
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct7 : 7;
} RvInstrR;

typedef struct {
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct2 : 2;
  u32 rs3    : 5;
} RvInstrR4;

typedef struct {
  u32 opcode  : 7;
  u32 rd      : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 imm11_0 : 12;
} RvInstrI;

typedef struct {
  u32 opcode  : 7;
  u32 imm4_0  : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm11_5 : 7;
} RvInstrS;

typedef struct {
  u32 opcode  : 7;
  u32 imm11   : 1;
  u32 imm4_1  : 4;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm10_5 : 6;
  u32 imm12   : 1;
} RvInstrB;

typedef struct {
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm31_12 : 20;
} RvInstrU;

typedef struct {
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm19_12 : 8;
  u32 imm11    : 1;
  u32 imm10_1  : 10;
  u32 imm20    : 1;
} RvInstrJ;

typedef struct {
  u32 quadrant   : 2;
  u32 opcode     : 5;
  u32            : 6;
  u32 instr15_13 : 3;
  u32            : 4;
  u32 instr24_20 : 5;
  u32 instr31_25 : 7;
} RvInstrGeneric;

#endif  // RVEMU_INSTR_H_
