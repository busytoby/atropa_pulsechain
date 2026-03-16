#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "tsfi_wavelet_arena.h"
#include "lau_memory.h"
#include "lau_registry.h"

// Mock the Verilog ICPC 120F Spider Trait Tree Evaluator
void evaluate_spider_taste(bool valid[4], int depths[4], uint64_t resonance_k, int *out_diameter, bool *out_good_taste) {
    int max1 = 0;
    int max2 = 0;
    for (int i = 0; i < 4; i++) {
        if (valid[i]) {
            if (depths[i] > max1) {
                max2 = max1;
                max1 = depths[i];
            } else if (depths[i] > max2) {
                max2 = depths[i];
            }
        }
    }
    int diameter = max1 + max2;
    *out_diameter = diameter;
    
    // Taste Logic: Harmonic matching
    *out_good_taste = (diameter > 0) && (resonance_k > 0) && (diameter % resonance_k == 0);
}

// Mock the Alligator's Portfolio Reaping
void alligator_reap_portfolio(TsfiWavelet **portfolio, int count) {
    printf("[ALLIGATOR] Spider lost 120F evaluation. Reaping portfolio of %d wavelets...\n", count);
    for (int i = 0; i < count; i++) {
        if (portfolio[i]) {
            // Force terminal intensity
            portfolio[i]->telemetry.Fa.monopole = 0; 
            printf("  -> Reaping Wavelet ID=%lu\n", portfolio[i]->telemetry.unique_id);
            lau_free(portfolio[i]);
            portfolio[i] = NULL;
        }
    }
}

void test_valid_spider_tree() {
    printf("[TEST] Valid Spider Tree (Diameter > 0)...\n");
    bool valid[4] = {true, true, false, false};
    int depths[4] = {2, 3, 0, 0};
    
    int diameter = evaluate_spider_diameter(valid, depths);
    assert(diameter == 5);
    
    printf("  [PASS] Spider diameter %d is rigid. Portfolio survives.\n", diameter);
}

void test_fractured_spider_tree() {
    printf("[TEST] Fractured Spider Tree (Diameter == 0)...\n");
    
    // Create a portfolio of 3 File Wavelets
    TsfiWavelet *portfolio[3];
    for (int i = 0; i < 3; i++) {
        portfolio[i] = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
        portfolio[i]->telemetry.unique_id = 100 + i;
        portfolio[i]->telemetry.trait = TSFI_TRAIT_FILE;
        portfolio[i]->telemetry.Fa.monopole = 2026; // Resonant initially
    }
    
    // Simulate a fractured tree (e.g. empty or circular, resulting in 0 valid legs)
    bool valid[4] = {false, false, false, false};
    int depths[4] = {0, 0, 0, 0};
    
    int diameter = evaluate_spider_diameter(valid, depths);
    assert(diameter == 0);
    
    printf("  [FRACTURE] Spider diameter is 0. Alligator provoked.\n");
    
    // Trigger the absolute consequence
    alligator_reap_portfolio(portfolio, 3);
    
    // Verify reaping
    for (int i = 0; i < 3; i++) {
        assert(portfolio[i] == NULL);
    }
    
    printf("  [PASS] Portfolio completely reaped and memory freed.\n");
}

int main() {
    printf("=== TSFi Spider Trait Tree (120F) & Reaping Test ===\n");
    (void)lau_registry_get_head(); // Init registry
    
    test_valid_spider_tree();
    test_fractured_spider_tree();
    
    printf("[SUCCESS] Alligator's consequence logic is absolute and rigid.\n");
    return 0;
}
