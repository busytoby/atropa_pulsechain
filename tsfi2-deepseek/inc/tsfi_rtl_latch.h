#ifndef TSFI_RTL_LATCH_H
#define TSFI_RTL_LATCH_H

#include <stdint.h>

typedef struct {
    float q;      // Output Q voltage (0.2V to 3.0V)
    float q_bar;  // Output Q_bar voltage (0.2V to 3.0V)
} TsfiRtlLatch;

void tsfi_rtl_latch_init(TsfiRtlLatch *latch);
void tsfi_rtl_latch_update(TsfiRtlLatch *latch, float v_set, float v_reset);

#endif /* TSFI_RTL_LATCH_H */
