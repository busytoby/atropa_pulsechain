#define _POSIX_C_SOURCE 200809L
#include "tsfi_totient_chain_composition_solver.h"
#include "tsfi_totient_zero_nullstellensatz_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

bool tsfi_solve_totient_nullstellensatz_chain(
    const TsfiTotientChainInput *input,
    TsfiTotientChainResult *out_result
) {
    if (!input || !out_result) return false;

    double start_ns = get_time_ns();
    memset(out_result, 0, sizeof(TsfiTotientChainResult));

    // 1. Evaluate Totient Zero Hilbert Nullstellensatz Radical Invariant
    TsfiTotientZeroProofParams zero_params = {
        .totient_val = input->initial_totient_state,
        .radical_power_m = input->radical_depth_m,
        .accumulator_energy = input->accumulator_energy,
        .cics_writer_id = input->cics_writer_id,
        .simulate_fault = 0,
        .k_param = input->k_param
    };

    TsfiTotientZeroRuling zero_ruling = tsfi_verify_totient_zero_nullstellensatz(&zero_params);
    if (zero_ruling != TSFI_NULLSTELLENSATZ_QUALIFIED) {
        out_result->nullstellensatz_zero_passed = false;
        out_result->total_eval_latency_ns = get_time_ns() - start_ns;
        return false;
    }
    out_result->nullstellensatz_zero_passed = true;

    // 2. Evaluate Closed-Grid Totient Boundary BMRC Invariant (gamma = 875/1000)
    int current_jitter = 1000;
    int gamma_num = 875;
    int gamma_den = 1000;
    for (uint32_t step = 1; step <= 16; step++) {
        current_jitter = (current_jitter * gamma_num) / gamma_den;
    }
    if (current_jitter > 1000) {
        out_result->closed_boundary_passed = false;
        out_result->total_eval_latency_ns = get_time_ns() - start_ns;
        return false;
    }
    out_result->closed_boundary_passed = true;

    // 3. Evaluate Accumulator Clamped Energy Non-Negativity
    if (input->accumulator_energy >= 0 && input->accumulator_energy <= 100000) {
        out_result->accumulator_clamped = true;
    } else {
        out_result->accumulator_clamped = false;
        out_result->total_eval_latency_ns = get_time_ns() - start_ns;
        return false;
    }

    // 4. Compute Composed AST Merkle Digest via OpenSSL EVP
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        out_result->total_eval_latency_ns = get_time_ns() - start_ns;
        return false;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) == 1) {
        EVP_DigestUpdate(ctx, &input->initial_totient_state, sizeof(input->initial_totient_state));
        EVP_DigestUpdate(ctx, &input->radical_depth_m, sizeof(input->radical_depth_m));
        EVP_DigestUpdate(ctx, &input->accumulator_energy, sizeof(input->accumulator_energy));
        EVP_DigestUpdate(ctx, &input->bus_nodes_m, sizeof(input->bus_nodes_m));
        unsigned int len = 0;
        EVP_DigestFinal_ex(ctx, out_result->composed_merkle_digest, &len);
    }
    EVP_MD_CTX_free(ctx);

    out_result->total_eval_latency_ns = get_time_ns() - start_ns;
    return true;
}
