#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_core_rope.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #10 Core-Rope ROM Test ===\n");

    uint16_t test_data[CORE_ROPE_WORDS] = {
        0x1234, 0xABCD, 0x5555, 0xAAAA,
        0x0000, 0xFFFF, 0x0F0F, 0xF0F0,
        0x1111, 0x2222, 0x4444, 0x8888,
        0x9ABC, 0xDEF0, 0x7777, 0xEEEE
    };

    TsfiCoreRopeMemory mem;
    tsfi_core_rope_init(&mem, test_data);

    // Test readings for all words
    for (int w = 0; w < CORE_ROPE_WORDS; w++) {
        float voltages[CORE_ROPE_BITS];
        uint16_t read_val = tsfi_core_rope_read(&mem, w, voltages);
        
        printf("[CORE-ROPE] Core %d: Expected = 0x%04X, Read = 0x%04X\n", w, test_data[w], read_val);
        assert(read_val == test_data[w]);

        // Validate voltage levels
        for (int b = 0; b < CORE_ROPE_BITS; b++) {
            if (test_data[w] & (1 << b)) {
                // Threaded through -> expects ~5V peak
                assert(fabs(voltages[b] - 5.0f) < 1e-5);
            } else {
                // Bypassed -> expects 0V
                assert(fabs(voltages[b] - 0.0f) < 1e-5);
            }
        }
    }

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_core_rope", NULL);

    printf("[PASS] Core-Rope Read-Only Memory simulation verified.\n");
    return 0;
}
