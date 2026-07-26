#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_mann_controller.h"

int main(void) {
    printf("=== RUNNING AUNCIENT MANN MEMORY CONTROLLER TESTS ===\n");

    TSFiMANNController mann;
    tsfi_mann_controller_init(&mann);

    // Verify initial states
    assert(mann.write_gate == 0.5f);
    assert(mann.read_gate == 0.5f);
    assert(mann.allocation_gate == 0.5f);
    
    // Check initial memory matrix is zero
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        assert(mann.usage_vector[i] == 0.0f);
        for (int j = 0; j < MANN_SLOT_WIDTH; j++) {
            assert(mann.memory_matrix[i][j] == 0.0f);
        }
    }

    // Define test vectors
    float key_vector[MANN_SLOT_WIDTH];
    float erase_vector[MANN_SLOT_WIDTH];
    float write_vector[MANN_SLOT_WIDTH];

    memset(key_vector, 0, sizeof(key_vector));
    memset(erase_vector, 0, sizeof(erase_vector));
    memset(write_vector, 0, sizeof(write_vector));

    key_vector[0] = 1.0f;
    erase_vector[0] = 1.0f;
    write_vector[0] = 4.2f; // Arbitrary value to write

    printf("1. Performing differentiable write operation...\n");
    tsfi_mann_controller_write(&mann, key_vector, 10.0f, erase_vector, write_vector);

    // Verify usage updated
    float total_usage = 0.0f;
    for (int i = 0; i < MANN_MEMORY_SLOTS; i++) {
        total_usage += mann.usage_vector[i];
    }
    assert(total_usage > 0.0f);
    printf("   ✓ Write usage updated successfully.\n");

    printf("2. Performing differentiable read query matching written key...\n");
    float read_vector[MANN_SLOT_WIDTH];
    tsfi_mann_controller_read(&mann, key_vector, 10.0f, read_vector);

    // Since we wrote 4.2 to slot 0 when matching key_vector[0] = 1.0,
    // the read out should have a positive response in index 0.
    printf("   Matched read value at index 0: %f\n", read_vector[0]);
    assert(read_vector[0] > 0.0f);
    printf("   ✓ Read match verified successfully.\n");

    printf("=== AUNCIENT MANN MEMORY CONTROLLER TESTS COMPLETE (PASS) ===\n");
    return 0;
}
