#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_edsac_acp_firewall.h"

// ----------------------------------------------------------------------------
// 1. EDSAC Initial Orders 1 & 2 Capability Policy Engine Implementation
// ----------------------------------------------------------------------------

bool tsfi_edsac_acp_firewall_init(tsfi_edsac_acp_firewall_state_t *fw) {
    if (!fw) return false;
    memset(fw, 0, sizeof(*fw));
    fw->prohibited_opcode_mask = EDSAC_ACP_PROHIBITED_OPCODE_MASK;
    fw->default_reject_active = true;
    return true;
}

bool tsfi_edsac_initial_orders_1_qualify(
    tsfi_edsac_acp_firewall_state_t *fw,
    const char *tool_name,
    const char *command_opcode_str,
    tsfi_edsac_qualification_phase_t *phase_out
) {
    if (!fw || !tool_name || !command_opcode_str || !phase_out) {
        if (phase_out) *phase_out = EDSAC_PHASE_REJECTED;
        return false;
    }
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    fw->total_instructions_audited++;

    // Prohibit unauthorized destructive operations under default-reject
    if (strstr(command_opcode_str, "rm -rf") ||
        strstr(command_opcode_str, "chmod 777") ||
        strstr(command_opcode_str, "mkfs") ||
        strstr(command_opcode_str, "dd if=/dev/zero")) {
        fw->rejected_impositions_count++;
        *phase_out = EDSAC_PHASE_REJECTED;
        return false;
    }

    // Check opcode against prohibited bitmask
    char first_op = command_opcode_str[0];
    if (first_op >= 'A' && first_op <= 'Z') {
        uint32_t shift = (uint32_t)(first_op - 'A');
        if ((fw->prohibited_opcode_mask >> shift) & 1) {
            fw->rejected_impositions_count++;
            *phase_out = EDSAC_PHASE_REJECTED;
            return false;
        }
    }

    *phase_out = EDSAC_PHASE_QUALIFIED;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    fw->firewall_evaluation_time_ns = (float)((ts1.tv_sec - ts0.tv_sec) * 1e9 + (ts1.tv_nsec - ts0.tv_nsec));
    return true;
}

bool tsfi_edsac_initial_orders_2_relocate(
    tsfi_edsac_acp_firewall_state_t *fw,
    uint32_t target_memory_offset,
    uint32_t segment_size,
    uint32_t max_allowed_boundary
) {
    if (!fw) return false;
    
    // Bounds verification: Enforce spatial memory layout
    if (target_memory_offset + segment_size > max_allowed_boundary) {
        fw->rejected_impositions_count++;
        return false;
    }
    return true;
}

bool tsfi_edsac_acp_gate_tool_dispatch(
    tsfi_edsac_acp_firewall_state_t *fw,
    tsfi_cordis_registry_t *reg,
    const char *plugin_name,
    const char *cmd,
    const char *args,
    char *out_buf,
    size_t out_len
) {
    if (!fw || !reg || !plugin_name || !cmd || !out_buf || out_len == 0) return false;

    // Step 1: Initial Orders 1 Qualification
    tsfi_edsac_qualification_phase_t phase = EDSAC_PHASE_UNQUALIFIED;
    if (!tsfi_edsac_initial_orders_1_qualify(fw, plugin_name, cmd, &phase) || phase != EDSAC_PHASE_QUALIFIED) {
        snprintf(out_buf, out_len, "[EDSAC ACP FIREWALL REJECT] Command '%s' blocked by Initial Orders 1 policy.", cmd);
        return false;
    }

    // Step 2: Pass through to Cordis Registry Dispatcher
    fw->approved_invocations_count++;
    return tsfi_cordis_dispatch_tool(reg, plugin_name, cmd, args, out_buf, out_len);
}
