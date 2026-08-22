#ifndef AUNCIENT_TSFI2_COCKPIT_UNIFIED_PIPELINE_THEOREMS_2351_2355_H
#define AUNCIENT_TSFI2_COCKPIT_UNIFIED_PIPELINE_THEOREMS_2351_2355_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COCKPIT_INSTRUMENT_DIALS 8
#define COCKPIT_TELEMETRY_BINS 32
#define TSFI2_COCKPIT_CANARY_GUARD 0x434F434B50495436ULL /* "COCKPIT6" */

typedef struct {
    uint32_t instrument_id;
    float needle_angle_rad;
    float telemetry_density_ash;
    float scoreboard_execution_load;
    bool is_instrument_calibrated;
} CockpitInstrumentDial;

typedef struct {
    uint64_t head_guard;
    CockpitInstrumentDial instruments[COCKPIT_INSTRUMENT_DIALS];
    double scott_density_bins[COCKPIT_TELEMETRY_BINS];
    uint64_t cockpit_rebar_vram_latch[64];
    uint64_t tail_guard;
    uint32_t total_cockpit_frames_rendered;
    uint32_t scoreboard_instructions_dispatched;
    uint32_t cdc6600_60bit_cockpit_words;
    uint32_t overflow_trapped_telemetry;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_cockpit_pipeline_synchronized;
    bool is_tsfi2_cockpit_memory_safe;
} Tsfi2CockpitUnifiedContext;

typedef struct {
    float in_silicon_cockpit_fidelity;
    float cockpit_strategy_datbin_merkle_ratio;
    float cockpit_refresh_latency_ns;
    uint64_t verified_cockpit_saat_clearances;

    bool cockpit_pipeline_verified;
    bool cockpit_strategy_merkle_verified;
    bool cockpit_submicro_latency_verified;
    bool cockpit_lossless_saat_verified;
    bool sovereign_2355_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Tsfi2CockpitBeyond2350State;

int cpm_tomie_tsfi2_cockpit_init(Tsfi2CockpitUnifiedContext *ctx);
int cpm_tomie_tsfi2_cockpit_render_tick(Tsfi2CockpitUnifiedContext *ctx, const double *telemetry_samples, size_t sample_count);
bool cpm_tomie_tsfi2_cockpit_assert_safety(Tsfi2CockpitUnifiedContext *ctx);

void auncient_tsfi2_cockpit_init(Tsfi2CockpitBeyond2350State *state);
bool auncient_tsfi2_cockpit_verify_theorems_2351_2355(Tsfi2CockpitBeyond2350State *state);
uint32_t auncient_tsfi2_cockpit_compute_rule18(const Tsfi2CockpitBeyond2350State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_TSFI2_COCKPIT_UNIFIED_PIPELINE_THEOREMS_2351_2355_H */
