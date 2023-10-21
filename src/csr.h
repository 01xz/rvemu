#ifndef RVEMU_CSR_H_
#define RVEMU_CSR_H_

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

#endif  // RVEMU_CSR_H_
