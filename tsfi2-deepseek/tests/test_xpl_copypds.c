#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// VTOC allocation structures matching project standard
typedef struct {
    char dataset_name[44];
    uint32_t start_track;
    uint32_t end_track;
    uint32_t block_size;
    uint32_t current_tracks_used;
} vtoc_entry_t;

static vtoc_entry_t mock_vtoc[] = {
    { "SYS1.ISPSLIB",     10,  20,  800,  5 }, // Source (capacity 10 tracks, 5 used)
    { "SYS2.ISPSLIB",     21,  25,  800,  4 }, // Destination 1 (capacity 5 tracks, 4 used)
    { "SYS3.ISPSLIB",     26,  27,  800,  2 }  // Destination 2 (capacity 1 track, 2 used - FULL!)
};

static bool lookup_vtoc(const char *dsn, vtoc_entry_t *out) {
    for (size_t i = 0; i < 3; i++) {
        if (strcmp(mock_vtoc[i].dataset_name, dsn) == 0) {
            *out = mock_vtoc[i];
            return true;
        }
    }
    return false;
}

// 1. RED ANALYZER Gating: Static destination name verification
static bool red_analyzer_validate_member_name(const char *name) {
    if (!name) return false;
    size_t len = strlen(name);
    if (len < 1 || len > 8) return false; // Name must be 1 to 8 characters

    // Must start with an alphabetic character
    if ((name[0] < 'A' || name[0] > 'Z') && (name[0] < 'a' || name[0] > 'z')) {
        return false;
    }
    // Following characters must be alphanumeric
    for (size_t i = 1; i < len; i++) {
        char c = name[i];
        if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true; // Valid PDS member name
}

// 2. BLACK XPLSM Gating: Dynamic copy space verification
static bool black_xplsm_verify_copy_space(
    const char *dest_dsn,
    uint32_t copy_size_tracks,
    uint32_t *interrupt_out
) {
    vtoc_entry_t entry;
    if (!lookup_vtoc(dest_dsn, &entry)) {
        if (interrupt_out) *interrupt_out = 26; // Trap 26: Target dataset not allocated
        return false;
    }

    uint32_t projected_tracks = entry.current_tracks_used + copy_size_tracks;
    uint32_t capacity = entry.end_track - entry.start_track + 1;

    if (projected_tracks > capacity) {
        if (interrupt_out) *interrupt_out = 25; // Trap 25: VTOC Destination Space Overflow
        return false;
    }
    return true; // Space verified
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COPYPDS MEMBER UTILITY VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail member name validation
    printf("[TEST] Verifying destination member name formats...\n");
    assert(red_analyzer_validate_member_name("BACKUP1") == true);
    assert(red_analyzer_validate_member_name("JCLTEST") == true);

    // Rejections: starts with number, invalid symbols, too long
    assert(red_analyzer_validate_member_name("1JCLTEST") == false);
    assert(red_analyzer_validate_member_name("JCL-TEST") == false);
    assert(red_analyzer_validate_member_name("JCL_LONG_NAME") == false);
    printf("   ✓ RED rail successfully verified member name syntax rules.\n");

    // Test 2: BLACK rail copy space checks
    printf("[TEST] Auditing target dataset space against BLACK rail...\n");
    uint32_t trap_code = 0;

    // Successful copy (projected track remains within limit 5)
    assert(black_xplsm_verify_copy_space("SYS2.ISPSLIB", 1, &trap_code) == true);

    // Failed copy due to space overflow (projected tracks 4+2=6 exceeds capacity 5)
    trap_code = 0;
    assert(black_xplsm_verify_copy_space("SYS2.ISPSLIB", 2, &trap_code) == false);
    assert(trap_code == 25); // Trap 25: Space overflow

    // Failed copy due to full destination dataset
    trap_code = 0;
    assert(black_xplsm_verify_copy_space("SYS3.ISPSLIB", 1, &trap_code) == false);
    assert(trap_code == 25); // Trap 25: Space overflow

    printf("   ✓ BLACK rail successfully blocked copying when destination space overflows.\n");

    printf("=============================================================\n");
    printf("ALL COPYPDS MEMBER UTILITY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
