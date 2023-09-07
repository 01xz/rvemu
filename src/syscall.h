#ifndef RVEMU_SYSCALL_H_
#define RVEMU_SYSCALL_H_

#include "machine.h"
#include "types.h"

typedef enum {
  kSysExit = 93,
  kSysExitGroup = 94,
  kSysGetpid = 172,
  kSysKill = 129,
  kSysTgkill = 131,
  kSysRead = 63,
  kSysWrite = 64,
  kSysOpenat = 56,
  kSysClose = 57,
  kSysLseek = 62,
  kSysBrk = 214,
  kSysLinkat = 37,
  kSysUnlinkat = 35,
  kSysMkdirat = 34,
  kSysRenameat = 38,
  kSysChdir = 49,
  kSysGetcwd = 17,
  kSysFstat = 80,
  kSysFstatat = 79,
  kSysFaccessat = 48,
  kSysPread = 67,
  kSysPwrite = 68,
  kSysUname = 160,
  kSysGetuid = 174,
  kSysGeteuid = 175,
  kSysGetgid = 176,
  kSysGetegid = 177,
  kSysGettid = 178,
  kSysSysinfo = 179,
  kSysMmap = 222,
  kSysMunmap = 215,
  kSysMremap = 216,
  kSysMprotect = 226,
  kSysPrlimit64 = 261,
  kSysGetmainvars = 2011,
  kSysRtSigaction = 134,
  kSysWritev = 66,
  kSysGettimeofday = 169,
  kSysTimes = 153,
  kSysFcntl = 25,
  kSysFtruncate = 46,
  kSysGetdents = 61,
  kSysDup = 23,
  kSysDup3 = 24,
  kSysReadlinkat = 78,
  kSysRtSigprocmask = 135,
  kSysIoctl = 29,
  kSysGetrlimit = 163,
  kSysSetrlimit = 164,
  kSysGetrusage = 165,
  kSysClockGettime = 113,
  kSysSetTidAddress = 96,
  kSysSetRobustList = 99,
  kSysMadvise = 233,
  kSysStatx = 291,
} SysCallType;

typedef enum {
  kSysOpen = 1024,
  kSysLink = 1025,
  kSysUnlink = 1026,
  kSysMkdir = 1030,
  kSysAccess = 1033,
  kSysStat = 1038,
  kSysLstat = 1039,
  kSysTime = 1062,
} OldSysCallType;

#define OLD_SYSCALL_THRESHOLD 1024

u64 do_syscall(Machine*, u64);

#endif  // RVEMU_SYSCALL_H_
