#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Dynamic allocation representation
typedef struct {
    char dataset_name[44];
    bool is_allocated;
    bool is_system;
} allocation_entry_t;

// Job queue/spool representation
typedef struct {
    char job_name[16];
    char step_name[16];
    uint32_t net_id;
    bool is_active;
} spool_entry_t;

// Mock allocations table
static allocation_entry_t mock_allocations[] = {
    { "SYS1.LINKLIB",   true,  true  }, // Protected System Library
    { "USER.TEMP.DSN1", true,  false }, // Temporary User Dataset
    { "USER.TEMP.DSN2", true,  false }  // Temporary User Dataset
};
#define ALLOC_COUNT (sizeof(mock_allocations)/sizeof(mock_allocations[0]))

// Mock JCL spool table
#define MAX_SPOOL 4
static spool_entry_t mock_spool[MAX_SPOOL] = {
    { "COMPJCL1", "XCOM1", 42, true  },
    { "COMPJCL2", "XCOM2", 43, true  },
    { "",          "",      0,  false },
    { "",          "",      0,  false }
};

// 1. FREEALL: Free all user allocations while protecting system libraries
static bool execute_freeall(uint32_t *freed_count_out, bool *red_rail_triggered) {
    if (!freed_count_out || !red_rail_triggered) return false;
    *freed_count_out = 0;
    *red_rail_triggered = false;

    for (size_t i = 0; i < ALLOC_COUNT; i++) {
        if (mock_allocations[i].is_allocated) {
            // RED ANALYZER Gating: Protect system datasets (start with SYS1.)
            if (mock_allocations[i].is_system) {
                *red_rail_triggered = true; // RED rail protection activated
                continue; // Skip de-allocation of system files
            }
            mock_allocations[i].is_allocated = false;
            (*freed_count_out)++;
        }
    }
    return true;
}

// 2. DQ: Query spool status and check queue overflow
static bool execute_dq_submit(
    const char *job_name,
    const char *step_name,
    uint32_t net_id,
    uint32_t *interrupt_out
) {
    if (!job_name || !step_name) return false;

    // Check if slot available
    int free_slot = -1;
    for (int i = 0; i < MAX_SPOOL; i++) {
        if (!mock_spool[i].is_active) {
            free_slot = i;
            break;
        }
    }

    // BLACK XPLSM Gating: Spool Queue Overflow Monitor
    if (free_slot == -1) {
        if (interrupt_out) *interrupt_out = 30; // Trap 30: Spool Queue Overflow
        return false; // Rejected by BLACK rail
    }

    // Submit job to spool
    snprintf(mock_spool[free_slot].job_name, sizeof(mock_spool[free_slot].job_name), "%s", job_name);
    snprintf(mock_spool[free_slot].step_name, sizeof(mock_spool[free_slot].step_name), "%s", step_name);
    mock_spool[free_slot].net_id = net_id;
    mock_spool[free_slot].is_active = true;

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSO FILE300 FREEALL AND DQ VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: FREEALL Execution & RED rail protection
    printf("[TEST] Running FREEALL utility to clear user allocations...\n");
    uint32_t freed_count = 0;
    bool red_triggered = false;

    assert(execute_freeall(&freed_count, &red_triggered) == true);
    assert(freed_count == 2); // USER.TEMP.DSN1 and DSN2 freed
    assert(red_triggered == true); // SYS1.LINKLIB was protected
    assert(mock_allocations[0].is_allocated == true); // System dataset remains allocated
    assert(mock_allocations[1].is_allocated == false); // User dataset successfully freed
    printf("   ✓ FREEALL cleared user datasets and protected system libraries (RED rail verified).\n");

    // Test 2: DQ Submission & BLACK rail overflow checks
    printf("[TEST] Submitting JCL tasks to spool queue...\n");
    uint32_t trap_code = 0;

    // Submit to Slot 2 successfully
    assert(execute_dq_submit("COMPJCL3", "XCOM3", 44, &trap_code) == true);
    // Submit to Slot 3 successfully
    assert(execute_dq_submit("COMPJCL4", "XCOM4", 45, &trap_code) == true);

    // Spool is now full (4/4 slots active). Submit attempt 5 should overflow.
    trap_code = 0;
    assert(execute_dq_submit("COMPJCL5", "XCOM5", 46, &trap_code) == false);
    assert(trap_code == 30); // Trap 30: Spool Queue Overflow
    printf("   ✓ BLACK rail successfully trapped spool queue overflow (Trap 30).\n");

    printf("=============================================================\n");
    printf("ALL TSO FILE300 UTILITIES TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
