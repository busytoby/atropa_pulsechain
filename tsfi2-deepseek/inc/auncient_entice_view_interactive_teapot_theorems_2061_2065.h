#ifndef AUNCIENT_ENTICE_VIEW_INTERACTIVE_TEAPOT_THEOREMS_2061_2065_H
#define AUNCIENT_ENTICE_VIEW_INTERACTIVE_TEAPOT_THEOREMS_2061_2065_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VIEW_MODE_WIREFRAME = 0,
    VIEW_MODE_FLAT = 1,
    VIEW_MODE_SMOOTH_SHADED = 2,
    VIEW_MODE_DISPLACED_MICROPOLYGONS = 3,
    VIEW_MODE_COUNT = 4
} EnticeViewMode;

typedef struct {
    EnticeViewMode current_view_mode;
    double camera_pitch;
    double camera_yaw;
    double camera_zoom;
    uint32_t cdc6600_transformed_vertices;
    uint32_t viewport_frames_rendered;
    bool is_view_matrix_locked;
    bool is_entice_pageturner_synced;
} EnticeViewInteractiveContext;

typedef struct {
    float in_silicon_entice_fidelity;
    float entice_strategy_datbin_merkle_ratio;
    float entice_view_transform_latency_ns;
    uint64_t verified_entice_saat_clearances;

    bool entice_view_pipeline_verified;
    bool entice_strategy_merkle_verified;
    bool entice_submicro_latency_verified;
    bool entice_lossless_saat_verified;
    bool sovereign_2065_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} EnticeViewInteractiveBeyond2060State;

int cpm_tomie_entice_view_init(EnticeViewInteractiveContext *ctx);
int cpm_tomie_entice_view_orbit_camera(EnticeViewInteractiveContext *ctx, double delta_pitch, double delta_yaw);
int cpm_tomie_entice_view_switch_mode(EnticeViewInteractiveContext *ctx, EnticeViewMode new_mode);
void auncient_entice_view_interactive_init(EnticeViewInteractiveBeyond2060State *state);
bool auncient_entice_view_interactive_verify_theorems_2061_2065(EnticeViewInteractiveBeyond2060State *state);
uint32_t auncient_entice_view_interactive_compute_rule18(const EnticeViewInteractiveBeyond2060State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ENTICE_VIEW_INTERACTIVE_TEAPOT_THEOREMS_2061_2065_H */
