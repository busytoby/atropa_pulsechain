#ifndef TSFI_DEEPSEEK_CORDIS_COMPOSABILITY_H
#define TSFI_DEEPSEEK_CORDIS_COMPOSABILITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_deepseek_cordis_harness.h"
#include "tsfi_deepseek_edsac_acp_firewall.h"

// ----------------------------------------------------------------------------
// 1. Spatiotemporal Composition Stage Definitions
// ----------------------------------------------------------------------------

typedef enum {
    CORDIS_STAGE_IDLE = 0,
    CORDIS_STAGE_PLAN = 1,
    CORDIS_STAGE_SPECULATIVE_PRE_EXEC = 2,
    CORDIS_STAGE_FIREWALL_VALIDATION = 3,
    CORDIS_STAGE_COMMIT = 4,
    CORDIS_STAGE_ROLLBACK = 5
} tsfi_cordis_stage_t;

typedef struct {
    char stage_name[32];
    uint64_t stage_start_ns;
    uint64_t stage_duration_ns;
    bool stage_passed;
} tsfi_cordis_stage_metrics_t;

// ----------------------------------------------------------------------------
// 2. In-Memory Static Prefix Cache Pinning
// ----------------------------------------------------------------------------

#define TSFI_MAX_PREFIX_PAGES 16
#define TSFI_PREFIX_PAGE_SIZE 4096

typedef struct {
    uint32_t total_pinned_pages;
    uint32_t pinned_token_count;
    uint64_t prefix_fingerprint;
    uint8_t page_buffer[TSFI_MAX_PREFIX_PAGES][TSFI_PREFIX_PAGE_SIZE];
    uint64_t cache_hit_count;
    uint64_t cache_miss_count;
} tsfi_cordis_prefix_cache_t;

bool tsfi_cordis_prefix_cache_init(tsfi_cordis_prefix_cache_t *cache);
bool tsfi_cordis_prefix_cache_pin(
    tsfi_cordis_prefix_cache_t *cache,
    const char *static_system_prompt,
    uint32_t token_count
);
bool tsfi_cordis_prefix_cache_lookup(
    tsfi_cordis_prefix_cache_t *cache,
    const char *incoming_prompt,
    bool *cache_hit_out
);

// ----------------------------------------------------------------------------
// 3. Multi-Profile Layering & Presets Engine
// ----------------------------------------------------------------------------

typedef enum {
    CORDIS_PROFILE_MINIMAL = 0,
    CORDIS_PROFILE_CODE = 1,
    CORDIS_PROFILE_HARDWARE_TRACE = 2,
    CORDIS_PROFILE_CUSTOM = 3
} tsfi_cordis_profile_type_t;

typedef struct {
    tsfi_cordis_profile_type_t type;
    char profile_name[32];
    uint32_t max_allowed_tokens;
    bool enable_landlock_sandbox;
    bool enable_stanag_coaxial;
    uint32_t max_concurrency;
} tsfi_cordis_profile_config_t;

// ----------------------------------------------------------------------------
// 4. Unified Spatiotemporal Composition Loop Controller
// ----------------------------------------------------------------------------

typedef struct {
    tsfi_cordis_stage_t current_stage;
    tsfi_cordis_profile_config_t active_profile;
    tsfi_cordis_prefix_cache_t prefix_cache;
    tsfi_cordis_stage_metrics_t stage_history[8];
    uint32_t stage_history_count;
    uint64_t total_composition_cycles;
} tsfi_cordis_composability_engine_t;

bool tsfi_cordis_composability_init(
    tsfi_cordis_composability_engine_t *engine,
    tsfi_cordis_profile_type_t profile_type
);

bool tsfi_cordis_composability_execute_cycle(
    tsfi_cordis_composability_engine_t *engine,
    tsfi_edsac_acp_firewall_state_t *firewall,
    const char *plan_intent,
    const char *tool_candidate,
    const char *tool_args,
    char *result_out,
    size_t max_result_len
);

#endif // TSFI_DEEPSEEK_CORDIS_COMPOSABILITY_H
