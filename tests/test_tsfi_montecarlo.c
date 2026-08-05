#include "tsfi_montecarlo.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

int main(void) {
    printf("[Test] Running standalone TSFi Monte Carlo Denoising tests...\n");

    // 1. Test Variance Estimation
    float samples[5] = {0.1f, 0.15f, 0.08f, 0.12f, 0.9f}; // high variance sample set
    double variance = 0.0;
    assert(tsfi_montecarlo_estimate_variance(samples, 5, 0.05, &variance));
    assert(variance > 0.05);

    // Test error boundaries for variance estimator
    assert(!tsfi_montecarlo_estimate_variance(NULL, 5, 0.05, &variance));
    assert(!tsfi_montecarlo_estimate_variance(samples, 1, 0.05, &variance));
    assert(!tsfi_montecarlo_estimate_variance(samples, 5, 0.05, NULL));
    printf("   ✓ Monte Carlo local variance estimation verified successfully\n");

    // 2. Test Cross-Bilateral Denoising Filter
    uint32_t noisy[4] = {0xFF102030, 0xFF122232, 0xFF8090A0, 0xFF8292A2};
    TSFiMCAuxFeatures features[4] = {
        {{0.0f, 1.0f, 0.0f}, 1.0f, {0.1f, 0.1f, 0.1f}, 0.0f},
        {{0.0f, 1.0f, 0.0f}, 1.05f, {0.11f, 0.11f, 0.11f}, 0.0f},
        {{1.0f, 0.0f, 0.0f}, 5.0f, {0.8f, 0.8f, 0.8f}, 0.0f}, // edge boundary
        {{1.0f, 0.0f, 0.0f}, 5.02f, {0.81f, 0.81f, 0.81f}, 0.0f}
    };
    uint32_t denoised[4] = {0};

    assert(tsfi_montecarlo_cross_bilateral_filter(noisy, features, denoised, 2, 2, 1.0f, 0.5f));
    assert(denoised[0] != 0);

    // Test error boundaries for cross-bilateral filter
    assert(!tsfi_montecarlo_cross_bilateral_filter(NULL, features, denoised, 2, 2, 1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, NULL, denoised, 2, 2, 1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, features, NULL, 2, 2, 1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, features, denoised, 0, 2, 1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, features, denoised, 2, -2, 1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, features, denoised, 2, 2, -1.0f, 0.5f));
    assert(!tsfi_montecarlo_cross_bilateral_filter(noisy, features, denoised, 2, 2, 1.0f, -0.5f));
    printf("   ✓ Monte Carlo cross-bilateral filter verified successfully\n");

    // 3. Test Non-Local Means Reconstruction
    float noisy_float[9] = {
        0.5f, 0.52f, 0.49f,
        0.51f, 0.9f, 0.5f, // spike noise in the center
        0.48f, 0.53f, 0.51f
    };
    float clean_float[9] = {0};

    assert(tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, 3, 3, 0.2f, 1, 1));
    assert(clean_float[4] < 0.8f); // the center spike is smoothed out by surrounding matches

    // Test error boundaries for NLM
    assert(!tsfi_montecarlo_non_local_means(NULL, noisy_float, clean_float, 3, 3, 0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, NULL, clean_float, 3, 3, 0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, NULL, 3, 3, 0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, -3, 3, 0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, 3, 0, 0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, 3, 3, -0.2f, 1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, 3, 3, 0.2f, -1, 1));
    assert(!tsfi_montecarlo_non_local_means(noisy_float, noisy_float, clean_float, 3, 3, 0.2f, 1, -1));
    printf("   ✓ Monte Carlo Non-Local Means reconstruction verified successfully\n");

    // 4. Test Emotional Non-Local Means Reconstruction
    float emotional_map[9] = {
        0.1f, 0.1f, 0.1f,
        0.1f, 0.9f, 0.1f, // high empathy/emotional feature in the center
        0.1f, 0.1f, 0.1f
    };
    float clean_emotional[9] = {0};
    assert(tsfi_montecarlo_emotional_non_local_means(noisy_float, emotional_map, clean_emotional, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(clean_emotional[4] > 0.0f);

    // Test error boundaries for emotional NLM
    assert(!tsfi_montecarlo_emotional_non_local_means(NULL, emotional_map, clean_emotional, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_emotional_non_local_means(noisy_float, NULL, clean_emotional, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_emotional_non_local_means(noisy_float, emotional_map, NULL, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_emotional_non_local_means(noisy_float, emotional_map, clean_emotional, -3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_emotional_non_local_means(noisy_float, emotional_map, clean_emotional, 3, 3, -0.2f, 1, 1, 0.8f));
    printf("   ✓ Monte Carlo Emotional Non-Local Means reconstruction verified successfully\n");

    // 5. Test Guided Path Non-Local Means Reconstruction
    float clean_guided[9] = {0};
    assert(tsfi_montecarlo_guided_path_non_local_means(noisy_float, emotional_map, clean_guided, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(clean_guided[4] > 0.0f);

    // Test error boundaries for guided path NLM
    assert(!tsfi_montecarlo_guided_path_non_local_means(NULL, emotional_map, clean_guided, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_guided_path_non_local_means(noisy_float, NULL, clean_guided, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_guided_path_non_local_means(noisy_float, emotional_map, NULL, 3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_guided_path_non_local_means(noisy_float, emotional_map, clean_guided, -3, 3, 0.2f, 1, 1, 0.8f));
    assert(!tsfi_montecarlo_guided_path_non_local_means(noisy_float, emotional_map, clean_guided, 3, 3, -0.2f, 1, 1, 0.8f));
    printf("   ✓ Monte Carlo Guided Path Non-Local Means reconstruction verified successfully\n");

    // 6. Test Local Linear Regression Denoising Pass
    float clean_regression[4] = {0};
    assert(tsfi_montecarlo_regression_denoise(noisy_float, features, clean_regression, 2, 2, 1, 1.5f, 0.5f));
    assert(clean_regression[0] > 0.0f);

    // Test error boundaries for regression denoiser
    assert(!tsfi_montecarlo_regression_denoise(NULL, features, clean_regression, 2, 2, 1, 1.5f, 0.5f));
    assert(!tsfi_montecarlo_regression_denoise(noisy_float, NULL, clean_regression, 2, 2, 1, 1.5f, 0.5f));
    assert(!tsfi_montecarlo_regression_denoise(noisy_float, features, NULL, 2, 2, 1, 1.5f, 0.5f));
    assert(!tsfi_montecarlo_regression_denoise(noisy_float, features, clean_regression, -2, 2, 1, 1.5f, 0.5f));
    assert(!tsfi_montecarlo_regression_denoise(noisy_float, features, clean_regression, 2, 2, -1, 1.5f, 0.5f));
    printf("   ✓ Monte Carlo local regression denoiser verified successfully\n");

    // 7. Test a posteriori relative standard error of the mean
    double rel_err = 0.0;
    float test_samples[2] = {1.0f, 1.2f};
    assert(tsfi_montecarlo_aposteriori_error_estimate(test_samples, 2, 0.05, &rel_err));
    assert(rel_err > 0.05);

    // Test error boundaries
    assert(!tsfi_montecarlo_aposteriori_error_estimate(NULL, 2, 0.05, &rel_err));
    assert(!tsfi_montecarlo_aposteriori_error_estimate(test_samples, 1, 0.05, &rel_err));
    assert(!tsfi_montecarlo_aposteriori_error_estimate(test_samples, 2, 0.05, NULL));
    printf("   ✓ Monte Carlo a posteriori relative error estimator verified successfully\n");

    printf("[Test] All Monte Carlo Denoising tests completed successfully.\n");
    return 0;
}
