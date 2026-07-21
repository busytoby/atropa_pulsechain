#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tsfi_schmidt_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_schmidt_engine_init(
    uint32_t schmidt_id,
    tsfi_schmidt_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_schmidt_engine_t));

    engine->schmidt_id = schmidt_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "SCHMIDT_SYNTAX_%08X.DAT.BIN", schmidt_id);
    return 0;
}

int tsfi_schmidt_syntax_check(
    tsfi_schmidt_engine_t *engine,
    const char *syntax_expr,
    int *is_valid_out
) {
    if (!engine || !syntax_expr || !is_valid_out) return -1;

    // Check basic Lee Schmidt CACM formal syntax rules: non-empty, balanced brackets/parentheses
    int paren_depth = 0;
    int bracket_depth = 0;
    int valid = 1;

    size_t len = strlen(syntax_expr);
    if (len == 0) valid = 0;

    for (size_t i = 0; i < len; i++) {
        if (syntax_expr[i] == '(') paren_depth++;
        else if (syntax_expr[i] == ')') paren_depth--;
        else if (syntax_expr[i] == '[') bracket_depth++;
        else if (syntax_expr[i] == ']') bracket_depth--;

        if (paren_depth < 0 || bracket_depth < 0) {
            valid = 0;
            break;
        }
    }

    if (paren_depth != 0 || bracket_depth != 0) valid = 0;

    *is_valid_out = valid;
    engine->metacompiler_evals++;
    return 0;
}

int tsfi_schmidt_lu_factorize(
    tsfi_schmidt_engine_t *engine,
    const double *matrix_A,
    size_t n,
    double *matrix_L_out,
    double *matrix_U_out
) {
    if (!engine || !matrix_A || n == 0 || !matrix_L_out || !matrix_U_out) return -1;

    memset(matrix_L_out, 0, n * n * sizeof(double));
    memset(matrix_U_out, 0, n * n * sizeof(double));

    // Doolittle LU factorization algorithm: A = L * U
    for (size_t i = 0; i < n; i++) {
        // L diagonal is 1.0
        matrix_L_out[i * n + i] = 1.0;

        for (size_t k = i; k < n; k++) {
            double sum = 0.0;
            for (size_t j = 0; j < i; j++) {
                sum += (matrix_L_out[i * n + j] * matrix_U_out[j * n + k]);
            }
            matrix_U_out[i * n + k] = matrix_A[i * n + k] - sum;
        }

        for (size_t k = i + 1; k < n; k++) {
            double sum = 0.0;
            for (size_t j = 0; j < i; j++) {
                sum += (matrix_L_out[k * n + j] * matrix_U_out[j * n + i]);
            }

            if (matrix_U_out[i * n + i] == 0.0) return -2; // Zero pivot encountered
            matrix_L_out[k * n + i] = (matrix_A[k * n + i] - sum) / matrix_U_out[i * n + i];
        }
    }

    engine->metacompiler_evals++;
    printf("[LEE SCHMIDT LU FACTORIZATION] ID: %u | Matrix Sz: %zux%zu | Evals: %u | Gas: %u | File: %s\n",
           engine->schmidt_id, n, n, engine->metacompiler_evals, engine->evm_gas_units, engine->tape_dat_bin);

    return 0;
}
