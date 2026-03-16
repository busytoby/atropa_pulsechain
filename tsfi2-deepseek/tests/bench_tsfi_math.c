#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <string.h>
#include "tsfi_math.h"
#include "lau_memory.h"

// Simple RDTSC wrapper

static void hex_to_bn(TSFiBigInt *bn, const char *hex) {
    if (strncmp(hex, "0x", 2) == 0) hex += 2;
    size_t len = strlen(hex);
    size_t byte_len = (len + 1) / 2;
    uint8_t *bytes = (uint8_t*)lau_malloc(byte_len);
    memset(bytes, 0, byte_len);
    for (size_t i = 0; i < len; i++) {
        char c = hex[len - 1 - i];
        int val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        bytes[byte_len - 1 - (i / 2)] |= (val << ((i % 2) * 4));
    }
    tsfi_bn_from_bytes(bn, bytes, byte_len);
    lau_free(bytes);
}

static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

int main() {
    printf("Starting TSFi Math Benchmark (Helmholtz Core)...\\n");

    // 1. Allocation
    printf("[1] Allocation Stress (10k allocs)...\\n");
    uint64_t start = rdtsc();
    for (int i=0; i<10000; i++) {
        TSFiBigInt *b = tsfi_bn_alloc();
        tsfi_bn_free(b);
    }
    uint64_t end = rdtsc();
    printf("    -> %lu cycles (avg %lu)\\n", end-start, (end-start)/10000);

    // 2. AVX512 Multiplication
    printf("[2] AVX512 Multiplication (1k ops)...\\n");
    TSFiBigInt *a = tsfi_bn_alloc();
    TSFiBigInt *b = tsfi_bn_alloc();
    TSFiBigInt *r = tsfi_bn_alloc();
    
    // Fill with pattern
    tsfi_bn_set_u64(a, 0xDEADBEEFCAFEBABE);
    tsfi_bn_set_u64(b, 0x1234567890ABCDEF);
    // Expand to used limbs
    a->active_limbs = TSFI_NUM_LIMBS / 2;
    b->active_limbs = TSFI_NUM_LIMBS / 2;
    for(int i=1; i<TSFI_NUM_LIMBS/2; i++) {
        a->limbs[i] = a->limbs[i-1] + 1;
        b->limbs[i] = b->limbs[i-1] * 2;
    }

    start = rdtsc();
    for (int i=0; i<1000; i++) {
        tsfi_bn_mul_avx512(r, a, b);
        // Avoid optimization
        __asm__ volatile("" : : "r"(r));
    }
    end = rdtsc();
    printf("    -> %lu cycles (avg %lu)\\n", end-start, (end-start)/1000);

    // 3. ModPow
    printf("[3] ModPow AVX512 (100 ops)...\\n");
    // Reduce size for reasonable time
    a->active_limbs = 4; // 256 bits approx
    b->active_limbs = 4;
    TSFiBigInt *m = tsfi_bn_alloc();
    m->active_limbs = 4;
    tsfi_bn_set_u64(m, 0xFFFFFFFFFFFFFFC5); // Prime-ish

    start = rdtsc();
    for (int i=0; i<100; i++) {
        tsfi_bn_modpow_avx512(r, a, b, m);
    }
    end = rdtsc();
    printf("    -> %lu cycles (avg %lu)\\n", end-start, (end-start)/100);

    tsfi_bn_free(a);
    tsfi_bn_free(b);
    tsfi_bn_free(r);
    tsfi_bn_free(m);

    // 4. Real-World Hex ModPow
    printf("[4] ModPow AVX512 (Real-World Hex 256-bit, 10k ops)...\\n");
    TSFiBigInt *a_hex = tsfi_bn_alloc();
    TSFiBigInt *b_hex = tsfi_bn_alloc();
    TSFiBigInt *m_hex = tsfi_bn_alloc();
    hex_to_bn(a_hex, "0xcc3a7079fe446dba713ea03991ade7381d1bac58d4fd4732623824f036d98c01");
    hex_to_bn(b_hex, "0x88a51eb9e9e557460fd781ef2ba69ff7e09091a7f98458b9d526da60b5132728");
    hex_to_bn(m_hex, "0xcc3a7079fe446dba713ea03991ade7381d1bac58d4fd4732623824f036d98c01");
    
    start = rdtsc();
    for (int i=0; i<10000; i++) {
        tsfi_bn_modpow_avx512(r, a_hex, b_hex, m_hex);
    }
    end = rdtsc();
    printf("    -> %lu cycles (avg %lu)\\n", end-start, (end-start)/10000);

    tsfi_bn_free(a_hex);
    tsfi_bn_free(b_hex);
    tsfi_bn_free(m_hex);

    printf("Done.\\n");

    extern void tsfi_bn_drain_pool(void);
    tsfi_bn_drain_pool();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}