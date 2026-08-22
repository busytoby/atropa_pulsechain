#ifndef AUNCIENT_TERMINOLOGY_VT100_RENDERMAN_OVERLAY_THEOREMS_2056_2060_H
#define AUNCIENT_TERMINOLOGY_VT100_RENDERMAN_OVERLAY_THEOREMS_2056_2060_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TERMINOLOGY_TEXT_ROWS 24
#define TERMINOLOGY_TEXT_COLS 80
#define TERMINOLOGY_TOTAL_CELLS (TERMINOLOGY_TEXT_ROWS * TERMINOLOGY_TEXT_COLS)

typedef struct {
    char text_grid[TERMINOLOGY_TEXT_ROWS][TERMINOLOGY_TEXT_COLS];
    uint32_t cursor_x;
    uint32_t cursor_y;
    uint32_t cdc6600_char_blits;
    uint32_t renderman_overlay_frames;
    bool is_alpha_composite_locked;
    bool is_ansi_escape_parser_synced;
} TerminologyRenderManOverlayContext;

typedef struct {
    float in_silicon_terminology_overlay_fidelity;
    float terminology_overlay_strategy_datbin_merkle_ratio;
    float terminology_overlay_glyph_latency_ns;
    uint64_t verified_terminology_overlay_saat_clearances;

    bool terminology_overlay_verified;
    bool terminology_strategy_merkle_verified;
    bool terminology_submicro_latency_verified;
    bool terminology_lossless_saat_verified;
    bool sovereign_2060_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} TerminologyRenderManOverlayBeyond2055State;

int cpm_tomie_terminology_overlay_init(TerminologyRenderManOverlayContext *ctx);
int cpm_tomie_terminology_overlay_write_string(TerminologyRenderManOverlayContext *ctx, uint32_t row, uint32_t col, const char *str);
int cpm_tomie_terminology_overlay_composite_frame(TerminologyRenderManOverlayContext *ctx);
void auncient_terminology_overlay_init(TerminologyRenderManOverlayBeyond2055State *state);
bool auncient_terminology_overlay_verify_theorems_2056_2060(TerminologyRenderManOverlayBeyond2055State *state);
uint32_t auncient_terminology_overlay_compute_rule18(const TerminologyRenderManOverlayBeyond2055State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_TERMINOLOGY_VT100_RENDERMAN_OVERLAY_THEOREMS_2056_2060_H */
