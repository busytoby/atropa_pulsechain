#ifndef TSFI_VAESEN_LLM_SOLVER_H
#define TSFI_VAESEN_LLM_SOLVER_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MOTZKIN_PRIME 953467954114363ULL
#define TSFI_VAESEN_LLM_K_PARAM 3
#define TSFI_VAESEN_LLM_BMRC_NUM 875
#define TSFI_VAESEN_LLM_BMRC_DENOM 1000

typedef struct {
    uint8_t token_lattice_freq_edo22;  /* [1..22] EDO-22 lattice step */
    uint16_t mathieu_modulation_q_milli; /* [0..5000] modulation index in milli-units */
    uint8_t weyl_closure_exactness_flag; /* 1 = exact holonomic closure Cl_S(I) */
    uint64_t engram_accumulator_energy;  /* [0..MotzkinPrime] stored trauma engram */
    uint64_t cics_writer_id;             /* 1000000 = verified citizen participant */
    uint8_t simulate_gradient_explosion; /* 0 = normal, 1 = test fault rollback */
    uint8_t k_param;                     /* must be 3 */
} TsfiVaesenLlmState;

typedef enum {
    TSFI_VAESEN_LLM_OK = 0,
    TSFI_VAESEN_LLM_ERR_K_EXPONENT = 1,
    TSFI_VAESEN_LLM_ERR_EDO22_FREQ = 2,
    TSFI_VAESEN_LLM_ERR_MATHIEU_MOD = 3,
    TSFI_VAESEN_LLM_ERR_WEYL_CLOSURE = 4,
    TSFI_VAESEN_LLM_ERR_ACCUMULATOR_NEG = 5,
    TSFI_VAESEN_LLM_ERR_CICS_UNVERIFIED = 6,
    TSFI_VAESEN_LLM_ERR_BMRC_CONTRACTION = 7,
    TSFI_VAESEN_LLM_ERR_SHADOW_CORRUPT = 8
} TsfiVaesenLlmStatus;

int tsfi_verify_vaesen_llm_scaffolding(const TsfiVaesenLlmState *state);

#endif /* TSFI_VAESEN_LLM_SOLVER_H */
