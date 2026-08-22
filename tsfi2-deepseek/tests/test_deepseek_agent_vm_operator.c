#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_agent_vm_operator.h"

static bool sample_scsi_skill(void *op_ctx, const char *args, char *res_out, size_t max_out) {
    tsfi_agent_vm_operator_t *op = (tsfi_agent_vm_operator_t *)op_ctx;
    snprintf(res_out, max_out, "OPERATOR [%s] (Trust=%.2f, Warmth=%.2f) executed SCSI skill with args: %s",
             op->operator_name, op->personality.scarpi_operational_trust, op->personality.hyde_vocal_warmth, args);
    return true;
}

int main(void) {
    printf("=== TESTING AGENT VM OPERATOR & SKILL DISCOVERY ===\n");

    // 1. Initialize Skill Discovery Catalog
    tsfi_skill_discovery_catalog_t catalog;
    assert(tsfi_skill_discovery_init(&catalog));

    assert(tsfi_skill_discovery_register(
        &catalog,
        "scsi_zmm_handshake",
        "Low-level virtual hardware register state machine verification",
        "CAP_HARDWARE_SCSI",
        true,
        sample_scsi_skill
    ));
    assert(catalog.total_discovered_skills == 1);
    printf("[1/3] Skill registered and cataloged in discovery registry\n");

    // 2. Initialize Agent VM Operator with Personality Archetype
    tsfi_agent_vm_operator_t teddy_bear_op;
    tsfi_personality_vector_t p_vec = {
        .keating_dominance = 0.35f,
        .scarpi_operational_trust = 0.99f,
        .hyde_vocal_warmth = 0.92f,
        .castle_diplomatic_alignment = 0.88f,
        .kramer_ward_fwhr_threat = 0.05f,
        .decay_rate_variance = 0.02f
    };
    assert(tsfi_agent_vm_operator_init(&teddy_bear_op, 101, "TeddyBear_Operator", &p_vec));
    printf("[2/3] Agent VM Operator initialized with personality vector\n");

    // 3. Mount and Execute Skill with EDSAC ACP Firewall Gating
    tsfi_edsac_acp_firewall_state_t fw;
    assert(tsfi_edsac_acp_firewall_init(&fw));

    assert(tsfi_agent_vm_operator_mount_skill(&teddy_bear_op, &catalog, "scsi_zmm_handshake", &fw));
    assert(teddy_bear_op.mounted_skills_count == 1);

    char exec_buf[512] = {0};
    assert(tsfi_agent_vm_operator_execute(&teddy_bear_op, &catalog, "scsi_zmm_handshake", "reg=0x32 keycode=d", exec_buf, sizeof(exec_buf)));
    assert(strstr(exec_buf, "TeddyBear_Operator") != NULL);
    assert(strstr(exec_buf, "Trust=0.99") != NULL);
    printf("[3/3] Agent VM Operator executed mounted skill: %s\n", exec_buf);

    printf("=== ALL AGENT VM OPERATOR TESTS PASSED ===\n");
    return 0;
}
