#ifndef RVEMU_CSR_H_
#define RVEMU_CSR_H_

typedef enum {
  // Unprivileged Floating-Point CSRs
  kFflags = 0x001,
  kFrm = 0x002,
  kFcsr = 0x003,
  // Unprivileged Counter/Timers
  kTime = 0xc01,
  // Machine Level CSRs
  kMvendorid = 0xf11,
  kMarchid = 0xf12,
  kMimpid = 0xf13,
  kMhartid = 0xf14,
  kMstatus = 0x300,
  kMedeleg = 0x302,
  kMideleg = 0x303,
  kMie = 0x304,
  kMtvec = 0x305,
  kMepc = 0x341,
  kMcause = 0x342,
  kMtval = 0x343,
  kMip = 0x344,
  // Supervisor Level CSRs
  kSstatus = 0x100,
  kSie = 0x104,
  kStvec = 0x105,
  kSepc = 0x141,
  kScause = 0x142,
  kStval = 0x143,
  kSip = 0x144,
  kSatp = 0x180,
  // Total Numbers
  kCsrNum = 4096,
} CsrType;

#endif  // RVEMU_CSR_H_
