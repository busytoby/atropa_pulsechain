#ifndef EVAS_SMART_HAL1961_DISCOVERY_FLIGHT_DECK_THEOREMS_2651_2655_H
#define EVAS_SMART_HAL1961_DISCOVERY_FLIGHT_DECK_THEOREMS_2651_2655_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_DISCOVERY_CONSOLES 64
#define HAL1961_FLIGHT_DECK_CANARY_GUARD 0x444953434F563636ULL /* "DISCOV66" */

typedef struct {
    uint32_t console_id;
    float telemetry_gauge_value;    /* Bounded [0.0, 1.0] */
    float phosphor_vector_color[3]; /* Cockpit CRT phosphor luminescence */
    float blink_frequency_hz;       /* 1.0 Hz - 4.0 Hz alert frequency */
    bool is_console_nominal;
} EvasHalDiscoveryConsoleNode;

typedef struct {
    uint64_t head_guard;
    EvasHalDiscoveryConsoleNode consoles[HAL1961_DISCOVERY_CONSOLES];
    float cabin_centrifuge_rpm;     /* 3.0 RPM artificial gravity ring */
    float oxygen_nitrogen_ratio;    /* 0.21 / 0.79 atmospheric mix */
    uint64_t evas_flight_deck_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_cockpit_cycles_executed;
    uint32_t console_telemetry_sweeps;
    uint32_t cdc6600_60bit_flight_deck_words;
    uint32_t overflow_trapped_flight_deck_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_flight_deck_telemetry_lossless;
    bool is_evas_flight_deck_memory_safe;
} EvasSmartHal1961DiscoveryFlightDeckContext;

typedef struct {
    float in_silicon_flight_deck_fidelity;
    float flight_deck_strategy_datbin_merkle_ratio;
    float flight_deck_render_latency_ns;
    uint64_t verified_flight_deck_saat_clearances;

    bool evas_flight_deck_pipeline_verified;
    bool flight_deck_strategy_merkle_verified;
    bool flight_deck_submicro_latency_verified;
    bool flight_deck_lossless_saat_verified;
    bool flight_deck_telemetry_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartFlightDeckBeyond2650State;

int evas_smart_hal1961_flight_deck_init(EvasSmartHal1961DiscoveryFlightDeckContext *ctx);
int evas_smart_hal1961_flight_deck_update_console(EvasSmartHal1961DiscoveryFlightDeckContext *ctx, uint32_t console_idx, float val, float out_rgb[3]);
bool evas_smart_hal1961_flight_deck_assert_safety(EvasSmartHal1961DiscoveryFlightDeckContext *ctx);

void evas_smart_flight_deck_beyond2650_init(EvasSmartFlightDeckBeyond2650State *state);
bool evas_smart_flight_deck_beyond2650_verify_theorems_2651_2655(EvasSmartFlightDeckBeyond2650State *state);
uint32_t evas_smart_flight_deck_beyond2650_compute_rule18(const EvasSmartFlightDeckBeyond2650State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_DISCOVERY_FLIGHT_DECK_THEOREMS_2651_2655_H */
