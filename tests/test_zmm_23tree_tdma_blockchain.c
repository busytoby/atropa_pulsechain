#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define BLOCK_DATA_SIZE 32
#define MAX_CHAIN_LEN 8

typedef struct {
    uint32_t index;
    uint32_t slot_id;
    uint32_t slot_duration_ms; // Dynamic Slot Resizing
    uint32_t guard_time_ms;     // Guard Time Calibration
    uint64_t clock_offset_ns;   // Decentralized Synchronization
    char data[BLOCK_DATA_SIZE];
    uint64_t prev_hash;
    uint64_t hash;
} Block;

typedef struct {
    Block chain[MAX_CHAIN_LEN];
    size_t length;
} TdmaBlockchain;

// 2-3 Tree Node Structure where each node hosts a TDMA-compliant blockchain
typedef struct TwoThreeNode {
    bool is_three_node;
    int keys[2]; // Node identifiers
    TdmaBlockchain chains[2]; // Independent blockchains per key
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Simple hash calculator
uint64_t compute_block_hash(const Block *b) {
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + b->index;
    hash = ((hash << 5) + hash) + b->slot_id;
    hash = ((hash << 5) + hash) + b->slot_duration_ms;
    hash = ((hash << 5) + hash) + b->guard_time_ms;
    hash = ((hash << 5) + hash) + b->clock_offset_ns;
    hash = ((hash << 5) + hash) + b->prev_hash;
    for (size_t i = 0; i < strlen(b->data); i++) {
        hash = ((hash << 5) + hash) + b->data[i];
    }
    return hash;
}

// Appends a block with dynamic slot sizing, guard times, and clock offsets
void add_tdma_block(TdmaBlockchain *blockchain, uint32_t slot_id, const char *data, uint32_t base_duration, uint32_t load_factor) {
    assert(blockchain->length < MAX_CHAIN_LEN);
    
    Block b;
    b.index = (uint32_t)blockchain->length;
    b.slot_id = slot_id;
    
    // 1. Dynamic Slot Resizing: Slot duration scales with load_factor
    b.slot_duration_ms = base_duration + (load_factor * 10);
    
    // 2. Guard Time Calibration: Dynamic guard interval proportional to duration
    b.guard_time_ms = b.slot_duration_ms / 8;
    
    // 3. Decentralized Synchronization: Mocked drift correction offset
    b.clock_offset_ns = (slot_id % 2 == 0) ? 5 : 0;
    
    strncpy(b.data, data, BLOCK_DATA_SIZE - 1);
    b.data[BLOCK_DATA_SIZE - 1] = '\0';
    b.prev_hash = (blockchain->length == 0) ? 0 : blockchain->chain[blockchain->length - 1].hash;
    b.hash = compute_block_hash(&b);

    blockchain->chain[blockchain->length] = b;
    blockchain->length++;
}

// Simple lookup in 2-3 Tree
TdmaBlockchain *lookup_tree_blockchain(TwoThreeNode *root, int key) {
    if (root == NULL) return NULL;
    
    if (root->keys[0] == key) {
        return &root->chains[0];
    }
    if (root->is_three_node && root->keys[1] == key) {
        return &root->chains[1];
    }
    
    if (key < root->keys[0]) {
        return lookup_tree_blockchain(root->children[0], key);
    } else if (!root->is_three_node) {
        return lookup_tree_blockchain(root->children[1], key);
    } else {
        if (key < root->keys[1]) {
            return lookup_tree_blockchain(root->children[1], key);
        } else {
            return lookup_tree_blockchain(root->children[2], key);
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT 2-3 TREE TDMA BLOCKCHAIN STORAGE\n");
    printf("=============================================================\n");

    // Initialize 2-3 Tree root node (three-node with keys 10 and 20)
    TwoThreeNode root;
    memset(&root, 0, sizeof(root));
    root.is_three_node = true;
    root.keys[0] = 10;
    root.keys[1] = 20;

    // 1. Propose blocks to Key 10's blockchain
    printf("1. Appending blocks to Key 10 with Dynamic Sizing & Guard Times...\n");
    TdmaBlockchain *chain10 = lookup_tree_blockchain(&root, 10);
    assert(chain10 != NULL);
    
    // Add block with low load (load_factor = 1)
    add_tdma_block(chain10, 0, "Genesis 10", 40, 1);
    assert(chain10->chain[0].slot_duration_ms == 50); // 40 + 10
    assert(chain10->chain[0].guard_time_ms == 6);      // 50 / 8

    // Add block with high load (load_factor = 5)
    add_tdma_block(chain10, 1, "Tx Block 1", 40, 5);
    assert(chain10->chain[1].slot_duration_ms == 90); // 40 + 50
    assert(chain10->chain[1].guard_time_ms == 11);     // 90 / 8
    
    printf("   ✓ Block sizing verified (Slot: %u ms, Guard: %u ms)\n\n", 
           chain10->chain[1].slot_duration_ms, chain10->chain[1].guard_time_ms);

    // 2. Propose blocks to Key 20's blockchain
    printf("2. Appending blocks to Key 20...\n");
    TdmaBlockchain *chain20 = lookup_tree_blockchain(&root, 20);
    assert(chain20 != NULL);
    add_tdma_block(chain20, 0, "Genesis 20", 30, 2);
    
    assert(chain20->length == 1);
    assert(chain20->chain[0].slot_duration_ms == 50); // 30 + 20
    printf("   ✓ Key 20 blockchain active.\n\n");

    // 3. Verify integrity
    printf("3. Verifying parent hashing across tree blockchains...\n");
    assert(chain10->chain[1].prev_hash == chain10->chain[0].hash);
    printf("   ✓ Ledger links are congruous.\n\n");

    printf("=============================================================\n");
    printf("2-3 TREE TDMA INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
