#ifndef RVEMU_CSR_H_
#define RVEMU_CSR_H_

#include "types.h"
typedef enum {
  // Unprivileged Floating-Point CSRs
  CSR_FFLAGS = 0x001,
  CSR_FRM = 0x002,
  CSR_FCSR = 0x003,
  // Unprivileged Counter/Timers
  CSR_TIME = 0xc01,
  // Machine Level CSRs
  CSR_MVENDORID = 0xf11,
  CSR_MARCHID = 0xf12,
  CSR_MIMPID = 0xf13,
  CSR_MHARTID = 0xf14,
  CSR_MSTATUS = 0x300,
  CSR_MEDELEG = 0x302,
  CSR_MIDELEG = 0x303,
  CSR_MIE = 0x304,
  CSR_MTVEC = 0x305,
  CSR_MEPC = 0x341,
  CSR_MCAUSE = 0x342,
  CSR_MTVAL = 0x343,
  CSR_MIP = 0x344,
  // Supervisor Level CSRs
  CSR_SSTATUS = 0x100,
  CSR_SIE = 0x104,
  CSR_STVEC = 0x105,
  CSR_SEPC = 0x141,
  CSR_SCAUSE = 0x142,
  CSR_STVAL = 0x143,
  CSR_SIP = 0x144,
  CSR_SATP = 0x180,
  // Total Numbers
  CSR_NUM = 4096,
} CsrType;

typedef struct {
  u64 nx : 1;
  u64 uf : 1;
  u64 of : 1;
  u64 dz : 1;
  u64 nv : 1;
  u64    : 59;
} Fflags;

typedef enum {
  kRne = 0x0,
  kRtz = 0x1,
  kRdn = 0x2,
  kRup = 0x3,
  kRmm = 0x4,
  kDyn = 0x7,
  kInvalid,
} RoundingMode;

typedef struct {
  u64 fflags   : 5;
  u64 frm      : 3;
  u64 reserved : 24;
  u64          : 32;
} Fcsr;

typedef struct {
  u64 wpri_0 : 1;
  u64 sie    : 1;
  u64 wpri_1 : 1;
  u64 mie    : 1;
  u64 wpri_2 : 1;
  u64 spie   : 1;
  u64 ube    : 1;
  u64 mpie   : 1;
  u64 spp    : 1;
  u64 wpri_3 : 2;
  u64 mpp    : 2;
  u64 fs     : 2;
  u64 xs     : 2;
  u64 mprv   : 1;
  u64 sum    : 1;
  u64 mxr    : 1;
  u64 tvm    : 1;
  u64 tw     : 1;
  u64 tsr    : 1;
  u64 wpri_4 : 9;
  u64 uxl    : 2;
  u64 sxl    : 2;
  u64 sbe    : 1;
  u64 mbe    : 1;
  u64 wpri_5 : 25;
  u64 sd     : 1;
} Mstatus;

typedef struct {
  u64 wpri_0 : 1;
  u64 sie    : 1;
  u64 wpri_1 : 3;
  u64 spie   : 1;
  u64 ube    : 1;
  u64 wpri_2 : 1;
  u64 spp    : 1;
  u64 vs     : 2;
  u64 wpri_3 : 2;
  u64 fs     : 2;
  u64 xs     : 2;
  u64 wpri_4 : 1;
  u64 sum    : 1;
  u64 mxr    : 1;
  u64 wpri_5 : 12;
  u64 uxl    : 2;
  u64 wpri_6 : 29;
  u64 sd     : 1;
} Sstatus;

#endif  // RVEMU_CSR_H_
