#ifndef TSFI_ZORSE_TOMOGRAPHY_H
#define TSFI_ZORSE_TOMOGRAPHY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_chamberland_duality.h"
#include "tsfi_stekloff_duality.h"

// Zorse Tomographic Slice Audit Struct
typedef struct {
    uint32_t slice_index;               // Tomographic slice sequence number
    uint32_t active_pattern;            // CHAMBERLAND_PATTERN_HYPERTOMOGRAPHY, HYPOTOMOGRAPHY, or DUAL
    uint32_t active_stekloff_mode;      // STEKLOFF_MODE_EPIBAR or STEKLOFF_MODE_HYPOBAR
    float    epibar_val;                // \bar{\epsilon}: Boundary expansion energy
    float    hypobar_val;               // \underline{\epsilon}: Interior compression energy
    float    chamberland_lambda;        // Marc Chamberland Duality Eigenvalue
    float    stekloff_sigma;            // Stekloff ratio relative to Qing domain
    bool     bijective_duality_ok;      // True if epibar \iff hypobar duality holds
} tsfi_zorse_tomography_slice_t;

// Audits a single Zorse GGUF activation vector slice under Tomographic Duality
bool tsfi_zorse_tomography_audit_slice(
    const float *x,
    int dim,
    uint32_t slice_idx,
    float qing_radius,
    tsfi_chamberland_accumulator_t *chamberland_acc,
    tsfi_zorse_tomography_slice_t *slice_out
);

// Writes binary Tomography WAL Receipt (.dat.bin) under Rule 13
bool tsfi_zorse_tomography_write_wal_receipt(
    const char *filepath,
    const tsfi_zorse_tomography_slice_t *slice
);

// Reads and parses binary Tomography WAL Ledger (.dat.bin) under Rule 13
bool tsfi_zorse_tomography_read_wal_ledger(
    const char *filepath,
    tsfi_zorse_tomography_slice_t *slices_out,
    size_t max_slices,
    size_t *slices_read_out
);

#endif // TSFI_ZORSE_TOMOGRAPHY_H
