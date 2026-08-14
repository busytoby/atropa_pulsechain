#ifndef TSFI_DEEPSEEK_CORDIS_HARNESS_H
#define TSFI_DEEPSEEK_CORDIS_HARNESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_CORDIS_MAX_PLUGINS        32
#define TSFI_CORDIS_MAX_SUBAGENTS      16
#define TSFI_CORDIS_MAX_BUFFER_SIZE    8192
#define TSFI_CORDIS_RECEIPT_MAGIC      0x43524453 // 'C''R''D''S'

// ============================================================================
// 1. Cordis Modular Plugin Interface (DeepSeek Harness v0.1 Specification)
// ============================================================================

typedef struct tsfi_cordis_plugin {
    char plugin_name[64];
    uint32_t plugin_version;
    uint32_t flags;
    void *user_data;
    bool (*init)(void *ctx);
    bool (*execute_tool)(void *ctx, const char *cmd, const char *args, char *out_buf, size_t out_len);
    bool (*flush_state)(void *ctx);
    void (*destroy)(void *ctx);
} tsfi_cordis_plugin_t;

typedef struct {
    uint32_t registered_count;
    tsfi_cordis_plugin_t plugins[TSFI_CORDIS_MAX_PLUGINS];
} tsfi_cordis_registry_t;

bool tsfi_cordis_registry_init(tsfi_cordis_registry_t *reg);
bool tsfi_cordis_register_plugin(tsfi_cordis_registry_t *reg, const tsfi_cordis_plugin_t *plugin);
bool tsfi_cordis_dispatch_tool(
    tsfi_cordis_registry_t *reg,
    const char *plugin_name,
    const char *cmd,
    const char *args,
    char *out_buf,
    size_t out_len
);

// ============================================================================
// 2. Hierarchical Sub-Agent Task Partitioning & Orchestration
// ============================================================================

typedef enum {
    CORDIS_AGENT_IDLE = 0,
    CORDIS_AGENT_RUNNING,
    CORDIS_AGENT_BLOCKED_TOOL,
    CORDIS_AGENT_COMPLETED,
    CORDIS_AGENT_ERROR
} tsfi_cordis_agent_state_t;

typedef struct {
    uint32_t agent_id;
    char role[64];
    uint32_t parent_session_id;
    uint32_t context_window_limit;
    uint32_t tokens_consumed;
    tsfi_cordis_agent_state_t state;
    char last_action[256];
} tsfi_cordis_subagent_t;

typedef struct {
    uint32_t active_agents_count;
    tsfi_cordis_subagent_t subagents[TSFI_CORDIS_MAX_SUBAGENTS];
    uint32_t cross_agent_messages_routed;
    float orchestration_latency_us;
} tsfi_cordis_orchestrator_state_t;

bool tsfi_cordis_orchestrator_init(tsfi_cordis_orchestrator_state_t *orch);
bool tsfi_cordis_spawn_subagent(
    tsfi_cordis_orchestrator_state_t *orch,
    const char *role,
    uint32_t parent_session_id,
    uint32_t context_limit,
    uint32_t *subagent_id_out
);
bool tsfi_cordis_route_subagent_message(
    tsfi_cordis_orchestrator_state_t *orch,
    uint32_t src_subagent_id,
    uint32_t dst_subagent_id,
    const char *message_payload
);

// ============================================================================
// 3. Incremental Streaming & Live AST Sub-Turn Validator
// ============================================================================

typedef struct {
    char token_stream_buffer[TSFI_CORDIS_MAX_BUFFER_SIZE];
    size_t stream_buf_pos;
    uint32_t live_ast_node_count;
    bool in_tool_call_block;
    char pending_tool_name[64];
    char pending_tool_payload[1024];
    bool syntax_valid;
    float streaming_validation_latency_us;
} tsfi_cordis_streaming_ast_state_t;

bool tsfi_cordis_streaming_ast_init(tsfi_cordis_streaming_ast_state_t *ast_state);
bool tsfi_cordis_streaming_ast_feed_token(
    tsfi_cordis_streaming_ast_state_t *ast_state,
    tsfi_cordis_registry_t *reg,
    const char *token_str,
    char *tool_exec_out,
    size_t max_tool_exec_len
);

// ============================================================================
// 4. Rule 13 Binary Receipt Logging (.dat.bin)
// ============================================================================

typedef struct {
    uint32_t magic;
    uint32_t session_id;
    uint32_t turn_id;
    uint32_t plugin_count;
    uint32_t subagent_count;
    uint32_t live_ast_nodes;
    float turn_latency_ms;
    float knapsack_overhead_us;
    char primary_model[64];
} tsfi_cordis_wal_receipt_t;

bool tsfi_cordis_write_wal_receipt(
    const char *dat_bin_path,
    const tsfi_cordis_wal_receipt_t *receipt
);

#endif // TSFI_DEEPSEEK_CORDIS_HARNESS_H
