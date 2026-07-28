#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// 1. 2-3 Tree Table Node Definition
typedef struct {
    uint32_t key1;
    uint32_t key2;
    char val1[8];
    char val2[8];
    uint32_t has_two_keys;
} node_23_t;

// 2. RDBMS Database Glyph Record Definition
typedef struct {
    uint32_t glyph_id;
    uint32_t width;
    uint32_t height;
    char symbol[8];
} glyph_record_t;

// 1. RED ANALYZER Gating: Static validation of database assets
static bool red_analyzer_validate_db_format(const char *filepath) {
    if (!filepath) return false;
    // Rule 13: Must support only .dat.bin formats for database slices
    if (strstr(filepath, ".dat.bin") == NULL) {
        return false; // Rejected by RED rail
    }
    return true;
}

// 2. BLACK XPLSM Gating: 2-3 Tree search lookup with recursion limit
#define MAX_LOOKUP_DEPTH 3
static bool black_xplsm_search_23_tree(
    const node_23_t *tree_array,
    uint32_t index,
    uint32_t target_key,
    char *val_out,
    uint32_t current_depth,
    uint32_t *interrupt_out
) {
    if (current_depth > MAX_LOOKUP_DEPTH) {
        if (interrupt_out) *interrupt_out = 34; // Trap 34: 2-3 Tree Search Depth Overflow
        return false; // BLACK rail block
    }

    node_23_t node = tree_array[index];
    if (target_key == node.key1) {
        strcpy(val_out, node.val1);
        return true;
    }
    if (node.has_two_keys && target_key == node.key2) {
        strcpy(val_out, node.val2);
        return true;
    }

    // Binary / Ternary branch simulation
    uint32_t next_index = index + 1; // Simplistic linear index offset branch
    return black_xplsm_search_23_tree(tree_array, next_index, target_key, val_out, current_depth + 1, interrupt_out);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS 2-3 TREE & RDBMS DATABASE VALIDATION SUITE\n");
    printf("=============================================================\n");

    const char *db_file = "graphics_assets.dat.bin";
    const char *bad_file = "graphics_assets.json";

    // Test 1: RED rail file format gating
    printf("[TEST] Checking database file extensions on RED rail...\n");
    assert(red_analyzer_validate_db_format(db_file) == true);
    assert(red_analyzer_validate_db_format(bad_file) == false);
    printf("   ✓ RED rail successfully checked extension constraints.\n");

    // Setup mock RDBMS Glyph Database
    printf("[TEST] Deploying RDBMS Glyph records to graphics_assets.dat.bin...\n");
    glyph_record_t rdbms_table[] = {
        { 1, 1, 1, "O" }, // Sun glyph
        { 2, 2, 1, "^^T" } // Tiger glyph
    };

    FILE *f = fopen(db_file, "wb");
    assert(f != NULL);
    fwrite(rdbms_table, sizeof(glyph_record_t), 2, f);
    fclose(f);

    // Read back and verify tables
    FILE *in = fopen(db_file, "rb");
    assert(in != NULL);
    glyph_record_t loaded_table[2];
    assert(fread(loaded_table, sizeof(glyph_record_t), 2, in) == 2);
    fclose(in);
    remove(db_file);

    assert(strcmp(loaded_table[0].symbol, "O") == 0);
    assert(strcmp(loaded_table[1].symbol, "^^T") == 0);
    printf("   ✓ RDBMS relational glyph records successfully written and validated.\n");

    // Setup mock 2-3 Tree database
    node_23_t tree[] = {
        { 10, 20, "SUN", "TIGER", 1 },
        { 30, 40, "SKY", "LAND",  1 }
    };

    // Test 2: BLACK rail 2-3 tree depth boundary checks
    printf("[TEST] Testing 2-3 tree search execution under BLACK rail...\n");
    char val[8] = {0};
    uint32_t trap_code = 0;

    // Successful search
    assert(black_xplsm_search_23_tree(tree, 0, 20, val, 0, &trap_code) == true);
    assert(strcmp(val, "TIGER") == 0);

    // Search exceeds max recursion depth
    trap_code = 0;
    assert(black_xplsm_search_23_tree(tree, 0, 999, val, 4, &trap_code) == false);
    assert(trap_code == 34); // Trap 34: Depth Overflow
    printf("   ✓ BLACK rail successfully trapped 2-3 tree search overflow (Trap 34).\n");

    printf("=============================================================\n");
    printf("ALL 2-3 TREE AND RDBMS DATABASE TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
