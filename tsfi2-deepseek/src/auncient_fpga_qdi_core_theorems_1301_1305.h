#ifndef AUNCIENT_FPGA_QDI_CORE_THEOREMS_1301_1305_H
#define AUNCIENT_FPGA_QDI_CORE_THEOREMS_1301_1305_H

#include "auncient_qdi_multiradix_barrel_theorems_1296_1300.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* FPGA Dual-Rail QDI Asynchronous Micro-Processor Core State */
typedef struct {
    uint32_t active_qdi_core_lanes;            /* 64 dual-rail QDI asynchronous processor lanes */
    uint32_t bound_muller_c_element_slices;    /* 32 Müller C-element completion grid slices in .dat.bin */
    float    qdi_core_fidelity;                /* 1.000 (Exact dual-rail 4-phase RTZ asynchronous execution fidelity) */
    float    qdi_handshake_latency_ns;         /* Sub-microsecond handshaking latency (< 1000.0 ns - Rule 11) */
    float    displacement_qdi_core_phase;      /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_qdi_core_certified;
} FpgaQdiCoreState;

typedef struct {
    float    in_silicon_qdi_core_fidelity;
    float    qdi_core_strategy_datbin_merkle_ratio;
    float    qdi_core_latency_ns;
    uint64_t verified_qdi_core_saat_clearances;
    bool     qdi_core_fidelity_verified;        /* Theorem 1301: QDI Core Operational Invariance */
    bool     qdi_core_strategy_merkle_verified; /* Theorem 1302: 2-3 Tree AST Merkle Strategy Guard (Rule 13, Rule 21) */
    bool     qdi_core_submicro_latency_verified;/* Theorem 1303: Sub-Microsecond Handshake Latency Guard (Rule 11) */
    bool     qdi_core_lossless_saat_verified;   /* Theorem 1304: 1.305B Saat Milestone Commutation Flow */
    bool     grand_1305_parity_closure_verified; /* Theorem 1305: Grand Master Parity Seal */
    uint32_t rule18_parity_checksum;
} FpgaQdiCoreBeyond1300State;

void auncient_fpga_qdi_core_init(FpgaQdiCoreBeyond1300State *state);
bool auncient_fpga_qdi_core_verify_theorems_1301_1305(FpgaQdiCoreBeyond1300State *state);
uint32_t auncient_fpga_qdi_core_compute_rule18(const FpgaQdiCoreBeyond1300State *state);

#endif /* AUNCIENT_FPGA_QDI_CORE_THEOREMS_1301_1305_H */
