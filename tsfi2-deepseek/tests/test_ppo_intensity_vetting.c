#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "tsfi_helmholtz.h"
#include "tsfi_math.h"

// Constants for the DeepSeek Physical Filter
#define WORLD_RESONANCE_K 2026
#define STOCHASTIC_NOISE  0xDEADBEEF

bool simulate_ppo_vetting(uint64_t intensity, uint64_t resonance_k) {
    // The DeepSeek Physical Filter Logic:
    // Only intensity that is a harmonic multiple of the resonance is "Relevant".
    if (intensity % resonance_k == 0) {
        return true; // Stable (1,1)
    } else {
        return false; // Active Discard (0,0)
    }
}

int main() {
    printf("=== TSFi Unit Test: PPO Quality of Intensity Vetting ===\n");

    // 1. Propose Resonant Intensity (Quality)
    uint64_t resonant_intensity = WORLD_RESONANCE_K * 5; 
    printf("[PROPOSAL 1] Resonant Intensity: %lu\n", resonant_intensity);
    
    bool result1 = simulate_ppo_vetting(resonant_intensity, WORLD_RESONANCE_K);
    if (result1) {
        printf("  -> [PASS] PPO Determination: QUALITY. NAND Trap: STABLE (1,1).\n");
    } else {
        printf("  -> [FAIL] PPO discarded resonant intensity!\n");
        return 1;
    }

    // 2. Propose Stochastic Intensity (No Relevance)
    uint64_t stochastic_intensity = WORLD_RESONANCE_K + 13; // Out of phase
    printf("[PROPOSAL 2] Stochastic Intensity: %lu\n", stochastic_intensity);
    
    bool result2 = simulate_ppo_vetting(stochastic_intensity, WORLD_RESONANCE_K);
    if (!result2) {
        printf("  -> [PASS] PPO Determination: NO RELEVANCE. NAND Trap: ACTIVE DISCARD (0,0).\n");
    } else {
        printf("  -> [FAIL] PPO allowed stochastic noise to pass vetting!\n");
        return 1;
    }

    // 3. Acoustic Link Simulation
    float pressure = result2 ? 0.05f : 1.0f;
    float heartbeat = result2 ? 40.0f : 110.0f;
    printf("\n--- ACOUSTIC TELEMETRY MIRROR ---\n");
    printf("Resulting pDAI Pressure: %.2f\n", pressure);
    printf("Resulting Heartbeat: %.1f Hz\n", heartbeat);

    printf("\n[SUCCESS] PPO Intensity Vetting Verified.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
