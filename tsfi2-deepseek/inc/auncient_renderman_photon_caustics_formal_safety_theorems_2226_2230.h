#ifndef AUNCIENT_RENDERMAN_PHOTON_CAUSTICS_FORMAL_SAFETY_THEOREMS_2226_2230_H
#define AUNCIENT_RENDERMAN_PHOTON_CAUSTICS_FORMAL_SAFETY_THEOREMS_2226_2230_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTON_MAP_MAX_CAPACITY 1024
#define PHOTON_MAP_CANARY_GUARD 0x50484F544F4E3636ULL /* "PHOTON66" */

typedef struct {
    float pos[3];
    float power[3];
    float dir[3];
    uint8_t kd_split_axis;
    bool is_stored;
} RendermanPhotonRecord;

typedef struct {
    uint64_t head_guard;
    RendermanPhotonRecord photons[PHOTON_MAP_MAX_CAPACITY];
    uint64_t tail_guard;
    uint32_t photons_stored_count;
    uint32_t overflow_trapped_photons;
    uint32_t cdc6600_60bit_caustic_words;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_kd_tree_balanced;
    bool is_photon_memory_safe;
} RendermanPhotonMapSafetyContext;

typedef struct {
    float in_silicon_photon_fidelity;
    float photon_strategy_datbin_merkle_ratio;
    float photon_lookup_latency_ns;
    uint64_t verified_photon_saat_clearances;

    bool photon_safety_pipeline_verified;
    bool photon_strategy_merkle_verified;
    bool photon_submicro_latency_verified;
    bool photon_lossless_saat_verified;
    bool sovereign_2230_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanPhotonSafetyBeyond2225State;

int cpm_tomie_renderman_photon_safety_init(RendermanPhotonMapSafetyContext *ctx);
int cpm_tomie_renderman_photon_store(RendermanPhotonMapSafetyContext *ctx, float x, float y, float z, float power);
bool cpm_tomie_renderman_photon_assert_safety(RendermanPhotonMapSafetyContext *ctx);

void auncient_renderman_photon_safety_init(RendermanPhotonSafetyBeyond2225State *state);
bool auncient_renderman_photon_safety_verify_theorems_2226_2230(RendermanPhotonSafetyBeyond2225State *state);
uint32_t auncient_renderman_photon_safety_compute_rule18(const RendermanPhotonSafetyBeyond2225State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_PHOTON_CAUSTICS_FORMAL_SAFETY_THEOREMS_2226_2230_H */
