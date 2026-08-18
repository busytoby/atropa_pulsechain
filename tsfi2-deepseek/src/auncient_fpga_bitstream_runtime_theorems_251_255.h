#ifndef AUNCIENT_FPGA_BITSTREAM_RUNTIME_THEOREMS_251_255_H
#define AUNCIENT_FPGA_BITSTREAM_RUNTIME_THEOREMS_251_255_H

#include "auncient_cpm_fpga_certification_theorems_246_250.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_FPGA_PARTIAL_REGIONS 4

typedef struct {
    uint32_t active_partial_regions;
    float    bitstream_dma_bandwidth_gbps;
    float    cdc_metastability_mtbf_years;
    uint64_t verified_zero_glitch_cycles;
    bool     pr_bitstream_integrity_verified;    /* Theorem 251 */
    bool     dma_burst_throughput_verified;      /* Theorem 252 */
    bool     cdc_metastability_immunity_verified;/* Theorem 253 */
    bool     zero_glitch_hot_swap_verified;      /* Theorem 254 */
    bool     fpga_runtime_grand_parity_verified; /* Theorem 255 */
    uint32_t rule18_parity_checksum;
} FpgaBitstreamRuntimeState;

void auncient_fpga_runtime_init(FpgaBitstreamRuntimeState *state);
bool auncient_fpga_runtime_verify_theorems_251_255(FpgaBitstreamRuntimeState *state);
uint32_t auncient_fpga_runtime_compute_rule18(const FpgaBitstreamRuntimeState *state);

#endif /* AUNCIENT_FPGA_BITSTREAM_RUNTIME_THEOREMS_251_255_H */
