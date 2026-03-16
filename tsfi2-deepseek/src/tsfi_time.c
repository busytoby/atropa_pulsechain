#include "tsfi_time.h"
#include "tsfi_wave512.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

#include <elf.h>
#include <sys/auxv.h>

int (*g_vdso_clock_gettime)(int, struct timespec *) = NULL;

__attribute__((constructor))
void tsfi_init_vdso(void) {
    if (g_vdso_clock_gettime) return;
    
    unsigned long vdso_base = getauxval(AT_SYSINFO_EHDR);
    if (!vdso_base) return;

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)vdso_base;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) return;

    Elf64_Phdr *phdr = (Elf64_Phdr *)(vdso_base + ehdr->e_phoff);
    Elf64_Dyn *dyn = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)(vdso_base + phdr[i].p_offset);
            break;
        }
    }
    if (!dyn) return;

    Elf64_Sym *symtab = NULL;
    const char *strtab = NULL;
    
    for (int i = 0; dyn[i].d_tag != DT_NULL; i++) {
        if (dyn[i].d_tag == DT_SYMTAB) symtab = (Elf64_Sym *)(vdso_base + dyn[i].d_un.d_ptr);
        if (dyn[i].d_tag == DT_STRTAB) strtab = (const char *)(vdso_base + dyn[i].d_un.d_ptr);
    }
    
    if (!symtab || !strtab) return;

    size_t num_syms = 1024;
    if ((const char*)strtab > (const char*)symtab) {
        num_syms = ((const char*)strtab - (const char*)symtab) / sizeof(Elf64_Sym);
    }

    for (size_t i = 0; i < num_syms; i++) {
        if (symtab[i].st_name != 0) {
            const char *name = strtab + symtab[i].st_name;
            if (strcmp(name, "__vdso_clock_gettime") == 0 || strcmp(name, "clock_gettime") == 0) {
                g_vdso_clock_gettime = (int (*)(int, struct timespec *))(vdso_base + symtab[i].st_value);
                return;
            }
        }
    }
}


unsigned long long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void tsfi_time_wait_ns(long long ns) {
    unsigned long long start = get_time_ns();
    while (get_time_ns() - start < (unsigned long long)ns) {
        __asm__ volatile ("pause"); 
    }
}

void tsfi_time_burn_avx512_ns(long long ns) {
    unsigned long long start = get_time_ns();
    wave512 d1 = wave512_set1(1.000001f);
    wave512 d2 = wave512_set1(0.999999f);
    wave512 acc = VZERO();
    while (get_time_ns() - start < (unsigned long long)ns) {
        acc = VFMADD213PS(acc, d1, d2);
        acc = VFMADD213PS(acc, d1, d2);
        acc = VFMADD213PS(acc, d1, d2);
        acc = VFMADD213PS(acc, d1, d2);
    }
    if (acc.z[0][0] == 12345.0f) tsfi_io_printf(stdout, "Impossible\n");
}

void measure_frequency() {
    unsigned long long start_ns = get_time_ns();
    unsigned long long start_tsc = __rdtsc();
    tsfi_time_burn_avx512_ns(100000000); 
    unsigned long long end_ns = get_time_ns();
    unsigned long long end_tsc = __rdtsc();
    double elapsed_sec = (double)(end_ns - start_ns) / 1e9;
    double cycles = (double)(end_tsc - start_tsc);
        double freq_ghz = (cycles / elapsed_sec) / 1e9;
        tsfi_io_printf(stdout, "[TSFI_TIME] Estimated Wave512 Frequency: %.3f GHz\n", freq_ghz);
    }
    