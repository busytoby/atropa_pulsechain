#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// VTOC structures matching project standard
typedef struct {
    char dataset_name[44];
    char record_format[4];
    uint32_t start_track;
    uint32_t end_track;
    uint32_t current_tracks_used;
    uint32_t fragmented_tracks; // "gas" from deleted members
} vtoc_entry_t;

static vtoc_entry_t mock_vtoc[] = {
    { "SYS1.ISPSLIB", "FB", 10, 20, 5, 2 }, // Partitioned (5 tracks used, 2 tracks gas)
    { "SYS1.SEQFILE", "PS", 21, 25, 4, 0 }  // Sequential (Not eligible for compression)
};

static bool lookup_vtoc(const char *dsn, vtoc_entry_t **out) {
    for (size_t i = 0; i < 2; i++) {
        if (strcmp(mock_vtoc[i].dataset_name, dsn) == 0) {
            *out = &mock_vtoc[i];
            return true;
        }
    }
    return false;
}

// 1. RED ANALYZER Gating: Static dataset format checks
static bool red_analyzer_validate_compress_target(const char *dsn) {
    vtoc_entry_t *entry = NULL;
    if (!lookup_vtoc(dsn, &entry)) {
        return false; // Target not found (static rejection)
    }

    // Only Partitioned Datasets (non-PS) can be compressed to reclaim member gas
    if (strcmp(entry->record_format, "PS") == 0) {
        return false; // Sequential dataset rejected by RED rail
    }

    return true; // Valid partitioned dataset
}

// 2. BLACK XPLSM Gating: Reclaim operation and VTOC update
static bool black_xplsm_execute_compress(const char *dsn, uint32_t *reclaimed_tracks_out) {
    vtoc_entry_t *entry = NULL;
    if (!lookup_vtoc(dsn, &entry)) return false;

    if (entry->fragmented_tracks == 0) {
        *reclaimed_tracks_out = 0;
        return true; // Nothing to reclaim, but operation succeeded
    }

    // Dynamically reclaim fragmented tracks
    uint32_t gas = entry->fragmented_tracks;
    entry->current_tracks_used -= gas;
    entry->fragmented_tracks = 0;
    *reclaimed_tracks_out = gas;

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDS COMPRESS UTILITY VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail target verification
    printf("[TEST] Verifying dataset types for compression...\n");
    // SYS1.ISPSLIB is partitioned (FB format) - accepted
    assert(red_analyzer_validate_compress_target("SYS1.ISPSLIB") == true);
    
    // SYS1.SEQFILE is sequential (PS format) - rejected by RED rail
    assert(red_analyzer_validate_compress_target("SYS1.SEQFILE") == false);
    
    assert(red_analyzer_validate_compress_target("INVALID.DSN") == false);
    printf("   ✓ RED rail successfully gated compression targets based on record format.\n");

    // Test 2: BLACK rail space reclamation
    printf("[TEST] Reclaiming fragmented blocks against BLACK rail...\n");
    vtoc_entry_t *entry = NULL;
    assert(lookup_vtoc("SYS1.ISPSLIB", &entry) == true);
    assert(entry->current_tracks_used == 5);
    assert(entry->fragmented_tracks == 2);

    uint32_t reclaimed = 0;
    assert(black_xplsm_execute_compress("SYS1.ISPSLIB", &reclaimed) == true);
    assert(reclaimed == 2);
    assert(entry->current_tracks_used == 3); // 5 - 2 = 3 tracks used now
    assert(entry->fragmented_tracks == 0);   // Fragmented space cleared
    printf("   ✓ BLACK rail successfully compressed dataset and updated VTOC track records.\n");

    printf("=============================================================\n");
    printf("ALL PDS COMPRESS UTILITY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
