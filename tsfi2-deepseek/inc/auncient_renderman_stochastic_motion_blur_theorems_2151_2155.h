#ifndef AUNCIENT_RENDERMAN_STOCHASTIC_MOTION_BLUR_THEOREMS_2151_2155_H
#define AUNCIENT_RENDERMAN_STOCHASTIC_MOTION_BLUR_THEOREMS_2151_2155_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOTION_BLUR_TIME_SAMPLES 8
#define MOTION_BLUR_SHUTTER_OPEN 0.0
#define MOTION_BLUR_SHUTTER_CLOSE 1.0

typedef struct {
    double time_stamp;
    double camera_matrix[4][4];
    double teapot_angular_vel;
    double sample_weight;
} StochasticTimeSample;

typedef struct {
    StochasticTimeSample time_slices[MOTION_BLUR_TIME_SAMPLES];
    uint32_t total_stochastic_samples;
    uint32_t cdc6600_motion_interpolations;
    bool is_shutter_interval_valid;
    bool is_motion_blur_accumulated;
} RendermanMotionBlurContext;

typedef struct {
    float in_silicon_motion_blur_fidelity;
    float motion_blur_strategy_datbin_merkle_ratio;
    float motion_blur_sample_latency_ns;
    uint64_t verified_motion_blur_saat_clearances;

    bool motion_blur_pipeline_verified;
    bool motion_blur_strategy_merkle_verified;
    bool motion_blur_submicro_latency_verified;
    bool motion_blur_lossless_saat_verified;
    bool sovereign_2155_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanMotionBlurBeyond2150State;

int cpm_tomie_renderman_motion_blur_init(RendermanMotionBlurContext *ctx);
int cpm_tomie_renderman_motion_blur_sample_frame(RendermanMotionBlurContext *ctx, uint32_t micropolygon_count);
void auncient_renderman_motion_blur_init(RendermanMotionBlurBeyond2150State *state);
bool auncient_renderman_motion_blur_verify_theorems_2151_2155(RendermanMotionBlurBeyond2150State *state);
uint32_t auncient_renderman_motion_blur_compute_rule18(const RendermanMotionBlurBeyond2150State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_STOCHASTIC_MOTION_BLUR_THEOREMS_2151_2155_H */
