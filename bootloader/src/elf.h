#ifndef ELF_H
#define ELF_H

#include <efi.h>

#define ELF_MAGIC 0x464C457F

// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
typedef struct __attribute__((packed)) { // to make sure compiler doesnt act up
    UINT32 magic;
    UINT8  class;
    UINT8  endian;
    UINT8  version;
    UINT8  abi;
    UINT8  padding[8];
    UINT16 type;
    UINT16 machine; // isa
    UINT32 elf_version;
    UINT64 entry;
    UINT64 phoff;
    UINT64 shoff;
    UINT32 flags;
    UINT16 ehsize;
    UINT16 phentsize;
    UINT16 phnum;
    UINT16 shentsize;
    UINT16 shnum;
    UINT16 shstrndx;
} elf64_header_t;

typedef struct __attribute__((packed)) {
    UINT32 type;
    UINT32 flags;
    UINT64 offset;
    UINT64 vaddr;
    UINT64 paddr;
    UINT64 filesz;
    UINT64 memsz;
    UINT64 align;
} elf64_phdr_t;

#endif
