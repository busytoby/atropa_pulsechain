#ifndef AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2111_2115_H
#define AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2111_2115_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHOTON_MAP_MAX_PHOTONS 1024
#define PHOTON_KD_TREE_MAX_NODES 2048

typedef struct {
    double position[3];
    double power[3];
    double incident_dir[3];
    uint8_t kd_plane;
} PhotonDescriptor;

typedef struct {
    PhotonDescriptor photons[PHOTON_MAP_MAX_PHOTONS];
    uint32_t total_emitted_photons;
    uint32_t stored_caustic_photons;
    uint32_t cdc6600_kd_queries;
    bool is_photon_kd_tree_balanced;
    bool is_caustic_density_estimated;
} RendermanPhotonMappingContext;

typedef struct {
    float in_silicon_photon_map_fidelity;
    float photon_map_strategy_datbin_merkle_ratio;
    float photon_map_lookup_latency_ns;
    uint64_t verified_photon_map_saat_clearances;

    bool photon_map_pipeline_verified;
    bool photon_map_strategy_merkle_verified;
    bool photon_map_submicro_latency_verified;
    bool photon_map_lossless_saat_verified;
    bool sovereign_2115_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanPhotonMappingBeyond2110State;

int cpm_tomie_renderman_photon_init(RendermanPhotonMappingContext *ctx);
int cpm_tomie_renderman_photon_emit_and_store(RendermanPhotonMappingContext *ctx, uint32_t count);
int cpm_tomie_renderman_photon_estimate_density(RendermanPhotonMappingContext *ctx, const double query_pt[3], double *out_caustic_flux);
void auncient_renderman_photon_mapping_init(RendermanPhotonMappingBeyond2110State *state);
bool auncient_renderman_photon_mapping_verify_theorems_2111_2115(RendermanPhotonMappingBeyond2110State *state);
uint32_t auncient_renderman_photon_mapping_compute_rule18(const RendermanPhotonMappingBeyond2110State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_PHOTON_MAPPING_CAUSTICS_THEOREMS_2111_2115_H */
