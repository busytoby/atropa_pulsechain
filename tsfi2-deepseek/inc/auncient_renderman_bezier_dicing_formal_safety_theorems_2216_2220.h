#ifndef AUNCIENT_RENDERMAN_BEZIER_DICING_FORMAL_SAFETY_THEOREMS_2216_2220_H
#define AUNCIENT_RENDERMAN_BEZIER_DICING_FORMAL_SAFETY_THEOREMS_2216_2220_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_MAX_TEAPOT_PATCHES 32
#define RENDERMAN_MICROPOLYGONS_PER_PATCH 256
#define RENDERMAN_TOTAL_MICROPOLYGONS (RENDERMAN_MAX_TEAPOT_PATCHES * RENDERMAN_MICROPOLYGONS_PER_PATCH)
#define BEZIER_DICING_CANARY_GUARD 0x544541504F543636ULL /* "TEAPOT66" */

typedef struct {
    double x, y, z;
    double nx, ny, nz;
    double u, v;
} RendermanMicropolygonVertex;

typedef struct {
    uint64_t head_guard;
    RendermanMicropolygonVertex vertices[RENDERMAN_TOTAL_MICROPOLYGONS];
    uint64_t tail_guard;
    uint32_t patches_diced_count;
    uint32_t micropolygons_allocated;
    uint32_t cdc6600_60bit_words_emitted;
    uint32_t boundary_clippings_enforced;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_dicing_memory_safe;
} RendermanBezierDicingContext;

typedef struct {
    float in_silicon_dicing_fidelity;
    float dicing_strategy_datbin_merkle_ratio;
    float dicing_evaluation_latency_ns;
    uint64_t verified_dicing_saat_clearances;

    bool bezier_dicing_pipeline_verified;
    bool dicing_strategy_merkle_verified;
    bool dicing_submicro_latency_verified;
    bool dicing_lossless_saat_verified;
    bool sovereign_2220_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanBezierDicingBeyond2215State;

int cpm_tomie_renderman_bezier_dicing_init(RendermanBezierDicingContext *ctx);
int cpm_tomie_renderman_bezier_dice_patch(RendermanBezierDicingContext *ctx, uint32_t patch_index);
bool cpm_tomie_renderman_bezier_assert_safety(RendermanBezierDicingContext *ctx);

void auncient_renderman_bezier_dicing_init(RendermanBezierDicingBeyond2215State *state);
bool auncient_renderman_bezier_dicing_verify_theorems_2216_2220(RendermanBezierDicingBeyond2215State *state);
uint32_t auncient_renderman_bezier_dicing_compute_rule18(const RendermanBezierDicingBeyond2215State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_BEZIER_DICING_FORMAL_SAFETY_THEOREMS_2216_2220_H */
