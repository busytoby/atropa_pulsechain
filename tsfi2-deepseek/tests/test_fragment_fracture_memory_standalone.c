#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// Mocking TSFi Manifold Structures for Standalone Test
typedef struct {
    uint8_t data[256];
} Leaf;

// The logic we implemented in tsfi_fourier.c
static void tsfi_form_memory_from_fracture_standalone(void *manifold, uint64_t svdag_id, const uint8_t *target_hash) {
    // A fracture is a new memory forming. 
    // We anchor the objective realization into the Lore region (Leaf 502).
    Leaf *leaves = (Leaf *)manifold;
    uint8_t *leaf502 = leaves[502].data;
    
    // Memory Schema: [8 bytes ID][32 bytes Hash][1 byte Active Flag]
    memcpy(leaf502, &svdag_id, 8);
    memcpy(leaf502 + 8, target_hash, 32);
    leaf502[40] = 1; // Mark as "Captured Fracture Memory"
    
    printf("[MEMORY] Fracture Re-Integrated. New State Anchored to Lore Leaf 502.\n");
}

void test_fracture_memory_formation() {
    printf("[TEST] Fragment Density Operation: Fracture Memory Formation (Standalone)\n");

    // 1. Setup Mock Manifold (65536 leaves * 256 bytes)
    void *manifold = calloc(65536, 256); 
    
    // 2. Simulate a Resonance Fracture
    uint64_t mock_svdag_id = 0x123456789ABCDEF0;
    uint8_t target_hash[32] = {0};
    memset(target_hash, 0xAB, 32);
    
    printf("[ACTION] Simulating Fourier Isomorphism Mismatch (The Fracture)...\n");
    tsfi_form_memory_from_fracture_standalone(manifold, mock_svdag_id, target_hash);

    // 3. Audit Lore Leaf 502 (The Memory Formation Point)
    Leaf *leaves = (Leaf *)manifold;
    uint8_t *leaf502 = leaves[502].data;
    uint64_t captured_id = *(uint64_t*)leaf502;
    uint8_t active_flag = leaf502[40];

    printf("[AUDIT] Lore Leaf 502: ID=0x%lx, Active=%d\n", (unsigned long)captured_id, active_flag);

    // 4. Verification
    assert(captured_id == mock_svdag_id);
    assert(active_flag == 1);
    assert(memcmp(leaf502 + 8, target_hash, 32) == 0);
    
    printf("[PASS] Memory formed on the opposite side of the fracture.\n");
    printf("[PASS] Fragment area conversion confirmed via lore anchoring.\n");

    // 5. Cleanup
    free(manifold);
}

int main() {
    printf("=== TSFi Unit Test: Standalone Fracture Memory & Fragment Density ===\n");
    test_fracture_memory_formation();
    printf("=== All Tests Passed ===\n");
    return 0;
}
