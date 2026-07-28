#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// VTOC Record Structure
typedef struct {
    char dataset_name[44];
    uint32_t start_track;
    uint32_t end_track;
    bool is_orphaned;
} vtoc_record_t;

// 1. RED ANALYZER Gating: Static overlap and boundary audit
static bool red_analyzer_audit_vtoc_boundaries(const vtoc_record_t *records, uint32_t count) {
    if (!records || count == 0) return false;

    for (uint32_t i = 0; i < count; i++) {
        // Validate individual range consistency
        if (records[i].start_track > records[i].end_track) {
            return false; // Invalid span (RED rail block)
        }

        // Check against all other records for overlapping spans
        for (uint32_t j = i + 1; j < count; j++) {
            // Overlap check formula: (startA <= endB) && (endA >= startB)
            if (records[i].start_track <= records[j].end_track && 
                records[i].end_track >= records[j].start_track) {
                return false; // Partition overlap conflict detected (RED rail block)
            }
        }
    }
    return true; // Valid clean boundaries
}

// 2. BLACK XPLSM Gating: Dynamic catalog scrubber and binary serialization
static bool black_xplsm_scrub_vtoc(
    vtoc_record_t *records,
    uint32_t count,
    const char *output_dat_bin,
    uint32_t *reclaimed_tracks_out
) {
    if (!records || count == 0 || !output_dat_bin) return false;
    *reclaimed_tracks_out = 0;

    // First perform dynamic reclaim check
    for (uint32_t i = 0; i < count; i++) {
        if (records[i].is_orphaned) {
            uint32_t span = records[i].end_track - records[i].start_track + 1;
            *reclaimed_tracks_out += span;
            
            // Scrub details
            records[i].is_orphaned = false;
            records[i].start_track = 0;
            records[i].end_track = 0;
            strcpy(records[i].dataset_name, "FREE.SPACE");
        }
    }

    // Rule 13: Codebase must only support .dat.bin extensions for block assets on disk
    if (strstr(output_dat_bin, ".dat.bin") == NULL) {
        return false;
    }

    // Serialize cleaned catalog to binary file
    FILE *f = fopen(output_dat_bin, "wb");
    if (!f) return false;

    for (uint32_t i = 0; i < count; i++) {
        fwrite(&records[i], sizeof(vtoc_record_t), 1, f);
    }
    fclose(f);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VTOC UTILITY CATALOG SCRUBBER VALIDATION SUITE\n");
    printf("=============================================================\n");

    const char *bin_path = "vtoc_catalog.dat.bin";

    // Test 1: Valid catalog boundaries
    printf("[TEST] Checking valid non-overlapping dataset track spans...\n");
    vtoc_record_t clean_catalog[] = {
        { "SYS1.LINKLIB",    0,  9, false },
        { "SYS1.ISPSLIB",   10, 20, false },
        { "SYS1.ISPPLIB",   21, 30, true  }  // Orphaned panel workspace
    };
    assert(red_analyzer_audit_vtoc_boundaries(clean_catalog, 3) == true);
    printf("   ✓ Clean boundaries verified successfully.\n");

    // Test 2: Overlapping tracks blocked by RED rail
    printf("[TEST] Testing RED rail track overlap check...\n");
    vtoc_record_t overlapping_catalog[] = {
        { "SYS1.LINKLIB",    0, 15, false }, // Overlaps with ISPSLIB below
        { "SYS1.ISPSLIB",   10, 20, false }
    };
    assert(red_analyzer_audit_vtoc_boundaries(overlapping_catalog, 2) == false);
    printf("   ✓ RED rail successfully blocked catalog overlap conflicts.\n");

    // Test 3: Reclaiming and serialization to .dat.bin
    printf("[TEST] Scrubbing orphaned tracks and serializing to binary file...\n");
    uint32_t reclaimed = 0;
    assert(black_xplsm_scrub_vtoc(clean_catalog, 3, bin_path, &reclaimed) == true);
    assert(reclaimed == 10); // Reclaimed 21 to 30 = 10 tracks
    assert(strcmp(clean_catalog[2].dataset_name, "FREE.SPACE") == 0);
    
    // Verify file output
    FILE *verify = fopen(bin_path, "rb");
    assert(verify != NULL);
    vtoc_record_t read_records[3];
    assert(fread(read_records, sizeof(vtoc_record_t), 3, verify) == 3);
    assert(strcmp(read_records[2].dataset_name, "FREE.SPACE") == 0);
    fclose(verify);
    remove(bin_path);
    printf("   ✓ Scrub completed successfully and serialized to binary .dat.bin.\n");

    printf("=============================================================\n");
    printf("ALL VTOC CATALOG SCRUBBER UTILITY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
