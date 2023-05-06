#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "elfdef.h"
#include "rvemu.h"
#include "utils.h"

static void load_prog_header(ElfProgHeader* elf_prog_header_p,
                             ElfHeader* elf_header_p, i64 i, FILE* fp) {
  if (fseek(fp, elf_header_p->e_phoff + elf_header_p->e_phentsize * i,
            SEEK_SET) != 0) {
    FATAL("Seek file failed");
  }

  if (fread((void*)elf_prog_header_p, 1, sizeof(ElfProgHeader), fp) !=
      sizeof(ElfProgHeader)) {
    FATAL("File too small");
  }
}

static int flags_to_mmap_prot(u32 flags) {
  return (flags & PF_R ? PROT_READ : 0) | (flags & PF_W ? PROT_WRITE : 0) |
         (flags & PF_X ? PROT_EXEC : 0);
}

static void mmu_load_segment(Mmu* mmu, ElfProgHeader* elf_prog_header_p,
                             int fd) {
  int page_size = getpagesize();

  u64 offset = elf_prog_header_p->p_offset;
  u64 aligned_offset = ROUNDDOWN(offset, page_size);

  u64 vaddr = TO_HOST(elf_prog_header_p->p_vaddr);
  u64 aligned_vaddr = ROUNDDOWN(vaddr, page_size);

  u64 filesz = elf_prog_header_p->p_filesz + (vaddr - aligned_vaddr);
  u64 memsz = elf_prog_header_p->p_memsz + (vaddr - aligned_vaddr);

  int prot = flags_to_mmap_prot(elf_prog_header_p->p_flags);

  u64 addr = (u64)mmap((void*)aligned_vaddr, filesz, prot,
                       MAP_PRIVATE | MAP_FIXED, fd, aligned_offset);
  assert(addr == aligned_vaddr);

  u64 remaining_bss = ROUNDUP(memsz, page_size) - ROUNDUP(filesz, page_size);
  if (remaining_bss > 0) {
    u64 addr = (u64)mmap((void*)aligned_vaddr + ROUNDUP(filesz, page_size),
                         remaining_bss, prot,
                         MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
    assert(addr == aligned_vaddr + ROUNDUP(filesz, page_size));
  }

  mmu->host_alloc =
      MAX(mmu->host_alloc, (aligned_vaddr + ROUNDUP(memsz, page_size)));
  mmu->base = mmu->alloc = TO_GUEST(mmu->host_alloc);
}

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

  ElfProgHeader elf_prog_header = {0};
  for (i64 i = 0; i < elf_header_p->e_phnum; ++i) {
    load_prog_header(&elf_prog_header, elf_header_p, i, fp);
    if (elf_prog_header.p_type == PT_LOAD) {
      mmu_load_segment(mmu, &elf_prog_header, fd);
    }
  }
}
