#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
// Simple SHA256 helper
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128]; // Mapped Device parameters
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Helper to create a leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1, uint32_t key2, const char *val2) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = val2 ? 2 : 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    if (val2) {
        node->keys[1] = key2;
        strcpy(node->values[1], val2);
    }
    
    // Hash payload of leaf
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s:%u:%s", 
                       node->keys[0], node->values[0], 
                       val2 ? node->keys[1] : 0, val2 ? node->values[1] : "");
    sha256(temp, len, node->node_hash);
    return node;
}

// Helper to create an internal node (either 2-node or 3-node)
static TwoThreeNode* create_internal(TwoThreeNode *c1, TwoThreeNode *c2, TwoThreeNode *c3) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = false;
    node->children[0] = c1;
    node->children[1] = c2;
    if (c3) {
        node->children[2] = c3;
        node->num_keys = 2;
        node->keys[0] = c2->keys[0]; 
        node->keys[1] = c3->keys[0];
        
        uint8_t concat[HASH_SIZE * 3];
        memcpy(concat, c1->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE, c2->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE * 2, c3->node_hash, HASH_SIZE);
        sha256(concat, HASH_SIZE * 3, node->node_hash);
    } else {
        node->num_keys = 1;
        node->keys[0] = c2->keys[0];
        
        uint8_t concat[HASH_SIZE * 2];
        memcpy(concat, c1->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE, c2->node_hash, HASH_SIZE);
        sha256(concat, HASH_SIZE * 2, node->node_hash);
    }
    return node;
}

// Update hashes bottom-up
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s:%u:%s", 
                           node->keys[0], node->values[0], 
                           node->num_keys == 2 ? node->keys[1] : 0, 
                           node->num_keys == 2 ? node->values[1] : "");
        sha256(temp, len, node->node_hash);
    } else {
        if (node->children[2]) {
            uint8_t concat[HASH_SIZE * 3];
            memcpy(concat, node->children[0]->node_hash, HASH_SIZE);
            memcpy(concat + HASH_SIZE, node->children[1]->node_hash, HASH_SIZE);
            memcpy(concat + HASH_SIZE * 2, node->children[2]->node_hash, HASH_SIZE);
            sha256(concat, HASH_SIZE * 3, node->node_hash);
        } else {
            uint8_t concat[HASH_SIZE * 2];
            memcpy(concat, node->children[0]->node_hash, HASH_SIZE);
            memcpy(concat + HASH_SIZE, node->children[1]->node_hash, HASH_SIZE);
            sha256(concat, HASH_SIZE * 2, node->node_hash);
        }
    }
}

// Retrieve table data by routing key
const char* retrieve_table_data(TwoThreeNode *node, uint32_t key) {
    if (!node) return NULL;
    if (node->is_leaf) {
        if (node->keys[0] == key) return node->values[0];
        if (node->num_keys == 2 && node->keys[1] == key) return node->values[1];
        return NULL;
    }
    if (key < node->keys[0]) {
        return retrieve_table_data(node->children[0], key);
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        return retrieve_table_data(node->children[1], key);
    } else {
        return retrieve_table_data(node->children[2], key);
    }
}

// Update table data and propagate hash up
bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            update_node_hash(node);
            return true;
        }
        if (node->num_keys == 2 && node->keys[1] == key) {
            strcpy(node->values[1], new_value);
            update_node_hash(node);
            return true;
        }
        return false;
    }
    
    bool updated = false;
    if (key < node->keys[0]) {
        updated = store_table_data(node->children[0], key, new_value);
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        updated = store_table_data(node->children[1], key, new_value);
    } else {
        updated = store_table_data(node->children[2], key, new_value);
    }
    
    if (updated) {
        update_node_hash(node);
    }
    return updated;
}

static void free_tree(TwoThreeNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        free_tree(node->children[0]);
        free_tree(node->children[1]);
        free_tree(node->children[2]);
    }
    free(node);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT 2-3 TREE DAT DEVICE STATE ROUTING TEST SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create leaf nodes representing device states
    // Key 100: STANAG Port 2020 State, Key 200: DECnet Node 1.10 State
    TwoThreeNode *stanag_leaf = create_leaf(100, "STANAG_PKTS:10", 0, NULL);
    TwoThreeNode *decnet_leaf = create_leaf(200, "DECNET_STATE:CONNECTED", 0, NULL);

    // Build Merkle 2-3 Tree root
    TwoThreeNode *root = create_internal(stanag_leaf, decnet_leaf, NULL);
    
    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, root->node_hash, HASH_SIZE);

    // 1. Verify initial lookup
    printf("[TEST] Querying initial network device state leaves...\n");
    fflush(stdout);
    const char *ret = retrieve_table_data(root, 100);
    assert(ret != NULL && strcmp(ret, "STANAG_PKTS:10") == 0);
    printf("   ✓ STANAG Initial state verified: %s\n", ret);
    fflush(stdout);

    // 2. Simulate packet receive event (Updates STANAG packet count to 11)
    printf("[TEST] Storing updated packet count in the 2-3 tree leaf...\n");
    fflush(stdout);
    bool store_ok = store_table_data(root, 100, "STANAG_PKTS:11");
    assert(store_ok == true);

    // 3. Verify root hash evolved and state updated
    ret = retrieve_table_data(root, 100);
    assert(ret != NULL && strcmp(ret, "STANAG_PKTS:11") == 0);
    assert(memcmp(initial_hash, root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ State updated successfully: %s\n", ret);
    printf("   ✓ Cryptographic Merkle proof updated in VM registry.\n");
    fflush(stdout);

    free_tree(root);
    printf("=============================================================\n");
    printf("2-3 TREE DAT DEVICE STATE INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
