#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef enum {
    LINK_UNREACHABLE,
    LINK_DOWN,       // Addressable on disk via .dat.bin lookup table, but not loaded in memory
    LINK_UP          // Actively loaded in memory
} link_status_t;

typedef struct {
    uint32_t node_id;
    bool is_cached_in_memory;
    char dat_bin_path[256]; // Addressable path to the quadtree database slice (.dat.bin)
} quadtree_link_entry_t;

// -------------------------------------------------------------
// Quadtree Link-State Status Evaluator (Rule 13 Compliant)
// -------------------------------------------------------------
link_status_t evaluate_quadtree_link_status(const quadtree_link_entry_t *entry) {
    if (entry->is_cached_in_memory) {
        return LINK_UP;
    }

    // Check if the binary database slice is addressable on disk
    if (access(entry->dat_bin_path, F_OK) == 0) {
        return LINK_DOWN; // Addressable via lookup table on disk, but not in memory
    }

    return LINK_UNREACHABLE;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT QUADTREE LINK-STATE STATUS SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create a temporary dummy .dat.bin database slice file for validation
    char temp_bin_path[] = "tests/slice_001.dat.bin";
    FILE *fp = fopen(temp_bin_path, "wb");
    assert(fp != NULL);
    uint32_t dummy_data = 0xDEADC0DE;
    size_t written = fwrite(&dummy_data, sizeof(uint32_t), 1, fp);
    assert(written == 1);
    fclose(fp);

    // 1. Link Up: Node is actively cached in memory
    quadtree_link_entry_t entry_up = {
        .node_id = 10,
        .is_cached_in_memory = true,
        .dat_bin_path = "tests/slice_001.dat.bin"
    };

    printf("[TEST] Checking LINK_UP status...\n");
    fflush(stdout);
    link_status_t status = evaluate_quadtree_link_status(&entry_up);
    assert(status == LINK_UP);
    printf("   ✓ Link is UP (actively cached).\n");
    fflush(stdout);

    // 2. Link Down: Node is not cached, but addressable via .dat.bin lookup table
    quadtree_link_entry_t entry_down = {
        .node_id = 11,
        .is_cached_in_memory = false,
        .dat_bin_path = "tests/slice_001.dat.bin"
    };

    printf("[TEST] Checking LINK_DOWN status...\n");
    fflush(stdout);
    status = evaluate_quadtree_link_status(&entry_down);
    assert(status == LINK_DOWN);
    printf("   ✓ Link is DOWN (not cached, but addressable on disk via .dat.bin lookup).\n");
    fflush(stdout);

    // 3. Link Unreachable: Node is not cached and the .dat.bin file does not exist
    quadtree_link_entry_t entry_unreachable = {
        .node_id = 12,
        .is_cached_in_memory = false,
        .dat_bin_path = "tests/nonexistent_slice.dat.bin"
    };

    printf("[TEST] Checking LINK_UNREACHABLE status...\n");
    fflush(stdout);
    status = evaluate_quadtree_link_status(&entry_unreachable);
    assert(status == LINK_UNREACHABLE);
    printf("   ✓ Link is UNREACHABLE (not in memory and database slice missing).\n");
    fflush(stdout);

    // Clean up temporary database slice file
    unlink(temp_bin_path);

    printf("=============================================================\n");
    printf("QUADTREE LINK-STATE STATUS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
