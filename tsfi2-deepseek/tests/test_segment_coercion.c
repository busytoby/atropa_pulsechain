#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"

// Simulate L1 Cache Line Size
#define CACHE_LINE_SIZE 64
// Simulate ZMM Register Size
#define LARGE_MEMBER_SIZE 512

void test_coercion_vs_split() {
    printf("[TEST] Segment Coercion vs WAVE(ANY) Splitting...\n");

    // 1. Prepare Source Data (512 bytes of "Large Member")
    uint8_t src_data[LARGE_MEMBER_SIZE];
    for(int i=0; i<LARGE_MEMBER_SIZE; i++) src_data[i] = (uint8_t)(i & 0xFF);

    // 2. Prepare Destination "Memory Section" (Stream of Cache Lines)
    // We want to store 512 bytes. If atom_size=64, we need 8 atoms.
    size_t atom_count = 8; 
    WaveStream stream = {0};
    stream.atom_size = CACHE_LINE_SIZE; // 64 bytes (Small Section)
    stream.stride = CACHE_LINE_SIZE;    // Packed tightly
    stream.count = atom_count;
    stream.data = lau_memalign(CACHE_LINE_SIZE, atom_count * stream.stride);
    memset(stream.data, 0, atom_count * stream.stride);

    // --- CASE A: Coercion (Simulated) ---
    // Try to treat the first atom pointer as a 512-byte buffer.
    // In a real strict environment (e.g. GPU shared memory or segmented cache), 
    // writing beyond atom_size might act undefined or be clamped.
    // Here, we simulate the "risk" by checking if we overwrite bounds if we didn't have the stream abstraction.
    // (In C flat memory, this just works, but logically it violates the "Section" boundary).
    
    // --- CASE B: WAVE(ANY) Split ---
    // Use the API to "put" the large member into the stream.
    // It should detect atom_size < src_size and SPLIT the write across atoms.
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    wave_store_any(&stream, 0, src_data, LARGE_MEMBER_SIZE);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_nsec - start.tv_nsec) / 1e3;
    printf("  [METRIC] WAVE(ANY) Split Write Time: %.2f us\n", elapsed_us);

    // 3. Verify Data Integrity (Reassembly)
    // We load it back (which should also auto-join the split segments)
    uint8_t read_back[LARGE_MEMBER_SIZE];
    memset(read_back, 0, LARGE_MEMBER_SIZE);
    
    wave_load_any(&stream, 0, read_back, LARGE_MEMBER_SIZE);

    // Check byte-for-byte
    int errors = 0;
    for(int i=0; i<LARGE_MEMBER_SIZE; i++) {
        if (read_back[i] != src_data[i]) {
            printf("  [FAIL] Data Mismatch at byte %d: Expected %02X, Got %02X\n", i, src_data[i], read_back[i]);
            errors++;
            if (errors > 10) break;
        }
    }

    if (errors == 0) {
        printf("  [PASS] Data Integrity Verified. Large member successfully split across %zu segments.\n", atom_count);
    } else {
        printf("  [FAIL] Data Corruption Detected.\n");
        exit(1);
    }

    // 4. Verify Physical Layout (Inspection)
    // Ensure it ACTUALLY split across the atoms in memory.
    // Byte 64 should be at the start of Atom 1 (index 1).
    uint8_t* atom1_ptr = (uint8_t*)WAVE_PTR(&stream, 1);
    if (atom1_ptr[0] == src_data[64]) {
        printf("  [PASS] Physical Layout Verified. Byte 64 is at Atom 1 Start.\n");
    } else {
        printf("  [FAIL] Physical Layout Invalid. Byte 64 not found at Atom 1 Start.\n");
        exit(1);
    }

    lau_free(stream.data);
    printf("[PASS] Coercion Avoidance Verified.\n");
}

int main() {
    test_coercion_vs_split();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}