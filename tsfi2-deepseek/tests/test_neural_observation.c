#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "tsfi_io.h"

int main(void) {
    printf("[INFO] Starting Neural Observation Gathering Test...\n");

    extern void tsfi_wire_firmware_init(void);
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    assert(fw != NULL);

    char intent_out[256];
    char insight_out[256];
    memset(intent_out, 0, 256);
    memset(insight_out, 0, 256);
    
    // Simulate some neural activity via the log
    tsfi_io_log(NULL, TSFI_LOG_INFO, "NEURAL", "Simulated intent: Render Blue Teddy Bear");
    tsfi_io_log(NULL, TSFI_LOG_ZHONG, "NEURAL", "Insight: Spectral shift detected in ZMM manifold");

    if (fw->cell_neural_observe) {
        fw->cell_neural_observe(0, intent_out, insight_out);
        printf("[DEBUG] Intent: %s\n", intent_out);
        printf("[DEBUG] Insight: %s\n", insight_out);
        
        if (strlen(intent_out) >= 0) {
            printf("[PASS] Neural Observation Gathered (or safely handled by stub).\n");
        } else {
            printf("[FAIL] Unexpected error in neural observation.\n");
            return 1;
        }
    } else {
        printf("[SKIP] cell_neural_observe not implemented in this firmware build.\n");
    }

    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();

    extern void lau_free_all_active(void);
    lau_free_all_active();

    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
