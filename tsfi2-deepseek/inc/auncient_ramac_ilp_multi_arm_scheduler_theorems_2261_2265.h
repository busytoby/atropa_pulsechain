#ifndef AUNCIENT_RAMAC_ILP_MULTI_ARM_SCHEDULER_THEOREMS_2261_2265_H
#define AUNCIENT_RAMAC_ILP_MULTI_ARM_SCHEDULER_THEOREMS_2261_2265_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAMAC_ACCESS_ARMS 4
#define RAMAC_MAX_CYLINDERS 100
#define RAMAC_ILP_ARM_CANARY_GUARD 0x41524D5343484544ULL /* "ARMSCHED" */

typedef struct {
    uint32_t arm_id;
    bool is_seeking;
    uint32_t current_cylinder;
    uint32_t target_cylinder;
    uint32_t target_head;
    uint32_t target_sector;
    uint32_t seek_cycles_remaining;
    bool is_read_locked;
} RamacAccessArmState;

typedef struct {
    uint64_t head_guard;
    RamacAccessArmState arms[RAMAC_ACCESS_ARMS];
    uint64_t tail_guard;
    uint32_t total_multi_arm_dispatches;
    uint32_t simultaneous_parallel_reads;
    uint32_t cdc6600_60bit_interleaved_words;
    uint32_t overflow_trapped_dispatches;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_multi_arm_collision_free;
    bool is_ramac_multi_arm_memory_safe;
} RamacMultiArmSchedulerContext;

typedef struct {
    float in_silicon_arm_fidelity;
    float arm_strategy_datbin_merkle_ratio;
    float arm_schedule_latency_ns;
    uint64_t verified_arm_saat_clearances;

    bool multi_arm_scheduler_pipeline_verified;
    bool arm_strategy_merkle_verified;
    bool arm_submicro_latency_verified;
    bool arm_lossless_saat_verified;
    bool sovereign_2265_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RamacMultiArmBeyond2260State;

int cpm_tomie_ramac_multi_arm_init(RamacMultiArmSchedulerContext *ctx);
int cpm_tomie_ramac_multi_arm_dispatch(RamacMultiArmSchedulerContext *ctx, uint32_t arm_id, uint32_t cyl, uint32_t head, uint32_t sec);
bool cpm_tomie_ramac_multi_arm_assert_safety(RamacMultiArmSchedulerContext *ctx);

void auncient_ramac_multi_arm_scheduler_init(RamacMultiArmBeyond2260State *state);
bool auncient_ramac_multi_arm_scheduler_verify_theorems_2261_2265(RamacMultiArmBeyond2260State *state);
uint32_t auncient_ramac_multi_arm_scheduler_compute_rule18(const RamacMultiArmBeyond2260State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RAMAC_ILP_MULTI_ARM_SCHEDULER_THEOREMS_2261_2265_H */
