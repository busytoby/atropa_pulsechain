#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_harness.h"
#include "tsfi_deepseek_edsac_acp_firewall.h"

static bool mock_hw_tool(void *ctx, const char *cmd, const char *args, char *out_buf, size_t out_len) {
    (void)ctx;
    snprintf(out_buf, out_len, "HW_SUCCESS: cmd=%s args=%s", cmd, args);
    return true;
}

int main(void) {
    printf("=== TESTING EDSAC INITIAL ORDERS 1 & 2 ACP FIREWALL ===\n");

    // 1. Initialize Firewall
    tsfi_edsac_acp_firewall_state_t fw;
    assert(tsfi_edsac_acp_firewall_init(&fw));
    assert(fw.default_reject_active);

    // 2. Test Initial Orders 1: Default-Reject on Destructive Commands
    tsfi_edsac_qualification_phase_t phase;
    assert(!tsfi_edsac_initial_orders_1_qualify(&fw, "terminal", "rm -rf /", &phase));
    assert(phase == EDSAC_PHASE_REJECTED && fw.rejected_impositions_count == 1);
    printf("[1/3] Destructive command rejected by Initial Orders 1\n");

    // Valid command qualification
    assert(tsfi_edsac_initial_orders_1_qualify(&fw, "terminal", "make test", &phase));
    assert(phase == EDSAC_PHASE_QUALIFIED);
    printf("[1/3] Authorized command qualified (latency: %.2f ns)\n", fw.firewall_evaluation_time_ns);

    // 3. Test Initial Orders 2: Relocatable Memory Boundary Check
    assert(tsfi_edsac_initial_orders_2_relocate(&fw, 0x1000, 0x0200, 0x2000)); // Valid bounds
    assert(!tsfi_edsac_initial_orders_2_relocate(&fw, 0x1F00, 0x0200, 0x2000)); // Overflow rejected
    printf("[2/3] Initial Orders 2 relocatable memory boundary verified\n");

    // 4. Test Unified ACP Tool Dispatch Gating
    tsfi_cordis_registry_t reg;
    assert(tsfi_cordis_registry_init(&reg));

    tsfi_cordis_plugin_t plug;
    memset(&plug, 0, sizeof(plug));
    strncpy(plug.plugin_name, "hardware_scsi", sizeof(plug.plugin_name) - 1);
    plug.execute_tool = mock_hw_tool;
    assert(tsfi_cordis_register_plugin(&reg, &plug));

    char out_buf[512] = {0};
    bool ok_dispatch = tsfi_edsac_acp_gate_tool_dispatch(
        &fw,
        &reg,
        "hardware_scsi",
        "read_register",
        "reg=0x32",
        out_buf,
        sizeof(out_buf)
    );
    assert(ok_dispatch);
    assert(strstr(out_buf, "HW_SUCCESS") != NULL);
    assert(fw.approved_invocations_count == 1);
    printf("[3/3] ACP Gated tool execution approved: %s\n", out_buf);

    printf("=== ALL EDSAC ACP FIREWALL TESTS PASSED ===\n");
    return 0;
}
