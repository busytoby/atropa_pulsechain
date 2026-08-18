#ifndef AUNCIENT_FPGA_BEYOND_915_THEOREMS_916_920_H
#define AUNCIENT_FPGA_BEYOND_915_THEOREMS_916_920_H

#include "auncient_fpga_beyond_910_theorems_911_915.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* FPGA Secondary & Auxiliary Micro-Core Sub-System Verification State */
typedef struct {
    uint32_t active_aux_subsystems;           /* 7 verified secondary & auxiliary subsystems */
    uint32_t puf_ecdsa_hw_keys;               /* PUF / ECDSA hardware cryptographic keys */
    float    sysmon_sampling_rate_mhz;        /* Mixed-signal SYSMON sampling bandwidth (MHz) */
    float    dfx_bitstream_swap_latency_ns;   /* ICAP Dynamic Function eXchange frame swap latency */
    float    displacement_aux_arch_phase;     /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_aux_arch_certified;
} FpgaAuxiliaryCoreArchitectureState;

typedef struct {
    float    in_silicon_aux_arch_fidelity;
    float    aux_arch_merkle_continuity_ratio;
    float    aux_telemetry_loop_latency_ns;
    uint64_t verified_aux_arch_saat_clearances;
    bool     pmc_power_sysmon_fidelity_verified; /* Theorem 916: PMC & SYSMON In-Silicon Integration */
    bool     smu_crypto_puf_merkle_verified;     /* Theorem 917: SMU Hardware Cryptographic Merkle Guard */
    bool     icap_dfx_hot_swap_latency_verified; /* Theorem 918: ICAP Dynamic Bitstream Sub-Microsecond Latency */
    bool     serdes_dfi_lossless_saat_verified;  /* Theorem 919: SerDes & DFI PHY 920M Saat Lossless Flow */
    bool     grand_920_parity_closure_verified;  /* Theorem 920: Grand Master Parity Seal */
    uint32_t rule18_parity_checksum;
} FpgaBeyond915State;

void auncient_fpga_beyond_915_init(FpgaBeyond915State *state);
bool auncient_fpga_beyond_915_verify_theorems_916_920(FpgaBeyond915State *state);
uint32_t auncient_fpga_beyond_915_compute_rule18(const FpgaBeyond915State *state);

#endif /* AUNCIENT_FPGA_BEYOND_915_THEOREMS_916_920_H */
