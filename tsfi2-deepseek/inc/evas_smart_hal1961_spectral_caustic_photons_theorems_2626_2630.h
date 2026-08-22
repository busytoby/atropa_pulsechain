#ifndef EVAS_SMART_HAL1961_SPECTRAL_CAUSTIC_PHOTONS_THEOREMS_2626_2630_H
#define EVAS_SMART_HAL1961_SPECTRAL_CAUSTIC_PHOTONS_THEOREMS_2626_2630_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_PHOTON_BEAM_COUNT 64
#define HAL1961_PHOTON_CANARY_GUARD 0x50484F544F4E3636ULL /* "PHOTON66" */

typedef struct {
    uint32_t photon_id;
    float origin_cyclops_xyz[3];
    float incident_ray_direction[3];
    float specular_refraction_ior;     /* IOR = 1.52 (Optical crown glass) */
    float porcelain_scatter_flux;      /* Translucent teapot rim flux */
    float photon_color_rgb[3];         /* Monochromatic red caustic flare */
    bool is_photon_absorbed_lossless;
} EvasHalPhotonBeamNode;

typedef struct {
    uint64_t head_guard;
    EvasHalPhotonBeamNode photons[HAL1961_PHOTON_BEAM_COUNT];
    float kd_tree_radius_mm;           /* 3.5 mm photon gather radius */
    float fresnel_reflectance_f0;      /* Schlick reflectance F0 = 0.04 */
    uint64_t evas_hal_photon_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_photons_traced;
    uint32_t kd_tree_spatial_queries_executed;
    uint32_t cdc6600_60bit_photon_words;
    uint32_t overflow_trapped_photon_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_photon_energy_conserved;
    bool is_evas_hal_photon_memory_safe;
} EvasSmartHal1961PhotonCausticsContext;

typedef struct {
    float in_silicon_hal_photon_fidelity;
    float hal_photon_strategy_datbin_merkle_ratio;
    float hal_photon_gather_latency_ns;
    uint64_t verified_hal_photon_saat_clearances;

    bool evas_hal_photon_pipeline_verified;
    bool hal_photon_strategy_merkle_verified;
    bool hal_photon_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_caustic_flux_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalPhotonBeyond2625State;

int evas_smart_hal1961_photon_init(EvasSmartHal1961PhotonCausticsContext *ctx);
int evas_smart_hal1961_photon_trace_beam(EvasSmartHal1961PhotonCausticsContext *ctx, uint32_t beam_idx, float angle_rad, float *out_caustic_flux);
bool evas_smart_hal1961_photon_assert_safety(EvasSmartHal1961PhotonCausticsContext *ctx);

void evas_smart_hal_photon_beyond2625_init(EvasSmartHalPhotonBeyond2625State *state);
bool evas_smart_hal_photon_beyond2625_verify_theorems_2626_2630(EvasSmartHalPhotonBeyond2625State *state);
uint32_t evas_smart_hal_photon_beyond2625_compute_rule18(const EvasSmartHalPhotonBeyond2625State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_SPECTRAL_CAUSTIC_PHOTONS_THEOREMS_2626_2630_H */
