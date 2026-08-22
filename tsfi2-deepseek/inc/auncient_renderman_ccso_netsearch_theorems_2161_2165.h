#ifndef AUNCIENT_RENDERMAN_CCSO_NETSEARCH_THEOREMS_2161_2165_H
#define AUNCIENT_RENDERMAN_CCSO_NETSEARCH_THEOREMS_2161_2165_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CCSO_MAX_ENTRIES 32
#define CCSO_FIELD_LEN 64

typedef struct {
    char handle[CCSO_FIELD_LEN];
    char name[CCSO_FIELD_LEN];
    char asset_datbin[CCSO_FIELD_LEN];
    char shader_rib[CCSO_FIELD_LEN];
    uint32_t contract_address; /* Rule 9: dynamic_<address> */
    bool is_verified_citizen;
} CcsoDirectoryEntry;

typedef struct {
    CcsoDirectoryEntry entries[CCSO_MAX_ENTRIES];
    uint32_t total_indexed_entries;
    uint32_t netsearch_queries_executed;
    uint32_t matching_assets_resolved;
    bool is_ccso_qi_ph_server_active;
    bool is_netsearch_index_locked;
} RendermanCcsoNetsearchContext;

typedef struct {
    float in_silicon_netsearch_fidelity;
    float ccso_strategy_datbin_merkle_ratio;
    float netsearch_lookup_latency_ns;
    uint64_t verified_netsearch_saat_clearances;

    bool ccso_netsearch_pipeline_verified;
    bool ccso_strategy_merkle_verified;
    bool ccso_submicro_latency_verified;
    bool ccso_lossless_saat_verified;
    bool sovereign_2165_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanCcsoNetsearchBeyond2160State;

int cpm_tomie_ccso_netsearch_init(RendermanCcsoNetsearchContext *ctx);
int cpm_tomie_ccso_netsearch_query(RendermanCcsoNetsearchContext *ctx, const char *search_key, CcsoDirectoryEntry *out_entry);
void auncient_renderman_ccso_netsearch_init(RendermanCcsoNetsearchBeyond2160State *state);
bool auncient_renderman_ccso_netsearch_verify_theorems_2161_2165(RendermanCcsoNetsearchBeyond2160State *state);
uint32_t auncient_renderman_ccso_netsearch_compute_rule18(const RendermanCcsoNetsearchBeyond2160State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_CCSO_NETSEARCH_THEOREMS_2161_2165_H */
