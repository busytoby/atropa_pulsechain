#include "tsfi_rtl_latch.h"
#include <string.h>

void tsfi_rtl_latch_init(TsfiRtlLatch *latch) {
    memset(latch, 0, sizeof(TsfiRtlLatch));
    latch->q = 0.2f;      // Default LOW
    latch->q_bar = 3.0f;  // Default HIGH
}

void tsfi_rtl_latch_update(TsfiRtlLatch *latch, float v_set, float v_reset) {
    // Logic thresholds
    const float v_thresh = 1.5f;
    const float v_high = 3.0f;
    const float v_low = 0.2f;

    int set_active = (v_set >= v_thresh);
    int reset_active = (v_reset >= v_thresh);

    if (set_active && reset_active) {
        // Both transistors conduct, pulling both outputs LOW (invalid state)
        latch->q = v_low;
        latch->q_bar = v_low;
    } else if (set_active) {
        // Set Q HIGH, Q_bar LOW
        latch->q = v_high;
        latch->q_bar = v_low;
    } else if (reset_active) {
        // Reset Q LOW, Q_bar HIGH
        latch->q = v_low;
        latch->q_bar = v_high;
    }
    // If set and reset are both low, cross-coupled feedback maintains previous state
}
