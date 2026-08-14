#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_harness.h"
#include "tsfi_deepseek_cordis_chain.h"

static bool dummy_tool(void *ctx, const char *cmd, const char *args, char *out_buf, size_t out_len) {
    (void)ctx;
    snprintf(out_buf, out_len, "OK: cmd=%s args=%s", cmd, args);
    return true;
}

int main(void) {
    printf("=== TESTING CORDIS TEMPORAL SCOPER & TOOL CHAINING ===\n");

    // 1. Test Temporal Composability & Side-Effect Scoping
    tsfi_cordis_temporal_scope_t scope;
    assert(tsfi_cordis_temporal_scope_init(&scope));

    void *temp_page = malloc(4096);
    assert(temp_page != NULL);
    assert(tsfi_cordis_track_side_effect(&scope, EFFECT_HEAP_PAGE, temp_page, -1, 42));
    assert(scope.active_effects_count == 1);

    // Rollback turn 42 side effects
    assert(tsfi_cordis_rollback_turn_effects(&scope, 42));
    assert(scope.effects[0].resource_ptr == NULL);
    assert(scope.total_cleanups_performed == 1);
    printf("[1/2] Temporal side-effect rollback verified (zero heap leaks)\n");

    // 2. Test Multi-Step Tool Chaining ("Code Mode")
    tsfi_cordis_registry_t reg;
    assert(tsfi_cordis_registry_init(&reg));

    tsfi_cordis_plugin_t p_scsi;
    memset(&p_scsi, 0, sizeof(p_scsi));
    strncpy(p_scsi.plugin_name, "scsi_bus", sizeof(p_scsi.plugin_name) - 1);
    p_scsi.execute_tool = dummy_tool;
    assert(tsfi_cordis_register_plugin(&reg, &p_scsi));

    tsfi_cordis_tool_chain_t chain;
    assert(tsfi_cordis_tool_chain_init(&chain, true));
    assert(tsfi_cordis_chain_add_step(&chain, "scsi_bus", "lock", "device=0x01", true));
    assert(tsfi_cordis_chain_add_step(&chain, "scsi_bus", "read_block", "lba=0x500", true));
    assert(tsfi_cordis_chain_add_step(&chain, "scsi_bus", "unlock", "device=0x01", true));

    char exec_summary[4096] = {0};
    assert(tsfi_cordis_execute_chain(&chain, &reg, exec_summary, sizeof(exec_summary)));
    assert(chain.executed_steps == 3);
    assert(strstr(exec_summary, "Step 1: scsi_bus::lock") != NULL);
    assert(strstr(exec_summary, "Step 2: scsi_bus::read_block") != NULL);
    assert(strstr(exec_summary, "Step 3: scsi_bus::unlock") != NULL);
    printf("[2/2] 3-step atomic tool chain executed in %.2f us\n", chain.chain_execution_time_us);

    printf("=== ALL CORDIS TEMPORAL & CHAINING TESTS PASSED ===\n");
    return 0;
}
