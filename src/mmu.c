#include <stdio.h>

#include "elfdef.h"
#include "rvemu.h"
#include "utils.h"

void mmu_load_elf(Mmu* mmu, int fd) {
  u8 buffer[sizeof(ElfHeader)];
  FILE* fp = fdopen(fd, "rb");
  if (fread(buffer, 1, sizeof(ElfHeader), fp) != sizeof(ElfHeader)) {
    FATAL("File too small");
  }

  ElfHeader* elf_header_p = (ElfHeader*)buffer;

  if (*(u32*)elf_header_p != *(u32*)ELFMAG) {
    FATAL("Bad elf file");
  }

  if (elf_header_p->e_machine != EM_RISCV ||
      elf_header_p->e_ident[EI_CLASS] != ELFCLASS64) {
    FATAL("Only riscv64 elf is supported");
  }

  mmu->entry = (u64)elf_header_p->e_entry;
}
