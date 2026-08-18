#ifndef AUNCIENT_SCHUURMAN_LIVING_LAB_H
#define AUNCIENT_SCHUURMAN_LIVING_LAB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SCHUURMAN_TPA_BASE 0x0100

typedef enum {
    SCHUURMAN_STAGE_EXPLORATION    = 1,
    SCHUURMAN_STAGE_CO_CREATION    = 2,
    SCHUURMAN_STAGE_EXPERIMENTATION= 3,
    SCHUURMAN_STAGE_EVALUATION     = 4,
    SCHUURMAN_STAGE_SCALING        = 5
} SchuurmanStage;

typedef struct {
    SchuurmanStage current_stage;
    float infrastructure_score;  /* Dimension 1: 0.0 to 1.0 */
    float methodology_score;     /* Dimension 2: 0.0 to 1.0 */
    float ecosystem_score;       /* Dimension 3: 0.0 to 1.0 */
    float triad_balance_ratio;
    float causal_effect_estimate;
    float nash_equilibrium_temp_c;
    bool stage_machine_verified;
    bool triad_balance_verified;
    bool temporal_dag_verified;
    bool causal_unconfoundedness_verified;
    bool co_creation_nash_verified;
    uint32_t rule18_parity_checksum;
} SchuurmanLivingLabState;

void auncient_schuurman_init(SchuurmanLivingLabState *state);
bool auncient_schuurman_verify_theorems_36_40(SchuurmanLivingLabState *state);
uint32_t auncient_schuurman_compute_rule18_checksum(const SchuurmanLivingLabState *state);

#endif /* AUNCIENT_SCHUURMAN_LIVING_LAB_H */
