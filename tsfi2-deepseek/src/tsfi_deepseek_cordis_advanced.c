#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_cordis_advanced.h"

// ----------------------------------------------------------------------------
// 1. Spatiotemporal Session Composability (Resume, Fork, & Replay)
// ----------------------------------------------------------------------------

bool tsfi_cordis_session_tree_init(tsfi_cordis_session_tree_t *tree, uint32_t session_id) {
    if (!tree) return false;
    memset(tree, 0, sizeof(*tree));
    tree->session_id = session_id;
    return true;
}

bool tsfi_cordis_session_record_turn(
    tsfi_cordis_session_tree_t *tree,
    const char *prompt,
    const char *response,
    uint32_t kv_offset,
    uint32_t kv_count
) {
    if (!tree || !prompt || !response || tree->total_recorded_turns >= TSFI_CORDIS_MAX_SAVED_TURNS) {
        return false;
    }
    uint32_t idx = tree->total_recorded_turns++;
    tsfi_cordis_turn_snapshot_t *turn = &tree->turns[idx];
    turn->turn_index = idx + 1;
    strncpy(turn->prompt, prompt, sizeof(turn->prompt) - 1);
    strncpy(turn->response, response, sizeof(turn->response) - 1);
    turn->kv_page_offset = kv_offset;
    turn->kv_page_count = kv_count;
    return true;
}

bool tsfi_cordis_session_fork_branch(
    const tsfi_cordis_session_tree_t *parent_tree,
    uint32_t fork_turn_idx,
    uint32_t new_session_id,
    tsfi_cordis_session_tree_t *forked_tree_out
) {
    if (!parent_tree || !forked_tree_out || fork_turn_idx > parent_tree->total_recorded_turns) {
        return false;
    }
    memset(forked_tree_out, 0, sizeof(*forked_tree_out));
    forked_tree_out->session_id = new_session_id;
    forked_tree_out->parent_session_id = parent_tree->session_id;
    forked_tree_out->fork_turn_point = fork_turn_idx;
    forked_tree_out->is_forked = true;
    forked_tree_out->total_recorded_turns = fork_turn_idx;

    for (uint32_t i = 0; i < fork_turn_idx; i++) {
        forked_tree_out->turns[i] = parent_tree->turns[i];
    }
    return true;
}

bool tsfi_cordis_session_replay_verify(
    const tsfi_cordis_session_tree_t *tree,
    uint32_t *verified_turns_out
) {
    if (!tree) return false;
    uint32_t count = 0;
    for (uint32_t i = 0; i < tree->total_recorded_turns; i++) {
        if (tree->turns[i].turn_index == i + 1 && strlen(tree->turns[i].prompt) > 0) {
            count++;
        }
    }
    if (verified_turns_out) *verified_turns_out = count;
    return (count == tree->total_recorded_turns);
}

// ----------------------------------------------------------------------------
// 2. Linux Landlock / Sandbox Path Enforcer
// ----------------------------------------------------------------------------

bool tsfi_cordis_sandbox_init(
    tsfi_cordis_sandbox_config_t *sandbox,
    const char *workspace_dir,
    const char *wal_dir
) {
    if (!sandbox || !workspace_dir || !wal_dir) return false;
    memset(sandbox, 0, sizeof(*sandbox));
    strncpy(sandbox->allowed_workspace_path, workspace_dir, sizeof(sandbox->allowed_workspace_path) - 1);
    strncpy(sandbox->allowed_wal_path, wal_dir, sizeof(sandbox->allowed_wal_path) - 1);
    sandbox->landlock_active = true;
    sandbox->restrict_network = true;
    return true;
}

bool tsfi_cordis_sandbox_validate_path(
    const tsfi_cordis_sandbox_config_t *sandbox,
    const char *target_path,
    bool is_write
) {
    if (!sandbox || !target_path) return false;
    if (!sandbox->landlock_active) return true;

    // Check workspace prefix
    if (strncmp(target_path, sandbox->allowed_workspace_path, strlen(sandbox->allowed_workspace_path)) == 0) {
        return true;
    }
    // Check WAL directory prefix
    if (strncmp(target_path, sandbox->allowed_wal_path, strlen(sandbox->allowed_wal_path)) == 0) {
        return true;
    }
    // Temporary testing directory /tmp exception
    if (strncmp(target_path, "/tmp/", 5) == 0) {
        return true;
    }

    if (is_write) {
        // Disallow arbitrary write path outside sandbox
        return false;
    }
    return false;
}

