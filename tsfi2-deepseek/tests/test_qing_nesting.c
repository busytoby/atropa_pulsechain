#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_qing_graph.h"



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

    // Left child has a nested child that contains a Qing graph node!
    tsfi_qing_graph_node nested_qing_nodes[TSFI_NET_COUNT];
    tsfi_qing_graph_init(nested_qing_nodes);
    CachedContract c2;
    strcpy(c2.name, "deep_nested_qing");
    c2.virtual_address = 0x999ULL;
    nested_qing_nodes[TSFI_NET_ZMM].bst_root = tsfi_qing_bst_insert(nested_qing_nodes[TSFI_NET_ZMM].bst_root, c2.virtual_address, &c2);

    tsfi_23_node *deep_nested_child = create_node(NODE_TYPE_QING_GRAPH);
    deep_nested_child->payload.qing_graph = &nested_qing_nodes[TSFI_NET_ZMM];
    child_left->children[child_left->child_count++] = deep_nested_child;

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

    // 4. Verify Recursive Search down the 2-3 tree
    printf("[NEST] Verifying recursive search down the 2-3 Tree...\n");
    CachedContract *recursive_res = tsfi_23_node_search(root, 0x999ULL);
    assert(recursive_res != NULL);
    assert(strcmp(recursive_res->name, "deep_nested_qing") == 0);
    printf("  [PASS] Successfully retrieved deep nested Qing 0x999 via recursive search.\n");

    // 5. Verify Dynamic 2-3 Tree Node Split
    printf("[NEST] Verifying 2-3 Tree Node Split mechanics...\n");
    tsfi_23_node *split_test_root = create_node(NODE_TYPE_RAW_DATA);
    tsfi_23_node *c_child1 = create_node(NODE_TYPE_RAW_DATA);
    tsfi_23_node *c_child2 = create_node(NODE_TYPE_RAW_DATA);
    tsfi_23_node *c_child3 = create_node(NODE_TYPE_RAW_DATA);
    tsfi_23_node *c_child4 = create_node(NODE_TYPE_RAW_DATA);

    tsfi_23_node *sibling_out = NULL;
    tsfi_23_node_add_child(split_test_root, c_child1, &sibling_out);
    assert(sibling_out == NULL);
    assert(split_test_root->child_count == 1);

    tsfi_23_node_add_child(split_test_root, c_child2, &sibling_out);
    assert(sibling_out == NULL);
    assert(split_test_root->child_count == 2);

    tsfi_23_node_add_child(split_test_root, c_child3, &sibling_out);
    assert(sibling_out == NULL);
    assert(split_test_root->child_count == 3);

    // Adding 4th child triggers split
    tsfi_23_node_add_child(split_test_root, c_child4, &sibling_out);
    assert(sibling_out != NULL);
    assert(split_test_root->child_count == 2);
    assert(sibling_out->child_count == 2);
    printf("  [PASS] Successfully verified 2-3 tree split and child balancing.\n");

    // 6. Verify Dynamic B-Tree Ordered Insert & Root Splits
    printf("[NEST] Verifying 2-3 Tree dynamic ordered key insertion & split propagation...\n");
    tsfi_23_node *dynamic_root = NULL;
    
    // We will insert 4 keys
    dynamic_root = tsfi_23_insert(dynamic_root, 10, NODE_TYPE_RAW_DATA, (void*)0x10);
    dynamic_root = tsfi_23_insert(dynamic_root, 20, NODE_TYPE_RAW_DATA, (void*)0x20);
    dynamic_root = tsfi_23_insert(dynamic_root, 30, NODE_TYPE_RAW_DATA, (void*)0x30);
    
    // At this point, it's a leaf node with 3 keys, so the insert of 30 triggers a split.
    // The root should split, making key count in root 1, and creating child nodes.
    assert(dynamic_root != NULL);
    assert(dynamic_root->is_leaf == 0);
    assert(dynamic_root->key_count == 1);
    assert(dynamic_root->keys[0] == 20); // 20 pushed up as parent split key

    // Insert 4th key
    dynamic_root = tsfi_23_insert(dynamic_root, 40, NODE_TYPE_RAW_DATA, (void*)0x40);

    // Let's search for keys recursively down this dynamic 2-3 tree
    CachedContract mock_c;
    strcpy(mock_c.name, "dynamic_nested_contract");
    mock_c.virtual_address = 0x888ULL;

    // Nest our mock contract under one of the payloads to prove hybrid lookup
    tsfi_qing_graph_node hybrid_qing_nodes[TSFI_NET_COUNT];
    tsfi_qing_graph_init(hybrid_qing_nodes);
    hybrid_qing_nodes[TSFI_NET_ZMM].bst_root = tsfi_qing_bst_insert(hybrid_qing_nodes[TSFI_NET_ZMM].bst_root, mock_c.virtual_address, &mock_c);

    tsfi_23_node *hybrid_node = create_node(NODE_TYPE_QING_GRAPH);
    hybrid_node->payload.qing_graph = &hybrid_qing_nodes[TSFI_NET_ZMM];

    // Attach hybrid node as child to dynamic root
    dynamic_root->children[dynamic_root->child_count++] = hybrid_node;

    // Search hybrid nested contract
    CachedContract *hybrid_res = tsfi_23_node_search(dynamic_root, 0x888ULL);
    assert(hybrid_res != NULL);
    assert(strcmp(hybrid_res->name, "dynamic_nested_contract") == 0);
    printf("  [PASS] Successfully verified dynamic split key insertion and recursive hybrid searches.\n");

    // Cleanup
    tsfi_qing_graph_destroy(qing_nodes);
    tsfi_qing_graph_destroy(nested_qing_nodes);
    tsfi_qing_graph_destroy(hybrid_qing_nodes);
    
    // Deep recursive destruction
    tsfi_23_node_destroy(root);
    tsfi_23_node_destroy(split_test_root);
    tsfi_23_node_destroy(sibling_out);
    tsfi_23_node_destroy(dynamic_root);

    printf("=== ALL NESTING TESTS PASSED ===\n");
    return 0;
}
