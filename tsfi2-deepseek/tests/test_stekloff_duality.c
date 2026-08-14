#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_stekloff_duality.h"

int main(void) {
    printf("================ TEST ALESSANDRINI-MAGNANINI STEKLOFF QING DOMAIN ================\n");

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.5f;

    tsfi_stekloff_state_t stekloff;
    bool ok_eval = tsfi_stekloff_eval_eigenvalue_qing(x_sample, 16, 1.25f, &stekloff);
    assert(ok_eval);
    assert(stekloff.qing_reference_radius == 1.25f);
    printf("  [PASS] Stekloff Eigenvalue relative to Qing evaluated (sigma = %.4f, var = %.4f).\n", stekloff.stekloff_sigma, stekloff.sigma_variance);

    uint32_t mode = 0;
    bool ok_mode = tsfi_stekloff_classify_mode_qing(&stekloff, &mode);
    assert(ok_mode);
    assert(mode == STEKLOFF_MODE_EPIBAR || mode == STEKLOFF_MODE_HYPOBAR);
    printf("  [PASS] Stekloff Qing Domain Mode classified (Mode = 0x%08X).\n", mode);

    printf("===================================================================================\n");
    return 0;
}
