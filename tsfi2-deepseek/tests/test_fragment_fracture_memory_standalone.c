#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Mocking TSFi Manifold Structures for Standalone Test
typedef struct {
    uint8_t data[256];
} Leaf;

// The logic we implemented in tsfi_fourier.c and tsfi_journal.c
static uint64_t tsfi_fault_characterize_standalone(const uint8_t *intent_hash, uint64_t realization_id) {
    uint64_t intent_prefix = *(uint64_t*)intent_hash;
    return intent_prefix ^ realization_id;
}

static void tsfi_form_memory_from_fracture_standalone(void *manifold, uint64_t svdag_id, const uint8_t *target_hash) {
    Leaf *leaves = (Leaf *)manifold;
    uint8_t *leaf502 = leaves[502].data;
    
    // 1. Calculate the Alligator's Heuristic Signature
    uint64_t fault_sig = tsfi_fault_characterize_standalone(target_hash, svdag_id);

    // 2. Memory Schema: [8B ID][32B Hash][8B Sig][4B Energy][1B Active]
    memcpy(leaf502, &svdag_id, 8);
    memcpy(leaf502 + 8, target_hash, 32);
    memcpy(leaf502 + 40, &fault_sig, 8);
    
    float initial_energy = 1.0f;
    memcpy(leaf502 + 48, &initial_energy, 4);
    leaf502[52] = 1; 
    
    printf("[MEMORY] Fracture Re-Integrated. Signature: 0x%lx, Energy: %.2f\n", 
           (unsigned long)fault_sig, initial_energy);
}

void tsfi_grow_fault_energy_standalone(void *manifold, float weight) {
    Leaf *leaves = (Leaf *)manifold;
    uint8_t *leaf502 = leaves[502].data;
    if (leaf502[52] != 1) return;

    float current_energy;
    memcpy(&current_energy, leaf502 + 48, 4);
    current_energy += (0.1f * weight);
    memcpy(leaf502 + 48, &current_energy, 4);
    
    printf("[DESIRE] Energy Grown: %.2f\n", current_energy);
}

void test_fracture_memory_formation() {
    printf("[TEST] Fragment Density Operation: Fault Signature & Energy Growth\n");

    // 1. Setup Mock Manifold
    void *manifold = calloc(65536, 256); 
    
    // 2. Simulate a Resonance Fracture (Intent vs Realization)
    uint64_t mock_svdag_id = 0xABCD1234567890ULL;
    uint8_t target_hash[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    
    printf("[ACTION] Simulating The Alligator's Audit (The Fracture)...\n");
    tsfi_form_memory_from_fracture_standalone(manifold, mock_svdag_id, target_hash);

    // 3. Verify Signature
    Leaf *leaves = (Leaf *)manifold;
    uint8_t *leaf502 = leaves[502].data;
    uint64_t captured_sig = *(uint64_t*)(leaf502 + 40);
    uint64_t expected_sig = (*(uint64_t*)target_hash) ^ mock_svdag_id;
    assert(captured_sig == expected_sig);
    printf("[PASS] Heuristic Signature Verified (The Bite is accurate).\n");

    // 4. Demonstrate Energy Growth (Desire to Evolve)
    printf("[ACTION] Feeding Energy to the Fracture...\n");
    for (int i = 0; i < 10; i++) {
        tsfi_grow_fault_energy_standalone(manifold, 1.0f);
    }

    float final_energy;
    memcpy(&final_energy, leaf502 + 48, 4);
    assert(final_energy >= 2.0f);
    printf("[PASS] Energy Growth Verified. Potential for evolution established.\n");

    // 5. Cleanup
    free(manifold);
}

int main() {
    printf("=== TSFi Unit Test: Standalone Fracture Memory & Fragment Density ===\n");
    test_fracture_memory_formation();
    printf("=== All Tests Passed ===\n");
    return 0;
}
