#ifndef TSFI_DEEPSEEK_CORDIS_ADVANCED_H
#define TSFI_DEEPSEEK_CORDIS_ADVANCED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_deepseek_cordis_harness.h"

#define TSFI_CORDIS_MAX_FORK_BRANCHES     8
#define TSFI_CORDIS_MAX_SAVED_TURNS       32
#define TSFI_CORDIS_PRESET_CODE_TOKENS    4096
#define TSFI_CORDIS_PRESET_MINIMAL_TOKENS 1024

// ============================================================================
// 1. Spatiotemporal Session Composability (Resume, Fork, & Replay)
// ============================================================================

typedef struct {
    uint32_t turn_index;
    char prompt[2048];
    char response[4096];
    uint32_t kv_page_offset;
    uint32_t kv_page_count;
} tsfi_cordis_turn_snapshot_t;

typedef struct {
    uint32_t session_id;
    uint32_t parent_session_id;
    uint32_t fork_turn_point;
    uint32_t total_recorded_turns;
    tsfi_cordis_turn_snapshot_t turns[TSFI_CORDIS_MAX_SAVED_TURNS];
    bool is_forked;
} tsfi_cordis_session_tree_t;

bool tsfi_cordis_session_tree_init(tsfi_cordis_session_tree_t *tree, uint32_t session_id);
bool tsfi_cordis_session_record_turn(
    tsfi_cordis_session_tree_t *tree,
    const char *prompt,
    const char *response,
    uint32_t kv_offset,
    uint32_t kv_count
);
bool tsfi_cordis_session_fork_branch(
    const tsfi_cordis_session_tree_t *parent_tree,
    uint32_t fork_turn_idx,
    uint32_t new_session_id,
    tsfi_cordis_session_tree_t *forked_tree_out
);
bool tsfi_cordis_session_replay_verify(
    const tsfi_cordis_session_tree_t *tree,
    uint32_t *verified_turns_out
);

// ============================================================================
// 2. Linux Landlock / Sandbox Enforcer for Tool Execution
// ============================================================================

typedef struct {
    char allowed_workspace_path[256];
    char allowed_wal_path[256];
    bool landlock_active;
    bool restrict_network;
    uint32_t violations_intercepted;
} tsfi_cordis_sandbox_config_t;

bool tsfi_cordis_sandbox_init(
    tsfi_cordis_sandbox_config_t *sandbox,
    const char *workspace_dir,
    const char *wal_dir
);
bool tsfi_cordis_sandbox_validate_path(
    const tsfi_cordis_sandbox_config_t *sandbox,
    const char *target_path,
    bool is_write
);

// ============================================================================
// 3. Operational Persona & Preset Profiles (Code, Minimal, Hardware)
// ============================================================================

typedef enum {
    CORDIS_PRESET_CODE = 0,
    CORDIS_PRESET_MINIMAL,
    CORDIS_PRESET_HARDWARE_TRACE,
    CORDIS_PRESET_CUSTOM
} tsfi_cordis_preset_type_t;

typedef struct {
    tsfi_cordis_preset_type_t preset_type;
    uint32_t max_token_budget;
    bool enable_ast_secondary_pass;
    bool enable_scsi_hardware_tracing;
    float knapsack_solve_target_us;
    char default_model_alias[64];
} tsfi_cordis_preset_profile_t;

bool tsfi_cordis_preset_load(
    tsfi_cordis_preset_type_t type,
    tsfi_cordis_preset_profile_t *profile_out
);

// ============================================================================
// 4. In-Process Hot-Reload of Model Adapters & Dynamic Quant Switcher
// ============================================================================

typedef struct {
    char active_model_path[256];
    char active_quant_type[16]; // e.g. "Q4_K_M", "Q8_0", "F16"
    uint32_t total_adapter_hot_swaps;
    float swap_latency_ms;
    bool session_context_preserved;
} tsfi_cordis_adapter_state_t;

bool tsfi_cordis_adapter_init(tsfi_cordis_adapter_state_t *adapter, const char *initial_path, const char *quant);
bool tsfi_cordis_adapter_hot_swap(
    tsfi_cordis_adapter_state_t *adapter,
    const char *new_model_path,
    const char *new_quant,
    bool preserve_kv_pages
);

#endif // TSFI_DEEPSEEK_CORDIS_ADVANCED_H
