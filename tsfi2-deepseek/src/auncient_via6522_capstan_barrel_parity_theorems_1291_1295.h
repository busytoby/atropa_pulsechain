#ifndef AUNCIENT_VIA6522_CAPSTAN_BARREL_PARITY_THEOREMS_1291_1295_H
#define AUNCIENT_VIA6522_CAPSTAN_BARREL_PARITY_THEOREMS_1291_1295_H

#include "auncient_lorentz_frame_check_theorems_1286_1290.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* VIA 6522 / Capstan Shaft vs Compact Lorentz Barrel ACID Parity State */
typedef struct {
    uint32_t active_compact_barrel_lanes;      /* 64 concurrent compact Lorentz recorder channels */
    uint32_t bound_acid_parity_slices;        /* 32 ACID equivalence slices in .dat.bin */
    float    via_capstan_barrel_fidelity;      /* 1.000 (Exact ACID parity vs VIA 6522 & capstan shaft) */
    float    compact_acid_latency_ns;          /* Sub-microsecond commit latency (< 1000.0 ns - Rule 11) */
    float    displacement_compact_phase;       /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_compact_barrel_certified;
} ViaCapstanBarrelParityState;

typedef struct {
    float    in_silicon_barrel_parity_fidelity;
    float    barrel_parity_strategy_datbin_merkle_ratio;
    float    barrel_parity_latency_ns;
    uint64_t verified_barrel_parity_saat_clearances;
    bool     barrel_parity_fidelity_verified;        /* Theorem 1291: Compact Lorentz Barrel ACID Operational Invariance */
    bool     barrel_parity_strategy_merkle_verified; /* Theorem 1292: 2-3 Tree AST Merkle Strategy Guard (Rule 13, Rule 21) */
    bool     barrel_parity_submicro_latency_verified;/* Theorem 1293: Sub-Microsecond ACID Latency Guard (Rule 11) */
    bool     barrel_parity_lossless_saat_verified;   /* Theorem 1294: 1.295B Saat Milestone Commutation Flow */
    bool     grand_1295_parity_closure_verified;     /* Theorem 1295: Grand Master Parity Seal */
    uint32_t rule18_parity_checksum;
} BarrelParityBeyond1290State;

void auncient_via_capstan_barrel_parity_init(BarrelParityBeyond1290State *state);
bool auncient_via_capstan_barrel_parity_verify_theorems_1291_1295(BarrelParityBeyond1290State *state);
uint32_t auncient_via_capstan_barrel_parity_compute_rule18(const BarrelParityBeyond1290State *state);

#endif /* AUNCIENT_VIA6522_CAPSTAN_BARREL_PARITY_THEOREMS_1291_1295_H */
