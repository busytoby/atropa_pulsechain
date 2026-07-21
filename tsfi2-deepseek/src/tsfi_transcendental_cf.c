#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_E
#define M_E 2.71828182845904523536
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tsfi_transcendental_cf.h"
#include "tsfi_lowpower_fet.h"

int tsfi_cf_compute_e_convergent(
    int depth_n,
    tsfi_cf_convergent_t *out_convergent
) {
    if (!out_convergent || depth_n < 0) return -1;
    memset(out_convergent, 0, sizeof(tsfi_cf_convergent_t));

    uint64_t p0 = 2, p1 = 3;
    uint64_t q0 = 1, q1 = 1;

    if (depth_n == 0) {
        out_convergent->numerator_p = p0;
        out_convergent->denominator_q = q0;
    } else if (depth_n == 1) {
        out_convergent->numerator_p = p1;
        out_convergent->denominator_q = q1;
    } else {
        uint64_t p_prev = p1, p_prev2 = p0;
        uint64_t q_prev = q1, q_prev2 = q0;
        uint64_t p_curr = 0, q_curr = 0;

        for (int i = 2; i <= depth_n; i++) {
            uint64_t a_i;
            if ((i % 3) == 2) {
                a_i = 2 * ((i + 1) / 3);
            } else {
                a_i = 1;
            }
            p_curr = a_i * p_prev + p_prev2;
            q_curr = a_i * q_prev + q_prev2;

            p_prev2 = p_prev; p_prev = p_curr;
            q_prev2 = q_prev; q_prev = q_curr;
        }
        out_convergent->numerator_p = p_curr;
        out_convergent->denominator_q = q_curr;
    }

    out_convergent->approximation = (double)out_convergent->numerator_p / (double)out_convergent->denominator_q;
    out_convergent->error_bound = fabs(M_E - out_convergent->approximation);
    out_convergent->evm_gas_cost = 500;

    snprintf(out_convergent->tape_dat_bin, sizeof(out_convergent->tape_dat_bin),
             "EULER_CF_%08X.DAT.BIN", (unsigned int)out_convergent->denominator_q);

    printf("[EULER CONTINUED FRACTION] Depth: %d | Convergent: %llu / %llu = %.10f | Error: %.12e | File: %s\n",
           depth_n, (unsigned long long)out_convergent->numerator_p,
           (unsigned long long)out_convergent->denominator_q,
           out_convergent->approximation, out_convergent->error_bound, out_convergent->tape_dat_bin);

    return 0;
}

int tsfi_cf_compute_pi_convergent(
    int depth_n,
    tsfi_cf_convergent_t *out_convergent
) {
    if (!out_convergent || depth_n < 0) return -1;
    memset(out_convergent, 0, sizeof(tsfi_cf_convergent_t));

    static const uint64_t pi_terms[] = {3, 7, 15, 1, 292, 1, 1, 1, 2, 13, 4};
    int max_terms = sizeof(pi_terms) / sizeof(pi_terms[0]);
    if (depth_n >= max_terms) depth_n = max_terms - 1;

    uint64_t p_prev2 = 1, p_prev = pi_terms[0];
    uint64_t q_prev2 = 0, q_prev = 1;
    uint64_t p_curr = p_prev, q_curr = q_prev;

    for (int i = 1; i <= depth_n; i++) {
        uint64_t a_i = pi_terms[i];
        p_curr = a_i * p_prev + p_prev2;
        q_curr = a_i * q_prev + q_prev2;

        p_prev2 = p_prev; p_prev = p_curr;
        q_prev2 = q_prev; q_prev = q_curr;
    }

    out_convergent->numerator_p = p_curr;
    out_convergent->denominator_q = q_curr;
    out_convergent->approximation = (double)out_convergent->numerator_p / (double)out_convergent->denominator_q;
    out_convergent->error_bound = fabs(M_PI - out_convergent->approximation);
    out_convergent->evm_gas_cost = 500;

    snprintf(out_convergent->tape_dat_bin, sizeof(out_convergent->tape_dat_bin),
             "PI_CF_%08X.DAT.BIN", (unsigned int)out_convergent->denominator_q);

    printf("[PI CONTINUED FRACTION] Depth: %d | Convergent: %llu / %llu = %.10f | Error: %.12e | File: %s\n",
           depth_n, (unsigned long long)out_convergent->numerator_p,
           (unsigned long long)out_convergent->denominator_q,
           out_convergent->approximation, out_convergent->error_bound, out_convergent->tape_dat_bin);

    return 0;
}
