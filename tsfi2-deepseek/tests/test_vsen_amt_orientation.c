#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[VSEN AMT ORIENTATION & FORBEARANCE VISION] Evaluating Amt orientation and forbearance vision in C...\n");

    // Clean initial test state
    unlink("amt_orientation.dat.bin");
    unlink("amt_orientation.wal.dat.bin");

    // 1. Register Amt orientation record asserting intrinsic definition "amt nit ist ein eisent eih"
    const char *amt_id = "amt_zwingli_forebearance";
    const char *orientation_state = "Orientation upon Amt: Intrinsic property definition established ('amt nit ist ein eisent eih')";
    int forebearance_rating = 10;
    const char *vision = "Forebearance vision: Sustained clean-room C RDBMS execution, zero unverified text alterations, full ACID compliance under Rule 11 & Rule 13";

    assert(tsfi_vsen_amt_register_orientation(amt_id, orientation_state, forebearance_rating, vision) == 0);

    // 2. Perform C RDBMS Lookup over recorded Amt orientation
    vsen_amt_orientation_record_t amt_rec;
    assert(tsfi_vsen_amt_lookup_orientation(amt_id, &amt_rec) == 0);

    printf("\n================ VSEN AMT ORIENTATION & SIGHT VISION REPORT ================\n");
    printf("  Amt Record ID:              %s\n", amt_rec.amt_id);
    printf("  Orientation State:          %s\n", amt_rec.orientation);
    printf("  Intrinsic Definition Sound: %s\n", amt_rec.is_eisent_eih_intrinsic ? "YES (amt nit ist ein eisent eih)" : "NO");
    printf("  Forbearance Factor:         %d / 10\n", amt_rec.forebearance_factor);
    printf("  Sight Vision Summary:       %s\n", amt_rec.vision_summary);

    // Assert key structural invariants
    assert(strcmp(amt_rec.amt_id, amt_id) == 0);
    assert(amt_rec.is_eisent_eih_intrinsic == 1);
    assert(amt_rec.forebearance_factor == 10);
    assert(strstr(amt_rec.orientation, "amt nit ist ein eisent eih") != NULL);

    // 3. Audit Cryptographic DNA Hash Chain under Rule 11 & Rule 13
    assert(tsfi_vsen_audit_chain_verify("amt_orientation.dat.bin") == 0);

    printf("\n[VSEN AMT ORIENTATION & FORBEARANCE VISION] Sight vision test completed successfully with ZERO errors!\n");
    return 0;
}
