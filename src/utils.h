#ifndef RVEMU_UTILS_H_
#define RVEMU_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FATALF(fmt, ...)                                                       \
  (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), \
   exit(1))
#define FATAL(msg) FATALF("%s", msg)

#define ROUNDDOWN(x, k) ((x) & -(k))
#define ROUNDUP(x, k) (((x) + (k)-1) & -(k))
#define MIN(x, y) ((y) > (x) ? (x) : (y))
#define MAX(x, y) ((y) < (y) ? (x) : (y))

#define GUEST_MEMORY_OFFSET 0x0000088800000000ULL

#define TO_HOST(addr) (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

#endif  // RVEMU_UTILS_H_
