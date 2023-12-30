#ifndef RVEMU_UTILS_H_
#define RVEMU_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

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

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

static inline u64 executable_load(u64 addr, u8 size) {
  switch (size) {
    case sizeof(u8):
      return *(u8*)TO_HOST(addr);
    case sizeof(u16):
      return *(u16*)TO_HOST(addr);
    case sizeof(u32):
      return *(u32*)TO_HOST(addr);
    case sizeof(u64):
      return *(u64*)TO_HOST(addr);
    default:
      FATALF("executable load fault, size: %u", size);
  }
}

static inline void executable_store(u64 addr, u64 value, u8 size) {
  switch (size) {
    case sizeof(u8):
      *(u8*)TO_HOST(addr) = (u8)value;
      return;
    case sizeof(u16):
      *(u16*)TO_HOST(addr) = (u16)value;
      return;
    case sizeof(u32):
      *(u32*)TO_HOST(addr) = (u32)value;
      return;
    case sizeof(u64):
      *(u64*)TO_HOST(addr) = value;
      return;
    default:
      FATALF("executable store fault, size: %u", size);
  }
}

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // RVEMU_UTILS_H_
