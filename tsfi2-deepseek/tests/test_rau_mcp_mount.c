#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

// Simulated RAU State aligned to 32-byte slots
typedef struct {
    uint32_t active_mappings[32]; // Maps Virtual (0-31) to Physical (0-3) or 0xff (spilled)
    uint64_t physical_regs[4];    // R0-R3
    uint64_t spill_cache[32];     // V0-V31
} SimulatedRAU;

// Simulated WMQ Bridge State
typedef struct {
    uint32_t wmq_target_core;
    uint32_t wmq_target_reg;
    uint64_t wmq_target_val;
} SimulatedWMQ;

// Simulates executing the Yul RAU logic upon receiving a WMQ SCSI packet
void simulate_rau_wmq_process(SimulatedRAU *rau, SimulatedWMQ *wmq) {
    if (!rau || !wmq) return;

    uint32_t v_reg = wmq->wmq_target_reg;
    uint64_t val = wmq->wmq_target_val;

    // Bounds checking
    if (v_reg >= 32) {
        printf("[DEBUG ERROR] v_reg out of bounds: %u\n", v_reg);
        return;
    }

    // Hardwired V0 check
    if (v_reg == 0) return;

    // Determine physical slot allocation or spill
    uint32_t p_reg = rau->active_mappings[v_reg];
    if (p_reg == 0xff) {
        // Simple allocation: allocate R(v_reg % 4)
        p_reg = v_reg % 4;
        rau->active_mappings[v_reg] = p_reg;
    }

    rau->physical_regs[p_reg] = val;
}

// Simulated MCP JSON-RPC handler that mounts the RAU over WMQ
int handle_mcp_write_call(SimulatedRAU *rau, const char *json_request) {
    if (!rau || !json_request) return -1;

    // Simple parser for test simulation purposes
    if (strstr(json_request, "\"method\":\"write_rau_register\"") == NULL) {
        return -2;
    }

    // Extract parameters using strstr and strchr/strtol to avoid sscanf format mismatches
    uint32_t core_id = 0;
    uint32_t v_reg = 999; // Default to out-of-bounds to test parser safety
    uint64_t value = 0;

    const char *p_core = strstr(json_request, "\"core_id\":");
    const char *p_vreg = strstr(json_request, "\"v_reg\":");
    const char *p_val = strstr(json_request, "\"value\":");

    if (p_core) {
        core_id = (uint32_t)strtoul(p_core + 10, NULL, 10);
    }
    if (p_vreg) {
        v_reg = (uint32_t)strtoul(p_vreg + 8, NULL, 10);
    }
    if (p_val) {
        value = (uint64_t)strtoull(p_val + 8, NULL, 10);
    }

    printf("[DEBUG] Parsed core_id=%u, v_reg=%u, value=%lu\n", core_id, v_reg, value);

    if (v_reg >= 32) {
        return -3;
    }

    // Format the WMQ SCSI packet from the MCP parameters
    SimulatedWMQ wmq;
    wmq.wmq_target_core = core_id;
    wmq.wmq_target_reg = v_reg;
    wmq.wmq_target_val = value;

    // Route and process inside RAU
    simulate_rau_wmq_process(rau, &wmq);
    return 0;
}

int main(void) {
    printf("=============================================================\n");
    printf("MCP TO Yul RAU CO-PROCESSOR MOUNTING INTEGRATION TEST\n");
    printf("=============================================================\n");

    SimulatedRAU rau;
    // Initialize mappings to 0xff (all registers unmapped/spilled initially)
    memset(rau.active_mappings, 0xff, sizeof(rau.active_mappings));
    memset(rau.physical_regs, 0, sizeof(rau.physical_regs));
    memset(rau.spill_cache, 0, sizeof(rau.spill_cache));

    // 1. Simulate receiving an MCP JSON-RPC write request to set V5 to 9999
    const char *mcp_json = "{\"method\":\"write_rau_register\",\"params\":{\"core_id\":1,\"v_reg\":5,\"value\":9999}}";
    printf("[TEST] Emulating incoming MCP JSON-RPC call:\n  %s\n", mcp_json);

    int res = handle_mcp_write_call(&rau, mcp_json);
    assert(res == 0);

    // 2. Verify state propagation to the physical register space of the target ALU
    uint32_t p_reg = rau.active_mappings[5];
    assert(p_reg != 0xff); // Should be successfully mapped
    assert(rau.physical_regs[p_reg] == 9999); // Value correctly updated

    printf("\n[TEST] Verifying MCP-to-WMQ-to-RAU mounting invariants...\n");
    printf("   ✓ V5 Mapping Resolved to Physical Reg: R%u\n", p_reg);
    printf("   ✓ Physical Register Value Updated to : %lu (Expected: 9999)\n", rau.physical_regs[p_reg]);

    printf("=============================================================\n");
    printf("MCP TO RAU MOUNTING TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
