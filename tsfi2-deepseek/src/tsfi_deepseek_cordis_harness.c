#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_cordis_harness.h"

// ----------------------------------------------------------------------------
// 1. Cordis Plugin Registry Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_registry_init(tsfi_cordis_registry_t *reg) {
    if (!reg) return false;
    memset(reg, 0, sizeof(*reg));
    return true;
}

bool tsfi_cordis_register_plugin(tsfi_cordis_registry_t *reg, const tsfi_cordis_plugin_t *plugin) {
    if (!reg || !plugin || reg->registered_count >= TSFI_CORDIS_MAX_PLUGINS) {
        return false;
    }
    for (uint32_t i = 0; i < reg->registered_count; i++) {
        if (strncmp(reg->plugins[i].plugin_name, plugin->plugin_name, sizeof(plugin->plugin_name)) == 0) {
            reg->plugins[i] = *plugin;
            return true;
        }
    }
    reg->plugins[reg->registered_count++] = *plugin;
    if (plugin->init) {
        return plugin->init(plugin->user_data);
    }
    return true;
}

bool tsfi_cordis_dispatch_tool(
    tsfi_cordis_registry_t *reg,
    const char *plugin_name,
    const char *cmd,
    const char *args,
    char *out_buf,
    size_t out_len
) {
    if (!reg || !plugin_name || !cmd || !out_buf || out_len == 0) return false;
    for (uint32_t i = 0; i < reg->registered_count; i++) {
        if (strcmp(reg->plugins[i].plugin_name, plugin_name) == 0) {
            if (reg->plugins[i].execute_tool) {
                return reg->plugins[i].execute_tool(reg->plugins[i].user_data, cmd, args, out_buf, out_len);
            }
            break;
        }
    }
    snprintf(out_buf, out_len, "[CORDIS ERROR] Plugin '%s' not registered or missing execute_tool.", plugin_name);
    return false;
}

// ----------------------------------------------------------------------------
// 2. Hierarchical Sub-Agent Task Partitioning Implementation
// ----------------------------------------------------------------------------

bool tsfi_cordis_orchestrator_init(tsfi_cordis_orchestrator_state_t *orch) {
    if (!orch) return false;
    memset(orch, 0, sizeof(*orch));
    return true;
}

bool tsfi_cordis_spawn_subagent(
    tsfi_cordis_orchestrator_state_t *orch,
    const char *role,
    uint32_t parent_session_id,
    uint32_t context_limit,
    uint32_t *subagent_id_out
) {
    if (!orch || !role || orch->active_agents_count >= TSFI_CORDIS_MAX_SUBAGENTS) {
        return false;
    }
    uint32_t idx = orch->active_agents_count++;
    tsfi_cordis_subagent_t *sub = &orch->subagents[idx];
    sub->agent_id = idx + 1;
    strncpy(sub->role, role, sizeof(sub->role) - 1);
    sub->parent_session_id = parent_session_id;
    sub->context_window_limit = (context_limit > 0) ? context_limit : 2048;
    sub->tokens_consumed = 0;
    sub->state = CORDIS_AGENT_IDLE;
    memset(sub->last_action, 0, sizeof(sub->last_action));

    if (subagent_id_out) {
        *subagent_id_out = sub->agent_id;
    }
    return true;
}

bool tsfi_cordis_route_subagent_message(
    tsfi_cordis_orchestrator_state_t *orch,
    uint32_t src_subagent_id,
    uint32_t dst_subagent_id,
    const char *message_payload
) {
    if (!orch || !message_payload) return false;
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    bool dst_found = false;
    for (uint32_t i = 0; i < orch->active_agents_count; i++) {
        if (orch->subagents[i].agent_id == dst_subagent_id) {
            dst_found = true;
            orch->subagents[i].state = CORDIS_AGENT_RUNNING;
            snprintf(orch->subagents[i].last_action, sizeof(orch->subagents[i].last_action),
                     "Msg from subagent %u: %.128s", src_subagent_id, message_payload);
            break;
        }
    }
    orch->cross_agent_messages_routed++;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    orch->orchestration_latency_us = (float)((ts1.tv_sec - ts0.tv_sec) * 1e6 + (ts1.tv_nsec - ts0.tv_nsec) * 1e-3);
    return dst_found;
}

