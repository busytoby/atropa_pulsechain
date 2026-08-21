#ifndef TSFI_FPGA_VAESEN_OPERATOR_SOLVER_H
#define TSFI_FPGA_VAESEN_OPERATOR_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_FPGA_VAESEN_K_PARAM 3
#define TSFI_FPGA_DSP_MAX_CLOCK_PS 1000

typedef struct {
    uint32_t dsp_clock_latency_ps;      /* <= 1000 ps */
    uint16_t digital_pll_phase_deg;     /* [0..359] */
    uint8_t  bram_slice_exactness_flag; /* 1 = exact on-chip BRAM mapping */
    uint16_t kouwenhoven_vitality;      /* [1..10000] */
    uint64_t cics_writer_id;            /* 1000000 = verified participant */
    uint8_t  simulate_bram_fault;       /* 0 = normal, 1 = fault injection test */
    uint8_t  k_param;                   /* must be 3 */
} TsfiFpgaVaesenOpState;

typedef enum {
    TSFI_FPGA_VAESEN_OK = 0,
    TSFI_FPGA_VAESEN_ERR_K_EXPONENT = 1,
    TSFI_FPGA_VAESEN_ERR_DSP_LATENCY = 2,
    TSFI_FPGA_VAESEN_ERR_DIGITAL_PLL = 3,
    TSFI_FPGA_VAESEN_ERR_BRAM_SLICE = 4,
    TSFI_FPGA_VAESEN_ERR_VITALITY_CRIT = 5,
    TSFI_FPGA_VAESEN_ERR_CICS_UNVERIFIED = 6,
    TSFI_FPGA_VAESEN_ERR_BMRC_CONTRACTION = 7,
    TSFI_FPGA_VAESEN_ERR_SHADOW_CORRUPT = 8
} TsfiFpgaVaesenOpStatus;

int tsfi_verify_fpga_vaesen_operator(const TsfiFpgaVaesenOpState *state);

#endif /* TSFI_FPGA_VAESEN_OPERATOR_SOLVER_H */
