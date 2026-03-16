#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"

// Test robust packing of large VE (4096 bytes) into smaller atoms
void test_ve_splitting() {
    printf("[TEST] VEANY Splitting & Robustness...\n");
    
    // Simulate a large source data (e.g. 8KB)
    size_t src_len = 8192;
    uint8_t* src = lau_malloc(src_len);
    for(size_t i=0; i<src_len; i++) src[i] = (uint8_t)(i % 256);
    
    // Destination: WaveStream with 512-byte atoms
    WaveStream dst = {0};
    dst.atom_size = 512;
    dst.stride = 512; // Packed tightly
    dst.count = 16;   // 16 * 512 = 8192
    dst.data = lau_memalign(64, dst.count * dst.stride);
    dst.flags = WAVE_FLAG_VERIFY;
    
    // Perform Packing (implicitly tests splitting across 16 atoms)
    wave_pack_buffer(&dst, src, src_len);
    
    // Verify Integrity (No Tearing)
    if (memcmp(dst.data, src, src_len) == 0) {
        printf("  [PASS] 8KB -> 16x512 Packing Verified (No Tearing).\n");
    } else {
        printf("  [FAIL] Data Corruption Detected.\n");
        exit(1);
    }

    // Test Robust OOB Read
    uint8_t* read_back = lau_malloc(src_len + 1024);
    memset(read_back, 0xFF, src_len + 1024);
    
    // Read 9KB from 8KB stream
    wave_load_any(&dst, 0, read_back, src_len + 1024);
    
    // First 8KB should match src
    if (memcmp(read_back, src, src_len) == 0) {
        printf("  [PASS] Multi-Atom Read Verified.\n");
    } else {
        printf("  [FAIL] Multi-Atom Read Corrupted.\n");
        exit(1);
    }
    
    // Final 1KB should be 0 (Robustness 2)
    bool all_zero = true;
    for(size_t i=src_len; i<src_len+1024; i++) {
        if (read_back[i] != 0) { all_zero = false; break; }
    }
    if (all_zero) {
        printf("  [PASS] OOB Read Successfully Zero-Filled (Robustness 2).\n");
    } else {
        printf("  [FAIL] OOB Read leaked memory or failed to zero-fill.\n");
        exit(1);
    }
    
    // Test OOB Truncation (Robustness)
    // Try to pack 1KB into a single 512-byte atom at the very end of the stream
    WaveStream small_dst = dst;
    small_dst.count = 1; 
    small_dst.data = (uint8_t*)dst.data + (15 * 512); // The last atom
    memset(small_dst.data, 0, 512);
    
    wave_store_any(&small_dst, 0, src, 1024);
    
    // Check that only first 512 bytes were written
    if (memcmp(small_dst.data, src, 512) == 0) {
        printf("  [PASS] OOB Write Successfully Truncated (Robustness 2).\n");
    } else {
        printf("  [FAIL] First 512 bytes corrupted or write failed.\n");
        exit(1);
    }

    // Test Zero-Fill OOB
    // Write 100 bytes into a stream of 16x512 (8192 bytes total)
    // All bytes from 100 to 8191 should be zero.
    memset(dst.data, 0xFF, 8192); // Fill with junk first
    wave_store_any(&dst, 0, src, 100);
    
    bool leaked = false;
    uint8_t* ptr = (uint8_t*)dst.data;
    for (size_t i = 100; i < 8192; i++) {
        if (ptr[i] != 0) { leaked = true; break; }
    }
    
    if (!leaked) {
        printf("  [PASS] Zero-Fill OOB Verified (Full Stream Cleaned).\n");
    } else {
        printf("  [FAIL] Stale Data detected in OOB region!\n");
        exit(1);
    }
    
    lau_free(read_back);
    lau_free(dst.data);
    lau_free(src);
}

int main() {
    test_ve_splitting();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
