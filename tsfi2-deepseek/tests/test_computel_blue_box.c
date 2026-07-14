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
    bool save_ok = blue_box_save_state_to_disk("assets/blue_box_test.dat");
    assert(save_ok == true);

    // Mangle current state in memory
    blue_box_init_block(99, zero_hash);
    state = blue_box_get_block_state();
    assert(state.block_number == 99);

    // Restore from disk
    bool load_ok = blue_box_load_state_from_disk("assets/blue_box_test.dat");
    assert(load_ok == true);
    state = blue_box_get_block_state();
    assert(state.block_number == 42);
    assert(state.active_trunk_mask == 257);
    assert(state.is_committed == true);

    // Clean up temporary test file
    remove("assets/blue_box_test.dat");
    remove("assets/blue_box_test.dat.hist");

    // 9. Test Ordered Hash Validation Guards and Commit Persist
    uint8_t init_hash[32] = {1, 2, 3, 4};
    blue_box_init_block(10, init_hash);
    blue_box_register_block_trunk(805);
    
    // First commit - creates the file
    bool persist_ok = blue_box_commit_and_persist_with_guard("assets/blue_box_guard_test.dat", 0, zero_hash);
    assert(persist_ok == true);
    
    BlueBoxBlockState saved_state = blue_box_get_block_state();
    assert(saved_state.is_committed == true);

    // Try committing out-of-order block (expected 11, but passing 12)
    blue_box_init_block(99, zero_hash);
    persist_ok = blue_box_commit_and_persist_with_guard("assets/blue_box_guard_test.dat", 12, saved_state.state_hash);
    assert(persist_ok == false);

    // Try committing with correct block but incorrect parent hash
    uint8_t wrong_hash[32] = {9, 9, 9};
    persist_ok = blue_box_commit_and_persist_with_guard("assets/blue_box_guard_test.dat", 10, wrong_hash);
    assert(persist_ok == false);

    // Commit correctly (parent block 10, correct parent hash)
    blue_box_init_block(0, zero_hash); // reset local memory
    blue_box_register_block_trunk(815);
    persist_ok = blue_box_commit_and_persist_with_guard("assets/blue_box_guard_test.dat", 10, saved_state.state_hash);
    assert(persist_ok == true);

    // Read back and assert evolved block number 11
    BlueBoxBlockState final_state;
    FILE *ft = fopen("assets/blue_box_guard_test.dat", "rb");
    assert(ft != NULL);
    assert(fread(&final_state, sizeof(BlueBoxBlockState), 1, ft) == 1);
    fclose(ft);
    assert(final_state.block_number == 11);
    
    remove("assets/blue_box_guard_test.dat");
    remove("assets/blue_box_guard_test.dat.hist");

    // 10. Test Non-Preferential Accumulator System
    blue_box_init_block(50, zero_hash);
    blue_box_accumulate_state(1000000000000000ULL);
    blue_box_commit_block();
    uint64_t accumulated = blue_box_get_accumulator();
    // 1000000000000000 % 953467954114363 = 46532045885637
    assert(accumulated == 46532045885637ULL);
    
    // Check that state hash has evolved
    state = blue_box_get_block_state();
    assert(state.state_hash[0] != 0);

    // Verify session key rotation
    assert(state.session_key != 0xDEADC0DE95346795ULL);

    // 11. Test Payload Encryption
    uint8_t payload[16] = "TestTreePayload";
    uint8_t original[16] = "TestTreePayload";
    blue_box_crypt_payload(payload, 16);
    // Payload should be altered
    assert(memcmp(payload, original, 16) != 0);
    // Decrypting should return the original text
    blue_box_crypt_payload(payload, 16);
    assert(memcmp(payload, original, 16) == 0);

    // 12. Test Access Codes (MAX Pay Codes & Centrex Features)
    uint32_t prev_gas = state.gas_allowance;
    bool pay_ok = blue_box_decode_access_code("*99*847321#");
    assert(pay_ok == true);
    state = blue_box_get_block_state();
    assert(state.gas_allowance == prev_gas + 250000);

    bool forward_ok = blue_box_decode_access_code("*72");
    assert(forward_ok == true);
    state = blue_box_get_block_state();
    assert((state.active_trunk_mask & (1U << 31)) != 0);

    bool cancel_ok = blue_box_decode_access_code("*73");
    assert(cancel_ok == true);
    state = blue_box_get_block_state();
    assert((state.active_trunk_mask & (1U << 31)) == 0);

    // 13. Test Red-Black Tree Immutability Indexing
    TwoThreeNode *tt1 = blue_box_create_leaf(10, "RbtData_10", 0, NULL);
    TwoThreeNode *tt2 = blue_box_create_leaf(5, "RbtData_5", 0, NULL);
    TwoThreeNode *tt3 = blue_box_create_leaf(15, "RbtData_15", 0, NULL);
    
    blue_box_rbt_insert(10, tt1);
    blue_box_rbt_insert(5, tt2);
    blue_box_rbt_insert(15, tt3);
    
    TwoThreeNode *found_node = blue_box_rbt_lookup(10);
    assert(found_node != NULL);
    assert(strcmp(blue_box_retrieve_23_data(found_node, 10), "RbtData_10") == 0);
    
    found_node = blue_box_rbt_lookup(5);
    assert(found_node != NULL);
    assert(strcmp(blue_box_retrieve_23_data(found_node, 5), "RbtData_5") == 0);
    
    found_node = blue_box_rbt_lookup(15);
    assert(found_node != NULL);
    assert(strcmp(blue_box_retrieve_23_data(found_node, 15), "RbtData_15") == 0);
    
    found_node = blue_box_rbt_lookup(999);
    assert(found_node == NULL);
    
    blue_box_free_23_tree(tt1);
    blue_box_free_23_tree(tt2);
    blue_box_free_23_tree(tt3);

    // 14. Test Red-Black Tree Persistent Reload from Disk
    remove("assets/rbt_reload_test.dat");
    remove("assets/rbt_reload_test.dat.hist");
    
    blue_box_init_block(100, zero_hash);
    bool r_ok = blue_box_commit_and_persist_with_guard("assets/rbt_reload_test.dat", 0, zero_hash);
    assert(r_ok == true);
    
    BlueBoxBlockState b100_state = blue_box_get_block_state();
    
    blue_box_init_block(0, zero_hash);
    blue_box_register_block_trunk(808);
    r_ok = blue_box_commit_and_persist_with_guard("assets/rbt_reload_test.dat", 100, b100_state.state_hash);
    assert(r_ok == true);
    
    BlueBoxBlockState b101_state = blue_box_get_block_state();
    
    // Mangle memory state and RBT index
    rbt_root = NULL;
    rbt_node_count = 0;
    blue_box_init_block(999, zero_hash);
    
    // Load and check if RBT gets rebuilt automatically from history ledger
    bool load_rbt_ok = blue_box_load_state_from_disk("assets/rbt_reload_test.dat");
    assert(load_rbt_ok == true);
    
    TwoThreeNode *h100 = blue_box_rbt_lookup(100);
    assert(h100 != NULL);
    const char *data100 = blue_box_retrieve_23_data(h100, 100);
    assert(data100 != NULL);
    assert(strstr(data100, "nonce:") != NULL);
    
    TwoThreeNode *h101 = blue_box_rbt_lookup(101);
    assert(h101 != NULL);
    assert(h101->keys[0] == b101_state.block_number);
    const char *data101 = blue_box_retrieve_23_data(h101, 101);
    assert(data101 != NULL);
    assert(strstr(data101, "nonce:") != NULL);
    
    // 15. Test Query RDBMS Engine
    remove("assets/rdbms_test.dat");
    remove("assets/rdbms_test.dat.hist");
    
    // Write 3 evolution blocks
    blue_box_init_block(200, zero_hash);
    blue_box_decode_access_code("*99*123#"); // Gas allowance = 750000
    r_ok = blue_box_commit_and_persist_with_guard("assets/rdbms_test.dat", 0, zero_hash);
    assert(r_ok == true);
    
    BlueBoxBlockState b200 = blue_box_get_block_state();
    
    blue_box_init_block(0, zero_hash);
    blue_box_register_block_trunk(805); // active_trunk_mask = (1 << 5)
    r_ok = blue_box_commit_and_persist_with_guard("assets/rdbms_test.dat", 200, b200.state_hash);
    assert(r_ok == true);
    
    BlueBoxBlockState b201 = blue_box_get_block_state();
    
    blue_box_init_block(0, zero_hash);
    blue_box_decode_access_code("*72"); // active_trunk_mask |= (1 << 31)
    r_ok = blue_box_commit_and_persist_with_guard("assets/rdbms_test.dat", 201, b201.state_hash);
    assert(r_ok == true);
    
    // Execute query scans
    uint32_t results[10] = {0};
    uint32_t matched = blue_box_query_blocks("assets/rdbms_test.dat", "block_number", ">", 200, results, 10);
    assert(matched == 2);
    assert(results[0] == 201);
    assert(results[1] == 202);
    
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "gas_allowance", "=", 750000, results, 10);
    assert(matched == 3); // all evolved blocks inherited 750000 gas
    
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "active_trunk_mask", "&", (1U << 31), results, 10);
    assert(matched == 1);
    assert(results[0] == 202);
    
    // 16. Test Blue Box 2-3 Tree Node Awareness and Binding
    TwoThreeNode *t1 = blue_box_create_leaf(100, "NodeData_100", 200, "NodeData_200");
    TwoThreeNode *t2 = blue_box_create_leaf(300, "NodeData_300", 0, NULL);
    TwoThreeNode *tree_root = blue_box_create_internal(t1, t2, NULL);
    
    // Bind root of 2-3 tree to Blue Box current state hash
    blue_box_bind_23_tree(tree_root);
    state = blue_box_get_block_state();
    assert(memcmp(state.state_hash, tree_root->node_hash, 32) == 0);
    
    // Retrieve via Blue Box function
    const char *ret = blue_box_retrieve_23_data(tree_root, 200);
    assert(ret != NULL);
    assert(strcmp(ret, "NodeData_200") == 0);
    
    // Update data at key and assert Blue Box state_hash is synced to the new root hash
    blue_box_store_23_data(tree_root, 200, "Updated_NodeData_200");
    blue_box_bind_23_tree(tree_root);
    
    state = blue_box_get_block_state();
    assert(memcmp(state.state_hash, tree_root->node_hash, 32) == 0);
    
    blue_box_free_23_tree(tree_root);

    // 17. Test RDBMS DML (Update & Delete) and RBT Optimized query lookups
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "block_number", "=", 200, results, 10);
    assert(matched == 1);
    assert(results[0] == 200);
    
    // Update Gas allowance of block 201 to 999999
    bool update_ok = blue_box_update_block_gas("assets/rdbms_test.dat", 201, 999999);
    assert(update_ok == true);
    
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "gas_allowance", "=", 999999, results, 10);
    assert(matched == 1);
    assert(results[0] == 201);
    
    // Delete block 201 (soft delete)
    bool delete_ok = blue_box_delete_block("assets/rdbms_test.dat", 201);
    assert(delete_ok == true);
    
    // Block 201 should no longer be returned by query scans
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "gas_allowance", "=", 999999, results, 10);
    assert(matched == 0);
    
    matched = blue_box_query_blocks("assets/rdbms_test.dat", "block_number", ">", 200, results, 10);
    assert(matched == 1); // Only block 202 matches now since 201 is deleted
    assert(results[0] == 202);

    // 18. Test RDBMS Transactions (Begin, Commit, Rollback)
    remove("assets/txn_test.dat");
    remove("assets/txn_test.dat.hist");
    
    blue_box_begin_transaction();
    BlueBoxBlockState tstate1 = { .block_number = 500, .gas_allowance = 55555, .nonce = 1, .is_committed = true };
    BlueBoxBlockState tstate2 = { .block_number = 501, .gas_allowance = 66666, .nonce = 2, .is_committed = true };
    
    bool txn_ok = blue_box_add_to_transaction(&tstate1);
    assert(txn_ok == true);
    txn_ok = blue_box_add_to_transaction(&tstate2);
    assert(txn_ok == true);
    
    // Commit transaction
    commit_ok = blue_box_commit_transaction("assets/txn_test.dat");
    assert(commit_ok == true);
    
    // Query committed transaction blocks
    matched = blue_box_query_blocks("assets/txn_test.dat", "gas_allowance", "=", 55555, results, 10);
    assert(matched == 1);
    assert(results[0] == 500);
    
    // Test Rollback
    blue_box_begin_transaction();
    BlueBoxBlockState tstate3 = { .block_number = 502, .gas_allowance = 77777, .nonce = 3, .is_committed = true };
    txn_ok = blue_box_add_to_transaction(&tstate3);
    assert(txn_ok == true);
    blue_box_rollback_transaction();
    
    // 502 should not be written
    matched = blue_box_query_blocks("assets/txn_test.dat", "gas_allowance", "=", 77777, results, 10);
    assert(matched == 0);
    
    remove("assets/txn_test.dat");
    remove("assets/txn_test.dat.hist");
    remove("assets/rdbms_test.dat");
    remove("assets/rdbms_test.dat.hist");
    remove("assets/rbt_reload_test.dat");
    remove("assets/rbt_reload_test.dat.hist");

    printf("[SUCCESS] All Computel Blue Box SF/MF, Red Box coin, immutable storage, block state, serialization, validation guards, accumulator, payload crypt, access codes, Red-Black Tree, Query RDBMS, 2-3 Tree Awareness, RDBMS DML, and Relational Transaction tests passed successfully.\n");
    return 0;
}
