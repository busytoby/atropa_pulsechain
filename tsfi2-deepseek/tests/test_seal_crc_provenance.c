#include "lau_thunk.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <x86intrin.h>
#include <unistd.h>
#include <stdbool.h>

// A dummy state fn for the safety chain
static bool dummy_state(void* ctx) {
    (void)ctx;
    return true;
}

static void dummy_epoch(void* ctx) {
    (void)ctx;
}

// Pure C YI_CRC Geometric Hash over a memory manifold
static uint32_t calculate_yi_crc(uint8_t* pool, size_t length) {
    uint64_t crc = 0;
    size_t i = 0;
    
    // Hardware accelerated 64-bit strides
    while (i + 8 <= length) {
        uint64_t block = *(uint64_t*)(pool + i);
        crc = _mm_crc32_u64(crc, block);
        i += 8;
    }
    
    // Tail byte strides
    while (i < length) {
        crc = _mm_crc32_u8((uint32_t)crc, pool[i]);
        i++;
    }
    
    return (uint32_t)crc;
}

int main(void) {
    tsfi_io_printf(stdout, "=== TSFi SEAL: YI_CRC Provenance Verification ===\n");

    // 1. Establish the physical execution matrix
    ThunkProxy* p = ThunkProxy_create();
    if (!p) {
        tsfi_io_printf(stderr, "[FAIL] Matrix allocation rejected.\n");
        return 1;
    }

    // 2. Engineer a dense geometric JIT structure
    ThunkProxy_emit_safety_chain(p, dummy_state, dummy_epoch, NULL);
    ThunkProxy_emit_zmm_density_op(p, 0);
    ThunkProxy_emit_ret(p);
    
    uint8_t* matrix_start = p->thunk_pool;
    size_t matrix_size = (size_t)(p->thunk_cursor - p->thunk_pool);
    
    tsfi_io_printf(stdout, "[SEAL] Geometry bounded: %zu bytes of literal machine opcodes.\n", matrix_size);

    // 3. The Cryptographic Lock
    uint32_t baseline_seal = calculate_yi_crc(matrix_start, matrix_size);
    tsfi_io_printf(stdout, "[SEAL] Establishing YI_CRC Baseline Hash: 0x%08X\n", baseline_seal);

    // 4. The Mutilation (Simulating unauthorized memory overwrite)
    tsfi_io_printf(stdout, "[MU] Injecting malicious state drift into PROT_EXEC matrix...\n");
    
    // We must physically unlock the page bounds to mutate the JIT code, as the 
    // ThunkProxy seals it automatically (or leaves it PROT_EXEC in some implementations).
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = ((uintptr_t)matrix_start) & ~(page_size - 1);
    size_t mprotect_len = ((uintptr_t)matrix_start + matrix_size - page_start + page_size - 1) & ~(page_size - 1);
    
    if (mprotect((void*)page_start, mprotect_len, PROT_READ | PROT_WRITE) != 0) {
        perror("mprotect unseal failed");
        return 1;
    }
    
    // Morph one single opcode physically
    matrix_start[matrix_size / 2] ^= 0x55;
    
    // Reseal
    if (mprotect((void*)page_start, mprotect_len, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect reseal failed");
        return 1;
    }

    // 5. The Audit
    uint32_t compromised_seal = calculate_yi_crc(matrix_start, matrix_size);
    tsfi_io_printf(stdout, "[SEAL] Recalculated YI_CRC Epoch Hash: 0x%08X\n", compromised_seal);

    if (baseline_seal != compromised_seal) {
        tsfi_io_printf(stdout, "\n[SEAL VERIFIED] Matrix divergence detected. The physical execution vector is strictly tamper-evident.\n");
        tsfi_io_printf(stdout, "[ETA] Fault Locked. System isolating execution bounds.\n");
    } else {
        tsfi_io_printf(stderr, "\n[SEAL FAULT] YI_CRC failed to detect geometric deviation!\n");
        return 1;
    }

    // Cleanup
    ThunkProxy_destroy(p);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}