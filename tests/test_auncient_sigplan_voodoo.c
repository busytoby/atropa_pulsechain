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
    double gamma;                  // Quantization scale factor (Must be > 0.0)
    uint32_t timestamp_ms;         // Timing parameter
} voodoo_cmd_t;

// Simulated SIGPLAN-VOODOO TMU Pipeline State
typedef struct {
    gate_state_t pipeline_gate;
    uint32_t frame_buffer[4];
    uint32_t active_pixel_count;
    uint32_t program_region_level; // Lifespan of active program
    uint32_t target_lock_references; // Linear reference count for pixel target
    uint32_t last_frame_timestamp_ms;
    uint32_t min_frame_delta_ms;
} sigplan_voodoo_tmu_t;

// -------------------------------------------------------------
// Unified SIGPLAN-VOODOO TMU Gating pipeline
// -------------------------------------------------------------
bool dispatch_sigplan_voodoo_tmu(sigplan_voodoo_tmu_t *tmu, const voodoo_cmd_t *cmd) {
    tmu->pipeline_gate = CUTOFF_STATE;

    // 1. Rate-limiting check: Throttles high frame rates
    uint32_t delta = cmd->timestamp_ms - tmu->last_frame_timestamp_ms;
    if (delta < tmu->min_frame_delta_ms) {
        return false; // Throttled
    }

    // 2. Quantization check: Prevent division-by-zero or inversion
    if (cmd->gamma <= 0.0) {
        return false; // Invalid scale factor
    }

    // 3. Bounded Real Check: Coordinates must remain in [0.0, 1.0]
    if (cmd->s < 0.0 || cmd->s > 1.0 || cmd->t < 0.0 || cmd->t > 1.0) {
        return false; // Coordinate escape trapped
    }

    // 4. Region Safety Check: Texture lifespan must contain program lifespan
    if (cmd->texture_region_level > tmu->program_region_level) {
        return false; // Region escape trapped
    }

    // 5. Linearity Check: Target buffer must have exactly one active lock reference
    if (tmu->target_lock_references != 1) {
        return false; // Linearity violation trapped
    }

    // Gating pass: open pipeline and commit pixel color
    tmu->pipeline_gate = CONDUC_STATE;
    tmu->last_frame_timestamp_ms = cmd->timestamp_ms;
    
    uint32_t quantized_color = (uint32_t)(cmd->color / cmd->gamma);
    if (tmu->active_pixel_count < 4) {
        tmu->frame_buffer[tmu->active_pixel_count] = quantized_color;
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
        .target_lock_references = 1,
        .last_frame_timestamp_ms = 1000,
        .min_frame_delta_ms = 16
    };

    // 1. Compliant SIGPLAN Voodoo TMU Command -> Should pass
    voodoo_cmd_t valid_cmd = {
        .s = 0.8,
        .t = 0.2,
        .color = 0xFF8000,
        .texture_region_level = 0, // Global texture
        .gamma = 1.0,
        .timestamp_ms = 1020
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
    bad_coord.timestamp_ms = 1040; // Forward timestamp to pass rate check

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
    bad_region.timestamp_ms = 1060; // Forward timestamp to pass rate check

    printf("[TEST] Dispatching region escape command...\n");
    fflush(stdout);
    ok = dispatch_sigplan_voodoo_tmu(&tmu, &bad_region);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Region escape trapped. TMU pipeline cutoff.\n");
    fflush(stdout);

    // 4. Rate-limiting check: Dispatching too fast (delta = 5ms) -> Should fail
    voodoo_cmd_t fast_cmd = valid_cmd;
    fast_cmd.timestamp_ms = 1025; // 1025 - 1020 (last valid) = 5ms (< 16ms min delta)

    printf("[TEST] Dispatching too-fast command (rate violation)...\n");
    fflush(stdout);
    ok = dispatch_sigplan_voodoo_tmu(&tmu, &fast_cmd);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Throttled frame timing trapped successfully.\n");
    fflush(stdout);

    // 5. Quantization check: division-by-zero scale -> Should fail
    voodoo_cmd_t bad_gamma = valid_cmd;
    bad_gamma.gamma = 0.0;
    bad_gamma.timestamp_ms = 1080; // Forward timestamp

    printf("[TEST] Dispatching invalid quantization scale command...\n");
    fflush(stdout);
    ok = dispatch_sigplan_voodoo_tmu(&tmu, &bad_gamma);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Zero scale quantization hazard trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN VOODOO TMU TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
