#ifndef AUNCIENT_FPGA_BEYOND_920_THEOREMS_921_925_H
#define AUNCIENT_FPGA_BEYOND_920_THEOREMS_921_925_H

#include "auncient_fpga_beyond_915_theorems_916_920.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* CPMTomie OS .dat.bin Binary File Header & Quadtree Slicing Constants (Rule 13) */
#define CPMTOMIE_BIN_MAGIC 0x43504D54 /* 'CPMT' in ASCII */
#define CPMTOMIE_BIN_VERSION 2

typedef struct {
    uint32_t magic;                          /* CPMTOMIE_BIN_MAGIC (Rule 13: .dat.bin only) */
    uint32_t version;                        /* CPMTOMIE_BIN_VERSION */
    uint32_t tpa_load_address;               /* CP/M TPA Base Address (0x0100 / 256) */
    uint32_t std_strategy_slice_offset;      /* Offset in .dat.bin for COBOL strategy / ALGOL std */
    uint32_t aux_process_binary_offset;      /* Offset in .dat.bin for Auxiliary Process .BIN */
    uint32_t in_machine_proof_witness;       /* Embedded In-Machine ALGOL 61 proof witness */
    uint32_t rule18_parity_crc;              /* Non-preferential 3-term recurrence parity (Rule 18) */
} CpmTomieBinHeader;

/* CPMTomie OS .dat.bin In-Machine Quadtree & Auxiliary Process Runtime State */
typedef struct {
    uint32_t dat_bin_total_slices;           /* Quadtree .dat.bin ledger database slices */
    uint32_t active_cpmtomie_bin_binaries;   /* Active .BIN executable processes in TPA */
    float    dat_bin_read_latency_ns;        /* Direct DMA .dat.bin slice read latency */
    float    in_machine_proof_exec_ns;       /* In-machine proof evaluation latency */
    float    displacement_cpm_bin_phase;     /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_cpmtomie_bin_certified;
} CpmTomieDatBinProcessState;

typedef struct {
    float    in_silicon_dat_bin_fidelity;
    float    cpmtomie_bin_merkle_continuity_ratio;
    float    cpm_dma_exec_latency_ns;
    uint64_t verified_dat_bin_saat_clearances;
    bool     dat_bin_format_rule13_verified;      /* Theorem 921: Rule 13 .dat.bin Quadtree Standard */
    bool     in_machine_proof_merkle_verified;    /* Theorem 922: Embedded In-Machine Proof Invariance */
    bool     cpm_tpa_bin_dispatch_latency_verified;/* Theorem 923: Sub-Microsecond .BIN Dispatch */
    bool     cpmtomie_aux_lossless_saat_verified; /* Theorem 924: 925M Saat Commutation Flow */
    bool     grand_925_parity_closure_verified;   /* Theorem 925: Grand Master Parity Seal */
    uint32_t rule18_parity_checksum;
} FpgaBeyond920State;

void auncient_fpga_beyond_920_init(FpgaBeyond920State *state);
bool auncient_fpga_beyond_920_verify_theorems_921_925(FpgaBeyond920State *state);
uint32_t auncient_fpga_beyond_920_compute_rule18(const FpgaBeyond920State *state);

#endif /* AUNCIENT_FPGA_BEYOND_920_THEOREMS_921_925_H */
