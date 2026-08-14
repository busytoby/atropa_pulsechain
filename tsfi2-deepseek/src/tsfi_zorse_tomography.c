#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_zorse_tomography.h"

bool tsfi_zorse_tomography_audit_slice(
    const float *x,
    int dim,
    uint32_t slice_idx,
    float qing_radius,
    tsfi_chamberland_accumulator_t *chamberland_acc,
    tsfi_zorse_tomography_slice_t *slice_out
) {
    if (!x || dim <= 0 || !slice_out) return false;

    slice_out->slice_index = slice_idx;

    // Update Chamberland epibar / hypobar accumulators
    if (chamberland_acc) {
        tsfi_chamberland_accumulator_update(chamberland_acc, x, dim, 0.25f, 0.25f);
        slice_out->epibar_val = chamberland_acc->epibar_accumulator;
        slice_out->hypobar_val = chamberland_acc->hypobar_accumulator;
        slice_out->chamberland_lambda = chamberland_acc->dual_eigenvalue;
        slice_out->active_pattern = chamberland_acc->selection_pattern;
    } else {
        slice_out->epibar_val = fabsf(x[0]);
        slice_out->hypobar_val = fabsf(x[dim / 2]);
        slice_out->chamberland_lambda = 1.0f;
        slice_out->active_pattern = CHAMBERLAND_PATTERN_BIJECTION_DUAL;
    }

    // Evaluate Stekloff mode relative to Qing reference domain
    tsfi_stekloff_state_t stekloff;
    if (tsfi_stekloff_eval_eigenvalue_qing(x, dim, qing_radius, &stekloff)) {
        slice_out->stekloff_sigma = stekloff.stekloff_sigma;
        slice_out->active_stekloff_mode = stekloff.active_stekloff_mode;
    } else {
        slice_out->stekloff_sigma = 1.0f;
        slice_out->active_stekloff_mode = STEKLOFF_MODE_HYPOBAR;
    }

    // Assert Bijective Duality condition: Epibar \iff Hypobar
    slice_out->bijective_duality_ok = (slice_out->chamberland_lambda > 0.01f);
    return true;
}

bool tsfi_zorse_tomography_write_wal_receipt(
    const char *filepath,
    const tsfi_zorse_tomography_slice_t *slice
) {
    if (!filepath || !slice) return false;
    FILE *f = fopen(filepath, "ab");
    if (!f) return false;

    // Write binary WAL receipt header under Rule 13 (.dat.bin layout)
    uint32_t magic = 0x544F4D4F; // "TOMO" Binary Magic
    fwrite(&magic, sizeof(uint32_t), 1, f);
    fwrite(slice, sizeof(tsfi_zorse_tomography_slice_t), 1, f);
    fclose(f);
    return true;
}

bool tsfi_zorse_tomography_read_wal_ledger(
    const char *filepath,
    tsfi_zorse_tomography_slice_t *slices_out,
    size_t max_slices,
    size_t *slices_read_out
) {
    if (!filepath || !slices_out || max_slices == 0) return false;
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    size_t count = 0;
    while (count < max_slices) {
        uint32_t magic = 0;
        if (fread(&magic, sizeof(uint32_t), 1, f) != 1) break;
        if (magic != 0x544F4D4F) break; // Verify "TOMO" Binary Magic under Rule 13

        if (fread(&slices_out[count], sizeof(tsfi_zorse_tomography_slice_t), 1, f) != 1) break;
        count++;
    }
    fclose(f);
    if (slices_read_out) *slices_read_out = count;
    return (count > 0);
}
