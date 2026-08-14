#ifndef TSFI_DEEPSEEK_CORDIS_CHAIN_H
#define TSFI_DEEPSEEK_CORDIS_CHAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_deepseek_cordis_harness.h"

#define TSFI_CORDIS_MAX_SIDE_EFFECTS    64
#define TSFI_CORDIS_MAX_CHAIN_STEPS     16

// ============================================================================
// 1. Temporal Composability & Side-Effect Scoper
// ============================================================================

typedef enum {
    EFFECT_HEAP_PAGE = 0,
    EFFECT_FILE_HANDLE,
    EFFECT_TOOL_HOOK,
    EFFECT_SCSI_LOCK
} tsfi_effect_type_t;

typedef struct {
    tsfi_effect_type_t type;
    void *resource_ptr;
    int descriptor_id;
    uint32_t turn_owner_id;
} tsfi_cordis_side_effect_t;

typedef struct {
    uint32_t active_effects_count;
    tsfi_cordis_side_effect_t effects[TSFI_CORDIS_MAX_SIDE_EFFECTS];
    uint32_t total_cleanups_performed;
} tsfi_cordis_temporal_scope_t;

bool tsfi_cordis_temporal_scope_init(tsfi_cordis_temporal_scope_t *scope);
bool tsfi_cordis_track_side_effect(
    tsfi_cordis_temporal_scope_t *scope,
    tsfi_effect_type_t type,
    void *res_ptr,
    int desc_id,
    uint32_t turn_id
);
bool tsfi_cordis_rollback_turn_effects(
    tsfi_cordis_temporal_scope_t *scope,
    uint32_t target_turn_id
);

// ============================================================================
// 2. Programmatic Multi-Step Tool Chaining Engine ("Code Mode")
// ============================================================================

typedef struct {
    char plugin_name[64];
    char command[64];
    char arguments[256];
    bool stop_on_failure;
} tsfi_cordis_chain_step_t;

typedef struct {
    uint32_t total_steps;
    tsfi_cordis_chain_step_t steps[TSFI_CORDIS_MAX_CHAIN_STEPS];
    uint32_t executed_steps;
    bool atomic_transaction;
    float chain_execution_time_us;
} tsfi_cordis_tool_chain_t;

bool tsfi_cordis_tool_chain_init(tsfi_cordis_tool_chain_t *chain, bool atomic);
bool tsfi_cordis_chain_add_step(
    tsfi_cordis_tool_chain_t *chain,
    const char *plugin,
    const char *cmd,
    const char *args,
    bool stop_on_fail
);
bool tsfi_cordis_execute_chain(
    tsfi_cordis_tool_chain_t *chain,
    tsfi_cordis_registry_t *reg,
    char *aggregate_out,
    size_t max_out_len
);

#endif // TSFI_DEEPSEEK_CORDIS_CHAIN_H
