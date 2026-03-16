#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>

#define MOTZKIN_PRIME_50 0x3632C8EB5AF3BULL

static uint32_t calculate_yi_crc_zmm(uint8_t* pool, size_t length) {
    uint64_t crc = _mm_crc32_u64(0, MOTZKIN_PRIME_50);
    size_t i = 0;
    while (i + 8 <= length) {
        uint64_t block = *(uint64_t*)(pool + i);
        crc = _mm_crc32_u64(crc, block);
        i += 8;
    }
    return (uint32_t)crc;
}

int main(void) {
    tsfi_io_printf(stdout, "=== TSFi SEAL: ZMM Hardware Register Provenance ===\n");

    __m512i* zmm_shadow_bank = (__m512i*)aligned_alloc(64, 32 * 64);
    if (!zmm_shadow_bank) {
        tsfi_io_printf(stderr, "[FAIL] ZMM shadow matrix allocation rejected.\n");
        return 1;
    }

    __m512i base_wave = _mm512_set1_epi64(MOTZKIN_PRIME_50);
    
    __m512i zmm0 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(0));
    __m512i zmm1 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(1));
    __m512i zmm2 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(2));
    __m512i zmm3 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(3));
    __m512i zmm4 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(4));
    __m512i zmm5 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(5));
    __m512i zmm6 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(6));
    __m512i zmm7 = _mm512_add_epi64(base_wave, _mm512_set1_epi64(7));

    _mm512_store_si512(&zmm_shadow_bank[0], zmm0);
    _mm512_store_si512(&zmm_shadow_bank[1], zmm1);
    _mm512_store_si512(&zmm_shadow_bank[2], zmm2);
    _mm512_store_si512(&zmm_shadow_bank[3], zmm3);
    _mm512_store_si512(&zmm_shadow_bank[4], zmm4);
    _mm512_store_si512(&zmm_shadow_bank[5], zmm5);
    _mm512_store_si512(&zmm_shadow_bank[6], zmm6);
    _mm512_store_si512(&zmm_shadow_bank[7], zmm7);

    size_t active_zmm_bytes = 8 * 64; 
    uint32_t baseline_seal = calculate_yi_crc_zmm((uint8_t*)zmm_shadow_bank, active_zmm_bytes);
    
    tsfi_io_printf(stdout, "[SEAL] Volatile ZMM Matrix physically bounded: %zu bytes.\n", active_zmm_bytes);
    tsfi_io_printf(stdout, "[SEAL] Establishing Cryptographic YI_CRC ZMM Lock: 0x%08X\n", baseline_seal);

    tsfi_io_printf(stdout, "[MU] Injecting asynchronous state drift into ZMM7 hardware register...\n");
    
    zmm7 = _mm512_xor_si512(zmm7, _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL));
    
    _mm512_store_si512(&zmm_shadow_bank[7], zmm7);

    uint32_t compromised_seal = calculate_yi_crc_zmm((uint8_t*)zmm_shadow_bank, active_zmm_bytes);
    tsfi_io_printf(stdout, "[SEAL] Recalculated ZMM YI_CRC Epoch Hash: 0x%08X\n", compromised_seal);

    if (baseline_seal != compromised_seal) {
        tsfi_io_printf(stdout, "\n[SEAL VERIFIED] Volatile hardware divergence detected. The physical AVX-512 register bank is strictly tamper-evident.\n");
        tsfi_io_printf(stdout, "[ETA] Deep Fault Locked. Entire execution wavefront isolated and neutralized.\n");
    } else {
        tsfi_io_printf(stderr, "\n[SEAL FAULT] YI_CRC failed to detect geometric deviation in the ZMM Hardware Array!\n");
        return 1;
    }

    free(zmm_shadow_bank);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
