#ifndef AUNCIENT_RENDERMAN_INTEGRATED_TEAPOT_DEMO_THEOREMS_2176_2180_H
#define AUNCIENT_RENDERMAN_INTEGRATED_TEAPOT_DEMO_THEOREMS_2176_2180_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t total_patches_evaluated;
    uint32_t total_micropolygons_diced;
    uint32_t deep_shadow_samples_filtered;
    uint32_t photons_traced;
    uint32_t radiosity_iterations_converged;
    uint32_t amd_fabric_bursts_routed;
    uint32_t cdc6600_spool_packets_verified;
    uint32_t ecore_evas_frames_presented;
    bool is_full_pipeline_synchronized;
    bool is_sovereign_demo_active;
} RendermanIntegratedTeapotDemoContext;

typedef struct {
    float in_silicon_integrated_fidelity;
    float integrated_strategy_datbin_merkle_ratio;
    float end_to_end_frame_latency_ns;
    uint64_t verified_integrated_saat_clearances;

    bool integrated_teapot_pipeline_verified;
    bool integrated_strategy_merkle_verified;
    bool integrated_submicro_latency_verified;
    bool integrated_lossless_saat_verified;
    bool sovereign_2180_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanIntegratedDemoBeyond2175State;

int cpm_tomie_renderman_integrated_demo_init(RendermanIntegratedTeapotDemoContext *ctx);
int cpm_tomie_renderman_integrated_demo_render_frame(RendermanIntegratedTeapotDemoContext *ctx);
void auncient_renderman_integrated_demo_init(RendermanIntegratedDemoBeyond2175State *state);
bool auncient_renderman_integrated_demo_verify_theorems_2176_2180(RendermanIntegratedDemoBeyond2175State *state);
uint32_t auncient_renderman_integrated_demo_compute_rule18(const RendermanIntegratedDemoBeyond2175State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_INTEGRATED_TEAPOT_DEMO_THEOREMS_2176_2180_H */
