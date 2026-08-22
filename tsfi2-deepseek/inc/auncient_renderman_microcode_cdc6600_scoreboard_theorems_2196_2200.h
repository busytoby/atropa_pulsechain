#ifndef AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2196_2200_H
#define AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2196_2200_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_SCOREBOARD_FUNCTIONAL_UNITS 10
#define CDC6600_RENDERMAN_MICROCODE_OPS 64

typedef enum {
    UNIT_BRANCH = 0,
    UNIT_BOOLEAN = 1,
    UNIT_SHIFT = 2,
    UNIT_ADD_LONG = 3,
    UNIT_FP_ADD = 4,
    UNIT_FP_MULT_1 = 5,
    UNIT_FP_MULT_2 = 6,
    UNIT_FP_DIV = 7,
    UNIT_INCREMENT_1 = 8,
    UNIT_INCREMENT_2 = 9
} Cdc6600FunctionalUnitType;

typedef struct {
    Cdc6600FunctionalUnitType type;
    bool is_busy;
    uint32_t op_code;
    uint32_t dest_reg;
    uint32_t src_reg_j;
    uint32_t src_reg_k;
    uint32_t cycles_remaining;
} Cdc6600ScoreboardUnitState;

typedef struct {
    Cdc6600ScoreboardUnitState units[CDC6600_SCOREBOARD_FUNCTIONAL_UNITS];
    uint32_t total_microcode_instructions_scheduled;
    uint32_t raw_hazard_stalls_prevented;
    uint32_t war_hazard_renamings_handled;
    uint32_t waw_hazard_barriers_cleared;
    bool is_scoreboard_hazard_free;
    bool is_microcode_pipeline_aligned;
} Cdc6600ScoreboardMicrocodeContext;

typedef struct {
    float in_silicon_scoreboard_fidelity;
    float scoreboard_strategy_datbin_merkle_ratio;
    float scoreboard_schedule_latency_ns;
    uint64_t verified_scoreboard_saat_clearances;

    bool scoreboard_pipeline_verified;
    bool scoreboard_strategy_merkle_verified;
    bool scoreboard_submicro_latency_verified;
    bool scoreboard_lossless_saat_verified;
    bool sovereign_2200_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600ScoreboardBeyond2195State;

int cpm_tomie_cdc6600_scoreboard_init(Cdc6600ScoreboardMicrocodeContext *ctx);
int cpm_tomie_cdc6600_scoreboard_schedule_microcode(Cdc6600ScoreboardMicrocodeContext *ctx, uint32_t instruction_count);
void auncient_cdc6600_scoreboard_init(Cdc6600ScoreboardBeyond2195State *state);
bool auncient_cdc6600_scoreboard_verify_theorems_2196_2200(Cdc6600ScoreboardBeyond2195State *state);
uint32_t auncient_cdc6600_scoreboard_compute_rule18(const Cdc6600ScoreboardBeyond2195State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2196_2200_H */
