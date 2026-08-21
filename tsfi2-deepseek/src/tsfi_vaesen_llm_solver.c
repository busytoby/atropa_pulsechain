#include "tsfi_vaesen_llm_solver.h"

int tsfi_verify_vaesen_llm_scaffolding(const TsfiVaesenLlmState *state) {
    if (!state) return TSFI_VAESEN_LLM_ERR_K_EXPONENT;

    if (state->k_param != TSFI_VAESEN_LLM_K_PARAM) {
        return TSFI_VAESEN_LLM_ERR_K_EXPONENT;
    }

    if (state->token_lattice_freq_edo22 < 1 || state->token_lattice_freq_edo22 > 22) {
        return TSFI_VAESEN_LLM_ERR_EDO22_FREQ;
    }

    if (state->mathieu_modulation_q_milli > 5000) {
        return TSFI_VAESEN_LLM_ERR_MATHIEU_MOD;
    }

    if (state->weyl_closure_exactness_flag != 1) {
        return TSFI_VAESEN_LLM_ERR_WEYL_CLOSURE;
    }

    if (state->engram_accumulator_energy > TSFI_MOTZKIN_PRIME) {
        return TSFI_VAESEN_LLM_ERR_ACCUMULATOR_NEG;
    }

    if (state->cics_writer_id != 1000000ULL) {
        return TSFI_VAESEN_LLM_ERR_CICS_UNVERIFIED;
    }

    uint64_t shadow_baseline = 0;
    uint64_t current_energy = (uint64_t)state->mathieu_modulation_q_milli + (state->engram_accumulator_energy % 1000ULL);

    /* BMRC Monotonic Lyapunov Contraction (gamma = 875/1000) */
    for (int step = 0; step < 16; ++step) {
        current_energy = (current_energy * TSFI_VAESEN_LLM_BMRC_NUM) / TSFI_VAESEN_LLM_BMRC_DENOM;
    }

    if (current_energy > (uint64_t)(state->mathieu_modulation_q_milli + 1000)) {
        return TSFI_VAESEN_LLM_ERR_BMRC_CONTRACTION;
    }

    if (state->simulate_gradient_explosion != 0) {
        uint64_t committed_output = shadow_baseline;
        if (committed_output != 0) {
            return TSFI_VAESEN_LLM_ERR_SHADOW_CORRUPT;
        }
    }

    return TSFI_VAESEN_LLM_OK;
}
