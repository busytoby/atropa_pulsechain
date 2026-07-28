#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// PDS Member representation
typedef struct {
    char member_name[16];
    uint32_t track_size;
    bool is_locked;
    bool is_active;
} pds_member_t;

// Mock VTOC and PDS Directory (SYS1.ISPSLIB)
static uint32_t vt_fragmented_tracks = 0;
static pds_member_t mock_pds[] = {
    { "TASK1",  2, false, true },
    { "TASK2",  1, false, true }
};
#define PDS_COUNT (sizeof(mock_pds)/sizeof(mock_pds[0]))

// 1. RED ANALYZER Gating: Static name and existence checks
static bool red_analyzer_validate_rename(const char *old_name, const char *new_name) {
    if (!old_name || !new_name) return false;

    // Check old member existence
    bool old_exists = false;
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (mock_pds[i].is_active && strcmp(mock_pds[i].member_name, old_name) == 0) {
            old_exists = true;
            break;
        }
    }
    if (!old_exists) return false; // Target not found (static rejection)

    // Check new name formatting
    size_t len = strlen(new_name);
    if (len < 1 || len > 8) return false;

    if ((new_name[0] < 'A' || new_name[0] > 'Z') && (new_name[0] < 'a' || new_name[0] > 'z')) {
        return false;
    }
    for (size_t i = 1; i < len; i++) {
        char c = new_name[i];
        if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true; // Valid rename format and target found
}

// 2. BLACK XPLSM Gating: Rename/Delete execution and lock/gas tracking
static bool black_xplsm_execute_delete(const char *member_name) {
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (mock_pds[i].is_active && strcmp(mock_pds[i].member_name, member_name) == 0) {
            // Reclaim locks and mark active state as false
            mock_pds[i].is_locked = false;
            mock_pds[i].is_active = false;
            
            // Allocate deleted track size to VTOC fragmented space ("gas")
            vt_fragmented_tracks += mock_pds[i].track_size;
            return true;
        }
    }
    return false; // Member not found
}

static bool black_xplsm_execute_rename(const char *old_name, const char *new_name) {
    for (size_t i = 0; i < PDS_COUNT; i++) {
        if (mock_pds[i].is_active && strcmp(mock_pds[i].member_name, old_name) == 0) {
            snprintf(mock_pds[i].member_name, sizeof(mock_pds[i].member_name), "%s", new_name);
            return true;
        }
    }
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RENEL MEMBER RENAME/DELETE VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail static rename validation
    printf("[TEST] Verifying rename properties and target states...\n");
    // Valid rename of existing member
    assert(red_analyzer_validate_rename("TASK1", "NEWTASK") == true);
    
    // Invalid rename: target does not exist
    assert(red_analyzer_validate_rename("NOTFOUND", "NEWTASK") == false);
    
    // Invalid rename: malformed destination name
    assert(red_analyzer_validate_rename("TASK1", "1NEWTASK") == false);
    printf("   ✓ RED rail successfully validated rename targets and new name constraints.\n");

    // Test 2: BLACK rail member deletion and VTOC fragmentation logging
    printf("[TEST] Deleting PDS members and auditing space fragmentation...\n");
    assert(mock_pds[0].is_active == true);
    assert(vt_fragmented_tracks == 0);

    // Delete TASK1 (size 2 tracks)
    assert(black_xplsm_execute_delete("TASK1") == true);
    assert(mock_pds[0].is_active == false); // Deleted
    assert(vt_fragmented_tracks == 2);      // 2 tracks marked as gas/fragmented
    printf("   ✓ BLACK rail successfully tracked deletion and logged fragmented space.\n");

    // Test 3: BLACK rail member rename execution
    printf("[TEST] Renaming active members...\n");
    assert(mock_pds[1].is_active == true);
    assert(strcmp(mock_pds[1].member_name, "TASK2") == 0);

    assert(black_xplsm_execute_rename("TASK2", "NEWT2") == true);
    assert(strcmp(mock_pds[1].member_name, "NEWT2") == 0);
    printf("   ✓ BLACK rail successfully executed rename operations.\n");

    printf("=============================================================\n");
    printf("ALL RENEL MEMBER UTILITY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
