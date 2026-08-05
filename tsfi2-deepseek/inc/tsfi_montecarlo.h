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

#endif // TSFI_MONTECARLO_H
