#ifndef AUNCIENT_CDC6600_EXTENDED_CORE_MEMORY_THEOREMS_2181_2185_H
#define AUNCIENT_CDC6600_EXTENDED_CORE_MEMORY_THEOREMS_2181_2185_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_ECM_BANKS 8
#define CDC6600_ECM_WORDS_PER_BANK 4096

typedef struct {
    uint32_t bank_id;
    uint64_t physical_ecm_base;
    uint32_t transfer_bursts;
    bool is_bank_unlocked;
    bool is_bank_parity_valid;
} Cdc6600EcmBankDescriptor;

typedef struct {
    Cdc6600EcmBankDescriptor banks[CDC6600_ECM_BANKS];
    uint32_t total_ecm_words_transferred;
    uint32_t direct_ppu_dma_cycles;
    bool is_ecm_parallel_access_synced;
    bool is_vsen_virtual_core_mapped;
} Cdc6600ExtendedCoreMemoryContext;

typedef struct {
    float in_silicon_ecm_fidelity;
    float ecm_strategy_datbin_merkle_ratio;
    float ecm_access_latency_ns;
    uint64_t verified_ecm_saat_clearances;

    bool cdc6600_ecm_pipeline_verified;
    bool ecm_strategy_merkle_verified;
    bool ecm_submicro_latency_verified;
    bool ecm_lossless_saat_verified;
    bool sovereign_2185_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600EcmBeyond2180State;

int cpm_tomie_cdc6600_ecm_init(Cdc6600ExtendedCoreMemoryContext *ctx);
int cpm_tomie_cdc6600_ecm_dma_burst(Cdc6600ExtendedCoreMemoryContext *ctx, uint32_t bank_id, uint32_t words_count);
void auncient_cdc6600_ecm_init(Cdc6600EcmBeyond2180State *state);
bool auncient_cdc6600_ecm_verify_theorems_2181_2185(Cdc6600EcmBeyond2180State *state);
uint32_t auncient_cdc6600_ecm_compute_rule18(const Cdc6600EcmBeyond2180State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_EXTENDED_CORE_MEMORY_THEOREMS_2181_2185_H */
