#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../src/tsfi_computel_blue_box.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Blue Box SF/MF test suite...\n");

    float buffer[200];

    // 1. Test SF Seizure (2600 Hz)
    generate_sf_seizure(buffer, 200);
    // Spot check sample (index 99) -> should be non-zero
    assert(buffer[99] != 0.0f);
    printf("[TEST] SF 2600 Hz seizure tone verified.\n");

    // 2. Test MF Keypulse 'K' (1100 + 1700 Hz)
    bool ok = generate_mf_tone('K', buffer, 200);
    assert(ok == true);
    assert(buffer[99] != 0.0f);

    // 3. Test MF Start 'S' (1500 + 1700 Hz)
    ok = generate_mf_tone('S', buffer, 200);
    assert(ok == true);
    assert(buffer[99] != 0.0f);

    // 4. Test invalid digit
    ok = generate_mf_tone('Z', buffer, 200);
    assert(ok == false);

    // 5. Test Red Box Payphone Coin Tones
    float coin_buf[2000];
    // Nickel (5 cents) -> 1 beep = 2 * 66 ms = 132 ms * 8000 = 1056 samples
    int samples = generate_red_box_coin_tone(5, coin_buf, 2000);
    assert(samples == 1056);

    // Dime (10 cents) -> 2 beeps = 4 * 66 ms = 264 ms * 8000 = 2112 samples
    float large_coin_buf[3000];
    samples = generate_red_box_coin_tone(10, large_coin_buf, 3000);
    assert(samples == 2112);

    // 6. Test Immutable ZMM Address Storage contained within Blue Box
    const char *addr800 = blue_box_get_immutable_address(800);
    assert(addr800 != NULL);
    assert(strcmp(addr800, "dynamic_0x0000000000000000000000000000000000000800") == 0);

    const char *addr805 = blue_box_get_immutable_address(805);
    assert(addr805 != NULL);
    assert(strcmp(addr805, "dynamic_0x0000000000000000000000000000000000000805") == 0);

    const char *addr_invalid = blue_box_get_immutable_address(999);
    assert(addr_invalid == NULL);

    // 7. Test Block State Management inside Blue Box
    uint8_t zero_hash[32] = {0};
    blue_box_init_block(42, zero_hash);
    BlueBoxBlockState state = blue_box_get_block_state();
    assert(state.block_number == 42);
    assert(state.is_committed == false);

    blue_box_register_block_trunk(800);
    blue_box_register_block_trunk(808);
    state = blue_box_get_block_state();
    // 800 - 800 = 0 -> bit 0. 808 - 800 = 8 -> bit 8.
    // 1 << 0 | 1 << 8 = 1 | 256 = 257.
    assert(state.active_trunk_mask == 257);

    bool commit_ok = blue_box_commit_block();
    assert(commit_ok == true);
    state = blue_box_get_block_state();
    assert(state.is_committed == true);

    // 8. Test State Serialization (Save/Load to Disk)
    bool save_ok = blue_box_save_state_to_disk("blue_box_test.dat");
    assert(save_ok == true);

    // Mangle current state in memory
    blue_box_init_block(99, zero_hash);
    state = blue_box_get_block_state();
    assert(state.block_number == 99);

    // Restore from disk
    bool load_ok = blue_box_load_state_from_disk("blue_box_test.dat");
    assert(load_ok == true);
    state = blue_box_get_block_state();
    assert(state.block_number == 42);
    assert(state.active_trunk_mask == 257);
    assert(state.is_committed == true);

    // Clean up temporary test file
    remove("blue_box_test.dat");

    // 9. Test Ordered Hash Validation Guards and Commit Persist
    uint8_t init_hash[32] = {1, 2, 3, 4};
    blue_box_init_block(10, init_hash);
    blue_box_register_block_trunk(805);
    
    // First commit - creates the file
    bool persist_ok = blue_box_commit_and_persist_with_guard("blue_box_guard_test.dat", 0, zero_hash);
    assert(persist_ok == true);
    
    BlueBoxBlockState saved_state = blue_box_get_block_state();
    assert(saved_state.is_committed == true);

    // Try committing out-of-order block (expected 11, but passing 12)
    blue_box_init_block(99, zero_hash);
    persist_ok = blue_box_commit_and_persist_with_guard("blue_box_guard_test.dat", 12, saved_state.state_hash);
    assert(persist_ok == false);

    // Try committing with correct block but incorrect parent hash
    uint8_t wrong_hash[32] = {9, 9, 9};
    persist_ok = blue_box_commit_and_persist_with_guard("blue_box_guard_test.dat", 10, wrong_hash);
    assert(persist_ok == false);

    // Commit correctly (parent block 10, correct parent hash)
    blue_box_init_block(0, zero_hash); // reset local memory
    blue_box_register_block_trunk(815);
    persist_ok = blue_box_commit_and_persist_with_guard("blue_box_guard_test.dat", 10, saved_state.state_hash);
    assert(persist_ok == true);

    // Read back and assert evolved block number 11
    BlueBoxBlockState final_state;
    FILE *ft = fopen("blue_box_guard_test.dat", "rb");
    assert(ft != NULL);
    assert(fread(&final_state, sizeof(BlueBoxBlockState), 1, ft) == 1);
    fclose(ft);
    assert(final_state.block_number == 11);
    
    remove("blue_box_guard_test.dat");

    printf("[SUCCESS] All Computel Blue Box SF/MF, Red Box coin, immutable storage, block state, disk serialization, and hash validation guards passed successfully.\n");
    return 0;
}
