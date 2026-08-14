#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_totient_qing_tomography.h"

int main(void) {
    printf("================ TEST DYNAMIC TOTIENT QING DUAL-BAR TOMOGRAPHY ================\n");

    tsfi_totient_qing_t *qing = tsfi_totient_qing_create("ZorseQing", 1000000ULL, 10);
    assert(qing != NULL);
    assert(qing->totient_qing_radius == 0.50f);
    printf("  [PASS] Dynamic Totient Qing initialized ('%s', Balance = %lu Saat, r_qing = %.4f).\n", qing->qing_symbol, qing->starting_balance_saat, qing->totient_qing_radius);

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.10f;

    uint32_t active_mode = 0;
    float duality_metric = 0.0f;
    bool ok_tomo = tsfi_totient_qing_eval_tomography(qing, x_sample, 16, &active_mode, &duality_metric);
    assert(ok_tomo);
    printf("  [PASS] Dual-Bar Tomography evaluated (Stekloff Mode = 0x%08X, Duality Metric = %.4f).\n", active_mode, duality_metric);

    tsfi_totient_qing_destroy(qing);
    printf("=================================================================================\n");
    return 0;
}
