#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_ENTRIES 16

typedef struct {
    uint32_t block_id;
    char key[32];
    uint64_t file_position; // Memory or file location pointer

} block_entry_t;

typedef struct {
    uint32_t total_blocks;
    block_entry_t entries[MAX_ENTRIES];
} tape_search_directory_t;

// Find data position using high-speed block header index scan
int64_t search_dat_tape_directory(const tape_search_directory_t *dir, const char *target_key) {
    printf("   [TAPE CONTROLLER] Scanning directory headers for key: '%s'...\n", target_key);
    
    for (uint32_t i = 0; i < dir->total_blocks; i++) {
        // Simulating rapid header scan bypassing main payloads
        if (strcmp(dir->entries[i].key, target_key) == 0) {
            printf("   [TAPE CONTROLLER] Block match found! ID: %u | Target Position: %lu\n", 
                   dir->entries[i].block_id, dir->entries[i].file_position);
            return (int64_t)dir->entries[i].file_position;
        }
    }
    
    printf("   [TAPE CONTROLLER] Key '%s' not found in directory headers.\n", target_key);
    return -1;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DAT TREE TAPE-SEARCH DIRECTORY SUITE\n");
    printf("=============================================================\n");

    // Initialize search directory structure (simulating headers of .dat.bin files)
    tape_search_directory_t directory = {
        .total_blocks = 3,
        .entries = {
            { .block_id = 101, .key = "quadtree_root", .file_position = 128 },
            { .block_id = 102, .key = "quadtree_leaf_left", .file_position = 2048 },
            { .block_id = 103, .key = "quadtree_leaf_right", .file_position = 4096 }
        }
    };

    // 1. Verify successful search for root node
    printf("[TEST] Searching for root node...\n");
    int64_t pos_root = search_dat_tape_directory(&directory, "quadtree_root");
    assert(pos_root == 128);
    printf("   ✓ Root node found at correct position.\n");

    // 2. Verify search for leaf node
    printf("[TEST] Searching for right leaf node...\n");
    int64_t pos_leaf = search_dat_tape_directory(&directory, "quadtree_leaf_right");
    assert(pos_leaf == 4096);
    printf("   ✓ Leaf node found at correct position.\n");

    // 3. Verify missing key returns error
    printf("[TEST] Searching for non-existent node...\n");
    int64_t pos_missing = search_dat_tape_directory(&directory, "quadtree_leaf_middle");
    assert(pos_missing == -1);
    printf("   ✓ Missing node correctly returned error.\n");

    printf("=============================================================\n");
    printf("TAPE-SEARCH DIRECTORY TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