// ----------------------------------------------------------------------------
// 3. Incremental Streaming & Live AST Sub-Turn Validator
// ----------------------------------------------------------------------------

bool tsfi_cordis_streaming_ast_init(tsfi_cordis_streaming_ast_state_t *ast_state) {
    if (!ast_state) return false;
    memset(ast_state, 0, sizeof(*ast_state));
    ast_state->syntax_valid = true;
    return true;
}

bool tsfi_cordis_streaming_ast_feed_token(
    tsfi_cordis_streaming_ast_state_t *ast_state,
    tsfi_cordis_registry_t *reg,
    const char *token_str,
    char *tool_exec_out,
    size_t max_tool_exec_len
) {
    if (!ast_state || !token_str) return false;
    struct timespec t_begin, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_begin);

    size_t tlen = strlen(token_str);
    if (ast_state->stream_buf_pos + tlen < sizeof(ast_state->token_stream_buffer) - 1) {
        memcpy(ast_state->token_stream_buffer + ast_state->stream_buf_pos, token_str, tlen);
        ast_state->stream_buf_pos += tlen;
        ast_state->token_stream_buffer[ast_state->stream_buf_pos] = '\0';
    }

    // Live AST check: Detect brace closures and syntax boundary keywords
    for (size_t k = 0; k < tlen; k++) {
        char ch = token_str[k];
        if (ch == '{' || ch == '}' || ch == ';' || ch == '(' || ch == ')') {
            ast_state->live_ast_node_count++;
        }
    }

    // Check for inline tool invocation tags: `<cordis_tool name="XYZ">ARGS</cordis_tool>`
    char *start_tag = strstr(ast_state->token_stream_buffer, "<cordis_tool name=\"");
    if (start_tag && !ast_state->in_tool_call_block) {
        char *name_start = start_tag + 19;
        char *name_end = strchr(name_start, '\"');
        if (name_end && (size_t)(name_end - name_start) < sizeof(ast_state->pending_tool_name)) {
            size_t nlen = (size_t)(name_end - name_start);
            memcpy(ast_state->pending_tool_name, name_start, nlen);
            ast_state->pending_tool_name[nlen] = '\0';
            ast_state->in_tool_call_block = true;
        }
    }

    if (ast_state->in_tool_call_block) {
        char *end_tag = strstr(ast_state->token_stream_buffer, "</cordis_tool>");
        if (end_tag) {
            char *payload_start = strchr(start_tag, '>') + 1;
            size_t payload_len = (size_t)(end_tag - payload_start);
            if (payload_len < sizeof(ast_state->pending_tool_payload)) {
                memcpy(ast_state->pending_tool_payload, payload_start, payload_len);
                ast_state->pending_tool_payload[payload_len] = '\0';
            }
            if (reg && tool_exec_out && max_tool_exec_len > 0) {
                tsfi_cordis_dispatch_tool(
                    reg,
                    ast_state->pending_tool_name,
                    "exec",
                    ast_state->pending_tool_payload,
                    tool_exec_out,
                    max_tool_exec_len
                );
            }
            ast_state->in_tool_call_block = false;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    ast_state->streaming_validation_latency_us = (float)((t_end.tv_sec - t_begin.tv_sec) * 1e6 + (t_end.tv_nsec - t_begin.tv_nsec) * 1e-3);
    return true;
}

// ----------------------------------------------------------------------------
// 4. Binary WAL Receipt Persistence (.dat.bin under Rule 13)
// ----------------------------------------------------------------------------

bool tsfi_cordis_write_wal_receipt(
    const char *dat_bin_path,
    const tsfi_cordis_wal_receipt_t *receipt
) {
    if (!dat_bin_path || !receipt) return false;
    FILE *f = fopen(dat_bin_path, "ab");
    if (!f) return false;
    size_t written = fwrite(receipt, sizeof(*receipt), 1, f);
    fclose(f);
    return (written == 1);
}
