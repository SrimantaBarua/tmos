// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>

#define EI_NIDENT     16
#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9

// Magic number
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

// Bit-ness
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

// Endian-ness
#define ELFDATANONE 0
#define ELFDATALSB 1
#define ELFDATAMSB 2

// OS ABI
#define ELFOSABI_NONE       0   // System V ABI

// 32-bit ELF file header
struct elf32_hdr {
	unsigned char e_ident[EI_NIDENT];  // Magic number and other information
	uint16_t      e_type;              // Object file type
	uint16_t      e_machine;           // Architecture
	uint32_t      e_version;           // Object file version
	uint32_t      e_entry;             // Entry point virtual address
	uint32_t      e_phoff;             // Program header table file offset
	uint32_t      e_shoff;             // Section header table file offset
	uint32_t      e_flags;             // Processor specific flags
	uint16_t      e_ehsize;            // ELF Header size in bytes
	uint16_t      e_phentsize;         // Program header table entry size
	uint16_t      e_phnum;             // Program header table entry count
	uint16_t      e_shentsize;         // Section header table entry size
	uint16_t      e_shnum;             // Section header table entry count
	uint16_t      e_shstrndx;          // Section header string table index
};

// 64-bit ELF file header
struct elf64_hdr {
	unsigned char e_ident[EI_NIDENT];  // Magic number and other information
	uint16_t      e_type;              // Object file type
	uint16_t      e_machine;           // Architecture
	uint32_t      e_version;           // Object file version
	uint64_t      e_entry;             // Entry point virtual address
	uint64_t      e_phoff;             // Program header table file offset
	uint64_t      e_shoff;             // Section header table file offset
	uint32_t      e_flags;             // Processor specific flags
	uint16_t      e_ehsize;            // ELF Header size in bytes
	uint16_t      e_phentsize;         // Program header table entry size
	uint16_t      e_phnum;             // Program header table entry count
	uint16_t      e_shentsize;         // Section header table entry size
	uint16_t      e_shnum;             // Section header table entry count
	uint16_t      e_shstrndx;          // Section header string table index
};

// Legal values for e_type (object file type)
#define ET_NONE 0               // No file type
#define ET_REL  1               // Relocatable file
#define ET_EXEC 2               // Executable file
#define ET_DYN  3               // Shared object file
#define ET_CORE 4               // Core file

// Legal values for e_machine (architecture)
#define EM_NONE   0             // No machine
#define EM_386    3             // Intel 80386
#define EM_ARM    40            // ARM
#define EM_x86_64 62            // AMD x86_64

// 32-bit ELF section header
struct elf32_shdr {
	uint32_t sh_name;              // Section name (string table index)
	uint32_t sh_type;              // Section type
	uint32_t sh_flags;             // Section flags
	uint32_t sh_addr;              // Section virtual address at execution
	uint32_t sh_offset;            // Section file offset
	uint32_t sh_size;              // Section size in bytes
	uint32_t sh_link;              // Link to another section
	uint32_t sh_info;              // Additional section info
	uint32_t sh_addralign;         // Section alignment
	uint32_t sh_entsize;           // Entry size if section holds table
};

// 64-bit ELF section header
struct elf64_shdr {
	uint32_t sh_name;              // Section name (string table index)
	uint32_t sh_type;              // Section type
	uint64_t sh_flags;             // Section flags
	uint64_t sh_addr;              // Section virtual address at execution
	uint64_t sh_offset;            // Section file offset
	uint64_t sh_size;              // Section size in bytes
	uint32_t sh_link;              // Link to another section
	uint32_t sh_info;              // Additional section info
	uint64_t sh_addralign;         // Section alignment
	uint64_t sh_entsize;           // Entry size if section holds table
};

