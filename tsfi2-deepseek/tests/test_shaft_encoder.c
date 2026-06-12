#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_shaft_encoder.h"

int count_set_bits(uint32_t n) {
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

int main() {
    printf("=== TSFi2 & Computer Design Issue #4 Shaft Encoder Test ===\n");

    const int bits = 8;
    const double pi2 = 2.0 * 3.14159265358979323846;
    
    // 1. Verify round-trip conversion for a sweep of 360 degrees
    printf("[ENCODER] Verifying 360-step sweep and Gray code round-trip...\n");
    uint32_t last_gray = 0;
    for (int step = 0; step < 360; step++) {
        double theta = ((double)step / 360.0) * pi2;
        
        uint32_t bin = tsfi_angle_to_binary(theta, bits);
        uint32_t gray = tsfi_binary_to_gray(bin);
        uint32_t decoded = tsfi_gray_to_binary(gray, bits);
        
        assert(bin == decoded);

        // 2. Verify unit-distance (Hamming distance of 1) property for consecutive Gray codes
        if (step > 0) {
            uint32_t diff = gray ^ last_gray;
            int bit_changes = count_set_bits(diff);
            
            // Note: Since 360 steps does not perfectly align with 256 quantization intervals,
            // consecutive angles may map to the same binary code (0 changes) or step by 1 (1 change).
            // Crucially, it must never exceed 1 bit change.
            assert(bit_changes <= 1);
        }
        last_gray = gray;
    }
    printf("  Verified Hamming distance <= 1 for sweep. (Passed)\n");

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_shaft_encoder", NULL);

    printf("[PASS] Binary Shaft Encoder and Gray Code translation verified.\n");
    return 0;
}
