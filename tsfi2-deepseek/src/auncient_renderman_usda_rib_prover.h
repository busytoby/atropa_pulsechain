#ifndef AUNCIENT_RENDERMAN_USDA_RIB_PROVER_H
#define AUNCIENT_RENDERMAN_USDA_RIB_PROVER_H

#include "auncient_teddy_bear_heart_usda.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RENDERMAN_RIB_MAX_BUFFER 32768
#define RENDERMAN_TPA_BASE 0x0100

typedef struct {
    float displacement_bound_radius;
    float bsdf_albedo_integral;
    float bssrdf_subsurface_mean_free_path_mm;
    float photometric_lux_scale;
    bool crack_free_tessellation_verified;
    bool energy_conservation_verified;
    bool bssrdf_flux_conserved;
    bool winchester_scsi_handshake_verified;
    uint32_t rib_bytes_emitted;
    char rib_buffer[RENDERMAN_RIB_MAX_BUFFER];
} RenderManRibState;

void auncient_renderman_rib_init(RenderManRibState *rib);
bool auncient_renderman_emit_teddy_bear_heart_rib(RenderManRibState *rib, const TeddyBearHeartUsdaAsset *teddy_bear);
bool auncient_renderman_verify_theorems_26_30(RenderManRibState *rib, const TeddyBearHeartUsdaAsset *teddy_bear);
uint32_t auncient_renderman_compute_rule18_checksum(const RenderManRibState *rib);

#endif /* AUNCIENT_RENDERMAN_USDA_RIB_PROVER_H */
