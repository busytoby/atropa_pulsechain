#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// VTOC Dataset Allocation Entry Structure
typedef struct {
    char dataset_name[44];
    uint32_t start_track;
    uint32_t end_track;
    uint32_t block_size;
    char record_format[4];
} vtoc_entry_t;

// Mock Volume Table of Contents (VTOC) database
static vtoc_entry_t mock_vtoc[] = {
    { "SYS1.LINKLIB",      0,   9, 6144, "U"   }, // Restricted Supervisor Zone
    { "SYS1.ISPSLIB",     10,  20,  800, "FB"  }, // Skeletons Library
    { "SYS1.ISPPLIB",     21,  30,  800, "FB"  }, // Panels Library
    { "CBTCOV.FILE012",   31,  50, 3120, "FB"  }  // UK Guide Tape Library
};
#define VTOC_COUNT (sizeof(mock_vtoc)/sizeof(mock_vtoc[0]))

// 1. VTOC Directory Query (TSO Command emulation)
static bool query_vtoc_dataset(const char *dsn, vtoc_entry_t *out) {
    if (!dsn || !out) return false;
    for (size_t i = 0; i < VTOC_COUNT; i++) {
        if (strcmp(mock_vtoc[i].dataset_name, dsn) == 0) {
            *out = mock_vtoc[i];
            return true;
        }
    }
    return false;
}

// 2. RED ANALYZER Gating: Static partition range audit
static bool red_analyzer_audit_vtoc(const char *dsn, uint32_t target_track) {
    vtoc_entry_t entry;
    if (!query_vtoc_dataset(dsn, &entry)) {
        return false; // Dataset not allocated in VTOC (Static Rejection)
    }

    // Prohibit writes to restricted Supervisor tracks (0..9)
    if (target_track < 10) {
        return false; // Threat detected (RED rail active-low failure)
    }

    // Verify track bounds
    if (target_track < entry.start_track || target_track > entry.end_track) {
        return false; // Out-of-bounds partition write (RED rail failure)
    }

    return true; // Audited successfully
}

// 3. BLACK XPLSM Gating: Dynamic track allocation overflow monitor
static bool black_xplsm_monitor_vtoc(const char *dsn, uint32_t current_track, uint32_t *interrupt_out) {
    vtoc_entry_t entry;
    if (!query_vtoc_dataset(dsn, &entry)) {
        if (interrupt_out) *interrupt_out = 26; // Trap 26: Dataset allocation missing
        return false;
    }

    // Verify dynamic track overflow limit
    if (current_track > entry.end_track) {
        if (interrupt_out) *interrupt_out = 25; // Trap 25: VTOC Allocation Space Overflow
        return false; // Active-high validation failure
    }

    return true; // Validation satisfied
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS VTOC METADATA REGISTRY VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: VTOC TSO Command Directory Queries
    printf("[TEST] Querying volume datasets via VTOC catalog...\n");
    vtoc_entry_t entry;
    assert(query_vtoc_dataset("SYS1.ISPSLIB", &entry) == true);
    assert(entry.start_track == 10 && entry.end_track == 20);
    assert(strcmp(entry.record_format, "FB") == 0);

    assert(query_vtoc_dataset("CBTCOV.FILE012", &entry) == true);
    assert(entry.block_size == 3120);

    assert(query_vtoc_dataset("INVALID.DSN", &entry) == false);
    printf("   ✓ VTOC dataset directory properties queried successfully.\n");

    // Test 2: RED ANALYZER Static Gating
    printf("[TEST] Auditing static track writes against RED rail...\n");
    // Normal valid write within allocated tracks
    assert(red_analyzer_audit_vtoc("SYS1.ISPSLIB", 15) == true);

    // Attempted write to restricted SYS1.LINKLIB track (Supervisor zone)
    assert(red_analyzer_audit_vtoc("SYS1.LINKLIB", 5) == false);

    // Attempted out-of-bounds write to unallocated track 25 for ISPSLIB
    assert(red_analyzer_audit_vtoc("SYS1.ISPSLIB", 25) == false);
    printf("   ✓ RED rail successfully blocked unauthorized and unallocated partition writes.\n");

    // Test 3: BLACK XPLSM Dynamic Gating & Space Overflow Traps
    printf("[TEST] Evaluating dynamic allocations against BLACK rail...\n");
    uint32_t trap_code = 0;

    // Normal execution within bounds
    assert(black_xplsm_monitor_vtoc("SYS1.ISPPLIB", 25, &trap_code) == true);

    // Write overflow (exceeds end_track 30 for ISPPLIB)
    trap_code = 0;
    assert(black_xplsm_monitor_vtoc("SYS1.ISPPLIB", 31, &trap_code) == false);
    assert(trap_code == 25); // Trap 25: Space overflow
    printf("   ✓ BLACK rail successfully trapped dynamic track allocation overflow (Trap 25).\n");

    printf("=============================================================\n");
    printf("ALL VTOC REGISTRY METADATA TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
