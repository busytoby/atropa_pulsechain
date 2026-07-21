#include "tsfi_parc_ingerman_display.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER Z INGERMAN DISPLAY VECTOR AUDIT (1961)  \n");
    printf("=======================================================\n");

    tsfi_ingerman_display_env_t env;
    assert(tsfi_ingerman_display_env_init(&env) == 0);
    printf("[PASS] Display Environment Initialized\n");

    // Create 3 nested lexical frames (Level 0: Global, Level 1: Outer Block, Level 2: Inner Block)
    tsfi_ingerman_stack_frame_t frame0, frame1, frame2;
    memset(&frame0, 0, sizeof(frame0));
    memset(&frame1, 0, sizeof(frame1));
    memset(&frame2, 0, sizeof(frame2));

    assert(tsfi_ingerman_display_push_block(&env, 101, &frame0) == 0); // Level 0
    assert(tsfi_ingerman_display_push_block(&env, 102, &frame1) == 0); // Level 1
    assert(tsfi_ingerman_display_push_block(&env, 103, &frame2) == 0); // Level 2
    printf("[PASS] Pushed 3 Nested Lexical Frames into Display Vector\n");

    // Store variables at different lexical levels via O(1) Display Vector
    assert(tsfi_ingerman_display_store(&env, 0, 0, 953467954114363ULL) == 0); // MotzkinPrime at Level 0 Slot 0
    assert(tsfi_ingerman_display_store(&env, 1, 2, 32) == 0);               // Keycode 32 at Level 1 Slot 2
    assert(tsfi_ingerman_display_store(&env, 2, 1, 30) == 0);               // Keycode 30 at Level 2 Slot 1
    printf("[PASS] Stored Variables across Lexical Levels via Display Vector\n");

    // Verify O(1) Lexical Lookups
    uint64_t v0 = tsfi_ingerman_display_lookup(&env, 0, 0);
    uint64_t v1 = tsfi_ingerman_display_lookup(&env, 1, 2);
    uint64_t v2 = tsfi_ingerman_display_lookup(&env, 2, 1);

    assert(v0 == 953467954114363ULL);
    assert(v1 == 32);
    assert(v2 == 30);
    printf("[PASS] O(1) Display Vector Lookup Verified (Level 0: %lu, Level 1: %lu, Level 2: %lu)\n", v0, v1, v2);

    // Pop inner block and verify level scope restoration
    assert(tsfi_ingerman_display_pop_block(&env) == 0);
    assert(env.current_depth == 2);
    assert(tsfi_ingerman_display_lookup(&env, 2, 1) == 0); // Level 2 popped
    printf("[PASS] Pop Frame Restored Lexical Depth to 2\n");

    printf("=======================================================\n");
    printf(" ALL PETER Z INGERMAN DISPLAY VECTOR TESTS PASSED      \n");
    printf("=======================================================\n");

    return 0;
}
