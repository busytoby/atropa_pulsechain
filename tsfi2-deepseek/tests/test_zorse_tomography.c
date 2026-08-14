#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_zorse_tomography.h"

int main(void) {
    printf("================ TEST ZORSE TOMOGRAPHY STUDIES MODULE ================\n");

    tsfi_chamberland_accumulator_t *acc = tsfi_chamberland_accumulator_create();
    assert(acc != NULL);

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.25f;

    tsfi_zorse_tomography_slice_t slice;
    bool ok_audit = tsfi_zorse_tomography_audit_slice(x_sample, 16, 1, 1.25f, acc, &slice);
    assert(ok_audit);
    assert(slice.slice_index == 1);
    assert(slice.bijective_duality_ok);

    printf("  [PASS] Zorse Tomography Slice 1 audited:\n");
    printf("         Pattern = 0x%08X, StekloffMode = 0x%08X\n", slice.active_pattern, slice.active_stekloff_mode);
    printf("         Epibar = %.4f, Hypobar = %.4f, LambdaDual = %.4f\n", slice.epibar_val, slice.hypobar_val, slice.chamberland_lambda);

    // Test writing binary WAL receipt under Rule 13 (.dat.bin layout)
    const char *wal_path = "zorse_tomography_test.dat.bin";
    remove(wal_path); // Clean previous test file if exists
    bool ok_wal = tsfi_zorse_tomography_write_wal_receipt(wal_path, &slice);
    assert(ok_wal);
    printf("  [PASS] Binary Tomography WAL Receipt committed to %s under Rule 13.\n", wal_path);

    // Test reading binary WAL ledger under Rule 13 (.dat.bin layout)
    tsfi_zorse_tomography_slice_t read_slices[4];
    size_t read_count = 0;
    bool ok_read = tsfi_zorse_tomography_read_wal_ledger(wal_path, read_slices, 4, &read_count);
    assert(ok_read);
    assert(read_count == 1);
    assert(read_slices[0].slice_index == 1);
    printf("  [PASS] Binary Tomography WAL Ledger parsed from %s under Rule 13 (count = %zu).\n", wal_path, read_count);

    tsfi_chamberland_accumulator_destroy(acc);
    printf("======================================================================\n");
    return 0;
}
