#ifndef AUNCIENT_RENDERMAN_RAMAC_ILP_DMA_INTERLEAVE_THEOREMS_2266_2270_H
#define AUNCIENT_RENDERMAN_RAMAC_ILP_DMA_INTERLEAVE_THEOREMS_2266_2270_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_RAMAC_DMA_CHANNELS 8
#define RENDERMAN_DMA_WORDS_PER_CHANNEL 1024
#define RENDERMAN_TOTAL_DMA_WORDS (RENDERMAN_RAMAC_DMA_CHANNELS * RENDERMAN_DMA_WORDS_PER_CHANNEL)
#define RENDERMAN_RAMAC_DMA_CANARY_GUARD 0x52414D4143444D41ULL /* "RAMACDMA" */

typedef struct {
    uint32_t channel_id;
    uint64_t ramac_cylinder_addr;
    uint32_t words_transferred;
    bool is_dma_channel_active;
    bool is_parity_valid;
} RendermanRamacDmaChannel;

typedef struct {
    uint64_t head_guard;
    RendermanRamacDmaChannel channels[RENDERMAN_RAMAC_DMA_CHANNELS];
    uint64_t dma_interleaved_buffer[RENDERMAN_TOTAL_DMA_WORDS];
    uint64_t tail_guard;
    uint32_t total_dma_bursts_completed;
    uint32_t teapot_micropolygons_streamed;
    uint32_t cdc6600_ppu_cycles_consumed;
    uint32_t overflow_trapped_bursts;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_8way_interleave_coherent;
    bool is_ramac_dma_memory_safe;
} RendermanRamacIlpDmaContext;

typedef struct {
    float in_silicon_dma_fidelity;
    float dma_strategy_datbin_merkle_ratio;
    float dma_interleave_latency_ns;
    uint64_t verified_dma_saat_clearances;

    bool ramac_dma_pipeline_verified;
    bool dma_strategy_merkle_verified;
    bool dma_submicro_latency_verified;
    bool dma_lossless_saat_verified;
    bool sovereign_2270_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanRamacDmaBeyond2265State;

int cpm_tomie_renderman_ramac_dma_init(RendermanRamacIlpDmaContext *ctx);
int cpm_tomie_renderman_ramac_dma_burst_8way(RendermanRamacIlpDmaContext *ctx, uint32_t words_per_chan);
bool cpm_tomie_renderman_ramac_dma_assert_safety(RendermanRamacIlpDmaContext *ctx);

void auncient_renderman_ramac_dma_init(RendermanRamacDmaBeyond2265State *state);
bool auncient_renderman_ramac_dma_verify_theorems_2266_2270(RendermanRamacDmaBeyond2265State *state);
uint32_t auncient_renderman_ramac_dma_compute_rule18(const RendermanRamacDmaBeyond2265State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_RAMAC_ILP_DMA_INTERLEAVE_THEOREMS_2266_2270_H */
