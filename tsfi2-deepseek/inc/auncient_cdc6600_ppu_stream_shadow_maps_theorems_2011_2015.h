#ifndef AUNCIENT_CDC6600_PPU_STREAM_SHADOW_MAPS_THEOREMS_2011_2015_H
#define AUNCIENT_CDC6600_PPU_STREAM_SHADOW_MAPS_THEOREMS_2011_2015_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_SHADOW_MAP_TILES 16
#define CDC6600_DEPTH_BUFFER_SIZE 256

typedef struct {
    uint32_t depth_buffer[CDC6600_DEPTH_BUFFER_SIZE];
    uint32_t ppu_stream_bytes_transferred;
    uint32_t shadow_rays_evaluated;
    uint32_t active_shading_ppus;
    bool is_shadow_pass_complete;
    bool is_zbuffer_occlusion_valid;
} Cdc6600PpuShadowMapContext;

typedef struct {
    float in_silicon_ppu_shadow_fidelity;
    float ppu_shadow_strategy_datbin_merkle_ratio;
    float ppu_shadow_stream_latency_ns;
    uint64_t verified_ppu_shadow_saat_clearances;

    bool cdc6600_ppu_shadow_verified;
    bool ppu_shadow_strategy_merkle_verified;
    bool ppu_shadow_submicro_latency_verified;
    bool ppu_shadow_lossless_saat_verified;
    bool grand_2015_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600PpuShadowBeyond2010State;

int cpm_tomie_cdc6600_ppu_shadow_init(Cdc6600PpuShadowMapContext *ctx);
int cpm_tomie_cdc6600_ppu_shadow_cast(Cdc6600PpuShadowMapContext *ctx, uint32_t light_x, uint32_t light_y, uint32_t light_z);
void auncient_cdc6600_ppu_shadow_init(Cdc6600PpuShadowBeyond2010State *state);
bool auncient_cdc6600_ppu_shadow_verify_theorems_2011_2015(Cdc6600PpuShadowBeyond2010State *state);
uint32_t auncient_cdc6600_ppu_shadow_compute_rule18(const Cdc6600PpuShadowBeyond2010State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_PPU_STREAM_SHADOW_MAPS_THEOREMS_2011_2015_H */
