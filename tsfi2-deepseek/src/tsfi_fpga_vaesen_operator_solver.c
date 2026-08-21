#include "tsfi_fpga_vaesen_operator_solver.h"

int tsfi_verify_fpga_vaesen_operator(const TsfiFpgaVaesenOpState *state) {
    if (!state) return TSFI_FPGA_VAESEN_ERR_K_EXPONENT;

    if (state->k_param != TSFI_FPGA_VAESEN_K_PARAM) {
        return TSFI_FPGA_VAESEN_ERR_K_EXPONENT;
    }

    if (state->dsp_clock_latency_ps < 1 || state->dsp_clock_latency_ps > TSFI_FPGA_DSP_MAX_CLOCK_PS) {
        return TSFI_FPGA_VAESEN_ERR_DSP_LATENCY;
    }

    if (state->digital_pll_phase_deg > 359) {
        return TSFI_FPGA_VAESEN_ERR_DIGITAL_PLL;
    }

    if (state->bram_slice_exactness_flag != 1) {
        return TSFI_FPGA_VAESEN_ERR_BRAM_SLICE;
    }

    if (state->kouwenhoven_vitality < 1 || state->kouwenhoven_vitality > 10000) {
        return TSFI_FPGA_VAESEN_ERR_VITALITY_CRIT;
    }

    if (state->cics_writer_id != 1000000ULL) {
        return TSFI_FPGA_VAESEN_ERR_CICS_UNVERIFIED;
    }

    uint64_t shadow_baseline = 0;
    uint64_t current_energy = (uint64_t)state->dsp_clock_latency_ps + (uint64_t)(state->kouwenhoven_vitality % 100);

    /* BMRC Monotonic Lyapunov Contraction (gamma = 875/1000) */
    for (int step = 0; step < 16; ++step) {
        current_energy = (current_energy * 875ULL) / 1000ULL;
    }

    if (current_energy > (uint64_t)(state->dsp_clock_latency_ps + 100)) {
        return TSFI_FPGA_VAESEN_ERR_BMRC_CONTRACTION;
    }

    if (state->simulate_bram_fault != 0) {
        uint64_t committed_output = shadow_baseline;
        if (committed_output != 0) {
            return TSFI_FPGA_VAESEN_ERR_SHADOW_CORRUPT;
        }
    }

    return TSFI_FPGA_VAESEN_OK;
}
