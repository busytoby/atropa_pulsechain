#ifndef TSFI_MONTECARLO_H
#define TSFI_MONTECARLO_H

#include <stdint.h>
#include <stdbool.h>

// Vector representation for auxiliary features
typedef struct {
    float x, y, z;
} TSFiMCVec3;

// Auxiliary feature channels (G-Buffer)
typedef struct {
    TSFiMCVec3 normal;
    float depth;
    TSFiMCVec3 albedo;
    float emotional_weight;
} TSFiMCAuxFeatures;

// Estimates local variance in a pixel patch to determine if adaptive sampling is needed.
// Returns true if variance is above threshold, indicating more samples are required.
bool tsfi_montecarlo_estimate_variance(const float *samples, int count, double threshold, double *variance_out);

// Applies a cross-bilateral filter to denoise a noisy pixel buffer using auxiliary features.
bool tsfi_montecarlo_cross_bilateral_filter(
    const uint32_t *noisy_color,
    const TSFiMCAuxFeatures *features,
    uint32_t *denoised_color,
    int width,
    int height,
    float spatial_sigma,
    float range_sigma
);

// Performs a patch-based Non-Local Means (NLM) reconstruction guided by an auxiliary features map.
bool tsfi_montecarlo_non_local_means(
    const float *noisy_input,
    const float *guidance_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius
);

// Performs a patch-based Non-Local Means (NLM) reconstruction favoring emotional features (warmth/empathy geometry maps) over color gradients.
bool tsfi_montecarlo_emotional_non_local_means(
    const float *noisy_input,
    const float *emotional_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius,
    float empathy_bias
);

// Performs a patch-based Non-Local Means (NLM) using guided path selection along emotional map gradients.
bool tsfi_montecarlo_guided_path_non_local_means(
    const float *noisy_input,
    const float *emotional_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_steps,
    float empathy_bias
);

// Performs a local linear regression denoising pass using G-Buffer features as regression covariates.
bool tsfi_montecarlo_regression_denoise(
    const float *noisy_input,
    const TSFiMCAuxFeatures *features,
    float *clean_output,
    int width,
    int height,
    int window_radius,
    float spatial_sigma,
    float feature_sigma
);

// Renders a Maniac Mansion frame built from the Z-machine using Vaesen guidance map based Monte Carlo path tracing.
bool tsfi_montecarlo_render_maniac_mansion(
    uint32_t room_id,
    uint32_t *pixels_out,
    float *depth_out,
    int width,
    int height,
    float time
);

// Estimates a posteriori relative error of the mean for image-space adaptive sampling.
bool tsfi_montecarlo_aposteriori_error_estimate(
    const float *samples,
    int count,
    double target_error,
    double *error_out
);

// Estimates a priori initial sample count based on local G-buffer gradients.
int tsfi_montecarlo_apriori_sample_count(
    const TSFiMCAuxFeatures *features,
    int x,
    int y,
    int width,
    int height,
    int base_samples
);

// Estimates adaptive filter sigmas based on local error metrics.
bool tsfi_montecarlo_adaptive_sigma(
    double local_error,
    float base_spatial_sigma,
    float base_range_sigma,
    float *adaptive_spatial,
    float *adaptive_range
);

// Transaction state container for filter parameters
typedef struct {
    float spatial_sigma;
    float range_sigma;
} TSFiMCFilterState;

// ACID Transaction container for filter updates
typedef struct {
    TSFiMCFilterState *target;
    TSFiMCFilterState backup;
    bool active;
} TSFiMCFilterTx;

// ACID-compliant transaction lifecycle functions
TSFiMCFilterTx tsfi_montecarlo_begin_filter_transaction(TSFiMCFilterState *state);
bool tsfi_montecarlo_commit_filter_transaction(TSFiMCFilterTx *tx, float next_spatial, float next_range);
void tsfi_montecarlo_rollback_filter_transaction(TSFiMCFilterTx *tx);

// Collaborative Block-Matching Non-Local Means (CNLM) filter
bool tsfi_montecarlo_collaborative_block_matching_filter(
    const float *noisy_input,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius
);

#endif // TSFI_MONTECARLO_H
