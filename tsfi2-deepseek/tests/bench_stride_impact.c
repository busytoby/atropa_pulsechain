#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <immintrin.h>
#include "tsfi_wave15.h"
#include "lau_memory.h"

// VGP is 120 bytes
typedef struct {
    float voxels[16]; // 64
    uint32_t op_ctrl; // 4
    uint32_t mutation_id; // 4
    uint64_t _pad[6]; // 48
    // Total 120. (64+4+4+48 = 120)
} __attribute__((packed)) VGP_120;

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// 1. Pure Packed (Stride 120)
void bench_pure_packed(size_t count) {
    size_t total_size = count * sizeof(VGP_120);
    uint8_t* buffer = (uint8_t*)lau_memalign(64, total_size);
    memset(buffer, 0, total_size);
    
    double start = get_time();
    
    for (size_t i = 0; i < count; i++) {
        size_t secret = i * sizeof(VGP_120);
        __m512 v1 = _mm512_loadu_ps(&buffer[secret]);
        __m512 v2 = _mm512_loadu_ps(&buffer[secret + 56]);
        
        v1 = _mm512_add_ps(v1, _mm512_set1_ps(1.0f));
        
        _mm512_storeu_ps(&buffer[secret], v1);
        _mm512_storeu_ps(&buffer[secret + 56], v2);
    }
    
    double end = get_time();
    printf("  Pure Packed (120B): %.2f GB/s\n", (total_size / (end - start)) / 1e9);
    lau_free(buffer);
}

// 2. Aligned Padded (Stride 128)
void bench_aligned_padded(size_t count) {
    size_t stride = 128;
    size_t total_size = count * stride;
    uint8_t* buffer = (uint8_t*)lau_memalign(64, total_size);
    memset(buffer, 0, total_size);
    
    double start = get_time();
    
    for (size_t i = 0; i < count; i++) {
        size_t secret = i * stride;
        __m512 v1 = _mm512_load_ps(&buffer[secret]);
        __m512 v2 = _mm512_load_ps(&buffer[secret + 64]);
        
        v1 = _mm512_add_ps(v1, _mm512_set1_ps(1.0f));
        
        _mm512_store_ps(&buffer[secret], v1);
        _mm512_store_ps(&buffer[secret + 64], v2);
    }
    
    double end = get_time();
    printf("  Aligned Padded (128B): %.2f GB/s\n", (total_size / (end - start)) / 1e9);
    lau_free(buffer);
}

// 3. Block Packed (Stride 2048, 17 items)
void bench_block_packed(size_t count) {
    size_t items_per_block = 17;
    size_t block_size = 2048;
    size_t num_blocks = (count + items_per_block - 1) / items_per_block;
    size_t total_size = num_blocks * block_size;
    
    uint8_t* buffer = (uint8_t*)lau_memalign(64, total_size);
    memset(buffer, 0, total_size);
    
    double start = get_time();
    
    size_t processed = 0;
    for (size_t b = 0; b < num_blocks; b++) {
        size_t block_offset = b * block_size;
        for (size_t i = 0; i < items_per_block; i++) {
            if (processed >= count) break;
            size_t item_offset = block_offset + i * 120;
            
            __m512 v1 = _mm512_loadu_ps(&buffer[item_offset]);
            __m512 v2 = _mm512_loadu_ps(&buffer[item_offset + 56]);
            
            v1 = _mm512_add_ps(v1, _mm512_set1_ps(1.0f));
            
            _mm512_storeu_ps(&buffer[item_offset], v1);
            _mm512_storeu_ps(&buffer[item_offset + 56], v2);
            processed++;
        }
    }
    
    double end = get_time();
    printf("  Block Packed (2KB/17): %.2f GB/s\n", (total_size / (end - start)) / 1e9);
    lau_free(buffer);
}

int main() {
    printf("=== TSFi VGP Stride Impact Benchmark ===\n");
    size_t count = 1024 * 1024 * 8;
    
    bench_pure_packed(count);
    bench_aligned_padded(count);
    bench_block_packed(count);
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
