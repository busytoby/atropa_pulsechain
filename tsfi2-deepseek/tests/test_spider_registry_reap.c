#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "tsfi_wavelet_arena.h"
#include "lau_memory.h"
#include "lau_registry.h"

// Mock the Verilog ICPC 120F Spider Trait Tree Evaluator
int evaluate_spider_diameter(bool valid[4], int depths[4]) {
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
    return max1 + max2;
}

// The Alligator queries the registry to reap the Spider's portfolio
void alligator_reap_registered_spider(uint64_t spider_id) {
    printf("[ALLIGATOR] Spider %lu lost 120F evaluation. Scanning registry for portfolio...\n", spider_id);
    
    // We cannot safely remove items while iterating the linked list directly if we just use a while loop,
    // but we can collect them first or use a safe removal pattern.
    // For simplicity in this test, we collect the pointers to reap.
    void *to_reap[128];
    int reap_count = 0;
    
    lau_registry_lock();
    LauMetadata *head = lau_registry_get_head();
    LauMetadata *current = head;
    while (current != NULL) {
        if (current->payload_start != NULL) {
            TsfiWavelet *W = (TsfiWavelet*)current->payload_start;
            // Check if this wavelet belongs to the fractured spider
            // We use the trait as a safety to ensure we are looking at a wavelet
            if (W->telemetry.trait == TSFI_TRAIT_FILE && W->telemetry.Fa.identity == spider_id) {
                to_reap[reap_count++] = W;
            }
        }
        current = current->next;
    }
    lau_registry_unlock();
    
    // Now perform the absolute consequence
    for (int i = 0; i < reap_count; i++) {
        TsfiWavelet *W = (TsfiWavelet*)to_reap[i];
        printf("  -> Registry hit: Reaping Wavelet ID=%lu (Spider Identity=%lu)\n", W->telemetry.unique_id, W->telemetry.Fa.identity);
        
        // Force terminal intensity
        W->telemetry.Fa.monopole = 0; 
        lau_free(W);
    }
    
    printf("  [PASS] Spider %lu portfolio completely erased from registry.\n", spider_id);
}

void test_registry_reap() {
    printf("[TEST] Registered Spider Portfolio Reaping...\n");
    
    uint64_t spider_id = 999;
    
    // 1. Spider populates the registry with its portfolio
    TsfiWavelet *W1 = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W1->telemetry.unique_id = 1; W1->telemetry.Fa.identity = spider_id; W1->telemetry.trait = TSFI_TRAIT_FILE;
    
    TsfiWavelet *W2 = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W2->telemetry.unique_id = 2; W2->telemetry.Fa.identity = spider_id; W2->telemetry.trait = TSFI_TRAIT_FILE;
    
    TsfiWavelet *W_Innocent = (TsfiWavelet*)lau_malloc_wired(sizeof(TsfiWavelet));
    W_Innocent->telemetry.unique_id = 3; W_Innocent->telemetry.Fa.identity = 888; // Different spider
    
    // Verify they are in the registry
    assert(lau_registry_find(W1) != NULL);
    assert(lau_registry_find(W2) != NULL);
    assert(lau_registry_find(W_Innocent) != NULL);
    
    // 2. Simulate 120F Hardware Fracture for Spider 999
    bool valid[4] = {false, false, false, false};
    int depths[4] = {0, 0, 0, 0};
    int diameter = evaluate_spider_diameter(valid, depths);
    assert(diameter == 0);
    
    printf("  [FRACTURE] Spider %lu diameter is 0.\n", spider_id);
    
    // 3. Trigger Alligator consequence
    alligator_reap_registered_spider(spider_id);
    
    // 4. Verify the registry
    assert(lau_registry_find(W1) == NULL);
    assert(lau_registry_find(W2) == NULL);
    assert(lau_registry_find(W_Innocent) != NULL); // Innocent survives
    
    printf("  [PASS] Registry selectively purged based on Spider identity.\n");
    
    // Cleanup innocent
    lau_free(W_Innocent);
}

int main() {
    printf("=== TSFi Spider Registry Reaping Test ===\n");
    (void)lau_registry_get_head(); // Ensure registry is active
    
    test_registry_reap();
    
    printf("[SUCCESS] Alligator registry reaping logic is rigid.\n");
    return 0;
}
