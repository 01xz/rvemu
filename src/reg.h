#ifndef RVEMU_REG_H_
#define RVEMU_REG_H_

#include "types.h"

typedef enum {
  // clang-format off
  XREG_ZERO,
  XREG_RA,
  XREG_SP,
  XREG_GP,
  XREG_TP,
  XREG_T0, XREG_T1, XREG_T2,
  XREG_S0, XREG_S1,
  XREG_A0, XREG_A1, XREG_A2, XREG_A3, XREG_A4, XREG_A5, XREG_A6, XREG_A7,
  XREG_S2, XREG_S3, XREG_S4, XREG_S5, XREG_S6, XREG_S7, XREG_S8, XREG_S9, XREG_S10, XREG_S11,
  XREG_T3, XREG_T4, XREG_T5, XREG_T6,
  XREG_NUM,
  // clang-format on
} XRegType;

typedef enum {
  // clang-format off
  FREG_FT0, FREG_FT1, FREG_FT2, FREG_FT3, FREG_FT4, FREG_FT5, FREG_FT6, FREG_FT7,
  FREG_FS0, FREG_FS1,
  FREG_FA0, FREG_FA1, FREG_FA2, FREG_FA3, FREG_FA4, FREG_FA5, FREG_FA6, FREG_FA7,
  FREG_FS2, FREG_FS3, FREG_FS4, FREG_FS5, FREG_FS6, FREG_FS7, FREG_FS8, FREG_FS9, FREG_FS10, FREG_FS11,
  FREG_FT8, FREG_FT9, FREG_FT10, FREG_FT11,
  FREG_NUM,
  // clang-format off
} FRegType;

typedef union {
  u64 lu;
  u32 wu;
  f64 d;
  f32 s;
} FReg;

#endif  // RVEMU_REG_H_
