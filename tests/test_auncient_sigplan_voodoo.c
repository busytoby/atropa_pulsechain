#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Voodoo Vertex Command
typedef struct {
    double s;
    double t;
    uint32_t color;
    uint32_t texture_region_level; // Lifespan of texture source
} voodoo_cmd_t;

// Simulated SIGPLAN-VOODOO TMU Pipeline State
typedef struct {
    gate_state_t pipeline_gate;
    uint32_t frame_buffer[4];
    uint32_t active_pixel_count;
    uint32_t program_region_level; // Lifespan of active program
    uint32_t target_lock_references; // Linear reference count for pixel target
} sigplan_voodoo_tmu_t;

// -------------------------------------------------------------
// Unified SIGPLAN-VOODOO TMU Gating pipeline
// -------------------------------------------------------------
bool dispatch_sigplan_voodoo_tmu(sigplan_voodoo_tmu_t *tmu, const voodoo_cmd_t *cmd) {
    tmu->pipeline_gate = CUTOFF_STATE;

    // 1. Bounded Real Check: Coordinates must remain in [0.0, 1.0]
    if (cmd->s < 0.0 || cmd->s > 1.0 || cmd->t < 0.0 || cmd->t > 1.0) {
        return false; // Coordinate escape trapped
    }

    // 2. Region Safety Check: Texture lifespan must contain program lifespan
    if (cmd->texture_region_level > tmu->program_region_level) {
        return false; // Region escape trapped
    }

    // 3. Linearity Check: Target buffer must have exactly one active lock reference
    if (tmu->target_lock_references != 1) {
        return false; // Linearity violation trapped
    }

    // Gating pass: open pipeline and commit pixel color
    tmu->pipeline_gate = CONDUC_STATE;
    if (tmu->active_pixel_count < 4) {
        tmu->frame_buffer[tmu->active_pixel_count] = cmd->color;
        tmu->active_pixel_count++;
    }
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN VOODOO TMU VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize pipeline state
    sigplan_voodoo_tmu_t tmu = {
        .pipeline_gate = CUTOFF_STATE,
        .frame_buffer = { 0 },
        .active_pixel_count = 0,
        .program_region_level = 1,
        .target_lock_references = 1
    };

    // 1. Compliant SIGPLAN Voodoo TMU Command -> Should pass
    voodoo_cmd_t valid_cmd = {
        .s = 0.8,
        .t = 0.2,
        .color = 0xFF8000,
        .texture_region_level = 0 // Global texture
    };

    printf("[TEST] Dispatching compliant SIGPLAN Voodoo TMU command...\n");
    fflush(stdout);
    bool ok = dispatch_sigplan_voodoo_tmu(&tmu, &valid_cmd);
    assert(ok == true);
    assert(tmu.pipeline_gate == CONDUC_STATE);
    assert(tmu.frame_buffer[0] == 0xFF8000);
    printf("   ✓ Pipeline approved. Conduction open, pixel color written.\n");
    fflush(stdout);

    // 2. Non-compliant Command: Coordinate escape -> Should fail
    voodoo_cmd_t bad_coord = valid_cmd;
    bad_coord.s = 1.1;

    printf("[TEST] Dispatching coordinate escape command...\n");
    fflush(stdout);
    ok = dispatch_sigplan_voodoo_tmu(&tmu, &bad_coord);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Coordinate escape trapped. TMU pipeline cutoff.\n");
    fflush(stdout);

    // 3. Non-compliant Command: Region escape -> Should fail
    voodoo_cmd_t bad_region = valid_cmd;
    bad_region.texture_region_level = 2; // Shorter lifespan than program

    printf("[TEST] Dispatching region escape command...\n");
    fflush(stdout);
    ok = dispatch_sigplan_voodoo_tmu(&tmu, &bad_region);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Region escape trapped. TMU pipeline cutoff.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN VOODOO TMU TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
