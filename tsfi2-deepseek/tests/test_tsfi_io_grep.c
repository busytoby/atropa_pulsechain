#include "tsfi_io.h"
#include "tsfi_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE (1024 * 1024 * 10) // 10 MB Buffer
#define MAX_MATCHES 100

int main() {
    tsfi_io_printf(stdout, "=== TSFi Native IO: AVX-512 Grep Validation ===\n");
    
    uint8_t* buffer = (uint8_t*)lau_memalign(64, BUFFER_SIZE);
    if (!buffer) {
        tsfi_io_printf(stderr, "[FAIL] Memory allocation failed.\n");
        return 1;
    }
    
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 'A' + (rand() % 26); 
    }

    const char* pattern = "SYSTEM_FAULT_DETECTED";
    size_t pat_len = strlen(pattern);
    
    memcpy(buffer + 1024, pattern, pat_len);
    memcpy(buffer + (1024 * 512) + 63, pattern, pat_len); 
    memcpy(buffer + BUFFER_SIZE - pat_len - 10, pattern, pat_len);

    size_t match_indices[MAX_MATCHES];
    
    uint64_t start = get_time_ns();
    size_t match_count = tsfi_io_grep(buffer, BUFFER_SIZE, pattern, pat_len, match_indices, MAX_MATCHES);
    uint64_t end = get_time_ns();

    double duration_ms = (end - start) / 1000000.0;
    double throughput_gbps = (BUFFER_SIZE / (1024.0 * 1024.0 * 1024.0)) / (duration_ms / 1000.0);

    tsfi_io_printf(stdout, "Pattern        : '%s'\n", pattern);
    tsfi_io_printf(stdout, "Matches Found  : %zu\n", match_count);
    tsfi_io_printf(stdout, "Scan Time      : %.2f ms\n", duration_ms);
    tsfi_io_printf(stdout, "SIMD Throughput: %.2f GB/s\n", throughput_gbps);

    if (match_count == 3 && 
        match_indices[0] == 1024 && 
        match_indices[1] == (1024 * 512) + 63 && 
        match_indices[2] == BUFFER_SIZE - pat_len - 10) {
        tsfi_io_printf(stdout, "[SUCCESS] Native AVX-512 Grep successfully locked all offsets without string.h dependencies.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Grep missed injections or reported false positives.\n");
        for (size_t i = 0; i < match_count; i++) {
            tsfi_io_printf(stderr, "  Match %zu at offset: %zu\n", i, match_indices[i]);
        }
        lau_free(buffer);
        return 1;
    }

    lau_free(buffer);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}