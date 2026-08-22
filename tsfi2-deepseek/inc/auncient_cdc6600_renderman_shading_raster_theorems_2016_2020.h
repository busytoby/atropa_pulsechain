#ifndef AUNCIENT_CDC6600_RENDERMAN_SHADING_RASTER_THEOREMS_2016_2020_H
#define AUNCIENT_CDC6600_RENDERMAN_SHADING_RASTER_THEOREMS_2016_2020_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_FRAME_WIDTH 512
#define RENDERMAN_FRAME_HEIGHT 512
#define RENDERMAN_TOTAL_PIXELS (RENDERMAN_FRAME_WIDTH * RENDERMAN_FRAME_HEIGHT)

typedef struct {
    uint32_t pixels_shaded;
    uint32_t specular_highlights_calculated;
    uint32_t ppu_scanline_dma_transfers;
    uint32_t active_shading_lanes;
    bool is_scanline_raster_complete;
    bool is_pageturner_presenter_synced;
} Cdc6600RenderManShadingContext;

typedef struct {
    float in_silicon_shading_fidelity;
    float shading_strategy_datbin_merkle_ratio;
    float shading_scanline_latency_ns;
    uint64_t verified_shading_saat_clearances;

    bool cdc6600_shading_raster_verified;
    bool shading_strategy_merkle_verified;
    bool shading_submicro_latency_verified;
    bool shading_lossless_saat_verified;
    bool grand_2020_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600RenderManShadingBeyond2015State;

int cpm_tomie_cdc6600_shading_init(Cdc6600RenderManShadingContext *ctx);
int cpm_tomie_cdc6600_shading_rasterize_scanlines(Cdc6600RenderManShadingContext *ctx, uint32_t scanline_start, uint32_t scanline_count);
void auncient_cdc6600_shading_init(Cdc6600RenderManShadingBeyond2015State *state);
bool auncient_cdc6600_shading_verify_theorems_2016_2020(Cdc6600RenderManShadingBeyond2015State *state);
uint32_t auncient_cdc6600_shading_compute_rule18(const Cdc6600RenderManShadingBeyond2015State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_RENDERMAN_SHADING_RASTER_THEOREMS_2016_2020_H */
