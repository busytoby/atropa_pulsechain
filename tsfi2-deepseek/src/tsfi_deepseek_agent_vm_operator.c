#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_deepseek_agent_vm_operator.h"

// ----------------------------------------------------------------------------
// 1. Dynamic Skill Discovery Catalog Implementation
// ----------------------------------------------------------------------------

bool tsfi_skill_discovery_init(tsfi_skill_discovery_catalog_t *catalog) {
    if (!catalog) return false;
    memset(catalog, 0, sizeof(*catalog));
    return true;
}

bool tsfi_skill_discovery_register(
    tsfi_skill_discovery_catalog_t *catalog,
    const char *name,
    const char *desc,
    const char *cap,
    bool is_hw,
    bool (*entry)(void*, const char*, char*, size_t)
) {
    if (!catalog || !name || !entry || catalog->total_discovered_skills >= TSFI_MAX_DISCOVERED_SKILLS) {
        return false;
    }
    uint32_t idx = catalog->total_discovered_skills++;
    tsfi_discovered_skill_t *sk = &catalog->skills[idx];
    sk->skill_id = idx + 1;
    strncpy(sk->skill_name, name, sizeof(sk->skill_name) - 1);
    if (desc) strncpy(sk->skill_description, desc, sizeof(sk->skill_description) - 1);
    if (cap) strncpy(sk->required_capability, cap, sizeof(sk->required_capability) - 1);
    sk->is_hardware_bridged = is_hw;
    sk->skill_entry_point = entry;
    return true;
}

// ----------------------------------------------------------------------------
// 2. Hot-Plugin Agent VM Operator Implementation
// ----------------------------------------------------------------------------

bool tsfi_agent_vm_operator_init(
    tsfi_agent_vm_operator_t *op,
    uint32_t id,
    const char *name,
    const tsfi_personality_vector_t *personality
) {
    if (!op || !name) return false;
    memset(op, 0, sizeof(*op));
    op->operator_id = id;
    strncpy(op->operator_name, name, sizeof(op->operator_name) - 1);
    if (personality) {
        op->personality = *personality;
    } else {
        // Default balanced neutral archetype
        op->personality.keating_dominance = 0.5f;
        op->personality.scarpi_operational_trust = 0.95f;
        op->personality.hyde_vocal_warmth = 0.8f;
        op->personality.castle_diplomatic_alignment = 0.7f;
        op->personality.kramer_ward_fwhr_threat = 0.1f;
        op->personality.decay_rate_variance = 0.05f;
    }
    op->is_vm_active = true;
    return true;
}

bool tsfi_agent_vm_operator_mount_skill(
    tsfi_agent_vm_operator_t *op,
    tsfi_skill_discovery_catalog_t *catalog,
    const char *skill_name,
    tsfi_edsac_acp_firewall_state_t *firewall
) {
    if (!op || !catalog || !skill_name || op->mounted_skills_count >= 16) {
        return false;
    }

    // Step 1: Firewall gate check before mounting capability
    if (firewall) {
        tsfi_edsac_qualification_phase_t phase;
        if (!tsfi_edsac_initial_orders_1_qualify(firewall, op->operator_name, skill_name, &phase) ||
            phase != EDSAC_PHASE_QUALIFIED) {
            return false; // Blocked by EDSAC Initial Orders 1
        }
    }

    // Step 2: Search catalog
    for (uint32_t i = 0; i < catalog->total_discovered_skills; i++) {
        if (strcmp(catalog->skills[i].skill_name, skill_name) == 0) {
            op->mounted_skill_ids[op->mounted_skills_count++] = catalog->skills[i].skill_id;
            return true;
        }
    }
    return false;
}

bool tsfi_agent_vm_operator_execute(
    tsfi_agent_vm_operator_t *op,
    tsfi_skill_discovery_catalog_t *catalog,
    const char *skill_name,
    const char *args,
    char *out_buf,
    size_t max_out_len
) {
    if (!op || !catalog || !skill_name || !out_buf || max_out_len == 0) return false;

    // Find registered skill
    for (uint32_t i = 0; i < catalog->total_discovered_skills; i++) {
        tsfi_discovered_skill_t *sk = &catalog->skills[i];
        if (strcmp(sk->skill_name, skill_name) == 0) {
            // Verify skill is mounted on this operator
            bool is_mounted = false;
            for (uint32_t m = 0; m < op->mounted_skills_count; m++) {
                if (op->mounted_skill_ids[m] == sk->skill_id) {
                    is_mounted = true;
                    break;
                }
            }
            if (!is_mounted) {
                snprintf(out_buf, max_out_len, "[OPERATOR ERROR] Skill '%s' not mounted on Operator '%s'.",
                         skill_name, op->operator_name);
                return false;
            }

            op->total_turns_dispatched++;
            return sk->skill_entry_point((void*)op, args, out_buf, max_out_len);
        }
    }

    snprintf(out_buf, max_out_len, "[OPERATOR ERROR] Skill '%s' not found in discovery catalog.", skill_name);
    return false;
}
