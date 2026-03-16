#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"

// --- Domain Structures ---
typedef struct { uint8_t data[14]; uint8_t secret; } WAVE15_Layout;
typedef struct { uint8_t data[22]; uint8_t secret; } WAVE23_Layout;

// --- Universal Kernel ---
uint64_t kernel_sum_secrets(const WaveStream* s) {
    uint64_t total = 0;
    uint32_t secret_offset = s->atom_size - 1;
    for (uint32_t i = 0; i < s->count; i++) {
        uint8_t* ptr = (uint8_t*)WAVE_PTR(s, i);
        uint8_t val = ptr[secret_offset];
        total += val;
    }
    return total;
}

// --- Vectorized Universal Kernel ---
uint64_t kernel_sum_secrets_avx512(const WaveStream* s) {
    uint64_t total = 0;
    uint32_t secret_offset = s->atom_size - 1;
    for (uint32_t i = 0; i < s->count; i++) {
        alignas(64) uint8_t temp[64];
        wave_load_any(s, i, temp, 64);
        total += temp[secret_offset];
    }
    return total;
}

void test_universal_dispatch() {
    printf("[TEST] Universal Kernel Dispatch...\n");
    size_t count = 1000;
    WaveStream s15 = {0};
    s15.atom_size = 15; s15.stride = 16; s15.count = count;
    s15.data = lau_memalign(64, count * s15.stride);
    for(uint32_t i=0; i<count; i++) {
        WAVE15_Layout* w = (WAVE15_Layout*)((uint8_t*)s15.data + i*s15.stride);
        w->secret = 1;
    }
    WaveStream s23 = {0};
    s23.atom_size = 23; s23.stride = 32; s23.count = count;
    s23.data = lau_memalign(64, count * s23.stride);
    for(uint32_t i=0; i<count; i++) {
        WAVE23_Layout* w = (WAVE23_Layout*)((uint8_t*)s23.data + i*s23.stride);
        w->secret = 2;
    }
    uint64_t sum15 = kernel_sum_secrets(&s15);
    uint64_t sum23 = kernel_sum_secrets(&s23);
    assert(sum15 == count);
    assert(sum23 == count * 2);
    uint64_t vsum15 = kernel_sum_secrets_avx512(&s15);
    uint64_t vsum23 = kernel_sum_secrets_avx512(&s23);
    assert(vsum15 == count);
    assert(vsum23 == count * 2);
    lau_free(s15.data);
    lau_free(s23.data);
    printf("[PASS] Universal Dispatch Verified.\n");
}

int main() {
    test_universal_dispatch();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
