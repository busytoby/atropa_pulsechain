#ifndef AUNCIENT_FULL_VSEN_IN_CPMTOMIE_FPGA_THEOREMS_1021_1025_H
#define AUNCIENT_FULL_VSEN_IN_CPMTOMIE_FPGA_THEOREMS_1021_1025_H

#include "auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Full In-CPMTomie VSEn Subsystem State (1:1 Complete Replacement & In-Silicon Improvement) */
typedef struct {
    uint32_t active_in_cpmtomie_vsen_partitions; /* 32 full VSEn partitions hosted directly in CPMTomie OS */
    uint32_t bound_bram_vsen_ring_channels;     /* 256 hardware-isolated BRAM circular FIFO queues */
    float    vsen_1to1_replacement_fidelity;    /* 1.000 (Exact 1:1 behavioral equivalence and superior throughput) */
    float    vsen_in_cpmtomie_latency_ns;       /* Sub-microsecond dispatch and spooling latency (< 1000.0 ns - Rule 11) */
    float    displacement_full_vsen_phase;      /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_full_vsen_in_cpmtomie_certified;
} FullVsenInCpmtomieState;

typedef struct {
    float    in_silicon_full_vsen_fidelity;
    float    full_vsen_merkle_continuity_ratio;
    float    full_vsen_dispatch_latency_ns;
    uint64_t verified_full_vsen_saat_clearances;
    bool     full_vsen_1to1_fidelity_verified;    /* Theorem 1021: Full VSEn 1:1 Replacement In-Silicon Fidelity Invariance */
    bool     full_vsen_merkle_continuity_verified;/* Theorem 1022: 2-3 Tree AST Merkle VSEn Partition Guard */
    bool     full_vsen_submicro_latency_verified; /* Theorem 1023: Sub-Microsecond Spooler Latency Guard */
    bool     full_vsen_lossless_saat_verified;    /* Theorem 1024: 1.025B Saat Commutation Flow */
    bool     grand_1025_parity_closure_verified;  /* Theorem 1025: Grand Master Parity Seal */
    uint32_t rule18_parity_checksum;
} FpgaFullVsenState;

void auncient_full_vsen_in_cpmtomie_init(FpgaFullVsenState *state);
bool auncient_full_vsen_in_cpmtomie_verify_theorems_1021_1025(FpgaFullVsenState *state);
uint32_t auncient_full_vsen_in_cpmtomie_compute_rule18(const FpgaFullVsenState *state);

#endif /* AUNCIENT_FULL_VSEN_IN_CPMTOMIE_FPGA_THEOREMS_1021_1025_H */
