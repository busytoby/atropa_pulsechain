#define _GNU_SOURCE
#include "tsfi_hotloader.h"
#include <dlfcn.h>
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <x86intrin.h>
#include <unistd.h>
#include <elf.h>

static uint32_t calculate_yi_crc(uint8_t* pool, size_t length) {
    uint64_t crc = 0;
    size_t i = 0;
    while (i + 8 <= length) {
        uint64_t block = *(uint64_t*)(pool + i);
        crc = _mm_crc32_u64(crc, block);
        i += 8;
    }
    while (i < length) {
        crc = _mm_crc32_u8((uint32_t)crc, pool[i]);
        i++;
    }
    return (uint32_t)crc;
}

int main(void) {
    tsfi_io_printf(stdout, "=== TSFi SEAL: Deep ELF Provenance Verification ===\n");

    const char* thunk_path = "thunks/test_deep_seal.c";
    FILE* f = fopen(thunk_path, "w");
    if (f) {
        fprintf(f, "int test_deep_seal(void) { return 42; }\n");
        fclose(f);
    } else {
        return 1;
    }

    void* active_ptr = tsfi_hotload_thunk(thunk_path, "test_deep_seal");
    if (!active_ptr) {
        tsfi_io_printf(stderr, "[FAIL] Hotloader rejected baseline compilation.\n");
        return 1;
    }

    Dl_info info;
    if (dladdr(active_ptr, &info) == 0) {
        tsfi_io_printf(stderr, "[FAIL] Native ELF reflection failed to resolve bounds.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SEAL] Symbol resolved at: %p\n", info.dli_saddr);
    tsfi_io_printf(stdout, "[SEAL] Physical Matrix Base (dli_fbase): %p\n", info.dli_fbase);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)info.dli_fbase;
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 || 
        ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        tsfi_io_printf(stderr, "[FAIL] Invalid ELF magic signature at matrix base.\n");
        return 1;
    }

    Elf64_Phdr* phdr = (Elf64_Phdr*)((uint8_t*)ehdr + ehdr->e_phoff);
    uint8_t* exec_start = NULL;
    size_t exec_size = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            exec_start = (uint8_t*)ehdr + phdr[i].p_vaddr;
            exec_size = phdr[i].p_memsz;
            break;
        }
    }

    if (!exec_start || exec_size == 0) {
        tsfi_io_printf(stderr, "[FAIL] No physical PROT_EXEC segment discovered in Standard Cell.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SEAL] Discovered geometric .text segment: %zu bytes at %p\n", exec_size, exec_start);

    uint32_t baseline_seal = calculate_yi_crc(exec_start, exec_size);
    tsfi_io_printf(stdout, "[SEAL] Establishing Deep YI_CRC Baseline Hash: 0x%08X\n", baseline_seal);

    tsfi_io_printf(stdout, "[MU] Injecting malicious state drift into Standard Cell execution matrix...\n");
    
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = ((uintptr_t)exec_start) & ~(page_size - 1);
    size_t mprotect_len = ((uintptr_t)exec_start + exec_size - page_start + page_size - 1) & ~(page_size - 1);
    
    if (mprotect((void*)page_start, mprotect_len, PROT_READ | PROT_WRITE) != 0) {
        perror("mprotect unseal failed");
        return 1;
    }
    
    
    if (mprotect((void*)page_start, mprotect_len, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect reseal failed");
        return 1;
    }

    uint32_t compromised_seal = calculate_yi_crc(exec_start, exec_size);
    tsfi_io_printf(stdout, "[SEAL] Recalculated Deep YI_CRC Epoch Hash: 0x%08X\n", compromised_seal);

    if (baseline_seal != compromised_seal) {
        tsfi_io_printf(stdout, "\n[SEAL VERIFIED] Matrix divergence detected across the external boundary. The Standard Cell payload is structurally tamper-evident.\n");
        tsfi_io_printf(stdout, "[ETA] Deep Fault Locked. Entire Shared Object invalidated.\n");
    } else {
        tsfi_io_printf(stderr, "\n[SEAL FAULT] YI_CRC failed to detect geometric deviation in the Standard Cell!\n");
        return 1;
    }

    unlink(thunk_path);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
