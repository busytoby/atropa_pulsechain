#ifndef AUNCIENT_RAMAC_BARREL_SHIFTER_DUAL_INC_THEOREMS_2256_2260_H
#define AUNCIENT_RAMAC_BARREL_SHIFTER_DUAL_INC_THEOREMS_2256_2260_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAMAC_ROTOR_SECTORS 20
#define RAMAC_BARREL_SHIFTER_CANARY_GUARD 0x524F544F52535441ULL /* "ROTORSTA" */

typedef struct {
    uint32_t current_rotor_sector;
    uint32_t target_stator_sector;
    uint32_t angular_phase_shift;
    uint64_t raw_rotor_word;
    uint64_t aligned_barrel_shifted_word;
    bool is_shift_aligned;
} RamacRotorStatorState;

typedef struct {
    uint64_t head_guard;
    RamacRotorStatorState channels[RAMAC_ROTOR_SECTORS];
    uint64_t tail_guard;
    uint32_t total_barrel_rotations_executed;
    uint32_t zero_latency_hits;
    uint32_t cdc6600_60bit_shifted_words;
    uint32_t overflow_trapped_shifts;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_barrel_rotation_bijective;
    bool is_ramac_barrel_shifter_memory_safe;
} RamacBarrelShifterContext;

typedef struct {
    float in_silicon_shifter_fidelity;
    float shifter_strategy_datbin_merkle_ratio;
    float shifter_align_latency_ns;
    uint64_t verified_shifter_saat_clearances;

    bool barrel_shifter_pipeline_verified;
    bool shifter_strategy_merkle_verified;
    bool shifter_submicro_latency_verified;
    bool shifter_lossless_saat_verified;
    bool sovereign_2260_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RamacBarrelShifterBeyond2255State;

int cpm_tomie_ramac_barrel_shifter_init(RamacBarrelShifterContext *ctx);
int cpm_tomie_ramac_barrel_shift_sector(RamacBarrelShifterContext *ctx, uint32_t rotor_sec, uint32_t stator_sec, uint64_t word);
bool cpm_tomie_ramac_barrel_shifter_assert_safety(RamacBarrelShifterContext *ctx);

void auncient_ramac_barrel_shifter_init(RamacBarrelShifterBeyond2255State *state);
bool auncient_ramac_barrel_shifter_verify_theorems_2256_2260(RamacBarrelShifterBeyond2255State *state);
uint32_t auncient_ramac_barrel_shifter_compute_rule18(const RamacBarrelShifterBeyond2255State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RAMAC_BARREL_SHIFTER_DUAL_INC_THEOREMS_2256_2260_H */
