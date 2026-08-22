#ifndef AUNCIENT_CDC6600_SCOREBOARD_DUAL_INCREMENT_THEOREMS_2251_2255_H
#define AUNCIENT_CDC6600_SCOREBOARD_DUAL_INCREMENT_THEOREMS_2251_2255_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_INCREMENT_UNITS 2
#define CDC6600_SCOREBOARD_A_REGISTERS 8
#define CDC6600_SCOREBOARD_B_REGISTERS 8
#define CDC6600_SCOREBOARD_X_REGISTERS 8
#define CDC6600_DUAL_INC_CANARY_GUARD 0x494E435236363030ULL /* "INCR6600" */

typedef struct {
    uint32_t unit_id;
    bool is_busy;
    uint32_t dest_a_or_b_reg;
    uint32_t src_b_or_k_reg;
    int32_t immediate_k;
    uint32_t cycles_remaining;
} Cdc6600IncrementUnit;

typedef struct {
    uint64_t head_guard;
    uint64_t a_regs[CDC6600_SCOREBOARD_A_REGISTERS]; /* Address registers (A1-A5 load X1-X5, A6-A7 store X6-X7) */
    uint64_t b_regs[CDC6600_SCOREBOARD_B_REGISTERS]; /* Index registers B0-B7 (B0 == 0) */
    uint64_t x_regs[CDC6600_SCOREBOARD_X_REGISTERS]; /* Operand registers X0-X7 */
    Cdc6600IncrementUnit inc_units[CDC6600_INCREMENT_UNITS];
    uint64_t tail_guard;
    uint32_t total_address_increments_executed;
    uint32_t memory_loads_initiated;
    uint32_t memory_stores_initiated;
    uint32_t overflow_trapped_instructions;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_b0_zero_invariant_held;
    bool is_dual_increment_memory_safe;
} Cdc6600ScoreboardDualIncrementContext;

typedef struct {
    float in_silicon_increment_fidelity;
    float increment_strategy_datbin_merkle_ratio;
    float increment_dispatch_latency_ns;
    uint64_t verified_increment_saat_clearances;

    bool dual_increment_pipeline_verified;
    bool increment_strategy_merkle_verified;
    bool increment_submicro_latency_verified;
    bool increment_lossless_saat_verified;
    bool sovereign_2255_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600DualIncrementBeyond2250State;

int cpm_tomie_cdc6600_dual_increment_init(Cdc6600ScoreboardDualIncrementContext *ctx);
int cpm_tomie_cdc6600_dual_increment_issue(Cdc6600ScoreboardDualIncrementContext *ctx, uint32_t unit_id, uint32_t dest_a, uint32_t src_b, int32_t k);
bool cpm_tomie_cdc6600_dual_increment_assert_safety(Cdc6600ScoreboardDualIncrementContext *ctx);

void auncient_cdc6600_dual_increment_init(Cdc6600DualIncrementBeyond2250State *state);
bool auncient_cdc6600_dual_increment_verify_theorems_2251_2255(Cdc6600DualIncrementBeyond2250State *state);
uint32_t auncient_cdc6600_dual_increment_compute_rule18(const Cdc6600DualIncrementBeyond2250State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_SCOREBOARD_DUAL_INCREMENT_THEOREMS_2251_2255_H */
