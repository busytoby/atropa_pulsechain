#ifndef AUNCIENT_RENDERMAN_TEAPOT_DEMO_THEOREMS_2001_2005_H
#define AUNCIENT_RENDERMAN_TEAPOT_DEMO_THEOREMS_2001_2005_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_TEAPOT_PATCHES 32
#define RENDERMAN_MICROPOLYGONS_PER_PATCH 256
#define RENDERMAN_FRAMEBUFFER_DIM 512

typedef struct {
    uint32_t patch_count;
    uint32_t micropolygons_generated;
    uint32_t amd_fabric_dies_active;
    uint32_t displacement_vertices_calculated;
    bool is_reyes_pipeline_converged;
    bool is_framebuffer_ready;
} RenderManTeapotDemoContext;

typedef struct {
    float in_silicon_renderman_fidelity;
    float renderman_strategy_datbin_merkle_ratio;
    float renderman_tile_render_latency_ns;
    uint64_t verified_renderman_saat_clearances;

    bool renderman_teapot_reyes_verified;
    bool renderman_strategy_merkle_verified;
    bool renderman_submicro_latency_verified;
    bool renderman_lossless_saat_verified;
    bool grand_2005_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RenderManTeapotBeyond2000State;

int cpm_tomie_renderman_teapot_init(RenderManTeapotDemoContext *ctx);
int cpm_tomie_renderman_teapot_tessellate_and_displace(RenderManTeapotDemoContext *ctx);
void auncient_renderman_teapot_init(RenderManTeapotBeyond2000State *state);
bool auncient_renderman_teapot_verify_theorems_2001_2005(RenderManTeapotBeyond2000State *state);
uint32_t auncient_renderman_teapot_compute_rule18(const RenderManTeapotBeyond2000State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_TEAPOT_DEMO_THEOREMS_2001_2005_H */
