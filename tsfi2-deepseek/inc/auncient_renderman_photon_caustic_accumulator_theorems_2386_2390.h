#ifndef AUNCIENT_RENDERMAN_PHOTON_CAUSTIC_ACCUMULATOR_THEOREMS_2386_2390_H
#define AUNCIENT_RENDERMAN_PHOTON_CAUSTIC_ACCUMULATOR_THEOREMS_2386_2390_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTON_CAUSTIC_BINS 64
#define PHOTON_MAP_CANARY_GUARD 0x50484F544F4E3336ULL /* "PHOTON36" */

typedef struct {
    uint32_t photon_id;
    float position_x;
    float position_y;
    float position_z;
    float power_lumens;
    float incident_theta_rad;
    bool is_photon_stored;
} RendermanCausticPhoton;

typedef struct {
    uint64_t head_guard;
    RendermanCausticPhoton photons[PHOTON_CAUSTIC_BINS];
    uint64_t photon_kdtree_latch[64];
    uint64_t tail_guard;
    uint32_t total_photons_traced;
    uint32_t caustic_irradiance_estimates;
    uint32_t cdc6600_60bit_photon_words;
    uint32_t overflow_trapped_photons;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_photon_energy_conserved;
    bool is_photon_map_memory_safe;
} RendermanPhotonMapContext;

typedef struct {
    float in_silicon_photon_fidelity;
    float photon_strategy_datbin_merkle_ratio;
    float photon_trace_latency_ns;
    uint64_t verified_photon_saat_clearances;

    bool photon_map_pipeline_verified;
    bool photon_strategy_merkle_verified;
    bool photon_submicro_latency_verified;
    bool photon_lossless_saat_verified;
    bool sovereign_2390_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanPhotonBeyond2385State;

int cpm_tomie_renderman_photon_init(RendermanPhotonMapContext *ctx);
int cpm_tomie_renderman_photon_deposit(RendermanPhotonMapContext *ctx, uint32_t photon_idx, float x, float y, float z, float power);
bool cpm_tomie_renderman_photon_assert_safety(RendermanPhotonMapContext *ctx);

void auncient_renderman_photon_init(RendermanPhotonBeyond2385State *state);
bool auncient_renderman_photon_verify_theorems_2386_2390(RendermanPhotonBeyond2385State *state);
uint32_t auncient_renderman_photon_compute_rule18(const RendermanPhotonBeyond2385State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_PHOTON_CAUSTIC_ACCUMULATOR_THEOREMS_2386_2390_H */
