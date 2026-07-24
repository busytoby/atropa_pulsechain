#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// SIGPLAN TMU Gating Audit State
typedef struct {
    double s;
    double t;
    uint32_t texture_region_level;  // Lifespan level of texture resource
    uint32_t program_region_level;  // Lifespan level of active guest program
    uint32_t pixel_lock_references; // Linearity reference check (Must be 1)
} sigplan_tmu_audit_t;

// -------------------------------------------------------------
// SIGPLAN TMU Auditor
// -------------------------------------------------------------
bool audit_sigplan_tmu(const sigplan_tmu_audit_t *audit) {
    // 1. Bounded Real Check: Coordinates must remain in [0.0, 1.0]
    if (audit->s < 0.0 || audit->s > 1.0 || audit->t < 0.0 || audit->t > 1.0) {
        return false; // Coordinate escape caught
    }

    // 2. Region Check: Texture lifespan must contain program lifespan
    // (A program cannot read a texture that is deallocated earlier)
    if (audit->texture_region_level > audit->program_region_level) {
        return false; // Lifetime containment violation
    }

    // 3. Linear Check: Pixel target must have exactly one active reference lock
    if (audit->pixel_lock_references != 1) {
        return false; // Aliasing or leak violation
    }

    return true; // Audit approved
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN TMU GATING AND MEMORY AUDITOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant TMU Gating -> Should pass
    sigplan_tmu_audit_t safe_audit = {
        .s = 0.75,
        .t = 0.25,
        .texture_region_level = 0, // Global texture (Level 0)
        .program_region_level = 1, // Local program (Level 1)
        .pixel_lock_references = 1 // Exactly one write owner
    };

    printf("[TEST] Auditing compliant TMU pipeline configuration...\n");
    fflush(stdout);
    bool ok = audit_sigplan_tmu(&safe_audit);
    assert(ok == true);
    printf("   ✓ Pipeline configuration approved by SIGPLAN safety metrics.\n");
    fflush(stdout);

    // 2. Escape violation: Program (Level 0) tries to use transient texture (Level 1)
    sigplan_tmu_audit_t region_escape = safe_audit;
    region_escape.texture_region_level = 1;
    region_escape.program_region_level = 0;

    printf("[TEST] Auditing transient texture region escape...\n");
    fflush(stdout);
    ok = audit_sigplan_tmu(&region_escape);
    assert(ok == false);
    printf("   ✓ Region escape violation caught successfully.\n");
    fflush(stdout);

    // 3. Linearity violation: Multiple TMU threads claim pixel target
    sigplan_tmu_audit_t duplicate_lock = safe_audit;
    duplicate_lock.pixel_lock_references = 2;

    printf("[TEST] Auditing duplicate pixel ownership lock...\n");
    fflush(stdout);
    ok = audit_sigplan_tmu(&duplicate_lock);
    assert(ok == false);
    printf("   ✓ Pixel ownership duplication trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN TMU AUDITOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
