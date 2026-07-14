#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

// Simple SHA256 helper
void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}

typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128]; // Table-based data payload
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Helper to create a leaf node
TwoThreeNode* create_leaf(uint32_t key1, const char *val1, uint32_t key2, const char *val2) {
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
TwoThreeNode* create_internal(TwoThreeNode *c1, TwoThreeNode *c2, TwoThreeNode *c3) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = false;
    node->children[0] = c1;
    node->children[1] = c2;
    if (c3) {
        node->children[2] = c3;
        node->num_keys = 2;
        node->keys[0] = c2->keys[0]; // standard 2-3 tree routing keys
        node->keys[1] = c3->keys[0];
        
        // Hash child nodes (3-node)
        uint8_t concat[HASH_SIZE * 3];
        memcpy(concat, c1->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE, c2->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE * 2, c3->node_hash, HASH_SIZE);
        sha256(concat, HASH_SIZE * 3, node->node_hash);
    } else {
        node->num_keys = 1;
        node->keys[0] = c2->keys[0];
        
        // Hash child nodes (2-node)
        uint8_t concat[HASH_SIZE * 2];
        memcpy(concat, c1->node_hash, HASH_SIZE);
        memcpy(concat + HASH_SIZE, c2->node_hash, HASH_SIZE);
        sha256(concat, HASH_SIZE * 2, node->node_hash);
    }
    return node;
}

// Update hashes bottom-up
void update_node_hash(TwoThreeNode *node) {
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

// Update table data at a key and propagate hash up
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

void free_tree(TwoThreeNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        free_tree(node->children[0]);
        free_tree(node->children[1]);
        free_tree(node->children[2]);
    }
    free(node);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACAB: DYNAMIC 2-3 MERKLE TABLE STORE & LOOKUP\n");
    printf("=============================================================\n");

    // Create leaf nodes with table data
    TwoThreeNode *leaf1 = create_leaf(10, "RowData_10", 20, "RowData_20");
    TwoThreeNode *leaf2 = create_leaf(30, "RowData_30", 0, NULL);
    TwoThreeNode *leaf3 = create_leaf(40, "RowData_40", 50, "RowData_50");
    
    // Assemble into 2-3 Tree structure
    // Node A (2-node): leaf1, leaf2
    TwoThreeNode *nodeA = create_internal(leaf1, leaf2, NULL);
    // Root (2-node): Node A, leaf3
    TwoThreeNode *root = create_internal(nodeA, leaf3, NULL);
    
    uint8_t initial_root_hash[HASH_SIZE];
    memcpy(initial_root_hash, root->node_hash, HASH_SIZE);
    
    // Retrieve tests
    printf("1. Querying table data from tree nodes:\n");
    const char *retrieved = retrieve_table_data(root, 20);
    assert(retrieved != NULL);
    assert(strcmp(retrieved, "RowData_20") == 0);
    printf("   ✓ Key 20: %s\n", retrieved);
    
    retrieved = retrieve_table_data(root, 40);
    assert(retrieved != NULL);
    assert(strcmp(retrieved, "RowData_40") == 0);
    printf("   ✓ Key 40: %s\n", retrieved);
    
    // Update data dynamically
    printf("2. Permanently storing new row data at key 40...\n");
    bool store_ok = store_table_data(root, 40, "Updated_RowData_40");
    assert(store_ok == true);
    
    // Retrieve again to confirm
    retrieved = retrieve_table_data(root, 40);
    assert(retrieved != NULL);
    assert(strcmp(retrieved, "Updated_RowData_40") == 0);
    printf("   ✓ Retrieved Updated Key 40: %s\n", retrieved);
    
    // Root hash should have evolved due to the change
    assert(memcmp(initial_root_hash, root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Merkle path evolved successfully. Root hash modified.\n");
    
    free_tree(root);
    printf("=============================================================\n");
    printf("AUNCIENT 2-3 MERKLE TABLE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