// Legal values for sh_type (Section type)
#define SHT_NULL          0	  // Section header table entry unused
#define SHT_PROGBITS      1   // Program data
#define SHT_SYMTAB        2   // Symbol table
#define SHT_STRTAB        3   // String table
#define SHT_RELA          4   // Relocation entries with addends
#define SHT_HASH          5   // Symbol hash table
#define SHT_DYNAMIC       6   // Dynamic linking info
#define SHT_NOTE          7   // Notes
#define SHT_NOBITS        8   // Program space with no data (BSS)
#define SHT_REL           9   // Relocation entries, no addends
#define SHT_SHLIB         10  // Reserved
#define SHT_DYNSYM        11  // Dynamic linker symbol table
#define SHT_INIT_ARRAY    14  // Array of constructors
#define SHT_FINI_ARRAY    15  // Array of destructors
#define SHT_PREINIT_ARRAY 16  // Array of pre-constructors
#define SHT_GROUP         17  // Section group
#define SHT_SYMTAB_SHNDX  18  // Entended section indices

// Legal values for sh_flags (section flags)
#define SHF_WRITE            (1 << 0)   // Writable
#define SHF_ALLOC            (1 << 1)   // Occupies memory
#define SHF_EXEC             (1 << 2)   // Executable
#define SHF_MERGE            (1 << 4)   // Might be merged
#define SHF_STRINGS          (1 << 5)   // Contains NULL-terminated strings
#define SHF_INFO_LINK        (1 << 6)   // 'sh_info' contains SHT index
#define SHF_LINK_ORDER       (1 << 7)   // Preserve order after combining
#define SHF_OS_NONCONFORMING (1 << 8)   // Non-standard OS specific handing
#define SHF_GROUP            (1 << 9)   // Section is member of a group
#define SHF_TLS              (1 << 10)  // Section holds thread-local data
#define SHF_COMPRESSED       (1 << 11)  // Section with compressed data
#define SHF_ORDERED          (1 << 30)  // Special ordering requirement

// 32-bit symbol table entry
struct elf32_sym {
	uint32_t      st_name;    // Symbol name (string table index)
	uint32_t      st_value;   // Symbol value
	uint32_t      st_size;    // Symbol size
	unsigned char st_info;    // Symbol type and binding
	unsigned char st_other;   // Symobl visibility
	uint16_t      st_shndx;   // Section index
};

// 64-bit symbol table entry
struct elf64_sym {
	uint32_t      st_name;   // Symbol name (string table index)
	unsigned char st_info;   // Symbol type and binding
	unsigned char st_other;  // Symbol visibility
	uint16_t      st_shndx;  // Section index
	uint64_t      st_value;  // Symbol value
	uint64_t      st_size;   // Symbol size
};

// 32-bit program segment header
struct elf32_phdr {
	uint32_t p_type;    // Segment type
	uint32_t p_offset;  // Segment file offset
	uint32_t p_vaddr;   // Segment virtual address
	uint32_t p_paddr;   // Segment physical address
	uint32_t p_filesz;  // Segment size in file
	uint32_t p_memsz;   // Segmet size in memory
	uint32_t p_flags;   // Segment flags
	uint32_t p_align;   // Segment alignment
};

// 64-bit program segment header
struct elf64_phdr {
	uint32_t p_type;    // Segment type
	uint32_t p_flags;   // Segment flags
	uint64_t p_offset;  // Segment file offset
	uint64_t p_vaddr;   // Segment virtual address
	uint64_t p_paddr;   // Segment physical address
	uint64_t p_filesz;  // Segment size in file
	uint64_t p_memsz;   // Segment size in memory
	uint64_t p_align;   // Segment alignment
};

// Legal values for p_type (segment type)
#define PT_NULL    0  // Program header table entry unused
#define PT_LOAD    1  // Loadable program segment
#define PT_DYNAMIC 2  // Dynamic linking info
#define PT_INTERP  3  // Program interpreter
#define PT_NOTE    4  // Auxiliary information
#define PT_SHLIB   5  // Reserved
#define PT_PHDR    6  // Entry for header table itself
#define PT_TLS     7  // Thread local storage segment

// Legal values for p_flags (segment flags)
#define PF_X  (1 << 0)  // Segment is executable
#define PF_W  (1 << 1)  // Segment is writable
#define PF_R  (1 << 2)  // Segment is readable
