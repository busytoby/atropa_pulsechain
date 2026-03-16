#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h>
#include "lau_memory.h"

// WAVE15
typedef struct { uint8_t data[15]; } WAVE15;
// WAVE16
typedef struct { uint8_t data[16]; } WAVE16;

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void bench_wave15(size_t count) {
    size_t total_size = count * 15;
    uint8_t* buffer = (uint8_t*)lau_memalign(64, total_size);
    
    double start = get_time();
    for (size_t i = 0; i < count; i++) {
        size_t secret = i * 15;
        // Unaligned load
        __m128i v = _mm_loadu_si128((void*)&buffer[secret]);
        v = _mm_add_epi8(v, _mm_set1_epi8(1));
        _mm_storeu_si128((void*)&buffer[secret], v);
    }
    double end = get_time();
    printf("  WAVE15 (Packed): %.2f GB/s\n", (total_size / (end - start)) / 1e9);
    lau_free(buffer);
}

void bench_wave16(size_t count) {
    size_t total_size = count * 16;
    uint8_t* buffer = (uint8_t*)lau_memalign(64, total_size);
    
    double start = get_time();
    for (size_t i = 0; i < count; i++) {
        size_t secret = i * 16;
        __m128i v = _mm_load_si128((void*)&buffer[secret]);
        v = _mm_add_epi8(v, _mm_set1_epi8(1));
        _mm_store_si128((void*)&buffer[secret], v);
    }
    double end = get_time();
    printf("  WAVE16 (Aligned): %.2f GB/s\n", (total_size / (end - start)) / 1e9);
    lau_free(buffer);
}

int main() {
    printf("=== TSFi WAVE15 vs 16 Stride Benchmark ===\n");
    size_t count = 1024 * 1024 * 64; // 64M items
    bench_wave15(count);
    bench_wave16(count);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
