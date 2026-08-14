#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_harness.h"

static bool dummy_scsi_tool(void *ctx, const char *cmd, const char *args, char *out_buf, size_t out_len) {
    (void)ctx;
    snprintf(out_buf, out_len, "[SCSI BRIDGE ACK] cmd=%s args=%s status=0x00", cmd, args);
    return true;
}

int main(void) {
    printf("=== TESTING DEEPSEEK CORDIS HARNESS & SUBAGENT C PIPELINE ===\n");

    // 1. Test Plugin Registry
    tsfi_cordis_registry_t reg;
    assert(tsfi_cordis_registry_init(&reg));

    tsfi_cordis_plugin_t p_scsi;
    memset(&p_scsi, 0, sizeof(p_scsi));
    strncpy(p_scsi.plugin_name, "scsi_zmm_bridge", sizeof(p_scsi.plugin_name) - 1);
    p_scsi.plugin_version = 1;
    p_scsi.execute_tool = dummy_scsi_tool;
    assert(tsfi_cordis_register_plugin(&reg, &p_scsi));

    char tool_out[512] = {0};
    bool ok_dispatch = tsfi_cordis_dispatch_tool(&reg, "scsi_zmm_bridge", "read_sector", "lba=0x1000", tool_out, sizeof(tool_out));
    assert(ok_dispatch);
    printf("[1/4] Plugin dispatch verified: %s\n", tool_out);

    // 2. Test Hierarchical Sub-Agent Task Partitioning
    tsfi_cordis_orchestrator_state_t orch;
    assert(tsfi_cordis_orchestrator_init(&orch));
    uint32_t sub1 = 0, sub2 = 0;
    assert(tsfi_cordis_spawn_subagent(&orch, "ast_synthesizer", 1, 4096, &sub1));
    assert(tsfi_cordis_spawn_subagent(&orch, "scsi_hardware_verifier", 1, 2048, &sub2));
    assert(sub1 == 1 && sub2 == 2);

    bool ok_route = tsfi_cordis_route_subagent_message(&orch, sub1, sub2, "Verify SCSI register 0x32 for 'd'");
    assert(ok_route);
    printf("[2/4] Subagent message routed successfully (latency: %.2f us)\n", orch.orchestration_latency_us);

    // 3. Test Streaming AST Sub-Turn Validator
    tsfi_cordis_streaming_ast_state_t ast;
    assert(tsfi_cordis_streaming_ast_init(&ast));

    char exec_buf[512] = {0};
    assert(tsfi_cordis_streaming_ast_feed_token(&ast, &reg, "int main() { ", exec_buf, sizeof(exec_buf)));
    assert(tsfi_cordis_streaming_ast_feed_token(&ast, &reg, "<cordis_tool name=\"scsi_zmm_bridge\">probe_registers</cordis_tool>", exec_buf, sizeof(exec_buf)));
    assert(ast.live_ast_node_count >= 3);
    printf("[3/4] Streaming AST tokens parsed (%u syntax nodes, tool exec: '%s')\n", ast.live_ast_node_count, exec_buf);

    // 4. Test Binary WAL Receipt (.dat.bin under Rule 13)
    tsfi_cordis_wal_receipt_t rcpt;
    memset(&rcpt, 0, sizeof(rcpt));
    rcpt.magic = TSFI_CORDIS_RECEIPT_MAGIC;
    rcpt.session_id = 101;
    rcpt.turn_id = 1;
    rcpt.plugin_count = reg.registered_count;
    rcpt.subagent_count = orch.active_agents_count;
    rcpt.live_ast_nodes = ast.live_ast_node_count;
    rcpt.turn_latency_ms = 0.42f;
    strncpy(rcpt.primary_model, "DeepSeek-Coder-6.7B", sizeof(rcpt.primary_model) - 1);

    const char *test_wal = "/tmp/test_cordis_receipt.dat.bin";
    assert(tsfi_cordis_write_wal_receipt(test_wal, &rcpt));
    printf("[4/4] Binary WAL receipt successfully written to %s\n", test_wal);

    printf("=== ALL DEEPSEEK CORDIS HARNESS TESTS PASSED ===\n");
    return 0;
}
