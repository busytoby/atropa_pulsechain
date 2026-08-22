#ifndef EVAS_SMART_DIPOLE_SSS_DIFFUSION_THEOREMS_2506_2510_H
#define EVAS_SMART_DIPOLE_SSS_DIFFUSION_THEOREMS_2506_2510_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_SMART_DIPOLE_SAMPLES 64
#define EVAS_SMART_DIPOLE_CANARY_GUARD 0x4556415344495036ULL /* "EVASDIP6" */

typedef struct {
    uint32_t sample_id;
    float sample_point_xyz[3];
    float incident_radiance_rgb[3];
    float radial_distance_r;
    float subsurface_fluence_rate;
    bool is_sample_evaluated;
} EvasSmartDipoleSssSampleNode;

typedef struct {
    uint64_t head_guard;
    EvasSmartDipoleSssSampleNode samples[EVAS_SMART_DIPOLE_SAMPLES];
    uint64_t evas_dipole_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_dipole_samples_computed;
    uint32_t bssrdf_diffusions_integrated;
    uint32_t cdc6600_60bit_dipole_words;
    uint32_t overflow_trapped_dipole_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_dipole_diffusion_energy_conserved;
    bool is_evas_smart_dipole_memory_safe;
} EvasSmartDipoleSssContext;

typedef struct {
    float in_silicon_dipole_fidelity;
    float dipole_strategy_datbin_merkle_ratio;
    float dipole_eval_latency_ns;
    uint64_t verified_dipole_saat_clearances;

    bool evas_dipole_pipeline_verified;
    bool dipole_strategy_merkle_verified;
    bool dipole_submicro_latency_verified;
    bool dipole_lossless_saat_verified;
    bool dipole_energy_conservation_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartDipoleBeyond2505State;

int evas_smart_dipole_sss_init(EvasSmartDipoleSssContext *ctx);
int evas_smart_dipole_sss_evaluate_sample(EvasSmartDipoleSssContext *ctx, uint32_t sample_idx, const float pos[3], const float rad[3], float dist_r, float *out_fluence);
bool evas_smart_dipole_sss_assert_safety(EvasSmartDipoleSssContext *ctx);

void evas_smart_dipole_beyond2505_init(EvasSmartDipoleBeyond2505State *state);
bool evas_smart_dipole_beyond2505_verify_theorems_2506_2510(EvasSmartDipoleBeyond2505State *state);
uint32_t evas_smart_dipole_beyond2505_compute_rule18(const EvasSmartDipoleBeyond2505State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_DIPOLE_SSS_DIFFUSION_THEOREMS_2506_2510_H */
