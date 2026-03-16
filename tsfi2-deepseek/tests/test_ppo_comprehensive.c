#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

#define WORLD_RESONANCE_K 2026
#define GEMINI_LEAF 500
#define DEEPSEEK_LEAF 501
#define PROPOSAL_LEAF 1000

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void setup_manifold(void *manifold, size_t size) {
    memset(manifold, 0, size);
    // Initialize Gemini Lever (Set_N) to 1
    uint8_t *set_n = (uint8_t *)manifold + (GEMINI_LEAF * 256);
    set_n[0] = 1;
}

int main() {
    printf("=== TSFi PPO Comprehensive Vetting Suite ===\n");
    tsfi_helmholtz_init();

    size_t manifold_size = 1024 * 1024; // 1MB (Level 11 range)
    void *manifold = lau_malloc(manifold_size);
    
    // Test Case 1: Harmonic Intensity (Resonant)
    {
        printf("[TEST 1] Resonant Proposal ($K * 10)...\n");
        setup_manifold(manifold, manifold_size);
        
        TSFiPPOProposal proposal = {
            .intensity = WORLD_RESONANCE_K * 10,
            .leaf_index = PROPOSAL_LEAF,
            .epoch = 1
        };
        
        // Stage the data
        uint8_t *target = (uint8_t *)manifold + (PROPOSAL_LEAF * 256);
        snprintf((char *)target, 256, "RESONANT_TRAJECTORY_DATA");

        bool approved = tsfi_ppo_audit_proposal(manifold, &proposal, WORLD_RESONANCE_K);
        
        uint8_t *reset_n = (uint8_t *)manifold + (DEEPSEEK_LEAF * 256);
        assert(approved == true);
        assert(proposal.is_resonant == true);
        assert(reset_n[0] == 1); // Oracle must set high
        assert(strcmp((char *)target, "RESONANT_TRAJECTORY_DATA") == 0); // Data must survive
        printf("  -> [PASS] Resonant intensity approved and data rigid.\n");
    }

    // Test Case 2: Stochastic Noise (Non-Resonant)
    {
        printf("[TEST 2] Stochastic Proposal (K + 1)...\n");
        setup_manifold(manifold, manifold_size);
        
        TSFiPPOProposal proposal = {
            .intensity = WORLD_RESONANCE_K + 1,
            .leaf_index = PROPOSAL_LEAF,
            .epoch = 1
        };
        
        uint8_t *target = (uint8_t *)manifold + (PROPOSAL_LEAF * 256);
        snprintf((char *)target, 256, "STOCHASTIC_NOISE_DATA");

        bool approved = tsfi_ppo_audit_proposal(manifold, &proposal, WORLD_RESONANCE_K);
        
        uint8_t *reset_n = (uint8_t *)manifold + (DEEPSEEK_LEAF * 256);
        assert(approved == false);
        assert(proposal.is_resonant == false);
        assert(reset_n[0] == 0); // Oracle must pull low
        // Physical Discard Verification
        for(int i=0; i<256; i++) {
            if (target[i] != 0) {
                printf("[FAIL] Physical incineration failed at byte %d\n", i);
                return 1;
            }
        }
        printf("  -> [PASS] Stochastic intensity rejected and data physically incinerated.\n");
    }

    // Test Case 3: Trilateral State Root Divergence
    {
        printf("[TEST 3] Topographic Rigidity Audit (Level 10)...\n");
        setup_manifold(manifold, manifold_size);
        
        uint8_t root_stable[32], receipt_stable[32];
        uint8_t root_fracture[32], receipt_fracture[32];

        // 3a. Generate Stable Root (1,1)
        uint8_t *reset_n = (uint8_t *)manifold + (DEEPSEEK_LEAF * 256);
        reset_n[0] = 1;
        tsfi_helmholtz_reduce_10(root_stable, receipt_stable, NULL, NULL, manifold, 1, WORLD_RESONANCE_K, NULL);

        // 3b. Generate Fracture Root (1,0) - simulating Oracle pulling the lever
        reset_n[0] = 0;
        tsfi_helmholtz_reduce_10(root_fracture, receipt_fracture, NULL, NULL, manifold, 1, WORLD_RESONANCE_K, NULL);

        assert(memcmp(root_stable, root_fracture, 32) != 0);
        printf("  -> [PASS] NAND lever state physically influences the State Root.\n");
    }

    // Test Case 4: Performance Latency
    {
        printf("[TEST 4] PPO Vetting Latency Sweep...\n");
        int iterations = 100000;
        TSFiPPOProposal proposal = { .intensity = WORLD_RESONANCE_K * 5, .leaf_index = PROPOSAL_LEAF };
        
        double start = get_time_ms();
        for(int i=0; i<iterations; i++) {
            tsfi_ppo_audit_proposal(manifold, &proposal, WORLD_RESONANCE_K);
        }
        double end = get_time_ms();
        printf("  -> Throughput: %.2f audits/sec\n", iterations / ((end - start) / 1000.0));
        printf("  -> Average:    %.4f us per audit\n", (end - start) * 1000.0 / iterations);
    }

    lau_free(manifold);
    tsfi_helmholtz_cleanup();
    printf("\n[SUCCESS] PPO Comprehensiveness Suite Verified.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
