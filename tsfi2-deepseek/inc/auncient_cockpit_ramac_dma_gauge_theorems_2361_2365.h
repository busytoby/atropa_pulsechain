#ifndef AUNCIENT_COCKPIT_RAMAC_DMA_GAUGE_THEOREMS_2361_2365_H
#define AUNCIENT_COCKPIT_RAMAC_DMA_GAUGE_THEOREMS_2361_2365_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAMAC_CYLINDER_SECTORS 8
#define RAMAC_WORDS_PER_SECTOR 128
#define RAMAC_TOTAL_WORDS (RAMAC_CYLINDER_SECTORS * RAMAC_WORDS_PER_SECTOR) /* 1,024 words */
#define COCKPIT_RAMAC_CANARY_GUARD 0x52414D4143474136ULL /* "RAMACGA6" */

typedef struct {
    uint32_t sector_id;
    uint32_t seek_head_cylinder;
    float telemetry_gauge_value;
    bool is_sector_coherent;
} RamacTelemetrySector;

typedef struct {
    uint64_t head_guard;
    RamacTelemetrySector sectors[RAMAC_CYLINDER_SECTORS];
    uint64_t ramac_cylinder_dma_latch[RAMAC_TOTAL_WORDS];
    uint64_t tail_guard;
    uint32_t total_dma_bursts_transferred;
    uint32_t gauge_sweeps_completed;
    uint32_t cdc6600_60bit_ramac_words;
    uint32_t overflow_trapped_dma_requests;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_ramac_dma_stream_lossless;
    bool is_cockpit_ramac_memory_safe;
} CockpitRamacDmaGaugeContext;

typedef struct {
    float in_silicon_ramac_fidelity;
    float ramac_strategy_datbin_merkle_ratio;
    float ramac_dma_latency_ns;
    uint64_t verified_ramac_saat_clearances;

    bool ramac_pipeline_verified;
    bool ramac_strategy_merkle_verified;
    bool ramac_submicro_latency_verified;
    bool ramac_lossless_saat_verified;
    bool sovereign_2365_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} CockpitRamacBeyond2360State;

int cpm_tomie_cockpit_ramac_dma_init(CockpitRamacDmaGaugeContext *ctx);
int cpm_tomie_cockpit_ramac_dma_stream(CockpitRamacDmaGaugeContext *ctx, uint32_t sector_id, float gauge_val);
bool cpm_tomie_cockpit_ramac_dma_assert_safety(CockpitRamacDmaGaugeContext *ctx);

void auncient_cockpit_ramac_dma_init(CockpitRamacBeyond2360State *state);
bool auncient_cockpit_ramac_dma_verify_theorems_2361_2365(CockpitRamacBeyond2360State *state);
uint32_t auncient_cockpit_ramac_dma_compute_rule18(const CockpitRamacBeyond2360State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_COCKPIT_RAMAC_DMA_GAUGE_THEOREMS_2361_2365_H */
