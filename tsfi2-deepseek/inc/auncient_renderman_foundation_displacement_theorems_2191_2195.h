#ifndef AUNCIENT_RENDERMAN_FOUNDATION_DISPLACEMENT_THEOREMS_2191_2195_H
#define AUNCIENT_RENDERMAN_FOUNDATION_DISPLACEMENT_THEOREMS_2191_2195_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t base_phase_offset;
    uint64_t chin_lower_clamp;
    uint64_t foundation_z_modulus;
    uint32_t displacement_vertices_calculated;
    uint32_t boundary_constraints_passed;
    bool is_displacementshader_linked;
    bool is_foundation_phase_locked;
} RendermanFoundationContext;

typedef struct {
    float in_silicon_foundation_fidelity;
    float foundation_strategy_datbin_merkle_ratio;
    float foundation_eval_latency_ns;
    uint64_t verified_foundation_saat_clearances;

    bool renderman_foundation_pipeline_verified;
    bool foundation_strategy_merkle_verified;
    bool foundation_submicro_latency_verified;
    bool foundation_lossless_saat_verified;
    bool sovereign_2195_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanFoundationBeyond2190State;

int cpm_tomie_renderman_foundation_init(RendermanFoundationContext *ctx);
int cpm_tomie_renderman_foundation_displace_grid(RendermanFoundationContext *ctx, uint32_t vertex_count);
void auncient_renderman_foundation_init(RendermanFoundationBeyond2190State *state);
bool auncient_renderman_foundation_verify_theorems_2191_2195(RendermanFoundationBeyond2190State *state);
uint32_t auncient_renderman_foundation_compute_rule18(const RendermanFoundationBeyond2190State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_FOUNDATION_DISPLACEMENT_THEOREMS_2191_2195_H */
