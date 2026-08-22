#ifndef AUNCIENT_THACHER_PROVER_THUNK_BRIDGE_THEOREMS_2441_2445_H
#define AUNCIENT_THACHER_PROVER_THUNK_BRIDGE_THEOREMS_2441_2445_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THACHER_PROVER_THUNK_SLOTS 64
#define THACHER_PROVER_CANARY_GUARD 0x54484150524F5636ULL /* "THAPROV6" */

typedef bool (*ThacherFormalProverFn)(void *proof_context);

typedef struct {
    uint32_t prover_thunk_id;
    ThacherFormalProverFn prover_fn;
    void *proof_context_ptr;
    bool is_theorem_proved;
    bool is_thunk_forced;
    uint32_t proof_witness_checksum;
} ThacherProverThunk;

typedef struct {
    uint64_t head_guard;
    ThacherProverThunk prover_thunks[THACHER_PROVER_THUNK_SLOTS];
    uint64_t proof_witness_latch[64];
    uint64_t tail_guard;
    uint32_t total_prover_thunks_registered;
    uint32_t runtime_theorems_proved;
    uint32_t cdc6600_60bit_prover_words;
    uint32_t overflow_trapped_prover_thunks;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_runtime_proving_lossless;
    bool is_thacher_prover_bridge_memory_safe;
} ThacherProverThunkBridgeContext;

typedef struct {
    float in_silicon_prover_thunk_fidelity;
    float prover_thunk_strategy_datbin_merkle_ratio;
    float prover_thunk_forcing_latency_ns;
    uint64_t verified_prover_thunk_saat_clearances;

    bool thacher_prover_bridge_pipeline_verified;
    bool prover_thunk_strategy_merkle_verified;
    bool prover_thunk_submicro_latency_verified;
    bool prover_thunk_lossless_saat_verified;
    bool sovereign_2445_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} ThacherProverBeyond2440State;

int cpm_tomie_thacher_prover_init(ThacherProverThunkBridgeContext *ctx);
int cpm_tomie_thacher_prover_register(ThacherProverThunkBridgeContext *ctx, uint32_t slot_idx, ThacherFormalProverFn fn, void *ctx_ptr);
int cpm_tomie_thacher_prover_force(ThacherProverThunkBridgeContext *ctx, uint32_t slot_idx, bool *out_proved);
bool cpm_tomie_thacher_prover_assert_safety(ThacherProverThunkBridgeContext *ctx);

void auncient_thacher_prover_init(ThacherProverBeyond2440State *state);
bool auncient_thacher_prover_verify_theorems_2441_2445(ThacherProverBeyond2440State *state);
uint32_t auncient_thacher_prover_compute_rule18(const ThacherProverBeyond2440State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_THACHER_PROVER_THUNK_BRIDGE_THEOREMS_2441_2445_H */
