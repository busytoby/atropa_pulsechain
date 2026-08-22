#ifndef AUNCIENT_RENDERMAN_SUBDIV_DISPLACEMENT_THEOREMS_2086_2090_H
#define AUNCIENT_RENDERMAN_SUBDIV_DISPLACEMENT_THEOREMS_2086_2090_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_MAX_SUBDIV_LEVELS 4
#define RENDERMAN_TOTAL_BEZIER_PATCHES 32

typedef struct {
    uint32_t patch_id;
    uint32_t subdiv_level;
    uint32_t micropolygons_generated;
    double min_bound[3];
    double max_bound[3];
    bool is_displaced;
    bool is_culled;
} RendermanSubdivPatchDescriptor;

typedef struct {
    RendermanSubdivPatchDescriptor patches[RENDERMAN_TOTAL_BEZIER_PATCHES];
    uint32_t total_micropolygons;
    uint32_t culled_patches;
    uint32_t cdc6600_dicing_cycles;
    bool is_reyes_dicing_complete;
    bool is_displacement_bound_tight;
} RendermanSubdivDisplacementContext;

typedef struct {
    float in_silicon_subdiv_fidelity;
    float subdiv_strategy_datbin_merkle_ratio;
    float subdiv_dicing_latency_ns;
    uint64_t verified_subdiv_saat_clearances;

    bool subdiv_pipeline_verified;
    bool subdiv_strategy_merkle_verified;
    bool subdiv_submicro_latency_verified;
    bool subdiv_lossless_saat_verified;
    bool sovereign_2090_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanSubdivBeyond2085State;

int cpm_tomie_renderman_subdiv_init(RendermanSubdivDisplacementContext *ctx);
int cpm_tomie_renderman_subdiv_dice_all(RendermanSubdivDisplacementContext *ctx);
void auncient_renderman_subdiv_init(RendermanSubdivBeyond2085State *state);
bool auncient_renderman_subdiv_verify_theorems_2086_2090(RendermanSubdivBeyond2085State *state);
uint32_t auncient_renderman_subdiv_compute_rule18(const RendermanSubdivBeyond2085State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_SUBDIV_DISPLACEMENT_THEOREMS_2086_2090_H */
