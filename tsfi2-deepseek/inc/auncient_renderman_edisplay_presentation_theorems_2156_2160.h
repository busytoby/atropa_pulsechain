#ifndef AUNCIENT_RENDERMAN_EDISPLAY_PRESENTATION_THEOREMS_2156_2160_H
#define AUNCIENT_RENDERMAN_EDISPLAY_PRESENTATION_THEOREMS_2156_2160_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char window_title[64];
    uint32_t width;
    uint32_t height;
    uint32_t rendered_frames;
    uint32_t ecore_evas_events_handled;
    bool is_edisplay_window_registered;
    bool is_interactive_orbit_active;
} EdisplayPresentationContext;

typedef struct {
    float in_silicon_presentation_fidelity;
    float presentation_strategy_datbin_merkle_ratio;
    float presentation_flip_latency_ns;
    uint64_t verified_presentation_saat_clearances;

    bool edisplay_presentation_verified;
    bool presentation_strategy_merkle_verified;
    bool presentation_submicro_latency_verified;
    bool presentation_lossless_saat_verified;
    bool sovereign_2160_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} EdisplayPresentationBeyond2155State;

int cpm_tomie_edisplay_init(EdisplayPresentationContext *ctx);
int cpm_tomie_edisplay_present_frame(EdisplayPresentationContext *ctx, uint32_t frame_index);
void auncient_edisplay_presentation_init(EdisplayPresentationBeyond2155State *state);
bool auncient_edisplay_presentation_verify_theorems_2156_2160(EdisplayPresentationBeyond2155State *state);
uint32_t auncient_edisplay_presentation_compute_rule18(const EdisplayPresentationBeyond2155State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_EDISPLAY_PRESENTATION_THEOREMS_2156_2160_H */
