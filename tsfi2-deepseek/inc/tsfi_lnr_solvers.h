#ifndef TSFI_LNR_SOLVERS_H
#define TSFI_LNR_SOLVERS_H

#include <stdbool.h>
#include "tsfi_montecarlo.h"

// Solver thunk function pointer type
typedef bool (*lnr_solver_thunk_t)(const double *A, const double *B, double *x, int n);

// The five solver thunks
bool lnr_solve_lu(const double *A, const double *B, double *x, int n);
bool lnr_solve_qr(const double *A, const double *B, double *x, int n);
bool lnr_solve_svd(const double *A, const double *B, double *x, int n);
bool lnr_solve_cg(const double *A, const double *B, double *x, int n);
bool lnr_solve_gmres(const double *A, const double *B, double *x, int n);

// Main LNR dynamic selection denoiser pass
bool tsfi_montecarlo_regression_denoise_lnr(
    const float *noisy_input,
    const TSFiMCAuxFeatures *features,
    const float *guidance_map,
    float *clean_output,
    int width,
    int height,
    int window_radius,
    float spatial_sigma,
    float feature_sigma
);

#endif // TSFI_LNR_SOLVERS_H
