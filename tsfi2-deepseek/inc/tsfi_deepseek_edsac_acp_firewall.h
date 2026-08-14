#ifndef TSFI_DEEPSEEK_EDSAC_ACP_FIREWALL_H
#define TSFI_DEEPSEEK_EDSAC_ACP_FIREWALL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_deepseek_cordis_harness.h"

// Prohibited EDSAC Opcode Bitmask (Default-Reject Posture)
#define EDSAC_ACP_PROHIBITED_OPCODE_MASK   0x00084210 // Masks unsafe direct memory jumps

// ============================================================================
// 1. EDSAC Initial Orders 1 & 2 Capability Policy Engine
// ============================================================================

typedef enum {
    EDSAC_PHASE_UNQUALIFIED = 0,
    EDSAC_PHASE_QUALIFIED,      // Passed Initial Orders 1 Opcode Check
    EDSAC_PHASE_RELOCATED,      // Passed Initial Orders 2 Boundary Alignment
    EDSAC_PHASE_REJECTED
} tsfi_edsac_qualification_phase_t;

typedef struct {
    uint32_t prohibited_opcode_mask;
    uint32_t total_instructions_audited;
    uint32_t rejected_impositions_count;
    uint32_t approved_invocations_count;
    float firewall_evaluation_time_ns;
    bool default_reject_active;
} tsfi_edsac_acp_firewall_state_t;

bool tsfi_edsac_acp_firewall_init(tsfi_edsac_acp_firewall_state_t *fw);

// Initial Orders 1: Early Opcode & Capability Qualification
bool tsfi_edsac_initial_orders_1_qualify(
    tsfi_edsac_acp_firewall_state_t *fw,
    const char *tool_name,
    const char *command_opcode_str,
    tsfi_edsac_qualification_phase_t *phase_out
);

// Initial Orders 2: Relocatable Boundary & Landlock Memory Offset Check
bool tsfi_edsac_initial_orders_2_relocate(
    tsfi_edsac_acp_firewall_state_t *fw,
    uint32_t target_memory_offset,
    uint32_t segment_size,
    uint32_t max_allowed_boundary
);

// Unified ACP Gating Hook for Cordis Tool Execution
bool tsfi_edsac_acp_gate_tool_dispatch(
    tsfi_edsac_acp_firewall_state_t *fw,
    tsfi_cordis_registry_t *reg,
    const char *plugin_name,
    const char *cmd,
    const char *args,
    char *out_buf,
    size_t out_len
);

#endif // TSFI_DEEPSEEK_EDSAC_ACP_FIREWALL_H
