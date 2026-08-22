#ifndef AUNCIENT_CDC6600_VSEN_SPOOL_SAFETY_THEOREMS_2121_2125_H
#define AUNCIENT_CDC6600_VSEN_SPOOL_SAFETY_THEOREMS_2121_2125_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CDC6600_WORD_BITS 60
#define CDC6600_PACKET_ALIGN_BYTES 8 /* 64-bit alignment for 60-bit word packing */
#define VSEN_MAX_SPOOL_CHANNELS 10

typedef struct {
    uint32_t channel_id;
    uint64_t words_transferred_60bit;
    uint32_t parity_errors_detected;
    uint32_t boundary_guards_passed;
    bool is_channel_active;
    bool is_spool_locked_safe;
} Cdc6600SpoolChannelDescriptor;

typedef struct {
    Cdc6600SpoolChannelDescriptor channels[VSEN_MAX_SPOOL_CHANNELS];
    uint32_t total_aligned_packets;
    uint32_t total_dma_bytes_spooled;
    uint32_t ring_buffer_head;
    uint32_t ring_buffer_tail;
    bool is_60bit_scoreboard_aligned;
    bool is_vsen_spool_failover_ready;
} Cdc6600VsenSpoolContext;

typedef struct {
    float in_silicon_spool_fidelity;
    float cdc6600_spool_strategy_datbin_merkle_ratio;
    float cdc6600_packet_align_latency_ns;
    uint64_t verified_cdc6600_spool_saat_clearances;

    bool cdc6600_spool_safety_verified;
    bool cdc6600_spool_strategy_merkle_verified;
    bool cdc6600_submicro_latency_verified;
    bool cdc6600_lossless_saat_verified;
    bool sovereign_2125_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Cdc6600VsenSpoolBeyond2120State;

int cpm_tomie_cdc6600_spool_init(Cdc6600VsenSpoolContext *ctx);
int cpm_tomie_cdc6600_spool_write_packet(Cdc6600VsenSpoolContext *ctx, uint32_t channel_id, const uint8_t *data, size_t len);
int cpm_tomie_cdc6600_spool_flush_all(Cdc6600VsenSpoolContext *ctx);
void auncient_cdc6600_vsen_spool_init(Cdc6600VsenSpoolBeyond2120State *state);
bool auncient_cdc6600_vsen_spool_verify_theorems_2121_2125(Cdc6600VsenSpoolBeyond2120State *state);
uint32_t auncient_cdc6600_vsen_spool_compute_rule18(const Cdc6600VsenSpoolBeyond2120State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CDC6600_VSEN_SPOOL_SAFETY_THEOREMS_2121_2125_H */
