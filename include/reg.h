#ifndef RVEMU_REG_H_
#define RVEMU_REG_H_

typedef enum {
  kZero,
  kRa, kSp, kGp, kTp,
  kT0, kT1, kT2,
  kS0, kS1,
  kA0, kA1, kA2, kA3, kA4, kA5, kA6, kA7,
  kS2, kS3, kS4, kS5, kS6, kS7, kS8, kS9, kS10, kS11,
  kT3, kT4, kT5, kT6,
  kNumGpReg,
} GpRegType;

#endif  // RVEMU_REG_H_
