#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// PDS Member representation
typedef struct {
    char member_name[8];
    uint32_t size_bytes;
    bool is_locked;
} pds_member_t;

// Mock PDS Directory (SYS1.ISPSLIB)
static pds_member_t mock_pds[] = {
    { "TASK1",     450, false }, // Normal size skeleton
    { "TASK2",    1200, false }, // Normal size panel
    { "BIGTASK", 68500, false }  // Exceeds 68KB limit!
};
#define PDS_COUNT (sizeof(mock_pds)/sizeof(mock_pds[0]))

// 1. RED ANALYZER Gating: Static 68KB Source File Size Constraint check
static bool red_analyzer_audit_member(const char *member_name) {
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (strcmp(mock_pds[i].member_name, member_name) == 0) {
            // Rule 8: Source files must remain strictly under 68,000 bytes
            if (mock_pds[i].size_bytes >= 68000) {
                return false; // Rejected by RED rail due to size constraint
            }
            return true; // Accepted
        }
    }
    return false; // Member not found
}

// 2. BLACK XPLSM Gating: Concurrent write lock monitor
static bool black_xplsm_acquire_write_lock(const char *member_name, uint32_t *interrupt_out) {
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (strcmp(mock_pds[i].member_name, member_name) == 0) {
            if (mock_pds[i].is_locked) {
                if (interrupt_out) *interrupt_out = 27; // Trap 27: Concurrent write collision
                return false; // Rejected by BLACK rail
            }
            mock_pds[i].is_locked = true; // Lock acquired
            return true;
        }
    }
    return false;
}

static void release_write_lock(const char *member_name) {
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (strcmp(mock_pds[i].member_name, member_name) == 0) {
            mock_pds[i].is_locked = false;
            break;
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ISPF PDS MEMBER DIRECTORY LISTER VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: PDS member properties catalog check
    printf("[TEST] Checking PDS directory catalog entries...\n");
    assert(mock_pds[0].size_bytes == 450);
    assert(mock_pds[2].size_bytes == 68500);
    printf("   ✓ PDS member catalog properties verified.\n");

    // Test 2: RED ANALYZER 68KB Source File Size Constraint Gating
    printf("[TEST] Testing RED rail size constraint gating (68KB limit)...\n");
    // Normal small member passes audit
    assert(red_analyzer_audit_member("TASK1") == true);
    
    // Member exceeding 68,000 bytes is statically blocked
    assert(red_analyzer_audit_member("BIGTASK") == false);
    printf("   ✓ RED rail successfully blocked oversized members (Rule 8 verification).\n");

    // Test 3: BLACK XPLSM Concurrent Write Lock Gating
    printf("[TEST] Evaluating write locks against BLACK rail...\n");
    uint32_t trap_code = 0;

    // Acquire lock on TASK1 successfully
    assert(black_xplsm_acquire_write_lock("TASK1", &trap_code) == true);
    
    // Attempt concurrent lock acquisition on locked TASK1 (expected fail)
    trap_code = 0;
    assert(black_xplsm_acquire_write_lock("TASK1", &trap_code) == false);
    assert(trap_code == 27); // Trap 27: Concurrent write lock collision
    
    // Release and re-acquire
    release_write_lock("TASK1");
    assert(black_xplsm_acquire_write_lock("TASK1", &trap_code) == true);
    printf("   ✓ BLACK rail successfully trapped concurrent write collisions (Trap 27).\n");

    printf("=============================================================\n");
    printf("ALL PDS MEMBER DIRECTORY LISTER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
