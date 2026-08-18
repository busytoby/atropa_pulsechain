#ifndef AUNCIENT_ZJ_SNARKS_THEOREMS_211_215_H
#define AUNCIENT_ZJ_SNARKS_THEOREMS_211_215_H

#include "auncient_cpm_polyphase_compiler_executor.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ZJ_SNARK_TPA_BASE 0x0100

typedef struct {
    uint32_t verification_latency_ns;
    uint32_t proof_size_bytes;
    float    covert_leakage_entropy;
    uint64_t verified_saat_settlement;
    bool     succinctness_verified;      /* Theorem 211 */
    bool     glass_box_non_interactive;  /* Theorem 212 */
    bool     jam_free_folding_verified;  /* Theorem 213 */
    bool     atomic_saat_payout_verified;/* Theorem 214 */
    bool     zj_snark_parity_verified;   /* Theorem 215 */
    uint32_t rule18_parity_checksum;
} ZjSnarkProofState;

void auncient_zj_snark_init(ZjSnarkProofState *state);
bool auncient_zj_snark_verify_theorems_211_215(ZjSnarkProofState *state);
uint32_t auncient_zj_snark_compute_rule18(const ZjSnarkProofState *state);

#endif /* AUNCIENT_ZJ_SNARKS_THEOREMS_211_215_H */
