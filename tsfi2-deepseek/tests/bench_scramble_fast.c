#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lau_memory.h"

int tsfi_scramble_verify_8way_avx512(const uint32_t *expected_crcs, const LauFooter *footers);

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main() {
    printf("=== TSFi Benchmark: Fast AVX-512 Scramble Verification ===\n");
    const int count = 10000;
    
    uint32_t *expected = aligned_alloc(64, count * sizeof(uint32_t));
    LauFooter *footers = aligned_alloc(64, count * sizeof(LauFooter));
    
    for (int i = 0; i < count; i++) {
        expected[i] = i;
        footers[i].checksum = i;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int total_valid = 0;
    for (int i = 0; i < 10000; i++) {
        // Verify in banks of 8
        for (int j = 0; j < count; j += 8) {
            total_valid += tsfi_scramble_verify_8way_avx512(&expected[j], &footers[j]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double total_ms = get_ms(t0, t1);
    printf("[BENCH] 100M Verifications: %.4f ms\n", total_ms);
    printf("[BENCH] Throughput: %.2f M-Verifications/sec\n", 100.0 / (total_ms / 1000.0));

    free(expected); free(footers);
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return total_valid > 0 ? 0 : 1;
}
