#ifndef TSFI_DEEPSEEK_AGENT_VM_OPERATOR_H
#define TSFI_DEEPSEEK_AGENT_VM_OPERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_deepseek_cordis_harness.h"
#include "tsfi_deepseek_edsac_acp_firewall.h"

#define TSFI_MAX_DISCOVERED_SKILLS   64
#define TSFI_MAX_OPERATOR_NAME_LEN   64

// ============================================================================
// 1. Dynamic Skill Discovery Registry
// ============================================================================

typedef struct {
    char skill_name[TSFI_MAX_OPERATOR_NAME_LEN];
    char skill_description[256];
    char required_capability[64];
    uint32_t skill_id;
    bool is_hardware_bridged;
    bool (*skill_entry_point)(void *operator_ctx, const char *args, char *res_out, size_t max_out);
} tsfi_discovered_skill_t;

typedef struct {
    uint32_t total_discovered_skills;
    tsfi_discovered_skill_t skills[TSFI_MAX_DISCOVERED_SKILLS];
} tsfi_skill_discovery_catalog_t;

bool tsfi_skill_discovery_init(tsfi_skill_discovery_catalog_t *catalog);
bool tsfi_skill_discovery_register(
    tsfi_skill_discovery_catalog_t *catalog,
    const char *name,
    const char *desc,
    const char *cap,
    bool is_hw,
    bool (*entry)(void*, const char*, char*, size_t)
);

// ============================================================================
// 2. Personality Modulator Vector & Archetype Embedding
// ============================================================================

typedef struct {
    float keating_dominance;          // Sclera / eyebrow dominance modulator
    float scarpi_operational_trust;   // Utilitarian quality & consistency trust
    float hyde_vocal_warmth;          // Conversational synchronization & tempo
    float castle_diplomatic_alignment;// Policy target engagement
    float kramer_ward_fwhr_threat;    // Perceived threat / assertiveness rating
    float decay_rate_variance;        // Capacitor discharge decay variance
} tsfi_personality_vector_t;

// ============================================================================
// 3. Hot-Plugin Agent VM Operator
// ============================================================================

typedef struct {
    uint32_t operator_id;
    char operator_name[TSFI_MAX_OPERATOR_NAME_LEN];
    tsfi_personality_vector_t personality;
    uint32_t mounted_skills_count;
    uint32_t mounted_skill_ids[16];
    uint32_t total_turns_dispatched;
    bool is_vm_active;
} tsfi_agent_vm_operator_t;

bool tsfi_agent_vm_operator_init(
    tsfi_agent_vm_operator_t *op,
    uint32_t id,
    const char *name,
    const tsfi_personality_vector_t *personality
);

bool tsfi_agent_vm_operator_mount_skill(
    tsfi_agent_vm_operator_t *op,
    tsfi_skill_discovery_catalog_t *catalog,
    const char *skill_name,
    tsfi_edsac_acp_firewall_state_t *firewall
);

bool tsfi_agent_vm_operator_execute(
    tsfi_agent_vm_operator_t *op,
    tsfi_skill_discovery_catalog_t *catalog,
    const char *skill_name,
    const char *args,
    char *out_buf,
    size_t max_out_len
);

#endif // TSFI_DEEPSEEK_AGENT_VM_OPERATOR_H
