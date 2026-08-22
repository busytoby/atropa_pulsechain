#ifndef AUNCIENT_SIGOPS_FORMAL_MEMORY_SAFETY_THEOREMS_2276_2280_H
#define AUNCIENT_SIGOPS_FORMAL_MEMORY_SAFETY_THEOREMS_2276_2280_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BLOCK_SIGOPS_LIMIT 20000
#define MAX_TRANSACTION_SIGOPS 2500
#define SIGOPS_CANARY_GUARD 0x5349474F50533636ULL /* "SIGOPS66" */

typedef struct {
    uint32_t tx_id;
    uint32_t sigops_count;
    bool is_p2pkh;
    bool is_multisig;
    bool is_script_valid;
} SigopsTransactionRecord;

typedef struct {
    uint64_t head_guard;
    SigopsTransactionRecord transactions[64];
    uint64_t tail_guard;
    uint32_t active_tx_count;
    uint32_t cumulative_block_sigops;
    uint32_t cdc6600_60bit_sigop_words;
    uint32_t rejected_overrun_sigops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_block_sigops_bounded;
    bool is_sigops_pipeline_memory_safe;
} SigopsVerificationContext;

typedef struct {
    float in_silicon_sigops_fidelity;
    float sigops_strategy_datbin_merkle_ratio;
    float sigops_verification_latency_ns;
    uint64_t verified_sigops_saat_clearances;

    bool sigops_pipeline_verified;
    bool sigops_strategy_merkle_verified;
    bool sigops_submicro_latency_verified;
    bool sigops_lossless_saat_verified;
    bool sovereign_2280_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} SigopsBeyond2275State;

int cpm_tomie_sigops_init(SigopsVerificationContext *ctx);
int cpm_tomie_sigops_add_tx(SigopsVerificationContext *ctx, uint32_t tx_id, uint32_t sigops, bool is_multisig);
bool cpm_tomie_sigops_assert_safety(SigopsVerificationContext *ctx);

void auncient_sigops_formal_safety_init(SigopsBeyond2275State *state);
bool auncient_sigops_formal_safety_verify_theorems_2276_2280(SigopsBeyond2275State *state);
uint32_t auncient_sigops_formal_safety_compute_rule18(const SigopsBeyond2275State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_SIGOPS_FORMAL_MEMORY_SAFETY_THEOREMS_2276_2280_H */
