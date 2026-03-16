#include <nmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lau_memory.h"

// Scalar baseline verification
static int verify_all_scalar(LauHeader **headers, int count) {
    int valid = 0;
    for (int i = 0; i < count; i++) {
        uint32_t crc = 0;
        crc = _mm_crc32_u64(crc, headers[i]->meta.alloc_size & 0x007FFFFFFFFFFFFFULL);
        crc = _mm_crc32_u64(crc, headers[i]->meta.instruction_hash);
        if (crc == headers[i]->footer.checksum) valid++;
    }
    return valid;
}

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main() {
    printf("=== TSFi Benchmark: Scramble Verification Baseline ===\n");
    const int count = 10000;
    LauHeader **headers = malloc(count * sizeof(LauHeader*));
    for (int i = 0; i < count; i++) {
        headers[i] = malloc(sizeof(LauHeader));
        headers[i]->meta.alloc_size = 1024;
        headers[i]->meta.instruction_hash = i;
        uint32_t crc = 0;
        crc = _mm_crc32_u64(crc, headers[i]->meta.alloc_size & 0x007FFFFFFFFFFFFFULL);
        crc = _mm_crc32_u64(crc, headers[i]->meta.instruction_hash);
        headers[i]->footer.checksum = crc;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int total_valid = 0;
    for (int i = 0; i < 1000; i++) {
        total_valid += verify_all_scalar(headers, count);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double total_ms = get_ms(t0, t1);
    printf("[BENCH] 10M Verifications: %.4f ms\n", total_ms);
    printf("[BENCH] Throughput: %.2f M-Verifications/sec\n", 10.0 / (total_ms / 1000.0));

    for (int i = 0; i < count; i++) free(headers[i]);
    free(headers);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return total_valid > 0 ? 0 : 1;
}
