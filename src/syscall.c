#include "syscall.h"

#include <sys/stat.h>

#include "machine.h"
#include "reg.h"
#include "types.h"
#include "utils.h"

static u64 handler_fstat(Machine* m) {
  u64 fd = machine_get_regx(m, kA0);
  u64 addr = machine_get_regx(m, kA1);
  return fstat(fd, (struct stat*)TO_HOST(addr));  // <sys/stat.h>
}

static u64 handler_ni_syscall(const Machine* m) {
  FATALF("ni syscall: %lu", machine_get_regx(m, kA7));
}

static u64 (*rv_syscall_handler[])(Machine*) = {
    [kSysExit] = handler_ni_syscall,
    [kSysExitGroup] = handler_ni_syscall,
    [kSysGetpid] = handler_ni_syscall,
    [kSysKill] = handler_ni_syscall,
    [kSysTgkill] = handler_ni_syscall,
    [kSysRead] = handler_ni_syscall,
    [kSysWrite] = handler_ni_syscall,
    [kSysOpenat] = handler_ni_syscall,
    [kSysClose] = handler_ni_syscall,
    [kSysLseek] = handler_ni_syscall,
    [kSysBrk] = handler_ni_syscall,
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
    [kSysGetmainvars] = handler_ni_syscall,
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
    [kSysOpen] = handler_ni_syscall,
    [kSysLink] = handler_ni_syscall,
    [kSysUnlink] = handler_ni_syscall,
    [kSysMkdir] = handler_ni_syscall,
    [kSysAccess] = handler_ni_syscall,
    [kSysStat] = handler_ni_syscall,
    [kSysLstat] = handler_ni_syscall,
    [kSysTime] = handler_ni_syscall,
};

#define RVEMU_SYSCALL_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

u64 do_syscall(Machine* m, u64 syscall) {
  u64 (*handler)(Machine*) = NULL;

  if (syscall < RVEMU_SYSCALL_ARRAY_SIZE(rv_syscall_handler)) {
    handler = rv_syscall_handler[syscall];
  } else if (syscall - OLD_SYSCALL_THRESHOLD <
             RVEMU_SYSCALL_ARRAY_SIZE(rv_old_syscall_handler)) {
    handler = rv_old_syscall_handler[syscall - OLD_SYSCALL_THRESHOLD];
  }

  if (!handler) {
    FATAL("unknown syscall");
  }

  return handler(m);
}
