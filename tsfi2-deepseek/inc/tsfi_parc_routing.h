#ifndef TSFI_PARC_ROUTING_H
#define TSFI_PARC_ROUTING_H

#include <stdint.h>

typedef struct {
    uint8_t dest_net;
    uint8_t next_gateway;
    uint16_t metric;
} tsfi_parc_route_t;

typedef struct {
    tsfi_parc_route_t routes[16];
    int route_count;
} tsfi_parc_routing_table_t;

/* Initialize empty routing table */
void tsfi_parc_routing_init(tsfi_parc_routing_table_t *rt);

/* Add or update a network route mapping */
int tsfi_parc_routing_add(tsfi_parc_routing_table_t *rt, uint8_t net, uint8_t gateway, uint16_t metric);

/* Lookup next gateway hop address for a destination network */
int tsfi_parc_routing_lookup(const tsfi_parc_routing_table_t *rt, uint8_t net, uint8_t *gateway_out);

#endif // TSFI_PARC_ROUTING_H
