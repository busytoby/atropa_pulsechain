#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tsfi_deepseek_cordis_chain.h"

// ----------------------------------------------------------------------------
// 1. Temporal Composability & Side-Effect Scoper Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_temporal_scope_init(tsfi_cordis_temporal_scope_t *scope) {
    if (!scope) return false;
    memset(scope, 0, sizeof(*scope));
    return true;
}

bool tsfi_cordis_track_side_effect(
    tsfi_cordis_temporal_scope_t *scope,
    tsfi_effect_type_t type,
    void *res_ptr,
    int desc_id,
    uint32_t turn_id
) {
    if (!scope || scope->active_effects_count >= TSFI_CORDIS_MAX_SIDE_EFFECTS) {
        return false;
    }
    uint32_t idx = scope->active_effects_count++;
    tsfi_cordis_side_effect_t *eff = &scope->effects[idx];
    eff->type = type;
    eff->resource_ptr = res_ptr;
    eff->descriptor_id = desc_id;
    eff->turn_owner_id = turn_id;
    return true;
}

bool tsfi_cordis_rollback_turn_effects(
    tsfi_cordis_temporal_scope_t *scope,
    uint32_t target_turn_id
) {
    if (!scope) return false;
    for (uint32_t i = 0; i < scope->active_effects_count; i++) {
        tsfi_cordis_side_effect_t *eff = &scope->effects[i];
        if (eff->turn_owner_id == target_turn_id) {
            switch (eff->type) {
                case EFFECT_HEAP_PAGE:
                    if (eff->resource_ptr) {
                        free(eff->resource_ptr);
                        eff->resource_ptr = NULL;
                    }
                    break;
                case EFFECT_FILE_HANDLE:
                    if (eff->descriptor_id >= 0) {
                        close(eff->descriptor_id);
                        eff->descriptor_id = -1;
                    }
                    break;
                default:
                    break;
            }
            scope->total_cleanups_performed++;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// 2. Programmatic Multi-Step Tool Chaining Engine Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_tool_chain_init(tsfi_cordis_tool_chain_t *chain, bool atomic) {
    if (!chain) return false;
    memset(chain, 0, sizeof(*chain));
    chain->atomic_transaction = atomic;
    return true;
}

bool tsfi_cordis_chain_add_step(
    tsfi_cordis_tool_chain_t *chain,
    const char *plugin,
    const char *cmd,
    const char *args,
    bool stop_on_fail
) {
    if (!chain || !plugin || !cmd || chain->total_steps >= TSFI_CORDIS_MAX_CHAIN_STEPS) {
        return false;
    }
    uint32_t idx = chain->total_steps++;
    tsfi_cordis_chain_step_t *st = &chain->steps[idx];
    strncpy(st->plugin_name, plugin, sizeof(st->plugin_name) - 1);
    strncpy(st->command, cmd, sizeof(st->command) - 1);
    if (args) {
        strncpy(st->arguments, args, sizeof(st->arguments) - 1);
    }
    st->stop_on_failure = stop_on_fail;
    return true;
}

bool tsfi_cordis_execute_chain(
    tsfi_cordis_tool_chain_t *chain,
    tsfi_cordis_registry_t *reg,
    char *aggregate_out,
    size_t max_out_len
) {
    if (!chain || !reg || !aggregate_out || max_out_len == 0) return false;
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    aggregate_out[0] = '\0';
    size_t cur_len = 0;

    for (uint32_t i = 0; i < chain->total_steps; i++) {
        tsfi_cordis_chain_step_t *st = &chain->steps[i];
        char step_buf[1024] = {0};

        bool ok = tsfi_cordis_dispatch_tool(
            reg,
            st->plugin_name,
            st->command,
            st->arguments,
            step_buf,
            sizeof(step_buf)
        );

        chain->executed_steps++;

        int written = snprintf(aggregate_out + cur_len, max_out_len - cur_len,
                               "[Step %u: %s::%s] => %s\n",
                               i + 1, st->plugin_name, st->command, step_buf);
        if (written > 0) {
            cur_len += (size_t)written;
        }

        if (!ok && st->stop_on_failure) {
            if (chain->atomic_transaction) {
                snprintf(aggregate_out + cur_len, max_out_len - cur_len,
                         "[CHAIN ABORTED] Atomic execution failed at step %u.\n", i + 1);
            }
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts1);
    chain->chain_execution_time_us = (float)((ts1.tv_sec - ts0.tv_sec) * 1e6 + (ts1.tv_nsec - ts0.tv_nsec) * 1e-3);
    return (chain->executed_steps == chain->total_steps);
}
