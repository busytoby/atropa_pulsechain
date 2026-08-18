#ifndef AUNCIENT_CPM_POLYPHASE_COMPILER_EXECUTOR_H
#define AUNCIENT_CPM_POLYPHASE_COMPILER_EXECUTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CPM_TPA_BASE_ADDR 0x0100
#define MAX_BYTECODE_INSTRUCTIONS 64
#define MAX_POLYPHASE_TENANTS 4

typedef enum {
    POLY_PHASE_A_LEXICAL = 0,    /* phi = 0, Base register */
    POLY_PHASE_B_RELOCATION = 1, /* phi = 2pi/3, Foundation register */
    POLY_PHASE_C_EXECUTION = 2   /* phi = 4pi/3, Element & Dynamo registers */
} CpmPolyPhase;

typedef struct {
    uint8_t  opcode;
    uint16_t operand;
    uint32_t execution_cycles;
} CpmBytecodeInstruction;

typedef struct {
    uint32_t tenant_id;
    uint32_t dna_seed;
    uint64_t saat_balance;
    float    phase_offset_rad;
    bool     is_active;
} CpmTenantExecutor;

typedef struct {
    uint8_t  tpa_memory[65536];
    uint32_t total_instructions;
    CpmBytecodeInstruction program[MAX_BYTECODE_INSTRUCTIONS];
    uint32_t pc;
    uint32_t accumulator;
    uint64_t base_reg;
    uint64_t foundation_reg;
    uint64_t element_reg;
    uint64_t dynamo_velocity;
    float    displacement_scale;
    CpmTenantExecutor tenants[MAX_POLYPHASE_TENANTS];
    uint32_t active_tenants_count;
    CpmPolyPhase current_phase;
    bool     phase_a_verified;
    bool     phase_b_verified;
    bool     phase_c_verified;
    bool     displacementshader_synced;
    bool     executor_parity_verified;
    uint32_t rule18_parity_checksum;
} CpmPolyphaseCompilerExecutor;

void auncient_cpm_poly_init(CpmPolyphaseCompilerExecutor *exec);
bool auncient_cpm_poly_phase_a_compile(CpmPolyphaseCompilerExecutor *exec, const char *source_text);
bool auncient_cpm_poly_phase_b_relocate(CpmPolyphaseCompilerExecutor *exec, uint16_t base_addr);
bool auncient_cpm_poly_phase_c_execute(CpmPolyphaseCompilerExecutor *exec, uint32_t max_steps);
bool auncient_cpm_poly_add_tenant(CpmPolyphaseCompilerExecutor *exec, uint32_t tenant_id, uint32_t dna, uint64_t initial_saat);
bool auncient_cpm_poly_verify_all(CpmPolyphaseCompilerExecutor *exec);
uint32_t auncient_cpm_poly_compute_rule18(const CpmPolyphaseCompilerExecutor *exec);

#endif /* AUNCIENT_CPM_POLYPHASE_COMPILER_EXECUTOR_H */
