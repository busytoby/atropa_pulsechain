#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"

double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void bench_atom_size(const char* name, uint32_t atom_size, uint32_t stride, size_t total_bytes) {
    uint32_t count = total_bytes / stride;
    void* buffer = lau_memalign(64, total_bytes);
    memset(buffer, 0, total_bytes);
    
    WaveStream stream = {0};
    stream.data = buffer;
    stream.count = count;
    stream.atom_size = (uint16_t)atom_size;
    stream.stride = (uint16_t)stride;

    double start = get_time_s();
    
    // Simulate processing loop
    uint8_t* raw = (uint8_t*)stream.data;
    for (uint32_t i = 0; i < count; i += 16) {
        if (i + 16 > count) break;
        // Load block
        __m512i v = _mm512_loadu_si512(raw + i * stride);
        // Dummy mutation
        v = _mm512_add_epi32(v, _mm512_set1_epi32(1));
        // Store back
        _mm512_storeu_si512(raw + i * stride, v);
    }
    
    double end = get_time_s();
    double gb_s = (total_bytes / (end - start)) / 1e9;
    
    printf("  %s (Atom: %dB, Stride: %dB): %.2f GB/s\n", name, atom_size, stride, gb_s);
    lau_free(buffer);
}

int main() {
    size_t total_bytes = 1024 * 1024 * 256; // 256MB
    printf("=== TSFi WAVE(Any) Universal Packing Throughput ===\n");
    
    bench_atom_size("WAVE7  (Sub-aligned)", 7, 8, total_bytes);
    bench_atom_size("WAVE15 (Standard)   ", 15, 16, total_bytes);
    bench_atom_size("WAVE23 (Irregular)  ", 23, 32, total_bytes);
    bench_atom_size("WAVE64 (Full ZMM)   ", 64, 64, total_bytes);
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}