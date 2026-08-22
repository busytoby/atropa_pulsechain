#ifndef AUNCIENT_CDC6600_RENDERMAN_TEAPOT_THEOREMS_2006_2010_H
#define AUNCIENT_CDC6600_RENDERMAN_TEAPOT_THEOREMS_2006_2010_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_PPU_COUNT 10
#define CDC6600_FUNCTIONAL_UNITS 10
#define CDC6600_REG_COUNT 24 /* 8 X (60-bit), 8 A (18-bit), 8 B (18-bit) */

typedef struct {
    uint64_t x_regs[8];  /* 60-bit operand registers */
    uint32_t a_regs[8];  /* 18-bit address registers */
    uint32_t b_regs[8];  /* 18-bit index registers */
    uint32_t ppu_active_mask;
    uint32_t patches_tessellated;
    uint32_t micropolygons_shaded;
    bool is_scoreboard_cleared;
    bool is_framebuffer_streamed;
} Cdc6600RenderManTeapotContext;

typedef struct {
    float in_silicon_cdc6600_renderman_fidelity;
    float cdc6600_renderman_strategy_datbin_merkle_ratio;
    float cdc6600_renderman_dispatch_latency_ns;
    uint64_t verified_cdc6600_renderman_saat_clearances;

    bool cdc6600_renderman_reyes_verified;
    bool cdc6600_renderman_strategy_merkle_verified;
    bool cdc6600_renderman_submicro_latency_verified;
    bool cdc6600_renderman_lossless_saat_verified;
    bool grand_2010_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600RenderManBeyond2005State;

int cpm_tomie_cdc6600_renderman_init(Cdc6600RenderManTeapotContext *ctx);
int cpm_tomie_cdc6600_renderman_render_frame(Cdc6600RenderManTeapotContext *ctx);
void auncient_cdc6600_renderman_init(Cdc6600RenderManBeyond2005State *state);
bool auncient_cdc6600_renderman_verify_theorems_2006_2010(Cdc6600RenderManBeyond2005State *state);
uint32_t auncient_cdc6600_renderman_compute_rule18(const Cdc6600RenderManBeyond2005State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_RENDERMAN_TEAPOT_THEOREMS_2006_2010_H */
