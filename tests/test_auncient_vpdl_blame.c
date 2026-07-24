#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MAX_QUARANTINE 8

typedef struct {
    uint64_t sequence;
    uint32_t sender_id;
    bool drift_detected;
} vpdl_packet_t;

// Forward declaration of node context
struct vpdl_blame_node;

typedef uint32_t (*vpdl_blame_rule_fn)(const struct vpdl_blame_node *node, const vpdl_packet_t *pkt);

typedef struct vpdl_blame_node {
    uint32_t quarantined_senders[MAX_QUARANTINE];
    int quarantine_count;
    vpdl_blame_rule_fn active_rule;
} vpdl_blame_node_t;

// Default Routing Rule: Route all to Port 1 regardless of sender status
static uint32_t default_route(const vpdl_blame_node_t *node, const vpdl_packet_t *pkt) {
    (void)node;
    (void)pkt;
    return 1; // Default Port
}

// Dynamically Generated Blame Rule: Route quarantined senders to Port 9 (Quarantine)
static uint32_t dynamic_blame_route(const vpdl_blame_node_t *node, const vpdl_packet_t *pkt) {
    for (int i = 0; i < node->quarantine_count; i++) {
        if (node->quarantined_senders[i] == pkt->sender_id) {
            return 9; // Port 9 - Quarantine Port
        }
    }
    return 1; // Port 1 - Standard Port
}

// Agent function: scans incoming packet vectors for clock drift and updates policies
static void agent_monitor_and_blame(vpdl_blame_node_t *node, const vpdl_packet_t *pkt) {
    if (pkt->drift_detected) {
        // Add sender to quarantine list if space exists
        if (node->quarantine_count < MAX_QUARANTINE) {
            // Check for duplicates
            bool exists = false;
            for (int i = 0; i < node->quarantine_count; i++) {
                if (node->quarantined_senders[i] == pkt->sender_id) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                node->quarantined_senders[node->quarantine_count++] = pkt->sender_id;
            }
        }
        // Hot-swap routing logic to target the blamed sender
        node->active_rule = dynamic_blame_route;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPDL AGENTIC BLAME & QUARANTINE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    vpdl_blame_node_t node = {
        .quarantine_count = 0,
        .active_rule = default_route
    };

    vpdl_packet_t pkt_good = { .sequence = 1001, .sender_id = 0x1234, .drift_detected = false };
    vpdl_packet_t pkt_drift = { .sequence = 1002, .sender_id = 0xFA99, .drift_detected = true };
    vpdl_packet_t pkt_subsequent = { .sequence = 1003, .sender_id = 0xFA99, .drift_detected = false };

    // 1. Initial State: Normal routing
    printf("[TEST] Routing packets under default APDL rules...\n");
    fflush(stdout);
    assert(node.active_rule(&node, &pkt_good) == 1);
    assert(node.active_rule(&node, &pkt_drift) == 1);
    printf("   ✓ All packets routed to Port 1 successfully.\n");
    fflush(stdout);

    // 2. Agent scans packet stream, detects drift, and updates policy
    printf("[TEST] Agent monitoring stream and executing blame lookup...\n");
    fflush(stdout);
    agent_monitor_and_blame(&node, &pkt_drift);
    assert(node.active_rule == dynamic_blame_route);
    assert(node.quarantine_count == 1);
    assert(node.quarantined_senders[0] == 0xFA99);
    printf("   ✓ Agent successfully blamed sender 0xFA99 and hot-swapped the route rules.\n");
    fflush(stdout);

    // 3. Verify hot-swapped route behavior
    printf("[TEST] Routing subsequent packets under active blame rule...\n");
    fflush(stdout);
    
    // Good sender should still go to Port 1
    uint32_t port_good = node.active_rule(&node, &pkt_good);
    assert(port_good == 1);

    // Blamed sender should now go to Port 9 (Quarantine)
    uint32_t port_bad = node.active_rule(&node, &pkt_subsequent);
    assert(port_bad == 9);

    printf("   ✓ Valid traffic continues to Port 1; Blamed traffic quarantined to Port 9.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VPDL BLAME TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
