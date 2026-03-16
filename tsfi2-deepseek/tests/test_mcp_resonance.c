#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_manifest.h"
#include "lau_memory.h"

// Test: MCP Tool Resonance (The "Clutch" Engagement)
// Verifies that a binary wavefront correctly manifests a tool call.

int main() {
    printf("[TEST] MCP Wavefront Resonance (Binary Clutch)...\n");

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    assert(state.manifold != NULL);
    assert(state.manifold->count == 2);

    uint32_t tension = 0;
    int32_t res;

    // --- Case 1: Target Tool 0 (wave512.run) ---
    printf("  [CASE 1] Targeting 'wave512.run'...\n");
    uint32_t mu_run[16];
    for(int i=0; i<16; i++) mu_run[i] = 0x11111111;
    mu_run[15] = (mu_run[15] & ~TSFI_SECRET_MASK) | 0xA01;

    res = tsfi_phi_resolve(mu_run, state.manifold, &tension);
    if (res == 0) {
        printf("    PASS: Resolved to 'wave512.run' well.\n");
    } else {
        printf("    FAIL: Resolution failure (res=%d, tension=%u)\n", res, tension);
        return 1;
    }

    // --- Case 2: Target Tool 1 (shell.read_file) ---
    printf("  [CASE 2] Targeting 'shell.read_file'...\n");
    uint32_t mu_read[16];
    for(int i=0; i<16; i++) mu_read[i] = 0x22222222;
    mu_read[15] = (mu_read[15] & ~TSFI_SECRET_MASK) | 0xA02;

    res = tsfi_phi_resolve(mu_read, state.manifold, &tension);
    if (res == 1) {
        printf("    PASS: Resolved to 'shell.read_file' well.\n");
    } else {
        printf("    FAIL: Resolution failure (res=%d, tension=%u)\n", res, tension);
        return 1;
    }

    // --- Case 3: Out of Domain (Dielectric Breakdown) ---
    printf("  [CASE 3] Out-of-Domain Rejection...\n");
    uint32_t mu_noise[16];
    for(int i=0; i<16; i++) mu_noise[i] = 0x99999999;
    mu_noise[15] = (mu_noise[15] & ~TSFI_SECRET_MASK) | 0x666; // Random secret

    res = tsfi_phi_resolve(mu_noise, state.manifold, &tension);
    if (res == -1) {
        printf("    PASS: Rejected non-resonant wavefront.\n");
    } else {
        printf("    FAIL: Allowed non-bijective manifestation (res=%d)\n", res);
        return 1;
    }

    tsfi_zmm_vm_destroy(&state);
    printf("[TEST] MCP Resonance Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}