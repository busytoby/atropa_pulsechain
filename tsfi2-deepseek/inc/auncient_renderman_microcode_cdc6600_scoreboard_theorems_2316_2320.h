#ifndef AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2316_2320_H
#define AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2316_2320_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_SCOREBOARD_UNITS 10
#define CDC6600_SCOREBOARD_CANARY_GUARD 0x53434F5245363630ULL /* "SCORE660" */

typedef enum {
    CDC_UNIT_BRANCH = 0,
    CDC_UNIT_BOOLEAN,
    CDC_UNIT_SHIFT,
    CDC_UNIT_ADD_FLOAT,
    CDC_UNIT_ADD_LONG,
    CDC_UNIT_MULT_FLOAT_1,
    CDC_UNIT_MULT_FLOAT_2,
    CDC_UNIT_DIV_FLOAT,
    CDC_UNIT_INC_1,
    CDC_UNIT_INC_2
} Cdc6600FunctionalUnitType;

typedef struct {
    Cdc6600FunctionalUnitType unit_type;
    bool is_busy;
    uint32_t dest_reg;    /* X0..X7, A0..A7, B0..B7 */
    uint32_t src_reg_j;
    uint32_t src_reg_k;
    uint32_t unit_qj;     /* Unit producing src_j */
    uint32_t unit_qk;     /* Unit producing src_k */
    bool is_rj_ready;
    bool is_rk_ready;
} Cdc6600ScoreboardUnitStatus;

typedef struct {
    uint64_t head_guard;
    Cdc6600ScoreboardUnitStatus units[CDC6600_SCOREBOARD_UNITS];
    uint64_t register_result_status[24]; /* 8 X + 8 A + 8 B */
    uint64_t tail_guard;
    uint32_t total_micro_instructions_issued;
    uint32_t hazards_resolved_war_waw_raw;
    uint32_t cdc6600_60bit_executed_words;
    uint32_t overflow_trapped_instructions;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_scoreboard_hazard_free;
    bool is_microcode_scoreboard_memory_safe;
} RendermanMicrocodeScoreboardContext;

typedef struct {
    float in_silicon_scoreboard_fidelity;
    float scoreboard_strategy_datbin_merkle_ratio;
    float scoreboard_issue_latency_ns;
    uint64_t verified_scoreboard_saat_clearances;

    bool scoreboard_pipeline_verified;
    bool scoreboard_strategy_merkle_verified;
    bool scoreboard_submicro_latency_verified;
    bool scoreboard_lossless_saat_verified;
    bool sovereign_2320_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanScoreboardBeyond2315State;

int cpm_tomie_renderman_microcode_scoreboard_init(RendermanMicrocodeScoreboardContext *ctx);
int cpm_tomie_renderman_microcode_scoreboard_issue(RendermanMicrocodeScoreboardContext *ctx, Cdc6600FunctionalUnitType unit, uint32_t dest, uint32_t rj, uint32_t rk);
bool cpm_tomie_renderman_microcode_scoreboard_assert_safety(RendermanMicrocodeScoreboardContext *ctx);

void auncient_renderman_microcode_scoreboard_init(RendermanScoreboardBeyond2315State *state);
bool auncient_renderman_microcode_scoreboard_verify_theorems_2316_2320(RendermanScoreboardBeyond2315State *state);
uint32_t auncient_renderman_microcode_scoreboard_compute_rule18(const RendermanScoreboardBeyond2315State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_MICROCODE_CDC6600_SCOREBOARD_THEOREMS_2316_2320_H */
