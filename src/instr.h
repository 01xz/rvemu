#ifndef RVEMU_INSTR_H_
#define RVEMU_INSTR_H_

typedef enum {
  // clang-format off
  kLb, kLh, kLw, kLd, kLbu, kLhu, kLwu,
  kFence,
  kFenceI,
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
  // clang-format on
} RvInstrType;

#endif  // RVEMU_INSTR_H_
