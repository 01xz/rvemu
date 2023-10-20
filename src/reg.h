#ifndef RVEMU_REG_H_
#define RVEMU_REG_H_

#include "types.h"

typedef enum {
  // clang-format off
  kZero,
  kRa,
  kSp,
  kGp,
  kTp,
  kT0, kT1, kT2,
  kS0, kS1,
  kA0, kA1, kA2, kA3, kA4, kA5, kA6, kA7,
  kS2, kS3, kS4, kS5, kS6, kS7, kS8, kS9, kS10, kS11,
  kT3, kT4, kT5, kT6,
  kXRegNum,
  // clang-format on
} XRegType;

typedef enum {
  // clang-format off
  kFt0, kFt1, kFt2, kFt3, kFt4, kFt5, kFt6, kFt7,
  kFs0, kFs1,
  kFa0, kFa1, kFa2, kFa3, kFa4, kFa5, kFa6, kFa7,
  kFs2, kFs3, kFs4, kFs5, kFs6, kFs7, kFs8, kFs9, kFs10, kFs11,
  kFt8, kFt9, kFt10, kFt11,
  kFRegNum,
  // clang-format on
} FRegType;

typedef union {
  u64 lu;
  u32 wu;
  f64 d;
  f32 s;
} FReg;

#endif  // RVEMU_REG_H_
