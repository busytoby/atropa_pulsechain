#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_delay_line.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #2 Delay Line Test ===\n");

    const int delay_length = 8;
    TsfiDelayLineProcessor proc;
    tsfi_delay_line_init(&proc, delay_length);

    // 1. Write an 8-bit pattern: 1, 0, 1, 1, 0, 0, 1, 0
    uint8_t pattern[8] = {1, 0, 1, 1, 0, 0, 1, 0};
    printf("[DELAY LINE] Writing 8-bit pattern to acoustic medium: ");
    proc.write_enable = 1;
    for (int i = 0; i < 8; i++) {
        printf("%d ", pattern[i]);
        tsfi_delay_line_step(&proc, pattern[i]);
    }
    printf("\n");

    // 2. Recirculate and read for two full cycles (16 steps)
    proc.write_enable = 0;
    proc.recirculate_enable = 1;
    printf("[DELAY LINE] Recirculating pattern (WE=0, RE=1)...\n");

    uint8_t output_pattern[16];
    for (int i = 0; i < 16; i++) {
        output_pattern[i] = tsfi_delay_line_step(&proc, 0);
    }

    // Verify first loop iteration
    printf("  Loop 1 read: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", output_pattern[i]);
        assert(output_pattern[i] == pattern[i]);
    }
    printf("(Passed)\n");

    // Verify second loop iteration
    printf("  Loop 2 read: ");
    for (int i = 8; i < 16; i++) {
        printf("%d ", output_pattern[i]);
        assert(output_pattern[i] == pattern[i - 8]);
    }
    printf("(Passed)\n");

    tsfi_delay_line_free(&proc);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_delay_line", NULL);

    printf("[PASS] Magnetostrictive Delay Line emulation completed successfully.\n");
    return 0;
}
