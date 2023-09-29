#include "syscall.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "machine.h"
#include "reg.h"
#include "types.h"
#include "utils.h"

static u64 handler_exit(Machine* m) {
  u64 ec = machine_get_xreg(m, kA0);
  exit(ec);  // #include <stdlib.h>
}

static u64 handler_write(Machine* m) {
  u64 fd = machine_get_xreg(m, kA0);
  u64 ptr = machine_get_xreg(m, kA1);
  u64 len = machine_get_xreg(m, kA2);
  return write(fd, (void*)TO_HOST(ptr), (size_t)len);  // #include <unistd.h>
}

static u64 handler_close(Machine* m) {
  u64 fd = machine_get_xreg(m, kA0);
  if (fd > 2) return close(fd);  // #include <unistd.h>
  return 0;
}

static u64 handler_brk(Machine* m) {
  u64 addr = machine_get_xreg(m, kA0);
  if (addr == 0) {
    addr = m->mmu.alloc;
  }
  assert(addr >= m->mmu.base);
  i64 inc = (i64)addr - m->mmu.alloc;
  mmu_alloc(&m->mmu, inc);
  return addr;
}

static u64 handler_fstat(Machine* m) {
  u64 fd = machine_get_xreg(m, kA0);
  u64 addr = machine_get_xreg(m, kA1);
  return fstat(fd, (struct stat*)TO_HOST(addr));  // #include <sys/stat.h>
}

static u64 handler_ni_syscall(Machine* m) {
  FATALF(", ni syscall: %lu, pc: %lx", machine_get_xreg(m, kA7), m->state.pc);
}

static u64 (*rv_syscall_handler[])(Machine*) = {
    [kSysExit] = handler_exit,
    [kSysExitGroup] = handler_ni_syscall,
    [kSysGetpid] = handler_ni_syscall,
    [kSysKill] = handler_ni_syscall,
    [kSysTgkill] = handler_ni_syscall,
    [kSysRead] = handler_ni_syscall,
    [kSysWrite] = handler_write,
    [kSysOpenat] = handler_ni_syscall,
    [kSysClose] = handler_close,
    [kSysLseek] = handler_ni_syscall,
    [kSysBrk] = handler_brk,
    [kSysLinkat] = handler_ni_syscall,
    [kSysUnlinkat] = handler_ni_syscall,
    [kSysMkdirat] = handler_ni_syscall,
    [kSysRenameat] = handler_ni_syscall,
    [kSysChdir] = handler_ni_syscall,
    [kSysGetcwd] = handler_ni_syscall,
    [kSysFstat] = handler_fstat,
    [kSysFstatat] = handler_ni_syscall,
    [kSysFaccessat] = handler_ni_syscall,
    [kSysPread] = handler_ni_syscall,
    [kSysPwrite] = handler_ni_syscall,
    [kSysUname] = handler_ni_syscall,
    [kSysGetuid] = handler_ni_syscall,
    [kSysGeteuid] = handler_ni_syscall,
    [kSysGetgid] = handler_ni_syscall,
    [kSysGetegid] = handler_ni_syscall,
    [kSysGettid] = handler_ni_syscall,
    [kSysSysinfo] = handler_ni_syscall,
    [kSysMmap] = handler_ni_syscall,
    [kSysMunmap] = handler_ni_syscall,
    [kSysMremap] = handler_ni_syscall,
    [kSysMprotect] = handler_ni_syscall,
    [kSysPrlimit64] = handler_ni_syscall,
    [kSysRtSigaction] = handler_ni_syscall,
    [kSysWritev] = handler_ni_syscall,
    [kSysGettimeofday] = handler_ni_syscall,
    [kSysTimes] = handler_ni_syscall,
    [kSysFcntl] = handler_ni_syscall,
    [kSysFtruncate] = handler_ni_syscall,
    [kSysGetdents] = handler_ni_syscall,
    [kSysDup] = handler_ni_syscall,
    [kSysDup3] = handler_ni_syscall,
    [kSysReadlinkat] = handler_ni_syscall,
    [kSysRtSigprocmask] = handler_ni_syscall,
    [kSysIoctl] = handler_ni_syscall,
    [kSysGetrlimit] = handler_ni_syscall,
    [kSysSetrlimit] = handler_ni_syscall,
    [kSysGetrusage] = handler_ni_syscall,
    [kSysClockGettime] = handler_ni_syscall,
    [kSysSetTidAddress] = handler_ni_syscall,
    [kSysSetRobustList] = handler_ni_syscall,
    [kSysMadvise] = handler_ni_syscall,
    [kSysStatx] = handler_ni_syscall,
};

static u64 (*rv_old_syscall_handler[])(Machine*) = {
    [kSysOpen - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysLink - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysUnlink - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysMkdir - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysAccess - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysStat - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysLstat - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [kSysTime - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
};

#define RVEMU_SYSCALL_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

u64 do_syscall(Machine* m, u64 syscall) {
  u64 (*handler)(Machine*) = NULL;

  if (syscall < RVEMU_SYSCALL_ARRAY_SIZE(rv_syscall_handler)) {
    handler = rv_syscall_handler[syscall];
  } else if (syscall - OLD_SYSCALL_THRESHOLD <
             RVEMU_SYSCALL_ARRAY_SIZE(rv_old_syscall_handler)) {
    handler = rv_old_syscall_handler[syscall - OLD_SYSCALL_THRESHOLD];
  } else if (syscall == kSysGetmainvars) {
    handler = handler_ni_syscall;
  }

  if (!handler) {
    FATAL("unknown syscall");
  }

  return handler(m);
}
