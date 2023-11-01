#include "syscall.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "machine.h"
#include "reg.h"
#include "types.h"
#include "utils.h"

#define NEWLIB_O_RDONLY 0x0
#define NEWLIB_O_WRONLY 0x1
#define NEWLIB_O_RDWR 0x2
#define NEWLIB_O_APPEND 0x8
#define NEWLIB_O_CREAT 0x200
#define NEWLIB_O_TRUNC 0x400
#define NEWLIB_O_EXCL 0x800

#define __REWRITE_FLAG(flag) \
  if (flags & NEWLIB_##flag) hostflags |= flag;

static inline int convert_flags(int flags) {
  int hostflags = 0;
  __REWRITE_FLAG(O_RDONLY);
  __REWRITE_FLAG(O_WRONLY);
  __REWRITE_FLAG(O_RDWR);
  __REWRITE_FLAG(O_APPEND);
  __REWRITE_FLAG(O_CREAT);
  __REWRITE_FLAG(O_TRUNC);
  __REWRITE_FLAG(O_EXCL);
  return hostflags;
}

#undef __REWRITE_FLAG

static u64 handler_exit(Machine* m) {
  u64 ec = machine_get_xreg(m, X_REG_A0);
  exit(ec);  // #include <stdlib.h>
}

static u64 handler_read(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  u64 buf = machine_get_xreg(m, X_REG_A1);
  u64 nbytes = machine_get_xreg(m, X_REG_A2);
  return read(fd, (void*)TO_HOST(buf), (size_t)nbytes);  // #include <unistd.h>
}

static u64 handler_write(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  u64 buf = machine_get_xreg(m, X_REG_A1);
  u64 n = machine_get_xreg(m, X_REG_A2);
  return write(fd, (void*)TO_HOST(buf), (size_t)n);  // #include <unistd.h>
}

static u64 handler_openat(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  u64 file = machine_get_xreg(m, X_REG_A1);
  u64 oflag = machine_get_xreg(m, X_REG_A2);
  u64 mode = machine_get_xreg(m, X_REG_A3);
  return openat(fd, (char*)TO_HOST(file), convert_flags(oflag),
                (mode_t)mode);  // #include <fcntl.h>
}

static u64 handler_close(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  if (fd > 2) return close(fd);  // #include <unistd.h>
  return 0;
}

static u64 handler_lseek(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  u64 offset = machine_get_xreg(m, X_REG_A1);
  u64 whence = machine_get_xreg(m, X_REG_A2);
  return lseek(fd, (off_t)offset, whence);  // #include <unistd.h>
}

static u64 handler_brk(Machine* m) {
  u64 addr = machine_get_xreg(m, X_REG_A0);
  if (addr == 0) {
    addr = m->mmu.alloc;
  }
  assert(addr >= m->mmu.base);
  i64 inc = (i64)addr - m->mmu.alloc;
  mmu_alloc(&m->mmu, inc);
  return addr;
}

static u64 handler_fstat(Machine* m) {
  u64 fd = machine_get_xreg(m, X_REG_A0);
  u64 addr = machine_get_xreg(m, X_REG_A1);
  return fstat(fd, (struct stat*)TO_HOST(addr));  // #include <sys/stat.h>
}

static u64 handler_gettimeofday(Machine* m) {
  u64 tv_addr = machine_get_xreg(m, X_REG_A0);
  u64 tz_addr = machine_get_xreg(m, X_REG_A1);
  struct timeval* tv = (struct timeval*)TO_HOST(tv_addr);
  struct timezone* tz =
      (tz_addr != 0) ? (struct timezone*)TO_HOST(tz_addr) : NULL;
  return gettimeofday(tv, tz);  // #include <sys/time.h>
}

static u64 handler_ni_syscall(Machine* m) {
  FATALF(", ni syscall: %lu, pc: %lx", machine_get_xreg(m, X_REG_A7),
         m->state.pc);
}

static u64 (*rv_syscall_handler[])(Machine*) = {
    [SYS_EXIT] = handler_exit,
    [SYS_EXIT_GROUP] = handler_exit,
    [SYS_GETPID] = handler_ni_syscall,
    [SYS_KILL] = handler_ni_syscall,
    [SYS_TGKILL] = handler_ni_syscall,
    [SYS_READ] = handler_read,
    [SYS_WRITE] = handler_write,
    [SYS_OPENAT] = handler_openat,
    [SYS_CLOSE] = handler_close,
    [SYS_LSEEK] = handler_lseek,
    [SYS_BRK] = handler_brk,
    [SYS_LINKAT] = handler_ni_syscall,
    [SYS_UNLINKAT] = handler_ni_syscall,
    [SYS_MKDIRAT] = handler_ni_syscall,
    [SYS_RENAMEAT] = handler_ni_syscall,
    [SYS_CHDIR] = handler_ni_syscall,
    [SYS_GETCWD] = handler_ni_syscall,
    [SYS_FSTAT] = handler_fstat,
    [SYS_FSTATAT] = handler_ni_syscall,
    [SYS_FACCESSAT] = handler_ni_syscall,
    [SYS_PREAD] = handler_ni_syscall,
    [SYS_PWRITE] = handler_ni_syscall,
    [SYS_UNAME] = handler_ni_syscall,
    [SYS_GETUID] = handler_ni_syscall,
    [SYS_GETEUID] = handler_ni_syscall,
    [SYS_GETGID] = handler_ni_syscall,
    [SYS_GETEGID] = handler_ni_syscall,
    [SYS_GETTID] = handler_ni_syscall,
    [SYS_SYSINFO] = handler_ni_syscall,
    [SYS_MMAP] = handler_ni_syscall,
    [SYS_MUNMAP] = handler_ni_syscall,
    [SYS_MREMAP] = handler_ni_syscall,
    [SYS_MPROTECT] = handler_ni_syscall,
    [SYS_PRLIMIT64] = handler_ni_syscall,
    [SYS_RT_SIGACTION] = handler_ni_syscall,
    [SYS_WRITEV] = handler_ni_syscall,
    [SYS_GETTIMEOFDAY] = handler_gettimeofday,
    [SYS_TIMES] = handler_ni_syscall,
    [SYS_FCNTL] = handler_ni_syscall,
    [SYS_FTRUNCATE] = handler_ni_syscall,
    [SYS_GETDENTS] = handler_ni_syscall,
    [SYS_DUP] = handler_ni_syscall,
    [SYS_DUP3] = handler_ni_syscall,
    [SYS_READLINKAT] = handler_ni_syscall,
    [SYS_RT_SIGPROCMASK] = handler_ni_syscall,
    [SYS_IOCTL] = handler_ni_syscall,
    [SYS_GETRLIMIT] = handler_ni_syscall,
    [SYS_SETRLIMIT] = handler_ni_syscall,
    [SYS_GETRUSAGE] = handler_ni_syscall,
    [SYS_CLOCK_GETTIME] = handler_ni_syscall,
    [SYS_SET_TID_ADDRESS] = handler_ni_syscall,
    [SYS_SET_ROBUST_LIST] = handler_ni_syscall,
    [SYS_MADVISE] = handler_ni_syscall,
    [SYS_STATX] = handler_ni_syscall,
};

static u64 handler_sysopen(Machine* m) {
  u64 file = machine_get_xreg(m, X_REG_A0);
  u64 oflag = machine_get_xreg(m, X_REG_A1);
  u64 mode = machine_get_xreg(m, X_REG_A2);
  return open((char*)TO_HOST(file), convert_flags(oflag),
              (mode_t)mode);  // #include <fcntl.h>
}

#define OLD_SYSCALL_THRESHOLD 1024

static u64 (*rv_old_syscall_handler[])(Machine*) = {
    [SYS_OPEN - OLD_SYSCALL_THRESHOLD] = handler_sysopen,
    [SYS_LINK - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_UNLINK - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_MKDIR - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_ACCESS - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_STAT - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_LSTAT - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
    [SYS_TIME - OLD_SYSCALL_THRESHOLD] = handler_ni_syscall,
};

u64 do_syscall(Machine* m, u64 syscall) {
  u64 (*handler)(Machine*) = NULL;

  if (syscall < SIZEOF_ARRAY(rv_syscall_handler)) {
    handler = rv_syscall_handler[syscall];
  } else if (syscall - OLD_SYSCALL_THRESHOLD <
             SIZEOF_ARRAY(rv_old_syscall_handler)) {
    handler = rv_old_syscall_handler[syscall - OLD_SYSCALL_THRESHOLD];
  } else if (syscall == SYS_GETMAINVARS) {
    handler = handler_ni_syscall;
  }

  if (!handler) {
    FATALF("unknown syscall: %lu", syscall);
  }

  return handler(m);
}
