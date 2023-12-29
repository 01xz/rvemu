#ifndef RVEMU_MMU_H_
#define RVEMU_MMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef struct {
  u64 entry;
  u64 host_alloc;
  u64 alloc;
  u64 base;
} Mmu;

void mmu_load_elf(Mmu*, int);

u64 mmu_alloc(Mmu*, i64);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // RVEMU_MMU_H_
