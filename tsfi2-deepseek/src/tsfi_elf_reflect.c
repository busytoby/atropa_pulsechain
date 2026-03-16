#include "tsfi_elf_reflect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ELF64 Data Structures (Minimal for our needs)
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    Elf64_Word    p_type;
    Elf64_Word    p_flags;
    Elf64_Off     p_offset;
    Elf64_Addr    p_vaddr;
    Elf64_Addr    p_paddr;
    Elf64_Xword   p_filesz;
    Elf64_Xword   p_memsz;
    Elf64_Xword   p_align;
} Elf64_Phdr;

typedef struct {
    Elf64_Word    st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half    st_shndx;
    Elf64_Addr    st_value;
    Elf64_Xword   st_size;
} Elf64_Sym;

#define PT_DYNAMIC  2
#define DT_NULL     0
#define DT_PLTGOT   3
#define DT_HASH     4
#define DT_STRTAB   5
#define DT_SYMTAB   6
#define DT_GNU_HASH 0x6ffffef5

typedef struct {
    Elf64_Xword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr  d_ptr;
    } d_un;
} Elf64_Dyn;

// Very basic implementation: 
// 1. Find our own base address via /proc/self/maps.
// 2. Parse ELF header -> Program headers -> Dynamic segment.
// 3. Find .dynsym and .dynstr.
// 4. Linear scan over symbols to find the best match (closest symbol address <= target address).

static Elf64_Addr get_base_address() {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    
    char line[512];
    Elf64_Addr base = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "r-xp") || strstr(line, "r--p")) {
            unsigned long start, end;
            if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
                // If it's the very first entry or our binary path, this is base.
                // We'll simplify and just take the very first readable map.
                if (base == 0) {
                    base = start;
                }
                if (strstr(line, "/tsfi2")) {
                    base = start;
                    break;
                }
            }
        }
    }
    fclose(f);
    return base;
}

int tsfi_raw_dladdr(void *addr, TSFiDlInfo *info) {
    if (!addr || !info) return 0;
    
    memset(info, 0, sizeof(TSFiDlInfo));
    info->dli_fname = "/proc/self/exe"; // Assuming self
    
    Elf64_Addr target = (Elf64_Addr)addr;
    Elf64_Addr base = get_base_address();
    if (!base) return 0;
    
    info->dli_fbase = (void*)base;
    
    Elf64_Ehdr *ehdr = (Elf64_Ehdr*)base;
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        return 0; // Not an ELF
    }
    
    Elf64_Phdr *phdr = (Elf64_Phdr*)(base + ehdr->e_phoff);
    Elf64_Dyn *dyn = NULL;
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)(base + phdr[i].p_vaddr);
            break;
        }
    }
    
    if (!dyn) return 0;
    
    Elf64_Sym *symtab = NULL;
    char *strtab = NULL;
    uint32_t *hashtab = NULL;
    
    for (Elf64_Dyn *d = dyn; d->d_tag != DT_NULL; d++) {
        if (d->d_tag == DT_SYMTAB) symtab = (Elf64_Sym*)d->d_un.d_ptr;
        if (d->d_tag == DT_STRTAB) strtab = (char*)d->d_un.d_ptr;
        if (d->d_tag == DT_HASH) hashtab = (uint32_t*)d->d_un.d_ptr;
        // In a real robust implementation, we'd handle DT_GNU_HASH for number of symbols.
    }
    
    if (!symtab || !strtab) return 0;
    
    // Determine number of symbols.
    // If we have DT_HASH, nchain is at index 1.
    uint32_t num_syms = 0;
    if (hashtab) {
        num_syms = hashtab[1];
    } else {
        // Fallback: Just guess a reasonable upper bound for dynamic symbols.
        num_syms = 10000;
    }
    
    Elf64_Sym *best_sym = NULL;
    
    for (uint32_t i = 0; i < num_syms; i++) {
        // Must be a function or object with a size.
        if (symtab[i].st_shndx == 0) continue; // Undefined
        
        Elf64_Addr sym_addr = base + symtab[i].st_value;
        if (symtab[i].st_value == 0) continue; // Skip absolute symbols not mapped
        
        if (target >= sym_addr && target < sym_addr + symtab[i].st_size) {
            // Perfect match inside bounds
            best_sym = &symtab[i];
            break;
        } else if (target >= sym_addr && (!best_sym || sym_addr > (base + best_sym->st_value))) {
            // Closest preceding symbol
            best_sym = &symtab[i];
        }
    }
    
    if (best_sym) {
        info->dli_sname = strtab + best_sym->st_name;
        info->dli_saddr = (void*)(base + best_sym->st_value);
        return 1;
    }
    
    return 1; // We found the base, but no specific symbol
}
