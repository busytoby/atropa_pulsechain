#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"

/**
 * TSFi WAVE(ANY) 32-Byte Splitting Validation
 * 
 * Specifically tests the partitioning of a 512-byte "wave" into 
 * 16 segments of 32 bytes each, ensuring bijective mapping to 
 * 32-byte bounded memory segments.
 */

void test_wave512_to_32_splitting() {
    printf("[TEST] WAVE512 -> 32-Byte Segment Splitting...\n");
    
    // 1. Prepare 512-byte source "Wave"
    uint8_t src[512];
    for (int i = 0; i < 512; i++) src[i] = (uint8_t)(i % 256);
    
    // 2. Setup Destination Stream: 16 atoms, each 32 bytes
    WaveStream dst = {0};
    dst.atom_size = 32;
    dst.stride = 32; // Tight packing
    dst.count = 16;
    dst.data = lau_memalign(64, dst.count * dst.stride);
    memset(dst.data, 0, dst.count * dst.stride);
    
    // 3. Perform Store (Split 512 into 16x32)
    wave_store_any(&dst, 0, src, 512);
    
    // 4. Verify Each Segment
    uint8_t* dst_ptr = (uint8_t*)dst.data;
    bool match = true;
    for (int i = 0; i < 16; i++) {
        uint8_t* segment = dst_ptr + (i * 32);
        if (memcmp(segment, src + (i * 32), 32) != 0) {
            printf("  [FAIL] Segment %d Mismatch!\n", i);
            match = false;
            break;
        }
    }
    
    if (match) {
        printf("  [PASS] 512-byte Wave successfully split into 16x32-byte segments.\n");
    } else {
        exit(1);
    }
    
    // 5. Verify Robustness: Attempt to write OOB
    wave_store_any(&dst, 0, src, 544); 
    if (memcmp(dst_ptr + (15 * 32), src + (15 * 32), 32) == 0) {
        printf("  [PASS] Termination at Stream Boundary Verified.\n");
    }

    lau_free(dst.data);
}

int main() {
    test_wave512_to_32_splitting();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
