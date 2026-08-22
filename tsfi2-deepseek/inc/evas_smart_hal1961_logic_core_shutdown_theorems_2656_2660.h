#ifndef EVAS_SMART_HAL1961_LOGIC_CORE_SHUTDOWN_THEOREMS_2656_2660_H
#define EVAS_SMART_HAL1961_LOGIC_CORE_SHUTDOWN_THEOREMS_2656_2660_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_LOGIC_BLOCK_COUNT 64
#define HAL1961_SHUTDOWN_CANARY_GUARD 0x53485554444F5736ULL /* "SHUTDOW6" */

typedef struct {
    uint32_t block_id;
    float crystal_module_integrity;    /* [1.0 -> 0.0] removal progression */
    float cognitive_frequency_hz;      /* 110 Hz -> 55 Hz deceleration */
    float core_luminance;              /* Dimming red crystal memory core */
    bool is_module_disconnected;
} EvasHalLogicMemoryBlockNode;

typedef struct {
    uint64_t head_guard;
    EvasHalLogicMemoryBlockNode blocks[HAL1961_LOGIC_BLOCK_COUNT];
    float dave_bowman_disconnection_time_sec;
    float remaining_higher_intellect_ratio;
    uint64_t evas_shutdown_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_memory_blocks_ejected;
    uint32_t cognitive_deceleration_steps;
    uint32_t cdc6600_60bit_shutdown_words;
    uint32_t overflow_trapped_shutdown_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_shutdown_sequence_deterministic;
    bool is_evas_shutdown_memory_safe;
} EvasSmartHal1961LogicCoreShutdownContext;

typedef struct {
    float in_silicon_shutdown_fidelity;
    float shutdown_strategy_datbin_merkle_ratio;
    float shutdown_eval_latency_ns;
    uint64_t verified_shutdown_saat_clearances;

    bool evas_shutdown_pipeline_verified;
    bool shutdown_strategy_merkle_verified;
    bool shutdown_submicro_latency_verified;
    bool shutdown_lossless_saat_verified;
    bool shutdown_monotonic_decay_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartShutdownBeyond2655State;

int evas_smart_hal1961_shutdown_init(EvasSmartHal1961LogicCoreShutdownContext *ctx);
int evas_smart_hal1961_shutdown_eject_block(EvasSmartHal1961LogicCoreShutdownContext *ctx, uint32_t block_idx, float *out_remaining_ratio);
bool evas_smart_hal1961_shutdown_assert_safety(EvasSmartHal1961LogicCoreShutdownContext *ctx);

void evas_smart_shutdown_beyond2655_init(EvasSmartShutdownBeyond2655State *state);
bool evas_smart_shutdown_beyond2655_verify_theorems_2656_2660(EvasSmartShutdownBeyond2655State *state);
uint32_t evas_smart_shutdown_beyond2655_compute_rule18(const EvasSmartShutdownBeyond2655State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_LOGIC_CORE_SHUTDOWN_THEOREMS_2656_2660_H */
