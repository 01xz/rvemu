#ifndef RVEMU_REG_H_
#define RVEMU_REG_H_

#include "types.h"

typedef enum {
  // clang-format off
  X_REG_ZERO,
  X_REG_RA,
  X_REG_SP,
  X_REG_GP,
  X_REG_TP,
  X_REG_T0, X_REG_T1, X_REG_T2,
  X_REG_S0, X_REG_S1,
  X_REG_A0, X_REG_A1, X_REG_A2, X_REG_A3, X_REG_A4, X_REG_A5, X_REG_A6, X_REG_A7,
  X_REG_S2, X_REG_S3, X_REG_S4, X_REG_S5, X_REG_S6, X_REG_S7, X_REG_S8, X_REG_S9, X_REG_S10, X_REG_S11,
  X_REG_T3, X_REG_T4, X_REG_T5, X_REG_T6,
  X_REG_NUM,
  // clang-format on
} XRegType;

typedef enum {
  // clang-format off
  F_REG_FT0, F_REG_FT1, F_REG_FT2, F_REG_FT3, F_REG_FT4, F_REG_FT5, F_REG_FT6, F_REG_FT7,
  F_REG_FS0, F_REG_FS1,
  F_REG_FA0, F_REG_FA1, F_REG_FA2, F_REG_FA3, F_REG_FA4, F_REG_FA5, F_REG_FA6, F_REG_FA7,
  F_REG_FS2, F_REG_FS3, F_REG_FS4, F_REG_FS5, F_REG_FS6, F_REG_FS7, F_REG_FS8, F_REG_FS9, F_REG_FS10, F_REG_FS11,
  F_REG_FT8, F_REG_FT9, F_REG_FT10, F_REG_FT11,
  F_REG_NUM,
  // clang-format off
} FRegType;

typedef union {
  u64 lu;
  u32 wu;
  f64 d;
  f32 s;
} FReg;

#endif  // RVEMU_REG_H_
