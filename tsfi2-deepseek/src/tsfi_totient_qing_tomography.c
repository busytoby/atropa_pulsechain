#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_totient_qing_tomography.h"

static float compute_euler_totient(uint32_t n) {
    uint32_t result = n;
    uint32_t p = 2;
    uint32_t temp = n;
    while (p * p <= temp) {
        if (temp % p == 0) {
            while (temp % p == 0) temp /= p;
            result -= result / p;
        }
        p++;
    }
    if (temp > 1) result -= result / temp;
    return (float)result * 0.125f;
}

tsfi_totient_qing_t* tsfi_totient_qing_create(
    const char *symbol,
    uint64_t starting_saat,
    uint32_t totient_n
) {
    tsfi_totient_qing_t *qing = (tsfi_totient_qing_t *)calloc(1, sizeof(tsfi_totient_qing_t));
    if (!qing) return NULL;

    strncpy(qing->qing_symbol, symbol ? symbol : "DefaultQing", sizeof(qing->qing_symbol) - 1);
    qing->starting_balance_saat = starting_saat > 0 ? starting_saat : 1000000ULL;
    qing->totient_n = totient_n > 0 ? totient_n : 10;
    qing->totient_qing_radius = compute_euler_totient(qing->totient_n);
    qing->acc = tsfi_chamberland_accumulator_create();

    return qing;
}

bool tsfi_totient_qing_eval_tomography(
    tsfi_totient_qing_t *qing,
    const float *x,
    int dim,
    uint32_t *active_stekloff_mode_out,
    float *duality_metric_out
) {
    if (!qing || !x || dim <= 0) return false;

    // 1. Stekloff Eigenvalue Evaluation relative to Totient Qing Boundary r_{qing}
    bool ok_stekloff = tsfi_stekloff_eval_eigenvalue_qing(x, dim, qing->totient_qing_radius, &qing->stekloff);
    if (ok_stekloff && active_stekloff_mode_out) {
        *active_stekloff_mode_out = qing->stekloff.active_stekloff_mode;
    }

    // 2. Chamberland Dual Tomography Accumulator Update (Epibar \bar{\epsilon} \iff Hypobar \underline{\epsilon})
    if (qing->acc) {
        float epibar_f = tsfi_chamberland_eval_epibar_23tree_functional(x, dim);
        float hypobar_c = tsfi_chamberland_eval_hypobar_rb_classification(x, dim > 32 ? 32 : dim, 1.0f);
        tsfi_chamberland_accumulator_update(qing->acc, x, dim, epibar_f, hypobar_c);

        if (duality_metric_out) {
            uint32_t pattern = 0;
            tsfi_chamberland_eval_duality_bijection(qing->acc, duality_metric_out, &pattern);
        }
    }

    return true;
}

void tsfi_totient_qing_destroy(tsfi_totient_qing_t *qing) {
    if (qing) {
        if (qing->acc) {
            tsfi_chamberland_accumulator_destroy(qing->acc);
        }
        free(qing);
    }
}
