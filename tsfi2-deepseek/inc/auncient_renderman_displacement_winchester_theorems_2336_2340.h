#ifndef AUNCIENT_RENDERMAN_DISPLACEMENT_WINCHESTER_THEOREMS_2336_2340_H
#define AUNCIENT_RENDERMAN_DISPLACEMENT_WINCHESTER_THEOREMS_2336_2340_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLACEMENT_VERTEX_BATCH 64
#define DISPLACEMENT_WINCHESTER_CANARY_GUARD 0x574D444953504C36ULL /* "WMDISPL6" */

typedef struct {
    uint32_t vertex_id;
    float base_x;
    float base_y;
    float base_z;
    float normal_x;
    float normal_y;
    float normal_z;
    float displaced_x;
    float displaced_y;
    float displaced_z;
    float displacement_scale;
    bool is_register_boundary_aligned;
} RendermanDisplacedVertex;

typedef struct {
    uint64_t head_guard;
    RendermanDisplacedVertex vertices[DISPLACEMENT_VERTEX_BATCH];
    uint64_t scsi_displacement_latch[64];
    uint64_t tail_guard;
    uint32_t total_displacements_evaluated;
    uint32_t boundary_constraints_satisfied;
    uint32_t cdc6600_60bit_displacement_words;
    uint32_t overflow_trapped_displacements;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_winchestermq_displacement_synced;
    bool is_displacement_shader_memory_safe;
} RendermanDisplacementShaderContext;

typedef struct {
    float in_silicon_displacement_fidelity;
    float displacement_strategy_datbin_merkle_ratio;
    float displacement_eval_latency_ns;
    uint64_t verified_displacement_saat_clearances;

    bool displacement_pipeline_verified;
    bool displacement_strategy_merkle_verified;
    bool displacement_submicro_latency_verified;
    bool displacement_lossless_saat_verified;
    bool sovereign_2340_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanDisplacementBeyond2335State;

int cpm_tomie_renderman_displacement_init(RendermanDisplacementShaderContext *ctx);
int cpm_tomie_renderman_displacement_evaluate(RendermanDisplacementShaderContext *ctx, uint32_t v_idx, float scale);
bool cpm_tomie_renderman_displacement_assert_safety(RendermanDisplacementShaderContext *ctx);

void auncient_renderman_displacement_init(RendermanDisplacementBeyond2335State *state);
bool auncient_renderman_displacement_verify_theorems_2336_2340(RendermanDisplacementBeyond2335State *state);
uint32_t auncient_renderman_displacement_compute_rule18(const RendermanDisplacementBeyond2335State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_DISPLACEMENT_WINCHESTER_THEOREMS_2336_2340_H */
