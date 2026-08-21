#ifndef TSFI_HELMHOLTZ_TRANSITIVE_DEDUCTION_SOLVER_H
#define TSFI_HELMHOLTZ_TRANSITIVE_DEDUCTION_SOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_HELMHOLTZ_MAX_STEPS 64
#define TSFI_HELMHOLTZ_MAX_NAME  128

typedef enum {
    TSFI_STEP_KIND_SOVEREIGN_SEED       = 0, // DNA / Hogan Bank 1,000,000 Saat Initialization
    TSFI_STEP_KIND_INITIAL_ORDERS1_PRE  = 1, // EDSAC Initial Orders 1 Instruction Prefetch Firewall
    TSFI_STEP_KIND_MOTZKIN_MODPOW       = 2, // 3-Term Non-Preferential Recurrence over MotzkinPrime
    TSFI_STEP_KIND_TOTIENT_NULLSTELLEN  = 3, // Totient = 0 Radical Variety Annihilation sqrt(I)
    TSFI_STEP_KIND_EDO22_HELMHOLTZ_ROPE = 4, // Helmholtz Acoustic Wave & 22-EDO Rotary Projection
    TSFI_STEP_KIND_ACCUMULATOR_CLAMP    = 5  // Non-Preferential Accumulator Energy Neutrality
} TsfiHelmholtzStepKind;

typedef struct {
    uint32_t step_index;
    TsfiHelmholtzStepKind kind;
    char descriptor[TSFI_HELMHOLTZ_MAX_NAME];
    int64_t in_register;
    int64_t out_register;
    uint32_t ast_proof_hash;
    bool step_verified;
} TsfiHelmholtzDeductionNode;

typedef struct {
    uint32_t step_count;
    TsfiHelmholtzDeductionNode steps[TSFI_HELMHOLTZ_MAX_STEPS];
    uint8_t deduction_merkle_root[32];
    double total_deduction_latency_ns;
    bool pipeline_verified;
} TsfiHelmholtzDeductionList;

bool tsfi_helmholtz_deduction_init(TsfiHelmholtzDeductionList *list);
bool tsfi_helmholtz_deduction_append_step(
    TsfiHelmholtzDeductionList *list,
    TsfiHelmholtzStepKind kind,
    const char *descriptor,
    int64_t in_val,
    uint32_t ast_proof_hash
);
bool tsfi_helmholtz_deduction_execute_in_order(TsfiHelmholtzDeductionList *list);

#endif // TSFI_HELMHOLTZ_TRANSITIVE_DEDUCTION_SOLVER_H
