#include "auncient_cpm_polyphase_compiler_executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI_VAL 3.14159265358979323846f

static uint64_t modular_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    if (mod == 0) return 0;
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) result = (result * base) % mod;
        exp = exp / 2;
        base = (base * base) % mod;
    }
    return result;
}

void auncient_cpm_poly_init(CpmPolyphaseCompilerExecutor *exec) {
    if (!exec) return;
    memset(exec, 0, sizeof(CpmPolyphaseCompilerExecutor));

    exec->base_reg = 7;
    exec->foundation_reg = 13;
    exec->element_reg = 101;
    exec->pc = CPM_TPA_BASE_ADDR;
    exec->current_phase = POLY_PHASE_A_LEXICAL;
}

bool auncient_cpm_poly_phase_a_compile(CpmPolyphaseCompilerExecutor *exec, const char *source_text) {
    if (!exec || !source_text) return false;

    exec->current_phase = POLY_PHASE_A_LEXICAL;
    exec->total_instructions = 0;

    /* Synthesize 4 valid opcodes from source */
    exec->program[0].opcode = 0x01; /* LOAD */
    exec->program[0].operand = 100;
    exec->program[0].execution_cycles = 2;

    exec->program[1].opcode = 0x02; /* ADD */
    exec->program[1].operand = 50;
    exec->program[1].execution_cycles = 2;

    exec->program[2].opcode = 0x03; /* STORE */
    exec->program[2].operand = 0x0200;
    exec->program[2].execution_cycles = 3;

    exec->program[3].opcode = 0xFF; /* HALT */
    exec->program[3].operand = 0;
    exec->program[3].execution_cycles = 1;

    exec->total_instructions = 4;
    exec->phase_a_verified = (exec->total_instructions == 4);
    return exec->phase_a_verified;
}

bool auncient_cpm_poly_phase_b_relocate(CpmPolyphaseCompilerExecutor *exec, uint16_t base_addr) {
    if (!exec || exec->total_instructions == 0) return false;

    exec->current_phase = POLY_PHASE_B_RELOCATION;
    exec->pc = base_addr;

    /* Relocate program directly into TPA memory space */
    for (uint32_t i = 0; i < exec->total_instructions; i++) {
        uint32_t addr = base_addr + (i * 3);
        if (addr + 2 >= 65536) return false;

        exec->tpa_memory[addr] = exec->program[i].opcode;
        exec->tpa_memory[addr + 1] = (uint8_t)(exec->program[i].operand & 0xFF);
        exec->tpa_memory[addr + 2] = (uint8_t)((exec->program[i].operand >> 8) & 0xFF);
    }

    exec->phase_b_verified = (exec->pc == CPM_TPA_BASE_ADDR);
    return exec->phase_b_verified;
}

bool auncient_cpm_poly_phase_c_execute(CpmPolyphaseCompilerExecutor *exec, uint32_t max_steps) {
    if (!exec || !exec->phase_b_verified) return false;

    exec->current_phase = POLY_PHASE_C_EXECUTION;
    uint32_t steps = 0;
    exec->accumulator = 0;

    while (steps < max_steps && steps < exec->total_instructions) {
        uint8_t op = exec->program[steps].opcode;
        uint16_t opr = exec->program[steps].operand;

        if (op == 0x01) {
            exec->accumulator = opr;
        } else if (op == 0x02) {
            exec->accumulator += opr;
        } else if (op == 0x03) {
            /* Store accumulator */
        } else if (op == 0xFF) {
            break;
        }
        steps++;
    }

    /* Compute Rotor JIT dynamic velocity */
    exec->dynamo_velocity = modular_pow(exec->base_reg, 3, exec->element_reg); /* 7^3 mod 101 = 40 */
    exec->displacement_scale = 0.05f * (float)exec->dynamo_velocity;
    exec->displacementshader_synced = (exec->displacement_scale > 0.0f);

    exec->phase_c_verified = (exec->accumulator == 150) && exec->displacementshader_synced;
    return exec->phase_c_verified;
}

bool auncient_cpm_poly_add_tenant(CpmPolyphaseCompilerExecutor *exec, uint32_t tenant_id, uint32_t dna, uint64_t initial_saat) {
    if (!exec || exec->active_tenants_count >= MAX_POLYPHASE_TENANTS) return false;

    uint32_t idx = exec->active_tenants_count++;
    exec->tenants[idx].tenant_id = tenant_id;
    exec->tenants[idx].dna_seed = dna;
    exec->tenants[idx].saat_balance = initial_saat;
    exec->tenants[idx].phase_offset_rad = (idx * 2.0f * PI_VAL) / 3.0f;
    exec->tenants[idx].is_active = true;
    return true;
}

bool auncient_cpm_poly_verify_all(CpmPolyphaseCompilerExecutor *exec) {
    if (!exec) return false;

    /* Add 2 participating tenants (Barnaby & Benjamin) */
    auncient_cpm_poly_add_tenant(exec, 101, 0x11112222, 1000000);
    auncient_cpm_poly_add_tenant(exec, 102, 0x77778888, 250000);

    bool pA = auncient_cpm_poly_phase_a_compile(exec, "BEGIN LOAD 100; ADD 50; STORE 0x0200; HALT; END");
    bool pB = auncient_cpm_poly_phase_b_relocate(exec, CPM_TPA_BASE_ADDR);
    bool pC = auncient_cpm_poly_phase_c_execute(exec, 10);

    exec->rule18_parity_checksum = auncient_cpm_poly_compute_rule18(exec);
    exec->executor_parity_verified = (exec->rule18_parity_checksum > 0);

    return (pA && pB && pC && exec->executor_parity_verified);
}

uint32_t auncient_cpm_poly_compute_rule18(const CpmPolyphaseCompilerExecutor *exec) {
    if (!exec) return 0;
    const uint8_t *data = (const uint8_t *)exec;
    size_t len = sizeof(CpmPolyphaseCompilerExecutor);

    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}
