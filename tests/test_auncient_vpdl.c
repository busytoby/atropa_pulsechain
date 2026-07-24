#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint64_t sequence;
    uint32_t payload_val;
} vpdl_packet_t;

// Function pointer representing a dynamically compiled VPDL rule
typedef uint32_t (*vpdl_rule_fn)(const vpdl_packet_t *pkt);

// Default static routing rule: route everything to Port 1
static uint32_t default_routing_rule(const vpdl_packet_t *pkt) {
    (void)pkt;
    return 1; // Port 1
}

// VPDL Hot-Swappable Node
typedef struct {
    vpdl_rule_fn active_rule;
    uint64_t swaps_performed;
} vpdl_node_t;

// Emulates compiling a VPDL script and generating a rule evaluator
// Script: "if seq > 500 route 2 else route 1"
static uint32_t compiled_rule_seq_500(const vpdl_packet_t *pkt) {
    if (pkt->sequence > 500) {
        return 2; // Port 2
    }
    return 1; // Port 1
}

// Script: "if payload == 999 route 3 else route 1"
static uint32_t compiled_rule_payload_999(const vpdl_packet_t *pkt) {
    if (pkt->payload_val == 999) {
        return 3; // Port 3
    }
    return 1; // Port 1
}

// Dynamic Hot-Swap execution
static void vpdl_node_hotswap(vpdl_node_t *node, vpdl_rule_fn new_rule) {
    node->active_rule = new_rule;
    node->swaps_performed++;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPDL DYNAMIC POLICY HOT-SWAP SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize node with default static APDL routing rules
    vpdl_node_t node = {
        .active_rule = default_routing_rule,
        .swaps_performed = 0
    };

    vpdl_packet_t pkt_low = { .sequence = 100, .payload_val = 10 };
    vpdl_packet_t pkt_high = { .sequence = 600, .payload_val = 20 };
    vpdl_packet_t pkt_spec = { .sequence = 120, .payload_val = 999 };

    // 1. Initial State: Default routing (all to Port 1)
    printf("[TEST] Routing packets under default routing policy...\n");
    fflush(stdout);
    assert(node.active_rule(&pkt_low) == 1);
    assert(node.active_rule(&pkt_high) == 1);
    printf("   ✓ All packets successfully routed to Port 1.\n");
    fflush(stdout);

    // 2. Hot-Swap to VPDL Sequence-based Rule
    printf("[TEST] Hot-swapping to VPDL sequence filter rule (seq > 500 -> Port 2)...\n");
    fflush(stdout);
    vpdl_node_hotswap(&node, compiled_rule_seq_500);
    
    assert(node.active_rule(&pkt_low) == 1);
    assert(node.active_rule(&pkt_high) == 2); // Shifted
    assert(node.swaps_performed == 1);
    printf("   ✓ Low sequence routed to Port 1; High sequence dynamically routed to Port 2.\n");
    fflush(stdout);

    // 3. Hot-Swap to VPDL Payload-based Rule
    printf("[TEST] Hot-swapping to VPDL payload value rule (val == 999 -> Port 3)...\n");
    fflush(stdout);
    vpdl_node_hotswap(&node, compiled_rule_payload_999);

    assert(node.active_rule(&pkt_low) == 1);
    assert(node.active_rule(&pkt_spec) == 3); // Shifted
    assert(node.swaps_performed == 2);
    printf("   ✓ Special payload dynamically routed to Port 3.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VPDL POLICY HOT-SWAP TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
