#ifndef AUNCIENT_RENDERMAN_NAVI48_RASTERIZER_THEOREMS_2146_2150_H
#define AUNCIENT_RENDERMAN_NAVI48_RASTERIZER_THEOREMS_2146_2150_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_FRAMEBUFFER_WIDTH 1920
#define RENDERMAN_FRAMEBUFFER_HEIGHT 1080
#define RENDERMAN_TILES_X 30
#define RENDERMAN_TILES_Y 17

typedef struct {
    uint32_t tile_x;
    uint32_t tile_y;
    uint32_t micropolygons_binned;
    uint32_t pixels_shaded;
    bool is_tile_rendered;
} RendermanRasterTile;

typedef struct {
    RendermanRasterTile tiles[RENDERMAN_TILES_Y][RENDERMAN_TILES_X];
    uint32_t total_binned_micropolygons;
    uint32_t navi48_shader_invocations;
    uint32_t cdc6600_ppu_tile_dispatches;
    bool is_reyes_rasterization_complete;
    bool is_framebuffer_synced;
} RendermanNavi48RasterContext;

typedef struct {
    float in_silicon_raster_fidelity;
    float raster_strategy_datbin_merkle_ratio;
    float raster_tile_latency_ns;
    uint64_t verified_raster_saat_clearances;

    bool navi48_raster_pipeline_verified;
    bool raster_strategy_merkle_verified;
    bool raster_submicro_latency_verified;
    bool raster_lossless_saat_verified;
    bool sovereign_2150_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanNavi48RasterBeyond2145State;

int cpm_tomie_renderman_navi48_init(RendermanNavi48RasterContext *ctx);
int cpm_tomie_renderman_navi48_rasterize_frame(RendermanNavi48RasterContext *ctx, uint32_t micropolygon_count);
void auncient_renderman_navi48_raster_init(RendermanNavi48RasterBeyond2145State *state);
bool auncient_renderman_navi48_raster_verify_theorems_2146_2150(RendermanNavi48RasterBeyond2145State *state);
uint32_t auncient_renderman_navi48_raster_compute_rule18(const RendermanNavi48RasterBeyond2145State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_NAVI48_RASTERIZER_THEOREMS_2146_2150_H */
