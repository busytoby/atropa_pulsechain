#ifndef AUNCIENT_RENDERMAN_SUB_SURFACE_SCATTERING_THEOREMS_2106_2110_H
#define AUNCIENT_RENDERMAN_SUB_SURFACE_SCATTERING_THEOREMS_2106_2110_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSSRDF_SAMPLE_COUNT 16

typedef struct {
    double surface_point[3];
    double normal_vec[3];
    double diffuse_mean_free_path;
    double absorption_coeff;
    double scattering_coeff;
} BssrdfPointSample;

typedef struct {
    BssrdfPointSample samples[BSSRDF_SAMPLE_COUNT];
    uint32_t total_bssrdf_evals;
    uint32_t cdc6600_dipole_evals;
    bool is_dipole_diffusion_locked;
    bool is_bssrdf_integrated;
} RendermanSubsurfaceScatteringContext;

typedef struct {
    float in_silicon_bssrdf_fidelity;
    float bssrdf_strategy_datbin_merkle_ratio;
    float bssrdf_eval_latency_ns;
    uint64_t verified_bssrdf_saat_clearances;

    bool bssrdf_pipeline_verified;
    bool bssrdf_strategy_merkle_verified;
    bool bssrdf_submicro_latency_verified;
    bool bssrdf_lossless_saat_verified;
    bool sovereign_2110_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanBssrdfBeyond2105State;

int cpm_tomie_renderman_bssrdf_init(RendermanSubsurfaceScatteringContext *ctx);
int cpm_tomie_renderman_bssrdf_integrate_surface(RendermanSubsurfaceScatteringContext *ctx, uint32_t patch_count);
void auncient_renderman_bssrdf_init(RendermanBssrdfBeyond2105State *state);
bool auncient_renderman_bssrdf_verify_theorems_2106_2110(RendermanBssrdfBeyond2105State *state);
uint32_t auncient_renderman_bssrdf_compute_rule18(const RendermanBssrdfBeyond2105State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_SUB_SURFACE_SCATTERING_THEOREMS_2106_2110_H */
