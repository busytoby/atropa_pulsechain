#ifndef AUNCIENT_RENDERMAN_REYES_PROGRESSIVE_RASTERIZER_THEOREMS_2246_2250_H
#define AUNCIENT_RENDERMAN_REYES_PROGRESSIVE_RASTERIZER_THEOREMS_2246_2250_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REYES_FRAMEBUFFER_TILES_X 16
#define REYES_FRAMEBUFFER_TILES_Y 16
#define REYES_SAMPLES_PER_PIXEL 16
#define REYES_RASTERIZER_CANARY_GUARD 0x52455945533636ULL /* "REYES66" */

typedef struct {
    float r, g, b, a;
    float z_depth;
    uint32_t sample_count;
    bool is_tile_converged;
} ReyesTilePixel;

typedef struct {
    uint64_t head_guard;
    ReyesTilePixel tiles[REYES_FRAMEBUFFER_TILES_Y][REYES_FRAMEBUFFER_TILES_X];
    uint64_t tail_guard;
    uint32_t progressive_passes_completed;
    uint32_t total_samples_rasterized;
    uint32_t cdc6600_60bit_framebuffer_words;
    uint32_t overflow_trapped_samples;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_progressive_energy_conserved;
    bool is_reyes_rasterizer_memory_safe;
} ReyesProgressiveRasterizerContext;

typedef struct {
    float in_silicon_rasterizer_fidelity;
    float rasterizer_strategy_datbin_merkle_ratio;
    float rasterizer_tile_latency_ns;
    uint64_t verified_rasterizer_saat_clearances;

    bool reyes_rasterizer_pipeline_verified;
    bool rasterizer_strategy_merkle_verified;
    bool rasterizer_submicro_latency_verified;
    bool rasterizer_lossless_saat_verified;
    bool sovereign_2250_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} ReyesRasterizerBeyond2245State;

int cpm_tomie_reyes_progressive_rasterizer_init(ReyesProgressiveRasterizerContext *ctx);
int cpm_tomie_reyes_rasterize_pass(ReyesProgressiveRasterizerContext *ctx, uint32_t pass_index);
bool cpm_tomie_reyes_assert_safety(ReyesProgressiveRasterizerContext *ctx);

void auncient_reyes_progressive_rasterizer_init(ReyesRasterizerBeyond2245State *state);
bool auncient_reyes_progressive_rasterizer_verify_theorems_2246_2250(ReyesRasterizerBeyond2245State *state);
uint32_t auncient_reyes_progressive_rasterizer_compute_rule18(const ReyesRasterizerBeyond2245State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_REYES_PROGRESSIVE_RASTERIZER_THEOREMS_2246_2250_H */
