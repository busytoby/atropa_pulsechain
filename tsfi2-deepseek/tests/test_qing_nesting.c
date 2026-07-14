#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_qing_graph.h"

typedef enum {
    NODE_TYPE_QING_GRAPH,
    NODE_TYPE_RAW_DATA,
    NODE_TYPE_FIRMWARE
} tsfi_node_type;

typedef struct tsfi_23_node {
    tsfi_node_type type;
    union {
        tsfi_qing_graph_node *qing_graph;
        struct {
            uint64_t key;
            uint64_t val;
        } raw;
        struct {
            char name[64];
            uint32_t checksum;
        } fw;
    } payload;
    struct tsfi_23_node *children[3];
    int child_count;
} tsfi_23_node;

static tsfi_23_node* create_node(tsfi_node_type type) {
    tsfi_23_node *node = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
    assert(node != NULL);
    node->type = type;
    node->child_count = 0;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

int main(void) {
    printf("=== TSFi 2-3 Tree Heterogeneous Nesting Verification ===\n");

    // 1. Initialize ZMM Qing network graph
    tsfi_qing_graph_node qing_nodes[TSFI_NET_COUNT];
    tsfi_qing_graph_init(qing_nodes);
    CachedContract c1;
    strcpy(c1.name, "root_nest_qing");
    c1.virtual_address = 0x777ULL;
    qing_nodes[TSFI_NET_ZMM].bst_root = tsfi_qing_bst_insert(qing_nodes[TSFI_NET_ZMM].bst_root, c1.virtual_address, &c1);

    // 2. Build 2-3 Tree Structure with Nested Payloads
    tsfi_23_node *root = create_node(NODE_TYPE_QING_GRAPH);
    root->payload.qing_graph = &qing_nodes[TSFI_NET_ZMM];

    // Left child: Raw Data node (non-Qing)
    tsfi_23_node *child_left = create_node(NODE_TYPE_RAW_DATA);
    child_left->payload.raw.key = 0xABCDULL;
    child_left->payload.raw.val = 0x1234ULL;
    root->children[root->child_count++] = child_left;

    // Right child: Firmware node (non-Qing)
    tsfi_23_node *child_right = create_node(NODE_TYPE_FIRMWARE);
    strcpy(child_right->payload.fw.name, "Heltec_Ook_LoRa");
    child_right->payload.fw.checksum = 0xAA55F00D;
    root->children[root->child_count++] = child_right;

    // 3. Verify Heterogeneous Nesting Retrieval
    printf("[NEST] Verifying root node (Qing Graph type)...\n");
    assert(root->type == NODE_TYPE_QING_GRAPH);
    CachedContract *res = tsfi_qing_bst_find(root->payload.qing_graph->bst_root, 0x777ULL);
    assert(res != NULL);
    assert(strcmp(res->name, "root_nest_qing") == 0);
    printf("  [PASS] Successfully retrieved Qing address 0x777 from root node.\n");

    printf("[NEST] Verifying left child node (Raw Data type)...\n");
    assert(root->children[0]->type == NODE_TYPE_RAW_DATA);
    assert(root->children[0]->payload.raw.key == 0xABCDULL);
    assert(root->children[0]->payload.raw.val == 0x1234ULL);
    printf("  [PASS] Successfully verified raw data payloads (Key: 0xABCD, Val: 0x1234).\n");

    printf("[NEST] Verifying right child node (Firmware type)...\n");
    assert(root->children[1]->type == NODE_TYPE_FIRMWARE);
    assert(strcmp(root->children[1]->payload.fw.name, "Heltec_Ook_LoRa") == 0);
    assert(root->children[1]->payload.fw.checksum == 0xAA55F00D);
    printf("  [PASS] Successfully verified firmware payloads (Name: %s, Checksum: 0x%X).\n", 
           root->children[1]->payload.fw.name, root->children[1]->payload.fw.checksum);

    // Cleanup
    tsfi_qing_graph_destroy(qing_nodes);
    free(child_left);
    free(child_right);
    free(root);

    printf("=== ALL NESTING TESTS PASSED ===\n");
    return 0;
}