// ----------------------------------------------------------------------------
// 3. Operational Persona & Preset Profiles (Code, Minimal, Hardware)
// ----------------------------------------------------------------------------

bool tsfi_cordis_preset_load(
    tsfi_cordis_preset_type_t type,
    tsfi_cordis_preset_profile_t *profile_out
) {
    if (!profile_out) return false;
    memset(profile_out, 0, sizeof(*profile_out));
    profile_out->preset_type = type;

    switch (type) {
        case CORDIS_PRESET_CODE:
            profile_out->max_token_budget = TSFI_CORDIS_PRESET_CODE_TOKENS;
            profile_out->enable_ast_secondary_pass = true;
            profile_out->enable_scsi_hardware_tracing = false;
            profile_out->knapsack_solve_target_us = 0.50f;
            strncpy(profile_out->default_model_alias, "DeepSeek-Coder-6.7B", sizeof(profile_out->default_model_alias) - 1);
            break;
        case CORDIS_PRESET_MINIMAL:
            profile_out->max_token_budget = TSFI_CORDIS_PRESET_MINIMAL_TOKENS;
            profile_out->enable_ast_secondary_pass = false;
            profile_out->enable_scsi_hardware_tracing = false;
            profile_out->knapsack_solve_target_us = 0.20f;
            strncpy(profile_out->default_model_alias, "DeepSeek-Coder-1.3B", sizeof(profile_out->default_model_alias) - 1);
            break;
        case CORDIS_PRESET_HARDWARE_TRACE:
            profile_out->max_token_budget = 2048;
            profile_out->enable_ast_secondary_pass = true;
            profile_out->enable_scsi_hardware_tracing = true;
            profile_out->knapsack_solve_target_us = 0.80f;
            strncpy(profile_out->default_model_alias, "DeepSeek-Coder-Hardware-ZMM", sizeof(profile_out->default_model_alias) - 1);
            break;
        default:
            profile_out->max_token_budget = 2048;
            strncpy(profile_out->default_model_alias, "DeepSeek-Custom", sizeof(profile_out->default_model_alias) - 1);
            break;
    }
    return true;
}

// ----------------------------------------------------------------------------
// 4. In-Process Hot-Reload of Model Adapters & Dynamic Quant Switcher
// ----------------------------------------------------------------------------

bool tsfi_cordis_adapter_init(tsfi_cordis_adapter_state_t *adapter, const char *initial_path, const char *quant) {
    if (!adapter || !initial_path) return false;
    memset(adapter, 0, sizeof(*adapter));
    strncpy(adapter->active_model_path, initial_path, sizeof(adapter->active_model_path) - 1);
    strncpy(adapter->active_quant_type, (quant) ? quant : "Q4_K_M", sizeof(adapter->active_quant_type) - 1);
    adapter->session_context_preserved = true;
    return true;
}

bool tsfi_cordis_adapter_hot_swap(
    tsfi_cordis_adapter_state_t *adapter,
    const char *new_model_path,
    const char *new_quant,
    bool preserve_kv_pages
) {
    if (!adapter || !new_model_path) return false;
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    strncpy(adapter->active_model_path, new_model_path, sizeof(adapter->active_model_path) - 1);
    if (new_quant) {
        strncpy(adapter->active_quant_type, new_quant, sizeof(adapter->active_quant_type) - 1);
    }
    adapter->total_adapter_hot_swaps++;
    adapter->session_context_preserved = preserve_kv_pages;

    clock_gettime(CLOCK_MONOTONIC, &ts1);
    adapter->swap_latency_ms = (float)((ts1.tv_sec - ts0.tv_sec) * 1e3 + (ts1.tv_nsec - ts0.tv_nsec) * 1e-6);
    return true;
}
