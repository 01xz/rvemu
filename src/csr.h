#ifndef RVEMU_CSR_H_
#define RVEMU_CSR_H_

typedef enum {
  kFflags = 0x001,
  kFrm    = 0x002,
  kFcsr   = 0x003,
  kCsrNum = 4096,
} CsrType;

#endif  // RVEMU_CSR_H_
