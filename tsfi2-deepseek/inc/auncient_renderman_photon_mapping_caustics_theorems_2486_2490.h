#ifndef AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2486_2490_H
#define AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2486_2490_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTON_MAP_CAPACITY 256
#define PHOTON_SEARCH_K_NEAREST 16
#define PHOTON_MAP_CANARY_GUARD 0x50484F544F4E3636ULL /* "PHOTON66" */

typedef struct {
    uint32_t photon_id;
    float position_x;
    float position_y;
    float position_z;
    float power_flux_watts;
    float incident_dir_theta;
    bool is_photon_stored;
} CausticPhotonNode;

typedef struct {
    uint64_t head_guard;
    CausticPhotonNode photons[PHOTON_MAP_CAPACITY];
    uint64_t photon_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_photons_emitted;
    uint32_t kdtree_nearest_searches_completed;
    uint32_t cdc6600_60bit_photon_words;
    uint32_t overflow_trapped_photons;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_caustic_flux_conserved;
    bool is_photon_map_memory_safe;
} RendermanPhotonMapContext;

typedef struct {
    float in_silicon_photon_fidelity;
    float photon_strategy_datbin_merkle_ratio;
    float photon_estimate_latency_ns;
    uint64_t verified_photon_saat_clearances;

    bool photon_mapping_pipeline_verified;
    bool photon_strategy_merkle_verified;
    bool photon_submicro_latency_verified;
    bool photon_lossless_saat_verified;
    bool sovereign_2490_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanPhotonBeyond2485State;

int cpm_tomie_photon_map_init(RendermanPhotonMapContext *ctx);
int cpm_tomie_photon_map_store(RendermanPhotonMapContext *ctx, uint32_t idx, float x, float y, float z, float power);
int cpm_tomie_photon_map_estimate_radiance(RendermanPhotonMapContext *ctx, float query_x, float query_y, float query_z, float *out_irradiance);
bool cpm_tomie_photon_map_assert_safety(RendermanPhotonMapContext *ctx);

void auncient_photon_map_init(RendermanPhotonBeyond2485State *state);
bool auncient_photon_map_verify_theorems_2486_2490(RendermanPhotonBeyond2485State *state);
uint32_t auncient_photon_map_compute_rule18(const RendermanPhotonBeyond2485State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2486_2490_H */
