#include <stdlib.h>
#include "tsfi_parc_csmacd.h"

int tsfi_parc_csmacd_backoff(tsfi_parc_network_node_t *node, int max_backoff_slots) {
    if (!node) return -1;

    if (node->attempts > 16) {
        node->attempts = 16;
    }
    node->attempts++;
    // Binary exponential backoff limit
    int limit = 1 << (node->attempts > 10 ? 10 : node->attempts);
    
    // Choose random slot in [0, limit-1]
    node->backoff_slots = rand() % limit;
    if (node->backoff_slots > max_backoff_slots) {
        node->backoff_slots = max_backoff_slots;
    }
    return node->backoff_slots;
}

int tsfi_parc_csmacd_transmit(tsfi_parc_network_node_t *tx_node, tsfi_parc_network_node_t *rx_node, int collision_chance) {
    if (!tx_node || !rx_node) return -1;

    // Check if transmitter is backed off
    if (tx_node->backoff_slots > 0) {
        tx_node->backoff_slots--;
        return 1; // backed off (waiting)
    }

    // Simulate collision probability
    if ((rand() % 100) < collision_chance) {
        tx_node->is_colliding = 1;
        rx_node->is_colliding = 1;
        tsfi_parc_csmacd_backoff(tx_node, 256);
        return -2; // collision detected
    }

    // Successful transmission
    tx_node->attempts = 0;
    tx_node->is_colliding = 0;
    return 0; // success
}
