#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef enum {
    LINK_UNREACHABLE,
    LINK_DOWN,       // Addressable on disk via .dat.bin, but not loaded in memory
    LINK_UP          // Actively loaded in memory
} link_status_t;

// Simulated in-memory quadtree payload
typedef struct {
    uint32_t payload_data;
} quadtree_payload_t;

// 2-3 Tree Node representing the memory-mapped lookup database
typedef struct TwoThreeNode {
    uint32_t key;
    quadtree_payload_t *payload; // In-memory loaded payload
    link_status_t status;
    char dat_bin_path[256];      // Disk file path for paging
} TwoThreeNode;

// -------------------------------------------------------------
// Reactive Paging Operations based on Link-State Updates
// -------------------------------------------------------------
bool page_in_quadtree(TwoThreeNode *node) {
    if (node->status == LINK_UP) {
        return true; // Already loaded
    }

    // Verify it is addressable on disk before loading
    FILE *fp = fopen(node->dat_bin_path, "rb");
    if (!fp) {
        node->status = LINK_UNREACHABLE;
        return false;
    }

    // Allocate memory and page-in the payload
    node->payload = malloc(sizeof(quadtree_payload_t));
    assert(node->payload != NULL);

    size_t read_bytes = fread(node->payload, sizeof(quadtree_payload_t), 1, fp);
    fclose(fp);

    if (read_bytes == 1) {
        node->status = LINK_UP; // State transitioned to UP
        return true;
    }

    free(node->payload);
    node->payload = NULL;
    node->status = LINK_UNREACHABLE;
    return false;
}

void page_out_quadtree(TwoThreeNode *node) {
    if (node->payload) {
        free(node->payload);
        node->payload = NULL;
    }

    // Check if it remains addressable on disk to determine post-unload state
    if (access(node->dat_bin_path, F_OK) == 0) {
        node->status = LINK_DOWN; // Successfully unloaded to disk
    } else {
        node->status = LINK_UNREACHABLE;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT QUADTREE REACTIVE PAGING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create temporary .dat.bin database slice file for paging validation
    char temp_bin_path[] = "tests/page_slice_001.dat.bin";
    FILE *fp = fopen(temp_bin_path, "wb");
    assert(fp != NULL);
    quadtree_payload_t dummy = { .payload_data = 0xABCDE };
    size_t written = fwrite(&dummy, sizeof(quadtree_payload_t), 1, fp);
    assert(written == 1);
    fclose(fp);

    // Initialize 2-3 tree node in LINK_DOWN state (addressable on disk, not in memory)
    TwoThreeNode node = {
        .key = 42,
        .payload = NULL,
        .status = LINK_DOWN,
        .dat_bin_path = "tests/page_slice_001.dat.bin"
    };

    // 1. Verify initial state
    assert(node.status == LINK_DOWN);
    assert(node.payload == NULL);
    printf("[TEST] Initial state is LINK_DOWN (addressable on disk)...\n");
    fflush(stdout);

    // 2. Trigger Page-In (Link State transitions to LINK_UP)
    printf("[TEST] Triggering Page-In (Link State -> LINK_UP)...\n");
    fflush(stdout);
    bool ok = page_in_quadtree(&node);
    assert(ok == true);
    assert(node.status == LINK_UP);
    assert(node.payload != NULL);
    assert(node.payload->payload_data == 0xABCDE);
    printf("   ✓ Payload loaded into memory. Status updated to LINK_UP.\n");
    fflush(stdout);

    // 3. Trigger Page-Out (Link State transitions back to LINK_DOWN)
    printf("[TEST] Triggering Page-Out (Link State -> LINK_DOWN)...\n");
    fflush(stdout);
    page_out_quadtree(&node);
    assert(node.status == LINK_DOWN);
    assert(node.payload == NULL);
    printf("   ✓ Payload unloaded from memory. Status restored to LINK_DOWN.\n");
    fflush(stdout);

    // Clean up temporary database slice file
    unlink(temp_bin_path);

    printf("=============================================================\n");
    printf("QUADTREE PAGING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
