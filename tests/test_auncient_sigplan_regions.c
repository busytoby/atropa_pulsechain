#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Regions are defined by nesting level (0 = global/outermost, higher = deeper/shorter lifespan)
typedef struct {
    uint32_t region_id;
    uint32_t nesting_level;
} sigplan_region_t;

// Pointer/Reference assignment in region-based memory
typedef struct {
    uint32_t target_region_level; // Lifetime of target reference pointer
    uint32_t source_region_level; // Lifetime of assigned value
} region_assignment_t;

// -------------------------------------------------------------
// Region-Based Memory Safety Check (SIGPLAN region rules)
// -------------------------------------------------------------
bool verify_region_assignment_safety(const region_assignment_t *assign) {
    // Safety Rule: A pointer residing in an outer region (longer lifespan, lower level)
    // cannot point to a value in a nested region (shorter lifespan, higher level).
    // Doing so results in a dangling reference when the inner region is deallocated.
    if (assign->source_region_level > assign->target_region_level) {
        return false; // Escape violation!
    }
    return true; // Safe region assignment
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN REGION-BASED MEMORY AUDITOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Safe assignment: Nested pointer (Level 1) pointing to Global value (Level 0)
    region_assignment_t safe_assign = {
        .target_region_level = 1,
        .source_region_level = 0
    };

    printf("[TEST] Checking safe region assignment...\n");
    fflush(stdout);
    bool ok = verify_region_assignment_safety(&safe_assign);
    assert(ok == true);
    printf("   ✓ Assignment approved. Lifetime containment holds.\n");
    fflush(stdout);

    // 2. Escape assignment: Global pointer (Level 0) pointing to Local value (Level 1) -> Violation
    region_assignment_t escape_assign = {
        .target_region_level = 0,
        .source_region_level = 1
    };

    printf("[TEST] Checking region escape assignment (Global <- Local)...\n");
    fflush(stdout);
    ok = verify_region_assignment_safety(&escape_assign);
    assert(ok == false); // Caught dangling pointer risk
    printf("   ✓ Region escape violation trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN REGIONS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
