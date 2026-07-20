#ifndef TSFI_PARC_CSMACD_H
#define TSFI_PARC_CSMACD_H

#include <stdint.h>

typedef struct {
    int attempts;
    int backoff_slots;
    int is_colliding;
} tsfi_parc_network_node_t;

/* Execute Metcalfe's exponential backoff calculation after a collision */
int tsfi_parc_csmacd_backoff(tsfi_parc_network_node_t *node, int max_backoff_slots);

/* Simulate packet transmission and collision detection between two nodes */
int tsfi_parc_csmacd_transmit(tsfi_parc_network_node_t *tx_node, tsfi_parc_network_node_t *rx_node, int collision_chance);

#endif // TSFI_PARC_CSMACD_H
