#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Voodoo TMU Command Packets
typedef struct {
    double s; // Texture coordinate U/S (0.0 - 1.0)
    double t; // Texture coordinate V/T (0.0 - 1.0)
    uint32_t color;
} voodoo_vertex_t;

// Voodoo TMU Pipeline state
typedef struct {
    gate_state_t pipeline_gate;
    uint32_t frame_buffer[4];
    uint32_t active_pixel_count;
} voodoo_tmu_t;

// -------------------------------------------------------------
// Voodoo TMU Coordinate Gating (Jin Li scheduling bounds)
// -------------------------------------------------------------
bool process_voodoo_vertex(voodoo_tmu_t *tmu, const voodoo_vertex_t *vtx) {
    tmu->pipeline_gate = CUTOFF_STATE;

    // Jin Li guard condition: Coordinates must be normalized (0.0 <= s, t <= 1.0)
    // Values outside this range represent out-of-bound TMU mapping attempts
    if (vtx->s < 0.0 || vtx->s > 1.0 || vtx->t < 0.0 || vtx->t > 1.0) {
        return false; // Coordinate escape trapped. TMU cutoff.
    }

    // Gating pass: open pipeline and commit pixel color
    tmu->pipeline_gate = CONDUC_STATE;
    if (tmu->active_pixel_count < 4) {
        tmu->frame_buffer[tmu->active_pixel_count] = vtx->color;
        tmu->active_pixel_count++;
    }
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VOODOO TMU COAXIAL GRAPHICS PIPELINE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    voodoo_tmu_t tmu = {
        .pipeline_gate = CUTOFF_STATE,
        .frame_buffer = { 0 },
        .active_pixel_count = 0
    };

    // 1. Compliant vertex: Normalized coordinates (0.5, 0.5) -> Should pass
    voodoo_vertex_t valid_vtx = {
        .s = 0.5,
        .t = 0.5,
        .color = 0xFF00FF
    };

    printf("[TEST] Processing compliant texture vertex...\n");
    fflush(stdout);
    bool ok = process_voodoo_vertex(&tmu, &valid_vtx);
    assert(ok == true);
    assert(tmu.pipeline_gate == CONDUC_STATE);
    assert(tmu.frame_buffer[0] == 0xFF00FF);
    printf("   ✓ Pixel committed. TMU pipeline gate opened successfully.\n");
    fflush(stdout);

    // 2. Out-of-bounds vertex: s coordinate exceeds limits (1.5) -> Should trap
    voodoo_vertex_t bad_vtx = {
        .s = 1.5,
        .t = 0.5,
        .color = 0x00FFFF
    };

    printf("[TEST] Processing out-of-bounds texture vertex...\n");
    fflush(stdout);
    ok = process_voodoo_vertex(&tmu, &bad_vtx);
    assert(ok == false);
    assert(tmu.pipeline_gate == CUTOFF_STATE);
    printf("   ✓ Out-of-bound TMU coordinates successfully blocked and cutoff.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VOODOO TMU TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
