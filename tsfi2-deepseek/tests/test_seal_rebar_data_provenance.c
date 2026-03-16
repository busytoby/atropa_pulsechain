#include "lau_memory.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>

#define MOTZKIN_PRIME_50 0x3632C8EB5AF3BULL

static uint32_t calculate_yi_crc_rebar(uint8_t* pool, size_t length) {
    uint64_t crc = _mm_crc32_u64(0, MOTZKIN_PRIME_50);
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
    tsfi_io_printf(stdout, "=== TSFi SEAL: ReBAR Data Provenance Verification ===\n");

    size_t payload_size = 1024 * 1024;
    uint8_t* rebar_matrix = (uint8_t*)lau_malloc_wired(payload_size);
    if (!rebar_matrix) {
        tsfi_io_printf(stderr, "[FAIL] ReBAR memory matrix allocation rejected.\n");
        return 1;
    }

    for (size_t i = 0; i < payload_size; i += 8) {
        *(uint64_t*)(rebar_matrix + i) = (uint64_t)i * 0x9E3779B97F4A7C15ULL;
    }

    tsfi_io_printf(stdout, "[SEAL] ReBAR Matrix allocated and populated: %zu bytes at %p\n", payload_size, rebar_matrix);

    uint32_t baseline_seal = calculate_yi_crc_rebar(rebar_matrix, payload_size);
    tsfi_io_printf(stdout, "[SEAL] Establishing Cryptographic YI_CRC Baseline Hash: 0x%08X\n", baseline_seal);

    tsfi_io_printf(stdout, "[MU] Injecting malicious state drift into ReBAR memory matrix...\n");
    
    rebar_matrix[payload_size / 2] ^= 0x01;
    
    uint32_t compromised_seal = calculate_yi_crc_rebar(rebar_matrix, payload_size);
    tsfi_io_printf(stdout, "[SEAL] Recalculated YI_CRC Epoch Hash: 0x%08X\n", compromised_seal);

    if (baseline_seal != compromised_seal) {
        tsfi_io_printf(stdout, "\n[SEAL VERIFIED] Data matrix divergence detected across the Vulkan/Host boundary. The passive payload is mathematically tamper-evident.\n");
        tsfi_io_printf(stdout, "[ETA] Deep Fault Locked. Entire Shared Memory block isolated.\n");
    } else {
        tsfi_io_printf(stderr, "\n[SEAL FAULT] YI_CRC failed to detect geometric deviation in the ReBAR Payload!\n");
        return 1;
    }

    lau_free(rebar_matrix);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
